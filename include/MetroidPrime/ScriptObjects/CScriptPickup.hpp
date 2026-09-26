#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;
class CEchoParameters;

class CScriptPickup : public CActor {
public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const CEchoParameters& echo, const CAABox& aabb, CPlayerState::EItemType itemType,
                int amount, int capacityIncrease, int itemPercentageIncrease, CAssetId pickupEffect,
                bool absoluteValue, bool unknown, bool autoSpin, bool blinkOut, float lifetime,
                float respawnTime, float fadeTime, float activateDelay, float, float, float, float, const CVector3f& orbitOffset);

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

  CPlayerState::EItemType GetItem() const;
  void SetSpawned();
  bool IsVisible() const;
  void ShowAllKeysCollectedAlert(CStateManager& mgr, CPlayerState* playerState, CPlayerState::EItemType itemType);

private:
  CPlayerState::EItemType mItemType; // x158
  int mAmount;
  int mCapacity;
  int mPercentageIncrease;
  float mLifeTime;
  float mRespawnTime;
  float x170;
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
  int x1bc;
  int x1c0;
  CVector3f mOrbitOffset;
  bool mUnknownProp : 1;
  bool mGenerated : 1;  // unk
  bool mInTractor : 1;  // unk
  bool mAbsoluteValue : 1;
  bool mEnableTractorTest : 1;  // unk
  bool mAutoSpin : 1;
  bool mUnk2 : 1;
  bool mUnk3 : 1;
  bool mBlinkOut : 1;

};

#endif // _CSCRIPTPICKUP
