#ifndef _LUAPOINTERTYPE_H_
#define _LUAPOINTERTYPE_H_

#include <iostream>
#include <memory>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaReferenceSet.hh"

namespace Lua{
  //! Base class for C++ objects being held by Lua.
  /*!
    This can be stored inside Lua userdata.
    The get_shared, get_weak, and get_c functions return a pointer of the requested type.
    If the pointer cannot be converted to the requested type,
      a LuaInvalidStackContents will be thrown.
   */
  template<typename T>
  class VariablePointer{
  public:
    virtual ~VariablePointer() { }


    virtual std::shared_ptr<T> get_shared() = 0;
    virtual std::weak_ptr<T> get_weak() = 0;
    virtual T* get_c(lua_State* L) = 0;

    //! Cleanup function for VariablePointer<T>
    /*!
      For use as the "__gc" function in a metatable.
      Assuming that the value being deleted is userdata that points to VariablePointer,
        call the destructor.
     */
    static int garbage_collect(lua_State* L) {
      void* storage = lua_touserdata(L, 1);
      VariablePointer* ptr = *static_cast<VariablePointer**>(storage);
      ptr->~VariablePointer();

      return 0;
    }
  };


  template<typename T>
  class VariableSharedPointer : public VariablePointer<T> {
  public:
    VariableSharedPointer(std::shared_ptr<T> shared)
      : ptr(shared) { }

    virtual std::shared_ptr<T> get_shared() { return ptr; }
    virtual std::weak_ptr<T> get_weak() { return ptr; }
    virtual T* get_c(lua_State*) { return ptr.get(); }

  private:
    std::shared_ptr<T> ptr;
  };

  template<typename T>
  class VariableWeakPointer : public VariablePointer<T> {
  public:
    VariableWeakPointer(std::weak_ptr<T> weak)
      : ptr(weak) { }

    virtual std::shared_ptr<T> get_shared() {
      auto output = ptr.lock();
      if(output) {
        return output;
      } else {
        throw LuaExpiredWeakPointer("Weak_ptr returned was no longer valid");
      }
    }

    virtual std::weak_ptr<T> get_weak() { return ptr; }
    virtual T* get_c(lua_State*) { return get_shared().get(); }

  private:
    std::weak_ptr<T> ptr;
  };

  template<typename T>
  class VariableCPointer : public VariablePointer<T> {
  public:
    VariableCPointer(T* cptr, unsigned long reference_id)
      : ptr(cptr), reference_id(reference_id) { }

    virtual std::shared_ptr<T> get_shared() {
      throw LuaIncorrectPointerType("Cannot convert C-style pointer to shared_ptr");
    }

    virtual std::weak_ptr<T> get_weak() {
      throw LuaIncorrectPointerType("Cannot convert C-style pointer to weak_ptr");
    }

    virtual T* get_c(lua_State* L) {
      if(IsValidReference(L, reference_id)) {
        return ptr;
      } else {
        throw LuaExpiredReference("C++ reference was no longer valid");
      }
    }

  private:
    T* ptr;
    unsigned long reference_id;
  };
}

#endif /* _LUAPOINTERTYPE_H_ */
