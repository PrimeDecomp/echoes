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
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
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
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
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
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CBasicSwarmData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerKnockBackMgr.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrBasicSwarmProperties.hpp"
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
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_18_B614() {}

extern "C" void fn_18_9044() {}

extern "C" void fn_18_E090() {}

extern "C" void fn_18_B618() {}

extern "C" bool fn_18_0() { return true; }

extern "C" bool fn_18_4314() { return true; }

extern "C" bool fn_18_4C() { return false; }

extern "C" int fn_18_44(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_18_54() { return false; }

extern "C" bool fn_18_5C() { return false; }

extern "C" bool fn_18_64() { return false; }

extern "C" int fn_18_88(int arg0) { return arg0 + 1876; }

extern "C" bool fn_18_90() { return true; }

extern "C" bool fn_18_AF94() { return false; }

extern "C" int fn_18_AEAC(int arg0) { return arg0 + 1468; }

extern "C" int fn_18_DAE4(int arg0) { return arg0 + 1468; }

extern "C" void fn_18_E094(int arg0, int arg1) { *(int*)(arg0 + 0x1c) = arg1; }

extern "C" int fn_18_E0A8(int arg0) { return *(int*)(arg0 + 0x1c); }

extern "C" void fn_18_1DE8(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" bool fn_18_7C(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern float lbl_18_rodata_2A4;
extern "C" float fn_18_8E78() { return lbl_18_rodata_2A4; }

extern float lbl_18_rodata_264;
extern "C" float fn_18_E09C() { return lbl_18_rodata_264; }

extern "C" void fn_18_6C(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" int fn_18_8664(int arg0) {
  if (*(int*)(arg0 + 0x7e8) == 1) {
    return 1;
  }
  return 0;
}

extern "C" int fn_18_897C(int arg0) {
  if (*((int*)(arg0 + 0x7e8)) == 0) {
    return 1;
  }
  return 0;
}

extern "C" int fn_18_8678(int arg0) { return (*(int*)(arg0 + 0x7e8) == 5) ? 1 : 0; }

extern "C" int fn_18_8954(int arg0) { return (*(int*)(arg0 + 0x7e8) == 4) ? 1 : 0; }

extern "C" int fn_18_8968(int arg0) {
  if (*(int*)(arg0 + 0x7e8) == 3) {
    return 1;
  }
  return 0;
}

extern "C" int fn_18_8A20(int arg0) { return (*(int*)(arg0 + 0x7c4) == 1) ? 1 : 0; }

extern "C" int fn_18_E0(int arg0) {
  return (*(int*)((*(int*)(arg0 + 0x48c)) + 0x37c) == 6) ? 1 : 0;
}

extern float lbl_18_rodata_310;
extern "C" bool fn_18_8648(int arg0) { return *(float*)(arg0 + 0x814) < lbl_18_rodata_310; }

extern float lbl_18_rodata_258;
extern "C" bool fn_18_8D88(int arg0) { return *(float*)(arg0 + 0x7e4) < lbl_18_rodata_258; }

extern "C" void fn_18_98(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_18_C350();
  fn_18_C350();
}

extern "C" void fn_18_4038() {
  void fn_18_4058();
  fn_18_4058();
}

extern "C" void fn_18_47B4(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::Render(*(const CStateManager*)arg1);
}

extern "C" void fn_18_C6DC() {
  void fn_18_C6FC();
  fn_18_C6FC();
}

extern "C" void RELExit() { SetSEmperorIngStage3_FuncPtrs(nullptr); }

extern "C" void fn_18_AB9C(int);
extern "C" void fn_18_2298(int arg0) { fn_18_AB9C(arg0 + 2112); }

extern "C" void fn_18_22BC(int arg0) { fn_18_AB9C(arg0 + 2096); }

extern "C" void fn_18_3648(int arg0, int arg1) {
  *(int*)arg0 = *(int*)(arg1 + 0x78);
  *(int*)(arg0 + 0x4) = *(int*)(arg1 + 0x7c);
  int temp_r4 = *(int*)(arg0 + 0x4);
  *(int*)temp_r4 = *(int*)temp_r4 + 1;
}

extern "C" void fn_18_3B8C(int arg0, int arg1) {
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7c0))->Destroy(*(CStateManager*)arg1);
}

extern "C" void fn_18_1DF4(int, int, int, int);
extern "C" void fn_18_1B88(int arg0, int arg1) { fn_18_1DF4(arg0, arg1, 3, 0); }

extern "C" void fn_18_1BB0(int arg0, int arg1) { fn_18_1DF4(arg0, arg1, *(int*)(arg0 + 0x868), 0); }

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
extern "C" void fn_18_B4(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern void* lbl_18_bss_30;
extern "C" void fn_18_C380();
extern "C" void fn_18_C350() {
  lbl_18_bss_30 = &fn_18_C380;
  SetSEmperorIngStage3_FuncPtrs((SEmperorIngStage3_FuncPtrs*)&(*(int*)&lbl_18_bss_30));
}

extern "C" void fn_18_7ADC(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 2:
    fn_18_1DF4(arg0, arg1, 0, 1);
    break;
  case 1:
    break;
  }
}

extern "C" int fn_18_27A0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_18_5E74(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_18_6034(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_18_6070(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_18_846C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_18_D9BC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_18_DE2C();
extern "C" int fn_18_ADF8(int arg0) {
  *(int*)(arg0 + 0x4) = 0;
  *(int*)(arg0 + 0x8) = 0;
  *(int*)(arg0 + 0xc) = 0;
  fn_18_DE2C();
  return arg0;
}

extern "C" void fn_18_39A0(int arg0, int arg1, int arg2) {
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

extern "C" int fn_18_DEE4(int arg0, int arg1, int arg2) {
  int var_r3 = *(int*)arg0;
  int temp_r0 = *(int*)arg1;
  while ((unsigned int)var_r3 != temp_r0) {
    if ((unsigned int)arg2 != 0) {
      *(float*)arg2 = *(float*)var_r3;
      *(float*)(arg2 + 0x4) = *(float*)(var_r3 + 0x4);
      *(float*)(arg2 + 0x8) = *(float*)(var_r3 + 0x8);
    }
    var_r3 = var_r3 + 12;
    arg2 = arg2 + 12;
  }
  return arg2;
}

extern "C" void fn_18_3A84(int arg0, int arg1, int arg2) {
  int fn_18_3B14(int, int);
  ((CStateManager*)arg1)
      ->GetObjectById(TUniqueId((ushort) * (unsigned short*)(fn_18_3B14(arg0, arg2) + 0x3c)));
}

extern int lbl_18_data_78C;
extern "C" int fn_18_311C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_78C;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_18_3ACC(int arg0, int arg1, int arg2) {
  int fn_18_3B14(int, int);
  ((CStateManager*)arg1)
      ->ObjectById(TUniqueId((ushort) * (unsigned short*)(fn_18_3B14(arg0, arg2) + 0x3c)));
}

extern "C" int fn_18_366C(int arg0, int arg1) {
  void fn_18_DFC8();
  if (arg0) {
    fn_18_DFC8();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_35F4(int arg0, int arg1) {
  void fn_18_DFC8();
  if (arg0) {
    if (arg0) {
      fn_18_DFC8();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_938(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_18_99B4(int, int);
extern "C" int fn_18_9960(int arg0, int arg1) {
  if (arg0) {
    fn_18_99B4(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_9A38(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_3B14(int arg0, int arg1) {
  if (!arg1) {
    return (int)&((CCollisionActorManager*)*(int*)(arg0 + 0x7c0))->GetCollisionDescFromIndex(10);
  }
  if (arg1 == 1) {
    return (int)&((CCollisionActorManager*)*(int*)(arg0 + 0x7c0))->GetCollisionDescFromIndex(11);
  }
  return 0;
}

extern "C" int fn_18_A934(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_B04();
extern "C" void fn_18_84A8(int arg0, int arg1) {
  int temp_r3 = fn_18_B04();
  *(float*)(arg0 + 0x7e4) =
      ((CRandom16*)(arg1 + 5860))->Range(*(float*)(temp_r3 + 0x4), *(float*)(temp_r3 + 0x8));
  *(int*)(arg0 + 0x7e8) = -1;
}

extern "C" int fn_18_A548(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 60))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_780;
extern "C" int fn_18_30C0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_780;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_774;
extern "C" int fn_18_33EC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_774;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_AB44(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_768;
extern "C" int fn_18_4E58(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_768;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_744;
extern "C" int fn_18_555C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_744;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_75C;
extern "C" int fn_18_518C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_75C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_750;
extern "C" int fn_18_55B8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_750;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_738;
extern "C" int fn_18_61FC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_738;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_720;
extern "C" int fn_18_717C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_720;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_72C;
extern "C" int fn_18_663C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_72C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_18_data_78C;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_C984(int arg0, int arg1) {
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

extern "C" int fn_18_D1B8(int arg0, int arg1) {
  if (arg0) {
    ((SLdrPlasmaBeamInfo*)(arg0 + 24))->~SLdrPlasmaBeamInfo();
    ((SLdrDamageInfo*)(arg0 + 8))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_D42C(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 72))->~SLdrDamageInfo();
    ((SLdrPlasmaBeamInfo*)arg0)->~SLdrPlasmaBeamInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_18_data_860;
extern "C" int fn_18_E02C(int arg0, int arg1) {
  void fn_18_E10C(int, int);
  if (arg0) {
    *(int*)arg0 = (int)&lbl_18_data_860;
    fn_18_E10C(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_C920(int arg0, int arg1) {
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

extern "C" int fn_18_D840(int arg0) {
  void fn_18_D9F8(int);
  fn_18_D9F8(arg0 + 12);
  fn_18_D9F8(arg0 + 24);
  fn_18_D9F8(arg0 + 36);
  fn_18_D9F8(arg0 + 48);
  fn_18_D9F8(arg0 + 60);
  fn_18_D9F8(arg0 + 72);
  fn_18_D9F8(arg0 + 84);
  fn_18_D9F8(arg0 + 96);
  float temp_f0 = lbl_18_rodata_258;
  *(float*)arg0 = temp_f0;
  *(float*)(arg0 + 0x4) = temp_f0;
  *(float*)(arg0 + 0x8) = temp_f0;
  return arg0;
}

extern "C" void fn_18_CE70(int, int);
extern "C" int fn_18_C9E4(int arg0, int arg1) {
  if (arg0) {
    fn_18_CE70(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_18_4080();
extern "C" void fn_18_4058(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_18_4080();
  }
}

extern "C" void fn_18_3B64(int arg0, int arg1, float arg2) {
  ((CCollisionActorManager*)*(int*)(arg0 + 0x7c0))
      ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
}

extern "C" void fn_18_C724();
extern "C" void fn_18_C6FC(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_18_C724();
  }
}

struct __mwdec_vt_0_fn_18_B588 {
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
extern "C" bool fn_18_B588(int arg0) {
  return (*(float*)(((__mwdec_vt_0_fn_18_B588*)arg0)->_13() + 0x4)) > lbl_18_rodata_258;
}

struct __mwdec_vt_0_fn_18_214 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5(unsigned char);
};
extern "C" void fn_18_214(int arg0, int arg1, int arg2, int arg3) {
  int fn_18_3ACC();
  int temp_r3 = fn_18_3ACC();
  if ((unsigned int)temp_r3 != 0) {
    ((__mwdec_vt_0_fn_18_214*)temp_r3)->_5(arg3);
  }
}

extern "C" int fn_18_8E0(int arg0, int arg1) {
  void fn_18_938(int, int);
  if (arg0) {
    fn_18_938(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_18_1BD8(int arg0, int arg1) {
  int fn_18_3ACC(int, int, int);
  ((CCollisionActor*)fn_18_3ACC(arg0, arg1, 0))
      ->SetDamageVulnerability(*(const CDamageVulnerability*)(arg0 + 2156));
  ((CCollisionActor*)fn_18_3ACC(arg0, arg1, 1))
      ->SetDamageVulnerability(*(const CDamageVulnerability*)(arg0 + 2156));
}

struct __mwdec_vt_0_fn_18_DFC8 {
  virtual void _0(int);
};
extern "C" void fn_18_DFC8(int arg0) {
  int temp_r3;
  if (--*(int*)(*(int*)(arg0 + 0x4)) <= 0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_18_DFC8*)temp_r3)->_0(1);
    }
    CMemory::Free((const void*)*(int*)(arg0 + 0x4));
  }
}

struct __mwdec_vt_0_fn_18_C190 {
  virtual void _0(int);
};
extern "C" int fn_18_C190(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_18_C190*)temp_r3)->_0(1);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_18_E10C(int arg0, int arg1) {
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

extern float lbl_18_rodata_270;
struct __mwdec_vt_0_fn_18_B61C {
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
extern "C" int fn_18_B61C(int arg0) {
  u32 var_r31 = true;
  float temp_f1 = *(float*)(0x4 + (*((__mwdec_vt_0_fn_18_B61C*)arg0))._13());
  if (!(temp_f1 <= lbl_18_rodata_258) && !(*(float*)(arg0 + 0x5e0) > lbl_18_rodata_270)) {
    var_r31 = false;
  }
  return var_r31;
}

extern "C" void fn_18_4BB8(int arg0, int arg1, float arg2) {
  int var_r31;
  if (arg0) {
    ((CElementGen*)arg0)->SetExternalParam(arg1, arg2);
    for (var_r31 = 0; var_r31 < ((CElementGen*)arg0)->GetNumSpawnedParticleSystems();
         var_r31 = var_r31 + 1) {
      fn_18_4BB8((int)((CElementGen*)arg0)->SpawnedParticleSystem(var_r31), arg1, arg2);
    }
  }
}

extern "C" void fn_18_AC00(int, float, float);
extern "C" void fn_18_AC44(int, float);
extern "C" void fn_18_2B68(int arg0, int arg1) {
  switch (arg1) {
  case 0:
    fn_18_AC00(arg0 + 2112, 200.0f, 8.0f);
    break;
  case 1:
    fn_18_AC00(arg0 + 2112, 100.0f, 4.0f);
    break;
  case 2:
    fn_18_AC44(arg0 + 2112, 10.0f);
    break;
  }
}

extern "C" void fn_18_4C3C(int arg0, int arg1, int arg2, float arg3) {
  if (!arg2) {
    ((CEntity*)arg0)
        ->SendScriptMsgs((EScriptObjectState)0x44454144, *(CStateManager*)arg1, kInvalidUniqueId,
                         (EScriptObjectMessage)-1);
  }
  ((CPatterned*)arg0)->Dead(*(CStateManager*)arg1, (EStateMsg)arg2, arg3);
}

extern unsigned char lbl_18_data_100[336];
extern unsigned char lbl_18_data_34C[336];
extern unsigned char lbl_18_data_4B4[32];
struct __mwdec_vt_0_fn_18_943C {
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
extern "C" void fn_18_943C(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_18_943C*)temp_r31)->_4(lbl_18_data_34C, 21);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_18_data_100, 21);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_18_data_4B4, 2);
}

struct __mwdec_vt_0_fn_18_1C34 {
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
  virtual int _14();
};
extern "C" void fn_18_1CCC(int, int);
extern "C" void fn_18_1C34(int arg0, int arg1) {
  int fn_18_3ACC(int, int, int);
  int temp_r31 = fn_18_3ACC(arg0, arg1, 0);
  fn_18_1CCC(arg0 + 2156, ((__mwdec_vt_0_fn_18_1C34*)temp_r31)->_14());
  ((CCollisionActor*)temp_r31)->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
  int temp_r31_2 = fn_18_3ACC(arg0, arg1, 1);
  ((CCollisionActor*)temp_r31_2)
      ->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
}

extern "C" CAABox fn_18_45B4(int arg0, int arg1, int arg2) {
  CVector3f stack_20 =
      ((CActor*)arg0)->CActor::GetSortingBounds(*(const CStateManager*)arg1).GetCenterPoint();
  return CAABox(stack_20 - CVector3f::sOneVector, stack_20 + CVector3f::sOneVector);
}

extern "C" void fn_18_39E0(int arg0, int arg1) {
  int fn_18_3A84(int, int, int);
  int temp_r31 = fn_18_3A84(arg0, arg1, 0);
  int temp_r3 = fn_18_3A84(arg0, arg1, 1);
  if (((*(unsigned char*)(temp_r31 + 0x20)) >> 7 & 1) ||
      ((*(unsigned char*)(temp_r3 + 0x20)) >> 7 & 1)) {
    ((CActor*)arg0)
        ->AddMaterial((EMaterialTypes)40, (EMaterialTypes)41, (EMaterialTypes)63,
                      *(CStateManager*)arg1);
  } else {
    ((CActor*)arg0)
        ->RemoveMaterial((EMaterialTypes)40, (EMaterialTypes)41, (EMaterialTypes)63,
                         *(CStateManager*)arg1);
  }
}

struct __mwdec_vt_0_fn_18_A00 {
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
struct __mwdec_vt_1_fn_18_A00 {
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
extern "C" float fn_18_A00(int arg0) {
  float var_f2 = rstl::min_val(1.0f, *(float*)(((__mwdec_vt_1_fn_18_A00*)arg0)->_13() + 0x4) /
                                         (*(float*)((__mwdec_vt_0_fn_18_A00*)arg0)->_13()));
  var_f2 = rstl::max_val(0.0f, var_f2);
  return 1.0f - var_f2;
}

extern "C" void fn_18_2BF0(int arg0, int arg1) {
  switch (arg1) {
  case 0:
    fn_18_AC00(arg0 + 2096, 75.0f, 15.0f);
    break;
  case 1:
    fn_18_AC00(arg0 + 2096, 200.0f, 10.0f);
    break;
  case 2:
    fn_18_AC00(arg0 + 2096, 25.0f, 5.0f);
    break;
  case 3:
    fn_18_AC44(arg0 + 2096, 5.0f);
    break;
  }
}

extern "C" int fn_18_D794(int arg0, int arg1) {
  void fn_18_D9BC(int, int);
  if (arg0) {
    fn_18_D9BC(arg0 + 96, -1);
    fn_18_D9BC(arg0 + 84, -1);
    fn_18_D9BC(arg0 + 72, -1);
    fn_18_D9BC(arg0 + 60, -1);
    fn_18_D9BC(arg0 + 48, -1);
    fn_18_D9BC(arg0 + 36, -1);
    fn_18_D9BC(arg0 + 24, -1);
    fn_18_D9BC(arg0 + 12, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_18_D9F8(int arg0) {
  *(unsigned char*)arg0 = 0;
  *(float*)(arg0 + 0x4) = 0.0f;
  *(float*)(arg0 + 0x8) = 0.0f;
}

extern "C" void fn_18_2C98(int arg0, int arg1) {
  float temp_f2 = *(float*)(arg1 + 0x38);
  *(float*)arg0 = *(float*)(arg1 + 0x28);
  *(float*)(arg0 + 0x4) = temp_f2;
  *(float*)(arg0 + 0x8) = 0.0f;
}

extern "C" void fn_18_D33C(int obj, const int obj2) {
  int val;
  int val2;
  int val3;
  int val4;
  unsigned short val5;
  int i = 0;
  int val6 = *(int*)((char*)obj2 + 0x8);
  *(int*)((char*)obj2 + 0x8) = val6 + 2;
  unsigned short val7 = *(unsigned short*)val6;
  while (i < val7) {
    val4 = *(int*)((char*)obj2 + 0x8);
    *(int*)((char*)obj2 + 0x8) = val4 + 4;
    val = *(int*)((char*)obj2 + 0x8);
    val2 = *(int*)val4;
    *(int*)((char*)obj2 + 0x8) = val + 2;
    val5 = *(unsigned short*)val;
    switch (val2) {
    case 362283306:
      LoadTypedefPlasmaBeamInfo(*(SLdrPlasmaBeamInfo*)obj, *(CInputStream*)obj2);
      break;
    case 863999268:
      LoadTypedefDamageInfo(*(SLdrDamageInfo*)(obj + 72), *(CInputStream*)obj2);
      break;
    case -151943722:
      val3 = *(int*)((char*)obj2 + 0x8);
      *(int*)((char*)obj2 + 0x8) = val3 + 4;
      *(int*)((char*)obj + 0x58) = *(int*)val3;
      break;
    default:
      ((CInputStream*)obj2)->ReadBytes(nullptr, val5);
      break;
    }
    i++;
  }
}
