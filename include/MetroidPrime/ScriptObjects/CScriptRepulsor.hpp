#ifndef _CSCRIPTREPULSOR
#define _CSCRIPTREPULSOR

#include "MetroidPrime/CActor.hpp"

// Echoes Wii exports name this class CScriptRepulsor; Prime calls its counterpart CRepulsor.
class CScriptRepulsor : public CActor {
public:
  // Guessed names; the cylindrical test ignores the vertical displacement.
  enum EShape { kS_Sphere, kS_Cylinder };
  // Guessed names, derived from the consumers of each flag.
  enum EFlags { kF_RepelPlayer = 1, kF_UseRightVector = 2 };

  CScriptRepulsor(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& transform, float radius, float strength, EShape shape,
                  uint flags);

  // CEntity
  ~CScriptRepulsor() override;
  CEntity* TypesMatch(int typeId) const override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  EShape GetShape() const;
  float GetStrength() const; // Guessed name
  float GetRadius() const;

private:
  float mRadius;
  float mStrength;
  EShape mShape;
  uint mFlags;
};
CHECK_SIZEOF(CScriptRepulsor, 0x168)

#endif // _CSCRIPTREPULSOR
