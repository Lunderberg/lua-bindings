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
    int GetX() const { return x; }
    void SetX(int x){ this->x = x; }

  private:
    int x;
  };

  int reference_func(TestClass& var){
    return 3*var.GetX();
  }

  int const_reference_func(const TestClass& var){
    return 4*var.GetX();
  }

  int const_value_func(const TestClass var){
    return 5*var.GetX();
  }

  int value_func(TestClass var){
    return 6*var.GetX();
  }

  void InitializeClass(Lua::LuaState& L){
    L.MakeClass<TestClass>("TestClass")
      .AddConstructor<>("make_TestClass")
      .AddMethod("GetX", &TestClass::GetX)
      .AddMethod("SetX", &TestClass::SetX);
  }
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

TEST(LuaClasses, ConstGetter){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("function const_getter(var)"
               "  return var:GetX() "
               "end");

  TestClass myclass;
  EXPECT_EQ(L.Call<int>("const_getter",std::cref(myclass)), 0);
}

TEST(LuaClasses, GetterSetter_LoadSafeLibs){
  Lua::LuaState L;
  L.LoadSafeLibs();
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
    var.SetX(42);
    L.LoadString("function accepts_TestClass(var) "
                 "  return var:GetX() "
                 "end");
    auto x = L.Call<int>("accepts_TestClass", var);
    EXPECT_EQ(x, 42);
  }
  EXPECT_EQ(constructor_called, 2); // One explicit constructor plus one default copy constructor.
  EXPECT_EQ(destructor_called, 2);  // One destructor of C++ object plus one of Lua-held object.
}

TEST(LuaClasses, DestructorCount_PassReferenceToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    TestClass var;
    var.SetX(42);
    L.LoadString("function accepts_TestClass(var) "
                 "  var:SetX(17) "
                 "  return var:GetX() "
                 "end");
    auto x = L.Call<int>("accepts_TestClass", std::ref(var));
    EXPECT_EQ(x, 17);
    EXPECT_EQ(var.GetX(), 17);
  }
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}

TEST(LuaClasses, DestructorCount_PassSharedPointerToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    auto var = std::make_shared<TestClass>();
    var->SetX(42);
    L.LoadString("function accepts_TestClass(var) "
                 "  return var:GetX() "
                 "end ");
    auto x = L.Call<int>("accepts_TestClass", var);
    EXPECT_EQ(x, 42);
  }
  // Same object in both C++ and Lua, should only be con/destructed once.
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}

TEST(LuaClasses, DestructorCount_PassWeakPointerToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    auto var = std::make_shared<TestClass>();
    var->SetX(42);
    std::weak_ptr<TestClass> weak_var = var;
    L.LoadString("function accepts_TestClass(var) "
                 "  return var:GetX() "
                 "end ");
    auto x = L.Call<int>("accepts_TestClass", weak_var);
    EXPECT_EQ(x, 42);
  }
  // Same object in both C++ and Lua, should only be con/destructed once.
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}

TEST(LuaClasses, DestructorCount_PassCPointerToLua){
  constructor_called = 0;
  destructor_called = 0;
  {
    Lua::LuaState L;
    InitializeClass(L);
    TestClass var;
    var.SetX(42);
    L.LoadString("function accepts_TestClass(var) "
                 "  return var:GetX() "
                 "end ");
    auto x = L.Call<int>("accepts_TestClass", &var);
    EXPECT_EQ(x, 42);
  }
  // Same object in both C++ and Lua, should only be con/destructed once.
  EXPECT_EQ(constructor_called, 1);
  EXPECT_EQ(destructor_called, 1);
}

TEST(LuaClasses, ReturnCppClass){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("function return_TestClass() "
               "  var = make_TestClass() "
               "  var:SetX(17) "
               "  return var "
               "end ");
  auto value = L.Call<TestClass>("return_TestClass");
  EXPECT_EQ(value.GetX(), 17);

  auto shared_value = L.Call<std::shared_ptr<TestClass> >("return_TestClass");
  EXPECT_EQ(shared_value->GetX(), 17);
}

TEST(LuaClasses, ReturnCppClassByWeakPtr){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("x = 0 "
               "function set_global(var) "
               "  var:SetX(var:GetX() + 1) "
               "  x = var "
               "end "

               "function get_global() "
               "  return x "
               "end ");

  {
    auto value = std::make_shared<TestClass>();
    value->SetX(42);
    std::weak_ptr<TestClass> weak_value = value;
    L.Call("set_global", weak_value);

    EXPECT_EQ(value->GetX(), 43);

    auto output = L.Call<std::weak_ptr<TestClass> >("get_global");
    auto shared_output = L.Call<std::shared_ptr<TestClass> >("get_global");
  }
  auto output = L.Call<std::weak_ptr<TestClass> >("get_global");
  EXPECT_THROW(L.Call<std::shared_ptr<TestClass> >("get_global"),
               LuaExpiredWeakPointer);
}

TEST(LuaClasses, ReturnCppClassByCPtr){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("x = 0 "
               "function set_global(var) "
               "  var:SetX(var:GetX() + 1) "
               "  x = var "
               "end "

               "function get_global() "
               "  return x "
               "end ");

  TestClass var;
  var.SetX(42);
  L.Call("set_global", &var);

  EXPECT_EQ(var.GetX(), 43);

  auto output = L.Call<TestClass*>("get_global");
  EXPECT_EQ(output, &var);

  EXPECT_THROW(L.Call<std::weak_ptr<TestClass> >("get_global"),
               LuaIncorrectPointerType);

  EXPECT_THROW(L.Call<std::shared_ptr<TestClass> >("get_global"),
               LuaIncorrectPointerType);
}

TEST(LuaClasses, PassRefToCpp){
  Lua::LuaState L;
  InitializeClass(L);
  L.SetGlobal("reference_func", reference_func);

  auto res = L.LoadString<double>("var = make_TestClass() "
                                  "var:SetX(17) "
                                  "return reference_func(var) ");
  EXPECT_EQ(res, 51);
}

TEST(LuaClasses, PassConstRefToCpp){
  Lua::LuaState L;
  InitializeClass(L);
  L.SetGlobal("const_reference_func", const_reference_func);

  auto res = L.LoadString<double>("var = make_TestClass() "
                                  "var:SetX(17) "
                                  "return const_reference_func(var) ");
  EXPECT_EQ(res, 68);
}

TEST(LuaClasses, ReferenceExpires){
  Lua::LuaState L;
  InitializeClass(L);
  L.LoadString("saved_copy = nil "
               " "
               "function accepts_TestClass(var) "
               "  saved_copy = var "
               "  var:SetX(17) "
               "  return var:GetX() "
               "end "
               " "
               "function method_returns_nil() "
               "  return saved_copy:GetX()==nil "
               "end "
               " "
               "function returns_afterward() "
               "  return saved_copy "
               "end ");

  TestClass var;
  var.SetX(42);

  auto x = L.Call<int>("accepts_TestClass", std::ref(var));
  EXPECT_EQ(x, 17);
  EXPECT_EQ(var.GetX(), 17);

  EXPECT_TRUE(L.Call<bool>("method_returns_nil"));

  EXPECT_THROW(L.Call<TestClass>("returns_afterward"),
               LuaExpiredReference);
}

TEST(LuaClasses, PassConst){
  Lua::LuaState L;
  InitializeClass(L);

  L.LoadString("function accepts_TestClass(var) "
               "  var:SetX(17) "
               "  return var:GetX() "
               "end ");

  TestClass var;
  var.SetX(42);
  EXPECT_THROW(L.Call("accepts_TestClass", std::cref(var)),
               LuaExecuteError);
  EXPECT_EQ(var.GetX(), 42);
}



TEST(LuaClasses, PassConstValueToCpp){
  Lua::LuaState L;
  InitializeClass(L);
  L.SetGlobal("const_value_func", const_value_func);
  L.SetGlobal("value_func", value_func);

  L.LoadString("function calls_value_func(var) "
               "  value_func(var) "
               "end "
               "function calls_const_value_func(var) "
               "  const_value_func(var) "
               "end ");

  TestClass var;

  EXPECT_NO_THROW(L.Call("calls_value_func", std::ref(var)));
  EXPECT_NO_THROW(L.Call("calls_value_func", std::cref(var)));
  EXPECT_NO_THROW(L.Call("calls_const_value_func", std::ref(var)));
  EXPECT_NO_THROW(L.Call("calls_const_value_func", std::cref(var)));

  L.LoadString<const TestClass>("local args = {...} "
                                "return args[1] ",
                                std::cref(var));
}
