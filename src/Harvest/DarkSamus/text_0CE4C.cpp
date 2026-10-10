// Raw matching-decompiler output (demwcc-echoes) for a REL without a source file yet.
// Kept for reference only: not cleaned up, names and types are placeholders.

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMRay.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CRayCastResult.hpp"
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
#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CToken.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/IObj.hpp"
#include "Kyoto/Input/CRumbleVoice.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CCylinder.hpp"
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
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/ActorCommon.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/BodyState/CBodyStateCmdMgr.hpp"
#include "MetroidPrime/BodyState/CBodyStateInfo.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CAxisAngle.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CDecalManager.hpp"
#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CKnockBackInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/CRELFileToken.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSimpleShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CSteeringBehaviors.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/Enemies/CAi.hpp"
#include "MetroidPrime/Enemies/CAiKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CAnimationState.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPathFindNavigation.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/PathFinding/CPathFindArea.hpp"
#include "MetroidPrime/PathFinding/CPathFindRegion.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrActorParameters.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAnimationSet.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrAudioPlaybackParms.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrDamageInfo.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrEditorProperties.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrIngPossessionData.hpp"
#include "MetroidPrime/ScriptLoader/Structs/SLdrPatternedAITypedef.hpp"
#include "MetroidPrime/ScriptLoaderRel.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/ScriptObjects/CScriptAIHint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptRepulsor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/StateMachineCommon.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerGun.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CFreezeBeamProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CImpactVisorEffect.hpp"
#include "MetroidPrime/Weapons/CProjectileInfo.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "MetroidPrime/Weapons/WeaponTypes.hpp"
#include "Weapons/CCollisionResponseData.hpp"
#include "Weapons/CDecalDescription.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "dolphin/gx/GXEnum.h"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/rmemory_allocator.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"
#include "types.h"

extern "C" void fn_10_14448() {}

extern "C" void fn_10_181DC() {}

extern "C" void fn_10_F920() {}

extern "C" int fn_10_16ADC() { return 8; }

extern "C" int fn_10_16B54(int arg0) { return *(unsigned char*)(arg0 + 0xa80); }

extern "C" int fn_10_16B5C(int arg0) { return *(unsigned char*)(arg0 + 0xa40); }

extern "C" int fn_10_CE4C(int arg0) { return arg0 + 1984; }

extern "C" int fn_10_17E48(int arg0) { return *(int*)(arg0 + 0x978); }

extern "C" bool fn_10_CE5C() { return false; }

extern "C" int fn_10_CE54(int arg0) { return *(unsigned char*)(arg0 + 0x44f); }

extern "C" bool fn_10_CE64() { return false; }

extern "C" int fn_10_CE94(int arg0) { return arg0 + 1876; }

extern "C" bool fn_10_CE9C() { return true; }

extern "C" bool fn_10_CEA4() { return false; }

extern "C" bool fn_10_CEAC() { return false; }

extern "C" int fn_10_F864(int arg0) { return *(unsigned char*)(arg0 + 0xd4c); }

extern "C" void fn_10_100D0(int arg0) { *(int*)(arg0 + 0x4) = 0; }

extern "C" bool fn_10_16B48(int arg0) {
  return CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0xab8), 1, 7);
}

extern "C" bool fn_10_11C54(int arg0) {
  return CGX::ShiftRightAndMask((uint) * ((char*)arg0 + 0xb20), 1, 7);
}

extern "C" float fn_10_CE88() { return kDefaultGravityAccel; }

extern "C" bool fn_10_CE7C(int arg0) {
  return CGX::ShiftRightAndMask((uint) * ((char*)arg0 + 0x34c), 1, 3);
}

extern "C" bool fn_10_F840(int arg0) {
  return CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0x90c), 1, 7);
}

extern "C" bool fn_10_F924(int arg0) {
  return CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0x90c), 1, 6);
}

extern "C" void fn_10_CE6C(int arg0) { *(unsigned short*)arg0 = kInvalidUniqueId.value; }

extern "C" int fn_10_1AA5C(int arg0, int arg1) { return (arg1 == 2) ? 1 : 0; }

extern "C" int fn_10_11724(int arg0) { return (*(int*)(arg0 + 0x984) == 16) ? 1 : 0; }

extern "C" int fn_10_11710(int arg0) {
  if (*(int*)(arg0 + 0x984) == 4) {
    return 1;
  }
  return 0;
}

extern "C" int fn_10_1174C(int arg0) {
  if (*(int*)(arg0 + 0x984) == 32) {
    return 1;
  }
  return 0;
}

extern "C" int fn_10_11778(int arg0) {
  if (*(int*)(arg0 + 0x984) == 256) {
    return 1;
  }
  return 0;
}

extern "C" int fn_10_11738(int arg0) { return (*(int*)(arg0 + 0x984) == 8) ? 1 : 0; }

extern "C" int fn_10_1178C(int arg0) {
  if (*(int*)(arg0 + 0x984) == 128) {
    return 1;
  } else {
    return 0;
  }
}

extern "C" int fn_10_117D8(int arg0) { return (*(int*)(arg0 + 0x984) == 1) ? 1 : 0; }

extern "C" int fn_10_117EC(int arg0) { return (*(int*)(arg0 + 0x984) == 512) ? 1 : 0; }

extern "C" int fn_10_11800(int arg0) { return (*(int*)(arg0 + 0x984) == 1024) ? 1 : 0; }

extern float lbl_10_rodata_544;
extern "C" float fn_10_1BDF0(int arg0) { return lbl_10_rodata_544 + *(float*)(arg0 + 0x3dc); }

extern "C" void fn_10_18628(int arg0, float arg1, float arg2) {
  *(float*)(arg0 + 0xe04) = *(float*)(arg0 + 0x8b0);
  *(float*)(arg0 + 0xe08) = arg1;
  *(float*)(arg0 + 0xe0c) = arg2;
}

extern int lbl_10_rodata_3BC;
extern "C" float fn_10_EDB8(int arg0) { return ((float*)&lbl_10_rodata_3BC)[arg0]; }

extern int lbl_10_rodata_39C;
extern "C" float fn_10_EDCC(int arg0) { return ((float*)&lbl_10_rodata_39C)[arg0]; }

extern int lbl_10_rodata_37C;
extern "C" int fn_10_EDE0(int arg0) { return ((int*)&lbl_10_rodata_37C)[arg0]; }

extern "C" bool fn_10_106F8(int arg0) { return *(float*)(arg0 + 0xc30) < *(float*)(arg0 + 0x8b0); }

extern "C" int fn_10_11760(int arg0) { return (*(int*)(arg0 + 0x984) == 0x100000) ? 1 : 0; }

extern "C" int fn_10_116F8(int arg0) {
  if (*(int*)(arg0 + 0x984) == 0x20000) {
    return 1;
  }
  return 0;
}

extern "C" void fn_10_CEB4(int arg0, int arg1) {
  *(float*)arg0 = *(float*)(arg1 + 0x54);
  *(float*)(arg0 + 0x4) = *(float*)(arg1 + 0x58);
  *(float*)(arg0 + 0x8) = *(float*)(arg1 + 0x5c);
}

extern "C" void fn_10_EA08(int arg0) {
  *(unsigned short*)(arg0 + 0x38) = kInvalidUniqueId.value;
  *(unsigned char*)(arg0 + 0x3a) = 0;
}

extern "C" int fn_10_FD30(int arg0) {
  if (CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0x420), 1, 6) == 0) {
    return 0;
  } else {
    return *(unsigned char*)(arg0 + 0xd58);
  }
}

extern "C" void RELMain() {
  void fn_10_CF14();
  fn_10_CF14();
}

extern "C" void fn_10_112E8() {
  void fn_10_11308();
  fn_10_11308();
}

extern "C" void fn_10_4E6C();
extern "C" void fn_10_134FC() { fn_10_4E6C(); }

extern "C" void fn_10_5010();
extern "C" void fn_10_1351C() { fn_10_5010(); }

extern "C" void fn_10_20D58() {
  void fn_10_20D78();
  fn_10_20D78();
}

extern "C" int fn_10_F84C(int arg0) {
  if (((*(unsigned char*)(arg0 + 0xcd0)) == 1)) {
    return 106;
  } else {
    return 46;
  }
}

extern "C" void fn_10_DE1C() {
  void fn_10_DE3C();
  fn_10_DE3C();
}

extern "C" void fn_10_10634(int arg0, int arg1) {
  void fn_10_17D50(int, int, int);
  fn_10_17D50(arg0, arg1, 1);
}

extern "C" void RELExit() { SetSDarkSamus_FuncPtrs(nullptr); }

extern "C" void fn_10_16B8C(int, int);
extern "C" void fn_10_16B64(int arg0, int arg1) { fn_10_16B8C(arg0, *(int*)(arg1 + 0x14fc) + 84); }

extern "C" int fn_10_16B1C(int arg0, int arg1) {
  if (!((CPathFindSearch*)(arg0 + 1984))->OnPath(*(CVector3f*)arg1)) {
    return 1;
  }
  return 0;
}

extern "C" void fn_10_1AA30(int arg0, int arg1, int arg2, int arg3) {
  if (((unsigned char)arg2 == 1)) {
    *(int*)arg1 = *(int*)arg1 | arg3;
  } else {
    *(int*)arg1 = *(int*)arg1 & u32(~arg3);
  }
}

extern "C" void fn_10_20C2C();
extern "C" int fn_10_20BFC(int arg0) {
  fn_10_20C2C();
  return arg0;
}

extern "C" void fn_10_20CC4();
extern "C" int fn_10_20C94(int arg0) {
  fn_10_20CC4();
  return arg0;
}

extern void* lbl_10_bss_110;
extern "C" void fn_10_CF44();
extern "C" void fn_10_CF14() {
  lbl_10_bss_110 = &fn_10_CF44;
  SetSDarkSamus_FuncPtrs((SDarkSamus_FuncPtrs*)&(*(int*)&lbl_10_bss_110));
}

extern float lbl_10_rodata_460;
extern "C" float fn_10_17CEC();
extern "C" bool fn_10_17CB8() { return fn_10_17CEC() > lbl_10_rodata_460; }

extern "C" int fn_11_B70();
extern "C" int fn_10_17E14() {
  int fn_10_17DE0();
  if ((unsigned int)fn_10_17DE0() == 0) {
    return 0;
  }
  return fn_11_B70();
}

extern "C" int fn_10_117A0(int arg0) {
  if ((*(unsigned char*)(arg0 + 0xdec)) == 1) {
    return (*(int*)(arg0 + 0x984) == 0x200000) ? 1 : 0;
  } else {
    return (*((int*)(arg0 + 0x984)) == 64) ? 1 : 0;
  }
}

extern float lbl_10_rodata_3E4;
extern "C" float fn_10_16AE4(int arg0) {
  const CAABox* temp_r3 = &((CPhysicsActor*)arg0)->GetBaseBoundingBox();
  return lbl_10_rodata_3E4 * temp_r3->GetWidth();
}

extern "C" int fn_10_2199C(int arg0) {
  void fn_10_219D4();
  fn_10_219D4();
  *(unsigned char*)(arg0 + 0x38) = 0;
  return arg0;
}

extern "C" bool fn_10_110E0(int arg0, int arg1) {
  void fn_10_1A5BC(int, int, unsigned char*, int*);
  unsigned char stack_c[100];
  int stack_8;
  *(int*)stack_c = 0;
  *(int*)&stack_8 = 0;
  fn_10_1A5BC(arg0, arg1, stack_c, &stack_8);
  return CGX::ShiftRightAndMask(uint(*(int*)&stack_8), 1, 23);
}

extern "C" int fn_10_1111C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern float lbl_10_rodata_3FC;
extern float lbl_10_rodata_46C;
extern "C" bool fn_10_112AC(int arg0) {
  float temp_f2 = *(float*)(arg0 + 0xc0c);
  if (lbl_10_rodata_3FC == temp_f2) {
    return false;
  } else {
    return lbl_10_rodata_46C + temp_f2 > *(float*)(arg0 + 0x8b0);
  }
}

extern "C" void fn_10_1382C(int arg0, int arg1) {
  CPlayer* temp_0 = (CPlayer*)*(int*)(arg1 + 0x14fc);
  temp_0->SetOrbitTargetId(kInvalidUniqueId, *(const CStateManager*)arg1);
}

extern "C" int fn_10_13FC0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_14180(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_14978(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" void fn_10_18FCC(int, int, int);
extern "C" void fn_10_19014();
extern "C" void fn_10_18F90(int arg0, int arg1, int arg2) {
  if (!arg2) {
    fn_10_19014();
  } else if (arg2 == 2) {
    fn_10_18FCC(arg0, arg1, 1);
  }
}

extern "C" int fn_10_214D0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_2150C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_21584(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_21548(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_215C0(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_21BE8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_21C9C(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" int fn_10_21CD8(int arg0, int arg1) {
  if (arg0 && (short)arg1 > 0) {
    CMemory::Free((const void*)arg0);
  }
  return arg0;
}

extern "C" bool fn_10_10658(int arg0) {
  float temp_f31 = *(float*)(arg0 + 0x3dc);
  return fn_10_17CEC() < temp_f31;
}

extern float lbl_10_rodata_514;
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
  virtual int _13();
};
extern "C" float fn_10_17E50(int arg0, int arg1) {
  return (*(float*)(((__mwdec_vt_0*)*(int*)(arg1 + 0x14fc))->_13() + 0x4)) - lbl_10_rodata_514;
}

extern float lbl_10_rodata_428;
extern float lbl_10_rodata_42C;
extern "C" bool fn_10_F800(int arg0) {
  if (*(float*)(arg0 + 0x964) > lbl_10_rodata_42C) {
    return true;
  } else {
    return lbl_10_rodata_428 + *(float*)(arg0 + 0xc44) > *(float*)(arg0 + 0x8b0);
  }
}

extern "C" bool fn_10_14284() {
  int fn_10_17E14();
  int temp_r3 = fn_10_17E14();
  if ((unsigned int)temp_r3 == 0) {
    return false;
  }
  return *(float*)(temp_r3 + 0x24) > lbl_10_rodata_3FC;
}

extern "C" void fn_10_207BC();
extern "C" int fn_10_20774(int arg0, int arg1) {
  fn_10_207BC();
  *(int*)(arg0 + 0x8) = arg1;
  ((CToken*)arg0)->Lock();
  return arg0;
}

extern int lbl_10_data_1070;
extern "C" int fn_10_20A24(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_1070;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_1B05C();
extern "C" int fn_10_1B00C(int arg0, int arg1) {
  if (arg0) {
    fn_10_1B05C();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_21A14(int arg0, int arg1, int arg2, int arg3, int arg4) {
  void fn_10_219D4();
  fn_10_219D4();
  *(int*)(arg0 + 0x38) = arg3;
  *(int*)(arg0 + 0x3c) = arg4;
  return arg0;
}

extern "C" int fn_10_1F534(int arg0, int arg1) {
  void fn_10_1F588(int, int);
  if (arg0) {
    fn_10_1F588(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_1F650(int arg0, int arg1) {
  void fn_10_1F588(int, int);
  if (arg0) {
    fn_10_1F588(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_1F5DC(int, int);
extern "C" int fn_10_1F9C0(int arg0, int arg1) {
  if (arg0) {
    fn_10_1F5DC(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" CColor fn_10_181E0(int arg0, int arg1) {
  float temp_f0 = *(float*)(arg0 + 0xd14);
  float var_f3 = rstl::min_val(temp_f0, *(float*)(arg0 + 0xde8));
  return CColor(var_f3 * *(float*)(arg0 + 0xd18), var_f3 * *(float*)(arg0 + 0xd1c),
                var_f3 * *(float*)(arg0 + 0xd20), lbl_10_rodata_3E4);
}

extern "C" int fn_10_1AA6C(int arg0) {
  float temp_f1 = fn_10_17CEC();
  if (temp_f1 < *(float*)(arg0 + 0x3dc)) {
    return 0;
  }
  if (temp_f1 > *(float*)(arg0 + 0x3e0)) {
    return 2;
  }
  return 1;
}

extern "C" void fn_10_1863C(int, float, float);
extern "C" void fn_10_1890C(int arg0) {
  float fn_10_185CC(int);
  float temp_f1;
  if ((*(unsigned char*)(arg0 + 0xdfc))) {
    ((CTexture*)*(int*)(arg0 + 0xdf8))->Load((_GXTexMapID)0, (CTexture::EClampMode)1);
    temp_f1 = fn_10_185CC(arg0);
    fn_10_1863C(arg0, temp_f1, *(float*)(arg0 + 0xe00));
  }
}

extern int lbl_10_data_10D4;
extern int lbl_10_data_1D4;
extern "C" int fn_10_10F98(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_10D4;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_10BC;
extern "C" int fn_10_12708(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_10BC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_10C8;
extern "C" int fn_10_12800(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_10C8;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_10B0;
extern "C" int fn_10_17760(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_10B0;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_10A4;
extern "C" int fn_10_18F34(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_10A4;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern float lbl_10_rodata_3E8;
extern "C" float fn_10_185CC(int arg0) {
  float temp_f0;
  float temp_f1;
  float temp_f2;
  float temp_f3;
  temp_f1 = *(float*)(arg0 + 0xe08);
  float temp_f4 = *(float*)(arg0 + 0xe0c);
  temp_f2 = *(float*)(arg0 + 0xe04);
  temp_f3 = *(float*)(arg0 + 0x8b0);
  if (temp_f3 > temp_f2 + (temp_f1 + temp_f4)) {
    return lbl_10_rodata_3FC;
  } else {
    temp_f0 = temp_f2 + temp_f1;
    if (temp_f3 > temp_f0) {
      return lbl_10_rodata_3E8 - (temp_f3 - temp_f0) / temp_f4;
    } else {
      return lbl_10_rodata_3E8;
    }
  }
}

extern int lbl_10_data_107C;
extern "C" int fn_10_209C8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_107C;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1070;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_21D98(int arg0, int arg1) {
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

extern "C" int fn_10_E0C4(int arg0, int arg1) {
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

extern "C" bool fn_10_10698(int arg0) {
  int temp_r4;
  if ((*(int*)(arg0 + 0x980)) == 0x8000) {
    return false;
  } else {
    temp_r4 = *((int*)(arg0 + 0x994));
    if (temp_r4 > 0 && (*(int*)(0x998 + arg0 + (temp_r4 - 1 << 2))) == 0x8000) {
      return 0;
    } else {
      return *(float*)(arg0 + 0xcc0) > *(float*)(arg0 + 0xcc8);
    }
  }
}

extern "C" int fn_10_21B84(int arg0, int arg1) {
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

extern "C" int fn_10_E060(int arg0, int arg1) {
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

extern "C" void fn_10_1A8F8(int, int, int, int, int, int, int);
extern "C" void fn_10_1A554(int arg0, int arg1, int arg2, int arg3) {
  void fn_10_1A980(int, int, int, int, int, int, int);
  if ((*(unsigned char*)(arg0 + 0xdec)) == 1) {
    fn_10_1A8F8(arg0, arg3, 0, 0x100000, 1, 0x400000, 0x800000);
  } else {
    fn_10_1A980(arg0, arg3, 0, 0x100000, 0x200000, 65535, 65535);
  }
}

extern float lbl_10_rodata_3F0;
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
extern "C" float fn_10_17E90(int arg0) {
  float temp_f31 = *(float*)((__mwdec_vt_0*)arg0)->_13();
  float temp_f0 = *(float*)(((__mwdec_vt_1*)arg0)->_13() + 0x4);
  return lbl_10_rodata_3F0 * (temp_f0 / temp_f31);
}

extern "C" int fn_10_EFC8();
extern "C" void fn_10_128D8(int arg0) {
  if (!(*(unsigned char*)(arg0 + 0x928)) && (unsigned char)fn_10_EFC8() == 0) {
    if (!(*(unsigned char*)(arg0 + 0x928))) {
      *(float*)(arg0 + 0x91c) = *(float*)(arg0 + 0x54);
      *(float*)(arg0 + 0x920) = *(float*)(arg0 + 0x58);
      *(float*)(arg0 + 0x924) = *(float*)(arg0 + 0x5c);
      *(unsigned char*)(arg0 + 0x928) = 1;
    } else {
      *(float*)(arg0 + 0x91c) = *(float*)(arg0 + 0x54);
      *(float*)(arg0 + 0x920) = *(float*)(arg0 + 0x58);
      *(float*)(arg0 + 0x924) = *(float*)(arg0 + 0x5c);
    }
  }
}

extern "C" void fn_10_20DA0();
extern "C" void fn_10_20D78(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_10_20DA0();
  }
}

extern "C" void fn_10_DE64();
extern "C" void fn_10_DE3C(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_10_DE64();
  }
}

extern "C" int fn_10_11308(int arg0, int arg1) {
  int temp_r3 = *(int*)(arg1 + 0x14fc);
  return ((((*(int*)(temp_r3 + 0x390)) == 0) ? *(int*)(temp_r3 + 0x38c) : 0) == 1) ? 1 : 0;
}

extern "C" int fn_10_219D4(int arg0) {
  fn_10_20C2C();
  *(int*)(arg0 + 0x2c) = 0;
  *(int*)(arg0 + 0x30) = 0;
  *(unsigned char*)(arg0 + 0x34) = 0;
  return arg0;
}

extern "C" int fn_10_1F588(int arg0, int arg1) {
  if (arg0) {
    fn_10_1F5DC(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_1AFB4(int arg0, int arg1) {
  void fn_10_1B00C(int, int);
  if (arg0) {
    fn_10_1B00C(arg0 + 24, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_450;
extern int lbl_10_data_964;
struct __mwdec_vt_0_fn_10_18D78 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3();
  virtual void _4(void*, int);
};
struct __mwdec_vt_1_fn_10_18D78 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
  virtual void _3(void*, int);
};
extern "C" void fn_10_18D78(int arg0, int arg1) {
  ((CPatterned*)arg0)->InitializeStateMachine(*(CStateManager*)arg1);
  ((__mwdec_vt_0_fn_10_18D78*)*(int*)(arg0 + 0x350))->_4(&lbl_10_data_450, 49);
  ((__mwdec_vt_1_fn_10_18D78*)*(int*)(arg0 + 0x350))->_3(&lbl_10_data_964, 43);
}

struct __mwdec_vt_0_fn_10_208C0 {
  virtual void _0(int);
};
extern "C" int fn_10_208C0(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if ((*(unsigned char*)arg0)) {
      temp_r3 = *(int*)(arg0 + 0x4);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_10_208C0*)temp_r3)->_0(1);
      }
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" float fn_10_1A6C4(int, int, int);
extern "C" int fn_10_17D50(int arg0, int arg1, int arg2) {
  int fn_10_17E14();
  int temp_r3 = fn_10_17E14();
  if (!temp_r3) {
    return 0;
  }
  switch (arg2) {
  case 2:
    return 0;
  case 2048:
    return *(unsigned char*)(temp_r3 + 0x40);
  }
  return fn_10_1A6C4(arg0, temp_r3, arg2) > 0.0f;
}

struct __mwdec_vt_0_fn_10_1F928 {
  virtual void _0(int);
};
extern "C" int fn_10_1F928(int arg0, int arg1) {
  int temp_r3;
  if (arg0) {
    if (arg0 + 164 && (*(unsigned char*)(arg0 + 0xa4))) {
      temp_r3 = *(int*)(arg0 + 0xa8);
      if ((unsigned int)temp_r3 != 0) {
        ((__mwdec_vt_0_fn_10_1F928*)temp_r3)->_0(1);
      }
    }
    ((CDamageVulnerability*)(arg0 + 80))->~CDamageVulnerability();
    fn_10_1F5DC(arg0, 0);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_FFB4(int, const CDamageVulnerability*);
extern "C" int fn_10_FF20(int arg0) {
  if ((*(unsigned char*)(arg0 + 0xcd0)) == 1) {
    return arg0 + 3284;
  }
  if ((unsigned int)(*(int*)(arg0 + 0x980) - 0x800000) == 0) {
    fn_10_FFB4(arg0 + 4048, ((CPatterned*)arg0)->CPatterned::GetDamageVulnerability());
    CWeaponTypeVulnerability stack_8(lbl_10_rodata_3FC, (CWeaponTypeVulnerability::EEffect)0,
                                     false);
    ((CDamageVulnerability*)(arg0 + 4048))->SetComboVulnerability(3, stack_8);
    return arg0 + 4048;
  }
  return (int)((CAi*)arg0)->CAi::DamageVulnerability();
}

extern "C" void fn_10_194AC(int arg0, int arg1, int arg2) {
  int fn_10_17DE0(int, int);
  unsigned short temp_r4;
  int temp_r3 = fn_10_17DE0(arg1, arg2);
  if ((unsigned int)temp_r3 == 0) {
    *(unsigned short*)arg0 = *(unsigned short*)(arg1 + 0x974);
  } else {
    temp_r4 = ((CEntity*)temp_r3)
                  ->FindConnectedObject(*(const CStateManager*)arg2, (EScriptObjectState)0x434f4e4e,
                                        (EScriptObjectMessage)-1)
                  .value;
    if (temp_r4 != kInvalidUniqueId.value) {
      *(unsigned short*)arg0 = temp_r4;
    } else {
      *(unsigned short*)arg0 = *(unsigned short*)(arg1 + 0x974);
    }
  }
}

extern float lbl_10_rodata_4C4;
extern "C" int fn_10_142F4(int, int, float);
extern "C" void fn_10_1A1D4(int arg0, int arg1, int arg2, int arg3) {
  void fn_10_1A980(int, int, int, int, int, int, int);
  fn_10_1A980(arg0, arg3, 0, 2048, 65535, 65535, 65535);
  if ((unsigned char)fn_10_142F4(arg0, arg1, lbl_10_rodata_4C4) == 0) {
    fn_10_1A8F8(arg0, arg3, 0, 1, 65535, 65535, 65535);
  }
}

struct __mwdec_vt_0_fn_10_1B154 {
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
  virtual int _13(float);
};
extern "C" void fn_10_1B154(int arg0, int arg1) {
  float temp_f1_2;
  float temp_f1 = *(float*)(arg0 + 0x10a0);
  if (temp_f1 > 0.0f && !(*(unsigned char*)(arg0 + 0x10a4))) {
    temp_f1_2 = *(float*)(((__mwdec_vt_0_fn_10_1B154*)arg0)->_13(temp_f1) + 0x4);
    if (temp_f1_2 < *(float*)(arg0 + 0x10a0)) {
      ((CEntity*)arg0)
          ->SendScriptMsgs((EScriptObjectState)0x41495331, *(CStateManager*)arg1, kInvalidUniqueId,
                           (EScriptObjectMessage)-1);
      *(unsigned char*)(arg0 + 0x10a4) = 1;
    }
  }
}

extern "C" int fn_10_166AC(int, int, int, int, float, float);
extern "C" bool fn_10_16600(int arg0, int arg1, int arg2, float arg3, float arg4) {
  int fn_10_16ADC(int);
  int var_r31;
  for (var_r31 = 0; var_r31 < fn_10_16ADC(arg0); var_r31 = var_r31 + 1) {
    if ((unsigned char)fn_10_166AC(arg0, arg1, arg2, var_r31, arg3, arg4) == 0) {
      return false;
    }
  }
  return true;
}

extern "C" void fn_10_1A278(int arg0, int arg1, int arg2, int arg3) {
  int fn_10_11308();
  void fn_10_1A980(int, int, int, int, int, int, int);
  if ((unsigned char)fn_10_11308() == 1) {
    fn_10_1A980(arg0, arg3, 0, 8, 16, 32, 65535);
    fn_10_1A980(arg0, arg3, 0, 128, 256, 512, 0x10000);
    fn_10_1A980(arg0, arg3, 1, 64, 65535, 65535, 65535);
  } else {
    fn_10_1A980(arg0, arg3, 0, 4, 65535, 65535, 65535);
  }
}

extern "C" int fn_10_10BBC(int arg0) {
  int var_r5;
  for (var_r5 = 0; var_r5 < 24; var_r5++) {
    if (1 << var_r5 == arg0) {
      return var_r5;
    }
  }
  return 65535;
}

extern int lbl_10_bss_128;
extern "C" void fn_10_105F4(int, int, void*);
extern "C" void fn_10_1B478(int, int, int, int);
extern "C" void fn_10_1C778(int, int, int, float);
extern "C" void fn_10_1D794(int, int, float);
extern "C" int fn_10_FC64(int, int, int);
extern "C" void fn_10_FB8C(int arg0, int arg1, int arg2, float arg3) {
  fn_10_1B478(arg0, 8192, arg1, arg2);
  fn_10_105F4(arg0, arg2, &lbl_10_bss_128);
  int temp_r3 = fn_10_FC64(arg0, arg1, 14);
  if (temp_r3) {
    if (!arg2) {
      fn_10_1C778(arg0, arg1, temp_r3 + 84, arg3);
    } else {
      *(unsigned short*)(arg0 + 0x718) = *(unsigned short*)(temp_r3 + 0x8);
      ((CPathFindNavigation*)(arg0 + 1780))
          ->PathFind(*(CStateManager*)arg1, (EStateMsg)arg2, arg3, *(CPatterned*)arg0);
    }
  }
  fn_10_1D794(arg0, arg2, arg3);
}

extern "C" int fn_10_19918(int arg0, int arg1, int arg2, int arg3) {
  int var_r31;
  int temp_r0;
  int temp_r4;
  int var_r7;
  int i;
  int var_r8 = 0;
  int var_r30 = 0;
  var_r31 = arg2 + 4;
  var_r7 = var_r31;
  while (var_r8 < (*(int*)arg2)) {
    if ((arg3 & 1 << var_r8)) {
      if ((*(int*)var_r7) <= 0) {
        *(int*)var_r7 = 1;
      }
      var_r30 = var_r30 + *(int*)var_r7;
    } else {
      *(int*)var_r7 = 0;
    }
    var_r7 = var_r7 + 4;
    var_r8 = var_r8 + 1;
  }
  if (!var_r30) {
    return 65535;
  }
  int temp_r3 = ((CRandom16*)(arg1 + 5860))->Next();
  temp_r4 = *(int*)arg2;
  int var_r3 = temp_r3 % var_r30;
  for (i = 0; i < temp_r4; i++) {
    temp_r0 = *(int*)var_r31;
    if (temp_r0 > 0) {
      var_r3 = var_r3 - temp_r0;
      if (var_r3 <= 0) {
        return 1 << i * 1;
      }
    }
    var_r31 = var_r31 + 4;
  }
  return 65535;
}

extern "C" void fn_10_1A440(int arg0, int arg1, int arg2, int arg3) {
  void fn_10_1A980(int, int, int, int, int, int, int);
  int fn_10_1AA6C();
  switch (fn_10_1AA6C()) {
  case 0:
    fn_10_1A8F8(arg0, arg3, 0, 1, 2, 2048, 65535);
    break;
  case 1:
    fn_10_1A980(arg0, arg3, 0, 1, 65535, 65535, 65535);
    break;
  case 2:
    fn_10_1A980(arg0, arg3, 0, 1, 4, 8, 16);
    fn_10_1A980(arg0, arg3, 0, 32, 64, 256, 65535);
    fn_10_1A980(arg0, arg3, 0, 0x200000, 0x100000, 65535, 65535);
    break;
  }
}

extern "C" int fn_10_E124(int arg0, int arg1) {
  if (arg0) {
    ((SLdrDamageInfo*)(arg0 + 1152))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1132))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1116))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 1088))->~SLdrDamageInfo();
    ((SLdrAudioPlaybackParms*)(arg0 + 1044))->~SLdrAudioPlaybackParms();
    ((SLdrDamageInfo*)(arg0 + 1004))->~SLdrDamageInfo();
    ((SLdrAnimationSet*)(arg0 + 992))->~SLdrAnimationSet();
    ((SLdrDamageInfo*)(arg0 + 956))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 924))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 904))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 876))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 848))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 812))->~SLdrDamageInfo();
    ((SLdrDamageInfo*)(arg0 + 788))->~SLdrDamageInfo();
    ((SLdrActorParameters*)(arg0 + 640))->~SLdrActorParameters();
    ((SLdrPatternedAITypedef*)(arg0 + 60))->~SLdrPatternedAITypedef();
    ((SLdrEditorProperties*)arg0)->~SLdrEditorProperties();
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_100DC(int arg0) {
  int var_r30;
  int var_r30_2;
  int var_r30_3;
  if ((*(unsigned char*)(arg0 + 0xcd0)) == 1) {
    return arg0 + 3284;
  }
  if ((unsigned int)(*(int*)(arg0 + 0x980) - 0x800000) == 0) {
    fn_10_FFB4(arg0 + 4048, ((CPatterned*)arg0)->CPatterned::GetDamageVulnerability());
    var_r30 = 0;
    while (true) {
      if (var_r30 != 10) {
        CWeaponTypeVulnerability stack_20(lbl_10_rodata_3FC, (CWeaponTypeVulnerability::EEffect)0,
                                          false);
        ((CDamageVulnerability*)(arg0 + 4048))->SetVulnerability(var_r30, stack_20);
        goto block_6;
      } else {
      block_6:;
        var_r30 = var_r30 + 1;
        if (var_r30 >= 21) {
          break;
        } else {
          continue;
        }
      }
    }
    var_r30_2 = 0;
    do {
      CWeaponTypeVulnerability stack_14(lbl_10_rodata_3FC, (CWeaponTypeVulnerability::EEffect)0,
                                        false);
      ((CDamageVulnerability*)(arg0 + 4048))->SetChargedVulnerability(var_r30_2, stack_14);
      var_r30_2 = var_r30_2 + 1;
    } while (var_r30_2 < 4);
    var_r30_3 = 0;
    do {
      CWeaponTypeVulnerability stack_8(lbl_10_rodata_3FC, (CWeaponTypeVulnerability::EEffect)0,
                                       false);
      ((CDamageVulnerability*)(arg0 + 4048))->SetComboVulnerability(var_r30_3, stack_8);
      var_r30_3 = var_r30_3 + 1;
    } while (var_r30_3 < 4);
    return arg0 + 4048;
  } else {
    return (int)((CPatterned*)arg0)->CPatterned::GetDamageVulnerability();
  }
}

extern "C" int fn_10_14648(int, int);
extern "C" void fn_10_1A34C(int, int, int*, int*);
extern "C" void fn_10_1969C(int arg0, int arg1) {
  int fn_10_19918(int, int, int*, int);
  void fn_10_1A1D4(int, int, int*, int*);
  void fn_10_1A440(int, int, int*, int*);
  void fn_10_1A554(int, int, int*, int*);
  void fn_10_1A5BC(int, int, int*, int*);
  void fn_10_1A980(int, int*, int, int, int, int, int);
  int temp_r0;
  int var_r3;
  unsigned char* temp_r4;
  unsigned char* var_r5;
  unsigned char stack_10[104];
  int stack_c;
  int stack_8;
  *(int*)&stack_c = 0;
  *(int*)&stack_8 = 0;
  fn_10_1A5BC(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A554(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A440(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A34C(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A1D4(arg0, arg1, &stack_c, &stack_8);
  if ((unsigned char)fn_10_14648(arg0, arg1) == 0) {
    fn_10_1A980(arg0, &stack_8, 0, 0x20000, 0x10000, 65535, 65535);
  }
  if ((void*)(arg0 + 3156) != (&stack_c)) {
    var_r5 = stack_10;
    var_r3 = arg0 + 3160;
    temp_r4 = var_r5 + (*(int*)&stack_c << 2);
    while (var_r5 != temp_r4) {
      temp_r0 = *(int*)var_r5;
      var_r5 = (var_r5 + 0x4);
      *(int*)var_r3 = temp_r0;
      var_r3 = var_r3 + 4;
    }
    *(int*)(arg0 + 0xc54) = *(int*)&stack_c;
  }
  *(int*)(arg0 + 0xcb8) = *(int*)&stack_8;
  *(int*)(arg0 + 0xcbc) = fn_10_19918(arg0, arg1, &stack_c, *(int*)&stack_8);
}

extern "C" void fn_10_19A1C(int, int, int*, int*);
extern "C" void fn_10_19B64(int, int, int*, int*);
extern "C" void fn_10_197D8(int arg0, int arg1) {
  int fn_10_19918(int, int, int*, int);
  void fn_10_1A1D4(int, int, int*, int*);
  void fn_10_1A278(int, int, int*, int*);
  void fn_10_1A440(int, int, int*, int*);
  void fn_10_1A554(int, int, int*, int*);
  void fn_10_1A5BC(int, int, int*, int*);
  int temp_r0;
  int var_r3;
  unsigned char* temp_r4;
  unsigned char* var_r5;
  unsigned char stack_10[104];
  int stack_c;
  int stack_8;
  *(int*)&stack_c = 0;
  *(int*)&stack_8 = 0;
  fn_10_1A5BC(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A554(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A278(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A440(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A34C(arg0, arg1, &stack_c, &stack_8);
  fn_10_1A1D4(arg0, arg1, &stack_c, &stack_8);
  fn_10_19B64(arg0, arg1, &stack_c, &stack_8);
  fn_10_19A1C(arg0, arg1, &stack_c, &stack_8);
  if ((void*)(arg0 + 3156) != (&stack_c)) {
    var_r5 = stack_10;
    var_r3 = arg0 + 3160;
    temp_r4 = var_r5 + (*(int*)&stack_c << 2);
    while (var_r5 != temp_r4) {
      temp_r0 = *(int*)var_r5;
      var_r5 = (var_r5 + 0x4);
      *(int*)var_r3 = temp_r0;
      var_r3 = var_r3 + 4;
    }
    *(int*)(arg0 + 0xc54) = *(int*)&stack_c;
  }
  *(int*)(arg0 + 0xcb8) = *(int*)&stack_8;
  *(int*)(arg0 + 0xcbc) = fn_10_19918(arg0, arg1, &stack_c, *(int*)&stack_8);
}

extern float lbl_10_rodata_44C;
extern "C" void fn_10_1E178(int, int, float);
extern "C" void fn_10_10710(int arg0, int arg1, int arg2, float arg3) {
  void fn_10_16B64(int, int, float, float);
  int fn_10_17E14(int, int);
  int temp_r31;
  float temp_f1;
  float temp_f1_2;
  unsigned char stack_8[28];
  fn_10_1B478(arg0, 1024, arg1, arg2);
  if (!arg2) {
    *(float*)(arg0 + 0xc30) = *(float*)(arg0 + 0x8b0);
    if ((unsigned int)fn_10_17E14(arg0, arg1) != 0) {
      temp_r31 = fn_10_17E14(arg0, arg1);
      temp_f1 = *(float*)(fn_10_17E14(arg0, arg1) + 0x18);
      temp_f1_2 = ((CRandom16*)(arg1 + 5860))->Range(temp_f1, *(float*)(temp_r31 + 0x1c));
      *(float*)(arg0 + 0xc30) = *(float*)(arg0 + 0xc30) + temp_f1_2;
    }
  } else if (arg2 == 2) {
    *(unsigned short*)(arg0 + 0xc24) = kInvalidUniqueId.value;
  }
  fn_10_1E178(arg0, arg2, arg3);
  fn_10_16B64(arg0, arg1, arg3, lbl_10_rodata_44C);
  float temp_f1_3 = CVector3f::sZeroVector.GetX();
  float temp_f2 = CVector3f::sZeroVector.GetY();
  float temp_f3 = CVector3f::sZeroVector.GetZ();
  *(float*)stack_8 = temp_f1_3;
  *(float*)(stack_8 + 0x4) = temp_f2;
  *(float*)(stack_8 + 0x8) = temp_f3;
  *(float*)(stack_8 + 0xc) = temp_f1_3;
  *(float*)(stack_8 + 0x10) = temp_f2;
  *(float*)(stack_8 + 0x14) = temp_f3;
  *(float*)(stack_8 + 0x18) = 1.0f;
  ((CBodyStateCmdMgr*)(*(int*)(arg0 + 0x48c) + 4))->DeliverCmd(*(const CBCLocomotionCmd*)stack_8);
}

struct __mwdec_vt_0_fn_10_11158 {
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
  virtual int _128();
};
struct __mwdec_vt_1_fn_10_11158 {
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
  virtual int _91(int, int);
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
  virtual int _95(int, int);
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
  virtual int _81(int, int);
};
extern "C" int fn_10_11158(int arg0, int arg1, int arg2) {
  if ((unsigned char)((__mwdec_vt_0_fn_10_11158*)arg0)->_128() == 1) {
    return 1;
  }
  if ((unsigned char)((__mwdec_vt_1_fn_10_11158*)arg0)->_91(arg1, arg2) == 1 &&
      (unsigned char)((__mwdec_vt_2*)arg0)->_95(arg1, arg2) == 1 &&
      (unsigned char)((__mwdec_vt_3*)arg0)->_81(arg1, arg2) == 1) {
    switch (*(int*)(arg0 + 0x980)) {
    case 8:
    case 64:
    case 512:
    case 1024:
    case 65536:
    case 2097152:
      return 1;
    case 65535:
      return *(unsigned char*)(arg0 + 0xdec);
    }
  }
  return 0;
}

struct __mwdec_vt_0_fn_10_18964 {
  virtual void _0();
  virtual void _1();
  virtual void _2();
};
extern "C" void fn_10_18238(int, int);
extern "C" void fn_10_183E8(int);
extern "C" int fn_10_9624(int);
extern "C" void fn_10_18964(int arg0, int arg1) {
  int fn_10_17E14(int, int);
  float fn_10_185CC(int);
  void fn_10_1890C(int);
  int temp_r3;
  CPlayerState::EPlayerVisor temp_r31 =
      ((CPlayerState*)*(int*)(arg1 + 0x15fc))->GetActiveVisor(*(const CStateManager*)arg1);
  if ((unsigned int)fn_10_17E14(arg0, arg1) == 0 ||
      (*(unsigned char*)(fn_10_17E14(arg0, arg1) + 0x33)) != 1 ||
      CGX::ShiftRightAndMask((uint) * (unsigned char*)(arg0 + 0xded), 1, 7) != 1) {
    if ((unsigned char)fn_10_9624(arg0) == 0) {
      if (*(float*)(arg0 + 0xde8) > 0.0f || temp_r31 == 3 || temp_r31 == 1) {
        if (*(float*)(arg0 + 0xd14) > 0.0f && !temp_r31 &&
            CGX::ShiftRightAndMask((uint) * ((char*)arg0 + 0x90c), 1, 5)) {
          fn_10_18238(arg0, arg1);
        } else {
          ((CPatterned*)arg0)->CPatterned::Render(*(const CStateManager*)arg1);
        }
      }
      fn_10_183E8(arg0);
    }
    temp_r3 = *(int*)(arg0 + 0xe64);
    if ((unsigned int)temp_r3 != 0) {
      ((__mwdec_vt_0_fn_10_18964*)temp_r3)->_2();
    }
  }
  if (fn_10_185CC(arg0) > 0.0f &&
      ((CPlayerState*)*(int*)(arg1 + 0x15fc))->GetActiveVisor(*(const CStateManager*)arg1) == 3) {
    fn_10_1890C(arg0);
  }
}

extern "C" void fn_10_10484(int obj, int obj2) {
  float f4;
  float f2 = *(float*)((char*)obj + 0xfcc);
  if (f2 > 0.0f) {
    float f5 = *(float*)((char*)obj + 0x8b0);
    if (5.3f + f2 > f5) {
      float f3 = lbl_10_rodata_3E8;
      float f = f3 - (f5 - f2) / 5.3f;
      f4 = sin(3.1415927f * (f3 - f * f));
      float f6 = 12.0f * f4;
      *(float*)((char*)obj2 + 0x8) += f6;
    }
  }
}

extern "C" void fn_10_15348(int obj) {
  *(float*)((char*)obj + 0x4) = CVector3f::sZeroVector.GetX();
  *(float*)((char*)obj + 0x8) = CVector3f::sZeroVector.GetY();
  *(float*)((char*)obj + 0xc) = CVector3f::sZeroVector.GetZ();
  *(float*)((char*)obj + 0x10) = CVector3f::sZeroVector.GetX();
  *(float*)((char*)obj + 0x14) = CVector3f::sZeroVector.GetY();
  *(float*)((char*)obj + 0x18) = CVector3f::sZeroVector.GetZ();
  *(int*)((char*)obj + 0x24) = 0;
  *(unsigned char*)((char*)obj + 0x28) =
      __rlwimi(*(unsigned char*)((char*)obj + 0x28), 0, 6, 25, 25);
  *(float*)((char*)obj + 0x10) = CVector3f::sZeroVector.GetX();
  *(float*)((char*)obj + 0x14) = CVector3f::sZeroVector.GetY();
  *(float*)((char*)obj + 0x18) = CVector3f::sZeroVector.GetZ();
  *(float*)((char*)obj + 0x4) = *(float*)((char*)obj + 0x10);
  *(float*)((char*)obj + 0x8) = *(float*)((char*)obj + 0x14);
  *(float*)((char*)obj + 0xc) = *(float*)((char*)obj + 0x18);
  *(int*)((char*)obj + 0x1c) = -1;
  *(int*)obj = -1;
  *(float*)((char*)obj + 0x20) = 0.0f;
  *(unsigned char*)((char*)obj + 0x28) =
      __rlwimi(*(unsigned char*)((char*)obj + 0x28), 0, 7, 24, 24);
}
