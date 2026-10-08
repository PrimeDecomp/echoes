#ifndef _CKROCUSS
#define _CKROCUSS

#include "types.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "rstl/single_ptr.hpp"

class CDecalDescription;

// Guessed class: a wall-crawling creature whose wings open and close on a timer. The open wings
// are the vulnerable side and glow with a point light that fades with the wing position.
class CKrocuss : public CPatterned {
public:
  enum EWingState {
    kWS_Opening,
    kWS_Open,
    kWS_Closing,
    kWS_Closed,
  };

  CKrocuss(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
           const CModelData& modelData, const CPatternedInfo& patternedInfo,
           const CActorParameters& actorParams, const CColor& wingLightColor,
           const CDamageVulnerability& closedVulnerability, CAssetId decalDescription,
           ushort openSound, ushort closeSound, float animSpeedScalar, float timeShellClosed,
           float timeToOpenShell, float timeShellOpen, float timeToCloseShell,
           float timeToCloseShellDamaged, float maxAudibleDistance);

  // CEntity
  ~CKrocuss() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  const CDamageVulnerability* GetDamageVulnerability(const CVector3f& position,
                                                     const CVector3f& direction,
                                                     const CDamageInfo& damage) const override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  // CPatterned
  void SetupStateMachine(CStateManager& mgr) override;

  // CKrocuss
  bool WingsOpen(CStateManager& mgr, const CTriggerData& data) const;    // Guessed name
  bool WingsOpening(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  bool WingsClosed(CStateManager& mgr, const CTriggerData& data) const;  // Guessed name
  bool WingsClosing(CStateManager& mgr, const CTriggerData& data) const; // Guessed name
  void Patrol(CStateManager& mgr, EStateMsg msg, float dt);
  void Stop(CStateManager& mgr, EStateMsg msg, float dt);  // Guessed name
  void Sniff(CStateManager& mgr, EStateMsg msg, float dt); // Guessed name

private:
  int mWingState;                 // Guessed name
  float mTimeShellClosed;         // Guessed name
  float mTimeToOpenShell;         // Guessed name
  float mTimeShellOpen;           // Guessed name
  float mTimeToCloseShell;        // Guessed name
  float mTimeToCloseShellDamaged; // Guessed name
  float mWingStateTime;           // Guessed name
  uint mWingAnimation;            // Guessed name
  float mAnimSpeedScalar;         // Guessed name
  TUniqueId mUnusedId;            // Guessed name
  float mMaxAudibleDistance;      // Guessed name
  float mUnusedValue;             // Guessed name; never initialised or read
  rstl::single_ptr< TCachedToken< CDecalDescription > > mDecal; // Guessed name
  float mUnusedFloat;                                           // Guessed name
  CDamageVulnerability mOpenVulnerability;                      // Guessed name
  CDamageVulnerability mClosedVulnerability;                    // Guessed name
  CColor mWingLightColor;                                       // Guessed name
  CVector3f mWingLightOffset;                                   // Guessed name
  TUniqueId mWingLightId;                                       // Guessed name
  ushort mOpenSound;                                            // Guessed name
  ushort mCloseSound;                                           // Guessed name
  bool mClosedByDamage : 1;                                     // Guessed name
};
CHECK_SIZEOF(CKrocuss, 0x870)

#endif // _CKROCUSS
