#include <string>

#include <gtest/gtest.h>

#include "LuaState.hh"

TEST(LuaGlobals, ReadWriteIntegers){
  Lua::LuaState L;
  L.LoadString("x = 5; y = -41");
  EXPECT_EQ(L.CastGlobal<int>("x"), 5);
  EXPECT_EQ(L.CastGlobal<int>("y"), -41);

  L.SetGlobal("x", 17);
  EXPECT_EQ(L.CastGlobal<int>("x"), 17);
}

TEST(LuaGlobals, ReadWriteStrings){
  Lua::LuaState L;
  L.LoadString("x = 'hi'; y = 'there'");
  EXPECT_EQ(L.CastGlobal<std::string>("x"), "hi");
  EXPECT_EQ(L.CastGlobal<std::string>("y"), "there");

  L.SetGlobal("x", "foo");
  EXPECT_EQ(L.CastGlobal<std::string>("x"), "foo");
}

TEST(LuaGlobals, ReadTable){
  Lua::LuaState L;
  L.LoadString("x = {}; x.a = 5; x.b = 'hi'; x[1] = 12345");
  auto table = L.GetGlobal("x");
  EXPECT_EQ(table["a"].Cast<int>(), 5);
  EXPECT_EQ(table["b"].Cast<std::string>(), "hi");
  EXPECT_EQ(table[1].Cast<int>(), 12345);
  table.Pop();

  L.LoadString("y=5");
  auto y = L.GetGlobal("y");
  EXPECT_THROW(y[1], LuaInvalidStackContents);
}
