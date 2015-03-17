#ifndef _LUAREAD_H_
#define _LUAREAD_H_

#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaRegistryNames.hh"
#include "LuaTableReference.hh"
#include "TemplateUtils.hh"

namespace Lua{

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, T>::type
  ReadDirect(lua_State* L, int index){
    int success;
    lua_Number output = lua_tonumberx(L, index, &success);
    // // The check of whether the value was indeed a number can give false negatives.
    // // For now, disabling the check.
    // // The check yields false at times when converting a luaTable to a vector<int>
    // if(!success){
    //   throw LuaInvalidStackContents("Lua value could not be converted to number");
    // }
    return output;
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
  ReadDirect(lua_State* L, int index){
    if(lua_isstring(L, index)){
      return lua_tostring(L, index);
    } else {
      throw LuaInvalidStackContents("Lua value could not be converted to string");
    }
  }

  template<typename T>
  struct ReadDefaultType{
    static T Read(lua_State* L, int index){
      auto obj = ReadDefaultType<std::shared_ptr<T> >::Read(L, index);
      return *obj;
    }
  };

  template<typename T>
  struct ReadDefaultType<std::shared_ptr<T> >{
    static std::shared_ptr<T> Read(lua_State* L, int index){
      if(!lua_isuserdata(L, index)){
        throw LuaInvalidStackContents("Value was not userdata");
      }

      void* storage = luaL_testudata(L, index, class_registry_entry<T>().c_str());

      if(!storage){
        throw LuaInvalidStackContents("Value could not be converted to requested type.");
      }

      std::shared_ptr<T> obj = *static_cast<std::shared_ptr<T>*>(storage);
      return obj;
    }
  };

  template<typename... Params>
  struct ReadDefaultType<std::tuple<Params...> >{
    static std::tuple<Params...> Read(lua_State* L, int index){
      return Read_Helper(L, index, build_indices<sizeof...(Params)>() );
    }
    template<int... Indices>
    static std::tuple<Params...> Read_Helper(lua_State* L, int index, indices<Indices...>){
      return std::make_tuple(Lua::Read<Params>(L, index+Indices)...);
    }
  };

  template<typename T>
  struct ReadDefaultType<std::vector<T> >{
    static std::vector<T> Read(lua_State* L, int index){
      Lua::LuaObject table(L, index);
      std::vector<T> output;
      int table_size = table.Length();

      for(int i=0; i<table_size; i++){
        auto value = table[i].Cast<T>();
        output.push_back(value);
      }
      return output;
    }
  };

  template<typename T>
  T ReadDirectIfPossible(lua_State* L, int index, int){
    return ReadDefaultType<T>::Read(L, index);
  }

  template<typename T>
  auto ReadDirectIfPossible(lua_State* L, int index, bool)
    -> decltype(ReadDirect<T>(L, index)) {
    return ReadDirect<T>(L, index);
  }

  template<typename T>
  T Read(lua_State* L, int index){
    return ReadDirectIfPossible<T>(L, index, true);
  }

}

#endif /* _LUAREAD_H_ */
