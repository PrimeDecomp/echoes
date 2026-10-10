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

extern "C" void fn_29_A094() {}

extern "C" void fn_29_A090() {}

extern "C" int fn_29_0(int arg0) { return arg0 + 2524; }

extern "C" bool fn_29_10() { return true; }

extern "C" bool fn_29_2C() { return false; }

extern "C" int fn_29_24(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_29_34() { return false; }

extern "C" int fn_29_64(int arg0) { return arg0 + 1876; }

extern "C" bool fn_29_6C() { return true; }

extern "C" int fn_29_8(int arg0) { return arg0 + 2760; }

extern float lbl_29_rodata_64;
extern "C" float fn_29_18() { return lbl_29_rodata_64; }

extern "C" float fn_29_58() { return kDefaultGravityAccel; }

extern "C" bool fn_29_4C(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" bool fn_29_A4E0(int arg0) { return *(unsigned char*)(arg0 + 0xe5c) >> 3 & 1; }

extern "C" bool fn_29_A658(int arg0) { return *(unsigned char*)(arg0 + 0xe5e) >> 6 & 1; }

extern "C" bool fn_29_A8C0(int arg0) { return *(unsigned char*)(arg0 + 0xe5c) >> 6 & 1; }

extern "C" bool fn_29_ACE0(int arg0) { return *(unsigned char*)(arg0 + 0xe5e) >> 7 & 1; }

extern "C" bool fn_29_B1E8(int arg0) { return *(unsigned char*)(arg0 + 0xe5d) >> 2 & 1; }

extern "C" void fn_29_3C(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" bool fn_29_AA94(int arg0) { return *(int*)(arg0 + 0xe2c) > 0; }

extern "C" int fn_29_AD2C(int arg0) { return (*(int*)(arg0 + 0x9d4) == 4) ? 1 : 0; }

extern "C" int fn_29_AD40(int arg0) {
  if (*(int*)(arg0 + 0x9d4) == 3) {
    return 1;
  }
  return 0;
}

extern "C" bool fn_29_AD68(int arg0) { return *(int*)(arg0 + 0xe00) != 0; }

extern "C" int fn_29_AD54(int arg0) { return (*(int*)(arg0 + 0x9d4) == 2) ? 1 : 0; }

extern "C" bool fn_29_AD14(int arg0) { return *(float*)(arg0 + 0xe44) > *(float*)(arg0 + 0x7cc); }

extern "C" int fn_29_B1F4(int arg0) { return (*(int*)(arg0 + 0x6b4) == 3) ? 1 : 0; }

extern float lbl_29_rodata_8C;
extern "C" bool fn_29_AFE4(int arg0) { return *(float*)(arg0 + 0xe30) < lbl_29_rodata_8C; }

extern float lbl_29_rodata_F4;
extern "C" bool fn_29_B000(int arg0) { return *(float*)(arg0 + 0xe34) < lbl_29_rodata_F4; }

extern "C" void fn_29_74(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_29_100();
  fn_29_100();
}

extern "C" void fn_29_12D4() {
  void fn_29_12F4();
  fn_29_12F4();
}

extern "C" void fn_29_3404() {
  void fn_29_3424();
  fn_29_3424();
}

extern "C" bool fn_29_AC78(int arg0) {
  return *(unsigned short*)(arg0 + 0xd78) != kInvalidUniqueId.value;
}

extern "C" bool fn_29_B1C8(int arg0) {
  return *(unsigned short*)(arg0 + 0xcdc) != kInvalidUniqueId.value;
}

extern "C" void fn_29_BFF0(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRenderAllViewports(*(CStateManager*)arg1);
}

extern "C" void RELExit() { SetSIng_FuncPtrs(nullptr); }

extern "C" void fn_29_A400(int arg0) {
  ((CIngSpotPathFindNavigation*)(arg0 + 3248))->HasPath(*(const CPatterned*)arg0);
}

extern "C" int fn_29_ACEC(int arg0) {
  int var_r4 = false;
  if ((*(unsigned char*)(arg0 + 0xe5c) >> 4 & 1) || (*(int*)(arg0 + 0xe2c)) > 0) {
    var_r4 = true;
  }
  return var_r4;
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
extern "C" void fn_29_90(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" int fn_29_45BC();
extern "C" bool fn_29_ADBC() { return fn_29_45BC() != 0; }

extern "C" void fn_29_A428(int arg0) {
  ((CIngSpotPathFindNavigation*)(arg0 + 3248))->IsPathOver(*(const CPatterned*)arg0);
}

extern "C" int fn_29_A340(int arg0) {
  int var_r6 = false;
  if ((*(unsigned char*)(arg0 + 0xe5d) >> 3 & 1) &&
      (*(unsigned short*)(arg0 + 0x6c0)) == kInvalidUniqueId.value) {
    var_r6 = true;
  }
  return var_r6;
}

extern void* lbl_29_bss_6C;
extern "C" void fn_29_130();
extern "C" void fn_29_100() {
  lbl_29_bss_6C = &fn_29_130;
  SetSIng_FuncPtrs((SIng_FuncPtrs*)&(*(int*)&lbl_29_bss_6C));
}

extern float lbl_29_rodata_B4;
extern "C" int fn_29_B01C(int arg0) {
  int var_r5 = false;
  if ((*(unsigned char*)(arg0 + 0xdc0) >> 7 & 1) && *(float*)(arg0 + 0xdb8) > lbl_29_rodata_B4) {
    var_r5 = true;
  }
  return var_r5;
}

extern "C" int fn_29_AFA8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" bool fn_29_A3C8(int arg0) {
  return ((CPathFindSearch*)(arg0 + 2524))->OnPath(*(const CVector3f*)(arg0 + 84)) != 0;
}

extern "C" int fn_29_BB50(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_29_E650(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_29_E750(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_29_E78C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_29_B890(int arg0) {
  int temp_r0 = *(int*)(arg0 + 0x9d4);
  if (temp_r0 == 5) {
    return;
  }
  if (temp_r0 < 5 && temp_r0 < 2) {
    if (temp_r0 >= 0) {
      return;
    }
    goto block_4;
  } else {
  block_4:;
    *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime;
  }
}

extern "C" void fn_29_94A8(int arg0, int arg1, int arg2) {
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

extern "C" int fn_29_AD7C() {
  int temp_r3 = fn_29_45BC();
  if ((unsigned int)temp_r3 != 0) {
    if (*(int*)(0x158 + temp_r3) == 4) {
      return 1;
    }
    return 0;
  }
  return 0;
}

extern int lbl_29_data_C50;
extern "C" int fn_29_5104(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C50;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_29_2684(int, int, int*);
extern "C" void fn_29_2B70(int, int, int*);
extern "C" void fn_29_2E8C(int arg0, int arg1, int arg2) {
  int stack_c;
  int stack_8;
  if ((*(int*)(arg0 + 0x9d4)) == 3) {
    *(unsigned short*)&stack_c = *(unsigned short*)arg2;
    fn_29_2B70(arg0, arg1, &stack_c);
  } else {
    *(unsigned short*)&stack_8 = *(unsigned short*)arg2;
    fn_29_2684(arg0, arg1, &stack_8);
  }
}

extern "C" int fn_29_20F0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_29_4510(int arg0, int arg1) {
  ((CScriptAIHint*)arg1)->SetInUse(true);
  *(unsigned short*)(arg0 + 0xd72) = *(unsigned short*)(arg1 + 0x8);
  *(unsigned short*)(arg0 + 0xd74) = *(unsigned short*)(arg0 + 0xd72);
}

extern "C" int fn_29_ECE4(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_11E4(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 52))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_123C(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 36))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_118C(int arg0, int arg1) {
  if (arg0) {
    ((SLdrPlasmaBeamInfo*)(arg0 + 44))->~SLdrPlasmaBeamInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_B0A4();
extern "C" int fn_29_B04C(int arg0) {
  s32 var_r31 = false;
  if ((unsigned char)fn_29_B0A4() &&
      (*(unsigned short*)(arg0 + 0xd6e)) != (*(unsigned short*)(arg0 + 0xd6c))) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_29_D3A0(int arg0, int arg1) {
  void fn_29_D3F8(int, int);
  if (arg0) {
    fn_29_D3F8(arg0 + 80, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_29_D4A8(int, int);
extern "C" int fn_29_D450(int arg0, int arg1) {
  if (arg0) {
    fn_29_D4A8(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_E6F8(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_29_data_C44;
extern "C" int fn_29_6158(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C44;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_29_data_C38;
extern "C" int fn_29_6D44(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C38;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_29_data_C2C;
extern "C" int fn_29_8C98(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C2C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_29_rodata_6C;
extern "C" void fn_29_4560(int arg0) {
  int temp_r31 = fn_29_45BC();
  if (temp_r31) {
    ((CScriptAIHint*)temp_r31)->SetInUse(false);
    *(float*)(temp_r31 + 0x170) = lbl_29_rodata_6C;
    *(unsigned short*)(arg0 + 0xd72) = kInvalidUniqueId.value;
  }
}

extern int lbl_29_data_C20;
extern "C" int fn_29_8E94(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C20;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_29_data_C14;
extern "C" int fn_29_944C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C14;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_29_data_C08;
extern "C" int fn_29_9A18(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_29_data_C08;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_29_data_C50;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_157C(int arg0, int arg1) {
  if (arg0) {
    if (*(unsigned char*)(arg0 + 0x4c)) {
      ((CModelData*)arg0)->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_1518(int arg0, int arg1) {
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

extern float lbl_29_rodata_C8;
extern "C" void fn_29_BF80(int arg0, int arg1) {
  switch (*(int*)(arg0 + 0x9d4)) {
  case 2:
  case 4:
  case 6:
  case 7:
  case 8:
  case 9:
    ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
    break;
  case 3:
  case 5:
    if (*(float*)(arg0 + 0xe40) < lbl_29_rodata_C8) {
      ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
    }
    break;
  }
}

extern "C" bool fn_29_ADE8(int arg0) {
  float temp_f2;
  float temp_f2_2;
  float temp_f4;
  float temp_f5;
  int temp_r3 = fn_29_45BC();
  if ((unsigned int)temp_r3 != 0) {
    temp_f2 = *(float*)(arg0 + 0x7c4);
    temp_f5 = *(float*)(arg0 + 0x58) - *(float*)(temp_r3 + 0x58);
    temp_f4 = *(float*)(arg0 + 0x54) - *(float*)(temp_r3 + 0x54);
    temp_f2_2 = *(float*)(arg0 + 0x5c) - *(float*)(temp_r3 + 0x5c);
    return temp_f2_2 * temp_f2_2 + (temp_f4 * temp_f4 + temp_f5 * temp_f5) > temp_f2 * temp_f2;
  }
  return true;
}

extern "C" void fn_29_131C();
extern "C" void fn_29_12F4(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_29_131C();
  }
}

extern "C" void fn_29_344C();
extern "C" void fn_29_3424(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_29_344C();
  }
}

extern "C" int fn_29_AA04(int arg0) {
  int fn_29_AD40();
  u32 var_r31 = false;
  if ((*(int*)(arg0 + 0x9d8)) == 3 && (unsigned char)fn_29_AD40() == 0) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_29_AA4C(int arg0) {
  int fn_29_AD54();
  s32 var_r31 = false;
  if ((*(int*)(arg0 + 0x9d8)) == 2 && (unsigned char)fn_29_AD54() == 0) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_29_2098(int arg0, int arg1) {
  void fn_29_20F0(int, int);
  if (arg0) {
    fn_29_20F0(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_29_D3F8(int arg0, int arg1) {
  void fn_29_D450(int, int);
  if (arg0) {
    fn_29_D450(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_29_5C28 {
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
  virtual float _10();
};
extern "C" void fn_29_5C28(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    *(int*)(arg0 + 0x9d4) = 5;
    break;
  case 1:
    if (((__mwdec_vt_0_fn_29_5C28*)*(int*)(arg0 + 0x350))->_10() > 2.0f) {
      ((CPatterned*)arg0)->DeathDelete(*(CStateManager*)arg1);
    }
    break;
  }
}

extern unsigned char lbl_29_data_250[784];
extern unsigned char lbl_29_data_6BC[464];
extern unsigned char lbl_29_data_91C[192];
struct __mwdec_vt_0_fn_29_B208 {
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
extern "C" void fn_29_B208(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_29_B208*)temp_r31)->_4(lbl_29_data_250, 49);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_29_data_6BC, 29);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_29_data_91C, 12);
}

extern "C" int fn_29_10F8(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 368) {
      ((SLdrPlasmaBeamInfo*)(arg0 + 412))->~SLdrPlasmaBeamInfo();
    }
    if (arg0 + 268) {
      ((CDamageVulnerability*)(arg0 + 320))->~CDamageVulnerability();
    }
    if (arg0 + 172) {
      ((CDamageVulnerability*)(arg0 + 208))->~CDamageVulnerability();
    }
    ((CDamageVulnerability*)(arg0 + 60))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_29_4C88(int arg0, int arg1) {
  int var_r3 = 2;
  if (((*(unsigned char*)(arg0 + 0x9d0)) >> 7 & 1)) {
    var_r3 = 3;
  }
  *(int*)(arg0 + 0x9d4) = var_r3;
  if (((*(unsigned char*)(arg0 + 0x9d0)) >> 7 & 1)) {
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)0);
    ((CSurfaceAlignmentHelper*)(arg0 + 3160))
        ->AlignNearPosition(*(CActor*)arg0, *(CStateManager*)arg1, *(const CVector3f*)(arg0 + 84),
                            1.0f);
  } else {
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)1);
  }
}

extern "C" void fn_29_4AA8(int arg0, int arg1) {
  *(float*)arg0 = *(float*)arg1;
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x4);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x8);
  *(float*)(arg0 + 0xc) = *(float*)(arg1 + 0xc);
  *(float*)(arg0 + 0x10) = *(float*)(arg1 + 0x10);
  *(float*)(arg0 + 0x14) = *(float*)(arg1 + 0x14);
  *(int*)(arg0 + 0x18) = *(int*)(arg1 + 0x18);
  *(int*)(arg0 + 0x1c) = *(int*)(arg1 + 0x1c);
  *(float*)(arg0 + 0x20) = *(float*)(arg1 + 0x20);
  *(float*)(arg0 + 0x24) = *(float*)(arg1 + 0x24);
  *(float*)(arg0 + 0x28) = *(float*)(arg1 + 0x28);
  *(float*)(arg0 + 0x2c) = *(float*)(arg1 + 0x2c);
  *(float*)(arg0 + 0x30) = *(float*)(arg1 + 0x30);
  *(float*)(arg0 + 0x34) = *(float*)(arg1 + 0x34);
  *(float*)(arg0 + 0x38) = *(float*)(arg1 + 0x38);
  *(int*)(arg0 + 0x3c) = *(int*)(arg1 + 0x3c);
  *(int*)(arg0 + 0x40) = *(int*)(arg1 + 0x40);
  *(float*)(arg0 + 0x44) = *(float*)(arg1 + 0x44);
  *(float*)(arg0 + 0x48) = *(float*)(arg1 + 0x48);
  *(unsigned char*)(arg0 + 0x4c) = *(unsigned char*)(arg1 + 0x4c);
}

extern "C" void fn_29_B8C0(int arg0, int arg1, int arg2) {
  if ((*(int*)(arg0 + 0x9d4)) == 2 && (*(int*)((*(int*)(arg0 + 0x48c)) + 0x37c)) != 6) {
    ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)2, true);
  } else {
    ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)2, false);
  }
  if ((int)*(unsigned short*)(arg2 + 0x10) == 2) {
    *(int*)(arg0 + 0x688) = 2;
  } else {
    *(int*)(arg0 + 0x688) = 1;
  }
  ((CPatterned*)arg0)->CPatterned::KnockBack(*(CStateManager*)arg1, *(const CKnockBackInfo*)arg2);
}

extern "C" void fn_29_84A4(int arg0, int arg1, int arg2, float arg3) {
  int temp_r31;
  float temp_f31;
  float temp_f1;
  switch (arg2) {
  case 0:
    temp_r31 = *(int*)(arg0 + 0x48c);
    temp_f31 = ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)2);
    temp_f1 =
        ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)1) / temp_f31;
    *(int*)((*(int*)(arg0 + 0x48c)) + 0x4c) = 1;
    ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->SetSteeringSpeedRange(temp_f1, temp_f1);
    break;
  }
  ((CWaypointNavigation*)(arg0 + 1720))
      ->Patrol(*(CStateManager*)arg1, (EStateMsg)arg2, arg3, *(CPatterned*)arg0);
}

extern "C" int fn_29_15DC(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 1824))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 1476))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 1128))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)(arg0 + 1080))->~SLdrDamageInfo();
    ((SLdrPlasmaBeamInfo*)(arg0 + 1004))->~SLdrPlasmaBeamInfo();
    ((SLdrDamageInfo*)(arg0 + 988))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 916))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 900))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_29_69A0(int arg0, int arg1, int arg2) {
  int stack_8;
  switch (arg2) {
  case 0:
    *(int*)(arg0 + 0x6b4) = 1;
    CPathFindPointSearchFilter stack_c(1000.0f, 0, -1);
    *(int*)&stack_8 = -1;
    if (!((CPathFindPointSearch*)(arg0 + 2760))
             ->FindClosestPhysicalPoint(*(const CVector3f*)(arg0 + 84), stack_8, stack_c)) {
      ((CActor*)arg0)
          ->SetTranslation(
              *(const CVector3f*)(*(int*)((*(int*)(arg0 + 0xac8)) + 0x190) + *(int*)&stack_8 * 28));
      ((CSurfaceAlignmentHelper*)(arg0 + 3160))
          ->AlignNearPosition(*(CActor*)arg0, *(CStateManager*)arg1, *(const CVector3f*)(arg0 + 84),
                              1.0f);
      *(int*)(arg0 + 0x6b4) = 3;
    }
    break;
  case 2:
    *(int*)(arg0 + 0x6b4) = 0;
    break;
  }
}
