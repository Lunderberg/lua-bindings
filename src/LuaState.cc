#include "lua-bindings/LuaState.hh"

#include <cassert>
#include <iostream>
#include <set>

#include "lua-bindings/detail/LuaRegistryNames.hh"
#include "lua-bindings/detail/LuaReferenceSet.hh"
#include "lua-bindings/detail/LuaKeepAlive.hh"

Lua::LuaState::LuaState(){
  memory[0] = 0;
  memory[1] = 0;
  lua_State* L = lua_newstate(limited_memory_alloc, &memory);
  shared_L = std::shared_ptr<lua_State>(L, lua_close);

  InitializeValidReferenceTable(L);
  InitializeKeepAliveTable(L);
}

Lua::LuaState::~LuaState() { }

void Lua::LuaState::LoadLibs(){
  luaL_openlibs(state());
}

void Lua::LuaState::LoadSafeLibs(){
  LoadLibs();

  LuaObject registry(state(), LUA_REGISTRYINDEX);
  LuaObject globals = registry[LUA_RIDX_GLOBALS].Get();
  LuaDelayedPop delayed(state(), 1); // Pop the globals table off when done.

  LuaObject sandbox = NewTable();

  std::vector<std::string> safe_funcs{
    "ipairs",
      "pairs",
      "print",
      "next",
      "pcall",
      "tonumber",
      "tostring",
      "type",
      "unpack",

      "coroutine.create",
      "coroutine.resume",
      "coroutine.running",
      "coroutine.status",
      "coroutine.wrap",
      "coroutine.yield",

      "string.byte",
      "string.char",
      "string.find",
      "string.format",
      "string.gmatch",
      "string.gsub",
      "string.len",
      "string.lower",
      "string.match",
      "string.rep",
      "string.reverse",
      "string.sub",
      "string.upper",

      "table.insert",
      "table.maxn",
      "table.remove",
      "table.sort",

      "math.abs",
      "math.acos",
      "math.asin",
      "math.atan",
      "math.atan2",
      "math.ceil",
      "math.cos",
      "math.cosh",
      "math.deg",
      "math.exp",
      "math.floor",
      "math.fmod",
      "math.frexp",
      "math.huge",
      "math.ldexp",
      "math.log",
      "math.log10",
      "math.max",
      "math.min",
      "math.modf",
      "math.pi",
      "math.pow",
      "math.rad",
      "math.random",
      "math.sin",
      "math.sinh",
      "math.sqrt",
      "math.tan",
      "math.tanh",

      "os.clock",
      "os.difftime",
      "os.time",
      };


  for(auto& val : safe_funcs){
    size_t dot_location = val.find(".");
    if(dot_location == std::string::npos){
      sandbox[val].Set(globals[val]);
    } else {
      std::string package = val.substr(0, dot_location);
      std::string function = val.substr(dot_location+1, val.size());

      if(!sandbox[package].Exists()){
        sandbox[package] = NewTable();
      }

      LuaObject global_package = globals[package].Get();
      LuaObject sandbox_package = sandbox[package].Get();
      LuaDelayedPop delayed(state(),2);

      sandbox_package[function].Set(global_package[function]);
    }
  }

  registry[LUA_RIDX_GLOBALS] = sandbox;
}

Lua::LuaObject Lua::LuaState::NewTable(){
  Lua::NewTable(state());
  return Lua::LuaObject(state());
}

void* Lua::limited_memory_alloc(void* ud, void* ptr, size_t osize, size_t nsize){
  unsigned long* memory_used = static_cast<unsigned long*>(ud);
  unsigned long* memory_allowed = memory_used+1;

  if(nsize == 0){
    *memory_used -= osize;
    free(ptr);
    return NULL;
  } else {
    if(*memory_allowed != 0 &&
       *memory_used + nsize - osize > *memory_allowed){
      return NULL;
    }
    ptr = realloc(ptr, nsize);
    if(ptr){
      *memory_used += (nsize - osize);
    }
    return ptr;
  }
}
