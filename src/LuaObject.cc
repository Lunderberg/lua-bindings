#include "LuaObject.hh"

#include "LuaState.hh"

Lua::LuaObject::LuaObject(lua_State* L, int stack_pos) :
  L(L) {
  this->stack_pos = lua_absindex(L, stack_pos);
}

int Lua::LuaObject::LuaType(){
  return lua_type(L, stack_pos);
}

bool Lua::LuaObject::IsNumber()        { return LuaType() == LUA_TNUMBER; }
bool Lua::LuaObject::IsString()        { return LuaType() == LUA_TSTRING; }
bool Lua::LuaObject::IsFunction()      { return LuaType() == LUA_TFUNCTION; }
bool Lua::LuaObject::IsNil()           { return LuaType() == LUA_TNIL; }
bool Lua::LuaObject::IsBoolean()       { return LuaType() == LUA_TBOOLEAN; }
bool Lua::LuaObject::IsTable()         { return LuaType() == LUA_TTABLE; }
bool Lua::LuaObject::IsUserData()      { return LuaType() == LUA_TUSERDATA; }
bool Lua::LuaObject::IsLightUserData() { return LuaType() == LUA_TLIGHTUSERDATA; }
bool Lua::LuaObject::IsThread()        { return LuaType() == LUA_TTHREAD; }

Lua::LuaTableReference<std::string> Lua::LuaObject::operator[](std::string key){
  if(!IsTable()){
    throw LuaInvalidStackContents("Object is not a table.");
  }
  return LuaTableReference<std::string>(L, stack_pos, key);
}
Lua::LuaTableReference<int> Lua::LuaObject::operator[](int key){
  if(!IsTable()){
    throw LuaInvalidStackContents("Object is not a table.");
  }
  return LuaTableReference<int>(L, stack_pos, key);
}

void Lua::LuaObject::MoveToTop(){
  lua_pushvalue(L, stack_pos);
  lua_remove(L, stack_pos);
  stack_pos = lua_absindex(L, -1);
}

void Lua::LuaObject::Pop(){
  lua_remove(L, stack_pos);
}

Lua::LuaObject Lua::Push(lua_State* L, lua_CFunction t){
  lua_pushcfunction(L, t);
  return Lua::LuaObject(L);
}

Lua::LuaObject Lua::Push(lua_State* L, const char* string){
  lua_pushstring(L, string);
  return Lua::LuaObject(L);
}

Lua::LuaObject Lua::Push(lua_State* L, std::string string){
  return Push(L, string.c_str());
}

Lua::LuaObject Lua::Push(lua_State* L, LuaObject obj){
  obj.MoveToTop();
  return Lua::LuaObject(L);
}

Lua::LuaObject Lua::NewTable(lua_State* L){
  lua_newtable(L);
  return Lua::LuaObject(L);
}

int call_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *reinterpret_cast<Lua::LuaCallable**>(storage);
  lua_remove(L, 1);
  int args_returned = callable->call(L);
  return args_returned;
}

int garbage_collect_cpp_function(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::LuaCallable* callable = *reinterpret_cast<Lua::LuaCallable**>(storage);
  delete callable;
  return 0;
}
