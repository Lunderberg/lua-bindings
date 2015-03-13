#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <string>

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

class TestClass{
public:
  TestClass() : x(0) { }
  int GetX(){ return x; }
  void SetX(int x){ this->x = x; }

  void PrintSelf(){ std::cout << "My x is " << x << "." << std::endl; }

private:
  int x;
};

int main(){
  LuaState L;
  L.LoadLibs();
  L.LoadFile("luascript.lua");

  cout << "** Execute a Lua function from C++" << endl;

  auto string_return = L.Call<std::string>("myluafunction", 5);
  cout << "The return value of the function was " << string_return << endl;

  L.Call("lua_func_with_params",5,3);

  L.SetGlobal("global_var", 5);
  L.Call("print_global");

  L.SetGlobal("double_number", double_number);
  L.SetGlobal("double_integer", double_integer);
  L.SetGlobal("sum_integers", sum_integers);
  L.SetGlobal("print_integer", print_integer);
  L.SetGlobal("print_hello", print_hello);
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

  MakeClass<TestClass>(L.state(), "TestClass")
    .AddConstructor<>("make_TestClass")
    .AddMethod("GetX", &TestClass::GetX)
    .AddMethod("SetX", &TestClass::SetX)
    .AddMethod("PrintSelf", &TestClass::PrintSelf);
  L.Call("test_classes");


  return 0;
}
