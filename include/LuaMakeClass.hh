#ifndef _LUAMAKECLASS_H_
#define _LUAMAKECLASS_H_

#include <string>

#include <lua.hpp>

#include "LuaCallable_MemberFunction.hh"
#include "LuaCallable_ObjectConstructor.hh"
#include "LuaCallable_ObjectDeleter.hh"
#include "LuaObject.hh"

namespace Lua{
  template<typename ClassType>
  class MakeClass {
  public:
    MakeClass(lua_State* L, std::string name) : L(L), name(name), metatable(L), index(L){
      luaL_newmetatable(L, name.c_str());
      metatable = LuaObject(L);
      metatable["__gc"] = new LuaCallable_ObjectDeleter<ClassType>();
      metatable["__metatable"] = "Access restricted";

      NewTable(L);
      index = LuaObject(L);
    }
    ~MakeClass(){
      metatable["__index"] = index;

      LuaObject registry(L, LUA_REGISTRYINDEX);
      registry[class_registry_entry<ClassType>()] = metatable;
    }

    template<typename RetVal, typename... Params>
    MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...)){
      index[method_name] = new LuaCallable_MemberFunction<ClassType, RetVal(Params...)>(func);
      return *this;
    }

    template<typename... Params>
    MakeClass& AddConstructor(std::string constructor_name = ""){
      if(constructor_name.size() == 0){
        constructor_name = name;
      }
      Push(L, new LuaCallable_ObjectConstructor<ClassType(Params...)>());
      lua_setglobal(L, constructor_name.c_str());

      return *this;
    }

  private:
    lua_State* L;
    std::string name;
    LuaObject metatable;
    LuaObject index;
  };
}

#endif /* _LUAMAKECLASS_H_ */
