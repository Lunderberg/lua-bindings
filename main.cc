#include <iostream>
#include <string>
#include <tuple>

#include <lua.hpp>

#include "LuaState.hh"
#include "LuaExceptions.hh"

double double_number(double x){
  return 2*x;
}

int double_integer(int x){
  return 2*x;
}

void print_integer(int x){
  std::cout << "In C++, integer is " << x << std::endl;
}

int sum_integers(int x, int y){
  return x+y;
}

void print_hello(){
  std::cout << "hello" << std::endl;
}

std::tuple<int, int> multiple_returns(){
  return std::make_tuple(5, 6);
}

class TestClass{
public:
  TestClass() : x(0) { }
  ~TestClass() {
    std::cout << "TestClass destructor called" << std::endl;
  }
  int GetX(){ return x; }
  void SetX(int x){ this->x = x; }

  void PrintSelf(){ std::cout << "My x is " << x << "." << std::endl; }

private:
  int x;
};

int main(){
  Lua::LuaState L;
  L.LoadLibs();

  L.MakeClass<TestClass>("TestClass")
    .AddConstructor<>("make_TestClass")
    .AddMethod("GetX", &TestClass::GetX)
    .AddMethod("SetX", &TestClass::SetX)
    .AddMethod("PrintSelf", &TestClass::PrintSelf);

  L.LoadFile("luascript.lua");

  std::cout << "** Execute a Lua function from C++" << std::endl;

  auto string_return = L.Call<std::string>("myluafunction", 5);
  std::cout << "The return value of the function was " << string_return << std::endl;

  L.Call("lua_func_with_params",5,3);

  L.SetGlobal("global_var", 5);
  L.Call("print_global");

  L.SetGlobal("double_number", double_number);
  L.SetGlobal("double_integer", double_integer);
  L.SetGlobal("sum_integers", sum_integers);
  L.SetGlobal("print_integer", print_integer);
  L.SetGlobal("print_hello", print_hello);
  L.SetGlobal("multiple_returns", multiple_returns);
  L.Call("test_cpp_functions");

  // Set a Lua table from inside C++
  auto table = L.NewTable();
  table["y"] = 5;
  table["x"] = "hello";
  L.SetGlobal("c_table", table);

  L.Call("read_table");

  // Read a Lua table from inside C++
  table = L.GetGlobal("lua_table");
  std::cout << "lua_table.a = " << table["a"].Cast<int>() << std::endl;
  std::cout << "lua_table.b = " << table["b"].Cast<std::string>() << std::endl;
  table.Pop();

  L.Call("test_classes");

  TestClass obj;
  obj.SetX(42);
  L.Call("accepts_testclass", obj);

  auto obj2 = L.Call<std::shared_ptr<TestClass> >("returns_testclass");
  std::cout << "It returned an object with " << obj2->GetX() << std::endl;
  obj2->SetX(77);
  L.Call("returns_testclass");

  auto output = L.Call<std::tuple<int,int,int,std::string> >("lua_multiple_returns");
  //std::cout << output << std::endl;
  std::cout << std::get<0>(output) << " " << std::get<1>(output) << " "
            << std::get<2>(output) << " " << std::get<3>(output) << std::endl;

  return 0;
}
