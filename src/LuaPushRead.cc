#include "LuaPushRead.hh"

#include "LuaCallable.hh"
#include "LuaObject.hh"
#include "LuaRegistryNames.hh"
#include "LuaTableReference.hh"

void Lua::PushValueDirect(lua_State* L, lua_CFunction t){
  lua_pushcfunction(L, t);
}

void Lua::PushValueDirect(lua_State* L, const char* string){
  lua_pushstring(L, string);
}

void Lua::PushValueDirect(lua_State* L, std::string string){
  PushValueDirect(L, string.c_str());
}

void Lua::PushValueDirect(lua_State*, LuaObject& obj){
  obj.MoveToTop();
}

void Lua::PushValueDirect(lua_State* L, Lua::LuaCallable* callable){
  // Define a new userdata, storing the LuaCallable in it.
  void* userdata = lua_newuserdata(L, sizeof(callable));
  *reinterpret_cast<Lua::LuaCallable**>(userdata) = callable;

  // Create the metatable
  int metatable_uninitialized = luaL_newmetatable(L, cpp_function_registry_entry.c_str());
  if(metatable_uninitialized){
    Lua::LuaObject table(L);
    table["__call"] = call_cpp_function;
    table["__gc"] = garbage_collect_cpp_function;
    table["__metatable"] = "Access restricted";
  }
  lua_setmetatable(L, -2);
}
