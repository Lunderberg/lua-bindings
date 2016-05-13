#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

namespace{
  class InnerClass {
  public:
    InnerClass() : x(0) { }

    int GetX() { return x; }
    void SetX(int x) { this->x = x; }

  private:
    int x;
  };

  class OuterClass {
  public:
    InnerClass& GetInner() { return inner; }

  private:
    InnerClass inner;
  };
}

TEST(LuaClasses, ReturnReference){
  Lua::LuaState L;
  L.MakeClass<InnerClass>("InnerClass")
    .AddMethod("SetX",&InnerClass::SetX)
    .AddMethod("GetX",&InnerClass::GetX);

  L.MakeClass<OuterClass>("OuterClass")
    .AddMethod("GetInner", &OuterClass::GetInner);


  L.LoadLibs();
  L.LoadString("function modify_reference(outer) "
               "  local inner = outer:GetInner() "
               "  inner:SetX(42) "
               "  print(inner:GetX()) "
               "  print(outer:GetInner():GetX()) "
               "end");

  OuterClass outer;
  L.Call("modify_reference", std::ref(outer));
  EXPECT_EQ(42, outer.GetInner().GetX());
}
