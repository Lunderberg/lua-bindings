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


#include "detail/LuaCallable.hh"
#include "detail/LuaCallable_CppFunction.hh"
#include "detail/LuaCallable_MemberFunction.hh"
#include "detail/LuaCoroutine.hh"
#include "detail/LuaDelayedPop.hh"
#include "detail/LuaExceptions.hh"
#include "detail/LuaMakeClass.hh"
#include "detail/LuaObject.hh"
#include "detail/LuaPush.hh"
#include "detail/LuaRead.hh"
#include "detail/LuaTableReference.hh"
#include "detail/TemplateUtils.hh"

namespace Lua{
  void* limited_memory_alloc(void* ud, void* ptr, size_t osize, size_t nsize);

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

    //! Returns the current memory used, in bytes.
    /*! Returns all memory allocated by the lua virtual machine.
      Note that this does not include any classes that are constructed by Lua,
        only memory held directly by the virtual machine.
     */
    unsigned long GetMemoryUsage(){ return memory[0]; }

    //! Set the limit of memory that can be used by the virtual machine.
    /*! The limit, in bytes, of the memory that can be allocated by the virtual machine.
      To allow any size allocation, set the maximum as 0.
     */
    void SetMaxMemory(unsigned long max_memory){ memory[1] = max_memory; }

    //! Returns the current memory limit.
    unsigned long GetMaxMemory(){ return memory[1]; }

    //! Load a file into Lua
    /*! Loads a file, then executes.
      May raise LuaFileNotFound, LuaFileExecuteError, or LuaFileParseError.
    */
    template<typename RetVal=void, typename... Params>
    RetVal LoadFile(const char* filename, Params&&... params){
      int load_result = luaL_loadfilex(L, filename, "t");
      if(load_result){
        auto error_message = Lua::Read<std::string>(L, -1);
        if(load_result == LUA_ERRFILE){
          throw LuaFileNotFound(filename);
        } else {
          throw LuaFileParseError(error_message);
        }
      }

      return CallFromStack<RetVal>(std::forward<Params>(params)...);
    }

    //! Load a string into Lua
    /*! Loads a string, then executes.
      May raise LuaFileExecuteError, or LuaFileParseError.
    */
    template<typename RetVal=void, typename... Params>
    RetVal LoadString(const std::string& lua_code, Params&&... params){
      int load_result = luaL_loadstring(L, lua_code.c_str());
      if(load_result){
        auto error_message = Lua::Read<std::string>(L, -1);
        throw LuaFileParseError(error_message);
      }

      return CallFromStack<RetVal>(std::forward<Params>(params)...);
    }

    //! Load all standard Lua libraries.
    /*! Loads all standard Lua libraries
      TODO: Provide more granular control, for creation of sandboxes.
    */
    void LoadLibs();

    //! Loads all libraries that are safe for untrusted users.
    void LoadSafeLibs();

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

    //! Returns a LuaObject representing the object requested.
    /*! Users of the library should avoid this function.
      It places something onto the stack and does not remove it.
      It might be necessary for some users, which is why it remains public.
     */
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
    template<typename RetVal=void, typename... Params>
    RetVal Call(const char* name, Params&&... params){
      lua_getglobal(L, name);
      return CallFromStack<RetVal>(std::forward<Params>(params)...);
    }

    //! Creates and returns a new table
    /*! Another of those functions that would be best avoided.
      It places a table on the stack, which the user must later remove.
     */
    Lua::LuaObject NewTable();

    //! Defines a class in the LuaState.
    /*! Returns a proxy object that can be used to make a class.
      Usage:
        Lua::LuaState L;
        L.MakeClass<ClassName>("ClassName")
          .AddConstructor<ParamType1, ParamType2, ...>("ConstructorName")
          .AddMethod("Method1", &ClassName::Method1)
          .AddMethod("Method2", &ClassName::Method2);

        ClassName, the template parameter, is the class being wrapped.
        "ClassName", the string parameter, is the name of the class, for debug messages.
        <ParamType1, ParamType2, ...> are the parameter types passed to the constructor being registered.
        "ConstructorName" is the name of the Lua function which exposes this contructor.
        "Method1" is the name of the method as it is exposed to Lua.
     */
    template<typename ClassType>
    Lua::MakeClass<ClassType> MakeClass(std::string name){
      return Lua::MakeClass<ClassType>(L, name);
    }

    //! Returns a new coroutine calling the function given.
    /*! Returns a new coroutine which can be resumed repeatedly.
      Each time, LuaCoroutine::Resume<Output>(params...) accepts any parameters.
      These are passed as arguments either to the function itself,
        or as the return values of coroutine.yield.
      The return value from LuaCoroutine::Resume are the values passed to coroutine.yield
        or the final return value from the function, whichever occurs.
     */
    LuaCoroutine NewCoroutine(const char* name){
      return LuaCoroutine(L, name);
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

    //! Calls a function from the stack.
    /*! Assumes that the Lua stack has a function on top.
      Pushes each argument onto the stack.
      Returns the return type requested.
     */
    template<typename RetVal=void, typename... Params>
    RetVal CallFromStack(Params&&... params){
      int top = lua_gettop(L) - 1; // -1 because the function is already on the stack.
      PushMany(std::forward<Params>(params)...);
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

    //! The internal lua state.
    lua_State* L;
    //! The total memory used and allowed
    /*! Keeps track of the memory allocated by the Lua virtual machine.
      memory[0] is the size in bytes that have been allocated.
      memory[1] is the maximum size in bytes.
      If memory[1] is 0, there is no restriction.
     */
    unsigned long memory[2];
  };
}

#endif /* _LUASTATE_H_ */
