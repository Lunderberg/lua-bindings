#include "lua-bindings/detail/LuaKeepAlive.hh"

#include <lua.hpp>

#include "lua-bindings/detail/LuaDelayedPop.hh"
#include "lua-bindings/detail/LuaObject.hh"
#include "lua-bindings/detail/LuaPush.hh"
#include "lua-bindings/detail/LuaRegistryNames.hh"
#include "lua-bindings/detail/LuaTableReference.hh"

void Lua::InitializeKeepAliveTable(lua_State* L){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  NewTable(L);
  registry[keepalive_table] = LuaObject(L);
}

int Lua::KeepObjectAlive(lua_State* L, int index){
  index = lua_absindex(L, index);

  LuaObject registry(L, LUA_REGISTRYINDEX);
  auto table = registry[keepalive_table].Get();
  LuaDelayedPop delay(L, 1);



  lua_pushvalue(L, index);
  return luaL_ref(L, table.StackPos());
}

void Lua::AllowToDie(lua_State* L, int reference){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  auto table = registry[keepalive_table].Get();
  LuaDelayedPop delay(L, 1);

  luaL_unref(L, table.StackPos(), reference);
}

void Lua::PushLivingToStack(lua_State* L, int reference){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  auto table = registry[keepalive_table].Get();
  lua_rawgeti(L, -1, reference);

  lua_remove(L, -2); // Can't use LuaDelayedPop because the table is not at the top.
}
