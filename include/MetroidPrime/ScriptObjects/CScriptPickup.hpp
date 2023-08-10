#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;
class CEchoParameters;

class CScriptPickup : public CActor {
  CPlayerState::EItemType itemType; // x158
  int amount;
  int capacity;
  float possibility;
  float fadeInTime;
  float lifeTime;
  float curTime;
  float tractorTime;
  float delayTimer;
  rstl::optional_object< TCachedToken< CGenDescription > > pickupParticleDesc;
  bool generated : 1;
  bool inTractor : 1;
  bool enableTractorTest : 1;

public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const CEchoParameters& echo, const CAABox& aabb, CPlayerState::EItemType itemType,
                int amount, int capacityIncrease, int itemPercentageIncrease, CAssetId pickupEffect,
                bool absoluteValue, bool unknown, bool autoSpin, bool blinkOut, float lifetime,
                float respawnTime, float fadeTime, float activateDelay);

  ~CScriptPickup();

  CEntity* TypesMatch(EEntityType) override;

  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void AcceptScriptMsg(CStateManager& mgr, CScriptMsg&) override;

  void Render(const CStateManager&) const override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;

  float GetPossibility() const;
  CPlayerState::EItemType GetItem() const;
  void SetSpawned();
  bool IsVisible() const;
};

#endif // _CSCRIPTPICKUP
