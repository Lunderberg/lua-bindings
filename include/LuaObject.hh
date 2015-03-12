#ifndef _LUAOBJECT_H_
#define _LUAOBJECT_H_

#include <functional>
#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaRegistryNames.hh"
#include "TemplateUtils.hh"

//! Dispatches a call to a C++ function when called from Lua
/*! Lua requires a strict C-style function pointer for callbacks.
  In addition, the function must interact directly with the lua_State.
  By having a universal callback that then dispatches,
    functions can be exposed to Lua more cleanly.
 */
int call_cpp_function(lua_State* L);

int garbage_collect_cpp_function(lua_State* L);

class LuaObject{
public:
  LuaObject(lua_State* L, int stack_pos=-1);
  virtual ~LuaObject() { }

  template<typename T>
  static typename std::enable_if<std::is_arithmetic<T>::value, LuaObject>::type
  Push(lua_State* L, T t){
    lua_pushnumber(L, t);
    return LuaObject(L);
  }

  static LuaObject Push(lua_State* L, lua_CFunction t);
  static LuaObject Push(lua_State* L, const char* string);
  static LuaObject Push(lua_State* L, std::string string);
  static LuaObject Push(lua_State* L, LuaObject obj);

  static LuaObject NewTable(lua_State* L);

  template<typename RetVal, typename... Params>
  static LuaObject Push(lua_State* L, std::function<RetVal(Params...)> func){
    // Define a new userdata, storing the LuaCallable in it.
    LuaCallable* callable = new LuaCallable_Implementation<RetVal(Params...)>(func);
    void* userdata = lua_newuserdata(L, sizeof(callable));
    *reinterpret_cast<LuaCallable**>(userdata) = callable;

    // Create the metatable
    int metatable_uninitialized = luaL_newmetatable(L, cpp_function_registry_entry.c_str());
    if(metatable_uninitialized){
      LuaObject table(L);
      table["__call"] = call_cpp_function;
      table["__gc"] = garbage_collect_cpp_function;
      table["__metatable"] = "Access restricted";
    }
    lua_setmetatable(L, -2);

    return LuaObject(L, -1);
  }

  template<typename RetVal, typename... Params>
  static LuaObject Push(lua_State* L, RetVal (*func)(Params...)){
    return Push(L, std::function<RetVal(Params...)>(func));
  }

  bool IsNumber();
  bool IsString();
  bool IsFunction();
  bool IsNil();
  bool IsBoolean();
  bool IsTable();
  bool IsUserData();
  bool IsLightUserData();
  bool IsThread();

  // Yucky template usage so that I can have compile-time checks
  //   of whether each cast is available.
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, T>::type Cast() {
    if(IsNumber()){
      return lua_tonumber(L, stack_pos);
    } else {
      throw LuaInvalidStackContents("Stack did not contain a number");
    }
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type Cast(){
    if(IsString()){
      return lua_tostring(L, stack_pos);
    } else {
      throw LuaInvalidStackContents("Stack did not contain a string");
    }
  }

  void MoveToTop();
  void Pop();

private:
  template<typename T>
  class LuaTableReference{
  public:
    LuaTableReference(lua_State* L, int table_stack_pos, T key)
      : L(L), table_stack_pos(table_stack_pos), key(key) { }

    template<typename V>
    LuaTableReference& operator=(V value){
      LuaObject::Push(L, key);
      LuaObject::Push(L, value);
      lua_settable(L, table_stack_pos);
      return *this;
    }

    template<typename RetVal>
    RetVal Cast(){
      LuaDelayedPop delayed(L, 1);
      return Get().Cast<RetVal>();
    }

    LuaObject Get(){
      LuaObject::Push(L, key);
      lua_gettable(L, table_stack_pos);
      return LuaObject(L, -1);
    }

  private:
    lua_State* L;
    int table_stack_pos;
    T key;
  };

public:
  LuaTableReference<std::string> operator[](std::string key);
  LuaTableReference<int> operator[](int key);

public:
  friend class LuaState;

  int LuaType();

  lua_State* L;
  int stack_pos;

  friend int call_cpp_function(lua_State*);
  friend int garbage_collect_cpp_function(lua_State*);
  //! Abstract base class for callable functions.
  /*! Allows for a single call signature regardless of the wrapped function.
   */
  class LuaCallable{
  public:
    virtual ~LuaCallable() { }
    virtual int call(lua_State* L) = 0;
  };

  template<typename T>
  class LuaCallable_Implementation;

  //! Wraps a std::function, converting arguments and return value to/from the Lua state.
  /*! Templated on the parameters and return type of the function.
    If either the parameters or the return type are not convertable to Lua types,
    it will fail to compile.
  */
  template<typename RetVal, typename... Params>
  class LuaCallable_Implementation<RetVal(Params...)> : public LuaCallable  {
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
      return call_helper_function(build_indices<sizeof...(Params)>(), func, L);
    }

  private:
    std::function<RetVal(Params...)> func;
  };

  template<int... Indices, typename RetVal, typename... Params>
  static int call_helper_function(indices<Indices...>, std::function<RetVal(Params...)> func, lua_State* L){
    if(lua_gettop(L) != sizeof...(Params)){
      throw LuaCppCallError("Incorrect number of arguments passed");
    }
    RetVal output = func(LuaObject(L, Indices+1).Cast<Params>()...);
    LuaObject::Push(L, output);
    return 1;
  }

  template<int... Indices, typename... Params>
  static int call_helper_function(indices<Indices...>, std::function<void(Params...)> func, lua_State* L){
    if(lua_gettop(L) != sizeof...(Params)){
      throw LuaCppCallError("Incorrect number of arguments passed");
    }
    func(LuaObject(L, Indices+1).Cast<Params>()...);
    return 0;
  }



};


#endif /* _LUAOBJECT_H_ */
