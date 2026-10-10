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
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CTimeProvider.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CIkChain.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CPirateEchoEmitter.hpp"
#include "MetroidPrime/CRagDoll.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"
#include "MetroidPrime/Enemies/CBurstFire.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CMetroid.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CSpacePirate.hpp"
#include "MetroidPrime/Enemies/CTeamAiRole.hpp"
#include "MetroidPrime/Enemies/EListenNoiseType.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAiJumpPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTargetingPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTrigger.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_72_E314() {}

extern "C" bool fn_72_0() { return true; }

extern "C" bool fn_72_10() { return true; }

extern "C" bool fn_72_2EA0() { return false; }

extern "C" bool fn_72_40() { return false; }

extern "C" int fn_72_54(int arg0) { return arg0 + 1876; }

extern "C" bool fn_72_5C() { return true; }

extern "C" int fn_72_8(int arg0) { return arg0 + 2336; }

extern "C" bool fn_72_EDDC() { return false; }

extern "C" bool fn_72_EDE4() { return true; }

extern float lbl_72_rodata_B00;
extern "C" float fn_72_24() { return lbl_72_rodata_B00; }

extern "C" void fn_72_18(int arg0, int arg1) {
  *(unsigned short*)arg0 = *(unsigned short*)(arg1 + 0xa94);
}

extern "C" bool fn_72_48(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" bool fn_72_6328(int arg0) { return *(unsigned char*)(arg0 + 0x8f5) >> 6 & 1; }

extern "C" bool fn_72_7804(int arg0) { return *(unsigned char*)(arg0 + 0x8f4) >> 7 & 1; }

extern "C" bool fn_72_8A84(int arg0) { return *(unsigned char*)(arg0 + 0x8f9) >> 4 & 1; }

extern "C" bool fn_72_ADB4(int arg0) { return *(unsigned char*)(arg0 + 0x8f8) >> 4 & 1; }

extern float lbl_72_rodata_B54;
extern "C" void fn_72_BBA0(int arg0) { *(float*)(arg0 + 0x904) = lbl_72_rodata_B54; }

extern float lbl_72_rodata_B9C;
extern "C" void fn_72_BBB0(int arg0) { *(float*)(arg0 + 0x904) = lbl_72_rodata_B9C; }

extern "C" void fn_72_30(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" bool fn_72_52F8(int arg0, int arg1, int arg2) {
  return *(float*)(arg0 + 0xbb0) > *(float*)arg2;
}

extern "C" int fn_72_BC94(int arg0) {
  return (*(unsigned short*)(arg0 + 0x6c0) == kInvalidUniqueId.value) ? 1 : 0;
}

extern "C" void fn_72_E090(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void fn_72_D4();
extern "C" void RELMain() { fn_72_D4(); }

extern "C" void fn_72_1031C() {
  void fn_72_1033C();
  fn_72_1033C();
}

extern "C" CModelData fn_72_452C(int arg0) { return CModelData(); }

extern "C" void fn_72_4B88();
extern "C" void fn_72_4B68() { fn_72_4B88(); }

extern "C" void fn_72_4D0C() {
  void fn_72_4D2C();
  fn_72_4D2C();
}

extern "C" void fn_72_C468(int arg0, int arg1, int arg2) {
  ((CPatterned*)arg0)->Stuck(*(CStateManager*)arg1, *(const CTriggerData*)arg2);
}

extern "C" void fn_72_CE0() {
  void fn_72_D00();
  fn_72_D00();
}

extern "C" void RELExit() { SetSSpacePirate_FuncPtrs(nullptr); }

extern "C" void fn_72_AEE0(int, int, int);
extern "C" void fn_72_AEB8(int arg0, int arg1) { fn_72_AEE0(arg0, *(int*)(arg0 + 0x8), arg1); }

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
  virtual void _13();
  virtual void _14();
};
extern "C" void fn_72_3A88(int arg0) { ((__mwdec_vt_0*)arg0)->_14(); }

extern "C" void fn_72_10288();
extern "C" int fn_72_10258(int arg0) {
  fn_72_10288();
  return arg0;
}

extern "C" int fn_72_4FC4(int arg0) {
  void fn_72_4FF4();
  fn_72_4FF4();
  return arg0;
}

extern float lbl_72_rodata_BEC;
extern "C" bool fn_72_635C(int arg0) {
  if ((*(unsigned char*)(arg0 + 0x8f4) >> 2 & 1) && *(float*)(arg0 + 0xa90) < lbl_72_rodata_BEC) {
    return true;
  } else {
    return false;
  }
}

extern float lbl_72_rodata_BE8;
extern "C" int fn_72_686C(int arg0) {
  s32 var_r5 = false;
  if (!(*(unsigned char*)(arg0 + 0x8f4) >> 3 & 1) || *(float*)(arg0 + 0xbac) > lbl_72_rodata_BE8) {
    var_r5 = true;
  }
  return var_r5;
}

extern "C" bool fn_72_779C(int arg0) {
  if ((*(int*)(arg0 + 0x888)) == 1) {
    return *(float*)(arg0 + 0xa90) <= lbl_72_rodata_B9C;
  } else {
    return false;
  }
}

extern "C" int fn_72_77D0(int arg0) {
  s32 var_r4 = false;
  if ((*(int*)(arg0 + 0xc04)) == -1 && (*(unsigned char*)(arg0 + 0x8f6) >> 7 & 1) &&
      (*(unsigned char*)(arg0 + 0x8fa) >> 5 & 1)) {
    var_r4 = true;
  }
  return var_r4;
}

extern float lbl_72_rodata_B44;
extern "C" bool fn_72_7B6C(int arg0, int arg1, int arg2) {
  return *(float*)(arg0 + 0xb24) <
         ((*(float*)arg2) != lbl_72_rodata_B9C ? (*(float*)arg2) : lbl_72_rodata_B44);
}

extern "C" int fn_72_10620(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" bool fn_72_7BA0(int arg0, int arg1, int arg2) {
  return *(float*)(arg0 + 0xb28) <
         ((*(float*)arg2) != lbl_72_rodata_B9C ? (*(float*)arg2) : lbl_72_rodata_B44);
}

extern "C" int fn_72_1065C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_72_10698(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_72_2488(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_72_4E94(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_72_3AB4(int arg0) {
  if ((*(int*)(arg0 + 0xc04)) != -1) {
    &CDamageVulnerability::PassThroughVulnerabilty();
  } else {
    ((CPatterned*)arg0)->CPatterned::GetDamageVulnerability();
  }
}

extern "C" int fn_72_4ED0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_72_75A0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern float lbl_72_rodata_B60;
extern "C" bool fn_72_638C(int arg0) {
  if ((*(unsigned char*)(arg0 + 0x8f4) >> 2 & 1) && (unsigned int)*(int*)(arg0 + 0xab0) == 0 &&
      *(float*)(arg0 + 0xa90) > lbl_72_rodata_B60) {
    return true;
  } else {
    return false;
  }
}

extern "C" int fn_72_B9A4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_72_BB64(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_72_8A44(int arg0, int arg1, int arg2) {
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

extern "C" void fn_72_20A8(int, int);
extern "C" int fn_72_2060(int arg0, int arg1) {
  fn_72_20A8(*(int*)arg0, 1);
  *(int*)arg0 = arg1;
  return arg0;
}

extern "C" void fn_72_4C18(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_72_4BD0(int arg0) {
  void fn_72_4B0C(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_72_4C18(arg0, stack_24, stack_14, stack_8);
  fn_72_4B0C(stack_24, -1);
}

extern unsigned char lbl_72_data_B20[12];
extern "C" int fn_72_28A8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_B20;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_72_7630(int arg0, int arg1, int arg2) {
  if (!arg2) {
    if (((*(unsigned char*)(arg0 + 0x8f4)) >> 6 & 1)) {
      ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)6);
    } else {
      ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)0);
    }
  }
}

extern "C" int fn_72_45A4(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_72_75DC(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_72_D40C();
extern "C" void fn_72_9478(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    *(int*)((*(int*)(arg0 + 0x48c)) + 0x4c) = 0;
    *(float*)(arg0 + 0x904) = 1.0f;
    break;
  case 1:
    fn_72_D40C();
    break;
  case 2:
    break;
  }
}

extern "C" int fn_72_134C(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 20))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_72_F444(int arg0, int arg1) {
  if (arg0) {
    fn_72_20A8(*(int*)arg0, 1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_B14[12];
extern "C" int fn_72_284C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_B14;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_B08[12];
extern "C" int fn_72_54B4(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_B08;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AFC[12];
extern "C" int fn_72_587C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AFC;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AF0[12];
extern "C" int fn_72_5B2C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AF0;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AE4[12];
extern "C" int fn_72_6810(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AE4;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AC8[12];
extern "C" int fn_72_6CE8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AC8;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_ABC[12];
extern "C" int fn_72_9E08(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_ABC;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AA4[12];
extern "C" int fn_72_B540(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AA4;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_AB0[12];
extern "C" int fn_72_A6C0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_AB0;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_A8C[12];
extern "C" int fn_72_C628(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_A8C;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_72_data_A98[12];
extern "C" int fn_72_B948(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)lbl_72_data_A98;
    if (arg0) {
      *(int*)arg0 = (int)lbl_72_data_B20;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_72_F88(int arg0, int arg1) {
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

extern "C" int fn_72_F24(int arg0, int arg1) {
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

extern "C" int fn_72_10DDC(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg1 + 0x4);
  if (arg1 == (unsigned int)*(int*)(arg0 + 0x4)) {
    *(int*)(arg0 + 0x4) = temp_r31;
  }
  *(int*)((*(int*)arg1) + 0x4) = *(int*)(arg1 + 0x4);
  *(int*)*(int*)(arg1 + 0x4) = *(int*)arg1;
  CMemory::Free((const void*)arg1);
  *(int*)(arg0 + 0x14) = *(int*)(arg0 + 0x14) - 1;
  return temp_r31;
}

extern "C" void fn_72_5280(int arg0, float arg1) {
  if (1.0f != ((CBodyController*)*(int*)(arg0 + 0x48c))->GetPercentageFrozen() &&
      !(*(float*)((*(int*)(arg0 + 0x48c)) + 0x5b8) > 0.0f)) {
    *(float*)(arg0 + 0xbb0) = *(float*)(arg0 + 0xbb0) + arg1;
  }
}

extern "C" void fn_72_10954(int, int);
extern "C" int fn_72_10C64(int arg0, int arg1, int arg2, int arg3, int arg4, float arg5, float arg6,
                           float arg7, float arg8) {
  *(int*)arg0 = arg1;
  *(int*)(arg0 + 0x4) = arg2;
  fn_72_10954(arg0 + 8, arg3);
  *(int*)(arg0 + 0x58) = arg4;
  *(float*)(arg0 + 0x5c) = arg5;
  *(float*)(arg0 + 0x60) = arg6;
  *(float*)(arg0 + 0x64) = arg7;
  *(float*)(arg0 + 0x68) = arg8;
  return arg0;
}

extern "C" void fn_72_24C4(int arg0, int arg1, float arg2) {
  int temp_r3 = *(int*)(arg0 + 0xb30);
  if ((unsigned int)temp_r3 == 0 || !((*(unsigned char*)(temp_r3 + 0x70)) >> 6 & 1)) {
    ((CBoneTracking*)(arg0 + 2604))->PreThink(*(CAnimData*)*(int*)((*(int*)(arg0 + 0x60)) + 0x10));
  }
  ((CPatterned*)arg0)->CPatterned::PreThink(arg2, *(CStateManager*)arg1);
}

extern float lbl_72_rodata_B6C;
extern "C" void fn_72_3A0C(int arg0, int arg1) {
  int temp_r3 = *(int*)(arg0 + 0xb30);
  if ((unsigned int)temp_r3 != 0 && ((*(unsigned char*)(temp_r3 + 0x70)) >> 6 & 1)) {
    ((CRagDoll*)temp_r3)->PreRenderAllViewports(*(CActor*)arg0, lbl_72_rodata_B6C);
    ((CActor*)arg0)->UpdatePortalSystemState(*(CStateManager*)arg1);
  } else {
    ((CPatterned*)arg0)->CPatterned::PreRenderAllViewports(*(CStateManager*)arg1);
  }
}

extern "C" void fn_72_10364();
extern "C" void fn_72_1033C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_72_10364();
  }
}

extern "C" void fn_72_4D54();
extern "C" void fn_72_4D2C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_72_4D54();
  }
}

extern "C" void fn_72_D28();
extern "C" void fn_72_D00(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_72_D28();
  }
}

struct __mwdec_vt_0_fn_72_558C {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual void _48();
  virtual void _49();
  virtual void _50();
  virtual void _51();
  virtual void _52();
  virtual void _53();
  virtual void _54();
  virtual void _55();
  virtual void _56();
  virtual void _57();
  virtual void _58();
  virtual void _59();
  virtual void _60();
  virtual void _61();
  virtual void _62();
  virtual void _63();
  virtual void _64();
  virtual void _65();
  virtual void _66();
};
extern "C" void fn_72_558C(int arg0) { ((__mwdec_vt_0_fn_72_558C*)arg0)->_66(); }

struct __mwdec_vt_0_fn_72_64 {
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
extern "C" void fn_72_64(int arg0) { ((__mwdec_vt_0_fn_72_64*)arg0)->_12(); }

struct __mwdec_vt_0_fn_72_E2DC {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual bool _48();
};
extern "C" bool fn_72_E2DC(int arg0) {
  return (unsigned int)127 - ((__mwdec_vt_0_fn_72_E2DC*)arg0)->_48() >> 31;
}

extern "C" void fn_72_503C();
extern "C" void fn_72_4FF4(int arg0) {
  void fn_72_CE0();
  if (!(*(unsigned char*)(arg0 + 0x4c))) {
    fn_72_CE0();
    *(unsigned char*)(arg0 + 0x4c) = 1;
  } else {
    fn_72_503C();
  }
}

extern "C" int fn_72_4AB8(int arg0, int arg1) {
  void fn_72_4B0C(int, int);
  if (arg0) {
    fn_72_4B0C(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_72_454C(int arg0, int arg1) {
  void fn_72_45A4(int, int);
  if (arg0) {
    fn_72_45A4(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_72_55B8 {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual void _48();
  virtual void _49();
  virtual void _50();
  virtual void _51();
  virtual void _52();
  virtual void _53();
  virtual void _54();
  virtual void _55();
  virtual void _56();
  virtual void _57();
  virtual void _58();
  virtual void _59();
  virtual void _60();
  virtual void _61();
  virtual void _62();
  virtual void _63();
  virtual void _64();
  virtual void _65();
  virtual int _66(int);
};
extern "C" int fn_72_55B8(int arg0) {
  int temp_r4 = *(int*)(arg0 + 0x48c);
  int ret_0 = 0;
  if ((*(int*)(temp_r4 + 0x37c)) != 13 &&
      (unsigned char)((__mwdec_vt_0_fn_72_55B8*)arg0)->_66(temp_r4) == 0) {
    ret_0 = 1;
  }
  return ret_0;
}

extern "C" int fn_72_4B0C(int arg0, int arg1) {
  void fn_72_4B68();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_72_4B68();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_72_5B88 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5(int);
};
extern "C" void fn_72_5B88(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    ((__mwdec_vt_0_fn_72_5B88*)arg0)->_5(0);
    ((CStateManager*)arg1)
        ->DeleteObjectRequest(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x8)));
    break;
  }
}

extern unsigned char lbl_72_data_1D8[624];
extern unsigned char lbl_72_data_604[592];
struct __mwdec_vt_0_fn_72_F0A0 {
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
extern "C" void fn_72_F0A0(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_72_F0A0*)temp_r31)->_4(lbl_72_data_1D8, 39);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_72_data_604, 37);
}

extern "C" void fn_72_339C(int arg0, int arg1, int arg2, int arg3) {
  void fn_72_36CC(int, int, int, int);
  int temp_r3 = *(int*)((char*)arg0 + 0x60);
  bool var_r4 = (unsigned int)*(int*)((char*)temp_r3 + 0x10) == 0 &&
                !(*(unsigned char*)((char*)temp_r3 + 0x28));
  if (!var_r4) {
    ((CModelData*)temp_r3)
        ->Render((CModelData::EWhichModel)0, *(const CTransform4f*)arg2, nullptr,
                 *(const CModelFlags*)arg3);
  }
  fn_72_36CC(arg0, arg1, arg2, arg3);
}

extern "C" int fn_72_FE8(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 1304) {
      ((SLdrDamageInfo*)(arg0 + 1324))->~SLdrDamageInfo();
    }
    ((SLdrDamageInfo*)(arg0 + 1220))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1196))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1176))->~SLdrDamageInfo();
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

template < class T0 >
int TCastToPtr(CEntity&);
extern "C" void fn_72_E3D8(int arg0, int arg1, int arg2) {
  int temp_r3_2;
  int temp_r4;
  ((CPatterned*)arg0)->CPatterned::Touch(*(CActor*)arg1, *(CStateManager*)arg2);
  int temp_r3 = *(int*)((char*)arg0 + 0xb30);
  if ((unsigned int)temp_r3 != 0 && ((*(unsigned char*)((char*)temp_r3 + 0x70)) >> 6 & 1)) {
    temp_r3_2 = TCastToPtr< CScriptTrigger >(*(CEntity*)arg1);
    if ((unsigned int)temp_r3_2 != 0 && ((*(unsigned char*)((char*)temp_r3_2 + 0x20)) >> 7 & 1) &&
        (*(int*)((char*)temp_r3_2 + 0x1a0) & 0x8000) &&
        *(float*)((char*)temp_r3_2 + 0x19c) > 0.0f) {
      temp_r4 = *(int*)((char*)arg0 + 0xb30);
      *(float*)((char*)temp_r4 + 0xb8) =
          *(float*)((char*)temp_r4 + 0xb8) + *(float*)((char*)temp_r3_2 + 0x190);
      *(float*)((char*)temp_r4 + 0xbc) =
          *(float*)((char*)temp_r4 + 0xbc) + *(float*)((char*)temp_r3_2 + 0x194);
      *(float*)((char*)temp_r4 + 0xc0) =
          *(float*)((char*)temp_r4 + 0xc0) + *(float*)((char*)temp_r3_2 + 0x198);
    }
  }
}

extern "C" void fn_72_6DA4(int arg0, int arg1, int arg2) {
  unsigned short temp_r7;
  switch (arg2) {
  case 0:
    if (!((*(unsigned char*)((char*)arg0 + 0x8fa)) >> 5 & 1)) {
      temp_r7 = *(unsigned short*)((char*)arg0 + 0x8);
      ushort temp_0 = temp_r7;
      ((CStateManager*)arg1)
          ->DeliverScriptMsg(CScriptMsg(TUniqueId(temp_0), TUniqueId(temp_0),
                                        (EScriptObjectMessage)0x44435456, kInvalidUniqueId,
                                        (EScriptObjectState)-1));
    }
    *(int*)((char*)arg0 + 0xc04) = -1;
    if (((*(unsigned char*)((char*)arg0 + 0x8fa)) >> 4 & 1)) {
      ((CStateManager*)arg1)
          ->DeleteObjectRequest(TUniqueId((ushort) * (unsigned short*)((char*)arg0 + 0x8)));
    }
    break;
  }
}

extern int gpRender;
struct __mwdec_vt_0_fn_72_36CC {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual void _48();
  virtual void _49();
  virtual void _50();
  virtual void _51();
  virtual void _52();
  virtual void _53();
  virtual void _54();
  virtual void _55();
  virtual void _56();
  virtual void _57();
  virtual void _58();
  virtual void _59();
  virtual void _60();
  virtual void _61();
  virtual void _62();
  virtual void _63();
  virtual void _64();
  virtual void _65();
  virtual void _66();
  virtual void _67();
  virtual void _68();
  virtual void _69();
  virtual void _70();
  virtual void _71();
  virtual void _72(int);
};
struct __mwdec_vt_1_fn_72_36CC {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual int _48(int);
};
struct __mwdec_vt_2 {
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
  virtual void _45();
  virtual void _46();
  virtual void _47();
  virtual void _48();
  virtual void _49();
  virtual void _50();
  virtual void _51();
  virtual void _52();
  virtual void _53();
  virtual void _54();
  virtual void _55();
  virtual void _56();
  virtual void _57();
  virtual void _58();
  virtual void _59();
  virtual void _60();
  virtual void _61();
  virtual void _62();
  virtual void _63();
  virtual void _64();
  virtual void _65();
  virtual void _66();
  virtual void _67();
  virtual void _68();
  virtual void _69();
  virtual void _70();
  virtual void _71();
  virtual void _72();
  virtual void _73();
};
extern "C" void fn_72_36CC(int arg0, int arg1, int arg2, int arg3) {
  int temp_r31 = ((CActor*)arg0)->GetRenderAlphaBufferAlpha(*(const CStateManager*)arg1);
  if (temp_r31 != -1) {
    ((__mwdec_vt_0_fn_72_36CC*)gpRender)->_72(temp_r31);
  }
  if ((unsigned char)((__mwdec_vt_1_fn_72_36CC*)arg0)->_48(arg1) &&
      (*(unsigned char*)((char*)arg0 + 0xc54))) {
    CTransform4f stack_68(
        *((CTransform4f*)arg2) *
        ((CActor*)arg0)->GetScaledLocatorTransform(*(const CSegId*)(arg0 + 2696)));
    ((CModelData*)(arg0 + 3080))
        ->Render(*(const CStateManager*)arg1, stack_68,
                 (const CActorLights*)*(int*)((char*)arg0 + 0xbc), *(const CModelFlags*)arg3);
  }
  if (temp_r31 != -1) {
    ((__mwdec_vt_2*)gpRender)->_73();
  }
}

extern "C" void fn_72_DEDC(int, int);
extern "C" void fn_72_5C10(int arg0, int arg1, int arg2, float arg3) {
  void fn_72_E314(int, int, int);
  ((CPatterned*)arg0)->Dead(*(CStateManager*)arg1, (EStateMsg)arg2, arg3);
  switch (arg2) {
  case 0:
    ((CBoneTracking*)(arg0 + 2604))->SetActive(false);
    fn_72_E314(arg0, arg1, 0);
    fn_72_DEDC(arg0, arg1);
    break;
  case 1:
    if ((*(int*)((char*)*(int*)((char*)arg0 + 0x48c) + 0x37c)) == 4) {
      ((CActor*)arg0)
          ->RemoveMaterial((EMaterialTypes)40, (EMaterialTypes)41, *(CStateManager*)arg1);
      ((CActor*)arg0)
          ->RemoveMaterial((EMaterialTypes)37, (EMaterialTypes)59, (EMaterialTypes)48,
                           *(CStateManager*)arg1);
      ((CActor*)arg0)->AddMaterial((EMaterialTypes)20, *(CStateManager*)arg1);
      *(float*)((char*)arg0 + 0x1c0) = CVector3f::sZeroVector.GetX();
      *(float*)((char*)arg0 + 0x1c4) = CVector3f::sZeroVector.GetY();
      *(float*)((char*)arg0 + 0x1c8) = CVector3f::sZeroVector.GetZ();
      ((CPhysicsActor*)arg0)->Stop();
    }
    break;
  case 2:
    break;
  }
}
