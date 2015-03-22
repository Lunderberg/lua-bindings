#ifndef _LUACOROUTINE_H_
#define _LUACOROUTINE_H_

#include <string>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"

namespace Lua{
  void yielding_hook(lua_State* L, lua_Debug* ar);


  class LuaCoroutine{
  public:
    LuaCoroutine(lua_State* parent, const char* function);

    template<typename RetVal = void, typename... Params>
    RetVal Resume(Params&&... params){
      int top = lua_gettop(thread);
      Lua::PushMany(thread, std::forward<Params>(params)...);

      ended_by_timeout = false;
      if(max_instructions > 0){
        lua_sethook(thread, yielding_hook, LUA_MASKCOUNT, max_instructions);
      }
      int result = lua_resume(thread, NULL, sizeof...(params));
      lua_sethook(thread, NULL, 0, 0);

      int nresults = lua_gettop(thread) - top;
      LuaDelayedPop delayed(thread, nresults);

      if(ended_by_timeout){
        throw LuaRuntimeTooLong("Function exceeded runtime allowed.");
      }

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

    void SetMaxInstructions(int instructions){
      max_instructions = instructions;
    }

  private:
    bool finished;
    lua_State* thread;
    int max_instructions;

    friend void yielding_hook(lua_State* L, lua_Debug* ar);
    static bool ended_by_timeout;
  };
}

#endif /* _LUACOROUTINE_H_ */
