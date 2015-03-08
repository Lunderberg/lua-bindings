#include "LuaObject.hh"

LuaObject::LuaObject(lua_State* L, int stack_pos) :
  L(L), stack_pos(stack_pos) { }

namespace{

  template<typename TypeConvert>
    auto GetValueType(lua_State* L, int stack_pos, int lua_type_index, TypeConvert type_convert)
    -> decltype(type_convert(L,0)) {
    bool correct_type = lua_type(L, stack_pos) == lua_type_index;
    if(!correct_type){
      throw LuaConversionError("Could not convert to requested type");
    }

    return type_convert(L, stack_pos);
  }
}

int LuaObject::LuaType(){
  return lua_type(L, stack_pos);
}

bool LuaObject::IsNumber()        { return LuaType() == LUA_TNUMBER; }
bool LuaObject::IsString()        { return LuaType() == LUA_TSTRING; }
bool LuaObject::IsFunction()      { return LuaType() == LUA_TFUNCTION; }
bool LuaObject::IsNil()           { return LuaType() == LUA_TNIL; }
bool LuaObject::IsBoolean()       { return LuaType() == LUA_TBOOLEAN; }
bool LuaObject::IsTable()         { return LuaType() == LUA_TTABLE; }
bool LuaObject::IsUserData()      { return LuaType() == LUA_TUSERDATA; }
bool LuaObject::IsLightUserData() { return LuaType() == LUA_TLIGHTUSERDATA; }
bool LuaObject::IsThread()        { return LuaType() == LUA_TTHREAD; }

double LuaObject::ToNumber() {
  return GetValueType(L, stack_pos, LUA_TNUMBER,
                      [](lua_State* L, int n){return lua_tonumber(L,n);});
}
std::string LuaObject::ToString() {
  // lua_tostring returns a char*, which may not be valid after popping the value from the stack.
  // Therefore, must immediately convert to std::string.
  return GetValueType(L, stack_pos, LUA_TSTRING,
                      [](lua_State* L, int n){return std::string(lua_tostring(L,n));});
}

