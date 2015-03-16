#ifndef _LUAMAKECLASS_H_
#define _LUAMAKECLASS_H_

#include <iostream>
#include <memory>
#include <string>

#include <lua.hpp>

#include "LuaCallable_MemberFunction.hh"
#include "LuaCallable_ObjectConstructor.hh"
#include "LuaObject.hh"
#include "LuaPush.hh"

int garbage_collect_arbitrary_object(lua_State* L);

namespace Lua{
  template<typename T>
  void delete_from_void_pointer(void* obj){
    std::shared_ptr<T>* class_obj = reinterpret_cast<std::shared_ptr<T>*>(obj);
    class_obj->~shared_ptr();
  }

  template<typename ClassType>
  class MakeClass {
  public:
    MakeClass(lua_State* L, std::string name) : L(L), name(name), metatable(L), index(L){
      luaL_newmetatable(L, name.c_str());
      metatable = LuaObject(L);
      metatable["__metatable"] = "Access restricted";

      // Store a deletion function pointer that can be found from the cclosure.
      void* storage = lua_newuserdata(L, sizeof(void(**)(void*)));
      *reinterpret_cast<void(**)(void*)>(storage) = delete_from_void_pointer<ClassType>;
      lua_pushcclosure(L, garbage_collect_arbitrary_object, 1);
      LuaObject gc(L);
      metatable["__gc"] = gc;

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
