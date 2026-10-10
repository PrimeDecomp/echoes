// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Animation/CCharAnimTime.hpp"
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
#include "MetroidPrime/CEchoEmitter.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CLineOfSightTracker.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CParticleDatabase.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/Cameras/CCameraShakerData.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Player/CStaticInterference.hpp"
#include "MetroidPrime/SEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageVulnerability.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEchoParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPlasmaBeamInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrShockWaveInfo.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraShaker.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDebris.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlayerHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakBall.hpp"
#include "MetroidPrime/Weapons/CBeamInfo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CShockWave.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CProjectileWeapon.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_14_17180() {}

extern "C" void fn_14_13B30(int arg0, int arg1) { *(int*)(arg0 + 0xfe8) = arg1; }

extern "C" int fn_14_1ABD8(int arg0) { return arg0 + 1892; }

extern "C" bool fn_14_DF6C() { return true; }

extern "C" bool fn_14_DF74() { return true; }

extern "C" void fn_14_13A74(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" void fn_14_14248(int arg0) { *(float*)(arg0 + 0x10e0) = *(float*)(arg0 + 0x7d8); }

extern "C" bool fn_14_171EC(int arg0) { return *(unsigned char*)(arg0 + 0x10f4) >> 1 & 1; }

extern "C" bool fn_14_1728C(int arg0) { return *(unsigned char*)(arg0 + 0x10f4) >> 4 & 1; }

extern "C" bool fn_14_173AC(int arg0) { return *(unsigned char*)(arg0 + 0x10f4) >> 7 & 1; }

extern "C" void fn_14_1AB24(int arg0) { *(unsigned char*)(arg0 + 0xc) = 0; }

extern "C" void fn_14_1560C(int arg0, int arg1) {
  *(unsigned short*)(arg0 + 0x1036) = *(unsigned short*)((*(int*)(arg1 + 0x14fc)) + 0x8);
}

extern "C" int fn_14_1734C(int arg0) { return (*(int*)(arg0 + 0x10d0) == 0) ? 1 : 0; }

extern "C" int fn_14_17338(int arg0) { return (*(int*)(0x10d0 + arg0) == 1) ? 1 : 0; }

extern "C" int fn_14_1735C(int arg0) { return (*(int*)(arg0 + 0x10d0) == 2) ? 1 : 0; }

extern "C" int fn_14_17370(int arg0) { return (*((int*)(arg0 + 0x10d0)) == 3) ? 1 : 0; }

extern "C" int fn_14_17398(int arg0) {
  if (*(int*)(arg0 + 0xfe8) == 4) {
    return 1;
  }
  return 0;
}

extern "C" int fn_14_17384(int arg0) {
  if (*(int*)(arg0 + 0xfe8) == 3) {
    return 1;
  }
  return 0;
}

extern "C" int fn_14_17454(int arg0) { return (*(int*)(arg0 + 0x6b4) == 3) ? 1 : 0; }

extern "C" float fn_14_F374(int arg0) {
  if ((*(int*)(arg0 + 0x10e4)) < 2) {
    return *(float*)(arg0 + 0x7e8);
  } else {
    return *(float*)(arg0 + 0x7e4);
  }
}

extern "C" void fn_14_105B8() {
  void fn_14_105D8();
  fn_14_105D8();
}

extern "C" void fn_14_106FC();
extern "C" void fn_14_106DC() { fn_14_106FC(); }

extern "C" bool fn_14_17198(int arg0) {
  return *(unsigned short*)(arg0 + 0x103a) != kInvalidUniqueId.value;
}

extern float lbl_14_rodata_960;
extern "C" bool fn_14_171B8(int arg0) { return *(float*)(arg0 + 0x10e0) <= lbl_14_rodata_960; }

extern "C" void fn_14_179E0() { &CDamageVulnerability::PassThroughVulnerabilty(); }

extern "C" void fn_14_17A44(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::PreRender(*(CStateManager*)arg1);
}

extern "C" void fn_14_11044(int arg0) {
  void fn_14_11068(int);
  fn_14_11068(arg0 + 40);
}

extern "C" void fn_14_13AE0(int arg0, float arg1) {
  *(float*)(arg0 + 0x10d4) = *(float*)(arg0 + 0x10d4) - arg1;
  *(float*)(arg0 + 0x10ec) = *(float*)(arg0 + 0x10ec) + arg1;
  *(float*)(arg0 + 0x10f0) = *(float*)(arg0 + 0x10f0) + arg1;
}

extern "C" void fn_14_1524C(int arg0, int arg1) {
  ((CLineOfSightTracker*)(arg0 + 3456))
      ->SetTarget(TUniqueId((ushort) * (unsigned short*)((*(int*)(arg1 + 0x14fc)) + 0x8)));
}

extern "C" bool fn_14_173CC(int arg0) {
  int temp_r0 = *(int*)(arg0 + 0xfe8);
  if (!temp_r0 || temp_r0 == 1) {
    return *(float*)(arg0 + 0x10d4) <= lbl_14_rodata_960;
  } else {
    return false;
  }
}

extern "C" int fn_14_1A12C(int arg0) {
  int temp_r0 = *(int*)(arg0 + 0x708);
  if (temp_r0 == 1 || temp_r0 == 3) {
    return arg0 + 1844;
  }
  return (int)&CDamageVulnerability::PassThroughVulnerabilty();
}

extern "C" int fn_14_F238(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_14_17A00(int arg0, int arg1) {
  ((CPatterned*)arg0)->CPatterned::AddToRenderer(*(const CStateManager*)arg1);
  ((CPatterned*)arg0)->CPatterned::Render(*(const CStateManager*)arg1);
}

extern "C" void fn_14_104C4(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_14_1047C(int arg0) {
  void fn_14_10680(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_14_104C4(arg0, stack_24, stack_14, stack_8);
  fn_14_10680(stack_24, -1);
}

extern float lbl_14_rodata_9DC;
extern "C" int fn_14_171F8(int arg0) {
  s32 var_r5;
  if ((*(unsigned short*)(arg0 + 0xda0)) != kInvalidUniqueId.value) {
    var_r5 = false;
    if ((*(unsigned char*)(arg0 + 0xdb8) >> 7 & 1) && *(float*)(arg0 + 0xdb0) > lbl_14_rodata_9DC) {
      var_r5 = true;
    }
    return var_r5;
  } else {
    return 1;
  }
}

extern "C" void fn_14_13C1C(int arg0) {
  ((CActor*)arg0)
      ->PlayCustomSound(
          *(const CVector3f*)(arg0 + 84),
          CVector3f(*(float*)(arg0 + 0x28), *(float*)(arg0 + 0x38), *(float*)(arg0 + 0x48)),
          *(const SLdrAudioPlaybackParms*)(arg0 + 2348), false);
}

extern int lbl_14_data_1018;
extern int lbl_14_data_688;
extern "C" int fn_14_12FE0(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_1018;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_14_data_FFC;
extern "C" int fn_14_169F4(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_FFC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_14_data_688;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_14_145A4(int arg0, int arg1) {
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, false);
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)2, true);
  ((CActor*)arg0)->RemoveMaterial((EMaterialTypes)63, *(CStateManager*)arg1);
}

extern "C" void fn_14_14604(int arg0, int arg1) {
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)15, true);
  ((CActor*)arg0)->SetVisorOrbitableFlags((CVisorParameters::EVisorOrbitableFlags)2, true);
  ((CActor*)arg0)->AddMaterial((EMaterialTypes)63, *(CStateManager*)arg1);
}

extern int lbl_14_data_1034;
extern "C" void fn_14_19908(int, int);
extern "C" int fn_14_1AAC4(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_14_data_1034;
    fn_14_19908(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_1ACB8(int arg0, int arg1) {
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
  virtual void _51(int);
};
extern "C" void fn_14_1561C(int arg0, int arg1, int arg2) {
  switch (arg2) {
  case 0:
    ((CStateManager*)arg1)->SetBossParams(kInvalidUniqueId, lbl_14_rodata_960, 0);
    ((__mwdec_vt_0*)arg0)->_51(arg1);
    break;
  }
}

struct CScannableObjectInfo;
struct CScannableObjectInfo {};
extern "C" int fn_14_177C8(int arg0) {
  int temp_r4_2;
  int temp_r4_3;
  int temp_r4_4;
  unsigned char temp_r4 = *(unsigned char*)(arg0 + 0x10f4);
  if (!(temp_r4 >> 3 & 1)) {
    if ((temp_r4 >> 4 & 1)) {
      temp_r4_2 = *(int*)(arg0 + 0xd7c);
      if ((unsigned int)temp_r4_2 != 0) {
        return *(int*)(temp_r4_2 + 0x8);
      }
    } else if ((temp_r4 >> 6 & 1)) {
      temp_r4_3 = *(int*)(arg0 + 0xd78);
      if ((unsigned int)temp_r4_3 != 0) {
        return *(int*)(temp_r4_3 + 0x8);
      }
    } else {
      temp_r4_4 = *(int*)(arg0 + 0xd74);
      if ((unsigned int)temp_r4_4 != 0) {
        return *(int*)(temp_r4_4 + 0x8);
      }
    }
  }
  return (int)((CPatterned*)arg0)->CPatterned::GetScannableObjectInfo();
}

extern "C" void fn_14_1AB30(int arg0) {
  *(float*)arg0 = CVector3f::sUpVector.GetX();
  *(float*)(arg0 + 0x4) = CVector3f::sUpVector.GetY();
  *(float*)(arg0 + 0x8) = CVector3f::sUpVector.GetZ();
}

extern "C" void fn_14_10600();
extern "C" void fn_14_105D8(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_14_10600();
  }
}

extern "C" int fn_14_10744(int arg0, int arg1) {
  void fn_14_10680(int, int);
  if (arg0) {
    fn_14_10680(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_14_10680(int arg0, int arg1) {
  void fn_14_106DC();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_14_106DC();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_14_11068(int arg0, int arg1) {
  if ((unsigned int)arg0 == arg1) {
    return;
  }
  if ((*(unsigned char*)(arg1 + 0x8))) {
    if (!(*(unsigned char*)(arg0 + 0x8))) {
      if ((unsigned int)arg0 != 0) {
        *(int*)arg0 = *(int*)arg1;
        *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x4);
      }
      *(unsigned char*)(arg0 + 0x8) = 1;
    } else {
      *(int*)arg0 = *(int*)arg1;
      *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x4);
    }
  } else {
    *(unsigned char*)(arg0 + 0x8) = 0;
  }
}

extern unsigned char lbl_14_data_808[320];
extern unsigned char lbl_14_data_A14[272];
extern unsigned char lbl_14_data_C80[464];
struct __mwdec_vt_0_fn_14_17468 {
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
extern "C" void fn_14_17468(int arg0, int arg1) {
  int temp_r31 = *(int*)(arg0 + 0x350);
  ((CPatterned*)arg0)->CPatterned::SetupStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_14_17468*)temp_r31)->_4(lbl_14_data_808, 20);
  ((__mwdec_vt_1*)temp_r31)->_3(lbl_14_data_A14, 17);
  ((__mwdec_vt_2*)temp_r31)->_5(lbl_14_data_C80, 29);
}

extern unsigned char lbl_14_bss_18;
struct __mwdec_vt_0_fn_14_17B28 {
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
  virtual void _45(int, int, int);
};
extern "C" void fn_14_17B28(int arg0, int arg1) {
  unsigned int stack_c;
  unsigned int stack_8;
  lbl_14_bss_18 = 1;
  *(int*)&stack_c = 0;
  *(int*)&stack_8 = 0;
  if ((*(unsigned char*)(arg0 + 0x421) & 1)) {
    ((CStateManager*)arg1)->GetCharacterRenderMaskAndTarget(stack_c, stack_8);
  }
  ((__mwdec_vt_0_fn_14_17B28*)arg0)->_45(arg1, *(int*)&stack_c, *(int*)&stack_8);
  lbl_14_bss_18 = 0;
}

extern "C" void fn_14_1223C(int, int);
extern "C" void fn_14_12818(int, int, float);
extern "C" void fn_14_17C98(int arg0, int arg1, float arg2) {
  void fn_14_13AE0(int, int, float);
  if (((*(unsigned char*)(arg0 + 0x20)) >> 7 & 1)) {
    ((CPatterned*)arg0)->CPatterned::Think(arg2, *(CStateManager*)arg1);
    ((CCollisionActorManager*)*(int*)(arg0 + 0xd70))
        ->Update(arg2, *(CStateManager*)arg1, (CCollisionActorManager::EUpdateOptions)0);
    ((CLineOfSightTracker*)(arg0 + 3456))->Update(arg2, *(CStateManager*)arg1);
    fn_14_13AE0(arg0, arg1, arg2);
    fn_14_12818(arg0, arg1, arg2);
    fn_14_1223C(arg0, arg1);
  }
}

extern "C" void fn_14_19F18(int arg0, int arg1) {
  float temp_f31;
  float temp_f30;
  float temp_f3 = 6.0f * *(float*)(arg0 + 0x710);
  float temp_f29 = temp_f3 * *(float*)(arg1 + 0x8);
  temp_f30 = temp_f3 * *(float*)(arg1 + 0x18);
  temp_f31 = temp_f3 * *(float*)(arg1 + 0x28);
  CTransform4f stack_8(*(const CTransform4f*)arg1);
  stack_8.AddTranslationX(temp_f29);
  stack_8.AddTranslationY(temp_f30);
  stack_8.AddTranslationZ(temp_f31);
  ((CActor*)arg0)->SetTransform(stack_8);
}

extern "C" void fn_14_17A64(int arg0, int arg1, int arg2, int arg3) {
  if (lbl_14_bss_18) {
    if (!((*(unsigned char*)(arg0 + 0x10f4)) >> 4 & 1) && (*(unsigned char*)(arg0 + 0xef0))) {
      CTransform4f stack_3c(((CPatterned*)arg0)->GetLctrTransform(CSegId(1)));
      ((CModelData*)(arg0 + 3748))
          ->Render(*(const CStateManager*)arg1, stack_3c, (const CActorLights*)*(int*)(arg0 + 0xbc),
                   *(const CModelFlags*)(arg0 + 252));
    }
    ((CPatterned*)arg0)
        ->CPatterned::RenderSystemsToBeDrawnLast(*(const CStateManager*)arg1, arg2, arg3);
  }
}

extern "C" void fn_14_10A2C(int, int, int);
extern "C" void fn_14_10CA0(int, int, int);
extern "C" void fn_14_111E0(int, int, int);
extern "C" void fn_14_FA50(int, int);
extern "C" void fn_14_17BBC(int arg0, int arg1, int arg2, int arg3, float arg4) {
  bool var_r0 = false;
  switch (arg3) {
  case 0:
    switch (*(int*)(arg0 + 0x10d0)) {
    case 0:
      fn_14_111E0(arg0, arg1, arg2 + 48);
      break;
    case 1:
      fn_14_10CA0(arg0, arg1, arg2 + 48);
      fn_14_FA50(arg0, arg1);
      break;
    case 2:
      fn_14_10A2C(arg0, arg1, arg2 + 48);
      break;
    }
    var_r0 = true;
    break;
  }
  if (!var_r0) {
    ((CPatterned*)arg0)
        ->CPatterned::DoUserAnimEvent(*(CStateManager*)arg1, *(const CInt32POINode*)arg2,
                                      (EUserEventType)arg3, arg4);
  }
}

extern "C" void fn_14_19CE0(int arg0, int arg1, float arg2) {
  switch (*(int*)(arg0 + 0x708)) {
  case 2:
    *(float*)(arg0 + 0x710) = *(float*)(arg0 + 0x710) - 0.75f * arg2;
    if (*(float*)(arg0 + 0x710) <= 0.0f) {
      *(float*)(arg0 + 0x710) = 0.0f;
      *(int*)(arg0 + 0x708) = 0;
    }
    ((CActor*)arg0)
        ->RemoveMaterial((EMaterialTypes)41, (EMaterialTypes)40, (EMaterialTypes)63,
                         *(CStateManager*)arg1);
    break;
  case 3:
    *(float*)(arg0 + 0x710) = 0.25f * arg2 + *(float*)(arg0 + 0x710);
    if (*(float*)(arg0 + 0x710) >= 1.0f) {
      *(float*)(arg0 + 0x710) = 1.0f;
      *(int*)(arg0 + 0x708) = 1;
    }
    ((CActor*)arg0)
        ->AddMaterial((EMaterialTypes)41, (EMaterialTypes)40, (EMaterialTypes)63,
                      *(CStateManager*)arg1);
    break;
  case 1:
    ((CActor*)arg0)
        ->AddMaterial((EMaterialTypes)41, (EMaterialTypes)40, (EMaterialTypes)63,
                      *(CStateManager*)arg1);
    break;
  case 0:
  case 4:
    ((CActor*)arg0)
        ->RemoveMaterial((EMaterialTypes)41, (EMaterialTypes)40, (EMaterialTypes)63,
                         *(CStateManager*)arg1);
    break;
  }
}

extern "C" int fn_14_11698(int, int, int*);
extern "C" void fn_14_11788(unsigned char*, int, float);
extern "C" void fn_14_118B0(int arg0, int arg1) {
  int temp_r3;
  int temp_r5;
  int temp_r5_2;
  unsigned char stack_24[20];
  unsigned char stack_18[12];
  unsigned char stack_c[12];
  int stack_8;
  *(int*)arg1 = 0;
  *(float*)&stack_8 = 0.0f;
  if ((unsigned char)fn_14_11698(arg0, arg0 + 84, &stack_8)) {
    fn_14_11788(stack_24, arg0, *(float*)&stack_8);
    temp_r5 = arg1 + *(int*)arg1 * 12;
    *(float*)(temp_r5 + 0x4) = *(float*)stack_24;
    *(float*)(temp_r5 + 0x8) = *(float*)(stack_24 + 0x4);
    *(float*)(temp_r5 + 0xc) = *(float*)(stack_24 + 0x8);
    *(int*)arg1 = *(int*)arg1 + 1;
    fn_14_11788(stack_18, arg0, 0.13962634f + *(float*)&stack_8);
    temp_r5_2 = arg1 + *(int*)arg1 * 12;
    *(float*)(temp_r5_2 + 0x4) = *(float*)stack_18;
    *(float*)(temp_r5_2 + 0x8) = *(float*)(stack_18 + 0x4);
    *(float*)(temp_r5_2 + 0xc) = *(float*)(stack_18 + 0x8);
    *(int*)arg1 = *(int*)arg1 + 1;
    fn_14_11788(stack_c, arg0, *(float*)&stack_8 - 0.13962634f);
    temp_r3 = arg1 + *(int*)arg1 * 12;
    *(float*)(temp_r3 + 0x4) = *(float*)stack_c;
    *(float*)(temp_r3 + 0x8) = *(float*)(stack_c + 0x4);
    *(float*)(temp_r3 + 0xc) = *(float*)(stack_c + 0x8);
    *(int*)arg1 = *(int*)arg1 + 1;
  }
}

extern "C" void fn_14_F0B8(int arg0, int arg1) {
  int temp_r3;
  int temp_r3_2;
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
    case -771585884:
      temp_r3_2 = *(int*)(arg1 + 0x8);
      *(int*)(arg1 + 0x8) = temp_r3_2 + 4;
      *(int*)arg0 = *(int*)temp_r3_2;
      break;
    case 1977389756:
      *(float*)(arg0 + 0x4) = ((CInputStream*)arg1)->ReadFloat();
      break;
    case -1642315924:
      *(float*)(arg0 + 0x8) = ((CInputStream*)arg1)->ReadFloat();
      break;
    case -33774283:
      *(float*)(arg0 + 0xc) = ((CInputStream*)arg1)->ReadFloat();
      break;
    case -855416020:
      *(float*)(arg0 + 0x10) = ((CInputStream*)arg1)->ReadFloat();
      break;
    case -246119529:
      *(float*)(arg0 + 0x14) = ((CInputStream*)arg1)->ReadFloat();
      break;
    case -110632378:
      *(float*)(arg0 + 0x18) = ((CInputStream*)arg1)->ReadFloat();
      break;
    default:
      ((CInputStream*)arg1)->ReadBytes(nullptr, temp_r5_2);
      break;
    }
    var_r29 = var_r29 + 1;
  }
}

extern "C" int fn_14_EBE0(int obj, int val) {
  void fn_14_F238(int, int);
  if (obj) {
    ((SLdrDamageVulnerability*)(obj + 1108))->~SLdrDamageVulnerability();
    ((SLdrDamageVulnerability*)(obj + 760))->~SLdrDamageVulnerability();
    fn_14_F238(obj + 732, -1);
    fn_14_F238(obj + 704, -1);
    fn_14_F238(obj + 676, -1);
    fn_14_F238(obj + 648, -1);
    fn_14_F238(obj + 620, -1);
    fn_14_F238(obj + 592, -1);
    ((SLdrAudioPlaybackParms*)(obj + 552))->~SLdrAudioPlaybackParms();
    ((SLdrDamageInfo*)(obj + 536))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(obj + 516))->~SLdrDamageInfo();
    ((SLdrPlasmaBeamInfo*)(obj + 440))->~SLdrPlasmaBeamInfo();
    ((SLdrDamageInfo*)(obj + 412))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(obj + 392))->~SLdrDamageInfo();
    ((SLdrAudioPlaybackParms*)(obj + 364))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 340))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 316))->~SLdrAudioPlaybackParms();
    ((SLdrEchoParameters*)(obj + 288))->~SLdrEchoParameters();
    ((SLdrAudioPlaybackParms*)(obj + 264))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 240))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 216))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 192))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 168))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 124))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 100))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 76))->~SLdrAudioPlaybackParms();
    ((SLdrAudioPlaybackParms*)(obj + 44))->~SLdrAudioPlaybackParms();
    if ((short)val > 0) {
      CMemory::Free((const void*)obj);
    }
  }
  return obj;
}

template < class T0 >
int TCastToPtr(CEntity*);
extern "C" void fn_14_F2B8(int obj, int obj2, int obj3, int val) {
  const __typeof__(((CEntity*)obj)
                       ->CheckConnectedObject(*(const CStateManager*)obj2, kSS_InternalState7,
                                              kSM_Follow)) temp_0 =
      ((CEntity*)obj)
          ->CheckConnectedObject(*(const CStateManager*)obj2, kSS_InternalState7, kSM_Follow);
  CActor* actor =
      (CActor*)TCastToPtr< CScriptWaypoint >(((CStateManager*)obj2)->ObjectById(temp_0));
  if (actor) {
    actor->SetTranslation(*(const CVector3f*)obj3);
    ((CEntity*)obj)
        ->SendScriptMsgs((EScriptObjectState)val, *(CStateManager*)obj2,
                         TUniqueId((ushort) * (unsigned short*)((char*)obj + 0x8)), kSM_None);
  }
}
