#ifndef _LUACALLABLE_MEMBERFUNCTION_H_
#define _LUACALLABLE_MEMBERFUNCTION_H_

#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"
#include "LuaRead.hh"
#include "LuaReferenceSet.hh"
#include "LuaRegistryNames.hh"
#include "TemplateUtils.hh"

namespace Lua{
  template<typename ClassType, typename T>
  class LuaCallable_MemberFunction;

  //! A wrapper around a member function, to be called from Lua.
  /*! Implements the LuaCallable interface.
    Verifies that the function was called with the correct parameters,
      and that the calling class is of the appropriate type.
    Reads each argument from the stack, calls the function,
      then pushes the result back onto the stack.

    Will read the stored value as a shared_ptr, weak_ptr, or c-style pointer,
      depending on which it was stored as.
    If the weak_ptr no longer points to a valid object,
      the method will return nil.
   */
  template<typename ClassType, typename RetVal, typename... Params>
  class LuaCallable_MemberFunction<ClassType, RetVal(Params...)> : public LuaCallable {
  public:
    // Const types cannot have non-const member functions called.
    template<typename T = ClassType,
             typename U = typename std::enable_if<!std::is_const<T>::value>::type>
    LuaCallable_MemberFunction(RetVal (ClassType::*func)(Params...)) : func(std::mem_fn(func)) { }

    // Either const or non-const types can have const member functions called.
    LuaCallable_MemberFunction(RetVal (ClassType::*func)(Params...) const ) : func(std::mem_fn(func)) { }

    virtual int call(lua_State* L){
      if(lua_gettop(L) != sizeof...(Params) + 1){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      auto ptr = ReadHeldPointer<ClassType>(L, 1);
      lua_remove(L,1);

      ClassType* cptr;
      try{
        cptr = static_cast<ClassType*>(ptr.get_c(L));
      } catch(LuaInvalidStackContents&) {
        Push(L, LuaNil());
        return 1;
      }

      return call_member_function_helper(build_indices<sizeof...(Params)>(), L, cptr);
    }

  private:
    //! Holds the method pointer.
    std::function<RetVal(ClassType*, Params...)> func;

    //! As in LuaCallable_CppFunction, needing to extract indices.
    /*! This function is used if RetVal is non-void.
     */
    template<int... Indices,
             typename R = RetVal,
             typename std::enable_if<!std::is_same<R, void>::value, int>::type = 0
             >
    int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj){
      RetVal output = func(obj, Read<Params>(L, Indices+1)...);
      int top = lua_gettop(L);
      if(std::is_reference<RetVal>::value &&
         std::is_const<RetVal>::value) {
        Push(L, std::cref(output));
      } else if (std::is_reference<RetVal>::value &&
                 !std::is_const<RetVal>::value){
          Push(L, std::ref(output));
      } else {
        Push(L, output);
      }
      return lua_gettop(L) - top;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    //! As in LuaCallable_CppFunction, need to have a special case for void.
    /*! This function is used if RetVal is void.
     */
    template<int... Indices,
             typename R = RetVal,
             typename std::enable_if<std::is_same<R, void>::value, int>::type = 0
             >
    int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj){
      func(obj, Read<Params, true>(L, Indices+1)...);
      return 0;
    }
#pragma GCC diagnostic pop
  };
}

#endif /* _LUACALLABLE_MEMBERFUNCTION_H_ */
