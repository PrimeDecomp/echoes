#ifndef _CSCRIPTAIWAYPOINT
#define _CSCRIPTAIWAYPOINT

#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

// Wii SEL class name; GameCube correspondence is supported by the type-39 casts and loader.
// Constructor/member spellings and qualifiers are reconstructed.
class CScriptAIWaypoint : public CScriptWaypoint {
public:
  CScriptAIWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, float speed, float pause, int flags, int locatorIndex,
                    int unknown);

  // CEntity
  ~CScriptAIWaypoint() override;
  CEntity* TypesMatch(int typeId) const override;

  float GetSpeed() const { return mSpeed; }
  float GetPause() const { return mPause; }
  uint GetFlags() const { return x164_; } // Guessed accessor name; unknown bits are preserved.

private:
  float mSpeed;
  float mPause;
  int mLocatorIndex;
  uint x164_; // Low-16-bit flags; individual meanings remain unresolved.
  int x168_;  // Property 0x166979d4; runtime meaning remains unresolved.
};
CHECK_SIZEOF(CScriptAIWaypoint, 0x170)

#endif // _CSCRIPTAIWAYPOINT
