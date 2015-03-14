#ifndef _LUAOBJECT_H_
#define _LUAOBJECT_H_

#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaExceptions.hh"

namespace Lua{
  void NewTable(lua_State* L);

  template<typename T>
  class LuaTableReference;

  class LuaObject{
  public:
    LuaObject(lua_State* L, int stack_pos=-1);
    virtual ~LuaObject() { }

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
  };
}

#endif /* _LUAOBJECT_H_ */
