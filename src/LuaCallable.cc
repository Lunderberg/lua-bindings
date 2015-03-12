#include "LuaCallable.hh"

int call_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  LuaCallable* callable = *reinterpret_cast<LuaCallable**>(storage);
  lua_remove(L, 1);
  int args_returned = callable->call(L);
  return args_returned;
}

int garbage_collect_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  LuaCallable* callable = *reinterpret_cast<LuaCallable**>(storage);
  delete callable;
  return 0;
}