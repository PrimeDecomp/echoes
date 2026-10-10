// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfo.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
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
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleSpawnSystem.hpp"
#include "Kyoto/Particles/CSpawnSystemDescription.hpp"
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
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CSurfaceAlignmentHelper.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
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
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDamageableTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSpecialFunction.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "WorldFormat/CCollisionSurface.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" int fn_30_13B88(int arg0) { return arg0 + 1392; }

extern "C" int fn_30_11E44(int arg0) { return arg0 + 1232; }

extern "C" int fn_30_13B90(int arg0) { return arg0 + 1392; }

extern "C" void fn_30_13B7C(int arg0, int arg1) {
  *(unsigned short*)arg0 = *(unsigned short*)(arg1 + 0x4f6);
}

extern "C" bool fn_30_13B98(int arg0) { return *(unsigned char*)(arg0 + 0x5d8) >> 2 & 1; }

extern "C" int fn_30_13C5C(const int arg0) {
  if ((*(unsigned char*)(0x5c0 + arg0)) <= 0) {
  } else {
    return arg0 + 1440;
  }
  return 0;
}

extern "C" void fn_30_1464C(int arg0, int arg1, int arg2) {
  ((CActor*)arg0)->CActor::Touch(*(CActor*)arg1, *(CStateManager*)arg2);
}

extern "C" void fn_30_16088() {
  void fn_30_160A8();
  fn_30_160A8();
}

extern "C" void fn_30_11E4C(int arg0, float arg1) {
  ((CMayaSpline*)(arg0 + 816))->EvaluateAt(arg1);
}

extern "C" void fn_30_15FF4();
extern "C" int fn_30_15FC4(int arg0) {
  fn_30_15FF4();
  return arg0;
}

extern "C" int fn_30_127C0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_30_11DF0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_30_13E68(int arg0, int arg1) {
  ((CActor*)arg0)->CActor::AddToRenderer(*(const CStateManager*)arg1);
  if ((*(unsigned char*)(arg0 + 0x5d8) >> 5 & 1) &&
      ((CStateManager*)arg1)->IsActorVisible(*(const CActor*)arg0)) {
    ((CActor*)arg0)->EnsureRendered(*(const CStateManager*)arg1);
  }
}

extern "C" void fn_30_136FC(int arg0, int arg1, int arg2, int arg3) {
  int temp_r0;
  int var_r7;
  int temp_r8 = *(int*)(arg1 + 0xc);
  var_r7 = *(int*)arg3;
  int temp_r6 = (*(int*)arg2 - temp_r8) / 4;
  int var_r9 = temp_r6;
  int var_r8 = temp_r8 + (temp_r6 << 2);
  while (var_r7 != (unsigned int)(*(int*)(arg1 + 0xc) + (*(int*)(arg1 + 0x4) << 2))) {
    temp_r0 = *(int*)var_r7;
    var_r9 = var_r9 + 1;
    var_r7 = var_r7 + 4;
    *(int*)var_r8 = temp_r0;
    var_r8 = var_r8 + 4;
  }
  *(int*)(arg1 + 0x4) = var_r9;
  *(int*)arg0 = *(int*)arg2;
}

extern "C" void fn_30_388C();
extern "C" void fn_30_160A8(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_30_388C();
  }
}

extern "C" void fn_30_1282C(int obj) {
  int pitch;
  float y;
  CVector3f position;
  CVector3f direction;
  position.SetX(*(float*)((char*)obj + 0x54));
  position.SetY(*(float*)((char*)obj + 0x58));
  position.SetZ(*(float*)((char*)obj + 0x5c));
  float z = *(float*)((char*)obj + 0x48);
  y = *(float*)((char*)obj + 0x38);
  direction.SetX(*(float*)((char*)obj + 0x28));
  direction.SetY(y);
  direction.SetZ(z);
  if ((unsigned int)*(int*)((char*)obj + 0x5c4) != 0) {
    float f = ((CVector3f*)(obj + 424))->Magnitude();
    if (f > 0.8f) {
      int val = (int)f * 500 + 692;
      if (val < 0) {
        pitch = 0;
      } else {
        pitch = 16384;
        if (val <= 16384) {
          pitch = val;
        }
      }
      CSfxManager::PitchBend(*(CSfxHandle*)&(*((char*)obj + 0x5c4)), pitch);
    }
    CSfxManager::UpdateEmitter(*(CSfxHandle*)&(*((char*)obj + 0x5c4)), position, direction, 127);
  }
}
