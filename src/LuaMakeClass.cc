#include "lua-bindings/detail/LuaMakeClass.hh"

#include "lua-bindings/detail/LuaObject.hh"

int garbage_collect_arbitrary_object(lua_State* L){
  void* storage = lua_touserdata(L, lua_upvalueindex(1));
  void(*func)(void*) = *static_cast<void(**)(void*)>(storage);

  void* object = lua_touserdata(L, 1);
  func(object);

  return 0;
}
