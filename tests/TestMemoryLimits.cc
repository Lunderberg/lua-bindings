#include <gtest/gtest.h>

#include "LuaState.hh"

TEST(LuaMemory, RunOverMemory){
  Lua::LuaState L;
  L.SetMaxMemory(16 * 1024); // 16 kB allowed.
  EXPECT_THROW(
               L.LoadString("large = {} "
                            "i = 1 "
                            "while true do "
                            "  large[i] = i "
                            "  i = i + 1 "
                            "end "),
               LuaOutOfMemoryError);
}
