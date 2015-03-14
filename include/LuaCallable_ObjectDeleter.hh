#ifndef _LUACALLABLE_OBJECTDELETER_H_
#define _LUACALLABLE_OBJECTDELETER_H_

#include <lua.hpp>

#include "LuaCallable.hh"

namespace Lua{
  template<typename ClassType>
  class LuaCallable_ObjectDeleter : public LuaCallable {
  public:
    LuaCallable_ObjectDeleter() { }
    virtual int call(lua_State* L){
      void* storage = lua_touserdata(L, 1);
      ClassType* obj = *reinterpret_cast<ClassType**>(storage);
      delete obj;
      return 0;
    }
  };
}

#endif /* _LUACALLABLE_OBJECTDELETER_H_ */
