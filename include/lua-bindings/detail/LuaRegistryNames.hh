#ifndef _LUAREGISTRYNAMES_H_
#define _LUAREGISTRYNAMES_H_

#include <sstream>
#include <string>

extern const std::string cpp_function_registry_entry;

extern const std::string cpp_reference_counter;
extern const std::string cpp_valid_reference_set;
extern const std::string cpp_reference_set_metatable;

extern const std::string keepalive_table;

template<typename T>
struct type_holder{ static void id(){ } };

template<typename T>
constexpr size_t type_id_safe(){
  return reinterpret_cast<size_t>(&type_holder<T>::id);
}

template<typename T>
struct class_registry_entry{
  static constexpr std::string get(){
    std::stringstream ss;
    ss << "Lua.Class." << type_id_safe<T>();
    return ss.str();
  }
};

template<typename T>
struct class_registry_entry<T&>{
  static constexpr std::string get(){
    return class_registry_entry<T>::get();
  }
};

#endif /* _LUAREGISTRYNAMES_H_ */
