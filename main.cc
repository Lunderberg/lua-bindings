#include <iostream>
#include <string>
#include <tuple>

#include "lua-bindings/LuaState.hh"

int main(){
  Lua::LuaState L;

  L.LoadSafeLibs();
  //L.LoadLibs();

  L.LoadString("a = {'one', 'two', 'three'} "
               "for i,v in ipairs(a) do "
               "  print(i,v) "
               "end ");

  L.LoadString("print(os.time())");

  return 0;
}
