#ifndef _LUASTATE_H_
#define _LUASTATE_H_

#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <lua.hpp>


#include "LuaCallable.hh"
#include "LuaCallable_CppFunction.hh"
#include "LuaCallable_MemberFunction.hh"
#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaMakeClass.hh"
#include "LuaObject.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"
#include "LuaTableReference.hh"
#include "TemplateUtils.hh"

namespace Lua{
  class LuaState {
  public:

    //! Constructs the LuaState.
    LuaState();

    //! Destructs the LuaState, removing self from the LuaState::all_states map.
    /*! Closes the internal lua_State
     */
    ~LuaState();

    LuaState(const LuaState&) = delete;
    LuaState(LuaState&&) = delete;
    LuaState& operator=(const LuaState&) = delete;

    //! Returns the internal lua_State
    /*! Use this as rarely as possible, if something cannot be done through the framework.
      As tpphe framework becomes more fully-featured, this may become a private function.
    */
    lua_State* state() { return L; }

    //! Load a file into Lua
    /*! Loads a file, then executes.
      May raise LuaFileNotFound, LuaFileExecuteError, or LuaFileParseError.
    */
    void LoadFile(const char* filename);

    //! Load all standard Lua libraries.
    /*! Loads all standard Lua libraries
      TODO: Provide more granular control, for creation of sandboxes.
    */
    void LoadLibs();

    //! Sets a global variable.
    /*! Sets a global variable.
      Will define a global variable inside the Lua environment.
      Requires that the value being passed can be converted to a Lua type.
      Will fail at compile time otherwise.
    */
    template<typename T>
    void SetGlobal(const char* name, T t){
      Push(t);
      lua_setglobal(L, name);
    }

    //! Returns the value of a global variable.
    /*! Returns the value of a global variable.

      @throws LuaInvalidStackContents The global variable cannot be converted to the requested type.
    */
    template<typename T>
    T CastGlobal(const char* name){
      auto obj = GetGlobal(name);
      LuaDelayedPop delayed(L, 1);
      return obj.Cast<T>();
    }

    Lua::LuaObject GetGlobal(const char* name){
      lua_getglobal(L, name);
      return Lua::LuaObject(L, -1);
    }

    //! Calls a Lua function
    /*! Calls a Lua function with parameters passed.
      The Lua function must be available in the global namespace.
      All parameters passed are converted to Lua types and passed to the function.

      Requires that all parameters passed can be converted to Lua types.
      Will fail at compile time otherwise.

      @throws LuaInvalidStackContents The return value cannot be converted to the requested type.
      @throws LuaFunctionExecuteError A lua error occurred during execution.
    */
    template<typename return_type=void, typename... Params>
    return_type Call(const char* name, Params... params){
      int top = lua_gettop(L);
      lua_getglobal(L, name);
      PushMany(std::forward<Params>(params)...);
      int result = lua_pcall(L, sizeof...(params), LUA_MULTRET, 0);
      int nresults= lua_gettop(L) - top;
      LuaDelayedPop delayed(L, nresults);
      if(result){
        auto error_message = Read<std::string>();
        throw LuaFunctionExecuteError(error_message);
      }
      return Read<return_type>(top - nresults);
    }

    Lua::LuaObject NewTable();

    template<typename ClassType>
    Lua::MakeClass<ClassType> MakeClass(std::string name){
      return Lua::MakeClass<ClassType>(L, name);
    }

  private:
    //! Pushes all arguments to the Lua stack
    /*! Pushes each parameter to the Lua stack, in the order given.
      Requires that all parameters can be converted to Lua types.
      Falls at compile time otherwise.
    */
    template<typename... Params>
    void PushMany(Params&&... params){
      Lua::PushMany(L, std::forward<Params>(params)...);
    }

    //! Pushes anything onto the Lua stack
    /*! Uses LuaObject::Push.
      I'm lazy, and would rather do "L->Push()" than "LuaObject::Push(L)"
    */
    template<typename T>
    Lua::LuaObject Push(T t){
      Lua::Push(L, t);
      return Lua::LuaObject(L);
    }

    //! Read value off of the current stack.
    /*! Read value off of the current stack.

      Assumes that it is possible to convert to the requrest type.
      Fails at compile time otherwise.

      @throws LuaInvalidStackContents  The stack position given cannot be converted to the requested type.
    */
    template<typename T>
    typename std::enable_if<!std::is_same<T, void>::value, T>::type
    Read(int stack_pos = -1){
      return Lua::Read<T>(L, stack_pos);
    }

    //! Reads no value from the current stack.
    /*! Another stunningly useful function, no?
      Needed for the Call function, which can have a return type of void.
    */
    template<typename T>
    typename std::enable_if<std::is_same<T, void>::value, T>::type
    Read() { }

    template<typename T>
    typename std::enable_if<std::is_same<T, void>::value, T>::type
    Read(int) { }

    //! The internal lua state.
    lua_State* L;
  };
}

#endif /* _LUASTATE_H_ */
