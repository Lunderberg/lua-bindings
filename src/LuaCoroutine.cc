#include "LuaCoroutine.hh"

Lua::LuaCoroutine::LuaCoroutine(lua_State* parent, const char* function)
  : finished(false) {
  thread = lua_newthread(parent);
  lua_getglobal(thread, function);
}
