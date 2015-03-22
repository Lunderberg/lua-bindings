#include "lua-bindings/detail/LuaObject.hh"

#include "lua-bindings/detail/LuaDelayedPop.hh"
#include "lua-bindings/detail/LuaRead.hh"

Lua::LuaObject::LuaObject(lua_State* L, int stack_pos) :
  L(L), stack_pos(lua_absindex(L, stack_pos)) { }

int Lua::LuaObject::LuaType(){
  return lua_type(L, stack_pos);
}

bool Lua::LuaObject::IsNumber()        { return LuaType() == LUA_TNUMBER; }
bool Lua::LuaObject::IsString()        { return LuaType() == LUA_TSTRING; }
bool Lua::LuaObject::IsFunction()      { return LuaType() == LUA_TFUNCTION; }
bool Lua::LuaObject::IsNil()           { return LuaType() == LUA_TNIL; }
bool Lua::LuaObject::IsBoolean()       { return LuaType() == LUA_TBOOLEAN; }
bool Lua::LuaObject::IsTable()         { return LuaType() == LUA_TTABLE; }
bool Lua::LuaObject::IsUserData()      { return LuaType() == LUA_TUSERDATA; }
bool Lua::LuaObject::IsLightUserData() { return LuaType() == LUA_TLIGHTUSERDATA; }
bool Lua::LuaObject::IsThread()        { return LuaType() == LUA_TTHREAD; }

void Lua::LuaObject::MoveToTop(){
  lua_pushvalue(L, stack_pos);
  lua_remove(L, stack_pos);
  stack_pos = lua_absindex(L, -1);
}

void Lua::LuaObject::Pop(){
  lua_remove(L, stack_pos);
}

void Lua::NewTable(lua_State* L){
  lua_newtable(L);
}

int Lua::LuaObject::Length(){
  lua_len(L, stack_pos);
  LuaDelayedPop delayed(L, 1);
  return Lua::Read<int>(L, -1);
}
