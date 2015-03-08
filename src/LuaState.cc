#include "LuaState.hh"

#include <iostream>
#include <string>


std::shared_ptr<LuaState> LuaState::create(){
  return std::make_shared<LuaState>(HiddenStruct());
}

LuaState::LuaState(HiddenStruct) : L(nullptr){
  L = luaL_newstate();
}

LuaState::~LuaState() {
  if(L){
    lua_close(L);
  }
}

template<>
void LuaState::Push(int t){
  lua_pushnumber(L, t);
}

template<>
void LuaState::Push(lua_CFunction t){
  lua_pushcfunction(L, t);
}

template<>
double LuaState::Read(int stack_pos){
  return lua_tonumber(L, stack_pos);
}

template<>
std::string LuaState::Read(int stack_pos){
  const char* output_char = lua_tostring(L, stack_pos);
  if(!output_char){
    throw LuaInvalidStackContents("Stack did not contain a string");
  }
  std::string output = output_char;
  return output;
}

template<>
void LuaState::Read(int /* stack_pos */) { }

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
