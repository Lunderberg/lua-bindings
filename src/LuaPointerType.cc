#include "lua-bindings/detail/LuaPointerType.hh"

int Lua::garbage_collect_upcaster(lua_State* L){
  void* storage = lua_touserdata(L, 1);
  Lua::Upcaster* upcaster = *static_cast<Lua::Upcaster**>(storage);
  delete upcaster;
  return 0;
}
