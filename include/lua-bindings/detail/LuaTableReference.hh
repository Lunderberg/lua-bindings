#ifndef _LUATABLEREFERENCE_H_
#define _LUATABLEREFERENCE_H_

#include <iostream>

#include <lua.hpp>

namespace Lua{
  class LuaObject;

  //! Convenience class for reading/writing to a lua table.
  template<typename T>
  class LuaTableReference{
  public:
    LuaTableReference(lua_State* L, int table_stack_pos, T key);

    template<typename V>
    LuaTableReference& operator=(V&& value);

    //! Must be used when setting a reference equal to another reference.
    /*! Otherwise, the copy constructor is called instead.
      I can't figure out a way around this.
     */
    template<typename V>
    LuaTableReference& Set(V&& value);

    template<typename RetVal>
    RetVal Cast();

    LuaObject Get();

    bool Exists();

  private:
    lua_State* L;
    int table_stack_pos;
    T key;
  };
}

#include "LuaDelayedPop.hh"
#include "LuaObject.hh"

template<typename T>
Lua::LuaTableReference<T>::LuaTableReference(lua_State* L, int table_stack_pos, T key)
  : L(L), table_stack_pos(lua_absindex(L, table_stack_pos)), key(key) { }

template<typename T>
template<typename V>
Lua::LuaTableReference<T>& Lua::LuaTableReference<T>::operator=(V&& value){
  return Set(std::forward<V>(value));
}

template<typename T>
template<typename V>
Lua::LuaTableReference<T>& Lua::LuaTableReference<T>::Set(V&& value){
  Push(L, key);
  Push(L, std::forward<V>(value));

  lua_settable(L, table_stack_pos);

  return *this;
}

template<typename T>
template<typename RetVal>
RetVal Lua::LuaTableReference<T>::Cast(){
  LuaDelayedPop delayed(L, 1);
  return Get().Cast<RetVal>();
}

template<typename T>
Lua::LuaObject Lua::LuaTableReference<T>::Get(){
  Push(L, key);
  lua_gettable(L, table_stack_pos);
  return LuaObject(L, -1);
}

template<typename T>
bool Lua::LuaTableReference<T>::Exists(){
  LuaDelayedPop delayed(L, 1);
  auto obj = Get();
  return !obj.IsNil();
}


#endif /* _LUATABLEREFERENCE_H_ */
