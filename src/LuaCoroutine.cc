#include "lua-bindings/detail/LuaCoroutine.hh"

bool Lua::LuaCoroutine::ended_by_timeout = false;

Lua::LuaCoroutine::LuaCoroutine(lua_State* parent, const char* function)
  : finished(false), max_instructions(-1) {
  thread = lua_newthread(parent);
  lua_getglobal(thread, function);
}

void Lua::yielding_hook(lua_State* L, lua_Debug*){
  Lua::LuaCoroutine::ended_by_timeout = true;
  lua_yield(L, 0);
}
