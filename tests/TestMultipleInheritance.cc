#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

namespace {
  struct BaseA {
    int x;
  };

  struct BaseB {
    int y;
  };

  struct Derived : BaseA, BaseB {
    int z;
  };
}

TEST(LuaSubclasses, ConvertToBaseA) {
  Lua::LuaState L;
  L.MakeClass<BaseA>("BaseA");
  L.MakeClass<Derived, BaseA>("Derived");

  Derived derived;
  L.SetGlobal("derived",&derived);
  BaseA* base_lua = L.CastGlobal<BaseA*>("derived");
  const BaseA* base_lua_const = L.CastGlobal<const BaseA*>("derived");

  BaseA* base_cpp = static_cast<BaseA*>(&derived);

  EXPECT_EQ(base_cpp, base_lua);
  EXPECT_EQ(base_cpp, base_lua_const);
}

TEST(LuaSubclasses, ConvertToBaseB) {
  Lua::LuaState L;
  L.MakeClass<BaseB>("BaseB");
  L.MakeClass<Derived, BaseB>("Derived");

  Derived derived;
  L.SetGlobal("derived",&derived);
  BaseB* base_lua = L.CastGlobal<BaseB*>("derived");
  const BaseB* base_lua_const = L.CastGlobal<const BaseB*>("derived");

  BaseB* base_cpp = static_cast<BaseB*>(&derived);

  EXPECT_EQ(base_cpp, base_lua);
  EXPECT_EQ(base_cpp, base_lua_const);
}
