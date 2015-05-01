#include <gtest/gtest.h>

#include <iostream>
#include <memory>

#include "lua-bindings/LuaState.hh"

TEST(LuaCoroutines, RunCoroutine){
  Lua::LuaState L;
  L.LoadLibs();

  L.LoadString("function yielding_func(a) "
               "  local b = coroutine.yield(a) "
               "  local c = coroutine.yield(b) "
               "  return c "
               "end");

  auto thread = L.NewCoroutine();
  thread.LoadFunc("yielding_func");

  auto res1 = thread.Resume<int>(5);
  EXPECT_EQ(res1, 5);
  ASSERT_TRUE(thread.IsRunning());
  auto res2 = thread.Resume<std::string>("hello");
  EXPECT_EQ(res2, "hello");
  ASSERT_TRUE(thread.IsRunning());
  auto res3 = thread.Resume<double>(42.5);
  EXPECT_EQ(res3, 42.5);
  EXPECT_FALSE(thread.IsRunning());
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

  auto thread = L.NewCoroutine();
  thread.SetMaxInstructions(100);
  thread.LoadFunc("yielding_func");

  auto res1 = thread.Resume<int>(5);
  EXPECT_EQ(res1, 5);
  ASSERT_TRUE(thread.IsRunning());

  auto res2 = thread.Resume<std::string>("hello");
  EXPECT_EQ(res2, "hello");
  ASSERT_TRUE(thread.IsRunning());

  EXPECT_THROW(thread.Resume<double>(42.5), LuaRuntimeTooLong);
  EXPECT_EQ( L.CastGlobal<int>("i"), 25); //25 loops until the 100 instructions run out.
}

TEST(LuaCoroutines, RepeatedFunction){
  Lua::LuaState L;

  L.LoadLibs();

  L.LoadString("function func() "
               "  return 5 "
               "end");

  auto thread = L.NewCoroutine();
  ASSERT_FALSE(thread.IsRunning());

  thread.LoadFunc("func");
  ASSERT_TRUE(thread.IsRunning());

  auto res1 = thread.Resume<int>(5);
  ASSERT_EQ(res1, 5);
  ASSERT_FALSE(thread.IsRunning());

  thread.LoadFunc("func");
  ASSERT_TRUE(thread.IsRunning());

  auto res2 = thread.Resume<int>(5);
  ASSERT_EQ(res2, 5);
  ASSERT_FALSE(thread.IsRunning());
}

TEST(LuaCoroutines, CoroutineDestructor){
  std::unique_ptr<Lua::LuaCoroutine> thread;

  {
    Lua::LuaState L;

    L.LoadString("function func() "
                 "  return 5 "
                 "end");

    EXPECT_EQ(lua_gettop(L.state()), 0);
    auto thread_obj = L.NewCoroutine();
    thread = std::unique_ptr<Lua::LuaCoroutine>(new Lua::LuaCoroutine(std::move(thread_obj)));
    EXPECT_EQ(lua_gettop(L.state()), 0);
  }

  // Make sure that the thread keeps the lua_State alive.
  thread->LoadFunc("func");
  auto res = thread->Resume<int>();
  EXPECT_EQ(res, 5);
}
