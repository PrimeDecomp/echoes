#ifndef _CSCRIPTHUDHINT
#define _CSCRIPTHUDHINT

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class CTexture;

// Guessed name. A textured HUD marker with visor filtering and horizontal-strip animation.
class CScriptHUDHint : public CActor {
public:
  // Guessed names. The native animation state is stopped, advancing or rewinding.
  enum EAnimationState { kAS_Stopped, kAS_Forward, kAS_Backward };
  // Reconstructed result type: minimum and maximum texture coordinates.
  typedef rstl::pair< CVector2f, CVector2f > TTextureCoordinates;

  CScriptHUDHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& transform, CAssetId hudTexture, float minScreenSize,
                 float maxScreenSize, float iconScale, float animationTime, int animationFrames,
                 int visorMask);

  // CEntity
  ~CScriptHUDHint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // Guessed names.
  void SetAnimationToMax(CStateManager& mgr);
  void SetAnimationToZero(CStateManager& mgr);
  CTexture* GetTexture() const;
  TTextureCoordinates GetTextureCoordinates() const;

private:
  rstl::optional_object< TLockedToken< CTexture > > mHudTexture;
  // Guessed names. The HUD renderer clamps the projected icon size to these limits.
  float mMinScreenSize;
  float mMaxScreenSize;
  float mIconScale;
  float mAnimationTime;
  int mAnimationFrames;
  int mVisorMask;
  float mAnimationPosition;
  EAnimationState mAnimationState;
};
CHECK_SIZEOF(CScriptHUDHint, 0x188)

#endif // _CSCRIPTHUDHINT
