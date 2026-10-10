// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
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
#include "MetroidPrime/CActorLights.hpp"
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
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CStaticInterference.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_14_9634() {}

extern "C" bool fn_14_0() { return true; }

extern "C" bool fn_14_8() { return true; }

extern "C" int fn_14_88(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_14_90() { return false; }

extern "C" bool fn_14_98() { return false; }

extern "C" bool fn_14_A0() { return false; }

extern "C" bool fn_14_A8() { return false; }

extern "C" int fn_14_D8(int arg0) { return arg0 + 1876; }

extern "C" bool fn_14_E0() { return true; }

extern "C" bool fn_14_E8() { return false; }

extern "C" bool fn_14_966C(int arg0) { return *(unsigned char*)(arg0 + 0x128c) >> 2 & 1; }

extern "C" void fn_14_7B84(int arg0) { *(float*)(arg0 + 0x11e4) = *(float*)(arg0 + 0x938); }

extern "C" bool fn_14_9A9C(int arg0) { return *(unsigned char*)(arg0 + 0x128c) >> 7 & 1; }

extern "C" bool fn_14_C0(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" float fn_14_CC() { return kDefaultGravityAccel; }

extern "C" void fn_14_B0(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" int fn_14_9AA8(int arg0) { return ((*(int*)(arg0 + 0x6b4)) == 3) ? 1 : 0; }

extern "C" bool fn_14_9A48(int arg0) {
  int temp_r4 = *(int*)(arg0 + 0x1284);
  return (unsigned int)(-1 - temp_r4 | temp_r4 + 1) >> 31;
}

extern "C" void fn_14_F0(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void fn_14_17C();
extern "C" void RELMain() { fn_14_17C(); }

extern "C" void fn_14_528() {
  void fn_14_548();
  fn_14_548();
}

extern "C" void fn_14_5F5C() {
  void fn_14_5F7C();
  fn_14_5F7C();
}

extern "C" void fn_14_9FE4() { &CDamageVulnerability::PassThroughVulnerabilty(); }

extern "C" void fn_14_78(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" void fn_14_A384(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRender(*(CStateManager*)arg1);
}

extern "C" void fn_14_D7BC() { &CDamageVulnerability::PassThroughVulnerabilty(); }

extern "C" void fn_14_A688(int arg0, int arg1, float arg2) {
  ((CPatterned*)arg0)->CPatterned::PreThink(arg2, *(CStateManager*)arg1);
}

extern "C" void RELExit() { SetSDigitalGuardian_FuncPtrs(nullptr); }

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
extern "C" void fn_14_10C(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" int fn_14_21C8();
extern "C" int fn_14_21A0() { return (fn_14_21C8() == 0) ? 1 : 0; }

extern "C" int fn_14_6A20(int arg0) {
  void fn_14_6A50();
  fn_14_6A50();
  return arg0;
}

extern "C" int fn_14_70B4(int arg0) {
  s32 temp_r0;
  s32 var_r4 = false;
  if (!(*(unsigned char*)(arg0 + 0x128c) >> 3 & 1)) {
    temp_r0 = *(int*)(arg0 + 0x1268);
    if (temp_r0 == 2 || temp_r0 == 1) {
      var_r4 = true;
    }
  }
  return var_r4;
}

extern float lbl_14_rodata_264;
extern "C" bool fn_14_9638(int arg0) {
  if ((*(int*)(arg0 + 0x11b8)) == 1) {
    return *(float*)(arg0 + 0x11c8) <= lbl_14_rodata_264;
  } else {
    return false;
  }
}

extern unsigned char lbl_14_bss_0;
extern "C" void fn_14_A3A4(int arg0, int arg1, int arg2, int arg3) {
  if (lbl_14_bss_0) {
    ((CPatterned*)arg0)
        ->CPatterned::RenderSystemsToBeDrawnLast(*(const CStateManager*)arg1, arg2, arg3);
  }
}

extern float lbl_14_rodata_2C4;
extern float lbl_14_rodata_2C8;
extern float lbl_14_rodata_2CC;
extern "C" float fn_14_6C74(int arg0) {
  if ((*(int*)(arg0 + 0x11b8))) {
    return lbl_14_rodata_2C4;
  } else if ((*(unsigned char*)(arg0 + 0x128c) >> 6 & 1)) {
    return lbl_14_rodata_2C8;
  } else {
    return lbl_14_rodata_2CC;
  }
}

extern "C" int fn_14_8384(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_8544(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_9A0C(int arg0) {
  int fn_14_705C();
  if ((*(int*)(0x1284 + arg0)) != -1) {
    if (!fn_14_705C()) {
      return 1;
    }
    return 0;
  } else {
    return 0;
  }
}

extern "C" int fn_14_9A60(int arg0) {
  float temp_f1 = lbl_14_rodata_264;
  int var_r0 = false;
  if (temp_f1 != *(float*)(arg0 + 0x1278) || temp_f1 != *(float*)(arg0 + 0x127c) ||
      temp_f1 != *(float*)(arg0 + 0x1280)) {
    var_r0 = true;
  }
  return var_r0;
}

extern "C" int fn_14_A348(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_C380(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_C3BC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_C3F8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_C434(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_14_C470(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_14_4838(int arg0, int arg1) {
  ((CCameraShakerManager*)*(int*)((*(int*)(arg1 + 0x151c)) + 0x88))
      ->RemoveCameraShaker(*(int*)(arg0 + 0x11c4));
  *(int*)(arg0 + 0x11c4) = -1;
}

struct CScannableObjectInfo;
struct CScannableObjectInfo {};
extern "C" int fn_14_9DBC(int arg0) {
  int temp_r4;
  if ((*(unsigned char*)(arg0 + 0x128c) >> 6 & 1)) {
    temp_r4 = *(int*)(arg0 + 0xe68);
    if ((unsigned int)temp_r4 != 0) {
      return *(int*)(temp_r4 + 0x8);
    }
  }
  return (int)((CPatterned*)arg0)->CPatterned::GetScannableObjectInfo();
}

extern "C" void fn_14_7B90(int arg0) {
  int fn_14_705C();
  float var_f0;
  if (fn_14_705C() < 4) {
    var_f0 = *(float*)(arg0 + 0x930);
  } else {
    var_f0 = *(float*)(arg0 + 0x934);
  }
  *(float*)(arg0 + 0x1274) = var_f0;
}

extern int lbl_14_data_688;
extern "C" int fn_14_8048(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_688;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_2040(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_6C24(int arg0, int arg1) {
  s32 var_r31 = false;
  if (!(*(int*)((*(int*)(arg0 + 0x48c)) + 0x37c)) ||
      !((CVector3f*)(arg1 + 4136))->IsMagnitudeSafe()) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_14_32AC(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_3BA8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_49D8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_C524(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_67C;
extern "C" int fn_14_7FEC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_67C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_670;
extern "C" int fn_14_8328(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_670;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_664;
extern "C" int fn_14_8738(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_664;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_64C;
extern "C" int fn_14_8C70(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_64C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_658;
extern "C" int fn_14_8978(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_658;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_640;
extern "C" int fn_14_9014(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_640;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_7D0(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x4c))) {
      ((CModelData*)arg0)->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_76C(int arg0, int arg1) {
  if (arg0) {
    if (*(unsigned char*)arg0) {
      delete (CAnimData*)*(int*)(arg0 + 0x4);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_4910(int arg0, int arg1) {
  void fn_14_4980(int, int);
  if (arg0) {
    fn_14_4980(arg0 + 160, -1);
    fn_14_4980(arg0 + 92, -1);
    fn_14_4980(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_14_CE1C(int arg0, int arg1, int arg2) {
  if ((unsigned char)arg2) {
    ((CActor*)arg0)->AddMaterial((EMaterialTypes)41, *(CStateManager*)arg1);
  } else {
    ((CActor*)arg0)->RemoveMaterial((EMaterialTypes)41, *(CStateManager*)arg1);
    ((CPlayer*)*(int*)(arg1 + 0x14fc))
        ->SetOrbitRequestForTarget(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x8)),
                                   (CPlayer::EPlayerOrbitRequest)8, *(CStateManager*)arg1);
  }
}

extern "C" int fn_14_830(int arg0, int arg1) {
  void fn_14_10D8(int, int);
  if (arg0) {
    fn_14_10D8(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_14_570();
extern "C" void fn_14_548(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_14_570();
  }
}

extern "C" void fn_14_5FA4();
extern "C" void fn_14_5F7C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_14_5FA4();
  }
}

extern "C" void fn_14_6A98();
extern "C" void fn_14_6A50(int arg0) {
  void fn_14_528();
  if (!(*(unsigned char*)(arg0 + 0x4c))) {
    fn_14_528();
    *(unsigned char*)(arg0 + 0x4c) = 1;
  } else {
    fn_14_6A98();
  }
}

extern "C" int fn_14_1FE8(int arg0, int arg1) {
  void fn_14_2040(int, int);
  if (arg0) {
    fn_14_2040(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_4980(int arg0, int arg1) {
  void fn_14_49D8(int, int);
  if (arg0) {
    fn_14_49D8(arg0 + 8, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_705C(int arg0) {
  int var_r5 = 0;
  unsigned short temp_r4 = kInvalidUniqueId.value;
  if ((*(unsigned short*)(arg0 + 0xfb0)) != (unsigned int)temp_r4) {
    var_r5 = 1;
  }
  if ((*(unsigned short*)(arg0 + 0xfb2)) != (unsigned int)temp_r4) {
    var_r5 = var_r5 + 1;
  }
  if ((*(unsigned short*)(arg0 + 0xfb4)) != (unsigned int)temp_r4) {
    var_r5 = var_r5 + 1;
  }
  if ((*(unsigned short*)(arg0 + 0xfb6)) != (unsigned int)temp_r4) {
    var_r5 = var_r5 + 1;
  }
  return var_r5;
}

struct __mwdec_vt_0_fn_14_C0E4 {
  virtual void _0(int);
};
extern "C" int fn_14_C0E4(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_14_C0E4*)temp_r3)->_0(1);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_14_data_204[224];
extern unsigned char lbl_14_data_380[208];
extern unsigned char lbl_14_data_9C[192];
struct __mwdec_vt_0_fn_14_9DFC {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4(unsigned char*, int);
};
struct __mwdec_vt_1 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3(unsigned char*, int);
};
struct __mwdec_vt_2 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5(unsigned char*, int);
};
extern "C" void fn_14_9DFC(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_14_9DFC*)temp_r31)->_4(lbl_14_data_9C, 12);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_14_data_204, 14);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_14_data_380, 13);
}

extern "C" void fn_14_1A68(int, int, int, int);
extern "C" void fn_14_24E4(int arg0, int arg1) {
  int var_r30 = arg0 + 4420;
  int var_r29 = arg0 + 4316;
  int var_r28 = arg0 + 4440;
  int var_r27 = 0;
  do {
    *(float*)var_r30 = *(float*)(arg0 + 0x7e0);
    *(int*)var_r29 = -1;
    *(float*)var_r28 = 0.0f;
    fn_14_1A68(arg0, arg1, var_r27, 1);
    var_r27 = var_r27 + 1;
    var_r30 = var_r30 + 4;
    var_r29 = var_r29 + 4;
    var_r28 = var_r28 + 4;
  } while (var_r27 < 4);
}

struct __mwdec_vt_0_fn_14_A3D4 {
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
  virtual void _15();
  virtual void _16();
  virtual void _17();
  virtual void _18();
  virtual void _19();
  virtual void _20();
  virtual void _21();
  virtual void _22();
  virtual void _23();
  virtual void _24();
  virtual void _25();
  virtual void _26();
  virtual void _27();
  virtual void _28();
  virtual void _29();
  virtual void _30();
  virtual void _31();
  virtual void _32();
  virtual void _33();
  virtual void _34();
  virtual void _35();
  virtual void _36();
  virtual void _37();
  virtual void _38();
  virtual void _39();
  virtual void _40();
  virtual void _41();
  virtual void _42();
  virtual void _43();
  virtual void _44();
  virtual void _45(int, int, int);
};
extern "C" void fn_14_A3D4(int arg0, int arg1) {
  unsigned int stack_c;
  unsigned int stack_8;
  lbl_14_bss_0 = 1;
  *(int*)&stack_c = 0;
  *(int*)&stack_8 = 0;
  if ((*(unsigned char*)(arg0 + 0x421) & 1)) {
    ((CStateManager*)arg1)->GetCharacterRenderMaskAndTarget(stack_c, stack_8);
  }
  ((__mwdec_vt_0_fn_14_A3D4*)arg0)->_45(arg1, *(int*)&stack_c, *(int*)&stack_8);
  lbl_14_bss_0 = 0;
}

extern "C" void fn_14_4AD8(int, int);
extern "C" int fn_14_4A2C(int arg0, int arg1) {
  *(int*)arg0 = *(int*)arg1;
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x4);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x8);
  *(float*)(arg0 + 0xc) = *(float*)(arg1 + 0xc);
  *(float*)(arg0 + 0x10) = *(float*)(arg1 + 0x10);
  *(float*)(arg0 + 0x14) = *(float*)(arg1 + 0x14);
  fn_14_4AD8(arg0 + 24, arg1 + 24);
  fn_14_4AD8(arg0 + 92, arg1 + 92);
  fn_14_4AD8(arg0 + 160, arg1 + 160);
  *(int*)(arg0 + 0xe4) = *(int*)(arg1 + 0xe4);
  *(float*)(arg0 + 0xe8) = *(float*)(arg1 + 0xe8);
  *(float*)(arg0 + 0xec) = *(float*)(arg1 + 0xec);
  *(float*)(arg0 + 0xf0) = *(float*)(arg1 + 0xf0);
  return arg0;
}

extern "C" void fn_14_5910(int arg0, int arg1) {
  *(unsigned short*)(arg0 + 0x11c0) =
      ((CEntity*)arg0)
          ->FindConnectedObject(*(const CStateManager*)arg1, (EScriptObjectState)0x49533030,
                                (EScriptObjectMessage)0x41544348)
          .value;
  *(unsigned short*)(arg0 + 0x10ce) =
      ((CEntity*)arg0)
          ->FindConnectedObject(*(const CStateManager*)arg1, (EScriptObjectState)0x49533034,
                                (EScriptObjectMessage)0x41435456)
          .value;
  *(unsigned short*)(arg0 + 0x116a) =
      ((CEntity*)arg0)
          ->CheckConnectedObject(*(const CStateManager*)arg1, (EScriptObjectState)0x49533035,
                                 (EScriptObjectMessage)0x464f4c57)
          .value;
}

extern "C" void fn_14_38BC(int arg0, int arg1, float arg2) {
  int fn_14_705C();
  int var_r0;
  int temp_r3;
  *(float*)(arg0 + 0x11e0) = *(float*)(arg0 + 0x11e0) - arg2;
  *(float*)(arg0 + 0x11e4) = *(float*)(arg0 + 0x11e4) - arg2;
  if (fn_14_705C() < 4) {
    temp_r3 = *(int*)(arg1 + 0x14fc);
    if ((*(int*)(temp_r3 + 0x390)) == 0) {
      var_r0 = *(int*)(temp_r3 + 0x38c);
    } else {
      var_r0 = 0;
    }
    if (var_r0 != 1) {
      *(float*)(arg0 + 0x1274) = *(float*)(arg0 + 0x1274) - arg2;
    } else {
      *(float*)(arg0 + 0x1274) = *(float*)(arg0 + 0x930);
    }
  } else {
    *(float*)(arg0 + 0x1274) = *(float*)(arg0 + 0x1274) - arg2;
  }
}

extern "C" void fn_14_2358(int, int, float);
extern "C" void fn_14_3F90(int, int);
extern "C" void fn_14_676C(int, int);
extern "C" void fn_14_6E04(int, int, float);
extern "C" void fn_14_A5BC(int arg0, int arg1, float arg2) {
  void fn_14_38BC(int, int, float);
  int temp_r3;
  if (((*(unsigned char*)(arg0 + 0x20)) >> 7 & 1)) {
    ((CPatterned*)arg0)->CPatterned::Think(arg2, *(CStateManager*)arg1);
    temp_r3 = *(int*)(arg0 + 0xe60);
    if ((unsigned int)temp_r3 == 0) {
      fn_14_676C(arg0, arg1);
    } else {
      ((CCollisionActorManager*)temp_r3)
          ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
      ((CCollisionActorManager*)*(int*)(arg0 + 0xe64))
          ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
    }
    fn_14_6E04(arg0, arg1, arg2);
    fn_14_2358(arg0, arg1, arg2);
    fn_14_38BC(arg0, arg1, arg2);
    fn_14_3F90(arg0, arg1);
  }
}

extern "C" CTransform4f fn_14_2578(int arg0, int arg1, int arg2) {
  float temp_f6;
  CTransform4f stack_80(*((CTransform4f*)arg1) *
                        ((CActor*)arg0)->GetScaledLocatorTransform(*(const CSegId*)(arg0 + 4281)));
  if ((*(unsigned char*)(arg0 + 0x1238))) {
    temp_f6 = (*(float*)(arg0 + 0x1264) - 0.5f) *
              (0.8f * ((CModelData*)(arg0 + 4588))->GetBounds().GetDepth());
    stack_80.AddTranslation(stack_80.GetUp() * temp_f6);
  }
  return CTransform4f(stack_80);
}

extern "C" int fn_14_10D8(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 1252))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 904))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 556))->~SLdrDamageVulnerability();
    ((SLdrAudioPlaybackParms*)(arg0 + 532))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 508))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 472))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 448))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 424))->~SLdrAudioPlaybackParms();
    ((SLdrEchoParameters*)(arg0 + 404))->~SLdrEchoParameters();
    ((SLdrEchoParameters*)(arg0 + 348))->~SLdrEchoParameters();
    ((SLdrAudioPlaybackParms*)(arg0 + 320))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 296))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 260))->~SLdrAudioPlaybackParms();
    ((SLdrDamageInfo*)(arg0 + 232))->~SLdrDamageInfo();
    ((SLdrShockWaveInfo*)(arg0 + 164))->~SLdrShockWaveInfo();
    ((SLdrAudioPlaybackParms*)(arg0 + 140))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 116))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 92))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 68))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(arg0 + 44))->~SLdrAudioPlaybackParms();
    ((SLdrDamageInfo*)(arg0 + 16))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_14_6864(int arg0) {
  void fn_14_6A20(int, CModelData*);
  int temp_r3;
  unsigned char stack_48[16];
  unsigned char stack_38[16];
  unsigned char stack_28[16];
  unsigned char stack_18[16];
  unsigned char stack_8[16];
  int temp_r4 = *(int*)(arg0 + 0x8dc);
  temp_r3 = *(int*)(arg0 + 0x60);
  float temp_f31 = *(float*)temp_r3;
  float temp_f30 = *(float*)(temp_r3 + 0x4);
  float temp_f29 = *(float*)(temp_r3 + 0x8);
  if ((unsigned int)(temp_r4 + 0x10000) != 0xffff) {
    *(int*)stack_48 = temp_r4;
    *(float*)(stack_48 + 0x4) = temp_f31;
    *(float*)(stack_48 + 0x8) = temp_f30;
    *(float*)(stack_48 + 0xc) = temp_f29;
    CModelData stack_188(*(const CStaticRes*)stack_48);
    fn_14_6A20(arg0 + 3692, &stack_188);
  }
  int temp_r3_2 = *(int*)(arg0 + 0x8e0);
  if ((unsigned int)(temp_r3_2 + 0x10000) != 0xffff) {
    *(int*)stack_38 = temp_r3_2;
    *(float*)((char*)stack_38 + 0x4) = temp_f31;
    *(float*)(stack_38 + 0x8) = temp_f30;
    *(float*)(stack_38 + 0xc) = temp_f29;
    CModelData stack_13c(*(const CStaticRes*)stack_38);
    fn_14_6A20(arg0 + 3772, &stack_13c);
  }
  int temp_r3_3 = *(int*)(arg0 + 0x918);
  if ((unsigned int)(temp_r3_3 + 0x10000) != 0xffff) {
    *(int*)stack_28 = temp_r3_3;
    *(float*)(stack_28 + 0x4) = temp_f31;
    *(float*)(stack_28 + 0x8) = temp_f30;
    *(float*)(stack_28 + 0xc) = temp_f29;
    CModelData stack_f0(*(const CStaticRes*)stack_28);
    fn_14_6A20(arg0 + 3852, &stack_f0);
  }
  int temp_r3_4 = *(int*)(arg0 + 0x94c);
  if ((unsigned int)(temp_r3_4 + 0x10000) != 0xffff) {
    *(int*)stack_18 = temp_r3_4;
    *(float*)(stack_18 + 0x4) = temp_f31;
    *(float*)(stack_18 + 0x8) = temp_f30;
    *(float*)(stack_18 + 0xc) = temp_f29;
    CModelData stack_a4(*(const CStaticRes*)stack_18);
    fn_14_6A20(arg0 + 4588, &stack_a4);
  }
  int temp_r3_5 = *(int*)(arg0 + 0x7e4);
  if ((unsigned int)(temp_r3_5 + 0x10000) != 0xffff) {
    *(int*)stack_8 = temp_r3_5;
    *(float*)(stack_8 + 0x4) = temp_f31;
    *(float*)(stack_8 + 0x8) = temp_f30;
    *(float*)(stack_8 + 0xc) = temp_f29;
    CModelData stack_58(*(const CStaticRes*)stack_8);
    fn_14_6A20(arg0 + 3932, &stack_58);
  }
}

template < class T0 >
int TCastToPtr(CEntity*);
extern "C" void fn_14_143C(int obj, int obj2, int obj3, int val) {
  const __typeof__(((CEntity*)obj)
                       ->CheckConnectedObject(*(const CStateManager*)obj2, kSS_InternalState6,
                                              kSM_Follow)) temp_0 =
      ((CEntity*)obj)
          ->CheckConnectedObject(*(const CStateManager*)obj2, kSS_InternalState6, kSM_Follow);
  unsigned int val2 = TCastToPtr< CScriptWaypoint >(((CStateManager*)obj2)->ObjectById(temp_0));
  if (val2 > 0) {
    ((CActor*)val2)->SetTranslation(*(const CVector3f*)obj3);
    ((CEntity*)obj)
        ->SendScriptMsgs((EScriptObjectState)val, *(CStateManager*)obj2,
                         TUniqueId((ushort) * (unsigned short*)((char*)obj + 0x8)), kSM_None);
  }
}

extern "C" void fn_14_1994(int obj, int obj2, int obj3, int obj4) {
  int val;
  if (*(unsigned char*)((char*)obj + 0xeb8)) {
    int val2 = obj + 4288;
    int i = 0;
    val = *(int*)((char*)*(int*)((char*)*(int*)((char*)obj + 0x60) + 0x10) + 0x10c);
    do {
      unsigned char val3 = *(unsigned char*)val2;
      CTransform4f xf(
          (*((CTransform4f*)obj3) * ((CActor*)obj)->GetScaledLocatorTransform(CSegId(val3))) *
          ((CQuaternion*)(*(int*)((char*)val + 0x68) + (val3 << 4)))->BuildTransform4f());
      ((CModelData*)(obj + 3692))
          ->Render(*(const CStateManager*)obj2, xf, (const CActorLights*)*(int*)((char*)obj + 0xbc),
                   *(const CModelFlags*)obj4);
      i++;
      val2++;
    } while (i < 4);
  }
}

extern "C" void fn_14_18B4(int obj, int obj2, int obj3, int obj4) {
  int val;
  if (!((*(unsigned char*)((char*)obj + 0x128c)) >> 4 & 1) &&
      (*(unsigned char*)((char*)obj + 0xf08))) {
    int val2 = obj + 4288;
    int i = 0;
    val = *(int*)((char*)*(int*)((char*)*(int*)((char*)obj + 0x60) + 0x10) + 0x10c);
    do {
      unsigned char val3 = *(unsigned char*)val2;
      CTransform4f xf(
          (*((CTransform4f*)obj3) * ((CActor*)obj)->GetScaledLocatorTransform(CSegId(val3))) *
          ((CQuaternion*)(*(int*)((char*)val + 0x68) + (val3 << 4)))->BuildTransform4f());
      ((CModelData*)(obj + 3772))
          ->Render(*(const CStateManager*)obj2, xf, (const CActorLights*)*(int*)((char*)obj + 0xbc),
                   *(const CModelFlags*)obj4);
      i++;
      val2++;
    } while (i < 4);
  }
}

struct __mwdec_vt_0_fn_14_7CB4 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual int _5(int);
};
extern "C" void fn_14_2CC4();
extern "C" void fn_14_7CB4(int obj, int obj2, const int val) {
  switch (val) {
  case 0:
    ((CCollisionActorManager*)*(int*)((char*)obj + 0xe64))->SetActive(*(CStateManager*)obj2, true);
    for (unsigned int i = 0;
         ((CCollisionActorManager*)*(int*)((char*)obj + 0xe60))->GetNumCollisionActors() > i;
         i += 1) {
      TUniqueId collisionActorId =
          (&((CCollisionActorManager*)*(int*)((char*)obj + 0xe60))->GetCollisionDescFromIndex(i))
              ->GetCollisionActorId();
      unsigned int val2 =
          TCastToPtr< CCollisionActor >(((CStateManager*)obj2)->ObjectById(collisionActorId));
      if (val2 > 0 && collisionActorId.value != (*(unsigned short*)((char*)obj + 0x10cc))) {
        ((__mwdec_vt_0_fn_14_7CB4*)val2)->_5(0);
      }
    }
    break;
  case 1:
    fn_14_2CC4();
    break;
  }
}

extern "C" void fn_14_9164(int obj, int val, int val2, float f) {
  float f2;
  float a;
  switch (val2) {
  case 0:
    *(int*)((char*)obj + 0x6b4) = 1;
    break;
  case 1:
    a = *(float*)(0x1288 + (char*)obj);
    *(float*)((char*)obj + 0x3c8) =
        rstl::max_val(a, *(float*)((char*)obj + 0x3c8) - f * 0.6666667f);
    { f2 = *((float*)((char*)obj + 0x1288)); }
    {
      int val3 = 0x8bc;
      ((CActor*)obj)
          ->SetSoundEventPitchBend(CCast::ToUint32(((*(float*)((char*)obj + 0x3c8)) - f2) / f2 *
                                                   (*(int*)((char*)obj + val3))) +
                                   8192);
    }
    if (*(float*)((char*)obj + 0x3c8) <= *(float*)((char*)obj + 0x1288)) {
      *(int*)((char*)obj + 0x6b4) = 3;
    }
    break;
  case 2:
    *(int*)((char*)obj + 0x6b4) = 0;
    *(float*)(0x3c8 + (char*)obj) = *(float*)((char*)obj + 0x1288);
    break;
  }
}
