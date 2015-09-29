#ifndef _LUAREAD_H_
#define _LUAREAD_H_

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaHoldWeakPtr.hh"
#include "LuaObject.hh"
#include "LuaPointerType.hh"
#include "LuaPush.hh"
#include "LuaReferenceSet.hh"
#include "LuaRegistryNames.hh"
#include "LuaTableReference.hh"
#include "TemplateUtils.hh"

namespace Lua{

  //! Read the type requested, without any further dispatch.
  /*! Each ReadDirect method reads from the stack in the given type,
      without any additional dispatch to methods.
   */
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value &&
                          !std::is_same<T, bool>::value, T>::type
    ReadDirect(lua_State* L, int index);

  template<typename T>
  typename std::enable_if<std::is_same<T, std::string>::value, T>::type
    ReadDirect(lua_State* L, int index);

  template<typename T>
  typename std::enable_if<std::is_same<T, bool>::value, T>::type
    ReadDirect(lua_State* L, int index);

  template<typename T>
  typename std::enable_if<std::is_same<T, void*>::value, T>::type
    ReadDirect(lua_State* L, int index);

  //! Helper method, for grabbing a pointer from the stack.
  template<typename T>
    VariablePointer<T>* ReadVariablePointer(lua_State* L, int index);

  //! Read from the stack, by value.
  template<typename T, bool allow_references>
  struct ReadDefaultType{
    static T Read(lua_State* L, int index);
  };

  //! Read from the stack, by value.
  template<typename T, bool allow_references>
  struct ReadDefaultType<const T, allow_references>{
    static T Read(lua_State* L, int index);
  };

  //! Read from the stack, by shared_ptr.
  template<typename T, bool allow_references>
  struct ReadDefaultType<std::shared_ptr<T>, allow_references >{
    static std::shared_ptr<T> Read(lua_State* L, int index);
  };

  //! Read from the stack, by weak_ptr
  template<typename T, bool allow_references>
  struct ReadDefaultType<std::weak_ptr<T>, allow_references >{
    static std::weak_ptr<T> Read(lua_State* L, int index);
  };

  //! Read from the stack, by c-style pointer.
  template<typename T, bool allow_references>
  struct ReadDefaultType<T*, allow_references>{
    static T* Read(lua_State* L, int index);
  };

  template<typename T, bool allow_references>
  struct ReadDefaultType<T&, allow_references>{
    static T& Read(lua_State* L, int index);
  };

  //! Read from the stack as a tuple.
  /*! This assumes that the values to be stored in the tuple start at "index",
      and proceed in order.
   */
  template<typename... Params, bool allow_references>
  struct ReadDefaultType<std::tuple<Params...>, allow_references >{
    static std::tuple<Params...> Read(lua_State* L, int index);

    template<int... Indices>
    static std::tuple<Params...> Read_Helper(lua_State* L, int index, indices<Indices...>);
  };

  //! Read as a std::vector<T>
  /*! This assumes that the lua value at "index" is a table with only integer keys.
    Assumes that each value stored in the table can be converted to T.
   */
  template<typename T, bool allow_references>
  struct ReadDefaultType<std::vector<T>, allow_references >{
    static std::vector<T> Read(lua_State* L, int index);
  };

  //! Read as a std::map<std::string, T>
  /*! This assumes that the lua value at "index" is a table with only string keys.
    Assumes that each value stored in the table can be converted to T.
   */
  template<typename T, bool allow_references>
  struct ReadDefaultType<std::map<std::string, T>, allow_references >{
    static std::map<std::string, T> Read(lua_State* L, int index);
  };

  template<typename RetVal, typename... Params, bool allow_references>
  struct ReadDefaultType<std::function<RetVal(Params...)>, allow_references >{
    static std::function<RetVal(Params...)> Read(lua_State* L, int index);
  };

  template<typename T, bool allow_references>
    T ReadDirectIfPossible(lua_State* L, int index, int);

  template<typename T, bool allow_references>
  auto ReadDirectIfPossible(lua_State* L, int index, bool)
    -> decltype(ReadDirect<T>(L, index));

  //! Reads any value from the lua stack
  /*! First, calls ReadDirectIfPossible.
    The auto -> decltype() construction ensures that ReadDirect<T>() is used if it is valid.
    Otherwise, ReadDefaultType<T>::Read() is used, which goes to a number of partial specializations.
   */
  template<typename T, bool allow_references = false>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
    Read(lua_State* L, int index);

  //! Empty functions, needed for some templates.
  template<typename T, bool allow_references = false>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
    Read(lua_State*, int);
}

#include "LuaFunctionWrapper.hh"

namespace Lua{

  //! Read the type requested, without any further dispatch.
  /*! Each ReadDirect method reads from the stack in the given type,
      without any additional dispatch to methods.
   */
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value &&
                          !std::is_same<T, bool>::value, T>::type
  ReadDirect(lua_State* L, int index){
    int success;
    lua_Number output = lua_tonumberx(L, index, &success);
    if(!success){
      throw LuaInvalidStackContents("Lua value could not be converted to number");
    }
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
  typename std::enable_if<std::is_same<T, void*>::value, T>::type
  ReadDirect(lua_State* L, int index){
    return lua_touserdata(L, index);
  }

  //! Helper method, for grabbing a pointer from the stack.
  template<typename T>
  VariablePointer<T>* ReadVariablePointer(lua_State* L, int index){
    if(!lua_isuserdata(L, index)){
      throw LuaInvalidStackContents("Value was not userdata");
    }

    void* storage = luaL_testudata(L, index, class_registry_entry<T>::get().c_str());

    // If 'const' is requested, can also return a non-const version.
    // This will not, however, cause the non-const request to return a const version.
    // The std::is_same check is not required, but allows this conditional
    //   to be optimized out when requesting a non-const member.
    if(!std::is_same<T, typename std::remove_const<T>::type>::value &&
       !storage){
      storage = luaL_testudata(L, index,
                               class_registry_entry<typename std::remove_const<T>::type>::get().c_str());
    }

    if(!storage){
      throw LuaInvalidStackContents("Value could not be converted to requested type.");
    }

    return static_cast<VariablePointer<T>*>(storage);
  }

  //! Read from the stack, by value.
  template<typename T, bool allow_references>
  T ReadDefaultType<T, allow_references>::Read(lua_State* L, int index){
    return ReadDefaultType<const T, allow_references>::Read(L, index);
  }

  //! Read from the stack, by value.
  template<typename T, bool allow_references>
  T ReadDefaultType<const T, allow_references>::Read(lua_State* L, int index){
    return *ReadDefaultType<const T*, allow_references>::Read(L, index);
  }

  //! Read from the stack, by shared_ptr.
  template<typename T, bool allow_references>
  std::shared_ptr<T> ReadDefaultType<std::shared_ptr<T>, allow_references>::Read(lua_State* L, int index){
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

  //! Read from the stack, by weak_ptr
  template<typename T, bool allow_references>
  std::weak_ptr<T> ReadDefaultType<std::weak_ptr<T>, allow_references >::Read(lua_State* L, int index){
    auto ptr = ReadVariablePointer<T>(L, index);
    if(ptr->type == PointerType::weak_ptr){
      return ptr->pointers.weak_ptr;
    } else if (ptr->type == PointerType::shared_ptr){
      return ptr->pointers.shared_ptr;
    } else {
      throw LuaIncorrectPointerType("Variable requested was not convertible to a weak_ptr");
    }

  }

  //! Read from the stack, by c-style pointer.
  template<typename T, bool allow_references>
  T* ReadDefaultType<T*, allow_references>::Read(lua_State* L, int index){
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
        throw LuaExpiredWeakPointer("Weak_ptr returned was no longer valid.");
      }
    }

    case PointerType::c_ptr:
    {
      if(IsValidReference(L, ptr->reference_id)){
        return ptr->pointers.c_ptr;
      } else {
        throw LuaExpiredReference("C++ reference was no longer valid");
      }
    }

    default:
      // Should never reach here.
      assert(false);
    }
  }

  template<typename T, bool allow_references>
  T& ReadDefaultType<T&, allow_references>::Read(lua_State* L, int index){
    static_assert(allow_references, "Unsafe to read as a reference here");
    T* ptr = ReadDefaultType<T*, allow_references>::Read(L, index);
    return *ptr;
  }

  //! Read from the stack as a tuple.
  /*! This assumes that the values to be stored in the tuple start at "index",
      and proceed in order.
   */
  template<typename... Params, bool allow_references>
  std::tuple<Params...> ReadDefaultType<std::tuple<Params...>, allow_references >
  ::Read(lua_State* L, int index){
    return Read_Helper(L, index, build_indices<sizeof...(Params)>() );
  }

  template<typename... Params, bool allow_references>
  template<int... Indices>
  std::tuple<Params...> ReadDefaultType<std::tuple<Params...>, allow_references >
  ::Read_Helper(lua_State* L, int index, indices<Indices...>){
    return std::make_tuple(Lua::Read<Params>(L, index+Indices)...);
  }

  //! Read as a std::vector<T>
  /*! This assumes that the lua value at "index" is a table with only integer keys.
    Assumes that each value stored in the table can be converted to T.
   */
  template<typename T, bool allow_references>
  std::vector<T> ReadDefaultType<std::vector<T>, allow_references >::Read(lua_State* L, int index){
    std::vector<T> output;

    Lua::LuaObject table(L, index);
    int table_size = table.Length();

    for(int i=0; i<table_size; i++){
      output.push_back(table[i+1].Cast<T>());
    }
    return output;
  }

  //! Read as a std::map<std::string, T>
  /*! This assumes that the lua value at "index" is a table with only string keys.
    Assumes that each value stored in the table can be converted to T.
   */
  template<typename T, bool allow_references>
  std::map<std::string, T> ReadDefaultType<std::map<std::string, T>, allow_references >
  ::Read(lua_State* L, int index){
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

  template<typename RetVal, typename... Params, bool allow_references>
  std::function<RetVal(Params...)> ReadDefaultType<std::function<RetVal(Params...)>, allow_references >
  ::Read(lua_State* L, int index){
    // If the value is a std::function previously exposed by C++,
    //   return it directly.
    void* as_userdata = lua_touserdata(L, index);
    if(as_userdata){
      Lua::LuaCallable* callable = *static_cast<Lua::LuaCallable**>(as_userdata);
      auto callable_cppfunction = dynamic_cast<LuaCallable_CppFunction<RetVal(Params...)>* >(callable);
      if(callable_cppfunction){
        return callable_cppfunction->GetFunc();
      }
    }

    // Otherwise, wrap the Lua function to be called.
    auto wrapper = std::make_shared<Lua::FunctionWrapper>(ExtractSharedPtr(L), index);
    return [wrapper](Params&&... params){
      return wrapper->Call<RetVal>(std::forward<Params>(params)...);
    };
  }

  template<typename T, bool allow_references>
  T ReadDirectIfPossible(lua_State* L, int index, int){
    return ReadDefaultType<T, allow_references>::Read(L, index);
  }

  template<typename T, bool allow_references>
  auto ReadDirectIfPossible(lua_State* L, int index, bool)
    -> decltype(ReadDirect<T>(L, index)) {
    return ReadDirect<T>(L, index);
  }

  //! Reads any value from the lua stack
  /*! First, calls ReadDirectIfPossible.
    The auto -> decltype() construction ensures that ReadDirect<T>() is used if it is valid.
    Otherwise, ReadDefaultType<T>::Read() is used, which goes to a number of partial specializations.
   */
  template<typename T, bool allow_references = false>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  Read(lua_State* L, int index){
    return ReadDirectIfPossible<T, allow_references>(L, index, true);
  }

  //! Empty functions, needed for some templates.
  template<typename T, bool allow_references = false>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  Read(lua_State*, int) { }

}

#endif /* _LUAREAD_H_ */
