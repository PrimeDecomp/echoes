// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharLayoutInfo.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Animation/CSkinRules.hpp"
#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/IObj.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
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
#include "MetroidPrime/CBoneTracking.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CStaticInterference.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIWaypoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptSafeZone.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_55_D304() {}

extern "C" bool fn_55_0() { return true; }

extern "C" bool fn_55_64() { return false; }

extern "C" int fn_55_5C(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_55_74() { return false; }

extern "C" bool fn_55_6C() { return false; }

extern "C" bool fn_55_8() { return true; }

extern "C" int fn_55_8E10(int arg0) { return *(int*)(arg0 + 0x7c4); }

extern "C" int fn_55_A4(int arg0) { return arg0 + 1876; }

extern "C" bool fn_55_AC() { return true; }

extern "C" bool fn_55_B4() { return false; }

extern "C" int fn_55_CDF0(int arg0) { return arg0 + 3584; }

extern "C" bool fn_55_8C(int arg0) { return *(unsigned char*)(arg0 + 0x34c) >> 3 & 1; }

extern "C" void fn_55_8E18(int arg0) { *(int*)(arg0 + 0xdf4) = 6; }

extern "C" void fn_55_8E24(int arg0) { *(int*)(arg0 + 0xdf4) = 5; }

extern "C" void fn_55_8E30(int arg0) { *(int*)(arg0 + 0xdf4) = 4; }

extern "C" void fn_55_8E3C(int arg0) { *(int*)(arg0 + 0xdf4) = 3; }

extern "C" void fn_55_8E48(int arg0) { *(int*)(arg0 + 0xdf4) = 2; }

extern "C" void fn_55_8E54(int arg0) { *(int*)(arg0 + 0xdf4) = 1; }

extern "C" void fn_55_8F88(int arg0) { *(float*)(arg0 + 0xf2c) = *(float*)(arg0 + 0x818); }

extern "C" void fn_55_91F4(int arg0) { *(int*)(arg0 + 0xdf0) = 2; }

extern "C" float fn_55_98() { return kDefaultGravityAccel; }

extern "C" void fn_55_9200(int arg0) { *(int*)(arg0 + 0xdf0) = 1; }

extern "C" void fn_55_A164(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" bool fn_55_C148(int arg0) { return *(unsigned char*)(arg0 + 0x165d) >> 6 & 1; }

extern "C" bool fn_55_CA10(int arg0) { return *(unsigned char*)(arg0 + 0x165c) >> 6 & 1; }

extern "C" bool fn_55_CA84(int arg0) { return *(unsigned char*)(arg0 + 0x165c) >> 7 & 1; }

extern "C" void fn_55_7C(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" void fn_55_4C(int arg0) { *(float*)(arg0 + 0x448) = CPatterned::skDamageHitTime; }

extern "C" int fn_55_C174(int arg0) { return ((*(int*)(0xde4 + arg0)) == 2) ? 1 : 0; }

extern "C" int fn_55_CA90(int arg0) {
  if (*(int*)(arg0 + 0x6b4) == 3) {
    return 1;
  }
  return 0;
}

extern "C" void fn_55_BC(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void RELMain() {
  void fn_55_148();
  fn_55_148();
}

extern "C" void fn_55_10604() {
  void fn_55_10624();
  fn_55_10624();
}

extern "C" void fn_55_2FFC() {
  void fn_55_301C();
  fn_55_301C();
}

extern "C" void fn_55_48DC();
extern "C" void fn_55_48BC() { fn_55_48DC(); }

extern "C" void fn_55_4A60() {
  void fn_55_4A80();
  fn_55_4A80();
}

extern "C" void fn_55_4F4() {
  void fn_55_514();
  fn_55_514();
}

extern "C" void fn_55_75BC() {
  void fn_55_75DC();
  fn_55_75DC();
}

extern float lbl_55_rodata_B8;
extern "C" bool fn_55_C154(int arg0) { return *(float*)(arg0 + 0xf2c) <= lbl_55_rodata_B8; }

extern "C" bool fn_55_C730(int arg0) {
  return *(unsigned short*)(arg0 + 0xe88) != kInvalidUniqueId.value;
}

extern "C" bool fn_55_C9B8(int arg0) { return *(float*)(arg0 + 0xf24) <= lbl_55_rodata_B8; }

extern "C" void fn_55_CF6C() { &CDamageVulnerability::PassThroughVulnerabilty(); }

extern "C" void fn_55_F894() {
  void fn_55_F8B4();
  fn_55_F8B4();
}

extern "C" void fn_55_10760(int, int);
extern "C" void fn_55_1073C(int arg0) { fn_55_10760(arg0, 0); }

extern "C" int fn_55_67D0();
extern "C" int fn_55_67A8() { return (fn_55_67D0() == 0) ? 1 : 0; }

extern "C" void RELExit() { SetSSandBoss_FuncPtrs(nullptr); }

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
extern "C" void fn_55_D8(int arg0) { ((__mwdec_vt_0*)arg0)->_12(); }

extern "C" void fn_55_117C(int arg0) {
  *(int*)arg0 = -1;
  *(int*)(arg0 + 0x4) = -1;
  *(int*)(arg0 + 0x8) = -1;
  *(int*)(arg0 + 0xc) = -1;
  *(int*)(arg0 + 0x10) = -1;
  *(int*)(arg0 + 0x14) = -1;
  *(int*)(arg0 + 0x18) = -1;
  *(int*)(arg0 + 0x1c) = -1;
  *(int*)(arg0 + 0x20) = -1;
}

extern "C" int fn_55_6E28(int arg0) {
  void fn_55_6E58();
  fn_55_6E58();
  return arg0;
}

extern "C" int fn_55_8C88(int arg0) {
  int var_r4 = false;
  if (!(*(unsigned char*)(arg0 + 0x165c) >> 3 & 1) &&
      (*(int*)((*(int*)(arg0 + 0x48c)) + 0x37c)) == 5) {
    var_r4 = true;
  }
  return var_r4;
}

extern void* lbl_55_bss_4;
extern "C" void fn_55_178();
extern "C" void fn_55_148() {
  lbl_55_bss_4 = &fn_55_178;
  SetSSandBoss_FuncPtrs((SSandBoss_FuncPtrs*)&(*(int*)&lbl_55_bss_4));
}

extern "C" int fn_55_F7F0(int arg0, int arg1) {
  void fn_55_F828(int);
  *(int*)arg0 = arg1;
  fn_55_F828(arg0 + 4);
  return arg0;
}

extern "C" int fn_55_10588(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_1064C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_1140(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_2510(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_4BE8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_4C24(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_5E20(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_D1CC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_F8DC(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_55_FB60(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

struct CScannableObjectInfo;
struct CScannableObjectInfo {};
extern "C" int fn_55_CDB0(int arg0) {
  int temp_r4;
  if (!(*(unsigned char*)(arg0 + 0x165c) >> 4 & 1)) {
    temp_r4 = *(int*)(arg0 + 0xdec);
    if ((unsigned int)temp_r4 != 0) {
      return *(int*)(temp_r4 + 0x8);
    }
  }
  return (int)((CPatterned*)arg0)->CPatterned::GetScannableObjectInfo();
}

extern int lbl_55_data_9A0;
extern "C" int fn_55_32DC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_9A0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_55_496C(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_55_4924(int arg0) {
  void fn_55_4860(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_55_496C(arg0, stack_24, stack_14, stack_8);
  fn_55_4860(stack_24, -1);
}

extern "C" void fn_55_F268();
extern "C" int fn_55_F220(int arg0, int arg1) {
  fn_55_F268();
  *(int*)(arg0 + 0x8) = arg1;
  ((CToken*)arg0)->Lock();
  return arg0;
}

extern int lbl_55_data_924;
extern "C" int fn_55_F4D0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_924;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_595C();
extern "C" int fn_55_C258(int arg0) {
  s32 var_r31 = false;
  if ((*(unsigned char*)(arg0 + 0x165d) >> 7 & 1) && fn_55_595C() == 1) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_55_10BC4(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_4E40(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_5568(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_16C0(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)arg0)->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_E11C(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_F09C(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_19D0(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 28))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_FB9C(int arg0, int arg1) {
  if (arg0) {
    delete (CCollisionActorManager*)*(int*)arg0;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_994;
extern "C" int fn_55_7FAC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_994;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_988;
extern "C" int fn_55_81FC(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_988;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_97C;
extern "C" int fn_55_9B5C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_97C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_970;
extern "C" int fn_55_A304(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_970;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_964;
extern "C" int fn_55_A670(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_964;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_958;
extern "C" int fn_55_AE7C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_958;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_9A0;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_930;
extern "C" int fn_55_F474(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_930;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_55_data_924;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_55_data_9AC;
extern "C" void fn_55_108E4(int, int);
extern "C" int fn_55_10C18(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_55_data_9AC;
    fn_55_108E4(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_79C(int arg0, int arg1) {
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

extern "C" int fn_55_8C20(int arg0, int arg1) {
  void fn_55_6E58();
  if (arg0 == (unsigned int)arg1) {
    return arg0;
  }
  if (*(unsigned char*)(arg1 + 0x4c)) {
    fn_55_6E58();
  } else {
    if (*(unsigned char*)(arg0 + 0x4c)) {
      ((CModelData*)arg0)->~CModelData();
    }
    *(unsigned char*)(arg0 + 0x4c) = 0;
  }
  return arg0;
}

extern "C" int fn_55_738(int arg0, int arg1) {
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

extern "C" int fn_55_4D78(int arg0, int arg1) {
  void fn_55_4DE8(int, int);
  if (arg0) {
    fn_55_4DE8(arg0 + 160, -1);
    fn_55_4DE8(arg0 + 92, -1);
    fn_55_4DE8(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_55_7BB0();
extern "C" void fn_55_86D0(int arg0, float arg1) {
  *(float*)(arg0 + 0xf1c) = *(float*)(arg0 + 0xf1c) - arg1;
  *(float*)(arg0 + 0xf18) = *(float*)(arg0 + 0xf18) - arg1;
  fn_55_7BB0();
  *(float*)(arg0 + 0xf20) = *(float*)(arg0 + 0xf20) + arg1;
  *(float*)(arg0 + 0xf34) = *(float*)(arg0 + 0xf34) + arg1;
}

extern "C" int fn_55_7FC(int arg0, int arg1) {
  void fn_55_D08(int, int);
  if (arg0) {
    fn_55_D08(arg0 + 760, -1);
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_55_D308(int arg0, int arg1) {
  unsigned int stack_c;
  unsigned int stack_8;
  if (!(*(unsigned char*)(arg0 + 0x165d) >> 2 & 1)) {
    *(int*)&stack_c = 0;
    *(int*)&stack_8 = 0;
    if ((*(unsigned char*)(arg0 + 0x421) & 1)) {
      ((CStateManager*)arg1)->GetCharacterRenderMaskAndTarget(stack_c, stack_8);
    }
    ((CPatterned*)arg0)
        ->CPatterned::RenderSystemsToBeDrawnLast(*(const CStateManager*)arg1, *(int*)&stack_c,
                                                 *(int*)&stack_8);
  }
}

extern "C" void fn_55_106E0();
extern "C" void fn_55_10624(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_106E0();
  }
}

extern "C" void fn_55_3044();
extern "C" void fn_55_301C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_3044();
  }
}

extern "C" void fn_55_4AA8();
extern "C" void fn_55_4A80(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_4AA8();
  }
}

extern "C" void fn_55_53C();
extern "C" void fn_55_514(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_53C();
  }
}

extern "C" void fn_55_7604();
extern "C" void fn_55_75DC(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_7604();
  }
}

extern "C" void fn_55_4B4();
extern "C" void fn_55_F8B4(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_55_4B4();
  }
}

extern "C" void fn_55_5728();
extern "C" void fn_55_6E58(int arg0) {
  void fn_55_4F4();
  if (!(*(unsigned char*)(arg0 + 0x4c))) {
    fn_55_4F4();
    *(unsigned char*)(arg0 + 0x4c) = 1;
  } else {
    fn_55_5728();
  }
}

extern "C" int fn_55_480C(int arg0, int arg1) {
  void fn_55_4860(int, int);
  if (arg0) {
    fn_55_4860(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_CA1C(int arg0) {
  int fn_55_8E10();
  s32 var_r31 = false;
  int temp_r3 = fn_55_8E10();
  int temp_r0 = *(int*)(arg0 + 0xde4);
  if (temp_r0 >= temp_r3 && temp_r0 < 3) {
    var_r31 = true;
  }
  return var_r31;
}

extern "C" int fn_55_4DE8(int arg0, int arg1) {
  void fn_55_4E40(int, int);
  if (arg0) {
    fn_55_4E40(arg0 + 8, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_5510(int arg0, int arg1) {
  void fn_55_5568(int, int);
  if (arg0) {
    fn_55_5568(arg0 + 4, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_55_4860(int arg0, int arg1) {
  void fn_55_48BC();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_55_48BC();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_55_F828(int arg0, int arg1, int arg2) {
  void fn_55_F894(int, int);
  int var_r31 = arg0;
  int var_r30 = 0;
  while (var_r30 < arg1) {
    fn_55_F894(var_r31, arg2);
    var_r30 = var_r30 + 1;
    var_r31 = var_r31 + 80;
  }
}

extern "C" int fn_55_1290(int arg0, int arg1) {
  void fn_55_16C0(int, int);
  if (arg0) {
    ((SLdrPlasmaBeamInfo*)(arg0 + 144))->~SLdrPlasmaBeamInfo();
    fn_55_16C0(arg0 + 72, -1);
    fn_55_16C0(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

struct __mwdec_vt_0_fn_55_F36C {
  virtual void _0(int);
};
extern "C" int fn_55_F36C(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if ((*(unsigned char*)arg0)) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_55_F36C*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_55_rodata_C0;
struct __mwdec_vt_0_fn_55_BD30 {
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
extern "C" void fn_55_BD30(int arg0, int arg1, int arg2) {
  float temp_f1;
  switch (arg2) {
  case 1:
    temp_f1 = ((__mwdec_vt_0_fn_55_BD30*)*(int*)(arg0 + 0x350))->_10();
    if (temp_f1 > lbl_55_rodata_C0) {
      ((CStateManager*)arg1)
          ->DeleteObjectRequest(TUniqueId((ushort) * (unsigned short*)(arg0 + 0x8)));
    }
    break;
  }
}

extern int lbl_80418FEC;
extern "C" void fn_55_6698(int, int);
extern "C" void fn_55_974C(int arg0, int arg1) {
  void fn_55_783C(int, int, int, int);
  *(int*)((char*)arg0 + 0x1458) = 1;
  *(int*)((char*)arg0 + 0x145c) = 1;
  *(int*)((char*)arg0 + 0x1460) = 1;
  *(int*)((char*)arg0 + 0x1464) = 1;
  *(int*)((char*)arg0 + 0x1468) = 1;
  *(int*)((char*)arg0 + 0x146c) = 1;
  *(int*)((char*)arg0 + 0x1470) = 1;
  *(int*)((char*)arg0 + 0x1474) = 1;
  fn_55_783C(arg0, arg1, arg0 + 5532, arg0 + 5532);
  fn_55_6698(arg0, arg1);
  *(int*)((char*)arg0 + 0x450) = lbl_80418FEC;
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, true);
}

extern unsigned char lbl_55_data_100[336];
extern unsigned char lbl_55_data_340[320];
extern unsigned char lbl_55_data_5AC[400];
struct __mwdec_vt_0_fn_55_CAA4 {
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
extern "C" void fn_55_CAA4(int arg0, int arg1) {
  int temp_r31 = *(int*)((char*)arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_55_CAA4*)temp_r31)->_4(lbl_55_data_100, 21);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_55_data_340, 20);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_55_data_5AC, 25);
}

extern "C" void fn_55_D208(int arg0, int arg1) {
  void fn_55_8A8C(int, int, int, int, unsigned char*);
  unsigned char stack_8[16];
  ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
  if (!((*(unsigned char*)((char*)arg0 + 0x165c)) >> 7 & 1)) {
    ((CPatterned*)arg0)->CPatterned::Render(*(const CStateManager*)arg1);
    *(unsigned char*)((char*)stack_8 + 0x4) = 2;
    *(unsigned char*)((char*)stack_8 + 0x5) = 0;
    *(unsigned short*)((char*)stack_8 + 0x6) = 3;
    *(int*)((char*)stack_8 + 0x8) = *(int*)((char*)arg0 + 0xf0c);
    fn_55_8A8C(arg0, arg1, arg0 + 36, arg0 + 252, stack_8);
  }
}

extern "C" void fn_55_97DC(int arg0, int arg1) {
  void fn_55_783C(int, int, int, int);
  *(int*)((char*)arg0 + 0x1458) = 2;
  *(int*)((char*)arg0 + 0x145c) = 2;
  *(int*)((char*)arg0 + 0x1460) = 2;
  *(int*)((char*)arg0 + 0x1464) = 2;
  *(int*)((char*)arg0 + 0x1468) = 2;
  *(int*)((char*)arg0 + 0x146c) = 2;
  *(int*)((char*)arg0 + 0x1470) = 2;
  *(int*)((char*)arg0 + 0x1474) = 2;
  fn_55_783C(arg0, arg1, arg0 + 5628, arg0 + 5628);
  fn_55_6698(arg0, arg1);
  *(int*)((char*)arg0 + 0x450) = lbl_80418FEC;
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, true);
}

extern "C" void fn_55_986C(int arg0, int arg1) {
  void fn_55_783C(int, int, int, int);
  *(int*)((char*)arg0 + 0x1458) = 2;
  *(int*)((char*)arg0 + 0x145c) = 2;
  *(int*)((char*)arg0 + 0x1460) = 2;
  *(int*)((char*)arg0 + 0x1464) = 2;
  *(int*)((char*)arg0 + 0x1468) = 2;
  *(int*)((char*)arg0 + 0x146c) = 2;
  *(int*)((char*)arg0 + 0x1470) = 2;
  *(int*)((char*)arg0 + 0x1474) = 2;
  fn_55_783C(arg0, arg1, arg0 + 5676, arg0 + 5676);
  fn_55_6698(arg0, arg1);
  *(int*)((char*)arg0 + 0x450) = lbl_80418FEC;
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, true);
}

extern "C" void fn_55_6250(int arg0, int arg1) {
  CTransform4f stack_74(((CPatterned*)arg0)->GetLctrTransform(*(const CSegId*)(arg0 + 3840)));
  ((CActor*)arg1)->SetTranslation(CVector3f(stack_74.Get03(), stack_74.Get13(), stack_74.Get23()));
  ((CPhysicsActor*)arg1)->Stop();
  &(*((CTransform4f*)(arg0 + 5280)) = ((CTransform4f*)(arg1 + 36))->GetRotation());
}

extern "C" void fn_55_96B0(int arg0, int arg1) {
  void fn_55_783C(int, int, int, const CDamageVulnerability*);
  *(int*)((char*)arg0 + 0x1458) = 2;
  *(int*)((char*)arg0 + 0x145c) = 2;
  *(int*)((char*)arg0 + 0x1460) = 2;
  *(int*)((char*)arg0 + 0x1464) = 2;
  *(int*)((char*)arg0 + 0x1468) = 2;
  *(int*)((char*)arg0 + 0x146c) = 2;
  *(int*)((char*)arg0 + 0x1470) = 2;
  *(int*)((char*)arg0 + 0x1474) = 2;
  fn_55_783C(arg0, arg1, arg0 + 5484, &CDamageVulnerability::ReflectVulnerabilty());
  fn_55_6698(arg0, arg1);
  *(int*)((char*)arg0 + 0x450) = lbl_80418FEC;
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, true);
}

extern "C" void fn_55_2750(int, int, float);
extern "C" void fn_55_84A8(int, int);
extern "C" void fn_55_D620(int arg0, int arg1, float arg2) {
  void fn_55_86D0(int, int, float);
  if (((*(unsigned char*)((char*)arg0 + 0x20)) >> 7 & 1)) {
    ((CPatterned*)arg0)->CPatterned::Think(arg2, *(CStateManager*)arg1);
    ((CCollisionActorManager*)*(int*)((char*)arg0 + 0xd98))
        ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
    ((CBoneTracking*)(arg0 + 3484))->Think(arg2);
    fn_55_86D0(arg0, arg1, arg2);
    fn_55_2750(arg0, arg1, arg2);
    fn_55_84A8(arg0, arg1);
  }
}

extern "C" int fn_55_F754(int arg0, int arg1) {
  int var_r31;
  int var_r30;
  if (arg0) {
    var_r30 = arg0 + 4;
    for (var_r31 = 0; var_r31 < (*(int*)arg0); var_r31 = var_r31 + 1) {
      if ((unsigned int)var_r30 != 0 && (*(unsigned char*)((char*)var_r30 + 0x4c))) {
        ((CModelData*)var_r30)->~CModelData();
      }
      var_r30 = var_r30 + 80;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_55_89E8(int arg0, int arg1, int arg2, int arg3) {
  void fn_55_8A8C(int, int, int, int, int);
  int temp_r3 = *(int*)((char*)arg0 + 0x60);
  bool var_r4 = (unsigned int)*(int*)((char*)temp_r3 + 0x10) == 0 &&
                !(*(unsigned char*)((char*)temp_r3 + 0x28));
  if (!var_r4) {
    ((CModelData*)temp_r3)
        ->Render((CModelData::EWhichModel)0, *(const CTransform4f*)arg2, nullptr,
                 *(const CModelFlags*)arg3);
  }
  fn_55_8A8C(arg0, arg1, arg2, arg3, arg3);
}

extern "C" void fn_55_4F40(int, int);
extern "C" int fn_55_4E94(int arg0, int arg1) {
  *(int*)arg0 = *(int*)arg1;
  *(float*)((char*)arg0 + 0x4) = *(float*)((char*)arg1 + 0x4);
  *(float*)((char*)arg0 + 0x8) = *(float*)((char*)arg1 + 0x8);
  *(float*)((char*)arg0 + 0xc) = *(float*)((char*)arg1 + 0xc);
  *(float*)((char*)arg0 + 0x10) = *(float*)((char*)arg1 + 0x10);
  *(float*)((char*)arg0 + 0x14) = *(float*)((char*)arg1 + 0x14);
  fn_55_4F40(arg0 + 24, arg1 + 24);
  fn_55_4F40(arg0 + 92, arg1 + 92);
  fn_55_4F40(arg0 + 160, arg1 + 160);
  *(int*)((char*)arg0 + 0xe4) = *(int*)((char*)arg1 + 0xe4);
  *(float*)((char*)arg0 + 0xe8) = *(float*)((char*)arg1 + 0xe8);
  *(float*)((char*)arg0 + 0xec) = *(float*)((char*)arg1 + 0xec);
  *(float*)((char*)arg0 + 0xf0) = *(float*)((char*)arg1 + 0xf0);
  return arg0;
}

extern "C" void fn_55_9600(int arg0, int arg1) {
  void fn_55_783C(int, int, int, const CDamageVulnerability*);
  *(int*)((char*)arg0 + 0x1458) = 0;
  *(int*)((char*)arg0 + 0x145c) = 2;
  *(int*)((char*)arg0 + 0x1460) = 2;
  *(int*)((char*)arg0 + 0x1464) = 2;
  *(int*)((char*)arg0 + 0x1468) = 2;
  *(int*)((char*)arg0 + 0x146c) = 2;
  *(int*)((char*)arg0 + 0x1470) = 2;
  *(int*)((char*)arg0 + 0x1474) = 2;
  fn_55_783C(arg0, arg1, arg0 + 5580, &CDamageVulnerability::ReflectVulnerabilty());
  fn_55_6698(arg0, arg1);
  *(int*)((char*)arg0 + 0x450) = CPatterned::skDamageColor.GetColor_u32();
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, false);
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)2, true);
}

extern "C" int fn_55_D08(int arg0, int arg1) {
  void fn_55_1140(int, int);
  void fn_55_1290(int, int);
  void fn_55_19D0(int, int);
  if (arg0) {
    ((SLdrDamageVulnerability*)(arg0 + 1148))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 800))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(arg0 + 452))->~SLdrDamageVulnerability();
    fn_55_1140(arg0 + 408, -1);
    fn_55_1140(arg0 + 372, -1);
    fn_55_1290(arg0 + 156, -1);
    fn_55_19D0(arg0 + 108, -1);
    ((SLdrDamageInfo*)(arg0 + 64))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 36))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 20))->~SLdrDamageInfo();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" bool fn_55_5B38(int arg0, int arg1) {
  float temp_f3;
  float temp_f4_3;
  CTransform4f stack_58(((CPatterned*)arg0)->GetLctrTransform(*(const CSegId*)(arg0 + 3840)));
  float temp_f4 = *(float*)((char*)arg0 + 0x820);
  float temp_f2 = *(float*)((char*)arg1 + 0x58) - stack_58.Get13();
  float temp_f1 = *(float*)((char*)arg1 + 0x54) - stack_58.Get03();
  float temp_f4_2 = *(float*)((char*)arg1 + 0x5c) - stack_58.Get23();
  if (temp_f4_2 * temp_f4_2 + (temp_f1 * temp_f1 + temp_f2 * temp_f2) < temp_f4 * temp_f4) {
    temp_f4_3 = *(float*)((char*)arg0 + 0x48);
    temp_f3 = *(float*)((char*)arg0 + 0x38);
    CVector3f stack_1c = CVector3f(*(float*)((char*)arg0 + 0x28), temp_f3, temp_f4_3);
    CVector2f stack_8(temp_f1, temp_f2);
    return CVector3f::GetAngleDiff(stack_1c, CVector3f(stack_8.GetX(), stack_8.GetY(), 0.0f)) <=
           0.2617994f;
  }
  return false;
}

extern "C" void fn_55_1418(int, int);
extern "C" void fn_55_11A8(int arg0, int arg1) {
  int temp_r3;
  int temp_r4;
  int temp_r4_2;
  unsigned short temp_r5_2;
  int var_r29 = 0;
  int temp_r5 = *(int*)((char*)arg1 + 0x8);
  *(int*)((char*)arg1 + 0x8) = temp_r5 + 2;
  unsigned short temp_r30 = *(unsigned short*)temp_r5;
  while (var_r29 < temp_r30) {
    temp_r4 = *(int*)((char*)arg1 + 0x8);
    *(int*)((char*)arg1 + 0x8) = temp_r4 + 4;
    temp_r3 = *(int*)((char*)arg1 + 0x8);
    temp_r4_2 = *(int*)temp_r4;
    *(int*)((char*)arg1 + 0x8) = temp_r3 + 2;
    temp_r5_2 = *(unsigned short*)temp_r3;
    switch (temp_r4_2) {
    case -1183297778:
      fn_55_1418(arg0, arg1);
      break;
    case -1196549156:
      fn_55_1418(arg0 + 72, arg1);
      break;
    case -1797626827:
      LoadTypedefPlasmaBeamInfo(*(SLdrPlasmaBeamInfo*)(arg0 + 144), *(CInputStream*)arg1);
      break;
    default:
      ((CInputStream*)arg1)->ReadBytes(nullptr, temp_r5_2);
      break;
    }
    var_r29 = var_r29 + 1;
  }
}
