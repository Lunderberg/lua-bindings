#ifndef _LUAREAD_H_
#define _LUAREAD_H_

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"
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
  typename std::enable_if<std::is_same<T, bool>::value, T>::type
  ReadDirect(lua_State* L, int index){
    return lua_toboolean(L, index);
  }

  template<typename T>
  VariablePointer<T>* ReadVariablePointer(lua_State* L, int index){
    if(!lua_isuserdata(L, index)){
      throw LuaInvalidStackContents("Value was not userdata");
    }

    void* storage = luaL_testudata(L, index, class_registry_entry<T>().c_str());

    if(!storage){
      throw LuaInvalidStackContents("Value could not be converted to requested type.");
    }

    return static_cast<VariablePointer<T>*>(storage);
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
      auto ptr = ReadVariablePointer<T>(L, index);
      if(ptr->type == PointerType::shared_ptr){
        return ptr->pointers.shared_ptr;

      } else if(ptr->type == PointerType::weak_ptr) {
        auto output = ptr->pointers.weak_ptr.lock();
        if(output){
          return output;
        } else {
          throw LuaExpiredWeakPointer("Weak_ptr returned was no longer valid");
        }

      } else {
        throw LuaIncorrectPointerType("Variable requested was not a shared_ptr");
      }

    }
  };

  template<typename T>
  struct ReadDefaultType<std::weak_ptr<T> >{
    static std::weak_ptr<T> Read(lua_State* L, int index){
      auto ptr = ReadVariablePointer<T>(L, index);
      if(ptr->type != PointerType::weak_ptr){
        throw LuaIncorrectPointerType("Variable requested was not a weak_ptr");
      }
      return ptr->pointers.weak_ptr;
    }
  };

  template<typename T>
  struct ReadDefaultType<T*>{
    static T* Read(lua_State* L, int index){
      auto ptr = ReadVariablePointer<T>(L, index);
      switch(ptr->type){
      case PointerType::shared_ptr:
        return ptr->pointers.shared_ptr.get();

      case PointerType::weak_ptr:
        {
          auto lock = ptr->pointers.weak_ptr.lock();
          if(lock){
            return lock.get();
          } else {
            throw LuaIncorrectPointerType("Variable requested was not a weak_ptr");
          }
        }

      case PointerType::c_ptr:
        return ptr->pointers.c_ptr;

      default:
        // Should never reach here.
        assert(false);
      }
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
      std::vector<T> output;

      Lua::LuaObject table(L, index);
      int table_size = table.Length();

      for(int i=0; i<table_size; i++){
        output.push_back(table[i+1].Cast<T>());
      }
      return output;
    }
  };

  template<typename T>
  struct ReadDefaultType<std::map<std::string, T> >{
    static std::map<std::string, T> Read(lua_State* L, int index){
      index = lua_absindex(L, index); // In case of negative index
      std::map<std::string, T> output;

      Lua::LuaObject table(L, index);

      lua_pushnil(L);
      LuaDelayedPop delay(L, 1);
      while(lua_next(L, index) ){
        LuaDelayedPop delay(L, 1);
        std::string key = Lua::Read<std::string>(L, -2);
        T value = Lua::Read<T>(L, -1);
        output[key] = value;
        lua_pop(L, 2); // Reading a string may modify its contents.  Therefore, replacing it.
        delay.SetNumPop(0);
        Lua::Push(L, key);
      }
      // The last call of lua_next pushes nothing to the stack.
      // We needed the safe-guard as any of the Lua::Read calls could throw,
      //   but now it must be disabled.
      delay.SetNumPop(0);
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
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index){
    return ReadDirectIfPossible<T>(L, index, true);
  }

  template<typename T>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read(lua_State*, int) { }

}

#endif /* _LUAREAD_H_ */
