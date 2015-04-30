#ifndef _LUACOROUTINE_H_
#define _LUACOROUTINE_H_

#include <memory>
#include <string>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"

namespace Lua{
  //! Hook to be called when the coroutine runs out of execution steps allowed.
  /*! Sets the boolean variable ended_by_timeout, then calls lua_yield.
    This allows LuaCoroutine::Resume to know that the function did not return normally.
   */
  void yielding_hook(lua_State* L, lua_Debug* ar);


  //! Holds a reference to a Lua coroutine.
  class LuaCoroutine{
  public:
    //! Constructs the Lua routine, preparing it for the first resume.
    LuaCoroutine(std::shared_ptr<lua_State> parent);

    ~LuaCoroutine();

    //! Starts a function call in the coroutine.
    void LoadFunc(const char* name);

    //! Load a file into Lua
    /*! Loads a file, then executes inside the coroutine.
    */
    void LoadFile(const char* filename);

    //! Load a string into Lua
    /*! Loads a string, then executes inside the coroutine.
    */
    void LoadString(const std::string& lua_code);

    //! Starts the coroutine until it either yields or returns.
    /*! Starts the coroutine, passing in the arguments given.
      The coroutine will continue until it yields or returns.
      If SetMaxInstructions has been called with a non-zero value,
        and the coroutine uses more that number of Lua instructions,
        LuaCoroutine::Resume will throw LuaRuntimeTooLong.
     */
    template<typename RetVal = void, typename... Params>
    RetVal Resume(Params&&... params){
      if(!running){
        throw LuaCoroutineNotRunning("Cannot resume a coroutine that is not running");
      }

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
        running = false;
      } else if (result == LUA_YIELD){
        running = true;
      } else {
        auto error_message = Read<std::string>(thread, -1);
        throw LuaCoroutineExecuteError(error_message);
      }
      return Lua::Read<RetVal>(thread, top - nresults);
    }

    //! Returns whether the coroutine is currently running.
    bool IsRunning(){ return running; }

    //! Set the maximum number of Lua instructions for each Resume.
    /*! If the coroutine has neither returned nor yielded by that time,
        a LuaRuntimeTooLong will be thrown by LuaCoroutine::Resume.
     */
    void SetMaxInstructions(int instructions){
      max_instructions = instructions;
    }

    //! Returns the current maximum instructions.
    int GetMaxInstructions(){
      return max_instructions;
    }

  private:
    std::shared_ptr<lua_State> parent;

    bool running;
    lua_State* thread;
    int max_instructions;
    int reference;

    friend void yielding_hook(lua_State* L, lua_Debug* ar);
    static bool ended_by_timeout;
  };
}

#endif /* _LUACOROUTINE_H_ */
