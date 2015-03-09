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

int sum_integers(int x, int y){
  return x+y;
}

int cppfunction(lua_State* L){
  double arg = luaL_checknumber(L,1);
  lua_pushnumber(L, arg*0.5);
  return 1;
}

int main(){
  auto L = LuaState::create();
  L->LoadLibs();
  L->LoadFile("luascript.lua");

  cout << "** Execute a Lua function from C++" << endl;

  auto string_return = L->Call<std::string>("myluafunction", 5);
  cout << "The return value of the function was " << string_return << endl;

  L->Call("lua_func_with_params",5,3);

  cout << "** Execute a C++ function from Lua" << endl;
  cout << "**** First register the function in Lua" << endl;
  L->SetGlobal("cppfunction", cppfunction);

  cout << "**** Call a Lua function that uses the C++ function" << endl;
  auto num_return = L->Call<double>("myfunction", 5);
  cout << "The return value of the function was " << num_return << endl;

  L->SetGlobal("global_var", 5);
  L->Call("print_global");

  L->SetGlobal("double_number", double_number);
  L->SetGlobal("double_integer", double_integer);
  L->SetGlobal("sum_integers", sum_integers);
  L->Call("test_double_number");


  return 0;
}
