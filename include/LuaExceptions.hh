#ifndef _LUAEXCEPTIONS_H_
#define _LUAEXCEPTIONS_H_

#include <stdexcept>

#define Exception(Parent, Name)                 \
  struct Name : public Parent {                 \
    using Parent::Parent;                       \
  }

struct LuaException : public std::runtime_error{
  using std::runtime_error::runtime_error;
};

Exception(LuaException, LuaFileNotFound);
Exception(LuaException, LuaInvalidStackContents);
Exception(LuaException, LuaFileParseError);

Exception(LuaException, LuaExecuteError);
Exception(LuaExecuteError, LuaCoroutineExecuteError);
Exception(LuaExecuteError, LuaOutOfMemoryError);

Exception(LuaException, LuaCppCallError);
Exception(LuaCppCallError, LuaIncorrectUserData);

Exception(LuaException, LuaClassNotRegistered);

Exception(LuaException, LuaRuntimeTooLong);

#undef Exception

#endif /* _LUAEXCEPTIONS_H_ */
