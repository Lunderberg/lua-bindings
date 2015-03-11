#include "LuaUtils.hh"

void LuaPush(lua_State* L, lua_CFunction t){
  lua_pushcfunction(L, t);
}

void LuaPush(lua_State* L, const char* string){
  lua_pushstring(L, string);
}

void LuaPush(lua_State* L, std::string string){
  LuaPush(L, string.c_str());
}
