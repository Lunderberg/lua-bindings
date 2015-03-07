#ifndef _LUASTATE_H_
#define _LUASTATE_H_

#include <memory>
#include <utility>

#include <lua.hpp>

#include "LuaExceptions.hh"

class LuaState {
public:
  static std::shared_ptr<LuaState> create();
  ~LuaState();

  void LoadFile(const char* filename);
  void LoadLibs();

  template<typename T>
  void SetGlobal(const char* name, T t){
    Push(t);
    lua_setglobal(L, name);
  }

  template<typename return_type=void, typename... Params>
  return_type Call(const char* name, Params... params){
    lua_getglobal(L, name);
    PushMany(params...);
    int result = lua_pcall(L, sizeof...(params),
                           (std::is_same<return_type, void>::value ? 0 : 1),
                           0);
    if(result){
      auto error_message = Pop<std::string>();
      throw LuaFunctionExecuteError(error_message);
    }
    return Pop<return_type>();
  }



protected:
  LuaState();

private:
  template<typename FirstParam, typename... Params>
  void PushMany(FirstParam&& first, Params&&... params){
    Push(std::forward<FirstParam>(first));
    PushMany(std::forward<Params>(params)...);
  }

  template<typename LastParam>
  void PushMany(LastParam&& last){
    Push(std::forward<LastParam>(last));
  }

  template<typename T>
  void Push(T t);

  template<typename T>
  T Pop();

  lua_State* L;
};

#endif /* _LUASTATE_H_ */
