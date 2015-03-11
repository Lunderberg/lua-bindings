#include "LuaObject.hh"

#include "LuaState.hh"

LuaObject::LuaObject(lua_State* L, int stack_pos) :
  L(L) {
  this->stack_pos = lua_absindex(L, stack_pos);
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

LuaObject::LuaTableReference<std::string> LuaObject::operator[](std::string key){
  if(!IsTable()){
    throw LuaInvalidStackContents("Object is not a table.");
  }
  return LuaTableReference<std::string>(L, stack_pos, key);
}
LuaObject::LuaTableReference<int> LuaObject::operator[](int key){
  if(!IsTable()){
    throw LuaInvalidStackContents("Object is not a table.");
  }
  return LuaTableReference<int>(L, stack_pos, key);
}

void LuaObject::MoveToTop(){
  lua_pushvalue(L, stack_pos);
  lua_remove(L, stack_pos);
  stack_pos = lua_absindex(L, -1);
}

void LuaObject::Pop(){
  lua_remove(L, stack_pos);
}
