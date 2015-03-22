#ifndef _LUACALLABLE_MEMBERFUNCTION_H_
#define _LUACALLABLE_MEMBERFUNCTION_H_

#include <iostream>
#include <memory>
#include <string>

#include <lua.hpp>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"
#include "LuaRegistryNames.hh"
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
      lua_remove(L, 1);

      if(!storage){
        throw LuaIncorrectUserData("Called method using incorrect type");
      }

      auto ptr = static_cast<VariablePointer<ClassType>*>(storage);

      switch(ptr->type){
      case PointerType::shared_ptr:
        {
          return call_member_function_helper(build_indices<sizeof...(Params)>(),
                                             L, *ptr->pointers.shared_ptr, func);
        }
      case PointerType::weak_ptr:
        {
          if(auto lock = ptr->pointers.weak_ptr.lock()){
            return call_member_function_helper(build_indices<sizeof...(Params)>(), L, *lock, func);
          } else {
            Push(L, "Bad weak_ptr");
            return 1;
          }
        }
      case PointerType::c_ptr:
        {
          return call_member_function_helper(build_indices<sizeof...(Params)>(),
                                             L, *ptr->pointers.c_ptr, func);
        }
      default:
        std::cout << "Calling on unknown pointer type, should never happen" << std::endl;
        assert(false);
      }
    }
  private:
    RetVal (ClassType::*func)(Params...);

    template<int... Indices, typename RetVal_func>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType& obj,
                                           RetVal_func (ClassType::*func)(Params...)){
      RetVal output = (obj.*func)(Read<Params>(L, Indices+1)...);
      int top = lua_gettop(L);
      Push(L, output);
      return lua_gettop(L) - top;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    template<int... Indices>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType& obj,
                                           void (ClassType::*func)(Params...)){
      (obj.*func)(Read<Params>(L, Indices+1)...);
      return 0;
    }
#pragma GCC diagnostic pop
  };
}

#endif /* _LUACALLABLE_MEMBERFUNCTION_H_ */
