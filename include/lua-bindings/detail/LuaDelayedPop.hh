#ifndef _LUADELAYEDPOP_H_
#define _LUADELAYEDPOP_H_

#include <lua.hpp>

//! Utility class for popping values.
/*! Utility class, for internal use.
  Pops values from the lua stack in the destructor,
  ensuring that the value is popped in an exception-safe manner.
 */
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
  int GetNumPop() const { return num_to_pop; }
private:
  lua_State* L;
  int num_to_pop;
};

#endif /* _LUADELAYEDPOP_H_ */
