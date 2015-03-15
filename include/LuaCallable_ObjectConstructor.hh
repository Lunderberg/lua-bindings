#ifndef _LUACALLABLE_OBJECTCONSTRUCTOR_H_
#define _LUACALLABLE_OBJECTCONSTRUCTOR_H_

#include <string>

#include <lua.hpp>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaRegistryNames.hh"
#include "TemplateUtils.hh"

namespace Lua{
  template<typename T>
  class LuaCallable_ObjectConstructor;
  template<typename ClassType, typename... Params>
  class LuaCallable_ObjectConstructor<ClassType(Params...)> : public LuaCallable {
  public:
    LuaCallable_ObjectConstructor() { }
    virtual int call(lua_State* L){
      return call_constructor_helper(build_indices<sizeof...(Params)>(), L);
    }
  private:
    template<int... Indices>
    static int call_constructor_helper(indices<Indices...>, lua_State* L){
      if(lua_gettop(L) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      ClassType* obj = new ClassType(LuaObject(L, Indices+1).Cast<Params>()...);
      void* userdata = lua_newuserdata(L, sizeof(obj));
      *reinterpret_cast<ClassType**>(userdata) = obj;

      luaL_newmetatable(L, class_registry_entry<ClassType>().c_str());
      lua_setmetatable(L, -2);

      return 1;
    }
  };
}

#endif /* _LUACALLABLE_OBJECTCONSTRUCTOR_H_ */
