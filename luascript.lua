print("Hello from Lua")

function myluafunction(times)
   return string.rep("(-)", times)
end

function lua_func_with_params(x,y)
   print(x)
   print(y)
   print(x+y)
end

global_var = 0
function print_global()
   print("Global variable is "..global_var)
end

function test_cpp_functions()
   local x = 5
   print("Double "..x.." is "..double_number(x))
   print("Double "..x.." is "..double_integer(x))
   local y = 10
   print("Sum of "..x.." and "..y.." is "..sum_integers(x,y))
   print_integer(x+y)
end

function read_table()
   print(c_table.x)
   print(c_table.y)
end

lua_table = {}
lua_table.a = 5
lua_table.b = 'hello'

function test_classes()
   print('Testing classes...')
   local var = make_TestClass()
   print("var.x = "..var:GetX())
   var:SetX(5)
   print("var.x = "..var:GetX())
   var:PrintSelf()
   print('Testing success.')
end

function accepts_testclass(x)
   print("I got a TestClass")
   print("It has x of "..x:GetX())
end

global_testclass = make_TestClass()
function returns_testclass()
   print("Current value is "..global_testclass:GetX())
   return global_testclass
end