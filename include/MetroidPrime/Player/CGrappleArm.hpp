#ifndef _CGRAPPLEARM
#define _CGRAPPLEARM

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TStateMachineState.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"

class CAnimCharacterSet;
class CElementGen;
class CGenDescription;
class CGunController;
class CParticleSwoosh;
class CRainSplashGenerator;
class CSwooshDescription;

// Dependency layout only. Echoes makes this a CEntity and adds its own state machine.
class CGrappleArm : public CEntity {
public:
  CGrappleArm(const CVector3f& scale, TUniqueId playerId, bool multiplayer);
  ~CGrappleArm() override;

private:
  uint x24_;
  CPlayerState::EPlayerSuit mLoadedSuit;
  rstl::optional_object< CModelData > mArmModel;
  CModelData mSkeletonModel;
  TToken< CAnimCharacterSet > mArmCharacter;
  rstl::vector< CToken > mAnimations;
  rstl::reserved_vector< rstl::vector< CToken >, 4 > mSuitDependencies;
  int mSuitDependencyIndex;
  TCachedToken< CStateMachine > mStateMachineToken;
  TStateMachineState< CGrappleArm > mStateMachine;
  CTransform4f mTransform;
  CTransform4f mGrappleLocatorXf;
  CTransform4f mAuxTransform;
  CVector3f mScale;
  CVector3f mGrapplePointPosition;
  rstl::single_ptr< CGunController > mGunController;
  TLockedToken< CGenDescription > mGrappleSegment;
  TLockedToken< CGenDescription > mGrappleClaw;
  TLockedToken< CGenDescription > mGrappleHit;
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
  int mAnimationState;
  uint x298_; // Initialized to one; not established as a boolean field.
  uint x29c_;
  int mAnimSfxPitch;
  rstl::pair< ushort, CSfxHandle > mAnimSfx;
  CSfxHandle mGrappleLoopSfx;
  CSfxHandle mSwooshSfx;
  short mRumbleHandle;
  short mSoundVolume;
  TUniqueId mPlayerId;
  uchar mGrappleLocator;
  rstl::reserved_vector< uchar, 3 > mBeamLocators;
  bool x2c4_24_ : 1;
  bool x2c4_25_ : 1;
  bool x2c4_26_ : 1;
  bool x2c4_27_ : 1;
};
CHECK_SIZEOF(CGrappleArm, 0x2c8)

#endif // _CGRAPPLEARM
