#ifndef _LUAPUSH_H_
#define _LUAPUSH_H_

#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaNil.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaRegistryNames.hh"
#include "TemplateUtils.hh"

namespace Lua{
  class LuaObject;
  class LuaCallable;
  template<typename T>
  class LuaTableReference;

  //! Pushes values onto the LuaStack.
  /*! Each of the PushValueDirect functions will push something immediately,
      without further dispatch.
   */
  void PushValueDirect(lua_State* L, lua_CFunction t);
  void PushValueDirect(lua_State* L, const char* string);
  void PushValueDirect(lua_State* L, std::string string);
  void PushValueDirect(lua_State* L, LuaObject& obj);
  void PushValueDirect(lua_State* L, LuaCallable* callable);
  void PushValueDirect(lua_State* L, bool b);
  void PushValueDirect(lua_State* L, LuaNil);

  template<typename T>
  void PushValueDirect(lua_State*, LuaTableReference<T> ref){
    ref.Get();
  }

  template<typename FirstParam, typename... Params>
  void PushMany(lua_State* L, FirstParam&& first, Params&&... params);

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type
  PushValueDirect(lua_State* L, T t);

  template<typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, std::function<RetVal(Params...)> func);

  template<typename ClassType, typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, RetVal (ClassType::*func)(Params...));

  template<typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, RetVal (*func)(Params...));


  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type
  PushValueDirect(lua_State* L, T t){
    lua_pushnumber(L, t);
  }

  template<typename T>
  class LuaCallable_CppFunction;
  template<typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, std::function<RetVal(Params...)> func){
    LuaCallable* callable = new LuaCallable_CppFunction<RetVal(Params...)>(func);
    PushValueDirect(L, callable);
  }

  template<typename T>
  void PushValueDirect(lua_State* L, std::shared_ptr<T> t){
    int metatable_exists = luaL_getmetatable(L, class_registry_entry<T>().c_str());
    lua_pop(L, 1); // luaL_getmetatable pushes nil if no such table exists
    if(!metatable_exists){
      throw LuaClassNotRegistered("The class requested was not registered with the LuaState");
    }

    int memsize = sizeof(VariablePointer<T>);
    void* userdata = lua_newuserdata(L, memsize );
    std::memset(userdata, 0, memsize);

    auto ptr = static_cast<VariablePointer<T>*>(userdata);
    ptr->type = PointerType::shared_ptr;
    ptr->pointers.shared_ptr = t;

    luaL_setmetatable(L, class_registry_entry<T>().c_str());
  }

  template<typename T>
  void PushValueDirect(lua_State* L, std::weak_ptr<T> t){
    int metatable_exists = luaL_getmetatable(L, class_registry_entry<T>().c_str());
    lua_pop(L, 1); // luaL_getmetatable pushes nil if no such table exists
    if(!metatable_exists){
      throw LuaClassNotRegistered("The class requested was not registered with the LuaState");
    }

    int memsize = sizeof(VariablePointer<T>);
    void* userdata = lua_newuserdata(L, memsize );
    std::memset(userdata, 0, memsize);

    auto ptr = static_cast<VariablePointer<T>*>(userdata);
    ptr->type = PointerType::weak_ptr;
    ptr->pointers.weak_ptr = t;

    luaL_setmetatable(L, class_registry_entry<T>().c_str());
  }

  // LuaCallable (and subclasses) is the only thing that is currently pushed by pointer.
  // Need the std::enable_if to make sure that this doesn't override that behavior.
  template<typename T>
  typename std::enable_if<!std::is_base_of<LuaCallable, T>::value >::type
  PushValueDirect(lua_State* L, T* t){
    int metatable_exists = luaL_getmetatable(L, class_registry_entry<T>().c_str());
    lua_pop(L, 1); // luaL_getmetatable pushes nil if no such table exists
    if(!metatable_exists){
      throw LuaClassNotRegistered("The class requested was not registered with the LuaState");
    }

    int memsize = sizeof(VariablePointer<T>);
    void* userdata = lua_newuserdata(L, memsize );
    std::memset(userdata, 0, memsize);

    auto ptr = static_cast<VariablePointer<T>*>(userdata);
    ptr->type = PointerType::c_ptr;
    ptr->pointers.c_ptr = t;

    luaL_setmetatable(L, class_registry_entry<T>().c_str());
  }

  template<typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, RetVal (*func)(Params...)){
    PushValueDirect(L, std::function<RetVal(Params...)>(func));
  }

  template<int... Indices, typename... Params>
  void PushValueDirect_TupleHelper(lua_State* L, std::tuple<Params...> tuple, indices<Indices...>){
    PushMany(L, std::get<Indices>(tuple)...);
  }

  template<typename... Params>
  void PushValueDirect(lua_State* L, std::tuple<Params...> tuple){
    PushValueDirect_TupleHelper(L, tuple, build_indices<sizeof...(Params)>());
  }

  template<typename T>
  void PushValueDirect(lua_State* L, std::vector<T> vec){
    Lua::NewTable(L);
    Lua::LuaObject table(L);
    for(unsigned int i=0; i<vec.size(); i++){
      table[i+1] = vec[i];
    }
  }

  template<typename T>
  void PushValueDirect(lua_State* L, std::map<std::string, T> map){
    Lua::NewTable(L);
    Lua::LuaObject table(L);
    for(auto iter : map){
      table[iter.first] = iter.second;
    }
  }

  // Need separate specialization for l-value reference, r-value reference.
  // Otherwise, it will try to make a std::shared_ptr<T&>, which is nonsensical.
  template<typename T>
  struct PushDefaultType{
    static void Push(lua_State* L, T&& t){
      auto obj = std::make_shared<T>(t);
      PushValueDirect(L, obj);
    }
  };

  // And here is the case for l-value references.
  template<typename T>
  struct PushDefaultType<T&>{
    static void Push(lua_State* L, T& t){
      auto obj = std::make_shared<T>(t);
      PushValueDirect(L, obj);
    }
  };

  template<typename T>
  auto PushDirectIfPossible(lua_State* L, T t, bool)
    -> decltype(PushValueDirect(L, t)) {
    PushValueDirect(L, t);
  }

  template<typename T>
  void PushDirectIfPossible(lua_State* L, T&& t, int) {
    PushDefaultType<T>::Push(L, std::forward<T>(t));
  }

  //! The mother method, which will push any object onto the lua stack, if possible.
  /*! First, calls PushDirectIfPossible.
    The auto -> decltype() construction makes PushValueDirect be preferred,
      but only if it is a valid construction.
    Otherwise, PushDefaultType<T>::Push is called.
    This has two special cases, one for rvalue references, and one for lvalue references.
   */
  template<typename T>
  void Push(lua_State* L, T&& t){
    PushDirectIfPossible(L, std::forward<T>(t), true);
  }

  //! Does nothing.  Needed for end of recursion of PushMany
  void PushMany(lua_State*);

  //! Pushes each value onto the stack, in order.
  template<typename FirstParam, typename... Params>
  void PushMany(lua_State* L, FirstParam&& first, Params&&... params){
    Push(L, std::forward<FirstParam>(first));
    PushMany(L, std::forward<Params>(params)...);
  }
}

#endif /* _LUAPUSH_H_ */
