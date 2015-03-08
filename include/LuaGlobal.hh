#ifndef _LUAGLOBAL_H_
#define _LUAGLOBAL_H_

#include <memory>
#include <string>

class LuaState;
class lua_State;

class LuaGlobal{
public:
  LuaGlobal(std::shared_ptr<LuaState> state, std::string name);

  bool IsNumber();
  double ToNumber();

  bool IsString();
  std::string ToString();

  bool IsFunction();

  bool IsNil();

private:
  std::shared_ptr<LuaState> lua_state;
  std::string name;
};

#endif /* _LUAGLOBAL_H_ */
