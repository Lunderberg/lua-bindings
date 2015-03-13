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
      if(lua_gettop(L) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }
      return call_helper_function(build_indices<sizeof...(Params)>(), func, L);
    }

  private:
    std::function<RetVal(Params...)> func;

    template<int... Indices, typename RetVal_func>
    static int call_helper_function(indices<Indices...>, std::function<RetVal_func(Params...)> func,
                                    lua_State* L){
      RetVal_func output = func(LuaObject(L, Indices+1).Cast<Params>()...);
      LuaObject::Push(L, output);
      return 1;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    template<int... Indices>
    static int call_helper_function(indices<Indices...>, std::function<void(Params...)> func, lua_State* L){
      func(LuaObject(L, Indices+1).Cast<Params>()...);
      return 0;
    }
    #pragma GCC diagnostic pop
  };


  template<typename ClassType, typename T>
  class LuaCallable_MemberFunction;

  template<typename ClassType, typename RetVal, typename... Params>
  class LuaCallable_MemberFunction<ClassType, RetVal(Params...)> : public LuaCallable {
  public:
    LuaCallable_MemberFunction(RetVal (ClassType::*func)(Params...))
    : func(func){ }
    virtual int call(lua_State* L){
      if(lua_gettop(L) != sizeof...(Params) + 1){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      void* storage = lua_touserdata(L, 1);
      ClassType* obj = *reinterpret_cast<ClassType**>(storage);
      lua_remove(L, 1);

      return call_member_function_helper(build_indices<sizeof...(Params)>(), L, obj, func);
    }
  private:
    RetVal (ClassType::*func)(Params...);

    template<int... Indices, typename RetVal_func>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj,
                                           RetVal_func (ClassType::*func)(Params...)){
      RetVal output = (obj->*func)(LuaObject(L, Indices+1).Cast<Params>()...);
      LuaObject::Push(L, output);
      return 1;
    }

    // g++ incorrectly flags lua_State* L as being unused when Params... is empty
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
    template<int... Indices>
    static int call_member_function_helper(indices<Indices...>, lua_State* L, ClassType* obj,
                                           void (ClassType::*func)(Params...)){
      (obj->*func)(LuaObject(L, Indices+1).Cast<Params>()...);
      return 0;
    }
    #pragma GCC diagnostic pop
  };

  template<typename ClassType>
  class LuaCallable_ObjectDeleter : public LuaCallable {
  public:
    LuaCallable_ObjectDeleter() { }
    virtual int call(lua_State* L){
      void* storage = lua_touserdata(L, 1);
      ClassType* obj = *reinterpret_cast<ClassType**>(storage);
      delete obj;
      return 0;
    }
  };

  template<typename T>
  class LuaCallable_ObjectConstructor;

  template<typename ClassType, typename... Params>
  class LuaCallable_ObjectConstructor<ClassType(Params...)> : public LuaCallable {
  public:
    LuaCallable_ObjectConstructor(std::string metatable_name)
      : metatable_name(metatable_name) { }
    virtual int call(lua_State* L){
      return call_constructor_helper(build_indices<sizeof...(Params)>(), L, metatable_name);
    }
  private:
    std::string metatable_name;

    template<int... Indices>
    static int call_constructor_helper(indices<Indices...>, lua_State* L, std::string metatable_name){
      if(lua_gettop(L) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      ClassType* obj = new ClassType(LuaObject(L, Indices+1).Cast<Params>()...);
      void* userdata = lua_newuserdata(L, sizeof(obj));
      *reinterpret_cast<ClassType**>(userdata) = obj;

      luaL_newmetatable(L, metatable_name.c_str());
      lua_setmetatable(L, -2);

      return 1;
    }
  };



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

  static LuaObject Push(lua_State* L, LuaCallable* callable){
    // Define a new userdata, storing the LuaCallable in it.
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
  static LuaObject Push(lua_State* L, std::function<RetVal(Params...)> func){
    LuaCallable* callable = new LuaCallable_Implementation<RetVal(Params...)>(func);
    return Push(L, callable);
  }

  template<typename ClassType, typename RetVal, typename... Params>
  static LuaObject Push(lua_State* L, RetVal (ClassType::*func)(Params...)){
    LuaCallable* callable = new LuaCallable_MemberFunction<ClassType, RetVal(Params...)>(func);
    return Push(L, callable);
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

  LuaTableReference<std::string> operator[](std::string key);
  LuaTableReference<int> operator[](int key);



private:
  friend class LuaState;

  int LuaType();

  lua_State* L;
  int stack_pos;

  friend int call_cpp_function(lua_State*);
  friend int garbage_collect_cpp_function(lua_State*);
};

template<typename ClassType>
class MakeClass {
public:
  MakeClass(lua_State* L, std::string name) : L(L), name(name), metatable(L), index(L){
    luaL_newmetatable(L, name.c_str());
    metatable = LuaObject(L);
    metatable["__gc"] = new LuaObject::LuaCallable_ObjectDeleter<ClassType>();
    metatable["__metatable"] = "Access restricted";

    index = LuaObject::NewTable(L);
  }
  ~MakeClass(){
    metatable["__index"] = index;
    lua_pop(L, 1);
  }

  template<typename RetVal, typename... Params>
  MakeClass& AddMethod(std::string method_name, RetVal (ClassType::*func)(Params...)){
    index[method_name] = func;
    return *this;
  }

  template<typename... Params>
  MakeClass& AddConstructor(std::string constructor_name = ""){
    if(constructor_name.size() == 0){
      constructor_name = name;
    }
    LuaObject::Push(L, new LuaObject::LuaCallable_ObjectConstructor<ClassType(Params...)>(name));
    lua_setglobal(L, constructor_name.c_str());

    return *this;
  }

private:
  lua_State* L;
  std::string name;
  LuaObject metatable;
  LuaObject index;
};


#endif /* _LUAOBJECT_H_ */
