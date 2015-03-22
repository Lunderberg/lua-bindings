#include "lua-bindings/detail/LuaCallable.hh"

int Lua::call_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *static_cast<Lua::LuaCallable**>(storage);
  lua_remove(L, 1);
  int args_returned = callable->call(L);
  return args_returned;
}

int Lua::garbage_collect_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *static_cast<Lua::LuaCallable**>(storage);
  delete callable;
  return 0;
}

