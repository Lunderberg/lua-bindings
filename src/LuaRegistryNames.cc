#include "lua-bindings/detail/LuaRegistryNames.hh"

const std::string Lua::cpp_function_registry_entry = "Lua.Cpp.Function.Metatable";

const std::string Lua::upcaster_registry_entry = "Lua.Upcaster.Metatable";

const std::string Lua::cpp_reference_counter = "Lua.Cpp.Reference.Counter";
const std::string Lua::cpp_valid_reference_set = "Lua.Cpp.Valid.Reference.Set";
const std::string Lua::cpp_reference_set_metatable = "Lua.Cpp.Reference.Set.Metatable";

const std::string Lua::keepalive_table = "Lua.KeepAlive.Table";

const std::string Lua::luastate_weakptr = "LuaState.WeakPtr";
const std::string Lua::luastate_weakptr_metatable = "LuaState.WeakPtr.Metatable";
