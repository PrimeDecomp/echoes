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

extern "C" bool fn_10_4AA8(int arg0) { return *(int*)(arg0 + 0xef8) != 0; }

extern "C" bool fn_10_4ABC(int arg0) {
  return CGX::ShiftRightAndMask((uint) * ((char*)arg0 + 0xf5c), 1, 7);
}

extern "C" int fn_10_8680(int arg0) { return (*(int*)(arg0 + 0x984) == 0x400000) ? 1 : 0; }

extern "C" int fn_10_8114(int arg0) {
  if (*(int*)(arg0 + 0x984) == 0x800000) {
    return 1;
  }
  return 0;
}

extern "C" void fn_10_4E6C();
extern "C" void fn_10_4E4C() { fn_10_4E6C(); }

extern "C" void fn_10_4FF0() {
  void fn_10_5010();
  fn_10_5010();
}

extern float lbl_10_rodata_13C;
extern "C" bool fn_10_6A40(int arg0) { return *(float*)(arg0 + 0xfb0) <= lbl_10_rodata_13C; }

extern "C" int fn_10_CDA0(int arg0) {
  if (*((int*)(arg0 + 0x984)) == 0x10000) {
    return 1;
  }
  return 0;
}

extern "C" bool fn_10_9624(int arg0) {
  return *(unsigned short*)(arg0 + 0xe38) != kInvalidUniqueId.value;
}

extern "C" int fn_10_1AA6C();
extern "C" int fn_10_6B78() { return (fn_10_1AA6C() == 0) ? 1 : 0; }

extern int lbl_10_data_1D4;
extern "C" int fn_10_4BF8(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_1D4;
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_4EFC(int, unsigned char*, unsigned char*, unsigned char*);
extern "C" void fn_10_4EB4(int arg0) {
  void fn_10_4DF0(unsigned char*, int);
  unsigned char stack_24[28];
  unsigned char stack_14[16];
  unsigned char stack_8[12];
  *(unsigned char*)(stack_24 + 0x14) = 0;
  *(unsigned char*)(stack_14 + 0xc) = 0;
  *(unsigned char*)(stack_8 + 0x8) = 0;
  fn_10_4EFC(arg0, stack_24, stack_14, stack_8);
  fn_10_4DF0(stack_24, -1);
}

extern "C" int fn_10_B670(int arg0, int arg1) {
  if (arg0) {
    CMemory::Free((const void*)*(int*)(arg0 + 0xc));
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_1C8;
extern "C" int fn_10_4B9C(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_1C8;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_1BC;
extern "C" int fn_10_6964(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_1BC;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern int lbl_10_data_1B0;
extern "C" int fn_10_8624(int arg0, int arg1) {
  if (arg0) {
    *(int*)arg0 = (int)&lbl_10_data_1B0;
    if (arg0) {
      *(int*)arg0 = (int)&lbl_10_data_1D4;
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" void fn_10_812C();
extern "C" void fn_10_8178(int arg0) {
  fn_10_812C();
  *(int*)(arg0 + 0xf60) = *(unsigned int*)&CSfxManager::AddEmitter(
      *(unsigned short*)(arg0 + 0xf5e), *(const CVector3f*)(arg0 + 84), 127, *(int*)(arg0 + 0x4),
      false, false, CSfxManager::kMedPriority);
}

extern float lbl_10_rodata_25C;
extern float lbl_10_rodata_278;
extern "C" void fn_10_A154(int arg0, float arg1) {
  float temp_f4 = *(float*)(arg0 + 0x54);
  float temp_f2 = *(float*)(arg0 + 0x58) - *(float*)(arg0 + 0xe9c);
  float temp_f3 = lbl_10_rodata_25C;
  float temp_f5 = temp_f4 - *(float*)(arg0 + 0xe98);
  if (temp_f3 + (temp_f5 * temp_f5 + temp_f2 * temp_f2) > lbl_10_rodata_278) {
    *(float*)(arg0 + 0xe98) = temp_f4;
    *(float*)(arg0 + 0xe9c) = *(float*)(arg0 + 0x58);
    *(float*)(arg0 + 0xea0) = *(float*)(arg0 + 0x5c);
    *(float*)(arg0 + 0xea4) = temp_f3;
  } else {
    *(float*)(arg0 + 0xea4) = *(float*)(arg0 + 0xea4) + arg1;
  }
}

extern float lbl_10_bss_128;
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
extern "C" void fn_10_6DAC(int arg0, int arg1) {
  if (arg1 == 2) {
    *(unsigned char*)(arg0 + 0xcd0) = 0;
  } else if (*(float*)(((__mwdec_vt_0*)arg0)->_13() + 0x4) <= 1.0f) {
    *(float*)(arg0 + 0xd24) = 1.0f;
    *(unsigned char*)(arg0 + 0xcd0) = 1;
    *(float*)(arg0 + 0xd18) = lbl_10_bss_128;
    *(float*)(arg0 + 0xd1c) = *(float*)((char*)&(*(int*)&lbl_10_bss_128) + 0x4);
    *(float*)(arg0 + 0xd20) = *(float*)((char*)&(*(int*)&lbl_10_bss_128) + 0x8);
  }
}

extern "C" void fn_10_5038();
extern "C" void fn_10_5010(int arg0) {
  if ((unsigned int)arg0 != 0) {
    fn_10_5038();
  }
}

extern "C" int fn_10_4D9C(int arg0, int arg1) {
  void fn_10_4DF0(int, int);
  if (arg0) {
    fn_10_4DF0(arg0, -1);
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}

extern "C" int fn_10_4DF0(int arg0, int arg1) {
  void fn_10_4E4C();
  if (arg0) {
    if ((*(unsigned char*)(arg0 + 0x14))) {
      fn_10_4E4C();
    }
    if ((short)arg1 > 0) {
      CMemory::Free((const void*)arg0);
    }
  }
  return arg0;
}
