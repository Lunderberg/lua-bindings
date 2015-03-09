#ifndef _LUASTATE_H_
#define _LUASTATE_H_

#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <lua.hpp>

#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaUtils.hh"

int call_cpp_function(lua_State* L);

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

  void Push(int t);
  void Push(lua_CFunction t);

  template<typename RetVal, typename... Params>
  void Push(RetVal(*func)(Params...)){
    Push(std::function<RetVal(Params...)>(func));
  }

  template<typename RetVal, typename... Params>
  void Push(std::function<RetVal(Params...)> func){
    Push(cpp_functions.size());
    auto callable_ptr = std::unique_ptr<LuaCallable_Implementation<RetVal, Params...>>(
                                    new LuaCallable_Implementation<RetVal, Params...>(func));
    cpp_functions.push_back(std::move(callable_ptr));
    lua_pushcclosure(L, call_cpp_function, 1);
  }

  template<typename T>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(int stack_pos = -1){
    LuaObject obj(L, stack_pos);
    return obj.Cast<T>();
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read() { }


  class LuaCallable{
  public:
    virtual ~LuaCallable() { }
    virtual int call(std::shared_ptr<LuaState> L) = 0;
  };

  template<typename RetVal, typename... Params>
  class LuaCallable_Implementation : public LuaCallable {
  public:
    LuaCallable_Implementation(std::function<RetVal(Params...)> function) :
      func(function) { }

    virtual int call(std::shared_ptr<LuaState> L){
      return call_helper(build_indices<sizeof...(Params)>(), L);
    }

  private:
    template<int... Indices>
    int call_helper(indices<Indices...>, std::shared_ptr<LuaState> L){
      if(lua_gettop(L->state()) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }
      RetVal output = func(L->Read<Params>(-Indices-1)...);
      L->Push(output);
      return 1;
    }

    std::function<RetVal(Params...)> func;
  };

  lua_State* L;
  std::vector<std::unique_ptr<LuaCallable> > cpp_functions;

  friend int call_cpp_function(lua_State*);
  static std::map<lua_State*, std::weak_ptr<LuaState> > all_states;
  static void RegisterCppState(std::shared_ptr<LuaState> state);
  static void DeregisterCppState(lua_State* c_state);
  static std::shared_ptr<LuaState> GetCppState(lua_State* c_state);
};

#endif /* _LUASTATE_H_ */
