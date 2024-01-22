#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;
class CEchoParameters;

class CScriptPickup : public CActor {
  CPlayerState::EItemType m_itemType; // x158
  int m_amount;
  int m_capacity;
  int m_percentageIncrease;
  float m_lifeTime;
  float m_respawnTime;
  float x170;
  float m_fadeTime;
  float m_curTime;
  float x17c;
  float m_pickupEffectLifetime;
  float m_activateDelay;
  float m_autoHomeRange;
  float m_delayUntilHome;
  float m_homingSpeed;
  float m_transformZ;
  rstl::optional_object< TCachedToken< CGenDescription > > m_pickupParticleDesc;
  CAABox m_touchBounds;
  int x1bc;
  int x1c0;
  CVector3f m_orbitOffset;
  bool m_unknownProp : 1;
  bool m_generated : 1;  // unk
  bool m_inTractor : 1;  // unk
  bool m_absoluteValue : 1;
  bool m_enableTractorTest : 1;  // unk
  bool m_autoSpin : 1;
  bool m_unk2 : 1;
  bool m_unk3 : 1;
  bool m_blinkOut : 1;

public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const CEchoParameters& echo, const CAABox& aabb, CPlayerState::EItemType itemType,
                int amount, int capacityIncrease, int itemPercentageIncrease, CAssetId pickupEffect,
                bool absoluteValue, bool unknown, bool autoSpin, bool blinkOut, float lifetime,
                float respawnTime, float fadeTime, float activateDelay, float, float, float, float, const CVector3f& orbitOffset);

  ~CScriptPickup();

  CEntity* TypesMatch(int typeId) const override;

  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;

  void Render(const CStateManager&) const override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;

  CPlayerState::EItemType GetItem() const;
  void SetSpawned();
  bool IsVisible() const;
};

#endif // _CSCRIPTPICKUP
