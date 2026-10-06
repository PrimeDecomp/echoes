#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;
struct SEchoParameters;

class CScriptPickup : public CActor {
public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const SEchoParameters& echo, const CAABox& aabb, CPlayerState::EItemType itemType,
                int amount, int capacityIncrease, int itemPercentageIncrease, CAssetId pickupEffect,
                bool absoluteValue, bool canHomeByDefault, bool autoSpin, bool blinkOut,
                float lifetime, float respawnTime, float fadeTime, float activateDelay,
                float pickupEffectLifetime, float autoHomeRange, float delayUntilHome,
                float homingSpeed, const CVector3f& orbitOffset);

  // CEntity
  ~CScriptPickup();
  CEntity* TypesMatch(int typeId) const override;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;

  // CActor
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  void PreRender(CStateManager&) override;
  void AddToRenderer(const CStateManager&) const override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;

  CPlayerState::EItemType GetItem() const;
  int GetAmount() const { return mAmount; }
  int GetCapacity() const { return mCapacity; }
  void SetWasGenerated(CStateManager& mgr);
  bool IsVisible() const;
  void ShowAllKeysCollectedAlert(CStateManager& mgr, CPlayerState* playerState,
                                 CPlayerState::EItemType itemType);

private:
  CPlayerState::EItemType mItemType; // x158
  int mAmount;
  int mCapacity;
  int mPercentageIncrease;
  float mLifeTime;
  float mRespawnTime;
  float mRespawnTimer;
  float mFadeTime;
  float mCurTime;
  float mTractorTime;
  float mPickupEffectLifetime;
  float mActivateDelay;
  float mAutoHomeRange;
  float mDelayUntilHome;
  float mHomingSpeed;
  float mTransformZ;
  rstl::optional_object< TToken< CGenDescription > > mPickupParticleDesc;
  CAABox mTouchBounds;
  int mFramesSinceLastSeen;
  int mHomingPlayerIndex;
  CVector3f mOrbitOffset;
  bool mCanHomeByDefault : 1;
  bool mInTractor : 1;
  bool mEnableTractorTest : 1;
  bool mAbsoluteValue : 1;
  bool mSuppressDeathScriptMsgs : 1;
  bool mAutoSpin : 1;
  bool mRenderedThisFrame : 1;
  bool mSuppressBobbing : 1;
  bool mBlinkOut : 1;
};
CHECK_SIZEOF(CScriptPickup, 0x1d8)

#endif // _CSCRIPTPICKUP
