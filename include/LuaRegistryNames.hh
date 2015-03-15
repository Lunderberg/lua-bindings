#ifndef _LUAREGISTRYNAMES_H_
#define _LUAREGISTRYNAMES_H_

#include <sstream>
#include <string>

extern const std::string cpp_function_registry_entry;

template<typename T>
struct type_holder{ static void id(){ } };

template<typename T>
constexpr size_t type_id_safe(){
  return reinterpret_cast<size_t>(&type_holder<T>::id);
}

template<typename T>
constexpr std::string class_registry_entry(){
  std::stringstream ss;
  ss << "Lua.Class." << type_id_safe<T>();
  return ss.str();
}

#endif /* _LUAREGISTRYNAMES_H_ */