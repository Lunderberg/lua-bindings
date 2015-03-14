#ifndef _LUAPUSHREAD_H_
#define _LUAPUSHREAD_H_

#include <functional>
#include <string>
#include <type_traits>

#include <lua.hpp>

namespace Lua{
  class LuaObject;
  class LuaCallable;

  void Push(lua_State* L, lua_CFunction t);
  void Push(lua_State* L, const char* string);
  void Push(lua_State* L, std::string string);
  void Push(lua_State* L, LuaObject& obj);
  void Push(lua_State* L, LuaCallable* callable);

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type
  Push(lua_State* L, T t);

  template<typename RetVal, typename... Params>
  void Push(lua_State* L, std::function<RetVal(Params...)> func);

  template<typename ClassType, typename RetVal, typename... Params>
  void Push(lua_State* L, RetVal (ClassType::*func)(Params...));

  template<typename RetVal, typename... Params>
  void Push(lua_State* L, RetVal (*func)(Params...));


  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value>::type
  Push(lua_State* L, T t){
    lua_pushnumber(L, t);
  }

  template<typename T>
  class LuaCallable_CppFunction;
  template<typename RetVal, typename... Params>
  void Push(lua_State* L, std::function<RetVal(Params...)> func){
    LuaCallable* callable = new LuaCallable_CppFunction<RetVal(Params...)>(func);
    Push(L, callable);
  }

  template<typename RetVal, typename... Params>
  void Push(lua_State* L, RetVal (*func)(Params...)){
    Push(L, std::function<RetVal(Params...)>(func));
  }
}

#endif /* _LUAPUSHREAD_H_ */
