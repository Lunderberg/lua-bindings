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

function test_double_number()
   local x = 5
   print("Double "..x.." is "..double_number(x))
   print("Double "..x.." is "..double_integer(x))
   local y = 10
   print("Sum of "..x.." and "..y.." is "..sum_integers(x,y))
end

function read_table()
   print(c_table.x)
   print(c_table.y)
end

lua_table = {}
lua_table.a = 5
lua_table.b = 'hello'