#include "LuaState.hh"

#include <cassert>
#include <iostream>

Lua::LuaState::LuaState() : L(nullptr){
  L = luaL_newstate();
}

Lua::LuaState::~LuaState() {
  if(L){
    lua_close(L);
  }
}

void Lua::LuaState::LoadLibs(){
  luaL_openlibs(L);
}

Lua::LuaObject Lua::LuaState::NewTable(){
  Lua::NewTable(L);
  return Lua::LuaObject(L);
}
