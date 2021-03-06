#ifndef _LUAREGISTRYNAMES_H_
#define _LUAREGISTRYNAMES_H_

#include <sstream>
#include <string>

namespace Lua {
  extern const std::string cpp_function_registry_entry;

  extern const std::string upcaster_registry_entry;

  extern const std::string cpp_reference_counter;
  extern const std::string cpp_valid_reference_set;
  extern const std::string cpp_reference_set_metatable;

  extern const std::string keepalive_table;

  extern const std::string luastate_weakptr;
  extern const std::string luastate_weakptr_metatable;

  template<typename T>
  struct type_holder{ static void id(){ } };

  template<typename T>
  constexpr size_t type_id_safe(){
    return reinterpret_cast<size_t>(&type_holder<T>::id);
  }

  template<typename T, bool nonconst=false>
  struct class_registry_entry{
    static std::string get(){
      std::stringstream ss;
      if(nonconst){
        ss << "Lua.Class." << type_id_safe<typename std::remove_const<T>::type>();
      } else {
        ss << "Lua.Class." << type_id_safe<T>();
      }
      return ss.str();
    }
  };

  template<typename T, bool nonconst>
  struct class_registry_entry<T&, nonconst>{
    static std::string get(){
      return class_registry_entry<T,nonconst>::get();
    }
  };
}

#endif /* _LUAREGISTRYNAMES_H_ */
