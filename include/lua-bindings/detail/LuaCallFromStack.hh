#ifndef _LUACALLFROMSTACK_H_
#define _LUACALLFROMSTACK_H_

#include <lua.hpp>

namespace Lua{
  template<typename RetVal=void, typename... Params>
  RetVal CallFromStack(lua_State* L, Params&&... params);
}

#include "LuaExceptions.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"
#include "LuaReferenceSet.hh"

namespace Lua{
  template<typename RetVal=void, typename... Params>
  RetVal CallFromStack(lua_State* L, Params&&... params){
    int top = lua_gettop(L) - 1; // -1 because the function is already on the stack.
    PreserveValidReferences ref_save(L);
    PushMany<true>(L, std::forward<Params>(params)...);
    int result = lua_pcall(L, sizeof...(params), LUA_MULTRET, 0);
    int nresults = lua_gettop(L) - top;
    LuaDelayedPop delayed(L, nresults);
    if(result){
      auto error_message = Read<std::string>(L, -1);
      if(result == LUA_ERRMEM){
        throw LuaOutOfMemoryError(error_message);
      } else {
        throw LuaExecuteError(error_message);
      }
    }
    return Lua::Read<RetVal>(L, top - nresults);
  }
}

#endif /* _LUACALLFROMSTACK_H_ */
