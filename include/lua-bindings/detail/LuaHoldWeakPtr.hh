#ifndef _LUAHOLDWEAKPTR_H_
#define _LUAHOLDWEAKPTR_H_

#include <memory>

#include <lua.hpp>

namespace Lua{
  void InitializeHeldWeakPtr(std::shared_ptr<lua_State> shared_L);
  std::shared_ptr<lua_State> ExtractSharedPtr(lua_State* L);
}

#endif /* _LUAHOLDWEAKPTR_H_ */
