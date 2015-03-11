#ifndef _LUAUTILS_H_
#define _LUAUTILS_H_

#include <string>
#include <type_traits>

#include <lua.hpp>

template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value>::type
LuaPush(lua_State* L, T t){
  lua_pushnumber(L, t);
}

void LuaPush(lua_State* L, lua_CFunction t);
void LuaPush(lua_State* L, const char* string);
void LuaPush(lua_State* L, std::string string);

#endif /* _LUAUTILS_H_ */
