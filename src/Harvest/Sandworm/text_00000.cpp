// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CJointData_LinearStorage.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Animation/CPASDatabase.hpp"
#include "Kyoto/Animation/CPoseAsTransforms_Linear.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
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
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBouncingBomb.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" bool fn_56_14A6C() { return false; }

extern "C" int fn_56_14A64(int arg0) { return arg0 + 1988; }

extern "C" bool fn_56_14AE4() { return false; }

extern "C" bool fn_56_14AEC() { return false; }

extern "C" int fn_56_14ADC(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_56_14B24() { return true; }

extern "C" int fn_56_14B1C(int arg0) { return arg0 + 1876; }

extern "C" bool fn_56_14B2C() { return false; }

extern "C" bool fn_56_14B34() { return false; }

extern "C" bool fn_56_30F0() { return false; }

extern "C" int fn_56_6790(int arg0) { return arg0 + 5416; }

extern "C" int fn_56_4AD0(int arg0) { return *(unsigned char*)(arg0 + 0x80); }

extern "C" int fn_56_9640(int arg0) { return *(unsigned short*)(arg0 + 0x1832); }

extern "C" int fn_56_9648(int arg0) { return *(unsigned short*)(arg0 + 0x1830); }

extern "C" bool fn_56_F710() { return true; }

extern "C" bool fn_56_14B04(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" float fn_56_14B10() { return kDefaultGravityAccel; }

extern "C" bool fn_56_2204(int arg0) { return *(unsigned char*)(arg0 + 0x2f4) >> 6 & 1; }

extern "C" bool fn_56_225C(int arg0) { return *(unsigned char*)(arg0 + 0x2f4) >> 7 & 1; }

extern "C" bool fn_56_35EC(int arg0) { return *(unsigned char*)(arg0 + 0x19b4) >> 7 & 1; }

extern "C" bool fn_56_3E6C(int arg0) { return *(unsigned char*)(arg0 + 0x8c1) >> 7 & 1; }

extern float lbl_56_rodata_148;
extern "C" float fn_56_9860() { return lbl_56_rodata_148; }

extern "C" bool fn_56_97F0(int arg0) { return *(unsigned char*)(arg0 + 0x1508) >> 7 & 1; }

extern "C" bool fn_56_BF38(int arg0) { return *(unsigned char*)(arg0 + 0x1604) >> 6 & 1; }

extern "C" void fn_56_A9F4(int arg0, int arg1, int arg2) {
  *(int*)(arg0 + 0x1518) = arg1;
  *(int*)(arg0 + 0x151c) = arg2;
}

extern "C" bool fn_56_C3E8(int arg0) { return *(unsigned char*)(arg0 + 0x8c0) >> 7 & 1; }

extern "C" void fn_56_14AF4(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern float lbl_56_rodata_150;
extern "C" void fn_56_3B9C(int arg0) { *(float*)(arg0 + 0x1984) = lbl_56_rodata_150; }

extern "C" int fn_56_9650() { return CSfxManager::kInternalInvalidSfxId; }

extern "C" void fn_56_11CE8(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" int fn_56_3750(int arg0) {
  if (*(int*)(0x1490 + arg0) == 7) {
    return 1;
  }
  return 0;
}

extern "C" int fn_56_3764(int arg0) { return (*(int*)(0x1490 + arg0) == 6) ? 1 : 0; }

extern "C" int fn_56_3778(int arg0) {
  if (*(int*)(arg0 + 0x1490) == 5) {
    return 1;
  }
  return 0;
}

extern "C" int fn_56_378C(int arg0) {
  if (*(int*)(arg0 + 0x1490) == 1) {
    return 1;
  }
  return 0;
}

extern "C" void fn_56_28F0(int arg0) {
  *(int*)arg0 = -1;
  *(int*)(arg0 + 0x4) = -1;
  *(int*)(arg0 + 0x8) = 0;
}

extern "C" void fn_56_14B3C(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void fn_56_2210(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x2e8);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x2ec);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x2f0);
}

extern "C" void fn_56_238C(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void fn_56_23A8(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x2e8);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x2ec);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x2f0);
}

extern "C" void fn_56_F368(int arg0) {
  *(float*)(arg0 + 0x1760) = *(float*)(arg0 + 0x54);
  *(float*)(arg0 + 0x1764) = *(float*)(arg0 + 0x58);
  *(float*)(arg0 + 0x1768) = *(float*)(arg0 + 0x5c);
}

extern "C" void fn_56_70();
extern "C" void RELMain() { fn_56_70(); }

extern "C" void fn_56_1088() {
  void fn_56_10A8();
  fn_56_10A8();
}

extern "C" CModelData fn_56_26A4(int arg0) { return CModelData(); }

extern "C" void fn_56_3D64();
extern "C" void fn_56_3D44() { fn_56_3D64(); }

extern float lbl_56_rodata_178;
extern "C" bool fn_56_4384(int arg0) { return *(float*)(arg0 + 0x176c) <= lbl_56_rodata_178; }

extern "C" void fn_56_8684();
extern "C" void fn_56_8664() { fn_56_8684(); }

extern "C" void fn_56_884C() {
  void fn_56_886C();
  fn_56_886C();
}

extern "C" void fn_56_97FC() {
  void fn_56_3A20();
  fn_56_3A20();
}

extern "C" void fn_56_A4F0(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRender(*(CStateManager*)arg1);
}

extern "C" void fn_56_D1EC() {
  void fn_56_D20C();
  fn_56_D20C();
}

extern "C" void fn_56_1080C(int, int);
extern "C" void fn_56_107E8(int arg0) { fn_56_1080C(arg0, 12); }

extern "C" int fn_56_5910(int arg0) {
  if (!(*(unsigned char*)(arg0 + 0x20) >> 7 & 1) || !(*(unsigned char*)(arg0 + 0x420) >> 6 & 1)) {
    return 0;
  } else {
    return 13;
  }
}

extern "C" void RELExit() { SetSSandworm_FuncPtrs(nullptr); }

extern "C" void fn_56_AA00(int arg0) {
  void fn_56_A9F4(int, int, int);
  fn_56_A9F4(arg0, -1, 13);
}

extern "C" void fn_56_F77C();
extern "C" void fn_56_F754(int arg0, int arg1, int arg2) {
  if (!arg2) {
    fn_56_F77C();
  }
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
extern "C" void fn_56_0(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" int fn_56_7390(int arg0, int arg1) {
  int temp_r3 = *(int*)(arg1 + 0x14fc);
  return ((((*(int*)(temp_r3 + 0x390)) == 0) ? *(int*)(temp_r3 + 0x38c) : 0) == 1) ? 1 : 0;
}

extern "C" float fn_56_10CA4(float arg0, float arg1, float arg2) {
  float temp_f1;
  float temp_f1_2;
  if (arg1 < arg0) {
    temp_f1 = arg0 - arg2;
    return rstl::max_val(temp_f1, arg1);
  } else {
    temp_f1_2 = arg0 + arg2;
    if (!(temp_f1_2 > arg1)) {
      return temp_f1_2;
    }
    return arg1;
  }
}

extern "C" int fn_56_12B64(int arg0) {
  void fn_56_12B94();
  fn_56_12B94();
  return arg0;
}

extern float lbl_56_rodata_168;
extern "C" bool fn_56_3E78(int arg0) {
  if ((unsigned int)(*(unsigned char*)(arg0 + 0x8c0) >> 4 & 1) == 1) {
    return false;
  } else {
    return *(float*)(arg0 + 0x13ac) > lbl_56_rodata_168;
  }
}

extern "C" int fn_56_26C4(int, int, int);
extern "C" int fn_56_3DE4(int arg0) {
  return ((unsigned char)fn_56_26C4(arg0 + 5264, 1, 3) == 0) ? 1 : 0;
}

extern "C" int fn_56_898C(int arg0) {
  void fn_56_884C();
  *(unsigned char*)(arg0 + 0x14) = 1;
  fn_56_884C();
  return arg0;
}

extern "C" void fn_56_DB6C(int arg0, int arg1, int arg2, int arg3) {
  ((CModelData*)arg2)
      ->Render(*(const CStateManager*)arg1, *(const CTransform4f*)arg3,
               (const CActorLights*)*(int*)(arg0 + 0xbc), *(const CModelFlags*)(arg0 + 252));
}

extern "C" int fn_56_10F54(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_1362C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13754(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_136A4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13CBC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13D50(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13EFC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13F38(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_13FD4(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_1894(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_70FC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_72BC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_8914(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_8950(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_56_EF50(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_56_1EDC(int);
extern "C" int fn_56_1E9C(int arg0) {
  *(unsigned char*)(arg0 + 0xc) = 0;
  *(unsigned char*)(arg0 + 0x1c) = 0;
  fn_56_1EDC(arg0 + 32);
  return arg0;
}

extern "C" void fn_56_F328(int arg0, int arg1, int arg2) {
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

extern int lbl_56_data_878;
extern "C" int fn_56_343C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_56_data_878;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_56_3EF4(int, int);
extern "C" void fn_56_3EAC(int arg0) {
  fn_56_3EF4(arg0, arg0 + 6212);
  fn_56_3EF4(arg0, arg0 + 6204);
  fn_56_3EF4(arg0, arg0 + 6208);
}

extern "C" void fn_56_A348(int);
extern "C" void fn_56_11078(int arg0) {
  if (!(*(unsigned char*)(arg0 + 0x8c0) >> 1 & 1)) {
    ((CAnimData*)*(int*)((*(int*)(arg0 + 0x60)) + 0x10))->BuildPoseIfNecessary();
    fn_56_A348(arg0);
  }
}

extern "C" void fn_56_AA28(int arg0, int arg1, int arg2) {
  int temp_r4;
  if ((unsigned int)(*(unsigned char*)(arg1 + 0x8c0) >> 6 & 1) == 1) {
    *(float*)arg0 = *(float*)(arg1 + 0x6d4);
    *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x6d8);
    *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x6dc);
  } else {
    temp_r4 = *(int*)(arg2 + 0x14fc);
    *(float*)arg0 = *(float*)(temp_r4 + 0x54);
    *(float*)(arg0 + 0x4) = *(float*)(temp_r4 + 0x58);
    *(float*)(arg0 + 0x8) = *(float*)(temp_r4 + 0x5c);
  }
}

extern float lbl_56_rodata_15C;
extern "C" bool fn_56_3E1C(int arg0) {
  if ((unsigned int)(*(unsigned char*)(arg0 + 0x8c0) >> 4 & 1) == 1) {
    return false;
  } else if (*(float*)(arg0 + 0x13ac) > lbl_56_rodata_148) {
    return true;
  } else {
    return *(float*)(arg0 + 0x1384) > lbl_56_rodata_15C;
  }
}

extern "C" int fn_56_52F8(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_85B4(int arg0, int arg1) {
  void fn_56_8608(int, int);
  if (arg0) {
    fn_56_8608(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_13EA4(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_13CF8(int arg0, int arg1) {
  if (arg0) {
    delete (CProjectedShadow*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_56_rodata_18C;
extern "C" float fn_56_6AA8(int, int, unsigned char*);
extern "C" bool fn_56_6A50(int arg0, int arg1) {
  float temp_f1;
  unsigned char stack_8[24];
  float temp_f2 = *(float*)(arg0 + 0x1574);
  temp_f1 = *(float*)(arg0 + 0x1564);
  int temp_r4 = *(int*)(arg1 + 0x14fc) + 84;
  *(float*)stack_8 = *(float*)(arg0 + 0x1554);
  *(float*)(stack_8 + 0x4) = temp_f1;
  *(float*)(stack_8 + 0x8) = temp_f2;
  return fn_56_6AA8(arg0, temp_r4, stack_8) < lbl_56_rodata_18C;
}

extern "C" bool fn_56_7E08(int arg0, int arg1) {
  unsigned short temp_r4;
  if ((unsigned int)arg1 == 0 || !(*(unsigned char*)(arg1 + 0x20) >> 7 & 1) ||
      (*(int*)(arg1 + 0x4)) != (*(int*)(arg0 + 0x4))) {
    return false;
  } else {
    temp_r4 = *(unsigned short*)(arg1 + 0x8);
    if (temp_r4 == (*(unsigned short*)(arg0 + 0x184a)) ||
        temp_r4 == (*(unsigned short*)(arg0 + 0x1848))) {
      return false;
    } else {
      return true;
    }
  }
}

extern int lbl_56_data_86C;
extern "C" int fn_56_3590(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_56_data_86C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_56_data_878;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_56_C828();
extern "C" void fn_56_CC8C(int, int);
extern "C" void fn_56_AC24(int arg0, int arg1) {
  if (!(*(unsigned char*)(arg0 + 0x8c0) >> 7 & 1)) {
    fn_56_C828();
  }
  if ((unsigned int)*(int*)(arg0 + 0x998) == 0) {
    fn_56_CC8C(arg0, arg1);
  }
}

extern int lbl_56_data_860;
extern "C" int fn_56_B4B0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_56_data_860;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_56_data_878;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_56_B85C(int arg0, int arg1, int arg2) {
  void fn_56_2770(int);
  if (!arg2) {
    fn_56_2770(arg0 + 5264);
    *(int*)(arg0 + 0x1494) = arg1;
  } else if (arg2 == 2) {
    *(int*)(arg0 + 0x1494) = -1;
  }
}

extern int lbl_56_data_854;
extern "C" int fn_56_B800(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_56_data_854;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_56_data_878;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_1330(int arg0, int arg1) {
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

extern "C" int fn_56_13C5C(int arg0, int arg1) {
  if (arg0) {
    if (*(unsigned char*)(arg0 + 0x8)) {
      ((CToken*)arg0)->~CToken();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_13BF8(int arg0, int arg1) {
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

extern "C" int fn_56_12CC(int arg0, int arg1) {
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

extern "C" void fn_56_C468(unsigned char*, int, int, int);
extern "C" void fn_56_C3F4(int arg0, int arg1, int arg2) {
  unsigned char stack_8[48];
  ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)7);
  if (!arg2) {
    fn_56_C468(stack_8, arg0, arg1, arg0 + 5740);
    ((CActor*)arg0)->SetTransform(*(const CTransform4f*)stack_8);
  }
}

extern int lbl_56_rodata_250;
extern "C" void fn_56_9C10(int arg0) {
  void* var_r31 = new ((char*)&lbl_56_rodata_250 + 0x45e, nullptr) CProjectedShadow(128, 128, 0, 1);
  delete (CProjectedShadow*)*(int*)(arg0 + 0x13bc);
  *(int*)(arg0 + 0x13bc) = (int)var_r31;
}

extern "C" void fn_56_10D0();
extern "C" void fn_56_10A8(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_56_10D0();
  }
}

extern "C" void fn_56_8894();
extern "C" void fn_56_886C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_56_8894();
  }
}

extern "C" void fn_56_D234();
extern "C" void fn_56_D20C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_56_D234();
  }
}

struct __mwdec_vt_0_fn_56_5E5C {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5(int);
};
extern "C" void fn_56_5E5C(int arg0, int arg1, int arg2) {
  if (!arg2) {
    ((__mwdec_vt_0_fn_56_5E5C*)arg0)->_5(0);
  }
}

extern "C" bool fn_56_981C(int arg0) {
  int fn_56_3A20();
  if ((unsigned int)(*(unsigned char*)(arg0 + 0x8c0) >> 4 & 1) == 1 &&
      (unsigned char)fn_56_3A20() == 0) {
    return true;
  }
  return false;
}

extern "C" void fn_56_12BDC();
extern "C" void fn_56_12B94(int arg0) {
  void fn_56_1088();
  if (!(*(unsigned char*)(arg0 + 0x4c))) {
    fn_56_1088();
    *(unsigned char*)(arg0 + 0x4c) = 1;
  } else {
    fn_56_12BDC();
  }
}

extern "C" bool fn_56_5530(int arg0) {
  int fn_56_3A20();
  if ((unsigned char)fn_56_3A20() == 0) {
    return false;
  }
  return *(unsigned char*)(arg0 + 0x968) != 0;
}

struct __mwdec_vt_0_fn_56_58C0 {
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
  virtual void _74();
  virtual void _75();
  virtual void _76();
  virtual void _77();
  virtual void _78();
  virtual void _79();
  virtual void _80();
  virtual void _81();
  virtual void _82();
  virtual void _83();
  virtual void _84();
  virtual void _85();
  virtual void _86();
  virtual void _87();
  virtual void _88();
  virtual void _89();
  virtual void _90();
  virtual void _91();
  virtual void _92();
  virtual void _93();
  virtual void _94();
  virtual void _95();
  virtual void _96();
  virtual void _97();
  virtual void _98();
  virtual void _99();
  virtual void _100();
  virtual void _101();
  virtual void _102();
  virtual void _103();
  virtual void _104();
  virtual void _105();
  virtual void _106();
  virtual bool _107();
};
extern "C" bool fn_56_58C0(int arg0) {
  return ((*(unsigned char*)(arg0 + 0x20) >> 7 & 1) == 0)
             ? false
             : ((__mwdec_vt_0_fn_56_58C0*)arg0)->_107() != 0;
}

extern "C" int fn_56_52A0(int arg0, int arg1) {
  void fn_56_52F8(int, int);
  if (arg0) {
    fn_56_52F8(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_8608(int arg0, int arg1) {
  void fn_56_8664();
  if (arg0) {
    if (*(unsigned char*)(arg0 + 0x14)) {
      fn_56_8664();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_56_13F74(int arg0, int arg1) {
  void fn_56_1894(int, int);
  if (arg0) {
    if (*(unsigned char*)(arg0 + 0x1c)) {
      fn_56_1894(arg0, -1);
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_56_rodata_184;
struct __mwdec_vt_0_fn_56_55F8 {
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
struct __mwdec_vt_1 {
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
extern "C" float fn_56_55F8(int arg0) {
  float temp_f31 = *(float*)((__mwdec_vt_0_fn_56_55F8*)arg0)->_13();
  float temp_f0 = *(float*)(((__mwdec_vt_1*)arg0)->_13() + 0x4);
  return lbl_56_rodata_184 * (temp_f0 / temp_f31);
}

struct __mwdec_vt_0_fn_56_1DD0 {
  virtual void _0(int);
};
extern "C" int fn_56_1DD0(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (*(unsigned char*)arg0) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_56_1DD0*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_56_27E8(int, int);
extern "C" void fn_56_2770(int arg0, int arg1) {
  int temp_r31 = arg0 + 12;
  while ((*(int*)(arg0 + 0x8)) >= 5) {
    fn_56_27E8(arg0 + 8, temp_r31);
  }
  int temp_r3 = temp_r31 + (*(int*)(arg0 + 0x8) << 2);
  if (temp_r3) {
    *(int*)temp_r3 = arg1;
  }
  *(int*)(arg0 + 0x8) = *(int*)(arg0 + 0x8) + 1;
}

extern "C" bool fn_56_3B24(int arg0, int arg1) {
  int fn_56_36A0(int, int);
  int fn_56_3A20();
  if ((unsigned char)fn_56_3A20() == 1 && *(float*)(arg0 + 0x1980) > lbl_56_rodata_150 &&
      !fn_56_36A0(arg0, arg1)) {
    return true;
  }
  return false;
}

struct __mwdec_vt_0_fn_56_22A0 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5(int);
};
extern "C" void fn_56_22A0(int arg0, int arg1, int arg2) {
  if ((unsigned char)arg2 == 1) {
    ((CActor*)arg0)->AddMaterial((EMaterialTypes)41, (EMaterialTypes)63, *(CStateManager*)arg1);
  } else {
    ((CActor*)arg0)->RemoveMaterial((EMaterialTypes)41, (EMaterialTypes)63, *(CStateManager*)arg1);
  }
  ((__mwdec_vt_0_fn_56_22A0*)arg0)->_5(arg2);
}

extern "C" int fn_56_E2C0(int arg0, int arg1) {
  int fn_56_2204();
  void fn_56_E19C(int, int);
  void fn_56_E1D0(int, int);
  int fn_56_E340();
  if ((unsigned char)fn_56_E340() == 1) {
    fn_56_E19C(arg0, arg1);
    return ((unsigned char)fn_56_2204() == 0) ? 1 : 0;
  }
  fn_56_E1D0(arg0, arg1);
  if ((unsigned char)fn_56_2204() == 0) {
    return 1;
  }
  return 0;
}

extern "C" bool fn_56_E340(int arg0) {
  int fn_56_36A0();
  int fn_56_3A20(int);
  if (fn_56_36A0() == 1) {
    if ((unsigned char)fn_56_3A20(arg0) == 0) {
      if (!(*(unsigned char*)(arg0 + 0x14d4))) {
        return true;
      }
    } else if (!(*(unsigned char*)(arg0 + 0x14d4)) &&
               (unsigned int)(*(unsigned char*)(arg0 + 0x19b4) >> 5 & 1) == 1) {
      return true;
    }
  }
  return false;
}

extern "C" int fn_56_12780(int arg0, int arg1) {
  if (arg0) {
    if (arg0 + 92 && (*(unsigned char*)((char*)arg0 + 0xa8))) {
      ((CModelData*)(arg0 + 92))->~CModelData();
    }
    if (arg0 + 12 && (*(unsigned char*)((char*)arg0 + 0x58))) {
      ((CModelData*)(arg0 + 12))->~CModelData();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_56_data_1C0[592];
extern unsigned char lbl_56_data_524[368];
extern unsigned char lbl_56_data_6A0[16];
struct __mwdec_vt_0_fn_56_D548 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4(unsigned char*, int);
};
struct __mwdec_vt_1_fn_56_D548 {
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
extern "C" void fn_56_D548(int arg0, int arg1) {
  ((CPatterned*)arg0)->InitializeStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_56_D548*)*(int*)((char*)arg0 + 0x350))->_4(lbl_56_data_1C0, 37);
  ((__mwdec_vt_1_fn_56_D548*)*(int*)((char*)arg0 + 0x350))->_3(lbl_56_data_524, 23);
  ((__mwdec_vt_2*)*(int*)((char*)arg0 + 0x350))->_5(lbl_56_data_6A0, 1);
}

struct __mwdec_vt_0_fn_56_3990 {
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
extern "C" int fn_56_3990(int arg0) {
  float temp_f1 = *(float*)((char*)((__mwdec_vt_0_fn_56_3990*)arg0)->_13() + 0x4);
  if (temp_f1 > *(float*)((char*)arg0 + 0x8c4)) {
    return arg0 + 2244;
  }
  if (temp_f1 > *(float*)((char*)arg0 + 0x8e4)) {
    return arg0 + 2276;
  }
  if (temp_f1 > *(float*)((char*)arg0 + 0x904)) {
    return arg0 + 2308;
  }
  if (temp_f1 > *(float*)((char*)arg0 + 0x924)) {
    return arg0 + 2340;
  }
  return arg0 + 2372;
}

extern "C" bool fn_56_AC80(int arg0) {
  int fn_56_36A0();
  if (fn_56_36A0() || ((unsigned int)*(unsigned char*)((char*)arg0 + 0x8c0) >> 4 & 1) == 1) {
    return false;
  } else {
    if ((*(int*)((char*)arg0 + 0x1494)) == 3) {
      return false;
    }
    if ((*(int*)((char*)arg0 + 0x1210)) < 13) {
      return false;
    }
  }
  return 5.0f + *(float*)((char*)arg0 + 0x134c) < *(float*)((char*)arg0 + 0x8b0);
}

extern "C" int fn_56_35F8(int arg0, int arg1) {
  unsigned char fn_56_2204();
  void fn_56_E19C(int, int);
  void fn_56_E1D0();
  int var_r31;
  if ((*(unsigned char*)((char*)arg0 + 0x14d4)) == 1 ||
      ((unsigned int)*(unsigned char*)((char*)arg0 + 0x8c0) >> 4 & 1) == 1) {
    return 0;
  } else {
    var_r31 = 0;
    fn_56_E1D0();
    if (fn_56_2204() == 1) {
      var_r31 = 1;
    }
    fn_56_E19C(arg0, arg1);
    if (fn_56_2204() == 1) {
      var_r31 = var_r31 + 1;
    }
  }
  return (var_r31 == 1) ? 1 : 0;
}

extern "C" int fn_56_3844(int, int);
extern "C" void fn_56_37A0(int arg0, int arg1, int arg2) {
  unsigned char fn_56_3A20();
  if (!arg2) {
    if (!fn_56_3A20()) {
      *(int*)((char*)arg0 + 0x1490) = 1;
    } else if ((*(int*)((char*)arg0 + 0x1490)) != 7) {
      *(int*)((char*)arg0 + 0x1490) = fn_56_3844(arg0, arg1);
      *(unsigned char*)((char*)arg0 + 0x968) = *(unsigned char*)((char*)arg0 + 0x968) + 1;
      if ((*(unsigned char*)((char*)arg0 + 0x968)) > 2 || fn_56_3844(arg0, arg1) == -1) {
        *(unsigned char*)((char*)arg0 + 0x968) = 0;
      }
    }
  }
}

extern "C" bool fn_56_36A0(int arg0, int arg1) {
  unsigned char fn_56_2204();
  unsigned char fn_56_3A20();
  int fn_56_E19C(int, int);
  int fn_56_E1D0(int, int);
  if (fn_56_3A20() == 1) {
    return false;
  }
  if ((unsigned int)fn_56_E1D0(arg0, arg1) != 0 && (unsigned int)fn_56_E19C(arg0, arg1) != 0) {
    fn_56_E1D0(arg0, arg1);
    if (!fn_56_2204()) {
      fn_56_E19C(arg0, arg1);
      if (!fn_56_2204()) {
        return false;
      }
    }
  }
  return true;
}

extern "C" void fn_56_10BE8(int arg0, float arg1) {
  float fn_56_10CA4(float, float, float);
  float temp_f1;
  int var_r30 = arg0;
  int var_r29 = 0;
  float temp_f31 = 3.0f * arg1;
  do {
    temp_f1 = *(float*)((char*)var_r30 + 0x12e8);
    if (var_r29 > (*(int*)((char*)arg0 + 0x1518)) && var_r29 < (*(int*)((char*)arg0 + 0x151c))) {
      *(float*)((char*)var_r30 + 0x12e8) =
          fn_56_10CA4(temp_f1, *(float*)((char*)arg0 + 0x1524), temp_f31);
    } else {
      *(float*)((char*)var_r30 + 0x12e8) = fn_56_10CA4(temp_f1, lbl_56_rodata_150, temp_f31);
    }
    var_r29 = var_r29 + 1;
    var_r30 = var_r30 + 4;
  } while (var_r29 < 13);
}

extern int gpRender;
struct __mwdec_vt_0_fn_56_4A14 {
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
  virtual int _18();
};
struct __mwdec_vt_1_fn_56_4A14 {
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
  virtual void _15(int);
};
struct __mwdec_vt_2_fn_56_4A14 {
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
  virtual int _18();
};
struct __mwdec_vt_3 {
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
  virtual void _15(int);
};
extern "C" void fn_56_4A14(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
  int temp_r3 = *(int*)((char*)arg0 + 0x19e4);
  if ((unsigned int)temp_r3 != 0 &&
      (unsigned char)((__mwdec_vt_0_fn_56_4A14*)temp_r3)->_18() == 1) {
    ((__mwdec_vt_1_fn_56_4A14*)gpRender)->_15(*(int*)((char*)arg0 + 0x19e4));
  }
  int temp_r3_2 = *(int*)((char*)arg0 + 0x19ec);
  if ((unsigned int)temp_r3_2 != 0 &&
      (unsigned char)((__mwdec_vt_2_fn_56_4A14*)temp_r3_2)->_18() == 1) {
    ((__mwdec_vt_3*)gpRender)->_15(*(int*)((char*)arg0 + 0x19ec));
  }
}

extern "C" int fn_56_1390(int arg0, int arg1) {
  void fn_56_1894(int, int);
  if (arg0) {
    ((SLdrIngPossessionData*)(arg0 + 1112))->~SLdrIngPossessionData();
    fn_56_1894(arg0 + 1084, -1);
    fn_56_1894(arg0 + 1056, -1);
    fn_56_1894(arg0 + 1028, -1);
    fn_56_1894(arg0 + 1000, -1);
    fn_56_1894(arg0 + 972, -1);
    ((SLdrDamageInfo*)(arg0 + 936))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 876))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 860))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 820))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 644))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 64))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}
