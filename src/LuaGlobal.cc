#include "LuaGlobal.hh"

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaState.hh"

template<typename TypeTest>
bool TestValueType(lua_State* L, const char* name, TypeTest type_test){
  lua_getglobal(L, name);
  bool result = type_test(L, -1);
  lua_pop(L, 1);
  return result;
}

template<typename TypeTest, typename TypeConvert>
auto GetValueType(lua_State* L, const char* name, TypeTest type_test, TypeConvert type_convert)
  -> decltype(type_convert(L,0)) {
  lua_getglobal(L, name);
  bool correct_type = type_test(L, -1);
  if(!correct_type){
    lua_pop(L, 1);
    throw LuaConversionError("Could not convert to requested type");
  }

  auto output = type_convert(L, -1);
  lua_pop(L, 1);
  return output;
}

LuaGlobal::LuaGlobal(std::shared_ptr<LuaState> state, std::string name) :
  lua_state(state), name(name) { }

bool LuaGlobal::IsNumber() {
  return TestValueType(lua_state->state(), name.c_str(), lua_isnumber);
}
double LuaGlobal::ToNumber() {
  return GetValueType(lua_state->state(), name.c_str(), lua_isnumber,
                      [](lua_State* L, int n){return lua_tonumber(L,n);});
}

bool LuaGlobal::IsString() {
  return TestValueType(lua_state->state(), name.c_str(), lua_isstring);
}
std::string LuaGlobal::ToString() {
  // lua_tostring returns a char*, which may not be valid after popping the value from the stack.
  // Therefore, must immediately convert to std::string.
  return GetValueType(lua_state->state(), name.c_str(), lua_isstring,
                      [](lua_State* L, int n){return std::string(lua_tostring(L,n));});
}

bool LuaGlobal::IsFunction() {
  return TestValueType(lua_state->state(), name.c_str(),
                       [](lua_State* L, int n){return lua_isfunction(L,n);});
}

bool LuaGlobal::IsNil() {
  return TestValueType(lua_state->state(), name.c_str(),
                       [](lua_State* L, int n){return lua_isnil(L,n);});
}
