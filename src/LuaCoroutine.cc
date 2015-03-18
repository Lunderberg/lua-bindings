#include "LuaCoroutine.hh"

Lua::LuaCoroutine::LuaCoroutine(lua_State* parent, const char* function){
  thread = lua_newthread(parent);
  lua_getglobal(thread, function);
}
