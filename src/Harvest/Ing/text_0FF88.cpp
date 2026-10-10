// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CResLoader.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CIngSpotData.hpp"
#include "MetroidPrime/Enemies/CIngSpotPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearch.hpp"
#include "MetroidPrime/PathFinding/CPathFindPointSearchFilter.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CEnvironmentVariable.hpp"
#include "MetroidPrime/Player/CGameStateEnvVarManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" CModelData fn_29_11490(int arg0) { return CModelData(); }

struct __mwdec_vt_0 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
};
struct __mwdec_vt_1 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
};
extern "C" void fn_29_10E64(int arg0) {
  ((__mwdec_vt_0*)*(int*)(arg0 + 0x2c0))->_2();
  ((__mwdec_vt_1*)*(int*)(arg0 + 0x2b0))->_2();
}

extern int gpRender;
struct __mwdec_vt_0_fn_29_10DEC {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5();
  virtual void _6();
  virtual void _7();
  virtual void _8();
  virtual void _9();
  virtual void _10();
  virtual void _11();
  virtual void _12();
  virtual void _13();
  virtual void _14();
  virtual void _15(int);
};
struct __mwdec_vt_1_fn_29_10DEC {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5();
  virtual void _6();
  virtual void _7();
  virtual void _8();
  virtual void _9();
  virtual void _10();
  virtual void _11();
  virtual void _12();
  virtual void _13();
  virtual void _14();
  virtual void _15(int);
};
extern "C" void fn_29_10DEC(int arg0) {
  int temp_r4 = *(int*)(arg0 + 0x2c0);
  if ((unsigned int)temp_r4 != 0) {
    ((__mwdec_vt_0_fn_29_10DEC*)gpRender)->_15(temp_r4);
  }
  int temp_r4_2 = *(int*)(arg0 + 0x2b0);
  if ((unsigned int)temp_r4_2 != 0) {
    ((__mwdec_vt_1_fn_29_10DEC*)gpRender)->_15(temp_r4_2);
  }
}
