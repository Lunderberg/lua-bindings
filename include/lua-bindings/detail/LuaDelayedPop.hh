#ifndef _LUADELAYEDPOP_H_
#define _LUADELAYEDPOP_H_

#include <lua.hpp>

class LuaDelayedPop{
public:
  LuaDelayedPop(lua_State* L, int num_to_pop) :
    L(L), num_to_pop(num_to_pop) { }

  ~LuaDelayedPop(){
    if(num_to_pop){
      lua_pop(L, num_to_pop);
    }
  }

  void SetNumPop(int num) { num_to_pop = num; }
private:
  lua_State* L;
  int num_to_pop;
};

#endif /* _LUADELAYEDPOP_H_ */
