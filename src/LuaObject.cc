#include "LuaObject.hh"

LuaObject::LuaObject(lua_State* L, int stack_pos) :
  L(L), stack_pos(stack_pos) { }

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

