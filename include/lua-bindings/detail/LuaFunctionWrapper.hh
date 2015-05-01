#ifndef _LUAFUNCTIONWRAPPER_H_
#define _LUAFUNCTIONWRAPPER_H_

#include <iostream>
#include <memory>

#include <lua.hpp>

namespace Lua {
  class FunctionWrapper{
  public:
    FunctionWrapper(std::shared_ptr<lua_State> shared_L, int index);
    ~FunctionWrapper();

    FunctionWrapper(const FunctionWrapper& other) = delete;
    FunctionWrapper(FunctionWrapper&& other);
    FunctionWrapper& operator=(const FunctionWrapper& other) = delete;

    template<typename RetVal=void, typename... Params>
    RetVal Call(Params&&... params);
  private:
    std::shared_ptr<lua_State> shared_L;
    int reference;
  };
}

#include "LuaCallFromStack.hh"
#include "LuaKeepAlive.hh"

namespace Lua{
  template<typename RetVal, typename... Params>
  RetVal FunctionWrapper::Call(Params&&... params){
    lua_State* L = shared_L.get();
    PushLivingToStack(L, reference);
    return Lua::CallFromStack<RetVal>(shared_L.get(), std::forward<Params>(params)...);
  }
}

#endif /* _LUAFUNCTIONWRAPPER_H_ */
