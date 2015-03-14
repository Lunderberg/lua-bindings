#ifndef _LUATABLEREFERENCE_H_
#define _LUATABLEREFERENCE_H_

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaObject.hh"

namespace Lua{
  template<typename T>
  class LuaTableReference{
  public:
    LuaTableReference(lua_State* L, int table_stack_pos, T key)
      : L(L), table_stack_pos(table_stack_pos), key(key) { }

    template<typename V>
    LuaTableReference& operator=(V value){
      Push(L, key);
      Push(L, value);
      lua_settable(L, table_stack_pos);
      return *this;
    }

    template<typename RetVal>
    RetVal Cast(){
      LuaDelayedPop delayed(L, 1);
      return Get().Cast<RetVal>();
    }

    LuaObject Get(){
      Push(L, key);
      lua_gettable(L, table_stack_pos);
      return LuaObject(L, -1);
    }

  private:
    lua_State* L;
    int table_stack_pos;
    T key;
  };
}

#endif /* _LUATABLEREFERENCE_H_ */
