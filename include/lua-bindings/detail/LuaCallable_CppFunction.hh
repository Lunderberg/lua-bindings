#ifndef _LUACALLABLE_CPPFUNCTION_H_
#define _LUACALLABLE_CPPFUNCTION_H_

#include <functional>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaPush.hh"
#include "TemplateUtils.hh"

namespace Lua{
  template<typename T>
  class LuaCallable_CppFunction;

  //! Wraps a std::function, converting arguments and return value to/from the Lua state.
  /*! Templated on the parameters and return type of the function.
    If either the parameters or the return type are not convertable to Lua types,
    it will fail to compile.
  */
  template<typename RetVal, typename... Params>
  class LuaCallable_CppFunction<RetVal(Params...)> : public LuaCallable  {
  public:
    LuaCallable_CppFunction(std::function<RetVal(Params...)> function) :
      func(function) { }

    //! Calls the wrapped function.
    /*! Uses the indices trick to number each parameter.
      From there, reads each argument from the stack and calls the function.
      Pushes the return value onto the stack.

      @throws LuaCppCallError The number of arguments passed from Lua is incorrect.
      @throws LuaInvalidStackContents The return valud could not be converted to the requested type.
    */
    virtual int call(lua_State* L){
      if(lua_gettop(L) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }
      return call_helper_function(build_indices<sizeof...(Params)>(), func, L);
    }

  private:
    //! The function being wrapped.
    std::function<RetVal(Params...)> func;

    //! Helper function to extract indices for each parameter.
    /*! I need to get an index for each parameter.
      The template voodoo magic is explained in TemplateUtils.hh
     */
    template<int... Indices, typename RetVal_func>
    static int call_helper_function(indices<Indices...>, std::function<RetVal_func(Params...)> func,
                                    lua_State* L){
      RetVal_func output = func(Read<Params>(L, Indices+1)...);
      int top = lua_gettop(L);
      Push(L, output);
      return lua_gettop(L) - top;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    //! Separate helper function for void functions.
    /*! I can't use the earlier template, because "void output = ..." is meaningless.
     */
    template<int... Indices>
    static int call_helper_function(indices<Indices...>, std::function<void(Params...)> func, lua_State* L){
      func(Read<Params>(L, Indices+1)...);
      return 0;
    }
#pragma GCC diagnostic pop
  };
}

#endif /* _LUACALLABLE_CPPFUNCTION_H_ */
