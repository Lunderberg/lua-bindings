#include <iostream>
#include <string>
#include <tuple>

#include "LuaState.hh"

int main(){
  Lua::LuaState L;
  L.LoadLibs();

  L.LoadString("function yielding_func(a) "
               "  local b = coroutine.yield(a) "
               "  local c = coroutine.yield(b) "
               "  local i = 0 "
               "  while true do "
               "    i = i+1 "
               "    print(i) "
               "  end "
               "end");

  auto thread = L.NewCoroutine("yielding_func");
  thread.SetMaxInstructions(100);

  auto res1 = thread.Resume<int>(5);
  std::cout << res1 << std::endl;
  auto res2 = thread.Resume<std::string>("hello");
  std::cout << res2 << std::endl;
  auto res3 = thread.Resume<int>(42.5);
  std::cout << res3 << std::endl;

  return 0;
}
