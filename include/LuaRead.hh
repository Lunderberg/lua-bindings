#ifndef _LUAREAD_H_
#define _LUAREAD_H_

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaRegistryNames.hh"

namespace Lua{
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, T>::type
  ReadDirect(lua_State* L, int index){
    int success;
    lua_Number output = lua_tonumberx(L, index, &success);
    if(!success){
      throw LuaInvalidStackContents("Lua value could not be converted to number");
    }
    return output;
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
  ReadDirect(lua_State* L, int index){
    if(lua_isstring(L, index)){
      return lua_tostring(L, index);
    } else {
      throw LuaInvalidStackContents("Lua value could not be converted to string");
    }
  }

  template<typename T>
  auto ReadDirectIfPossible(lua_State* L, int index, bool)
    -> decltype(ReadDirect<T>(L, index)) {
    return ReadDirect<T>(L, index);
  }

  template<typename T>
  T ReadDirectIfPossible(lua_State* L, int index, int){
    if(!lua_isuserdata(L, index)){
      throw LuaInvalidStackContents("Value was not userdata");
    }

    void* storage = luaL_testudata(L, index, class_registry_entry<T>().c_str());

    if(!storage){
      throw LuaInvalidStackContents("Value could not be converted to requested type.");
    }

    T* obj = *reinterpret_cast<T**>(storage);
    return *obj;
  }

  template<typename T>
  T Read(lua_State* L, int index){
    return ReadDirectIfPossible<T>(L, index, true);
  }

}

#endif /* _LUAREAD_H_ */
