#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include "LuaState.hh"

TEST(LuaFunctions, VoidFunction){
  Lua::LuaState L;
  L.LoadString("x = 0; function increment() x = x + 1 end");
  EXPECT_EQ(L.CastGlobal<int>("x"), 0);
  L.Call("increment");
  EXPECT_EQ(L.CastGlobal<int>("x"), 1);
  L.Call("increment");
  EXPECT_EQ(L.CastGlobal<int>("x"), 2);

  L.LoadString("function return_seven() return 7 end");
  EXPECT_EQ(L.Call<int>("return_seven"), 7);
}

TEST(LuaFunctions, SingleParameter){
  Lua::LuaState L;
  L.LoadString("function triple(x) return 3*x end");
  EXPECT_EQ(L.Call<double>("triple",1.5), 4.5);
  L.LoadLibs();
  L.LoadString("function string_repeat(x) return string.rep('(-)', x) end");
  EXPECT_EQ(L.Call<std::string>("string_repeat", 3), "(-)(-)(-)");
}

TEST(LuaFunctions, MultiReturn){
  Lua::LuaState L;
  L.LoadString("function multireturn() return 3,4 end");
  EXPECT_EQ(L.Call<int>("multireturn"), 3);
  auto res = L.Call<std::tuple<int,int> >("multireturn");
  EXPECT_EQ(res, std::make_tuple(3,4));
}

TEST(LuaFunctions, ReturnTable){
  Lua::LuaState L;
  L.LoadString("function vector_int_return() "
               "  return {1,2,3,4} "
               "end");
  auto vector_res = L.Call<std::vector<int> >("vector_int_return");
  EXPECT_EQ(vector_res.size(), 4);
  EXPECT_EQ(vector_res[0], 1);
  EXPECT_EQ(vector_res[1], 2);
  EXPECT_EQ(vector_res[2], 3);
  EXPECT_EQ(vector_res[3], 4);

  L.LoadString("function map_int_return() "
               "   return {a=1, b=42} "
               "end");
  auto map_res = L.Call<std::map<std::string, int> >("map_int_return");
  EXPECT_EQ(map_res.size(), 2);
  EXPECT_EQ(map_res["a"], 1);
  EXPECT_EQ(map_res["b"], 42);
}
