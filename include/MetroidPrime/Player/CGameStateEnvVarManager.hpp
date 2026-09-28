#ifndef _CGAMESTATEENVVARMANAGER
#define _CGAMESTATEENVVARMANAGER

#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "rstl/map.hpp"
#include "rstl/string.hpp"

// Class name corroborated by the MP2 Wii SEL; method names are inferred.
class CGameStateEnvVarManager {
public:
  // Guessed enum name; these select the system and per-game SAVW variable lists.
  enum EVariableScope { kVS_System, kVS_Game };

  explicit CGameStateEnvVarManager(EVariableScope scope);
  CGameStateEnvVarManager(EVariableScope scope, CBitStreamReader& in);
  CEnvironmentVariable* FindEnvironmentVariable(const char* name);
  void InitializeMemoryState();
  void PutTo(CBitStreamWriter& out) const;

private:
  void LoadFields();
  void AddVariable(const rstl::string& name, const CEnvironmentVariable& variable);

  EVariableScope mScope;
  rstl::map< rstl::string, CEnvironmentVariable > mVariables;
};
CHECK_SIZEOF(CGameStateEnvVarManager, 0x18)

#endif // _CGAMESTATEENVVARMANAGER
