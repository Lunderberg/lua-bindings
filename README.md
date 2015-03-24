Lua-bindings
============

These are bindings intended to easily embed Lua into a C++11 project.

Everything is done by including a single header file, `<lua-bindings/LuaState.hh>`,
  then compiling against the static library created by the included SConstruct.
The static library can also be built from the included SConscript,
  as part of a larger build.

Loading files
-------------

To load a file, use LuaState::LoadFile()

    Lua::LuaState L;
    L.LoadFile("some_file.lua");

Alternatively, use LuaState::LoadString() to directly load a string containing code.

    Lua::LuaState L;
    L.LoadString("print('hi')");

Loading Libraries
-----------------

There are two functions available, `LuaState::LoadLibs` and `LuaState::LoadSafeLibs`.
These each will load standard libraries available in Lua.
However, `LuaState::LoadSafeLibs` will only load library functions that are safe
  for an untrusted user to have access to.
The list of functions available in safe mode is as follows.

   - `ipairs`
   - `pairs`
   - `print`
   - `next`
   - `pcall`
   - `tonumber`
   - `tostring`
   - `type`
   - `unpack`

   - `coroutine.create`
   - `coroutine.resume`
   - `coroutine.running`
   - `coroutine.status`
   - `coroutine.wrap`

   - `string.byte`
   - `string.char`
   - `string.find`
   - `string.format`
   - `string.gmatch`
   - `string.gsub`
   - `string.len`
   - `string.lower`
   - `string.match`
   - `string.rep`
   - `string.reverse`
   - `string.sub`
   - `string.upper`

   - `table.insert`
   - `table.maxn`
   - `table.remove`
   - `table.sort`

   - `math.abs`
   - `math.acos`
   - `math.asin`
   - `math.atan`
   - `math.atan2`
   - `math.ceil`
   - `math.cos`
   - `math.cosh`
   - `math.deg`
   - `math.exp`
   - `math.floor`
   - `math.fmod`
   - `math.frexp`
   - `math.huge`
   - `math.ldexp`
   - `math.log`
   - `math.log10`
   - `math.max`
   - `math.min`
   - `math.modf`
   - `math.pi`
   - `math.pow`
   - `math.rad`
   - `math.random`
   - `math.sin`
   - `math.sinh`
   - `math.sqrt`
   - `math.tan`
   - `math.tanh`

   - `os.clock`
   - `os.difftime`
   - `os.time`

Getting/Setting Global Variables
--------------------------------

The C++ code can read/write global variables to be available in Lua.
When setting a variable, the type is automatically determined from the parameter.

    Lua::LuaState L;
    L.SetGlobal("x", 5);

When reading a variable, the type must be specified.

    int x = L.CastGlobal<int>("x");

Functions
---------

To call Lua functions from C++, use the LuaState::Call() method.
Note the templated parameter which gives the return type of the function.
If the Lua function returns a value that cannot be converted to the requested type,
a `LuaInvalidStackContents` will be thrown.

    Lua::LuaState L;
    L.LoadString("function add_one(x) return x+1 end");
    int output = L.Call<int>(42);

Lua functions can return multiple values.
If the return value is requested as a std::tuple<>, then multiple values will be read.

    Lua::LuaState L;
    L.LoadString("function multiple_returns() return 5, 'hello' end");
    std::tuple<int, std::string> output =
      L.Call<std::tuple<int, std::string> >("multiple_returns");

C++ functions can be registered, so that they can be called from within Lua.

    int sum_numbers(int x, int y){
      return x+y;
    }

    Lua::LuaState L;
    L.SetGlobal("sum_numbers", sum_numbers);

This works for any c-style function pointer or std::function.

Classes
-------

To register a class, use LuaState::MakeClass().
Consider the following class.

    class Example{
    public:
      Example();
      Example(int x);

      int GetX();
      void SetX(int x);

    private:
      int x;
    };

To register it for use in Lua, one would use the following code.

    Lua::LuaState L;
    L.MakeClass<Example>("Example")
      .AddConstructor<>("make_Example")
      .AddConstructor<int>("make_Example_int")
      .AddMethod("GetX", &Example::GetX)
      .AddMethod("SetX", &Example::SetX)

Only the constructors and methods that are registered are available for use in Lua.
This allows, for example, a class that can be passed in to Lua,
  but cannot be constructed from within Lua.

Lua Tables
----------

Two types of tables are supported as parameters and return values,
  when calling functions across the C++/Lua boundary.

The first are tables with only numerical keys,
  and only a single value type.
These can be converted to `std::vector<T>`.

    Lua::LuaState L;
    L.LoadString("function return_table() return {'a','b','c'} end");
    std::vector<std::string> output =
       L.Call<std::vector<std::string> >("return_table");

The second are tables with only string keys,
  and only a single value type.
These can be converted to `std::map<std::string, T>`.

    Lua::LuaState L;
    L.LoadString("function return_table() return {a=5, b=7, c=9} end");
    std::map<std::string, int> output =
      L.Call<std::map<std::string, int> >("return_table");

Lua Coroutines
--------------

A function can be started as a Lua coroutine, rather than running directly.
The main advantage is that coroutines can be yielded, while a function call cannot.

    Lua::LuaState L;
    L.LoadSafeLibs();
    L.LoadString("function yields() print('First') coroutine.yield() print('Second') end");

    auto coroutine = L.NewCoroutine("yields");
    coroutine.Resume(); // Prints 'First'
    coroutine.Resume(); // Prints 'Second'

In addition, coroutines can have limits placed on the number of Lua instructions that can be run.
This prevents user scripts from suspending the program if they infinite loop.

    Lua::LuaState L;
    L.LoadString("function infinite_loop() while true do end");

    auto coroutine = L.NewCoroutine("infinite_loop");
    coroutine.SetMaxInstruction(10000);
    coroutine.Resume(); // throws a LuaRuntimeTooLong.

Any arguments passed to `LuaCoroutine::Resume()` are given as the return values of `coroutine.yield()`.
Similarly, any returns requested from `LuaCoroutine::Resume()` will give the value
  passed into `coroutine.yield()`, or the value of the `return` statement if the function finishes.
The `IsFinished()` method can be used to determine whether a coroutine is completed,
or has just yielded.

    Lua::LuaState L;
    L.LoadSafeLibs();
    L.LoadString("functions yields_return(x) y = coroutine.yield(x) return y end");

    auto coroutine = L.NewCoroutine("yields_return");
    auto return1 = coroutine.Resume<int>(5);
    assert(!coroutine.IsFinished());
    auto return2 = coroutine.Resume<std::string>("hello");
    assert(coroutine.IsFinished());