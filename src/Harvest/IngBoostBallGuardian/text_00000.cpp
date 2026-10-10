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

extern "C" void fn_30_B764() {}

extern "C" void fn_30_B768() {}

extern "C" int fn_30_0(int arg0) { return arg0 + 2796; }

extern "C" bool fn_30_10() { return true; }

extern "C" int fn_30_24(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_30_2C() { return false; }

extern "C" bool fn_30_34() { return false; }

extern "C" int fn_30_4028(int arg0) { return *(int*)(arg0 + 0x254); }

extern "C" int fn_30_64(int arg0) { return arg0 + 1876; }

extern "C" bool fn_30_6C() { return true; }

extern "C" int fn_30_8(int arg0) { return arg0 + 3032; }

extern float lbl_30_rodata_64;
extern "C" float fn_30_18() { return lbl_30_rodata_64; }

extern "C" void fn_30_2094(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" bool fn_30_4C(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" float fn_30_58() { return kDefaultGravityAccel; }

extern "C" bool fn_30_B788(int arg0) { return *(unsigned char*)(arg0 + 0x11ee) & 1; }

extern "C" bool fn_30_B794(int arg0) { return *(unsigned char*)(arg0 + 0x11ee) >> 4 & 1; }

extern "C" bool fn_30_B7A0(int arg0) { return *(unsigned char*)(arg0 + 0x11ef) >> 7 & 1; }

extern "C" bool fn_30_B7D4(int arg0) { return *(unsigned char*)(arg0 + 0x11ee) >> 5 & 1; }

extern "C" bool fn_30_BAA8(int arg0) { return *(unsigned char*)(arg0 + 0x11ec) >> 4 & 1; }

extern "C" bool fn_30_BC20(int arg0) { return *(unsigned char*)(arg0 + 0x11ed) & 1; }

extern "C" bool fn_30_BE88(int arg0) { return *(unsigned char*)(arg0 + 0x11ec) >> 7 & 1; }

extern "C" bool fn_30_C1A4(int arg0) { return *(unsigned char*)(arg0 + 0x11ed) >> 1 & 1; }

extern "C" bool fn_30_C6AC(int arg0) { return *(unsigned char*)(arg0 + 0x11ed) >> 3 & 1; }

extern "C" void fn_30_E4E0(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" void fn_30_3C(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" bool fn_30_C05C(int arg0) { return *(int*)(arg0 + 0x115c) > 0; }

extern "C" int fn_30_C1F0(int arg0) { return (*(int*)(arg0 + 0xae4) == 4) ? 1 : 0; }

extern "C" int fn_30_C204(int arg0) { return (*(int*)(arg0 + 0xae4) == 3) ? 1 : 0; }

extern "C" int fn_30_C218(int arg0) { return (*(int*)(arg0 + 0xae4) == 2) ? 1 : 0; }

extern "C" bool fn_30_C22C(int arg0) { return *(int*)(arg0 + 0x1130) != 0; }

extern "C" int fn_30_C6B8(int arg0) {
  if (*(int*)(0x6b4 + arg0) == 3) {
    return 1;
  }
  return 0;
}

extern "C" bool fn_30_C1D8(int arg0) { return *(float*)(arg0 + 0x11c0) > *(float*)(arg0 + 0x83c); }

extern "C" void fn_30_74(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern float lbl_30_rodata_98;
extern "C" bool fn_30_C4A8(int arg0) { return *(float*)(arg0 + 0x11ac) < lbl_30_rodata_98; }

extern float lbl_30_rodata_FC;
extern "C" bool fn_30_C4C4(int arg0) { return *(float*)(arg0 + 0x11b0) < lbl_30_rodata_FC; }

extern "C" void RELMain() {
  void fn_30_100();
  fn_30_100();
}

extern "C" void fn_30_111C() {
  void fn_30_113C();
  fn_30_113C();
}

extern "C" int fn_30_B76C(int arg0) {
  if (*(unsigned short*)(arg0 + 0x1096) == kInvalidUniqueId.value) {
    return 1;
  }
  return 0;
}

extern "C" void fn_30_4C10() {
  void fn_30_4C30();
  fn_30_4C30();
}

extern "C" void fn_30_A8C() {
  void fn_30_AAC();
  fn_30_AAC();
}

extern "C" void fn_30_D230() { &CDamageVulnerability::ImmuneVulnerabilty(); }

extern "C" bool fn_30_C68C(int arg0) {
  return *(unsigned short*)(arg0 + 0xfd8) != kInvalidUniqueId.value;
}

extern "C" void fn_30_D2C0(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRenderAllViewports(*(CStateManager*)arg1);
}

extern "C" int fn_30_B7AC(int arg0) {
  s32 var_r4 = false;
  if ((*(unsigned char*)(arg0 + 0x11ee) >> 3 & 1) && !(*(unsigned char*)(arg0 + 0x11ed) >> 1 & 1)) {
    var_r4 = true;
  }
  return var_r4;
}

extern "C" void RELExit() { SetSIngBoostBallGuardian_FuncPtrs(nullptr); }

extern "C" void fn_30_B9C8(int arg0) {
  ((CIngSpotPathFindNavigation*)(arg0 + 3520))->HasPath(*(const CPatterned*)arg0);
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
extern "C" void fn_30_90(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" int fn_30_60C0();
extern "C" bool fn_30_C280() { return fn_30_60C0() != 0; }

extern "C" int fn_30_C1B0(int arg0) {
  int var_r4 = false;
  if ((*(unsigned char*)(0x11ec + arg0) >> 5 & 1) || (*(int*)(arg0 + 0x115c)) > 0) {
    var_r4 = true;
  }
  return var_r4;
}

extern "C" void fn_30_B9F0(int arg0) {
  ((CIngSpotPathFindNavigation*)(arg0 + 3520))->IsPathOver(*(const CPatterned*)arg0);
}

extern void* lbl_30_bss_6C;
extern "C" void fn_30_130();
extern "C" void fn_30_100() {
  lbl_30_bss_6C = &fn_30_130;
  SetSIngBoostBallGuardian_FuncPtrs((SIngBoostBallGuardian_FuncPtrs*)&(*(int*)&lbl_30_bss_6C));
}

extern float lbl_30_rodata_80;
extern "C" int fn_30_C4E0(int arg0) {
  u32 var_r5 = false;
  if ((*(unsigned char*)(arg0 + 0x10f0) >> 7 & 1) && *(float*)(arg0 + 0x10e8) > lbl_30_rodata_80) {
    var_r5 = true;
  }
  return var_r5;
}

extern "C" int fn_30_B908(int arg0) {
  int var_r6 = false;
  if ((*(unsigned char*)(0x11ed + arg0) >> 4 & 1) &&
      (*(unsigned short*)(arg0 + 0x6c0)) == kInvalidUniqueId.value) {
    var_r6 = true;
  }
  return var_r6;
}

extern "C" void fn_30_76F4(int arg0, int arg1, int arg2, int arg3) {
  if ((*(int*)(arg0 + 0xae4)) == 7) {
    *(int*)(arg0 + 0xae4) = 2;
  }
  ((CPatterned*)arg0)
      ->CPatterned::Death(*(CStateManager*)arg1, *(const CVector3f*)arg2, (EScriptObjectState)arg3);
}

extern "C" bool fn_30_B990(int arg0) {
  return ((CPathFindSearch*)(arg0 + 2796))->OnPath(*(const CVector3f*)(arg0 + 84)) != 0;
}

extern "C" int fn_30_101A8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_30_104F4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_30_10530(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_30_18B4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_30_C46C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_30_194C(int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6,
                           float arg7, float arg8, float arg9, float arg10, float arg11,
                           float arg12, float arg13, float arg14) {
  *(float*)arg0 = arg7;
  *(float*)(arg0 + 0x4) = arg8;
  *(float*)(arg0 + 0x8) = arg9;
  *(float*)(arg0 + 0xc) = arg10;
  *(float*)(arg0 + 0x10) = arg11;
  *(float*)(arg0 + 0x14) = arg12;
  *(int*)(arg0 + 0x18) = arg1;
  *(int*)(arg0 + 0x1c) = arg2;
  *(int*)(arg0 + 0x20) = arg3;
  *(int*)(arg0 + 0x24) = arg4;
  *(int*)(arg0 + 0x28) = arg5;
  *(int*)(arg0 + 0x2c) = arg6;
  *(float*)(arg0 + 0x30) = arg13;
  *(float*)(arg0 + 0x34) = arg14;
}

extern "C" int fn_30_CE14(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_30_CB54(int arg0) {
  int temp_r0 = *(int*)(arg0 + 0xae4);
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

extern "C" void fn_30_A91C(int arg0, int arg1, int arg2) {
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

extern "C" void fn_30_3068(int, int, int);
extern "C" void fn_30_4030(int arg0, int arg1, float arg2, float arg3) {
  *(float*)(arg0 + 0x11d4) = *(float*)(arg0 + 0x11d4) + arg3;
  if (*(float*)(arg0 + 0x11d4) >= arg2 * *(float*)(arg0 + 0xec0)) {
    fn_30_3068(arg0, arg1, 1);
  }
}

extern int lbl_30_data_BF0;
extern "C" int fn_30_6B74(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BF0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_30_4078(int, int, int*);
extern "C" void fn_30_44A4(int, int, int*);
extern "C" void fn_30_479C(int arg0, int arg1, int arg2) {
  int stack_c;
  int stack_8;
  if ((*(int*)(arg0 + 0xae4)) == 3) {
    *(unsigned short*)&stack_c = *(unsigned short*)arg2;
    fn_30_44A4(arg0, arg1, &stack_c);
  } else {
    *(unsigned short*)&stack_8 = *(unsigned short*)arg2;
    fn_30_4078(arg0, arg1, &stack_8);
  }
}

extern "C" void fn_30_6014(int arg0, int arg1) {
  ((CScriptAIHint*)arg1)->SetInUse(true);
  *(unsigned short*)(arg0 + 0x108e) = *(unsigned short*)(arg1 + 0x8);
  *(unsigned short*)(arg0 + 0x1090) = *(unsigned short*)(arg0 + 0x108e);
}

extern "C" int fn_30_3838(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_B30(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_6F4(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_E4EC(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_1049C(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_C568();
extern "C" int fn_30_C510(int arg0) {
  s32 var_r31 = false;
  if ((unsigned char)fn_30_C568() &&
      (*(unsigned short*)(arg0 + 0x108a)) != (*(unsigned short*)(arg0 + 0x1088))) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_30_EACC(int arg0, int arg1) {
  void fn_30_EB24(int, int);
  if (arg0) {
    fn_30_EB24(arg0 + 80, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_30_EBD4(int, int);
extern "C" int fn_30_EB7C(int arg0, int arg1) {
  if (arg0) {
    fn_30_EBD4(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_30_data_BE4;
extern "C" int fn_30_7D00(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BE4;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_30_data_BF0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_30_data_BD8;
extern "C" int fn_30_84C4(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BD8;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_30_data_BF0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_30_data_BCC;
extern "C" int fn_30_A308(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BCC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_30_data_BF0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_30_data_BC0;
extern "C" int fn_30_A8C0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BC0;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_30_data_BF0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_30_data_BB4;
extern "C" int fn_30_AFE0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_30_data_BB4;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_30_data_BF0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_13C4(int arg0, int arg1) {
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

extern "C" int fn_30_1078(int arg0, int arg1) {
  void fn_30_2C44(int, int);
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 668))->~CDamageVulnerability();
    fn_30_2C44(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_30_rodata_70;
extern "C" void fn_30_6064(int arg0) {
  int temp_r31 = fn_30_60C0();
  if (temp_r31) {
    ((CScriptAIHint*)temp_r31)->SetInUse(false);
    *(float*)(temp_r31 + 0x170) = lbl_30_rodata_70;
    *(unsigned short*)(arg0 + 0x108e) = kInvalidUniqueId.value;
  }
}

extern "C" int fn_30_1360(int arg0, int arg1) {
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

extern float lbl_30_rodata_E0;
extern "C" void fn_30_D250(int arg0, int arg1) {
  switch (*(int*)(arg0 + 0xae4)) {
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
    if (*(float*)(arg0 + 0x11bc) < lbl_30_rodata_E0) {
      ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
    }
    break;
  }
}

extern "C" bool fn_30_C2AC(int arg0) {
  float temp_f2;
  float temp_f2_2;
  float temp_f4;
  float temp_f5;
  int temp_r3 = fn_30_60C0();
  if ((unsigned int)temp_r3 != 0) {
    temp_f2 = *(float*)(arg0 + 0x834);
    temp_f5 = *(float*)(arg0 + 0x58) - *(float*)(temp_r3 + 0x58);
    temp_f4 = *(float*)(arg0 + 0x54) - *(float*)(temp_r3 + 0x54);
    temp_f2_2 = *(float*)(arg0 + 0x5c) - *(float*)(temp_r3 + 0x5c);
    return temp_f2_2 * temp_f2_2 + (temp_f4 * temp_f4 + temp_f5 * temp_f5) > temp_f2 * temp_f2;
  }
  return true;
}

extern "C" void fn_30_4C58();
extern "C" void fn_30_4C30(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_30_4C58();
  }
}

extern "C" void fn_30_1164();
extern "C" void fn_30_113C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_30_1164();
  }
}

extern "C" void fn_30_F78();
extern "C" void fn_30_AAC(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_30_F78();
  }
}

extern "C" int fn_30_BFCC(int arg0) {
  int fn_30_C204();
  int var_r31 = false;
  if ((*(int*)(arg0 + 0xae8)) == 3 && (unsigned char)fn_30_C204() == 0) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_30_C014(int arg0) {
  int fn_30_C218();
  s32 var_r31 = false;
  if ((*(int*)(arg0 + 0xae8)) == 2 && (unsigned char)fn_30_C218() == 0) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_30_37E0(int arg0, int arg1) {
  void fn_30_3838(int, int);
  if (arg0) {
    fn_30_3838(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_69C(int arg0, int arg1) {
  void fn_30_6F4(int, int);
  if (arg0) {
    fn_30_6F4(arg0 + 8, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_EB24(int arg0, int arg1) {
  void fn_30_EB7C(int, int);
  if (arg0) {
    fn_30_EB7C(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_A24(int arg0, int arg1, int arg2) {
  void fn_30_A8C(int, int);
  int var_r31 = arg0;
  int var_r30 = arg2;
  int var_r29 = arg1;
  while (var_r29) {
    fn_30_A8C(var_r30, var_r31);
    var_r29 = var_r29 - 1;
    var_r31 = var_r31 + 56;
    var_r30 = var_r30 + 56;
  }
  return var_r30;
}

extern "C" int fn_30_1424(int arg0, int arg1) {
  void fn_30_14AC(int, int);
  void fn_30_2C44(int, int);
  if (arg0) {
    fn_30_14AC(arg0 + 1428, -1);
    fn_30_2C44(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_30_2C44(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 320))->~SLdrDamageVulnerability();
    ((SLdrPlasmaBeamInfo*)(arg0 + 240))->~SLdrPlasmaBeamInfo();
    ((SLdrDamageInfo*)(arg0 + 224))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 152))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 136))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_30_data_250[784];
extern unsigned char lbl_30_data_6B0[448];
extern unsigned char lbl_30_data_8DC[144];
struct __mwdec_vt_0_fn_30_C6CC {
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
extern "C" void fn_30_C6CC(int arg0, int arg1) {
  int temp_r31 = *(int*)((char*)arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_30_C6CC*)temp_r31)->_4(lbl_30_data_250, 49);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_30_data_6B0, 28);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_30_data_8DC, 9);
}

extern "C" int fn_30_14AC(int arg0, int arg1) {
  void fn_30_69C(int, int);
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 548))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 532))->~SLdrDamageInfo();
    ((SLdrDamageVulnerability*)(arg0 + 168))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)(arg0 + 104))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 88))->~SLdrDamageInfo();
    fn_30_69C(arg0 + 20, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_30_68F0(int arg0, int arg1) {
  int var_r3 = 2;
  if (((*(unsigned char*)((char*)arg0 + 0xae0)) >> 7 & 1)) {
    var_r3 = 3;
  }
  *(int*)((char*)arg0 + 0xae4) = var_r3;
  if (((*(unsigned char*)((char*)arg0 + 0xae0)) >> 7 & 1)) {
    ((CBodyController*)*(int*)((char*)arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)0);
    ((CSurfaceAlignmentHelper*)(arg0 + 3432))
        ->AlignNearPosition(*(CActor*)arg0, *(CStateManager*)arg1, *(const CVector3f*)(arg0 + 84),
                            1.0f);
  } else {
    ((CBodyController*)*(int*)((char*)arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)1);
  }
}

extern "C" void fn_30_6710(int arg0, int arg1) {
  *(float*)arg0 = *(float*)arg1;
  *(float*)((char*)arg0 + 0x4) = *(float*)((char*)arg1 + 0x4);
  *(float*)((char*)arg0 + 0x8) = *(float*)((char*)arg1 + 0x8);
  *(float*)((char*)arg0 + 0xc) = *(float*)((char*)arg1 + 0xc);
  *(float*)((char*)arg0 + 0x10) = *(float*)((char*)arg1 + 0x10);
  *(float*)((char*)arg0 + 0x14) = *(float*)((char*)arg1 + 0x14);
  *(int*)((char*)arg0 + 0x18) = *(int*)((char*)arg1 + 0x18);
  *(int*)((char*)arg0 + 0x1c) = *(int*)((char*)arg1 + 0x1c);
  *(float*)((char*)arg0 + 0x20) = *(float*)((char*)arg1 + 0x20);
  *(float*)((char*)arg0 + 0x24) = *(float*)((char*)arg1 + 0x24);
  *(float*)((char*)arg0 + 0x28) = *(float*)((char*)arg1 + 0x28);
  *(float*)((char*)arg0 + 0x2c) = *(float*)((char*)arg1 + 0x2c);
  *(float*)((char*)arg0 + 0x30) = *(float*)((char*)arg1 + 0x30);
  *(float*)((char*)arg0 + 0x34) = *(float*)((char*)arg1 + 0x34);
  *(float*)((char*)arg0 + 0x38) = *(float*)((char*)arg1 + 0x38);
  *(int*)((char*)arg0 + 0x3c) = *(int*)((char*)arg1 + 0x3c);
  *(int*)((char*)arg0 + 0x40) = *(int*)((char*)arg1 + 0x40);
  *(float*)((char*)arg0 + 0x44) = *(float*)((char*)arg1 + 0x44);
  *(float*)((char*)arg0 + 0x48) = *(float*)((char*)arg1 + 0x48);
  *(unsigned char*)((char*)arg0 + 0x4c) = *(unsigned char*)((char*)arg1 + 0x4c);
}

extern "C" void fn_30_CB84(int arg0, int arg1, int arg2) {
  if ((*(int*)((char*)arg0 + 0xae4)) == 2 &&
      (*(int*)((char*)*(int*)((char*)arg0 + 0x48c) + 0x37c)) != 6) {
    ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)2, true);
  } else {
    ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)2, false);
  }
  if ((int)*(unsigned short*)((char*)arg2 + 0x10) == 2) {
    *(int*)((char*)arg0 + 0x688) = 2;
  } else {
    *(int*)((char*)arg0 + 0x688) = 1;
  }
  ((CPatterned*)arg0)->CPatterned::KnockBack(*(CStateManager*)arg1, *(const CKnockBackInfo*)arg2);
}

extern "C" void fn_30_9BB0(int arg0, int arg1, int arg2, float arg3) {
  int temp_r31;
  float temp_f31;
  float temp_f1;
  switch (arg2) {
  case 0:
    temp_r31 = *(int*)((char*)arg0 + 0x48c);
    temp_f31 = ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)2);
    temp_f1 =
        ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)1) / temp_f31;
    *(int*)((char*)*(int*)((char*)arg0 + 0x48c) + 0x4c) = 1;
    ((CBodyStateCmdMgr*)(*(int*)((char*)arg0 + 0x48c) + 4))
        ->SetSteeringSpeedRange(temp_f1, temp_f1);
    break;
  }
  ((CWaypointNavigation*)(arg0 + 1720))
      ->Patrol(*(CStateManager*)arg1, (EStateMsg)arg2, arg3, *(CPatterned*)arg0);
}

extern "C" void fn_30_8120(int arg0, int arg1, int arg2) {
  int stack_8;
  switch (arg2) {
  case 0:
    *(int*)((char*)arg0 + 0x6b4) = 1;
    CPathFindPointSearchFilter stack_c(1000.0f, 0, -1);
    *(int*)&stack_8 = -1;
    if (!((CPathFindPointSearch*)(arg0 + 3032))
             ->FindClosestPhysicalPoint(*(const CVector3f*)(arg0 + 84), stack_8, stack_c)) {
      ((CActor*)arg0)
          ->SetTranslation(
              *(const CVector3f*)(*(int*)((char*)*(int*)((char*)arg0 + 0xbd8) + 0x190) +
                                  *(int*)&stack_8 * 28));
      ((CSurfaceAlignmentHelper*)(arg0 + 3432))
          ->AlignNearPosition(*(CActor*)arg0, *(CStateManager*)arg1, *(const CVector3f*)(arg0 + 84),
                              1.0f);
      *(int*)((char*)arg0 + 0x6b4) = 3;
    }
    break;
  case 2:
    *(int*)((char*)arg0 + 0x6b4) = 0;
    break;
  }
}
