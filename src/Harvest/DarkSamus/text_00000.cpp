// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMRay.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObj.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CCylinder.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CFreezeBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CCollisionResponseData.hpp"
#include "Weapons/CDecalDescription.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "dolphin/gx/GXEnum.h"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" float fn_10_2EFC(int arg0) { return *(float*)(arg0 + 0x314); }

extern "C" void fn_10_29DC(int arg0, int arg1, int arg2) {
  ((CActor*)arg0)->CActor::AcceptScriptMsg(*(CStateManager*)arg1, *(const CScriptMsg*)arg2);
}

extern "C" CModelData fn_10_2E44(int arg0) { return CModelData(); }

extern "C" void fn_10_3520() {
  void fn_10_3540();
  fn_10_3540();
}

extern "C" bool fn_10_2F04(int arg0) {
  return CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0x318), 1, 6);
}

extern "C" void fn_10_E5C(int arg0, int arg1) {
  ((CActor*)arg0)->EnsureRendered(*(const CStateManager*)arg1);
}

extern "C" void fn_10_B28(int, int);
extern "C" void fn_10_B04(int arg0) { fn_10_B28(arg0, 0); }

struct __mwdec_vt_0 {
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
};
extern "C" void fn_10_0(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" void fn_10_1B24(int arg0, int arg1, int arg2) {
  float temp_f3;
  float temp_f4;
  float temp_f5;
  float temp_f6;
  temp_f5 = *(float*)(arg1 + 0x8);
  float temp_f7 = *(float*)(arg2 + 0x4);
  temp_f3 = *(float*)arg1;
  float temp_f2 = *(float*)(arg2 + 0x8);
  temp_f4 = *(float*)(arg1 + 0x4);
  temp_f6 = *(float*)arg2;
  *(float*)arg0 = temp_f4 * temp_f2 - temp_f7 * temp_f5;
  *(float*)(arg0 + 0x4) = temp_f5 * temp_f6 - temp_f2 * temp_f3;
  *(float*)(arg0 + 0x8) = temp_f3 * temp_f7 - temp_f6 * temp_f4;
}

extern int lbl_10_data_BC;
extern "C" int fn_10_ABC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_BC;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_2098(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_3174(int arg0, int arg1) {
  if (CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0x318), 1, 7) == 1 &&
      !((CPlayerState*)*(int*)(arg1 + 0x15fc))->GetActiveVisor(*(const CStateManager*)arg1)) {
    gpRender->AllocatePhazonSuitMaskTexture();
  }
  ((CActor*)arg0)->CActor::PreRender(*(CStateManager*)arg1);
}

extern "C" void fn_10_1C08(int, int);
extern "C" void fn_10_2254(int, int, float);
extern "C" void fn_10_2C(int, int, float);
extern "C" void fn_10_2918(int arg0, int arg1, float arg2) {
  ((CWeapon*)arg0)->CWeapon::Think(arg2, *(CStateManager*)arg1);
  if (!(arg2 <= 0.0f)) {
    *(float*)(arg0 + 0x1f0) = *(float*)(arg0 + 0x1f0) + arg2;
    fn_10_2254(arg0, arg1, arg2);
    fn_10_1C08(arg0, arg1);
    fn_10_2C(arg0, arg1, arg2);
    if (*(float*)(arg0 + 0x1f0) > 10.0f) {
      ((CStateManager*)arg1)
          ->DeleteObjectRequest(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x8)));
    }
  }
}

extern "C" void fn_10_31E4(int, int);
extern "C" void fn_10_3B08(int arg0, int arg1, float arg2) {
  ((CActor*)arg0)->CActor::Think(arg2, *(CStateManager*)arg1);
  if ((unsigned int)*(int*)(arg0 + 0x354) == 0) {
    fn_10_31E4(arg0, arg1);
  }
  ((CCollisionActorManager*)*(int*)(arg0 + 0x354))
      ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
  ((CActor*)arg0)->UpdateAnimation(2.0f * arg2, *(CStateManager*)arg1, true);
  *(float*)(arg0 + 0x310) = rstl::max_val(*(float*)(arg0 + 0x310) - arg2, 0.0f);
}

extern "C" void fn_10_3568();
extern "C" void fn_10_3540(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_10_3568();
  }
}

extern "C" int fn_10_2040(int arg0, int arg1) {
  void fn_10_2098(int, int);
  if (arg0) {
    fn_10_2098(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_10_2798 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_1 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_2 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_3 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_4 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_5 {
  virtual void _0();
  virtual void _1(int, int);
};
struct __mwdec_vt_6 {
  virtual void _0();
  virtual void _1(int, int);
};
extern "C" void fn_10_2798(int arg0) {
  int temp_r3;
  int temp_r3_2;
  int temp_r3_3;
  int temp_r3_4;
  int temp_r3_5;
  int temp_r3_6;
  int temp_r3_7;
  CRandom16 stack_8(99);
  CGlobalRandom stack_c(stack_8);
  CParticleGlobals::SetEmitterTime(0);
  int temp_r31 = *(int*)(arg0 + 0x1d0);
  if ((unsigned int)*(int*)(temp_r31 + 0x24) != 0) {
    temp_r3 = *(int*)(temp_r31 + 0x30);
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_10_2798*)temp_r3)->_1(0, arg0 + 540);
    }
    temp_r3_2 = *(int*)(temp_r31 + 0x28);
    if ((unsigned int)temp_r3_2 != 0) {
      ((__mwdec_vt_1*)temp_r3_2)->_1(0, arg0 + 504);
    }
  }
  if ((unsigned int)*(int*)(temp_r31 + 0x38) != 0) {
    temp_r3_3 = *(int*)(temp_r31 + 0x44);
    if ((unsigned int)temp_r3_3 != 0) {
      ((__mwdec_vt_2*)temp_r3_3)->_1(0, arg0 + 544);
    }
    temp_r3_4 = *(int*)(temp_r31 + 0x3c);
    if ((unsigned int)temp_r3_4 != 0) {
      ((__mwdec_vt_3*)temp_r3_4)->_1(0, arg0 + 516);
    }
  }
  if ((unsigned int)*(int*)(temp_r31 + 0x4c) != 0) {
    temp_r3_5 = *(int*)(temp_r31 + 0x5c);
    if ((unsigned int)temp_r3_5 != 0) {
      ((__mwdec_vt_4*)temp_r3_5)->_1(0, arg0 + 548);
    }
    temp_r3_6 = *(int*)((char*)temp_r31 + 0x60);
    if ((unsigned int)temp_r3_6 != 0) {
      ((__mwdec_vt_5*)temp_r3_6)->_1(0, arg0 + 552);
    }
    temp_r3_7 = *(int*)(temp_r31 + 0x50);
    if ((unsigned int)temp_r3_7 != 0) {
      ((__mwdec_vt_6*)temp_r3_7)->_1(0, arg0 + 528);
    }
  }
}
