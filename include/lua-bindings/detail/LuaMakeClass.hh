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
    MakeClass(lua_State* L, std::string name)
      : L(L), name(name),
        nonconst_index(L,name),
        const_index(L,"const." + name) { }
    ~MakeClass(){ }

    template<typename RetVal, typename... Params>
    MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...)){
      nonconst_index.AddMethod(method_name, func);
      return *this;
    }

    template<typename RetVal, typename... Params>
    MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...) const){
      const_index.AddMethod(method_name, func);
      nonconst_index.AddMethod(method_name, func);
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

    template<typename IClass, typename IBase>
    class ClassIndexGen {
    public:
      ClassIndexGen(lua_State* L, std::string name)
        : L(L), name(name), metatable(L), index(L) {
        luaL_newmetatable(L, name.c_str());
        metatable = LuaObject(L);
        metatable["__metatable"] = "Access restricted";
        metatable["__gc"] = VariablePointer<IClass>::garbage_collect;

        NewTable(L);
        index = LuaObject(L);
      }

      ~ClassIndexGen() {
        LuaObject registry(L, LUA_REGISTRYINDEX);

        if(!std::is_same<BaseClass, void>::value){
          NewTable(L);
          LuaObject index_metatable(L);
          {
            LuaObject base_metatable = registry[class_registry_entry<IBase>::get()].Get();
            LuaDelayedPop delayed(L,1);
            index_metatable["__index"].Set(base_metatable["__index"]);
          }
          index_metatable["__metatable"] = "Access restricted";
          lua_setmetatable(L, index.StackPos());
        }

        metatable["__index"] = index;
        registry[class_registry_entry<IClass>::get()] = metatable;
      }

      // Disable adding a non-const method to a const index.
      template<typename RetVal, typename... Params,
               typename T = IClass,
               typename U = typename std::enable_if<!std::is_const<T>::value>::type >
      void AddMethod(std::string method_name, RetVal (IClass::*func)(Params...)){
        index[method_name] = new LuaCallable_MemberFunction<IClass, RetVal(Params...)>(func);
      }

      // Can always add a const method.
      template<typename RetVal, typename... Params>
      void AddMethod(std::string method_name, RetVal (IClass::*func)(Params...) const){
        index[method_name] = new LuaCallable_MemberFunction<IClass, RetVal(Params...)>(func);
      }

    private:
      lua_State* L;
      std::string name;
      LuaObject metatable;
      LuaObject index;
    };


    lua_State* L;
    std::string name;

    ClassIndexGen<ClassType, BaseClass> nonconst_index;
    ClassIndexGen<const ClassType, const BaseClass> const_index;
  };
}

#endif /* _LUAMAKECLASS_H_ */
