#include <gtest/gtest.h>

#include "LuaState.hh"

TEST(LuaCoroutines, RunCoroutine){
  Lua::LuaState L;
  L.LoadLibs();

  L.LoadString("function yielding_func(a) "
               "  local b = coroutine.yield(a) "
               "  local c = coroutine.yield(b) "
               "  return c "
               "end");

  auto thread = L.NewCoroutine("yielding_func");
  auto res1 = thread.Resume<int>(5);
  EXPECT_EQ(res1, 5);
  ASSERT_FALSE(thread.IsFinished());
  auto res2 = thread.Resume<std::string>("hello");
  EXPECT_EQ(res2, "hello");
  ASSERT_FALSE(thread.IsFinished());
  auto res3 = thread.Resume<double>(42.5);
  EXPECT_EQ(res3, 42.5);
  EXPECT_TRUE(thread.IsFinished());
}

TEST(LuaCoroutines, MaxInstructions){
  Lua::LuaState L;

  L.LoadLibs();

  L.LoadString("i = 0 "
               "function yielding_func(a) "
               "  local b = coroutine.yield(a) "
               "  local c = coroutine.yield(b) "
               "  while true do "
               "    i = i + 1 "
               "  end "
               "end");

  auto thread = L.NewCoroutine("yielding_func");
  thread.SetMaxInstructions(100);

  auto res1 = thread.Resume<int>(5);
  EXPECT_EQ(res1, 5);
  ASSERT_FALSE(thread.IsFinished());

  auto res2 = thread.Resume<std::string>("hello");
  EXPECT_EQ(res2, "hello");
  ASSERT_FALSE(thread.IsFinished());

  EXPECT_THROW(thread.Resume<double>(42.5), LuaRuntimeTooLong);
  EXPECT_EQ( L.CastGlobal<int>("i"), 25); //25 loops until the 100 instructions run out.
}
