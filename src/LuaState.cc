#include "LuaState.hh"

#include <iostream>
#include <string>

// Trickery needed to let a shared_ptr work with a protected constructor.
// LuaState should never be constructed other than as a shared_ptr,
//   since otherwise, LuaObjs may point to an invalid LuaState.
namespace {
  struct concrete_LuaState : public LuaState { };
};

std::shared_ptr<LuaState> LuaState::create(){
  return std::make_shared<concrete_LuaState>();
}

LuaState::LuaState() : L(nullptr){
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
double LuaState::Pop(){
  double output = lua_tonumber(L, -1);
  lua_pop(L,1);
  return output;
}

template<>
std::string LuaState::Pop(){
  const char* output_char = lua_tostring(L, -1);
  if(!output_char){
    throw LuaInvalidStackContents("Stack did not contain a string");
  }
  std::string output = output_char;
  lua_pop(L,1);
  return output;
}

template<>
void LuaState::Pop() { }

void LuaState::LoadFile(const char* filename) {
  int load_result = luaL_loadfile(L, filename);
  if(load_result){
    auto error_message = Pop<std::string>();
    if(load_result == LUA_ERRFILE){
      throw LuaFileNotFound(filename);
    } else {
      throw LuaFileParseError(error_message);
    }
  }

  int run_result = lua_pcall(L, 0, LUA_MULTRET, 0);
  if(run_result){
    auto error_message = Pop<std::string>();
    throw LuaFileExecuteError(error_message);
  }
}

void LuaState::LoadLibs(){
  luaL_openlibs(L);
}
