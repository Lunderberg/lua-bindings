#include "lua-bindings/detail/LuaFunctionWrapper.hh"

Lua::FunctionWrapper::FunctionWrapper(std::shared_ptr<lua_State> shared_L, int index)
  : shared_L(shared_L) {
  reference = KeepObjectAlive(shared_L.get(), index);
}

Lua::FunctionWrapper::FunctionWrapper(FunctionWrapper&& other){
  shared_L = other.shared_L;
  reference = other.reference;
  other.reference = -1;
}

Lua::FunctionWrapper::~FunctionWrapper(){
  if(reference != -1){
    AllowToDie(shared_L.get(), reference);
  }
}
