#ifndef _LUACALLABLE_OBJECTCONSTRUCTOR_H_
#define _LUACALLABLE_OBJECTCONSTRUCTOR_H_

#include <cstring>
#include <memory>
#include <string>

#include <lua.hpp>

#include "LuaCallable.hh"
#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaRegistryNames.hh"
#include "TemplateUtils.hh"

namespace Lua{
  template<typename T>
  class LuaCallable_ObjectConstructor;

  template<typename ClassType, typename... Params>
  class LuaCallable_ObjectConstructor<ClassType(Params...)> : public LuaCallable {
  public:
    LuaCallable_ObjectConstructor() { }
    virtual int call(lua_State* L){
      return call_constructor_helper(build_indices<sizeof...(Params)>(), L);
    }

  private:
    template<int... Indices>
    static int call_constructor_helper(indices<Indices...>, lua_State* L){
      if(lua_gettop(L) != sizeof...(Params)){
        throw LuaCppCallError("Incorrect number of arguments passed");
      }

      int memsize = sizeof(VariablePointer<ClassType>);
      void* storage = lua_newuserdata(L, memsize);
      std::memset(storage, 0, memsize);


      auto ptr = static_cast<VariablePointer<ClassType>*>(storage);

      auto object = std::make_shared<ClassType>(Read<Params>(L, Indices+1)...);
      ptr->pointers.shared_ptr = object;

      luaL_newmetatable(L, class_registry_entry<ClassType>::get().c_str());
      lua_setmetatable(L, -2);

      return 1;
    }
  };
}

#endif /* _LUACALLABLE_OBJECTCONSTRUCTOR_H_ */
