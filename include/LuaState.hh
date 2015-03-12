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
#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "TemplateUtils.hh"

class LuaState : public std::enable_shared_from_this<LuaState> {
  //! Dummy structure for preventing declaration of LuaState on the stack.
  /*! std::make_shared requires a public constructor.
    This way, the only constructor available requires a private member,
      and can therefore only be called from within the class.
   */
  struct HiddenStruct { };
public:

  //! Constructs the LuaState, requiring the private HiddenStruct.
  /*! I would like to make the constructor private, but that breaks std::shared_ptr
    Instead, this depends on a privately available class.
   */
  LuaState(HiddenStruct);

  //! Destructs the LuaState, removing self from the LuaState::all_states map.
  /*! Closes the internal lua_State and removes self from LuaState::all_states.
   */
  ~LuaState();

  LuaState(const LuaState&) = delete;
  LuaState(LuaState&&) = delete;
  LuaState& operator=(const LuaState&) = delete;

  //! Creates and returns a shared_ptr to a LuaState
  /*! For implementation reasons, all LuaState objects must be contained in std::shared_ptr
    This is guaranteed by restricted all LuaState constructors and instead providing creat().
   */
  static std::shared_ptr<LuaState> create();

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

  LuaObject GetGlobal(const char* name){
    lua_getglobal(L, name);
    return LuaObject(L, -1);
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
    PushMany(params...);
    int result = lua_pcall(L, sizeof...(params), LUA_MULTRET, 0);
    int nresults= lua_gettop(L) - top;
    LuaDelayedPop delayed(L, nresults);
    if(result){
      auto error_message = Read<std::string>();
      throw LuaFunctionExecuteError(error_message);
    }
    return Read<return_type>();
  }

  LuaObject NewTable();

public:
  //private:
  //! Pushes all arguments to the Lua stack
  /*! Pushes each parameter to the Lua stack, in the order given.
    Requires that all parameters can be converted to Lua types.
    Falls at compile time otherwise.
   */
  template<typename FirstParam, typename... Params>
  void PushMany(FirstParam&& first, Params&&... params){
    Push(std::forward<FirstParam>(first));
    PushMany(std::forward<Params>(params)...);
  }
  //! Pushes zero arguments to the Lua stack.
  /*! A very useful function, isn't it?
    This is needed as the end of recursion of PushMany().
   */
  void PushMany(){ }

  //! Pushes a number onto the Lua stack
  /*! Pushes a number onto the Lua stack
    The number must be some arithmetic type.
   */
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, LuaObject>::type Push(T t){
    LuaObject::Push(L, t);
    return LuaObject(L, -1);
  }

  //! Pushes a lua_CFunction onto the Lua stack
  /*! Pushes a C-style function pointer int(*)(lua_State*).
    This does not register anything in the cpp_functions vector.
   */
  LuaObject Push(lua_CFunction t);

  //! Pushes a string onto the Lua stack
  LuaObject Push(const char* string);

  //! Pushes a string onto the Lua stack
  LuaObject Push(std::string string);

  LuaObject Push(LuaObject obj);

  //! Pushes a C++ function onto the Lua stack.
  /*! Pushes a C++ function onto the Lua stack.
    Converts to a std::function for consistency, then pushes.

    Requires that all function parameters are convertable from Lua types.
    Fails at compile time otherwise.

    Requires that function return type is convertable to a Lua type.
    Fails at compile time otherwise.
   */
  template<typename RetVal, typename... Params>
  LuaObject Push(RetVal(*func)(Params...)){
    return Push(std::function<RetVal(Params...)>(func));
  }

  //! Pushes a std::function onto the Lua stack.
  /*! Pushes a std::function onto the Lua stack.
    Creates a new LuaCallable_Implementation, which is added to the cpp_functions vector.
    Pushes a new lua_cclosure with the index of the new function in the cpp_functions vector.

    When called, first goes to call_cpp_function, which finds the appropriate std::function to call.
   */
  template<typename RetVal, typename... Params>
  LuaObject Push(std::function<RetVal(Params...)> func){
    // Define a new userdata, storing the LuaCallable in it.
    LuaCallable* callable = new LuaCallable_Implementation<RetVal, Params...>(func);
    void* userdata = lua_newuserdata(L, sizeof(callable));
    *reinterpret_cast<LuaCallable**>(userdata) = callable;

    // Create the metatable
    auto table = NewTable();
    table["__call"] = call_cpp_function;
    table["__gc"] = garbage_collect_cpp_function;
    lua_setmetatable(L, -2);

    return LuaObject(L, -1);
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
    LuaObject obj(L, stack_pos);
    return obj.Cast<T>();
  }

  //! Reads no value from the current stack.
  /*! Another stunningly useful function, no?
    Needed for the Call function, which can have a return type of void.
   */
  template<typename T>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read() { }

  //! The internal lua state.
  lua_State* L;
};

#endif /* _LUASTATE_H_ */
