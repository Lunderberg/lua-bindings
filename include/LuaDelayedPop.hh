#ifndef _LUADELAYEDPOP_H_
#define _LUADELAYEDPOP_H_

#include <lua.hpp>

class LuaDelayedPop{
public:
  LuaDelayedPop(lua_State* L, int num_to_pop) :
    L(L), num_to_pop(num_to_pop) { }

  ~LuaDelayedPop(){
    lua_pop(L, num_to_pop);
  }
private:
  lua_State* L;
  int num_to_pop;
};

#endif /* _LUADELAYEDPOP_H_ */
