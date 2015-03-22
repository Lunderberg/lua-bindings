#ifndef _LUAOBJECT_H_
#define _LUAOBJECT_H_

#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaExceptions.hh"

namespace Lua{
  void NewTable(lua_State* L);

  template<typename T>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index);

  template<typename T>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index);

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

    template<typename T>
    T Cast(){
      return Read<T>(L, stack_pos);
    }

    int Length();

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
