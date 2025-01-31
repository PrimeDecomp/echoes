#ifndef _CPLAYER
#define _CPLAYER

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TReservedAverage.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayerEnergyDrain.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CMorphBall;
class CPlayerState;
class CCameraManager;
class CPlayerGun;

namespace NPlayer {
enum EPlayerMovementState {
  kMS_OnGround,
  kMS_Jump,
  kMS_ApplyJump,
  kMS_Falling,
  kMS_FallingMorphed,
};
};

class CPlayer : public CPhysicsActor {
public:
  enum ESurfaceRestraints {
    kSR_Normal,
    kSR_Air,
    kSR_Ice,
    kSR_Organic,
    kSR_Water,
    kSR_Lava,
    kSR_Phazon,
    kSR_Shrubbery,
  };
  enum EPlayerCameraState {
    kCS_FirstPerson,
    kCS_Ball,
    kCS_Two,
    kCS_Transitioning,
    kCS_Spawned,
  };
  enum EPlayerMorphBallState {
    kMS_Unmorphed,
    kMS_Morphed,
    kMS_Morphing,
    kMS_Unmorphing,
  };

  enum EPlayerOrbitState {
    kOS_NoOrbit,
    kOS_OrbitObject,
    kOS_OrbitPoint,
    kOS_OrbitCarcass,
    kOS_ForcedOrbitObject, // For CMetroidBeta attack
    kOS_Grapple,
  };
  enum EPlayerOrbitType {
    kOT_Close,
    kOT_Far,
    kOT_Default,
  };
  enum EPlayerOrbitRequest {
    kOR_StopOrbit,
  };
  enum EPlayerZoneInfo {
    kZI_Targeting,
    kZI_Scan,
  };
  enum EPlayerZoneType {
    kZT_Always = -1,
    kZT_Box,
    kZT_Ellipse,
  };
  enum EPlayerScanState {
    kSS_NotScanning,
    kSS_Scanning,
    kSS_ScanComplete,
  };
  enum EGrappleState {
    kGS_None,
    kGS_Firing,
    kGS_Pull,
    kGS_Swinging,
    kGS_JumpOff,
  };

  struct UnkStructA {
    char pad[0xa8];

    UnkStructA(TUniqueId);
  };
  struct UnkStructB {
    char pad[0x8e];
  };
  struct UnkStructC {
    char pad[0x1f0];
  };

  CPlayer(TUniqueId uid, const CTransform4f& xf, const CAABox& aabb, CAssetId resId,
          const CVector3f& playerScale, float mass, float stepUp, float stepDown, float ballRadius,
          const CMaterialList& ml, CPlayerState*, CCameraManager*, bool, int playerIndex, int,
          int charIdx);

  int GetPlayerIndex() const;
  const CVector3f& GetLookDir() const { return m_lookDir; };
  CMorphBall* GetMorphBall() { return m_morphBall; }
  CPlayerState* GetPlayerState() { return m_playerState; }

  EPlayerMorphBallState GetMorphballTransitionState() const { return m_morphBallState; }
  EPlayerMorphBallState GetSpawnedMorphballState() const { return m_spawnedMorphBallState; }
  float fn_8000BE98() const;
  void fn_80019E40(CStateManager&, int);

  void Teleport(const CTransform4f& xf, CStateManager& mgr, bool resetBallCam);
  void SetSpawnedMorphBallState(EPlayerMorphBallState state, CStateManager& mgr);
  const CCameraManager* GetCameraManager() const { return m_cameraManager; }

private:
  NPlayer::EPlayerMovementState m_movementState;         // 0x2d0
  rstl::vector< CToken > m_ballTransitionsRes;           // 0x2d4
  TUniqueId m_attachedActor;                             // 0x2e4
  float m_attachedActorTime;                             // 0x2e8
  CPlayerEnergyDrain m_energyDrain;                      // 0x2ec
  float m_startingJumpTimeout;                           // 0x300
  float m_sjTimer;                                       // 0x304
  float m_minJumpTimeout;                                // 0x308
  float m_jumpCameraTimer;                               // 0x30c
  int m_jumpPresses;                                     // 0x310
  float m_fallCameraTimer;                               // 0x314
  float x318_unk;                                        // 0x318
  bool m_cancelCameraPitch;                              // 0x31c
  float m_timeSinceJump;                                 // 0x320
  float x324_unk;                                        // 0x324
  float x328_unk;                                        // 0x328
  CVector3f x32c_unk;                                    // 0x32c
  CVector3f m_lastSpaceJumpPosition;                     // 0x338
  ESurfaceRestraints m_surfaceRestraint;                 // 0x344
  rstl::reserved_vector< float, 6 > m_accelerationTable; // 0x348
  uint m_curAcceleration;                                // 0x364
  float m_accelerationChangeTimer;                       // 0x368
  CAABox m_fpBounds;                                     // 0x36c
  float m_ballTransHeight;                               // 0x384
  EPlayerCameraState m_cameraState;                      // 0x388
  EPlayerMorphBallState m_morphBallState;                // 0x38c
  EPlayerMorphBallState m_spawnedMorphBallState;         // 0x390
  bool x394_unk_sa;                                      // 0x394
  EPlayerMorphBallState x398_unk_sa_state;               // 0x398
  EPlayerMorphBallState x39c_unk;                        // 0x39c
  float m_fallingTime;                                   // 0x3a0
  EPlayerOrbitState m_orbitState;                        // 0x3a4
  EPlayerOrbitType m_orbitType;                          // 0x3a8
  int m_orbitRequest;                                    // 0x3ac
  TUniqueId m_orbitTargetId;                             // 0x3b0
  CVector3f m_orbitPoint;                                // 0x3b4
  CVector3f m_orbitVector;                               // 0x3c0
  float m_orbitModeTimer;                                // 0x3cc
  EPlayerZoneInfo m_orbitZoneMode;                       // 0x3d0
  EPlayerZoneType m_orbitZoneType;                       // 0x3d4
  int x3d8_unk;                                          // 0x3d8
  TUniqueId m_orbitNextTargetId;                         // 0x3dc
  float x3e0_unk;                                        // 0x3e0
  rstl::vector< TUniqueId > m_nearbyOrbitObjects;        // 0x3e4
  rstl::vector< TUniqueId > m_onScreenOrbitObjects;      // 0x468
  rstl::vector< TUniqueId > m_offScreenOrbitObjects;     // 0x4ec
  bool m_orbitLockEstablished;                           // 0x570
  float m_orbitPreventionTimer;                          // 0x574
  bool m_sidewaysDashing;                                // 0x578
  float m_strafeInputAtDash;                             // 0x57c
  float m_dashTimer;                                     // 0x580
  float m_dashButtonHoldTime;                            // 0x584
  bool m_doneSidewaysDashing;                            // 0x588
  uint m_orbitSource;                                    // 0x58c
  bool m_orbitingEnemy;                                  // 0x590
  bool x591_unk;                                         // 0x591
  float x594_unk;                                        // 0x594
  int x598_unk;                                          // 0x598
  int x59c_unk;                                          // 0x59c
  float x5a0_unk;                                        // 0x5a0
  float x5a4_unk;                                        // 0x5a4
  float m_dashSpeedMultiplier;                           // 0x5a8
  bool m_noStrafeDashBlend;                              // 0x5ac
  float m_dashDuration;                                  // 0x5b0
  float m_strafeDashBlendDuration;                       // 0x5b4
  EPlayerScanState m_scanState;                          // 0x5b8
  float m_scanningTime;                                  // 0x5bc
  float m_curScanTime;                                   // 0x5c0
  TUniqueId m_scanningObject;                            // 0x5c4
  uint m_scanningObjectId;                               // 0x5c8
  EGrappleState m_grappleState;                          // 0x5cc
  float m_grappleSwingTimer;                             // 0x5d0
  CVector3f m_grappleSwingAxis;                          // 0x5d4
  float x5e0_unk;                                        // 0x5e0
  float x5e4_unk;                                        // 0x5e4
  float x5e8_unk;                                        // 0x5e8
  float m_grappleJumpTimeout;                            // 0x5ec
  uchar x5f0_unk;                                        // 0x5f0
  bool m_inFreeLook;                                     // 0x5f1
  bool m_lookButtonHeld;                                 // 0x5f2
  bool m_lookAnalogHeld;                                 // 0x5f3
  bool x5f4_unk;                                         // 0x5f4
  bool x5f5_unk;                                         // 0x5f5
  float m_curFreeLookCenteredTime;                       // 0x5f8
  float m_freeLookYawAngle;                              // 0x5fc
  float m_horizFreeLookAngleVel;                         // 0x600
  float m_freeLookPitchAngle;                            // 0x604
  float m_vertFreeLookAngleVel;                          // 0x608
  TUniqueId m_aimTarget;                                 // 0x60c
  CVector3f m_targetAimPosition;                         // 0x610
  TReservedAverage< CVector3f, 10 > m_aimTargetAverage;  // 0x61c
  CVector3f m_assistedTargetAim;                         // 0x698
  float m_aimTargetTimer;                                // 0x6a4
  int x6a8_unk;                                          // 0x6a8
  char x6ac_unk[0x800];                                  // 0x6ac
  int xeac_unk;                                          // 0xeac
  int xeb0_unk;                                          // 0xeb0
  float xeb4_unk;                                        // 0xeb4
  float xeb8_unk;                                        // 0xeb8
  rstl::single_ptr< CPlayerGun > m_gun;                  // 0xebc
  float m_gunAlpha;                                      // 0xec0
  int xec4_unk;                                          // 0xec4
  bool xec8_unk;                                         // 0xec8
  bool xec9_unk;                                         // 0xec9
  short xeca_unk;                                        // 0xeca
  CColor xecc_unk;                                       // 0xecc
  int xed0_unk;                                          // 0xed0
  UnkStructA* xed4_unk;                                   // 0xed4
  void* xed8_unk;                                        // 0xed8
  UnkStructB xedc_unk;                                   // 0xedc
  int xf6c_unk;                                          // 0xf6c
  UnkStructC* xf70_unk;                                   // 0xf70
  TReservedAverage< float, 20 > m_moveSpeedAvg;          // 0xf74
  float m_moveSpeed;                                     // 0xfc8
  float m_flatMoveSpeed;                                 // 0xfcc
  CVector3f m_lookDir;                                   // 0xfd0
  char m_pad_3[0x198];                                   // 0xFDC
  CMorphBall* m_morphBall;                               // 0x1174
  char m_pad_4[0x19C];                                   // 0x1178
  CPlayerState* m_playerState;                           // 0x1314
  CCameraManager* m_cameraManager;                       // 0x1318
};
// size: 0x14c8

#endif // _CPLAYER
