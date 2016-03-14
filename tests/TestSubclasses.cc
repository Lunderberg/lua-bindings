#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

namespace {
  class BaseClass {
  public:
    int GetX() { return 1; }
  };

  class DerivedClass : public BaseClass {
  public:
    int GetY() { return 2; }
  };

  void accepts_base_class(BaseClass&) { }
  void accepts_derived_class(DerivedClass&) { }

  void InitializeClass(Lua::LuaState& L) {
    L.MakeClass<BaseClass>("BaseClass")
      .AddConstructor<>("BaseClass")
      .AddMethod("GetX",&BaseClass::GetX);

    L.MakeClass<DerivedClass, BaseClass>("DerivedClass")
      .AddConstructor<>("DerivedClass")
      .AddMethod("GetY",&DerivedClass::GetY);

    L.SetGlobal("accepts_base_class", accepts_base_class);
    L.SetGlobal("accepts_derived_class", accepts_derived_class);
  }
}

TEST(LuaSubclasses, CallBaseMethods) {
  Lua::LuaState L;
  InitializeClass(L);

  L.LoadString("base = BaseClass()");
  L.LoadString("derived = DerivedClass()");

  EXPECT_EQ(L.LoadString<int>("return base:GetX()"), 1);

  EXPECT_EQ(L.LoadString<int>("return base:GetX()"), 1);
  EXPECT_EQ(L.LoadString<int>("return derived:GetX()"), 1);

  EXPECT_THROW(L.LoadString("return base:GetY()"), LuaExecuteError);
  EXPECT_EQ(L.LoadString<int>("return derived:GetY()"), 2);
}

TEST(LuaSubclasses, PassToFunction) {
  Lua::LuaState L;
  InitializeClass(L);


  L.LoadString("base = BaseClass() "
               "derived = DerivedClass()");

  EXPECT_NO_THROW(L.LoadString("accepts_base_class(base)"));
  EXPECT_NO_THROW(L.LoadString("accepts_base_class(derived)"));
  EXPECT_NO_THROW(L.LoadString("accepts_derived_class(derived)"));

  EXPECT_THROW(L.LoadString("accepts_derived_class(base)"),
               LuaExecuteError);
}
