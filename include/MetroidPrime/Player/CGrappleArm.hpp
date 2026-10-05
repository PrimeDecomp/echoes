#ifndef _CGRAPPLEARM
#define _CGRAPPLEARM

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CAnimCharacterSet;
class CActorLights;
class CInt32POINode;
class CElementGen;
class CGenDescription;
class CGunController;
class CParticleSwoosh;
class CRainSplashGenerator;
class CRumbleManager;
class CPlayer;
class CSkinnedModel;
class CSwooshDescription;
struct SSkinningWorkspace;

class CGrappleArm : public CEntity {
public:
  // Guessed names. Echoes's animation-state numbering differs from Prime's.
  enum EArmState {
    kAS_IntoGrapple = 0,
    kAS_IntoGrappleIdle = 1,
    kAS_FireGrapple = 2,
    kAS_Three = 3,
    kAS_ConnectGrapple = 4,
    kAS_Connected = 5,
    kAS_OutOfGrapple = 6,
    kAS_Seven = 7,
    kAS_Done = 8
  };

  // Guessed names for the requests consumed by the SamusArmFSM resource.
  enum EStateFlags {
    kSF_Default = 0x1,
    kSF_GunChanging = 0x2,
    kSF_FreeLook = 0x4,
    kSF_ComboFire = 0x8,
    kSF_Grappling = 0x10,
    kSF_Fidget = 0x20
  };

  CGrappleArm(const CVector3f& scale, TUniqueId playerId, bool multiplayer);

  // CEntity
  ~CGrappleArm() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CGrappleArm; guessed names, in target virtual-slot order.
  virtual void TryInitializeStateMachine(CStateManager& mgr);
  virtual void InitializeStateMachine(CStateManager& mgr);

  void TouchModel(const CStateManager& mgr) const;
  void PreRender(CStateManager& mgr, const CVector3f& cameraPos);
  void Render(const CStateManager& mgr, const CVector3f& pos, const CModelFlags& flags,
              const CActorLights* lights) const;
  void RenderGrappleBeam(const CStateManager& mgr, const CVector3f& pos, bool firstPerson) const;
  void Update(float dt, CStateManager& mgr);
  void UpdateArmMovement(float dt, CStateManager& mgr);
  void UpdateSwingAction(float dt, CStateManager& mgr);
  bool UpdateGrappleBeam(float dt, const CTransform4f& beamLocator, CStateManager& mgr);
  void UpdateGrappleBeamFX(CStateManager& mgr, const CVector3f& gunPos, const CVector3f& beamPos,
                           const CTransform4f& rotation, bool firstPerson);
  void ResetAuxParams(bool resetGunController);
  void Activate(bool active);
  void SetAnimState(EArmState state);
  void GrappleBeamConnected(CStateManager& mgr);
  void GrappleBeamDisconnected();
  void DisconnectGrappleBeam();
  void DoUserAnimEvents(CStateManager& mgr);
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type);
  static void PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                             void* context);
  void EnterFreeLook(CStateManager& mgr);
  void EnterIdle(CStateManager& mgr);
  void EnterComboFire(CStateManager& mgr);
  void EnterFidget(CStateManager& mgr, int type, int gunId, int animSet);
  void EnterStruck(CStateManager& mgr, float angle, bool bigStrike, bool notInFreeLook);
  void ReturnToDefault(CStateManager& mgr, float delay, bool reset);
  void SetStateFlags(uint flags); // Guessed name.
  uint GetStateFlags() const { return mStateFlags; }
  CGunController* GunController() { return mGunController.get(); }
  bool IsGrappling() const { return (mStateFlags & kSF_Grappling) != 0; }
  bool IsLoadingDependencies() const { return mDependenciesLoading; }
  void SetAuxTransform(const CTransform4f& xf) { mAuxTransform = xf; }
  EArmState GetAnimState() const { return mAnimationState; }
  bool IsGrappleBeamActive() const { return mBeamActive; }
  CTransform4f GetTransform() const { return mTransform; }
  void SetTransform(const CTransform4f& xf) { mTransform = xf; }

  // Callback names are preserved in the original state-machine registration strings.
  void Start(CStateManager& mgr, int msg, float dt);
  void DownAtSide(CStateManager& mgr, int msg, float dt);
  void HoldingGun(CStateManager& mgr, int msg, float dt);
  void WaitAnimOver(CStateManager& mgr, int msg, float dt);
  void WeaponChange(CStateManager& mgr, int msg, float dt);
  void Fidget(CStateManager& mgr, int msg, float dt);
  void Grappling(CStateManager& mgr, int msg, float dt);
  bool HoldGun(CStateManager& mgr, const float& arg);
  bool AnimOver(CStateManager& mgr, const float& arg);
  bool GunChanging(CStateManager& mgr, const float& arg);
  bool FidgetActive(CStateManager& mgr, const float& arg);
  bool GrappleActive(CStateManager& mgr, const float& arg);

private:
  // Guessed names for Echoes-specific helpers.
  void BuildBeamDependencyList(bool multiplayer);
  void ResetStateMachine(CStateManager& mgr);
  CStateMachine* GetStateMachine();
  void LoadBeamDependencies(CPlayerState::EBeamId beam);
  void PlayGrappleAnimation(CAnimData& animData, int anim);
  void UpdateGrappleModel(CStateManager& mgr, CPlayerState::EPlayerSuit suit, bool force);
  CPlayer* GetPlayer(CStateManager& mgr) const;
  CRumbleManager* GetRumbleManager(CStateManager& mgr) const;

  CPlayerState::EPlayerSuit mCurrentSuit;
  CPlayerState::EPlayerSuit mLoadedSuit;
  rstl::optional_object< CModelData > mArmModel;
  CModelData mGrappleGearModel;
  TToken< CAnimCharacterSet > mArmCharacter;
  rstl::vector< CToken > mAnimations;
  rstl::reserved_vector< rstl::vector< CToken >, 4 > mBeamDependencies;
  CPlayerState::EBeamId mBeamId;
  TCachedToken< CStateMachine > mStateMachineToken;
  TStateMachineState< CGrappleArm > mStateMachine;
  CTransform4f mTransform;
  CTransform4f mAuxTransform;
  CTransform4f mGrappleLocatorXf;
  CVector3f mScale;
  CVector3f mGrapplePointPosition;
  rstl::single_ptr< CGunController > mGunController;
  TLockedToken< CGenDescription > mGrappleSegment;
  TLockedToken< CGenDescription > mGrappleClaw;
  TLockedToken< CGenDescription > mGrappleHitDesc;
  TLockedToken< CGenDescription > mGrappleMuzzle;
  TLockedToken< CSwooshDescription > mGrappleSwoosh;
  rstl::single_ptr< CElementGen > mSegmentGenerator;
  rstl::single_ptr< CElementGen > mClawGenerator;
  rstl::single_ptr< CElementGen > mHitGenerator;
  rstl::single_ptr< CElementGen > mMuzzleGenerator;
  rstl::single_ptr< CParticleSwoosh > mSwooshGenerator;
  rstl::single_ptr< CRainSplashGenerator > mRainSplashGenerator;
  rstl::single_ptr< CElementGen > mMultiplayerSegmentGenerator;
  rstl::single_ptr< CParticleSwoosh > mMultiplayerSwooshGenerator;
  float mBeamT;
  float mBeamDistance;
  float mAnglePhase;
  float mXAmplitude;
  float mZAmplitude;
  float mSwingT;
  EArmState mAnimationState;
  uint mStateFlags;
  uint mSoundSetIndex;
  int mAnimSfxPitch;
  rstl::pair< ushort, CSfxHandle > mAnimSfx;
  CSfxHandle mGrappleLoopSfx;
  CSfxHandle mSwooshSfx;
  short mRumbleHandle;
  short mSoundPan;
  TUniqueId mPlayerId;
  CSegId mGrappleLocator;
  rstl::reserved_vector< CSegId, 3 > mBeamLocators;
  bool mStateMachineInitialized : 1;
  bool mBeamActive : 1;
  bool mGrappleHit : 1;
  bool mDependenciesLoading : 1;
};
CHECK_SIZEOF(CGrappleArm, 0x2c8)

#endif // _CGRAPPLEARM
