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

//#include "LuaCallable.hh"
#include "LuaDelayedPop.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"

int call_doubledouble(lua_State* L);

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

  void Push(std::function<double(double)> func){
    Push(cpp_functions.size());
    cpp_functions.push_back(std::unique_ptr<LuaCallable_DoubleDouble>(new LuaCallable_DoubleDouble(func)));
    lua_pushcclosure(L, call_doubledouble, 1);
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

  class LuaCallable_DoubleDouble : public LuaCallable {
  public:
    LuaCallable_DoubleDouble(std::function<double(double)> function) :
      func(function) { }

    virtual int call(std::shared_ptr<LuaState> L){
      assert(lua_gettop(L->state()) == 1);
      double argument = LuaObject(L->state(), -1).Cast<double>();
      double output = func(argument);
      L->Push(output);
      return 1;
    }

  private:
    std::function<double(double)> func;
  };

  lua_State* L;
  //std::vector<std::function<double(double)> > functions;
  std::vector<std::unique_ptr<LuaCallable> > cpp_functions;

  friend int call_doubledouble(lua_State*);
  static std::map<lua_State*, std::weak_ptr<LuaState> > all_states;
  static void RegisterCppState(std::shared_ptr<LuaState> state);
  static void DeregisterCppState(lua_State* c_state);
  static std::shared_ptr<LuaState> GetCppState(lua_State* c_state);
};

#endif /* _LUASTATE_H_ */
