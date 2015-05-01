#include "lua-bindings/detail/LuaHoldWeakPtr.hh"

#include "lua-bindings/detail/LuaObject.hh"
#include "lua-bindings/detail/LuaPush.hh"
#include "lua-bindings/detail/LuaRead.hh"
#include "lua-bindings/detail/LuaRegistryNames.hh"
#include "lua-bindings/detail/LuaTableReference.hh"

namespace{
  int garbage_collect_weakptr(lua_State* L){
    void* storage = lua_touserdata(L, 1);
    auto weak_ptr = static_cast<std::weak_ptr<lua_State>* >(storage);
    weak_ptr->~weak_ptr();
    return 0;
  }
}

void Lua::InitializeHeldWeakPtr(std::shared_ptr<lua_State> shared_L){
  LuaObject registry(shared_L.get(), LUA_REGISTRYINDEX);
  void* storage = lua_newuserdata(shared_L.get(), sizeof(std::weak_ptr<lua_State>));
  new (storage) std::weak_ptr<lua_State>(shared_L);

  luaL_newmetatable(shared_L.get(), luastate_weakptr_metatable.c_str());
  LuaObject table(shared_L.get());
  table["__gc"] = garbage_collect_weakptr;
  table["__metatable"] = "Access restricted";
  lua_setmetatable(shared_L.get(), -2);

  registry[luastate_weakptr] = LuaObject(shared_L.get(), -1);
}

std::shared_ptr<lua_State> Lua::ExtractSharedPtr(lua_State* L){
  LuaObject registry(L, LUA_REGISTRYINDEX);
  void* storage = registry[luastate_weakptr].Cast<void*>();
  auto weak_ptr = static_cast<std::weak_ptr<lua_State>* >(storage);
  return weak_ptr->lock();
}
