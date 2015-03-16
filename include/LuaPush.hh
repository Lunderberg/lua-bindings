#ifndef _LUAPUSH_H_
#define _LUAPUSH_H_

#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaRegistryNames.hh"

namespace Lua{
  class LuaObject;
  class LuaCallable;

  void PushValueDirect(lua_State* L, lua_CFunction t);
  void PushValueDirect(lua_State* L, const char* string);
  void PushValueDirect(lua_State* L, std::string string);
  void PushValueDirect(lua_State* L, LuaObject& obj);
  void PushValueDirect(lua_State* L, LuaCallable* callable);

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

  template<typename RetVal, typename... Params>
  void PushValueDirect(lua_State* L, RetVal (*func)(Params...)){
    PushValueDirect(L, std::function<RetVal(Params...)>(func));
  }

  template<typename T>
  struct PushDefaultType{
    static void Push(lua_State* L, const T& t){
      auto obj = std::make_shared<T>(t);
      PushDefaultType<std::shared_ptr<T> >::Push(L, obj);
    }
  };

  template<typename T>
  struct PushDefaultType<std::shared_ptr<T> > {
    static void Push(lua_State* L, std::shared_ptr<T> t){
      int metatable_exists = luaL_getmetatable(L, class_registry_entry<T>().c_str());
      lua_pop(L, 1); // luaL_getmetatable pushes nil if no such table exists
      if(!metatable_exists){
        throw LuaClassNotRegistered("The class requested was not registered with the LuaState");
      }

      void* userdata = lua_newuserdata(L, sizeof(std::shared_ptr<T>) );
      std::memset(userdata, 0, sizeof(std::shared_ptr<T>));
      *reinterpret_cast<std::shared_ptr<T>*>(userdata) = t;

      luaL_setmetatable(L, class_registry_entry<T>().c_str());
    }
  };


  template<typename T>
  auto PushDirectIfPossible(lua_State* L, T t, bool)
    -> decltype(PushValueDirect(L, t)) {
    PushValueDirect(L, t);
  }

  template<typename T>
  void PushDirectIfPossible(lua_State* L, T t, int) {
    PushDefaultType<T>::Push(L, std::forward<T>(t));
  }

  template<typename T>
  void Push(lua_State* L, T t){
    PushDirectIfPossible(L, std::forward<T>(t), true);
  }
}

#endif /* _LUAPUSH_H_ */
