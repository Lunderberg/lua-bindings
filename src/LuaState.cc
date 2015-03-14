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

void Lua::LuaState::LoadFile(const char* filename) {
  int load_result = luaL_loadfile(L, filename);
  if(load_result){
    auto error_message = Read<std::string>();
    if(load_result == LUA_ERRFILE){
      throw LuaFileNotFound(filename);
    } else {
      throw LuaFileParseError(error_message);
    }
  }

  int run_result = lua_pcall(L, 0, LUA_MULTRET, 0);
  if(run_result){
    auto error_message = Read<std::string>();
    throw LuaFileExecuteError(error_message);
  }
}

void Lua::LuaState::LoadLibs(){
  luaL_openlibs(L);
}

Lua::LuaObject Lua::LuaState::NewTable(){
  Lua::NewTable(L);
  return Lua::LuaObject(L);
}
