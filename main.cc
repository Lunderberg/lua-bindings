#include <iostream>
#include <string>
#include <tuple>

#include "lua-bindings/LuaState.hh"

// int main(){
//   Lua::LuaState L;

//   L.LoadSafeLibs();
//   //L.LoadLibs();

//   L.LoadString("a = {'one', 'two', 'three'} "
//                "for i,v in ipairs(a) do "
//                "  print(i,v) "
//                "end ");

//   L.LoadString("print(os.time())");

//   return 0;
// }

class TestClass{
  public:
    TestClass() : x(0) {
    }
    TestClass(const TestClass& other) : x(other.x) {
    }
    ~TestClass() {
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

int main(){
  Lua::LuaState L;
  L.MakeClass<TestClass>("TestClass")
    .AddConstructor<>("make_TestClass")
    .AddMethod("GetX", &TestClass::GetX)
    .AddMethod("SetX", &TestClass::SetX);
  L.SetGlobal("const_reference_func", const_reference_func);
  L.SetGlobal("reference_func", reference_func);

  int res = L.LoadString<int>("var = make_TestClass() "
                              "var:SetX(17) "
                              "return const_reference_func(var) ");
  std::cout << res << std::endl;
}
