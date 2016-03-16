#ifndef _LUAMAKECLASS_H_
#define _LUAMAKECLASS_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaCallable_MemberFunction.hh"
#include "LuaCallable_ObjectConstructor.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"

int garbage_collect_arbitrary_object(lua_State* L);

namespace Lua{
  //! Utility class for exporting C++ classes into Lua.
  /*!Returns a proxy object that can be used to make a class.
    Usage:
      Lua::LuaState L;
      L.MakeClass<ClassName>("ClassName")
        .AddConstructor<ParamType1, ParamType2, ...>("ConstructorName")
        .AddMethod("Method1", &ClassName::Method1)
        .AddMethod("Method2", &ClassName::Method2);

      ClassName, the template parameter, is the class being wrapped.
      "ClassName", the string parameter, is the name of the class, for debug messages.
      <ParamType1, ParamType2, ...> are the parameter types passed to the constructor being registered.
      "ConstructorName" is the name of the Lua function which exposes this contructor.
      "Method1" is the name of the method as it is exposed to Lua.
  */
  template<typename ClassType, typename BaseClass=void>
  class MakeClass {
  public:
    MakeClass(lua_State* L, std::string name) : L(L), name(name), metatable(L), index(L),
                                                const_metatable(L), const_index(L) {
      {
        luaL_newmetatable(L, name.c_str());
        metatable = LuaObject(L);
        metatable["__metatable"] = "Access restricted";

        // Store a deletion function pointer that can be found from the cclosure.
        void* storage = lua_newuserdata(L, sizeof(void(**)(void*)));
        *static_cast<void(**)(void*)>(storage) = VariablePointer<ClassType>::delete_voidp;
        lua_pushcclosure(L, garbage_collect_arbitrary_object, 1);
        LuaObject gc(L);
        metatable["__gc"] = gc;

        NewTable(L);
        index = LuaObject(L);
      }

      {
        const_name = "const." + name;
        luaL_newmetatable(L, const_name.c_str());
        const_metatable = LuaObject(L);
        const_metatable["__metatable"] = "Access restricted";

        // Store a deletion function pointer that can be found from the cclosure.
        void* storage = lua_newuserdata(L, sizeof(void(**)(void*)));
        *static_cast<void(**)(void*)>(storage) = VariablePointer<const ClassType>::delete_voidp;
        lua_pushcclosure(L, garbage_collect_arbitrary_object, 1);
        LuaObject gc(L);
        const_metatable["__gc"] = gc;

        NewTable(L);
        const_index = LuaObject(L);
      }
    }
    ~MakeClass(){
      LuaObject registry(L, LUA_REGISTRYINDEX);

      {
        if(!std::is_same<BaseClass, void>::value){
          NewTable(L);
          LuaObject index_metatable(L);
          {
            LuaObject base_metatable = registry[class_registry_entry<const BaseClass>::get()].Get();
            LuaDelayedPop delayed(L,1);
            index_metatable["__index"].Set(base_metatable["__index"]);
          }
          //index_metatable["__metatable"] = "Access restricted";
          lua_setmetatable(L, const_index.StackPos());
        }

        const_metatable["__index"] = const_index;
        registry[class_registry_entry<const ClassType>::get()] = const_metatable;
      }

      {
        if(!std::is_same<BaseClass, void>::value){
          NewTable(L);
          LuaObject index_metatable(L);
          {
            LuaObject base_metatable = registry[class_registry_entry<BaseClass>::get()].Get();
            LuaDelayedPop delayed(L,1);
            index_metatable["__index"].Set(base_metatable["__index"]);
          }
          //index_metatable["__metatable"] = "Access restricted";
          lua_setmetatable(L, index.StackPos());
        }

        metatable["__index"] = index;
        registry[class_registry_entry<ClassType>::get()] = metatable;
      }
    }

    template<typename RetVal, typename... Params>
    MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...)){
      index[method_name] = new LuaCallable_MemberFunction<ClassType, RetVal(Params...)>(func);
      return *this;
    }

    template<typename RetVal, typename... Params>
    MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...) const){
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

    std::string const_name;
    LuaObject const_metatable;
    LuaObject const_index;
  };
}

#endif /* _LUAMAKECLASS_H_ */
