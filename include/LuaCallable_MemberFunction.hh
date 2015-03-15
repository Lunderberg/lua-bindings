#ifndef _LUACALLABLE_MEMBERFUNCTION_H_
#define _LUACALLABLE_MEMBERFUNCTION_H_

#include <string>

#include <lua.hpp>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "TemplateUtils.hh"


namespace Lua{
  template<typename ClassType, typename T>
  class LuaCallable_MemberFunction;

  template<typename ClassType, typename RetVal, typename... Params>
  class LuaCallable_MemberFunction<ClassType, RetVal(Params...)> : public LuaCallable {
  public:
    LuaCallable_MemberFunction(RetVal (ClassType::*func)(Params...)) : func(func) { }
    virtual int call(lua_State* L){
      if(lua_gettop(L) != sizeof...(Params) + 1){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      void* storage = luaL_testudata(L, 1, class_registry_entry<ClassType>().c_str());

      if(!storage){
        throw LuaIncorrectUserData("Called method using incorrect type");
      }

      ClassType* obj = *reinterpret_cast<ClassType**>(storage);
      lua_remove(L, 1);

      return call_member_function_helper(build_indices<sizeof...(Params)>(), L, obj, func);
    }
  private:
    RetVal (ClassType::*func)(Params...);

    template<int... Indices, typename RetVal_func>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj,
                                           RetVal_func (ClassType::*func)(Params...)){
      RetVal output = (obj->*func)(Read<Params>(L, Indices+1)...);
      Push(L, output);
      return 1;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    template<int... Indices>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj,
                                           void (ClassType::*func)(Params...)){
      (obj->*func)(Read<Params>(L, Indices+1)...);
      return 0;
    }
#pragma GCC diagnostic pop
  };
}

#endif /* _LUACALLABLE_MEMBERFUNCTION_H_ */
