#include <memory>

#include <gtest/gtest.h>

#include "lua-bindings/LuaState.hh"

namespace{
  int constructor_called = 0;
  int destructor_called = 0;

  class TestClass{
  public:
    TestClass() : x(0) {
      constructor_called++;
    }
    TestClass(const TestClass& other) : x(other.x) {
      constructor_called++;
    }
    ~TestClass() {
      destructor_called++;
    }
    int GetX(){ return x; }
    void SetX(int x){ this->x = x; }

  private:
    int x;
  };
}

void InitializeClass(Lua::LuaState& L){
  L.MakeClass<TestClass>("TestClass")
    .AddConstructor<>("make_TestClass")
    .AddMethod("GetX", &TestClass::GetX)
    .AddMethod("SetX", &TestClass::SetX);
}

TEST(LuaClasses, GetterSetter){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("function getter_setter()"
               "  local var = make_TestClass()"
               "  var:SetX(17)"
               "  return var:GetX()"
               "end");
  EXPECT_EQ(L.Call<int>("getter_setter"), 17);
}

TEST(LuaClasses, DestructorCount_MakeInLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    L.LoadString("var = make_TestClass()");
  }
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}


TEST(LuaClasses, DestructorCount_PassValueToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    TestClass var;
    L.LoadString("function accepts_TestClass(x) end");
    L.Call("accepts_TestClass", var);
  }
  EXPECT_EQ(constructor_called, 2); // One explicit constructor plus one default copy constructor.
  EXPECT_EQ(destructor_called, 2);  // One destructor of C++ object plus one of Lua-held object.
}

TEST(LuaClasses, DestructorCount_PassSharedPointerToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    auto var = std::make_shared<TestClass>();
    L.LoadString("function accepts_TestClass(x) end");
    L.Call("accepts_TestClass", var);
  }
  // Same object in both C++ and Lua, should only be con/destructed once.
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}
