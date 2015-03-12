#ifndef _LUACALLABLE_H_
#define _LUACALLABLE_H_

#include <functional>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "TemplateUtils.hh"

//! Abstract base class for callable functions.
/*! Allows for a single call signature regardless of the wrapped function.
 */
class LuaCallable{
public:
  virtual ~LuaCallable() { }
  virtual int call(lua_State* L) = 0;
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
  virtual int call(lua_State* L){
    return call_helper(build_indices<sizeof...(Params)>(), L);
  }

private:
  template<int... Indices>
  int call_helper(indices<Indices...>, lua_State* L){
    if(lua_gettop(L) != sizeof...(Params)){
      throw LuaCppCallError("Incorrect number of arguments passed");
    }
    RetVal output = func(LuaObject(L, Indices+1).Cast<Params>()...);
    LuaObject::Push(L, output);
    return 1;
  }

  std::function<RetVal(Params...)> func;
};

//! Dispatches a call to a C++ function when called from Lua
/*! Lua requires a strict C-style function pointer for callbacks.
  In addition, the function must interact directly with the lua_State.
  By having a universal callback that then dispatches,
    functions can be exposed to Lua more cleanly.
 */
int call_cpp_function(lua_State* L);

int garbage_collect_cpp_function(lua_State* L);

#endif /* _LUACALLABLE_H_ */
