#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

TEST(LuaStdFunction, CallSimple){
  std::function<double(double)> func;
  {
    Lua::LuaState L;
    L.LoadString("function func(x) "
                 "  return 3*x "
                 "end ");
    func = L.CastGlobal<std::function<double(double)> >("func");
  }
  EXPECT_EQ(func(2), 6);
}

TEST(LuaStdFunction, CallClosure){
  std::function<int()> func;
  {
    Lua::LuaState L;
    L.LoadString("function new_counter() "
                 "  local i = 0 "
                 "  return function ()"
                 "    i = i + 1 "
                 "    return i "
                 "  end "
                 "end ");
    func = L.Call<std::function<int()> >("new_counter");
  }
  EXPECT_EQ(func(), 1);
  EXPECT_EQ(func(), 2);
  EXPECT_EQ(func(), 3);
}
