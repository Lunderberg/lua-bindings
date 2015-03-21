#include <gtest/gtest.h>

#include "LuaState.hh"

TEST(LuaSandbox, MemoryLimit){
  Lua::LuaState L;
  L.SetMaxMemory(16 * 1024); // 16 kB allowed.
  EXPECT_THROW(L.LoadString("large = {} "
                            "i = 1 "
                            "while true do "
                            "  large[i] = i "
                            "  i = i + 1 "
                            "end "),
               LuaOutOfMemoryError);
}

TEST(LuaSandbox, RestrictedFunctions){
  Lua::LuaState L;
  L.LoadSafeLibs();

  // ipairs is available as global function
  L.LoadString("a = {'one', 'two', 'three'} "
               "for i,v in ipairs(a) do end ");
  // os.time is available inside package
  L.LoadString("os.time()");

  EXPECT_THROW(L.LoadString("os.execute('echo hi')"),
               LuaExecuteError);
}
