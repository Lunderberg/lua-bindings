#include <string>

#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

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

TEST(LuaGlobals, ReadWriteBoolean){
  Lua::LuaState L;
  L.LoadString("x = true; y = false");
  EXPECT_EQ(L.CastGlobal<bool>("x"), true);
  EXPECT_EQ(L.CastGlobal<bool>("y"), false);
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
  EXPECT_THROW(y[1], Lua::LuaInvalidStackContents);
}

TEST(LuaGlobals, TransferBetweenTables){
  Lua::LuaState L;

  L.LoadString("x = {a = 5, b = 'hi', [1] = 12345}");
  auto x = L.GetGlobal("x");

  auto y = L.NewTable();
  y["a"] = x["a"].Cast<int>();
  y["b"].Set(x["b"]);

  EXPECT_EQ(y["a"].Cast<int>(), 5);
  EXPECT_EQ(y["b"].Cast<std::string>(), "hi");
}
