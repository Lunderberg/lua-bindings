#include "lua-bindings/detail/LuaCoroutine.hh"

#include "lua-bindings/detail/LuaKeepAlive.hh"

bool Lua::LuaCoroutine::ended_by_timeout = false;

Lua::LuaCoroutine::LuaCoroutine(std::shared_ptr<lua_State> parent)
  : parent(parent), running(false), max_instructions(-1) {
  thread = lua_newthread(parent.get());
  LuaDelayedPop delayed(parent.get(), 1);

  reference = KeepObjectAlive(parent.get(), -1);
}

Lua::LuaCoroutine::~LuaCoroutine(){
  if(reference != -1){
    AllowToDie(parent.get(), reference);
  }
}

Lua::LuaCoroutine::LuaCoroutine(LuaCoroutine&& other)
  : parent(other.parent), running(other.running), thread(other.thread),
    max_instructions(other.max_instructions), reference(other.reference){
  other.reference = -1;
}

void Lua::yielding_hook(lua_State* L, lua_Debug*){
  Lua::LuaCoroutine::ended_by_timeout = true;
  lua_yield(L, 0);
}

//! Starts a function call in the coroutine.
void Lua::LuaCoroutine::LoadFunc(const char* name){
  if(running){
    throw LuaCoroutineAlreadyRunning("Cannot start code if code is already running");
  }

  lua_getglobal(thread, name);
  running = true;
}

//! Load a file into Lua
/*! Loads a file, then executes inside the coroutine.
 */
void Lua::LuaCoroutine::LoadFile(const char* filename){
  if(running){
    throw LuaCoroutineAlreadyRunning("Cannot start code if code is already running");
  }

  PushCodeFile(thread, filename);
  running = true;
}

//! Load a string into Lua
/*! Loads a string, then executes inside the coroutine.
 */
void Lua::LuaCoroutine::LoadString(const std::string& lua_code){
  if(running){
    throw LuaCoroutineAlreadyRunning("Cannot start code if code is already running");
  }

  PushCodeString(thread, lua_code);
  running = true;
}
