#include "lua-bindings/detail/LuaCallable.hh"

#include <stdexcept>

int Lua::LuaCallable::call_noexcept(lua_State* L){
  try{
    return call(L);
  } catch (std::exception& e) {
    return luaL_error(L, "C++ exception: %s", e.what());
  } catch (...) {
    return luaL_error(L, "C++ exception thrown during execution");
  }
}

int Lua::call_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *static_cast<Lua::LuaCallable**>(storage);
  lua_remove(L, 1);
  int args_returned = callable->call_noexcept(L);
  return args_returned;
}

int Lua::garbage_collect_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *static_cast<Lua::LuaCallable**>(storage);
  delete callable;
  return 0;
}

