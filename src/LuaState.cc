#include "LuaState.hh"

#include <cassert>
#include <iostream>

std::map<lua_State*, std::weak_ptr<LuaState> > LuaState::all_states;

int call_cpp_function(lua_State* L){
  auto state = LuaState::GetCppState(L);
  int function_index = LuaObject(L, lua_upvalueindex(1)).Cast<int>();
  int args_returned = state->cpp_functions.at(function_index)->call(state);
  return args_returned;
}

void LuaState::RegisterCppState(std::shared_ptr<LuaState> state){
  assert(all_states.count(state->L) == 0);
  all_states[state->L] = state;
}

void LuaState::DeregisterCppState(lua_State* c_state){
  assert(all_states.count(c_state) == 1);
  all_states.erase(c_state);
}

std::shared_ptr<LuaState> LuaState::GetCppState(lua_State* c_state){
  assert(all_states.count(c_state) == 1);
  return all_states[c_state].lock();
}

std::shared_ptr<LuaState> LuaState::create(){
  auto output = std::make_shared<LuaState>(HiddenStruct());
  LuaState::RegisterCppState(output);
  return output;
}

LuaState::LuaState(HiddenStruct) : L(nullptr){
  L = luaL_newstate();
}

LuaState::~LuaState() {
  if(L){
    lua_close(L);
  }
  DeregisterCppState(L);
}

LuaObject LuaState::Push(lua_CFunction t){
  LuaPush(L, t);
  return LuaObject(L, -1);
}

LuaObject LuaState::Push(const char* string){
  LuaPush(L, string);
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
  lua_newtable(L);
  return LuaObject(L, -1);
}
