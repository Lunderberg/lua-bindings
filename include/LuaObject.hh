#ifndef _LUAOBJECT_H_
#define _LUAOBJECT_H_

#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"

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

  static void Push(lua_State* L, lua_CFunction t);
  static void Push(lua_State* L, const char* string);
  static void Push(lua_State* L, std::string string);

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

private:
  friend class LuaState;

  int LuaType();

  lua_State* L;
  int stack_pos;
};

#endif /* _LUAOBJECT_H_ */
