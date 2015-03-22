#include "lua-bindings/detail/LuaTableReference.hh"


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
