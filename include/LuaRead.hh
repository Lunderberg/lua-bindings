#ifndef _LUAREAD_H_
#define _LUAREAD_H_

#include <lua.hpp>

#include "LuaExceptions.hh"

namespace Lua{
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, T>::type
  Read(lua_State* L, int index){
    int success;
    lua_Number output = lua_tonumberx(L, index, &success);
    if(!success){
      throw LuaInvalidStackContents("Lua value could not be converted to number");
    }
    return output;
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
  Read(lua_State* L, int index){
    if(lua_isstring(L, index)){
      return lua_tostring(L, index);
    } else {
      throw LuaInvalidStackContents("Lua value could not be converted to string");
    }
  }


}

#endif /* _LUAREAD_H_ */
