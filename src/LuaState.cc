#include "LuaState.hh"

#include <cassert>
#include <iostream>

std::shared_ptr<LuaState> LuaState::create(){
  auto output = std::make_shared<LuaState>(HiddenStruct());
  return output;
}

LuaState::LuaState(HiddenStruct) : L(nullptr){
  L = luaL_newstate();
}

LuaState::~LuaState() {
  if(L){
    lua_close(L);
  }
}

LuaObject LuaState::Push(lua_CFunction t){
  LuaObject::Push(L, t);
  return LuaObject(L, -1);
}

LuaObject LuaState::Push(const char* string){
  LuaObject::Push(L, string);
  return LuaObject(L, -1);
}

LuaObject LuaState::Push(std::string string){
  return Push(string.c_str());
}

LuaObject LuaState::Push(LuaObject obj){
  obj.MoveToTop();
  return LuaObject(L, -1);
}

void LuaState::LoadFile(const char* filename) {
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

void LuaState::LoadLibs(){
  luaL_openlibs(L);
}

LuaObject LuaState::NewTable(){
  return LuaObject::NewTable(L);
}
