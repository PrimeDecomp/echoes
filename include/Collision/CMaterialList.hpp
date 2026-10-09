#ifndef _CMATERIALLIST
#define _CMATERIALLIST

#include "rstl/construct.hpp"
#include "types.h"

class CInputStream;

enum EMaterialTypes {
  kMT_NoStepLogic = 0,
  kMT_Stone = 1,
  kMT_Metal = 2,
  kMT_Grass = 3,
  kMT_Ice = 4,
  kMT_Pillar = 5,
  kMT_MetalGrating = 6,
  kMT_Phazon = 7,
  kMT_Dirt = 8,
  kMT_Unknown9 = 9,   // Guessed name; footstep sound set "dgrass".
  kMT_Unknown10 = 10, // Guessed name; footstep sound set "dwal".
  kMT_Snow = 11,
  kMT_Fabric = 12, // Guessed name; footstep sound set "fabr".
  kMT_HalfPipe = 13,
  kMT_Plastic = 14, // Guessed name; footstep sound set "plas".
  kMT_Wire = 15,    // Guessed name; footstep sound set "wire".
  kMT_Shield = 16,
  kMT_Sand = 17,
  kMT_Unknown18 = 18, // Surface type with a CRagDoll material response; footstep sound set "moth".
  kMT_Web = 19,       // Guessed name; footstep sound set "web", CRagDoll material response.
  kMT_ProjectilePassthrough = 20,
  kMT_CameraPassthrough = 21,
  kMT_Wood = 22,
  kMT_Organic = 23,
  kMT_NoEdgeCollision = 24,
  kMT_Rubber = 25, // Guessed name; footstep sound set "rubb", CRagDoll restitution of 5.
  kMT_SeeThrough = 26,
  kMT_ScanPassthrough = 27,
  kMT_AIPassthrough = 28,
  kMT_Ceiling = 29,
  kMT_Wall = 30,
  kMT_Floor = 31,
  kMT_Player = 32,
  kMT_Character = 33,
  kMT_Trigger = 34,
  kMT_Projectile = 35,
  kMT_Bomb = 36,
  kMT_GroundCollider = 37,
  kMT_NoStaticCollision = 38,
  kMT_Scannable = 39,
  kMT_Target = 40,
  kMT_Orbit = 41,
  kMT_Occluder = 42,
  kMT_Immovable = 43,
  kMT_Debris = 44,
  kMT_PowerBomb = 45,
  kMT_Unknown46 = 46,
  kMT_CollisionActor = 47,
  kMT_AIBlock = 48,
  kMT_Platform = 49,
  kMT_NonSolidDamageable = 50,
  kMT_RadarObject = 51,
  kMT_PlatformSlave = 52,
  kMT_AIJoint = 53,
  kMT_Unknown54 = 54,
  kMT_SolidCharacter = 55,
  kMT_ExcludeFromLineOfSightTest = 56,
  kMT_ExcludeFromRadar = 57,
  kMT_NoPlayerCollision = 58,
  kMT_Solid = 59,
  kMT_NoPlatformCollision = 60,
  kMT_Unknown61 = 61,   // Included by the spider ball surface filter.
  kMT_SeekerTarget = 63 // Target-derived name: seeker lock-on eligibility.
};

class CMaterialList {
public:
  CMaterialList() : mValue(0) {}
  explicit CMaterialList(const EMaterialTypes& m1) : mValue(0) { Add(m1); }
  CMaterialList(const EMaterialTypes& m1, const EMaterialTypes& m2) : mValue(0) {
    Add(m1);
    Add(m2);
  }
  CMaterialList(const EMaterialTypes& m1, const EMaterialTypes& m2, const EMaterialTypes& m3)
  : mValue(0) {
    Add(m1);
    Add(m2);
    Add(m3);
  }
  CMaterialList(const EMaterialTypes& m1, const EMaterialTypes& m2, const EMaterialTypes& m3,
                const EMaterialTypes& m4)
  : mValue(0) {
    Add(m1);
    Add(m2);
    Add(m3);
    Add(m4);
  }
  CMaterialList(const EMaterialTypes& m1, const EMaterialTypes& m2, const EMaterialTypes& m3,
                const EMaterialTypes& m4, const EMaterialTypes& m5)
  : mValue(0) {
    Add(m1);
    Add(m2);
    Add(m3);
    Add(m4);
    Add(m5);
  }
  CMaterialList(const EMaterialTypes& m1, const EMaterialTypes& m2, const EMaterialTypes& m3,
                const EMaterialTypes& m4, const EMaterialTypes& m5, const EMaterialTypes& m6)
  : mValue(0) {
    Add(m1);
    Add(m2);
    Add(m3);
    Add(m4);
    Add(m5);
    Add(m6);
  }
  explicit CMaterialList(u64 value) : mValue(value) {}
  // Guessed identity: adjacent to BitPosition; consumes one aligned 64-bit value.
  explicit CMaterialList(CInputStream& in);
  u64 GetValue() const { return mValue; }

  void Add(EMaterialTypes material) { mValue |= u64(1) << material; }
  void Add(const CMaterialList& material) { mValue |= material.mValue; }
  void Remove(EMaterialTypes material) { mValue &= ~(u64(1) << material); }
  void Remove(const CMaterialList& material) { mValue &= ~material.mValue; }
  CMaterialList Union(const CMaterialList& other) const {
    return CMaterialList(mValue | other.mValue);
  }
  bool HasMaterial(EMaterialTypes material) const {
    return (mValue & (u64(1) << material)) ? true : false;
  }
  // HasMaterials__13CMaterialListCFv weak
  // GetField__13CMaterialListCFUxUx weak
  // Intersection__13CMaterialListCFRC13CMaterialList weak
  static int BitPosition(u64 flags);
  // GetMaterialString__13CMaterialListCFv weak
  bool SharesMaterials(const CMaterialList& other) const {
    return (other.mValue & mValue) ? true : false;
  }

private:
  u64 mValue;

  // static CMaterialList kEverything;
};
CHECK_SIZEOF(CMaterialList, 0x8)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(CMaterialList)
}

#endif // _CMATERIALLIST
