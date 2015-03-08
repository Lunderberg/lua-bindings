#ifndef _LUASTATE_H_
#define _LUASTATE_H_

#include <iostream>
#include <memory>
#include <utility>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"

class LuaState : public std::enable_shared_from_this<LuaState> {
  // std::make_shared requires a public constructor.
  // Having all constructors require a private struct ensures that it can only be called from this class.
  // In turn, this allows for restriction of the LuaState to always be made through a shared_ptr
  struct HiddenStruct { };
public:
  LuaState(HiddenStruct);
  ~LuaState();

  LuaState(const LuaState&) = delete;
  LuaState(LuaState&&) = delete;
  LuaState& operator=(const LuaState&) = delete;

  static std::shared_ptr<LuaState> create();

  lua_State* state() { return L; }

  void LoadFile(const char* filename);
  void LoadLibs();

  template<typename T>
  void SetGlobal(const char* name, T t){
    Push(t);
    lua_setglobal(L, name);
  }

  template<typename T>
  T GetGlobal(const char* name){
    lua_getglobal(L, name);
    LuaDelayedPop(L, 1);
    LuaObject obj(L, -1);
    return obj.Cast<T>();
  }

  template<typename return_type=void, typename... Params>
  return_type Call(const char* name, Params... params){
    int top = lua_gettop(L);
    lua_getglobal(L, name);
    PushMany(params...);
    int result = lua_pcall(L, sizeof...(params), LUA_MULTRET, 0);
    int nresults= lua_gettop(L) - top;
    LuaDelayedPop delayed(L, nresults);
    if(result){
      auto error_message = Read<std::string>();
      throw LuaFunctionExecuteError(error_message);
    }
    return Read<return_type>();
  }

private:
  template<typename FirstParam, typename... Params>
  void PushMany(FirstParam&& first, Params&&... params){
    Push(std::forward<FirstParam>(first));
    PushMany(std::forward<Params>(params)...);
  }
  void PushMany(){ }

  template<typename T>
  void Push(T t);

  template<typename T>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(int stack_pos = -1){
    LuaObject obj(L, stack_pos);
    return obj.Cast<T>();
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read() { }

  lua_State* L;
};

#endif /* _LUASTATE_H_ */
