#ifndef _LUAPOINTERTYPE_H_
#define _LUAPOINTERTYPE_H_

#include <memory>

namespace Lua{
  enum class PointerType : char {
    shared_ptr,
      weak_ptr,
      c_ptr,
      };

  template<typename T>
  struct VariablePointer{
    PointerType type;
    union{
      std::shared_ptr<T> shared_ptr;
      std::weak_ptr<T> weak_ptr;
      T* c_ptr;
    } pointers;
    unsigned long reference_id;
  };
}

#endif /* _LUAPOINTERTYPE_H_ */
