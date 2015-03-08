print("Hello from Lua")

function myluafunction(times)
   return string.rep("(-)", times)
end

function myfunction(arg)
   return cppfunction(arg)
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