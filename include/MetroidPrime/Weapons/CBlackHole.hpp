#ifndef _CBLACKHOLE
#define _CBLACKHOLE

#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CElementGen;
class CGenDescription;

// Guessed class name; the native creation label is "DarkBlackHole".
class CBlackHole : public CWeapon {
public:
  // Guessed names for the two independently observed flag roles.
  enum EFlags {
    kF_PullPlayers = 1 << 0,
    kF_CreationSound = 1 << 1,
  };

  CBlackHole(const rstl::optional_object< TToken< CGenDescription > >& particle,
             TUniqueId uid, TAreaId areaId, TUniqueId owner, const CTransform4f& xf,
             const CDamageInfo& damage, const rstl::string& name, float radius, float duration,
             uint flags);

  // CEntity
  ~CBlackHole() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

private:
  void ApplyDamageToWorld(const CVector3f& position, CStateManager& mgr);
  void UpdateRadius();

  float mElapsedTime;
  float mPullStrength;
  float mPullConeAngleDegrees;
  float mAttractionRange;
  CVector3f mPullDirection;
  rstl::single_ptr< CElementGen > mParticleGen;
  CAssetId mSourceId;
  TUniqueId mLightId;
  float mRadius;
  float mDuration;
  uint mFlags;
};
CHECK_SIZEOF(CBlackHole, 0x200)

#endif // _CBLACKHOLE
