#include <gtest/gtest.h>

#include "LuaState.hh"

namespace{
  double double_number(double x){
    return 2*x;
  }

  int double_integer(int x){
    return 2*x;
  }

  int external_var;
  void increment_integer(){
    external_var++;
  }

  int sum_integers(int x, int y){
    return x+y;
  }

  std::tuple<int, int> multiple_returns(){
    return std::make_tuple(5, 6);
  }
}

TEST(CppFunctions, CallFunctions){
  Lua::LuaState L;
  L.SetGlobal("double_number", double_number);
  auto res1 = L.LoadString<double>("return double_number(5)");
  EXPECT_EQ(res1, 10.0);

  L.SetGlobal("double_integer", double_integer);
  auto res2 = L.LoadString<int>("return double_integer(5)");
  EXPECT_EQ(res2, 10);

  L.SetGlobal("increment_integer", increment_integer);
  EXPECT_EQ(external_var, 0);
  L.LoadString("increment_integer()");
  EXPECT_EQ(external_var, 1);

  L.SetGlobal("sum_integers", sum_integers);
  auto res3 = L.LoadString<int>("return sum_integers(5,10)");
  EXPECT_EQ(res3, 15);

  L.SetGlobal("multiple_returns", multiple_returns);
  auto res4 = L.LoadString<int>("x,y = multiple_returns(); return x+y");
  EXPECT_EQ(res4, 11);

  EXPECT_THROW(L.LoadString("non_existing_func()"), LuaExecuteError);
}
