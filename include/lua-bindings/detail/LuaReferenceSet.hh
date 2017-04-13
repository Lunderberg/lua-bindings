#ifndef _LUAREFERENCESET_H_
#define _LUAREFERENCESET_H_

#include <set>

struct lua_State;

namespace Lua{
  void InitializeValidReferenceTable(lua_State* L);
  unsigned long GenerateReferenceID(lua_State* L);
  bool IsValidReference(lua_State* L, unsigned long reference_id);

  class PreserveValidReferences{
  public:
    PreserveValidReferences(lua_State* L);
    ~PreserveValidReferences();
  private:
    lua_State* L;
    std::set<unsigned long> saved_values;
  };
}

#endif /* _LUAREFERENCESET_H_ */
