#include "lua-bindings/detail/LuaReferenceSet.hh"

#include <set>

#include <lua.hpp>

#include "lua-bindings/detail/LuaObject.hh"
#include "lua-bindings/detail/LuaPush.hh"
#include "lua-bindings/detail/LuaRead.hh"
#include "lua-bindings/detail/LuaRegistryNames.hh"
#include "lua-bindings/detail/LuaTableReference.hh"

namespace{
  int garbage_collect_reference_set(lua_State* L){
    void* storage = lua_touserdata(L, 1);
    auto reference_set = static_cast<std::set<unsigned long>*>(storage);
    reference_set->~set();
    return 0;
  }
}

void Lua::InitializeValidReferenceTable(lua_State* L){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  registry[cpp_reference_counter] = 1;

  // Make a std::set<unsigned long> in lua-controlled memory.
  int memsize = sizeof(std::set<unsigned long>);
  void* storage = lua_newuserdata(L, memsize);
  new (storage) std::set<unsigned long>();

  // Garbage collect the std::set appropriately
  luaL_newmetatable(L, cpp_reference_set_metatable.c_str());
  LuaObject table(L);
  table["__gc"] = garbage_collect_reference_set;
  table["__metatable"] = "Access restricted";
  lua_setmetatable(L, -2);

  // Add to registry
  registry[cpp_valid_reference_set] = LuaObject(L, -1);
}

bool Lua::IsValidReference(lua_State* L, unsigned long reference_id){
  if(reference_id == 0){
    return true;
  } else {
    LuaObject registry(L, LUA_REGISTRYINDEX);
    void* storage = registry[cpp_valid_reference_set].Cast<void*>();
    auto reference_set = static_cast<std::set<unsigned long>*>(storage);
    return reference_set->count(reference_id);
  }
}

Lua::PreserveValidReferences::PreserveValidReferences(lua_State* L)
  : L(L) {
  LuaObject registry(L, LUA_REGISTRYINDEX);
  void* storage = registry[cpp_valid_reference_set].Cast<void*>();
  auto reference_set = static_cast<std::set<unsigned long>*>(storage);
  saved_values = *reference_set;
}

Lua::PreserveValidReferences::~PreserveValidReferences(){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  void* storage = registry[cpp_valid_reference_set].Cast<void*>();
  auto reference_set = static_cast<std::set<unsigned long>*>(storage);
  *reference_set = std::move(saved_values);
}
