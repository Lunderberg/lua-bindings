#ifndef _LUAPOINTERTYPE_H_
#define _LUAPOINTERTYPE_H_

#include <iostream>
#include <memory>
#include <type_traits>
#include <vector>

#include <lua.hpp>

#include "LuaExceptions.hh"
#include "LuaReferenceSet.hh"

namespace Lua{
  template<typename T, typename U>
  std::weak_ptr<T> weak_pointer_cast(const std::weak_ptr<U>& weak) {
    std::shared_ptr<U> shared = weak.lock();
    return std::static_pointer_cast<T>(std::move(shared));
  }

  //! Base class for C++ objects being held by Lua.
  /*!
    This can be stored inside Lua userdata.
    The get_shared, get_weak, and get_c functions return a pointer of the requested type.
    If the pointer cannot be converted to the requested type,
      a LuaInvalidStackContents will be thrown.
   */
  class HeldPointer {
  public:
    virtual ~HeldPointer() { }

    virtual std::shared_ptr<void> get_shared() = 0;
    virtual std::weak_ptr<void> get_weak() = 0;
    virtual void* get_c(lua_State* L) = 0;

    static int garbage_collect(lua_State* L) {
      void* storage = lua_touserdata(L, 1);
      HeldPointer* ptr = *static_cast<HeldPointer**>(storage);
      ptr->~HeldPointer();

      return 0;
    }
  };

  template<typename T>
  class VariableSharedPointer : public HeldPointer {
  public:
    VariableSharedPointer(std::shared_ptr<T> shared)
      : ptr(shared) { }

    virtual std::shared_ptr<void> get_shared() { return ptr; }
    virtual std::weak_ptr<void> get_weak() { return ptr; }
    virtual void* get_c(lua_State*) { return ptr.get(); }

  private:
    std::shared_ptr<T> ptr;
  };

  template<typename T>
  class VariableWeakPointer : public HeldPointer {
  public:
    VariableWeakPointer(std::weak_ptr<T> weak)
      : ptr(weak) { }

    virtual std::shared_ptr<void> get_shared() {
      auto output = ptr.lock();
      if(output) {
        return output;
      } else {
        throw LuaExpiredWeakPointer("Weak_ptr returned was no longer valid");
      }
    }

    virtual std::weak_ptr<void> get_weak() { return ptr; }
    virtual void* get_c(lua_State*) { return get_shared().get(); }

  private:
    std::weak_ptr<T> ptr;
  };

  template<typename T>
  class VariableCPointer : public HeldPointer {
  public:
    VariableCPointer(T* cptr, unsigned long reference_id)
      : ptr(cptr), reference_id(reference_id) { }

    virtual std::shared_ptr<void> get_shared() {
      throw LuaIncorrectPointerType("Cannot convert C-style pointer to shared_ptr");
    }

    virtual std::weak_ptr<void> get_weak() {
      throw LuaIncorrectPointerType("Cannot convert C-style pointer to weak_ptr");
    }

    virtual void* get_c(lua_State* L) {
      if(IsValidReference(L, reference_id)) {
        return const_cast<typename std::remove_const<T>::type*>(ptr);
      } else {
        throw LuaExpiredReference("C++ reference was no longer valid");
      }
    }

  private:
    T* ptr;
    unsigned long reference_id;
  };

  //! Upcasts from a particular class to base class.
  /*!
    Class designed to be stored for each class known by the LuaState.
    At the time of Lua::Read, the derived class is not known.
    This allows Lua::Read to upcast until it reaches the base class.
   */
  class Upcaster {
  public:
    virtual ~Upcaster() { }

    virtual std::shared_ptr<void> upcast(std::shared_ptr<void> derived_void) = 0;
    virtual std::weak_ptr<void> upcast(std::weak_ptr<void> derived_void) = 0;
    virtual void* upcast(void* derived_void) = 0;
  };

  int garbage_collect_upcaster(lua_State* L);

  template<typename Base, typename Derived>
  class Upcaster_Impl : public Upcaster {
  public:
    virtual std::shared_ptr<void> upcast(std::shared_ptr<void> derived_void) {
      std::shared_ptr<Derived> derived = std::static_pointer_cast<Derived>(derived_void);
      std::shared_ptr<Base> base = derived;
      return std::shared_ptr<void>(base);
    }

    virtual std::weak_ptr<void> upcast(std::weak_ptr<void> derived_void) {
      std::shared_ptr<void> shared_void = derived_void.lock();
      return upcast(shared_void);
    }

    virtual void* upcast(void* derived_void) {
      Derived* derived = static_cast<Derived*>(derived_void);
      Base* base = derived;
      return base;
    }
  };

  class PointerAccess {
  public:
    PointerAccess(HeldPointer* p, std::vector<Upcaster*> upcasters)
      : p(p), upcasters(upcasters) { }

    std::shared_ptr<void> get_shared() {
      auto output = p->get_shared();
      for(auto upcaster : upcasters) {
        output = upcaster->upcast(output);
      }
      return output;
    }

    std::weak_ptr<void> get_weak() {
      auto output = p->get_weak();
      for(auto upcaster : upcasters) {
        output = upcaster->upcast(output);
      }
      return output;
    }

    void* get_c(lua_State* L) {
      auto output = p->get_c(L);
      for(auto upcaster : upcasters) {
        output = upcaster->upcast(output);
      }
      return output;
    }

  private:
    HeldPointer* p;
    std::vector<Upcaster*> upcasters;
  };
}

#endif /* _LUAPOINTERTYPE_H_ */
