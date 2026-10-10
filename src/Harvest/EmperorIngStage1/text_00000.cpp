// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CGameSplineDesc.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/CWarp.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrHealthInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_16_4AE8() {}

extern "C" void fn_16_BE98() {}

extern "C" bool fn_16_B9AC() { return false; }

extern "C" int fn_16_B9A4(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_16_B9B4() { return false; }

extern "C" bool fn_16_B9BC() { return false; }

extern "C" bool fn_16_B9C4() { return false; }

extern "C" int fn_16_B9F4(int arg0) { return arg0 + 1876; }

extern "C" bool fn_16_B9FC() { return true; }

extern "C" bool fn_16_BA04() { return false; }

extern "C" bool fn_16_BA0C() { return false; }

extern "C" void fn_16_BE9C(int arg0, int arg1) { *(int*)(arg0 + 0x1c) = arg1; }

extern "C" int fn_16_BEB0(int arg0) { return *(int*)(arg0 + 0x1c); }

extern "C" float fn_16_B9E8() { return kDefaultGravityAccel; }

extern "C" bool fn_16_B9DC(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern float lbl_16_rodata_3B4;
extern "C" float fn_16_BEA4() { return lbl_16_rodata_3B4; }

extern "C" void fn_16_B9CC(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" bool fn_16_4AEC(int arg0) { return *(float*)(arg0 + 0xa50) > *(float*)(arg0 + 0xc34); }

extern float lbl_16_rodata_380;
extern "C" bool fn_16_4B04(int arg0) { return *(float*)(arg0 + 0xa4c) < lbl_16_rodata_380; }

extern "C" void fn_16_BA14(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_16_A270();
  fn_16_A270();
}

extern "C" void fn_16_22B0() {
  void fn_16_22D0();
  fn_16_22D0();
}

extern "C" void fn_16_39CC() {
  void fn_16_39EC();
  fn_16_39EC();
}

extern "C" void fn_16_46D4() {
  void fn_16_46F4();
  fn_16_46F4();
}

extern "C" bool fn_16_4BB4(int arg0) { return *(float*)(arg0 + 0xa3c) <= lbl_16_rodata_380; }

extern "C" void fn_16_52FC(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRender(*(CStateManager*)arg1);
}

extern "C" void fn_16_86BC() {
  void fn_16_86DC();
  fn_16_86DC();
}

extern "C" void fn_16_8938();
extern "C" void fn_16_8918() { fn_16_8938(); }

extern "C" void fn_16_A5FC() {
  void fn_16_A61C();
  fn_16_A61C();
}

extern "C" void fn_16_BB1C() {
  void fn_16_22D0();
  fn_16_22D0();
}

extern "C" void fn_16_B994(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" void RELExit() { SetSEmperorIngStage1_FuncPtrs(nullptr); }

extern "C" void fn_16_1DFC(int arg0) {
  *(float*)(arg0 + 0x4) = lbl_16_rodata_380;
  if ((*(int*)(arg0 + 0x28)) == 2) {
    return;
  }
  *(int*)(arg0 + 0x28) = 1;
}

extern "C" int fn_16_231C();
extern "C" bool fn_16_22F8() { return (unsigned int)fn_16_231C() >> 31; }

extern "C" void fn_16_4E44(int arg0, int arg1) {
  *(int*)arg0 = *(int*)(arg1 + 0x78);
  *(int*)(arg0 + 0x4) = *(int*)(arg1 + 0x7c);
  int temp_r4 = *(int*)(arg0 + 0x4);
  *(int*)temp_r4 = *(int*)temp_r4 + 1;
}

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
extern "C" void fn_16_60(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern void* lbl_16_bss_0;
extern "C" void fn_16_A2A0();
extern "C" void fn_16_A270() {
  lbl_16_bss_0 = &fn_16_A2A0;
  SetSEmperorIngStage1_FuncPtrs((SEmperorIngStage1_FuncPtrs*)&(*(int*)&lbl_16_bss_0));
}

extern "C" int fn_16_8980(int arg0) {
  void fn_16_86BC();
  *(unsigned char*)(arg0 + 0x14) = 1;
  fn_16_86BC();
  return arg0;
}

extern "C" int fn_16_3340(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_16_3500(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_16_55D4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_16_48C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_16_5610(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_16_8844(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_16_8E4(int arg0, int arg1) {
  *(float*)(arg0 + 0xa4c) =
      ((CRandom16*)(arg1 + 5860))->Range(*(float*)(arg0 + 0xcf4), *(float*)(arg0 + 0xcf8));
}

extern "C" int fn_16_8880(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_16_2DDC(int arg0, int arg1, int arg2) {
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

extern int lbl_16_data_3BC;
extern "C" int fn_16_5C04(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_16_data_3BC;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_16_rodata_3D8;
extern "C" bool fn_16_4B20(int arg0, int arg1) {

  int temp_r6 = *(int*)(arg1 + 0x14fc);
  float temp_f3 = *(float*)(arg0 + 0x54) - *(float*)(temp_r6 + 0x54);
  float temp_f1 = *(float*)(arg0 + 0x58) - *(float*)(temp_r6 + 0x58);
  return lbl_16_rodata_380 + (temp_f3 * temp_f3 + temp_f1 * temp_f1) < lbl_16_rodata_3D8;
}

extern "C" int fn_16_2A68(int arg0, int arg1) {
  void fn_16_BD6C();
  if (arg0) {
    fn_16_BD6C();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

template < class T0 >
void TCastToPtr(CEntity*);
extern "C" void fn_16_2FA8(int arg0, int arg1) {
  TCastToPtr< CCollisionActor >(
      ((CStateManager*)arg1)
          ->ObjectById(
              (&((CCollisionActorManager*)*(int*)(arg0 + 0x7cc))->GetCollisionDescFromIndex(0))
                  ->GetCollisionActorId()));
}

extern "C" int fn_16_4E68(int arg0, int arg1) {
  void fn_16_BDD0();
  if (arg0) {
    fn_16_BDD0();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_716C(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    ((CEntity*)arg0)
        ->SendScriptMsgs((EScriptObjectState)0x44454144, *(CStateManager*)arg1, kInvalidUniqueId,
                         (EScriptObjectMessage)-1);
    break;
  }
}

extern "C" int fn_16_2AB8(int arg0, int arg1) {
  void fn_16_BD6C();
  if (arg0) {
    if (arg0) {
      fn_16_BD6C();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_3AA8(int, int, int, int);
extern "C" void fn_16_1E20(int arg0, int arg1, int arg2) {
  int temp_r3 = fn_16_3AA8(*(int*)arg0, arg1, *(int*)(arg0 + 0x8), 4);
  if ((unsigned int)temp_r3 != 0) {
    ((CCollisionActor*)temp_r3)->SetDamageVulnerability(*(const CDamageVulnerability*)arg2);
  }
}

extern "C" int fn_16_4DF0(int arg0, int arg1) {
  void fn_16_BDD0();
  if (arg0) {
    if (arg0) {
      fn_16_BDD0();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_6844(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_8AA0(int arg0, int arg1) {
  void fn_16_88BC(int, int);
  if (arg0) {
    fn_16_88BC(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_8FB8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_3B70(int arg0, int arg1) {
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7c8))->Destroy(*(CStateManager*)arg1);
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7cc))->Destroy(*(CStateManager*)arg1);
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7d0))->Destroy(*(CStateManager*)arg1);
}

extern "C" int fn_16_992C(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_9E48(int arg0, int arg1) {
  void fn_16_A1D4(int, int);
  if (arg0) {
    fn_16_A1D4(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_9FB0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_B668(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)arg0)->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_9B90(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_B130(int arg0, int arg1) {
  if (arg0) {
    ((SLdrShockWaveInfo*)(arg0 + 8))->~SLdrShockWaveInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_24AC(int);
extern "C" void fn_16_2524(int, int, float);
extern "C" void fn_16_26B8();
extern "C" void fn_16_2B0C(int arg0, int arg1, float arg2) {
  fn_16_26B8();
  fn_16_2524(arg0, arg1, arg2);
  fn_16_24AC(arg0);
}

extern int lbl_16_data_3B0;
extern "C" int fn_16_5BA8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_16_data_3B0;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_16_data_3BC;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_B52C(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 4))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_16_data_3A4;
extern "C" int fn_16_5D60(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_16_data_3A4;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_16_data_3BC;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_16_data_398;
extern "C" int fn_16_76F0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_16_data_398;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_16_data_3BC;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_2C9C(int, int, unsigned char*);
extern "C" int fn_16_3060();
extern "C" void fn_16_2C3C(int arg0, int arg1) {
  float temp_f1;
  unsigned char stack_8[16];
  int temp_r3 = fn_16_3060();
  float temp_f2 = *(float*)(temp_r3 + 0x48);
  temp_f1 = *(float*)(temp_r3 + 0x38);
  *(float*)stack_8 = *(float*)(temp_r3 + 0x28);
  *(float*)(stack_8 + 0x4) = temp_f1;
  *(float*)(stack_8 + 0x8) = temp_f2;
  fn_16_2C9C(arg0, arg1, stack_8);
}

extern "C" void fn_16_2E1C(int arg0, int arg1, int arg2) {
  void fn_16_1F34(int, int, int);
  int var_r31;
  int var_r30 = 0;
  var_r31 = arg0 + 2008;
  while (var_r30 < (*(int*)(arg0 + 0x7d4))) {
    fn_16_1F34(var_r31, arg1, arg2);
    var_r31 = var_r31 + 72;
    var_r30 = var_r30 + 1;
  }
}

extern "C" int fn_16_A8A4(int arg0, int arg1) {
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

extern "C" int fn_16_2F70();
extern "C" int fn_16_2E7C(int arg0) {
  int i;
  int var_r31 = 0;
  int temp_r3 = fn_16_2F70();
  int var_r4 = arg0 + 2008;
  for (i = 0; i < temp_r3; i++) {
    if (!(*(int*)(var_r4 + 0x28))) {
      var_r31 = var_r31 + 1;
    }
    var_r4 = var_r4 + 72;
  }
  return var_r31;
}

extern "C" int fn_16_AA70(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 72))->~SLdrDamageInfo();
    ((SLdrPlasmaBeamInfo*)arg0)->~SLdrPlasmaBeamInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_B378(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 16))->~SLdrDamageInfo();
    ((SLdrAnimationSet*)(arg0 + 4))->~SLdrAnimationSet();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_16_data_510;
extern "C" int fn_16_BE34(int arg0, int arg1) {
  void fn_16_BF14(int, int);
  if (arg0) {
    *(int*)arg0 = (int)&lbl_16_data_510;
    fn_16_BF14(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_A840(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)arg0)) {
      delete (CAnimData*)*(int*)(arg0 + 0x4);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_9F04();
extern "C" int fn_16_9E9C(int arg0, int arg1, int arg2, const int arg3, float arg4) {
  fn_16_9F04();
  *(int*)(arg0 + 0x44) = arg2;
  *(float*)(arg0 + 0x48) = arg4;
  *(unsigned char*)(arg0 + 0x4c) =
      (arg3 << 7 & 128) | (*(unsigned char*)(arg0 + 0x4c) & 0xffffff7f);
  return arg0;
}

extern "C" void fn_16_3AFC(int arg0, int arg1, float arg2) {
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7c8))
      ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7cc))
      ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7d0))
      ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
}

extern "C" void fn_16_3668(int arg0, int arg1, float arg2) {
  void fn_16_2B0C(int, int, float);
  int var_r31;
  int var_r30 = 0;
  var_r31 = arg0 + 2008;
  while (var_r30 < (*(int*)(arg0 + 0x7d4))) {
    fn_16_2B0C(var_r31, arg1, arg2);
    var_r31 = var_r31 + 72;
    var_r30 = var_r30 + 1;
  }
}

extern "C" int fn_16_A904(int arg0, int arg1) {
  void fn_16_AEDC(int, int);
  if (arg0) {
    fn_16_AEDC(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_22D0(int arg0, int arg1, int arg2) {
  void fn_16_22F8(int, int);
  fn_16_22F8(arg1, arg2);
}

extern "C" void fn_16_3A14();
extern "C" void fn_16_39EC(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_16_3A14();
  }
}

extern "C" void fn_16_471C();
extern "C" void fn_16_46F4(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_16_471C();
  }
}

extern "C" void fn_16_8704();
extern "C" void fn_16_86DC(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_16_8704();
  }
}

extern "C" void fn_16_A644();
extern "C" void fn_16_A61C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_16_A644();
  }
}

struct __mwdec_vt_0_fn_16_4B6C {
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
  virtual int _13();
};
extern "C" bool fn_16_4B6C(int arg0) {
  return (*(float*)(((__mwdec_vt_0_fn_16_4B6C*)arg0)->_13() + 0x4)) <= lbl_16_rodata_380;
}

extern "C" int fn_16_67EC(int arg0, int arg1) {
  void fn_16_6844(int, int);
  if (arg0) {
    fn_16_6844(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_A1D4(int arg0, int arg1) {
  void fn_16_9FB0(int, int);
  if (arg0) {
    fn_16_9FB0(arg0 + 8, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_16_88BC(int arg0, int arg1) {
  void fn_16_8918();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_16_8918();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_16_BD6C {
  virtual void _0(int);
};
extern "C" void fn_16_BD6C(int arg0) {
  int temp_r3;
  if (--*(int*)(*(int*)(arg0 + 0x4)) <= 0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_16_BD6C*)temp_r3)->_0(1);
    }
    CMemory::Free((const void*)*(int*)(arg0 + 0x4));
  }
}

struct __mwdec_vt_0_fn_16_BDD0 {
  virtual void _0(int);
};
extern "C" void fn_16_BDD0(int arg0) {
  int temp_r3;
  if (--*(int*)(*(int*)(arg0 + 0x4)) <= 0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_16_BDD0*)temp_r3)->_0(1);
    }
    CMemory::Free((const void*)*(int*)(arg0 + 0x4));
  }
}

extern "C" int fn_16_BF14(int arg0, int arg1) {
  int var_r31;
  int temp_r3;
  if (arg0) {
    var_r31 = *(int*)(arg0 + 0x4);
    while ((unsigned int)var_r31 != (*(int*)(arg0 + 0x8))) {
      temp_r3 = var_r31;
      var_r31 = *(int*)(var_r31 + 0x4);
      CMemory::Free((const void*)temp_r3);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_6D10(int arg0, int arg1, float arg2) {
  int var_r31;
  if (arg0) {
    ((CElementGen*)arg0)->SetExternalParam(arg1, arg2);
    for (var_r31 = 0; var_r31 < ((CElementGen*)arg0)->GetNumSpawnedParticleSystems();
         var_r31 = var_r31 + 1) {
      fn_16_6D10((int)((CElementGen*)arg0)->SpawnedParticleSystem(var_r31), arg1, arg2);
    }
  }
}

extern "C" void fn_16_1F34(int arg0, int arg1, int arg2) {
  int temp_r3 = fn_16_3AA8(*(int*)arg0, arg1, *(int*)(arg0 + 0x8), 4);
  if ((unsigned int)temp_r3 != 0) {
    if ((unsigned char)arg2) {
      ((CActor*)temp_r3)
          ->AddMaterial((EMaterialTypes)40, (EMaterialTypes)41, (EMaterialTypes)63,
                        (EMaterialTypes)57, *(CStateManager*)arg1);
    } else {
      ((CActor*)temp_r3)
          ->RemoveMaterial((EMaterialTypes)40, (EMaterialTypes)41, (EMaterialTypes)63,
                           (EMaterialTypes)57, *(CStateManager*)arg1);
    }
  }
}

extern "C" int fn_16_B86C(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 1052))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 704))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 356))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 8))->~SLdrDamageVulnerability();
    ((SLdrHealthInfo*)arg0)->~SLdrHealthInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_16_data_19C[96];
extern unsigned char lbl_16_data_220[48];
extern unsigned char lbl_16_data_94[192];
struct __mwdec_vt_0_fn_16_8AF4 {
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
extern "C" void fn_16_8AF4(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_16_8AF4*)temp_r31)->_4(lbl_16_data_19C, 6);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_16_data_94, 12);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_16_data_220, 3);
}

extern "C" int fn_16_AEDC(int arg0, int arg1) {
  void fn_16_AA70(int, int);
  void fn_16_B130(int, int);
  void fn_16_B378(int, int);
  void fn_16_B52C(int, int);
  void fn_16_B668(int, int);
  void fn_16_B86C(int, int);
  if (arg0) {
    ((SLdrAudioPlaybackParms*)(arg0 + 1660))->~SLdrAudioPlaybackParms();
    fn_16_AA70(arg0 + 1556, -1);
    fn_16_B130(arg0 + 1496, -1);
    fn_16_B378(arg0 + 1448, -1);
    fn_16_B52C(arg0 + 1424, -1);
    fn_16_B668(arg0 + 1408, -1);
    fn_16_B86C(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_16_B5C8(int arg0, int arg1) {
  int temp_r3;
  int temp_r4;
  int temp_r4_2;
  unsigned short temp_r5_2;
  int var_r29 = 0;
  int temp_r5 = *(int*)(arg1 + 0x8);
  *(int*)(arg1 + 0x8) = temp_r5 + 2;
  unsigned short temp_r30 = *(unsigned short*)temp_r5;
  while (var_r29 < temp_r30) {
    temp_r4 = *(int*)(arg1 + 0x8);
    *(int*)(arg1 + 0x8) = temp_r4 + 4;
    temp_r3 = *(int*)(arg1 + 0x8);
    temp_r4_2 = *(int*)temp_r4;
    *(int*)(arg1 + 0x8) = temp_r3 + 2;
    temp_r5_2 = *(unsigned short*)temp_r3;
    switch (temp_r4_2) {
    case 863999268:
      LoadTypedefDamageInfo(*(SLdrDamageInfo*)arg0, *(CInputStream*)arg1);
      break;
    default:
      ((CInputStream*)arg1)->ReadBytes(nullptr, temp_r5_2);
      break;
    }
    var_r29 = var_r29 + 1;
  }
}

extern "C" void fn_16_353C(int, int, int);
extern "C" void fn_16_7B40(int, int);
extern "C" void fn_16_830(int, int, int);
extern "C" void fn_16_7C28(int arg0, int arg1, int arg2) {
  float temp_f1;
  float temp_f2;
  float temp_f3;
  unsigned char stack_8[28];
  switch (arg2) {
  case 0:
    fn_16_830(arg0, arg1, 1);
    fn_16_353C(arg0, arg1, 1);
    break;
  case 1:
    temp_f1 = CVector3f::sZeroVector.GetX();
    temp_f2 = CVector3f::sZeroVector.GetY();
    temp_f3 = CVector3f::sZeroVector.GetZ();
    *(float*)stack_8 = temp_f1;
    *(float*)(stack_8 + 0x4) = temp_f2;
    *(float*)(stack_8 + 0x8) = temp_f3;
    *(float*)(stack_8 + 0xc) = temp_f1;
    *(float*)(stack_8 + 0x10) = temp_f2;
    *(float*)(stack_8 + 0x14) = temp_f3;
    *(float*)(stack_8 + 0x18) = 1.0f;
    ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->DeliverCmd(*(const CBCLocomotionCmd*)stack_8);
    fn_16_7B40(arg0, arg1);
    break;
  case 2:
    break;
  }
}

struct __mwdec_vt_0_fn_16_1E6C {
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
  virtual int _12();
};
extern "C" void fn_16_1E6C(int arg0, int arg1) {
  void fn_16_1F34(int, int, int);
  int temp_r31;
  int temp_r3_2;
  int temp_r3 = fn_16_3AA8(*(int*)arg0, arg1, *(int*)(arg0 + 0x8), 4);
  if ((unsigned int)temp_r3 != 0) {
    temp_r31 = *(int*)arg0;
    temp_r3_2 = ((__mwdec_vt_0_fn_16_1E6C*)temp_r3)->_12();
    *(float*)temp_r3_2 = *(float*)(temp_r31 + 0xaac);
    *(float*)(temp_r3_2 + 0x4) = *(float*)(temp_r31 + 0xab0);
    *(float*)(temp_r3_2 + 0x8) = *(float*)(temp_r31 + 0xab4);
    *(int*)(temp_r3_2 + 0xc) = *(int*)(temp_r31 + 0xab8);
    *(unsigned short*)(temp_r3_2 + 0x10) = *(unsigned short*)(temp_r31 + 0xabc);
    *(unsigned short*)(temp_r3_2 + 0x12) = *(unsigned short*)(temp_r31 + 0xabe);
    *(int*)(temp_r3_2 + 0x14) = *(int*)(temp_r31 + 0xac0);
    *(unsigned short*)(temp_r3_2 + 0x18) = *(unsigned short*)(temp_r31 + 0xac4);
    *(unsigned short*)(temp_r3_2 + 0x1a) = *(unsigned short*)(temp_r31 + 0xac6);
    *(unsigned char*)(temp_r3_2 + 0x1c) = *(unsigned char*)(temp_r31 + 0xac8);
  }
  fn_16_1F34(arg0, arg1, 1);
}

extern "C" void fn_16_2EE0(int obj, int val) {
  int val3 = fn_16_2F70();
  int val2 = obj + 2008;
  for (int i = 0; i < (*(int*)((char*)obj + 0x7d4)); i++) {
    if ((unsigned char)val && i < val3) {
      *(unsigned char*)((char*)val2 + 0x24) =
          __rlwimi(*(unsigned char*)((char*)val2 + 0x24), 1, 7, 24, 24);
    } else {
      *(unsigned char*)((char*)val2 + 0x24) =
          __rlwimi(*(unsigned char*)((char*)val2 + 0x24), 0, 7, 24, 24);
    }
    val2 += 72;
  }
}

extern "C" void fn_16_A980(int obj, int obj2) {
  int val;
  int val2;
  int val4;
  unsigned short val5;
  int val6 = *(int*)((char*)obj2 + 0x8);
  int i = 0;
  *(int*)((char*)obj2 + 0x8) = val6 + 2;
  unsigned short val7 = *(unsigned short*)val6;
  while (i < val7) {
    int val3 = *(int*)((char*)obj2 + 0x8);
    *(int*)((char*)obj2 + 0x8) = val3 + 4;
    val = *(int*)((char*)obj2 + 0x8);
    val4 = *(int*)val3;
    *(int*)((char*)obj2 + 0x8) = val + 2;
    val5 = *(unsigned short*)val;
    switch (val4) {
    case 362283306:
      LoadTypedefPlasmaBeamInfo(*(SLdrPlasmaBeamInfo*)obj, *(CInputStream*)obj2);
      break;
    case 863999268:
      LoadTypedefDamageInfo(*(SLdrDamageInfo*)(obj + 72), *(CInputStream*)obj2);
      break;
    case 1601975598:
      val2 = *(int*)((char*)obj2 + 0x8);
      *(int*)((char*)obj2 + 0x8) = val2 + 4;
      *(int*)((char*)obj + 0x58) = *(int*)val2;
      break;
    default:
      ((CInputStream*)obj2)->ReadBytes(nullptr, val5);
      break;
    }
    i++;
  }
}
