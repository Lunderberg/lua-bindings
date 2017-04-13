#ifndef _LUAKEEPALIVE_H_
#define _LUAKEEPALIVE_H_

struct lua_State;

namespace Lua{
  void InitializeKeepAliveTable(lua_State* L);
  int KeepObjectAlive(lua_State* L, int index);
  void AllowToDie(lua_State* L, int reference);
  void PushLivingToStack(lua_State* L, int reference);
}

#endif /* _LUAKEEPALIVE_H_ */
