// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
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
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/IObj.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CPlane.hpp"
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
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CElitePirateGrenadeLauncher.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CWaypointNavigation.hpp"
#include "MetroidPrime/Enemies/SPositionHistory.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTeamAiMgr.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" int fn_15_0(int arg0) { return arg0 + 2640; }

extern "C" bool fn_15_1A0C() { return false; }

extern "C" int fn_15_52C8(int arg0) { return arg0 + 3280; }

extern "C" int fn_15_50B4(int arg0) { return *(unsigned char*)(arg0 + 0xd08); }

extern "C" int fn_15_5C(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_15_64() { return false; }

extern "C" bool fn_15_6C() { return false; }

extern "C" int fn_15_8(int arg0) { return arg0 + 2496; }

extern "C" int fn_15_9C(int arg0) { return arg0 + 1876; }

extern "C" bool fn_15_A4() { return true; }

extern "C" bool fn_15_AC() { return false; }

extern "C" bool fn_15_B4() { return false; }

extern "C" bool fn_15_18AC(int arg0) { return *(unsigned char*)(arg0 + 0xd24) >> 7 & 1; }

extern "C" bool fn_15_18CC(int arg0) { return *(unsigned char*)(arg0 + 0xd24) >> 6 & 1; }

extern "C" bool fn_15_60BC(int arg0) { return *(unsigned char*)(arg0 + 0xc10) >> 2 & 1; }

extern "C" bool fn_15_60C8(int arg0) { return *(unsigned char*)(arg0 + 0xc10) >> 3 & 1; }

extern "C" bool fn_15_61C0(int arg0) { return *(unsigned char*)(arg0 + 0xc44) >> 7 & 1; }

extern "C" bool fn_15_63B0(int arg0) { return *(unsigned char*)(arg0 + 0xc10) >> 6 & 1; }

extern "C" float fn_15_90() { return kDefaultGravityAccel; }

extern "C" bool fn_15_84(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" void fn_15_1B50(int arg0, int arg1) {
  *(unsigned char*)(arg0 + 0xc10) = *(unsigned char*)(arg0 + 0xc10) & 0xffffffbf | arg1 << 6 & 64;
}

extern "C" void fn_15_74(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" bool fn_15_18B8(int arg0) { return *(int*)(arg0 + 0xd20) > 0; }

extern "C" int fn_15_2CE8(int arg0) {
  if (*(int*)((*(int*)(arg0 + 0x48c)) + 0x580) == 0) {
    return 1;
  }
  return 0;
}

extern "C" int fn_15_4E7C(int arg0) { return (*(int*)(0xd00 + arg0) == 1) ? 1 : 0; }

extern "C" void fn_15_4C(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" bool fn_15_61CC(int arg0) { return *(int*)(arg0 + 0xc40) != 1; }

extern "C" void fn_15_BC(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_15_148();
  fn_15_148();
}

extern float lbl_15_rodata_114;
extern "C" void fn_15_27DC(int arg0, float arg1) {
  float temp_f2 = *(float*)(arg0 + 0xa34);
  if (!(temp_f2 > lbl_15_rodata_114)) {
    return;
  }
  *(float*)(arg0 + 0xa34) = temp_f2 - arg1;
}

extern "C" void fn_15_32C0() {
  void fn_15_32E0();
  fn_15_32E0();
}

extern "C" int fn_15_6F28(int arg0) {
  if (*(unsigned short*)(arg0 + 0x6c0) == kInvalidUniqueId.value) {
    return 1;
  }
  return 0;
}

extern "C" void fn_15_6C64(int arg0, int arg1, int arg2) {
  ((CPatterned*)arg0)->AnimOver(*(CStateManager*)arg1, *(const CTriggerData*)arg2);
}

extern "C" void fn_15_78A8() {
  void fn_15_78C8();
  fn_15_78C8();
}

extern "C" void fn_15_7A64();
extern "C" void fn_15_7A44() { fn_15_7A64(); }

extern "C" void fn_15_84EC(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
}

extern "C" void fn_15_CE8() {
  void fn_15_D08();
  fn_15_D08();
}

extern bool lbl_15_rodata_198;
extern "C" int fn_15_4488(int arg0) {
  return ((*(int*)(arg0 + 0xc6c)) == 0) ? (int)((&lbl_15_rodata_198) + 0x3a9)
                                        : (int)((&lbl_15_rodata_198) + 0x3b9);
}

extern "C" void RELExit() { SetSElitePirate_FuncPtrs(nullptr); }

extern "C" int fn_15_2FF0();
extern "C" int fn_15_2FC8() { return (fn_15_2FF0() == 0) ? 1 : 0; }

extern "C" int fn_15_15F8();
extern "C" int fn_15_15CC() { return (fn_15_15F8() == 27) ? 1 : 0; }

extern "C" void fn_15_2448(int, int, int, long long*);
extern "C" void fn_15_24F0(int arg0, int arg1, int arg2) {
  long long stack_8;
  *(unsigned short*)&stack_8 = *(unsigned short*)(arg0 + 0x9fa);
  fn_15_2448(arg0, arg1, arg2, &stack_8);
}

extern "C" void fn_15_6618(int, int, long long*);
extern "C" void fn_15_6820(int arg0, int arg1) {
  long long stack_8;
  *(unsigned short*)&stack_8 = *(unsigned short*)(arg0 + 0x9fa);
  fn_15_6618(arg0, arg1, &stack_8);
}

extern "C" bool fn_15_6D68(int arg0) {
  float var_f0;
  if ((*(int*)(arg0 + 0xc6c)) == 1) {
    var_f0 = *(float*)(arg0 + 0x8ec);
  } else {
    var_f0 = *(float*)(arg0 + 0x8f0);
  }
  return (*(float*)(arg0 + 0xc60)) > var_f0;
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
extern "C" void fn_15_D8(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern void* lbl_15_bss_0;
extern "C" void fn_15_178();
extern "C" void fn_15_148() {
  lbl_15_bss_0 = &fn_15_178;
  SetSElitePirate_FuncPtrs((SElitePirate_FuncPtrs*)&(*(int*)&lbl_15_bss_0));
}

extern "C" bool fn_15_5084() { return fn_15_15F8() != 23; }

extern "C" int fn_15_1A14(int arg0, int arg1) {
  return (((CPathFindSearch*)(arg0 + 2640))
              ->OnPath(*(const CVector3f*)(*(int*)(arg1 + 0x14fc) + 84)) == 0)
             ? 1
             : 0;
}

extern "C" void fn_15_1A84(int, int);
extern "C" void fn_15_1A48(int arg0, int arg1) {
  if ((*(int*)(arg1 + 0xcb0)) == 5) {
    fn_15_1A84(arg0, arg1 + 2364);
  } else {
    fn_15_1A84(arg0, arg1 + 2316);
  }
}

extern "C" void fn_15_555C(int arg0, int arg1) {
  *(unsigned short*)(arg0 + 0xa26) =
      CScriptTeamAiMgr::ChoosePlayer(*(const CStateManager*)arg1, *(const CActor*)arg0).value;
}

extern "C" int fn_15_7970(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_79AC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_955C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_9A74(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_9C34(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_B264(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_B2A0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_15_B2DC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern int lbl_15_data_954;
extern "C" int fn_15_4A98(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_954;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_15_rodata_10C;
extern float lbl_15_rodata_E8;
extern "C" bool fn_15_6D94(int arg0) {
  float temp_f3 = *(float*)(arg0 + 0xc5c);
  float temp_f4 = *(float*)(arg0 + 0xa3c);
  if (*(float*)(arg0 + 0xa48) > lbl_15_rodata_10C * (temp_f3 + temp_f4)) {
    return true;
  } else {
    return lbl_15_rodata_E8 + temp_f3 < temp_f4;
  }
}

extern "C" void fn_15_77B4(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_15_776C(int arg0) {
  void fn_15_79E8(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_15_77B4(arg0, stack_24, stack_14, stack_8);
  fn_15_79E8(stack_24, -1);
}

extern "C" void fn_15_AC00();
extern "C" int fn_15_ABB8(int arg0, int arg1) {
  fn_15_AC00();
  *(int*)(arg0 + 0x8) = arg1;
  ((CToken*)arg0)->Lock();
  return arg0;
}

extern int lbl_15_data_8F0;
extern "C" int fn_15_AE68(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_8F0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_15_47DC(int, int);
extern "C" void fn_15_4790(int arg0, int arg1, int arg2) {
  if (arg2 == 2) {
    fn_15_47DC(arg0 + 3092, *(int*)(arg0 + 0xc2c));
    *(int*)(arg0 + 0xc2c) = 0;
  } else {
    *(int*)(arg0 + 0xc2c) = arg1;
  }
}

extern int lbl_15_data_8D0;
extern "C" int fn_15_B318(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_8D0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_2788(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_2B64(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_15_6064(int arg0, int arg1, int arg2) {
  void fn_15_850C();
  fn_15_850C();
  if (!arg2) {
    ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->ClearLocomotionCmds();
    ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)1);
  }
}

extern int lbl_15_data_948;
extern "C" int fn_15_4A3C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_948;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_15_data_954;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_15_data_93C;
extern "C" int fn_15_4CBC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_93C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_15_data_954;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_B6F0(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_15_data_930;
extern "C" int fn_15_526C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_930;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_15_data_954;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_15_rodata_DC;
extern "C" bool fn_15_61E4(int arg0) {
  int temp_r3 = fn_15_15F8();
  if (temp_r3 == 23 || temp_r3 == 27) {
    return true;
  }
  return *(float*)(arg0 + 0xa40) > lbl_15_rodata_DC;
}

extern int lbl_15_data_924;
extern "C" int fn_15_5450(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_924;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_15_data_954;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_15_data_8FC;
extern "C" int fn_15_AE0C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_15_data_8FC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_15_data_8F0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_B7CC(int arg0, int arg1) {
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

extern "C" int fn_15_F90(int arg0, int arg1) {
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

extern "C" int fn_15_B0E4(int arg0, int arg1) {
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

extern "C" int fn_15_C44(int arg0, int arg1) {
  if (arg0) {
    ((SLdrShockWaveInfo*)(arg0 + 276))->~SLdrShockWaveInfo();
    ((SLdrShockWaveInfo*)(arg0 + 228))->~SLdrShockWaveInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_F2C(int arg0, int arg1) {
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

extern "C" void fn_15_3308();
extern "C" void fn_15_32E0(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_15_3308();
  }
}

extern "C" void fn_15_78F0();
extern "C" void fn_15_78C8(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_15_78F0();
  }
}

extern "C" void fn_15_D30();
extern "C" void fn_15_D08(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_15_D30();
  }
}

extern "C" int fn_15_7AAC(int arg0, int arg1) {
  void fn_15_79E8(int, int);
  if (arg0) {
    fn_15_79E8(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_2730(int arg0, int arg1) {
  void fn_15_2788(int, int);
  if (arg0) {
    fn_15_2788(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_15_79E8(int arg0, int arg1) {
  void fn_15_7A44();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_15_7A44();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_15_8E34 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4();
  virtual void _5();
  virtual void _6(int);
};
struct __mwdec_vt_1 {
  virtual void _0();
  virtual void _1(float);
};
extern "C" void fn_15_8E34(int arg0, float arg1) {
  int temp_r3 = *(int*)(arg0 + 0xc94);
  if ((unsigned int)temp_r3 != 0) {
    ((__mwdec_vt_0_fn_15_8E34*)temp_r3)->_6(arg0 + 2040);
    ((__mwdec_vt_1*)*(int*)(arg0 + 0xc94))->_1(arg1);
  }
}

struct __mwdec_vt_0_fn_15_52D0 {
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
  virtual void _107();
  virtual void _108();
  virtual void _109();
  virtual void _110();
  virtual void _111();
  virtual void _112(int, int, float);
};
extern "C" void fn_15_52D0(int arg0, int arg1, int arg2, float arg3) {
  void fn_15_850C();
  fn_15_850C();
  ((__mwdec_vt_0_fn_15_52D0*)arg0)->_112(arg1, arg2, arg3);
}

extern "C" void fn_15_5FF0(int arg0, int arg1, int arg2) {
  void fn_15_4790(int, int);
  void fn_15_850C(int, int, int);
  fn_15_4790(arg0, 5);
  fn_15_850C(arg0, arg1, arg2);
  ((CBodyController*)*(int*)(arg0 + 0x48c))
      ->SetLocomotionType((pas::ELocomotionType) * (int*)(arg0 + 0xc40));
  ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)1, false);
}

struct __mwdec_vt_0_fn_15_4410 {
  virtual void _0(int);
};
extern "C" int fn_15_4410(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (*(unsigned char*)arg0) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_15_4410*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_15_AD04 {
  virtual void _0(int);
};
extern "C" int fn_15_AD04(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (*(unsigned char*)arg0) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_15_AD04*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern unsigned char lbl_15_data_178[496];
extern unsigned char lbl_15_data_464[336];
extern unsigned char lbl_15_data_5CC[32];
struct __mwdec_vt_0_fn_15_9840 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4(unsigned char*, int);
};
struct __mwdec_vt_1_fn_15_9840 {
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
extern "C" void fn_15_9840(int arg0, int arg1) {
  ((CPatterned*)arg0)->InitializeStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_15_9840*)*(int*)(arg0 + 0x350))->_4(lbl_15_data_178, 31);
  ((__mwdec_vt_1_fn_15_9840*)*(int*)(arg0 + 0x350))->_3(lbl_15_data_464, 21);
  ((__mwdec_vt_2*)*(int*)(arg0 + 0x350))->_5(lbl_15_data_5CC, 2);
}

extern "C" void fn_15_8D84(int arg0, float arg1) {
  int fn_15_2CE8();
  if ((unsigned char)fn_15_2CE8() == 1) {
    *(float*)(arg0 + 0xd0c) = *(float*)(arg0 + 0xd0c) + arg1 / 0.8f;
    if (*(float*)(arg0 + 0xd0c) > 1.0f) {
      *(float*)(arg0 + 0xd0c) = 1.0f;
    }
  } else {
    *(float*)(arg0 + 0xd0c) = *(float*)(arg0 + 0xd0c) - arg1 / 0.4f;
    if (*(float*)(arg0 + 0xd0c) < 0.0f) {
      *(float*)(arg0 + 0xd0c) = 0.0f;
    }
  }
}

extern "C" void fn_15_54AC(int arg0, int arg1) {
  if (((*(unsigned char*)(arg1 + 0x16e8)) >> 7 & 1)) {
    if (fn_15_15F8() == 23) {
      *(int*)(arg0 + 0xd00) = 1;
    } else if (((CRandom16*)(arg1 + 5860))->Range(lbl_15_rodata_114, 1.0f) < 0.8f) {
      if (!(*(int*)(arg0 + 0xd04))) {
        *(int*)(arg0 + 0xd00) = 1;
      } else {
        *(int*)(arg0 + 0xd00) = 0;
      }
    } else {
      *(int*)(arg0 + 0xd00) = *(int*)(arg0 + 0xd04);
    }
  }
}

extern "C" void fn_15_BDFC(int arg0, int arg1, int arg2, int arg3) {
  switch (arg1) {
  case 0:
    *(int*)(arg0 + 0x6b4) = 1;
    ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->DeliverCmd(*(const CBCLocomotionCmd*)arg3);
    break;
  case 1:
    if (((CAnimationState*)(arg0 + 1716))
            ->CanIssueCommand(*(const CBodyController*)*(int*)(arg0 + 0x48c),
                              (pas::EAnimationState)arg2)) {
      ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->DeliverCmd(*(const CBCLocomotionCmd*)arg3);
    }
    break;
  case 2:
    *(int*)(arg0 + 0x6b4) = 0;
    break;
  }
}

extern "C" int fn_15_FF0(int arg0, int arg1) {
  if (arg0) {
    ((SLdrIngPossessionData*)(arg0 + 1160))->~SLdrIngPossessionData();
    ((SLdrAnimationSet*)(arg0 + 1140))->~SLdrAnimationSet();
    ((SLdrAnimationSet*)(arg0 + 1128))->~SLdrAnimationSet();
    ((SLdrActorParameters*)(arg0 + 1008))->~SLdrActorParameters();
    ((SLdrDamageInfo*)(arg0 + 960))->~SLdrDamageInfo();
    ((SLdrShockWaveInfo*)(arg0 + 892))->~SLdrShockWaveInfo();
    ((SLdrShockWaveInfo*)(arg0 + 844))->~SLdrShockWaveInfo();
    ((SLdrDamageInfo*)(arg0 + 760))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_15_5F08 {
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
  virtual void _107();
  virtual void _108();
  virtual void _109();
  virtual void _110();
  virtual void _111();
  virtual void _112();
  virtual void _113();
  virtual void _114();
  virtual void _115();
  virtual void _116();
  virtual void _117();
  virtual void _118();
  virtual void _119();
  virtual void _120();
  virtual void _121();
  virtual void _122();
  virtual void _123();
  virtual void _124();
  virtual void _125();
  virtual void _126();
  virtual void _127();
  virtual void _128();
  virtual void _129();
  virtual void _130();
  virtual void _131();
  virtual void _132();
  virtual void _133();
  virtual void _134();
  virtual void _135();
  virtual void _136(int, int);
};
extern "C" void fn_15_5F08(int arg0, int arg1, int arg2) {
  void fn_15_4790(int, int);
  void fn_15_850C(int, int, int);
  fn_15_4790(arg0, 6);
  fn_15_850C(arg0, arg1, arg2);
  ((CBodyController*)*(int*)(arg0 + 0x48c))->SetLocomotionType((pas::ELocomotionType)1);
  if (!arg2) {
    *(float*)(arg0 + 0x81c) = *(float*)(arg0 + 0x54);
    *(float*)(arg0 + 0x820) = *(float*)(arg0 + 0x58);
    *(float*)(arg0 + 0x824) = *(float*)(arg0 + 0x5c);
    ((CEntity*)arg0)
        ->SendScriptMsgs((EScriptObjectState)0x4154544b, *(CStateManager*)arg1, kInvalidUniqueId,
                         (EScriptObjectMessage)-1);
  } else if (arg2 == 2) {
    ((CKnockBackMgr*)(arg0 + 1568))->EnableAnimReaction((CKnockBackMgr::EAnimReaction)1, true);
    ((__mwdec_vt_0_fn_15_5F08*)arg0)->_136(arg1, 1);
  }
}
