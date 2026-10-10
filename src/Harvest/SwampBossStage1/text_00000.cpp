// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
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
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
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
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
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
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" bool fn_78_0() { return true; }

extern "C" int fn_78_44(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_78_4C() { return false; }

extern "C" bool fn_78_54() { return false; }

extern "C" bool fn_78_5C() { return false; }

extern "C" int fn_78_5CD4(int arg0) { return *(unsigned char*)(arg0 + 0xbd8); }

extern "C" int fn_78_65E8(int arg0) { return *(unsigned char*)(arg0 + 0xd14); }

extern "C" int fn_78_8004(int arg0) { return arg0 + 3452; }

extern "C" int fn_78_8C(int arg0) { return arg0 + 1876; }

extern "C" bool fn_78_94() { return true; }

extern "C" bool fn_78_9C() { return false; }

extern "C" bool fn_78_653C(int arg0) { return *(unsigned char*)(arg0 + 0xc04) >> 7 & 1; }

extern "C" bool fn_78_74(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" float fn_78_80() { return kDefaultGravityAccel; }

extern "C" void fn_78_64(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" void fn_78_25F4(int arg0) {
  *(int*)(arg0 + 0xbfc) = *(int*)(arg0 + 0xbfc) + 1;
  *(int*)(arg0 + 0xc34) = 3;
}

extern "C" int fn_78_66D4(int arg0) {
  if ((*(int*)(arg0 + 0xc34)) == 2) {
    return 1;
  }
  return 0;
}

extern float lbl_78_rodata_1B0;
extern "C" bool fn_78_44B0(int arg0) { return *(float*)(arg0 + 0xc38) > lbl_78_rodata_1B0; }

extern "C" void fn_78_A4(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_78_130();
  fn_78_130();
}

extern "C" void fn_78_4B4() {
  void fn_78_4D4();
  fn_78_4D4();
}

extern "C" bool fn_78_4D1C(int arg0, int arg1, int arg2) {
  return *(float*)(arg0 + 0xb18) > *(float*)(arg0 + 0xd78) + *(float*)arg2;
}

extern "C" void fn_78_82E4() {
  void fn_78_8304();
  fn_78_8304();
}

extern "C" void fn_78_84A0();
extern "C" void fn_78_8480() { fn_78_84A0(); }

extern "C" void fn_78_8E18() {
  void fn_78_8E38();
  fn_78_8E38();
}

extern "C" void fn_78_69D8(int arg0) { ((CPatterned*)arg0)->CPatterned::GetDamageVulnerability(); }

extern "C" void fn_78_3D34(int, int, int);
extern "C" void fn_78_3E90(int arg0, int arg1) { fn_78_3D34(arg0, arg1, 0x44454352); }

extern "C" void fn_78_7F38(int, int);
extern "C" void fn_78_7F14(int arg0, int arg1) { fn_78_7F38(arg0, arg1 + 2960); }

extern "C" void RELExit() { SetSSwampBossStage1_FuncPtrs(nullptr); }

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
extern "C" void fn_78_C0(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" int fn_78_9A1C(int arg0, int arg1, int arg2) {
  if (arg1 == 3 || arg2 == 3) {
    return 1;
  } else {
    if ((arg1 == arg2)) {
      return 1;
    } else {
      return 0;
    }
  }
}

extern "C" void fn_78_785C(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0xb40);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0xb44);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0xb48);
  *(float*)(arg0 + 0xc) = *(float*)(arg1 + 0xb4c);
  *(float*)(arg0 + 0x10) = *(float*)(arg1 + 0xb50);
  *(float*)(arg0 + 0x14) = *(float*)(arg1 + 0xb54);
}

extern void* lbl_78_bss_20;
extern "C" void fn_78_160();
extern "C" void fn_78_130() {
  lbl_78_bss_20 = &fn_78_160;
  SetSSwampBossStage1_FuncPtrs((SSwampBossStage1_FuncPtrs*)&(*(int*)&lbl_78_bss_20));
}

extern "C" int fn_78_1B54(int arg0) {
  void fn_78_4B4();
  *(unsigned char*)(arg0 + 0x4c) = 1;
  fn_78_4B4();
  return arg0;
}

extern "C" int fn_78_8574(int arg0) {
  void fn_78_82E4();
  *(unsigned char*)(arg0 + 0x14) = 1;
  fn_78_82E4();
  return arg0;
}

extern "C" int fn_78_1074(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_78_2C3C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_78_2DFC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_78_4870(int arg0, int arg1) {
  int temp_r3 = *(int*)(arg1 + 0x14fc);
  if ((*(int*)(temp_r3 + 0x3a4)) == 4) {
    ((CPlayer*)temp_r3)->SetOrbitState((CPlayer::EPlayerOrbitState)0, *(const CStateManager*)arg1);
  }
}

extern "C" int fn_78_7788(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_78_7AA8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_78_83AC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_78_83E8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_78_3088(int arg0, int arg1, int arg2) {
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

extern unsigned char lbl_78_data_718[12];
extern "C" int fn_78_3444(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_78_data_718;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_5518(int arg0) {
  switch (*(int*)(arg0 + 0xb84)) {
  case 0:
    return arg0 + 2704;
  case 1:
    return arg0 + 2740;
  case 2:
    return arg0 + 2776;
  default:
    return arg0 + 2776;
  }
}

extern "C" bool fn_78_65F0(int arg0) {
  float temp_f1 = *(float*)(arg0 + 0xbf8);
  if (temp_f1 > 8.0f) {
    return true;
  } else if ((*(int*)(arg0 + 0xbfc)) < 3) {
    return false;
  } else {
    return temp_f1 > 0.5f;
  }
}

extern "C" void fn_78_81F0(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_78_81A8(int arg0) {
  void fn_78_8424(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_78_81F0(arg0, stack_24, stack_14, stack_8);
  fn_78_8424(stack_24, -1);
}

extern "C" void fn_78_3E4C(int arg0, int arg1) {
  fn_78_3D34(arg0, arg1, 0x494e4352);
  ((CPlayer*)*(int*)(arg1 + 0x14fc))
      ->SetMoveState((NPlayer::EPlayerMovementState)2, *(CStateManager*)arg1);
}

extern "C" bool fn_78_A528(int arg0) {
  switch (*(int*)(arg0 + 0xb14)) {
  case 0:
  case 3:
  case 4:
  case 5:
  case 6:
  case 9:
  case 10:
    return true;
  default:
    return false;
  }
}

extern "C" void fn_78_55AC(int, int);
extern "C" void fn_78_5560(int arg0, int arg1, int arg2) {
  if (arg2 == 2) {
    fn_78_55AC(arg0 + 2812, *(int*)(arg0 + 0xb14));
    *(int*)(arg0 + 0xb14) = -1;
  } else {
    *(int*)(arg0 + 0xb14) = arg1;
  }
}

struct CScannableObjectInfo;
struct CScannableObjectInfo {};
extern "C" int fn_78_A570(int arg0) {
  if ((*(int*)((*(int*)(arg0 + 0x48c)) + 0x580)) == 7 ||
      !((*(unsigned char*)(arg0 + 0xb58)) >> 6 & 1)) {
    return 0;
  }
  return (int)((CPatterned*)arg0)->CPatterned::GetScannableObjectInfo();
}

extern "C" int fn_78_7EC0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_AEB8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_ADAC(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 12))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_78_data_70C[12];
extern "C" int fn_78_33E8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_78_data_70C;
    if (arg0) {
      *(int*)arg0 = (int)lbl_78_data_718;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_78_data_700[12];
extern "C" int fn_78_3580(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_78_data_700;
    if (arg0) {
      *(int*)arg0 = (int)lbl_78_data_718;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_78_data_6F4[12];
extern "C" int fn_78_3848(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_78_data_6F4;
    if (arg0) {
      *(int*)arg0 = (int)lbl_78_data_718;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_B098(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_75C(int arg0, int arg1) {
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

extern "C" int fn_78_B970(int arg0, int arg1) {
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x8))) {
      ((CToken*)arg0)->~CToken();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_78_252C(int arg0, int arg1) {
  if ((*(int*)(arg0 + 0xb5c)) != 1) {
    ((CEntity*)arg0)
        ->SendScriptMsgs((EScriptObjectState)0x41495332, *(CStateManager*)arg1, kInvalidUniqueId,
                         (EScriptObjectMessage)-1);
    *(int*)(arg0 + 0xb5c) = 1;
  }
}

extern "C" int fn_78_2840(int arg0, int arg1) {
  switch (arg1) {
  case 0:
    return 0x49533035;
  case 1:
    return 0x49533034;
  case 2:
    return 0x49533036;
  case 3:
    return 0x49533037;
  default:
    return 0x49533030;
  }
}

extern "C" int fn_78_28A4(int arg0, int arg1) {
  switch (arg1) {
  case 0:
    return 0x49533031;
  case 1:
    return 0x49533030;
  case 2:
    return 0x49533032;
  case 3:
    return 0x49533033;
  default:
    return 0x49533030;
  }
}

extern "C" void fn_78_2590(int arg0, int arg1) {
  if ((*(int*)(arg0 + 0xb5c))) {
    ((CEntity*)arg0)
        ->SendScriptMsgs((EScriptObjectState)0x41495331, *(CStateManager*)arg1, kInvalidUniqueId,
                         (EScriptObjectMessage)-1);
    *(int*)(arg0 + 0xb5c) = 0;
  }
}

extern "C" int fn_78_B0F0(int arg0, int arg1) {
  if (arg0) {
    if (arg0 && (*(unsigned char*)(arg0 + 0x8))) {
      ((CToken*)arg0)->~CToken();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_B03C(int arg0, int arg1) {
  if (arg0) {
    if (arg0) {
      delete (CCollisionActorManager*)*(int*)arg0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_78_260C();
extern "C" void fn_78_9744(int, int, int, int);
extern "C" void fn_78_38A4(int arg0, int arg1, int arg2) {
  if (!arg2) {
    fn_78_260C();
    *(unsigned char*)(arg0 + 0xd14) = 0;
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)7);
    fn_78_9744(arg0, arg1, 3, 42);
  }
}

extern "C" int fn_78_6F8(int arg0, int arg1) {
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

extern "C" void fn_78_5304(int arg0, int arg1, int arg2) {
  ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)7);
  if (!arg2) {
    *(float*)(arg0 + 0xd78) = *(float*)(arg0 + 0xb18);
    fn_78_9744(arg0, arg1, 2, 32);
  }
}

extern "C" int fn_78_7BC(int arg0, int arg1) {
  void fn_78_CA4(int, int);
  if (arg0) {
    fn_78_CA4(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_78_4FC();
extern "C" void fn_78_4D4(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_78_4FC();
  }
}

extern "C" void fn_78_832C();
extern "C" void fn_78_8304(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_78_832C();
  }
}

extern "C" void fn_78_8E60();
extern "C" void fn_78_8E38(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_78_8E60();
  }
}

struct __mwdec_vt_0_fn_78_6548 {
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
extern "C" bool fn_78_6548(int arg0) {
  return *(float*)(((__mwdec_vt_0_fn_78_6548*)arg0)->_13() + 0x4) <= 1.01f;
}

extern "C" int fn_78_8694(int arg0, int arg1) {
  void fn_78_8424(int, int);
  if (arg0) {
    fn_78_8424(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_78_19A4(int arg0) {
  *(float*)(arg0 + 0xdc) = 0.0f;
  *(float*)(arg0 + 0x30) = 0.0f;
  *(float*)(arg0 + 0x2c) = 0.0f;
  *(float*)(arg0 + 0x24) = 0.0f;
  *(int*)(arg0 + 0x28) = 0;
  *(float*)(arg0 + 0x18) = CVector3f::sZeroVector.GetX();
  *(float*)(arg0 + 0x1c) = CVector3f::sZeroVector.GetY();
  *(float*)(arg0 + 0x20) = CVector3f::sZeroVector.GetZ();
  *(float*)(arg0 + 0xc) = *(float*)(arg0 + 0x18);
  *(float*)(arg0 + 0x10) = *(float*)(arg0 + 0x1c);
  *(float*)(arg0 + 0x14) = *(float*)(arg0 + 0x20);
}

struct __mwdec_vt_0_fn_78_6590 {
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
extern "C" bool fn_78_6590(int arg0) {
  return *(float*)(((__mwdec_vt_0_fn_78_6590*)arg0)->_13() + 0x4) <=
         *(float*)(arg0 + 0xd0c) - *(float*)(arg0 + 0x828);
}

extern "C" int fn_78_7E68(int arg0, int arg1) {
  void fn_78_7EC0(int, int);
  if (arg0) {
    fn_78_7EC0(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_8424(int arg0, int arg1) {
  void fn_78_8480();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_78_8480();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_78_64D0 {
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
extern "C" bool fn_78_64D0(int arg0) {
  if (*(float*)(((__mwdec_vt_0_fn_78_64D0*)arg0)->_13() + 0x4) <= 1.0f) {
    return false;
  }
  return *(float*)(arg0 + 0xb7c) < *(float*)(arg0 + 0xb1c);
}

struct __mwdec_vt_0_fn_78_1CB4 {
  virtual void _0(int);
};
extern "C" int fn_78_1CB4(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (*(unsigned char*)arg0) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_78_1CB4*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_78_1D90 {
  virtual void _0(int);
};
extern "C" int fn_78_1D90(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (arg0 && (*(unsigned char*)arg0)) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_78_1D90*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_78_549C(int arg0) {
  int fn_78_5518();
  int temp_r3 = fn_78_5518();
  switch (*(int*)(arg0 + 0xb88)) {
  case 0:
    return *(int*)(temp_r3 + 0x14);
  case 1:
    return *(int*)(temp_r3 + 0x18);
  case 2:
    return *(int*)(temp_r3 + 0x1c);
  case 3:
    return *(int*)(temp_r3 + 0x20);
  }
  return *(int*)(temp_r3 + 0x14);
}

extern "C" int fn_78_1918(int obj, int val) {
  if (obj) {
    if (obj + 136 && (*(unsigned char*)((char*)obj + 0xd4))) {
      ((CModelData*)(obj + 136))->~CModelData();
    }
    if (obj + 56 && (*(unsigned char*)((char*)obj + 0x84))) {
      ((CModelData*)(obj + 56))->~CModelData();
    }
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

extern unsigned char lbl_78_data_30C[400];
extern unsigned char lbl_78_data_4CC[64];
extern unsigned char lbl_78_data_D0[272];
struct __mwdec_vt_0_fn_78_6864 {
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
extern "C" void fn_78_6864(int obj, int obj2) {
  ((CPatterned*)obj)->InitializeStateMachine(*(CStateManager*)obj2);
  ((__mwdec_vt_0_fn_78_6864*)*(int*)((char*)obj + 0x350))->_4(lbl_78_data_D0, 17);
  ((__mwdec_vt_1*)*(int*)((char*)obj + 0x350))->_3(lbl_78_data_30C, 25);
  ((__mwdec_vt_2*)*(int*)((char*)obj + 0x350))->_5(lbl_78_data_4CC, 4);
}

extern "C" int fn_78_A5B4(int);
extern "C" void fn_78_5CDC(int obj, int obj2) {
  if (((CRandom16*)(obj2 + 5860))->Range(0.0f, 1.0f) < 0.7f) {
    if (!(*(int*)((char*)obj + 0xb6c))) {
      *(int*)((char*)obj + 0xb6c) = 1;
    } else {
      *(int*)((char*)obj + 0xb6c) = 0;
    }
  }
  int val = *(int*)((char*)obj + 0x48c);
  ((CBodyController*)val)->SetLocomotionType((pas::ELocomotionType)fn_78_A5B4(obj));
}

extern "C" void fn_78_4D3C();
extern "C" void fn_78_568C(int, int);
extern "C" void fn_78_5D6C(int obj, int val, int val2) {
  int val3;
  fn_78_4D3C();
  *(unsigned char*)((char*)obj + 0xbd8) = 1;
  if (!val2) {
    val3 = *(int*)((char*)obj + 0x48c);
    ((CBodyController*)val3)->SetLocomotionType((pas::ELocomotionType)fn_78_A5B4(obj));
    if ((*(int*)((char*)obj + 0xb84)) == (*(int*)((char*)obj + 0xbec))) {
      *(int*)((char*)obj + 0xb88) = *(int*)((char*)obj + 0xb88) + 1;
      fn_78_568C(obj, val);
    }
  }
}

struct __mwdec_vt_0_fn_78_1B8C {
  virtual void _0(int);
};
extern "C" int fn_78_1B8C(int obj, int val) {
  if (obj) {
    if (obj + 48 && (*(unsigned char*)((char*)obj + 0x30))) {
      unsigned int val2 = *(int*)((char*)obj + 0x34);
      if (val2 != 0) {
        ((__mwdec_vt_0_fn_78_1B8C*)val2)->_0(1);
      }
    }
    ((SLdrShockWaveInfo*)obj)->~SLdrShockWaveInfo();
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

extern float lbl_78_rodata_1CC;
extern "C" void fn_78_5C30(int obj, int obj2) {
  void fn_78_5CDC(int, int);
  int val;
  *(float*)&val = 0.017453292f * *(float*)((char*)obj + 0xbd0);
  ((CActor*)obj)
      ->SetTransform(CQuaternion::ZRotation(*(const CRelAngle*)(&val))
                         .BuildTransform4f(*(const CVector3f*)(obj + 84)));
  *(float*)((char*)obj + 0xbd0) =
      *(float*)((char*)obj + 0xbd0) + ((CRandom16*)(obj2 + 5860))->Range(lbl_78_rodata_1CC, 220.0f);
  fn_78_5CDC(obj, obj2);
}

extern "C" void fn_78_93D4();
extern "C" int fn_78_A5CC();
extern "C" void fn_78_6198(int obj, int val, int val2, float f) {
  void fn_78_8868(int, int, int);
  int val3;
  switch (val2) {
  case 0:
    val3 = *(int*)((char*)obj + 0x48c);
    ((CBodyController*)val3)->SetLocomotionType((pas::ELocomotionType)fn_78_A5CC());
    fn_78_8868(obj, val, 0);
    fn_78_9744(obj, val, 3, 10);
    fn_78_9744(obj, val, 2, 16);
    break;
  case 1:
    *(float*)((char*)obj + 0xb1c) = *(float*)((char*)obj + 0xb1c) + f;
    fn_78_93D4();
    break;
  }
}

extern "C" int fn_78_CA4(int obj, int val) {
  void fn_78_1074(int, int);
  void fn_78_12E8(int, int);
  if (obj) {
    fn_78_1074(obj + 792, -1);
    fn_78_1074(obj + 756, -1);
    fn_78_1074(obj + 720, -1);
    fn_78_12E8(obj + 500, -1);
    ((SLdrDamageInfo*)(obj + 472))->~SLdrDamageInfo();
    ((SLdrDamageVulnerability*)(obj + 116))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)(obj + 76))->~SLdrDamageInfo();
    ((SLdrShockWaveInfo*)(obj + 16))->~SLdrShockWaveInfo();
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

extern float lbl_78_rodata_15C;
extern "C" void fn_78_2E38(int, int, float);
extern "C" void fn_78_716C(int, int, int, float);
extern "C" void fn_78_7224(int, float);
extern "C" void fn_78_729C(int, int, float);
extern "C" void fn_78_35DC(int obj, int val, int val2, float f) {
  void fn_78_5560(int, int);
  fn_78_5560(obj, 3);
  ((CBodyController*)*(int*)((char*)obj + 0x48c))->SetLocomotionType(pas::kLT_Internal5);
  if (val2 == 1) {
    fn_78_729C(obj, val, f);
    fn_78_716C(obj, val, 0, lbl_78_rodata_15C);
    fn_78_7224(obj, f);
    fn_78_2E38(obj, val, f);
  }
}

extern "C" int fn_78_12E8(int obj, int val) {
  if (obj) {
    ((SLdrAudioPlaybackParms*)(obj + 196))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 172))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 144))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 120))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 96))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 72))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 48))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 24))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)obj)->~SLdrAudioPlaybackParms();
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

extern unsigned char lbl_78_rodata_8[320];
template < class T0 >
int TCastToPtr(CEntity*);
