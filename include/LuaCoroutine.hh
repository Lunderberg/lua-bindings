#ifndef _LUACOROUTINE_H_
#define _LUACOROUTINE_H_

#include <string>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"

namespace Lua{
  class LuaCoroutine{
  public:
    LuaCoroutine(lua_State* parent, const char* function);

    template<typename RetVal = void, typename... Params>
    RetVal Resume(Params&&... params){
      int top = lua_gettop(thread);
      Lua::PushMany(thread, std::forward<Params>(params)...);
      int result = lua_resume(thread, NULL, sizeof...(params));
      int nresults = lua_gettop(thread) - top;
      LuaDelayedPop delayed(thread, nresults);
      if(result == LUA_OK){
        finished = true;
      } else if (result == LUA_YIELD){
        finished = false;
      } else {
        auto error_message = Read<std::string>(thread, -1);
        throw LuaCoroutineExecuteError(error_message);
      }
      return Lua::Read<RetVal>(thread, top - nresults);
    }

    bool IsFinished(){ return finished; }

  private:
    bool finished;
    lua_State* thread;
  };
}

#endif /* _LUACOROUTINE_H_ */
