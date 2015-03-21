#include "LuaState.hh"

#include <cassert>
#include <iostream>

Lua::LuaState::LuaState() : L(nullptr){
  memory[0] = 0;
  memory[1] = 0;
  L = lua_newstate(limited_memory_alloc, &memory);
}

Lua::LuaState::~LuaState() {
  if(L){
    lua_close(L);
  }
}

void Lua::LuaState::LoadLibs(){
  luaL_openlibs(L);
}

Lua::LuaObject Lua::LuaState::NewTable(){
  Lua::NewTable(L);
  return Lua::LuaObject(L);
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
