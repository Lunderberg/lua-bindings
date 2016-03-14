#ifndef _LUAOBJECT_H_
#define _LUAOBJECT_H_

#include <string>
#include <type_traits>

#include <lua.hpp>

#include "LuaExceptions.hh"

namespace Lua{
  //! Creates a new table on the stack.
  void NewTable(lua_State* L);

  template<typename T, bool allow_references = false>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index);

  template<typename T, bool allow_references = false>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index);

  template<typename T>
  class LuaTableReference;

  //! Utility Class for holding a lua object that is currently on the stack.
  class LuaObject{
  public:
    LuaObject(lua_State* L, int stack_pos=-1);
    virtual ~LuaObject() { }

    bool IsNumber() const;
    bool IsString() const;
    bool IsFunction() const;
    bool IsNil() const;
    bool IsBoolean() const;
    bool IsTable() const;
    bool IsUserData() const;
    bool IsLightUserData() const;
    bool IsThread() const;

    template<typename T>
    T Cast(){
      return Read<T>(L, stack_pos);
    }

    int Length() const;

    void MoveToTop();
    void Pop();

    int StackPos() const;

    LuaTableReference<std::string> operator[](std::string key);
    LuaTableReference<int> operator[](int key);

    bool operator==(const LuaObject& other) const;

  private:
    friend class LuaState;

    int LuaType() const;

    lua_State* L;
    int stack_pos;
  };
}

#endif /* _LUAOBJECT_H_ */
