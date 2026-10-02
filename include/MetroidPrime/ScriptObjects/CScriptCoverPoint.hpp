#ifndef _CSCRIPTCOVERPOINT
#define _CSCRIPTCOVERPOINT

#include "Kyoto/Animation/CharacterCommon.hpp"
#include "MetroidPrime/CActor.hpp"

// Original Wii export name, correlated with the GameCube COVR loader and consumers.
class CScriptCoverPoint : public CActor {
public:
  CScriptCoverPoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, uint flags, bool crouch, float horizontalSafeAngle,
                    float verticalSafeAngle, float lockTime);

  // CEntity
  ~CScriptCoverPoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  pas::ECoverDirection GetAttackDirection() const;
  bool ShouldCrouch() const;
  bool ShouldStay() const;
  bool ShouldWallHang() const;
  bool ShouldLandHere() const;
  bool Blown(const CVector3f& point) const;
  bool GetInUse(TUniqueId uid) const;
  void SetInUse(bool inUse);
  void Reserve(TUniqueId id) { mOccupant = id; }
  void ResetCooldown() { mTimeRemaining = 0.f; } // Guessed name.

private:
  uint mFlags;
  float mHorizontalSafeHalfAngle; // Radians; not Prime's cosine threshold.
  float mVerticalSafeHalfAngle;   // Radians; not Prime's sine threshold.
  float mLockTime;
  bool mCrouch : 1;
  bool mInUse : 1;
  TUniqueId mOccupant;
  TUniqueId mRetreatPoint; // Guessed name; first Retreat connection target.
  rstl::optional_object< CAABox > mTouchBounds;
  float mTimeRemaining;
};
CHECK_SIZEOF(CScriptCoverPoint, 0x190)

#endif // _CSCRIPTCOVERPOINT
