// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
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
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CStaticInterference.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" int fn_9_0(int arg0) { return arg0 + 2344; }

extern "C" bool fn_9_4D7C() { return false; }

extern "C" bool fn_9_5F34() { return false; }

extern "C" int fn_9_68(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_9_70() { return false; }

extern "C" bool fn_9_78() { return false; }

extern "C" bool fn_9_8() { return true; }

extern "C" int fn_9_9C(int arg0) { return arg0 + 1876; }

extern "C" bool fn_9_A4() { return true; }

extern "C" int fn_9_D474(int arg0) { return arg0 + 2580; }

extern float lbl_9_rodata_400;
extern "C" float fn_9_10() { return lbl_9_rodata_400; }

extern "C" bool fn_9_90(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" bool fn_9_B1C8(int arg0) { return *(unsigned char*)(arg0 + 0xc64) >> 6 & 1; }

extern "C" bool fn_9_B1D4(int arg0) { return *(unsigned char*)(arg0 + 0xc64) >> 2 & 1; }

extern "C" bool fn_9_BAEC(int arg0) { return *(unsigned char*)(arg0 + 0xae8) >> 7 & 1; }

extern "C" bool fn_9_BAF8(int arg0) { return *(unsigned char*)(arg0 + 0x91c) >> 6 & 1; }

extern "C" bool fn_9_BB04(int arg0) { return *(unsigned char*)(arg0 + 0x91c) >> 7 & 1; }

extern "C" void fn_9_E7EC(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" void fn_9_80(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" int fn_9_BDCC(int arg0) { return (*(int*)(arg0 + 0x6b4) == 3) ? 1 : 0; }

extern "C" void fn_9_AC(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" bool fn_9_AFB8(int arg0) { return *(float*)(arg0 + 0xb8c) >= *(float*)(arg0 + 0xb90); }

extern "C" void fn_9_58(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" int fn_9_B7FC(int arg0) {
  if (*(unsigned short*)(arg0 + 0x6c0) == kInvalidUniqueId.value) {
    return 1;
  }
  return 0;
}

extern float lbl_9_rodata_47C;
extern "C" bool fn_9_BAB4(int arg0) { return *(float*)(arg0 + 0xb60) < lbl_9_rodata_47C; }

extern "C" void RELMain() {
  void fn_9_138();
  fn_9_138();
}

extern float lbl_9_rodata_4E0;
extern "C" bool fn_9_BAD0(int arg0) { return *(float*)(arg0 + 0xb64) < lbl_9_rodata_4E0; }

extern "C" void fn_9_49D8() {
  void fn_9_49F8();
  fn_9_49F8();
}

extern "C" CModelData fn_9_5780(int arg0) { return CModelData(); }

extern "C" void fn_9_600C();
extern "C" void fn_9_5FEC() { fn_9_600C(); }

extern "C" void fn_9_6190() {
  void fn_9_61B0();
  fn_9_61B0();
}

extern "C" void fn_9_A60() {
  void fn_9_A80();
  fn_9_A80();
}

extern float lbl_9_rodata_494;
extern "C" bool fn_9_B0B4(int arg0) { return *(float*)(arg0 + 0xb5c) <= lbl_9_rodata_494; }

extern "C" bool fn_9_B0D4(int arg0) {
  return *(unsigned short*)(arg0 + 0xbd2) != kInvalidUniqueId.value;
}

extern "C" bool fn_9_B1A8(int arg0) {
  return *(unsigned short*)(arg0 + 0xbd0) != kInvalidUniqueId.value;
}

extern "C" bool fn_9_B0F4(int arg0) {
  if ((*(unsigned char*)(arg0 + 0x91c) >> 3 & 1)) {
    return false;
  } else {
    return *(unsigned char*)(arg0 + 0xc63) & 1;
  }
}

extern "C" void fn_9_C4FC(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
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
extern "C" void fn_9_C8(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" void RELExit() { SetSCommandoPirate_FuncPtrs(nullptr); }

extern void* lbl_9_bss_28;
extern "C" void fn_9_168();
extern "C" void fn_9_138() {
  lbl_9_bss_28 = &fn_9_168;
  SetSCommandoPirate_FuncPtrs((SCommandoPirate_FuncPtrs*)&(*(int*)&lbl_9_bss_28));
}

extern "C" int fn_9_B660(int arg0) {
  s32 temp_r4 = *(unsigned char*)(0xc64 + arg0);
  s32 var_r5 = false;
  if ((temp_r4 >> 7 & 1) && !(*(unsigned char*)(arg0 + 0x91d) >> 5 & 1) && !(temp_r4 & 1)) {
    var_r5 = true;
  }
  return var_r5;
}

extern "C" void fn_9_ADA0(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)0);
    break;
  }
}

extern "C" int fn_9_BD98(int arg0) {
  u32 var_r4 = false;
  if ((*(int*)(arg0 + 0x920)) == -1 && (*(unsigned char*)(arg0 + 0x91d) >> 7 & 1) &&
      (*(unsigned char*)(arg0 + 0xc63) >> 5 & 1)) {
    var_r4 = true;
  }
  return var_r4;
}

extern "C" int fn_9_C2CC(int arg0) {
  if ((*(unsigned char*)(arg0 + 0xc65) >> 2 & 1)) {
    return arg0 + 2628;
  }
  return (int)((CPatterned*)arg0)->CPatterned::GetDamageVulnerability();
}

extern "C" bool fn_9_B958(int arg0) {
  return ((CPathFindSearch*)(arg0 + 2344))->OnPath(*(const CVector3f*)(arg0 + 84)) != 0;
}

extern "C" int fn_9_4568(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_5D38(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_5EF8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_6318(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_6354(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_958C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_E8BC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_E8F8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_9_E9E8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_9_4188(int, int);
extern "C" int fn_9_4140(int arg0, int arg1) {
  fn_9_4188(*(int*)arg0, 1);
  *(int*)arg0 = arg1;
  return arg0;
}

extern int lbl_9_data_AA0;
extern "C" int fn_9_2A50(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_AA0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_9_609C(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_9_6054(int arg0) {
  void fn_9_5F90(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_9_609C(arg0, stack_24, stack_14, stack_8);
  fn_9_5F90(stack_24, -1);
}

extern "C" bool fn_9_BB7C(int arg0) {
  if ((*(unsigned char*)(arg0 + 0xc63) >> 6 & 1) && !(*(unsigned char*)(arg0 + 0x91d) >> 7 & 1) &&
      !(*(unsigned char*)(arg0 + 0x91c) >> 3 & 1)) {
    return *(unsigned short*)(arg0 + 0xbce) != kInvalidUniqueId.value;
  } else {
    return false;
  }
}

extern "C" int fn_9_23D0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_B990(int arg0, int arg1, int arg2) {
  s32 var_r31 = false;
  if (!(*(unsigned char*)(arg0 + 0xc65) >> 7 & 1) ||
      ((CPatterned*)arg0)->PathOver(*(CStateManager*)arg1, *(const CTriggerData*)arg2)) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_9_95C8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_D2F0(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_934(int arg0, int arg1) {
  if (arg0) {
    ((CDamageVulnerability*)(arg0 + 28))->~CDamageVulnerability();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_BA78();
extern "C" int fn_9_BA20(int arg0) {
  s32 var_r31 = false;
  if ((unsigned char)fn_9_BA78() &&
      (*(unsigned short*)(arg0 + 0xbcc)) != (*(unsigned short*)(arg0 + 0xbca))) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_9_D8AC(int arg0, int arg1) {
  if (arg0) {
    fn_9_4188(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_11B4(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 20))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A94;
extern "C" int fn_9_29F4(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A94;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A78;
extern "C" int fn_9_4FAC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A78;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_E934(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A6C;
extern "C" int fn_9_6B70(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A6C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A44;
extern "C" int fn_9_7668(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A44;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A60;
extern "C" int fn_9_7040(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A60;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A38;
extern "C" int fn_9_8B14(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A38;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A2C;
extern "C" int fn_9_9008(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A2C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A20;
extern "C" int fn_9_9E4C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A20;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A14;
extern "C" int fn_9_A460(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A14;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_A08;
extern "C" int fn_9_A74C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_A08;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_9FC;
extern "C" int fn_9_AF5C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_9_data_9FC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_9_data_AA0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_8D4(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 216) {
      ((CDamageVulnerability*)(arg0 + 244))->~CDamageVulnerability();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_D08(int arg0, int arg1) {
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

extern "C" int fn_9_1054(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 16))->~SLdrDamageVulnerability();
    ((SLdrDamageInfo*)arg0)->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_CA4(int arg0, int arg1) {
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

template < class T0 >
int TCastToPtr(CEntity*);
extern "C" bool fn_9_B1E0(int arg0, int arg1) {
  int temp_r3;
  if ((*(unsigned char*)(arg0 + 0xc66) >> 7 & 1) && !(*(unsigned char*)(arg0 + 0xc64) & 1)) {
    temp_r3 = TCastToPtr< CScriptAIWaypoint >(
        ((CStateManager*)arg1)->ObjectById(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x6c0))));
    if ((unsigned int)temp_r3 != 0 && (*(int*)(temp_r3 + 0x164) & 32)) {
      return true;
    }
  }
  return false;
}

extern "C" void fn_9_CAC8(int arg0, int arg1, float arg2) {
  int temp_r3 = *(int*)(arg0 + 0xbfc);
  if ((unsigned int)temp_r3 == 0 || !(*(unsigned char*)(temp_r3 + 0x70) >> 6 & 1)) {
    ((CBoneTracking*)(arg0 + 2676))->PreThink(*(CAnimData*)*(int*)((*(int*)(arg0 + 0x60)) + 0x10));
  }
  ((CPatterned*)arg0)->CPatterned::PreThink(arg2, *(CStateManager*)arg1);
}

extern "C" void fn_9_ED04();
extern "C" int fn_9_F50C(int arg0, float arg1, float arg2, float arg3, float arg4, float arg5,
                         float arg6, float arg7, float arg8) {
  fn_9_ED04();
  *(float*)(arg0 + 0x50) = arg1;
  *(float*)(arg0 + 0x54) = arg4;
  *(float*)(arg0 + 0x58) = arg2;
  *(float*)(arg0 + 0x5c) = arg3;
  *(float*)(arg0 + 0x60) = arg5;
  *(float*)(arg0 + 0x64) = arg6;
  *(float*)(arg0 + 0x68) = arg7;
  *(float*)(arg0 + 0x6c) = arg8;
  return arg0;
}

extern float lbl_9_rodata_458;
extern "C" void fn_9_C51C(int arg0, int arg1) {
  int temp_r3 = *(int*)(arg0 + 0xbfc);
  if ((unsigned int)temp_r3 != 0 && (*(unsigned char*)(temp_r3 + 0x70) >> 6 & 1)) {
    ((CRagDoll*)temp_r3)->PreRenderAllViewports(*(CActor*)arg0, lbl_9_rodata_458);
    ((CActor*)arg0)->UpdatePortalSystemState(*(CStateManager*)arg1);
  } else {
    ((CPatterned*)arg0)->CPatterned::PreRenderAllViewports(*(CStateManager*)arg1);
  }
}

extern int lbl_9_bss_38;
extern unsigned char lbl_9_data_AAC[152];
extern "C" void fn_9_F314(int, int);
extern "C" int fn_9_F290(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_9_data_AAC;
    lbl_9_bss_38 = lbl_9_bss_38 - 1;
    ((CRELFileToken*)(arg0 + 1028))->~CRELFileToken();
    fn_9_F314(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_9_996C(int arg0, int arg1, int arg2, float arg3) {
  int temp_r31;
  float temp_f31;
  int temp_r3;
  int temp_r3_2;
  float temp_f1;
  switch (arg2) {
  case 0:
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)1);
    temp_r3 = *(int*)(arg0 + 0x48c);
    ((CBodyController*)temp_r3)->SetTurnSpeed(*(float*)(temp_r3 + 0x590) / 2.0f);
    temp_r31 = *(int*)(arg0 + 0x48c);
    temp_f31 = ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)2);
    temp_f1 =
        ((CBodyStateInfo*)(temp_r31 + 876))->GetLocomotionSpeed((pas::ELocomotionAnim)1) / temp_f31;
    *(int*)((*(int*)(arg0 + 0x48c)) + 0x4c) = 1;
    ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->SetSteeringSpeedRange(temp_f1, temp_f1);
    break;
  case 2:
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)3);
    temp_r3_2 = *(int*)(arg0 + 0x48c);
    ((CBodyController*)temp_r3_2)->SetTurnSpeed(2.0f * *(float*)(temp_r3_2 + 0x590));
    *(int*)((*(int*)(arg0 + 0x48c)) + 0x4c) = 0;
    break;
  }
  ((CWaypointNavigation*)(arg0 + 1720))
      ->Patrol(*(CStateManager*)arg1, (EStateMsg)arg2, arg3, *(CPatterned*)arg0);
}

extern "C" void fn_9_8D28(int arg0, int arg1, int arg2) {
  unsigned short temp_r7;
  switch (arg2) {
  case 0:
    if (!((*(unsigned char*)(arg0 + 0xc63)) >> 5 & 1)) {
      temp_r7 = *(unsigned short*)(arg0 + 0x8);
      ushort temp_0 = temp_r7;
      ((CStateManager*)arg1)
          ->DeliverScriptMsg(CScriptMsg(TUniqueId(temp_0), TUniqueId(temp_0),
                                        (EScriptObjectMessage)0x44435456, kInvalidUniqueId,
                                        (EScriptObjectState)-1));
    }
    *(int*)(arg0 + 0x920) = -1;
    if (((*(unsigned char*)(arg0 + 0xc63)) >> 4 & 1)) {
      ((CStateManager*)arg1)
          ->DeleteObjectRequest(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x8)));
    }
    break;
  }
}

extern "C" int fn_9_D68(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 1304) {
      ((SLdrDamageVulnerability*)(arg0 + 1320))->~SLdrDamageVulnerability();
      ((SLdrDamageInfo*)(arg0 + 1304))->~SLdrDamageInfo();
    }
    if (arg0 + 1224) {
      ((SLdrDamageInfo*)(arg0 + 1244))->~SLdrDamageInfo();
    }
    ((SLdrDamageInfo*)(arg0 + 1200))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1180))->~SLdrDamageInfo();
    ((SLdrIngPossessionData*)(arg0 + 760))->~SLdrIngPossessionData();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_9_4A20();
extern "C" void fn_9_49F8(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_9_4A20();
  }
}

extern "C" void fn_9_61D8();
extern "C" void fn_9_61B0(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_9_61D8();
  }
}

extern "C" void fn_9_AA8();
extern "C" void fn_9_A80(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_9_AA8();
  }
}

extern "C" int fn_9_5F3C(int arg0, int arg1) {
  void fn_9_5F90(int, int);
  if (arg0) {
    fn_9_5F90(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_2378(int arg0, int arg1) {
  void fn_9_23D0(int, int);
  if (arg0) {
    fn_9_23D0(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_9_5F90(int arg0, int arg1) {
  void fn_9_5FEC();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_9_5FEC();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_9_E7F8 {
  virtual void _0(int);
};
extern "C" int fn_9_E7F8(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    temp_r3 = *(int*)arg0;
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_9_E7F8*)temp_r3)->_0(1);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_9_data_198;
extern int lbl_9_data_568;
struct __mwdec_vt_0_fn_9_BDE0 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4(void*, int);
};
struct __mwdec_vt_1 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3(void*, int);
};
extern "C" void fn_9_BDE0(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_9_BDE0*)temp_r31)->_4(&lbl_9_data_198, 34);
  ((__mwdec_vt_1*)temp_r31)->_3(&lbl_9_data_568, 36);
}
