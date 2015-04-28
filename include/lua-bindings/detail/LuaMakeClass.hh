#ifndef _LUAMAKECLASS_H_
#define _LUAMAKECLASS_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

#include <lua.hpp>

#include "LuaCallable_MemberFunction.hh"
#include "LuaCallable_ObjectConstructor.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"

int garbage_collect_arbitrary_object(lua_State* L);

namespace Lua{

  //! Templated function for deleting C++ objects owned by Lua.
  /*! Unfortunately, "__gc" functions must be directly callable,
      rather than being objects with a "__call" method attached.
    Therefore, I cannot attach a LuaCallable as I would elsewhere.

    This function is templated on the class to be deleted.
    It is then attached as a lua_cclosure with this function,
      and the pointer itself.
   */
  template<typename T>
  void delete_from_void_pointer(void* storage){
    auto ptr = static_cast<VariablePointer<T>*>(storage);

    switch(ptr->type){
    case PointerType::shared_ptr:
      {
        ptr->pointers.shared_ptr.~shared_ptr();
      }
      break;
    case PointerType::weak_ptr:
      {
        ptr->pointers.weak_ptr.~weak_ptr();
      }
      break;
    case PointerType::c_ptr:
      break;
    default:
      // Unknown pointer type, should never reach here.
      assert(false);
    }

  }

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
  template<typename ClassType>
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
        *static_cast<void(**)(void*)>(storage) = delete_from_void_pointer<ClassType>;
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
        *static_cast<void(**)(void*)>(storage) = delete_from_void_pointer<const ClassType>;
        lua_pushcclosure(L, garbage_collect_arbitrary_object, 1);
        LuaObject gc(L);
        const_metatable["__gc"] = gc;

        NewTable(L);
        const_index = LuaObject(L);
      }
    }
    ~MakeClass(){
      {
        const_metatable["__index"] = const_index;

        LuaObject registry(L, LUA_REGISTRYINDEX);
        registry[class_registry_entry<const ClassType>::get()] = const_metatable;
      }

      {
        metatable["__index"] = index;

        LuaObject registry(L, LUA_REGISTRYINDEX);
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
