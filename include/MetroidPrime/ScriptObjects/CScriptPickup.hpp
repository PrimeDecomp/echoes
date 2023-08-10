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
  int percentage;
  float lifeTime;
  float respawnTime;
  float x170;
  float fadeTime;
  float curTime;
  float x17c;
  float timeLeftToSet;
  float activateDelay;
  float autoHomeRange;
  float delayUntilHome;
  float homingSpeed;
  float transformZ;
  rstl::optional_object< TCachedToken< CGenDescription > > pickupParticleDesc;
  CAABox touchBounds;
  int x1bc;
  int x1c0;
  CVector3f orbitOffset;
  bool unknownProp : 1;
  bool generated : 1;  // unk
  bool inTractor : 1;  // unk
  bool absoluteValue : 1;
  bool enableTractorTest : 1;  // unk
  bool autoSpin : 1;
  bool unk2 : 1;
  bool unk3 : 1;
  bool blinkOut : 1;

public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const CEchoParameters& echo, const CAABox& aabb, CPlayerState::EItemType itemType,
                int amount, int capacityIncrease, int itemPercentageIncrease, CAssetId pickupEffect,
                bool absoluteValue, bool unknown, bool autoSpin, bool blinkOut, float lifetime,
                float respawnTime, float fadeTime, float activateDelay, float, float, float, float, const CVector3f& orbitOffset);

  ~CScriptPickup();

  CEntity* TypesMatch(EEntityType) override;

  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void AcceptScriptMsg(CStateManager& mgr, CScriptMsg&) override;

  void Render(const CStateManager&) const override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;

  CPlayerState::EItemType GetItem() const;
  void SetSpawned();
  bool IsVisible() const;
};

#endif // _CSCRIPTPICKUP
