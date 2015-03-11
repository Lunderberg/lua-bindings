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

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaUtils.hh"
#include "TemplateUtils.hh"

//! Dispatches a call to a C++ function when called from Lua
/*! Lua requires a strict C-style function pointer for callbacks.
  In addition, the function must interact directly with the lua_State.
  By having a universal callback that then dispatches,
    functions can be exposed to Lua more cleanly.
 */
int call_cpp_function(lua_State* L);

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
    As the framework becomes more fully-featured, this may become a private function.
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
    LuaPush(L, t);
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
    Push(cpp_functions.size());
    auto callable_ptr = std::unique_ptr<LuaCallable_Implementation<RetVal, Params...>>(
                                    new LuaCallable_Implementation<RetVal, Params...>(func));
    cpp_functions.push_back(std::move(callable_ptr));
    lua_pushcclosure(L, call_cpp_function, 1);
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

  //! Abstract base class for callable functions.
  /*! Allows for a single call signature regardless of the wrapped function.
   */
  class LuaCallable{
  public:
    virtual ~LuaCallable() { }
    virtual int call(std::shared_ptr<LuaState> L) = 0;
  };

  //! Wraps a std::function, converting arguments and return value to/from the Lua state.
  /*! Templated on the parameters and return type of the function.
    If either the parameters or the return type are not convertable to Lua types,
      it will fail to compile.
   */
  template<typename RetVal, typename... Params>
  class LuaCallable_Implementation : public LuaCallable {
  public:
    LuaCallable_Implementation(std::function<RetVal(Params...)> function) :
      func(function) { }

    //! Calls the wrapped function.
    /*! Uses the indices trick to number each parameter.
      From there, reads each argument from the stack and calls the function.
      Pushes the return value onto the stack.

      @throws LuaCppCallError The number of arguments passed from Lua is incorrect.
      @throws LuaInvalidStackContents The return valud could not be converted to the requested type.
     */
    virtual int call(std::shared_ptr<LuaState> L){
      return call_helper(build_indices<sizeof...(Params)>(), L);
    }

  private:
    template<int... Indices>
    int call_helper(indices<Indices...>, std::shared_ptr<LuaState> L){
      if(lua_gettop(L->state()) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }
      RetVal output = func(L->Read<Params>(Indices+1)...);
      L->Push(output);
      return 1;
    }

    std::function<RetVal(Params...)> func;
  };

  //! The internal lua state.
  lua_State* L;

  //! A container to hold the callable lua objects.
  std::vector<std::unique_ptr<LuaCallable> > cpp_functions;

  //! The call_cpp_function must be able to access the list of cpp_functions held.
  friend int call_cpp_function(lua_State*);

  //! Map to determine LuaState, given a lua_State.
  /*! Each LuaState registers here upon construction, and deregisters on destruction.
    This way, functions called from lua can determine the parent LuaState.
   */
  static std::map<lua_State*, std::weak_ptr<LuaState> > all_states;

  //! Registers a LuaState in the map.
  static void RegisterCppState(std::shared_ptr<LuaState> state);

  //! Removes a LuaState from the map.
  static void DeregisterCppState(lua_State* c_state);

  //! Retrieves a LuaState from the map.
  static std::shared_ptr<LuaState> GetCppState(lua_State* c_state);
};

#endif /* _LUASTATE_H_ */
