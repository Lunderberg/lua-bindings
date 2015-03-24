#ifndef _LUACALLABLE_H_
#define _LUACALLABLE_H_

#include <lua.hpp>

namespace Lua{
  //! Abstract base class for callable functions.
  /*! Allows for a single call signature regardless of the wrapped function.
   */
  class LuaCallable{
  public:
    virtual ~LuaCallable() { }
    virtual int call(lua_State* L) = 0;
  };

  //! Dispatches a call to a C++ function when called from Lua
  /*! Lua requires a strict C-style function pointer for callbacks.
    In addition, the function must interact directly with the lua_State.
    By having a universal callback that then dispatches,
    functions can be exposed to Lua more cleanly.
  */
  int call_cpp_function(lua_State* L);

  //! Calls the destructor of the LuaCallable.
  /*! As before, I need a C-style function pointer.
   */
  int garbage_collect_cpp_function(lua_State* L);
}

#endif /* _LUACALLABLE_H_ */
