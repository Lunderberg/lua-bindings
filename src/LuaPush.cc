#include "lua-bindings/detail/LuaPush.hh"

#include "lua-bindings/detail/LuaCallable.hh"
#include "lua-bindings/detail/LuaObject.hh"
#include "lua-bindings/detail/LuaRead.hh"
#include "lua-bindings/detail/LuaRegistryNames.hh"
#include "lua-bindings/detail/LuaTableReference.hh"

void Lua::PushValueDirect(lua_State* L, lua_CFunction t){
  lua_pushcfunction(L, t);
}

void Lua::PushValueDirect(lua_State* L, const char* string){
  lua_pushstring(L, string);
}

void Lua::PushValueDirect(lua_State* L, bool b){
  lua_pushboolean(L, b);
}

void Lua::PushValueDirect(lua_State* L, std::string string){
  PushValueDirect(L, string.c_str());
}

void Lua::PushValueDirect(lua_State*, LuaObject& obj){
  obj.MoveToTop();
}

void Lua::PushValueDirect(lua_State* L, LuaNil){
  lua_pushnil(L);
}

void Lua::PushValueDirect(lua_State* L, Lua::LuaCallable* callable){
  // Define a new userdata, storing the LuaCallable in it.
  void* userdata = lua_newuserdata(L, sizeof(callable));
  *static_cast<Lua::LuaCallable**>(userdata) = callable;

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

void Lua::PushMany(lua_State*){ }

void Lua::PushCodeFile(lua_State* L, const char* filename){
  int load_result = luaL_loadfilex(L, filename, "t");
  if(load_result){
    auto error_message = Lua::Read<std::string>(L, -1);
    if(load_result == LUA_ERRFILE){
      throw LuaFileNotFound(filename);
    } else {
      throw LuaFileParseError(error_message);
    }
  }
}

void Lua::PushCodeString(lua_State* L, const std::string& lua_code){
  int load_result = luaL_loadstring(L, lua_code.c_str());
  if(load_result){
    auto error_message = Lua::Read<std::string>(L, -1);
    throw LuaFileParseError(error_message);
  }
}
