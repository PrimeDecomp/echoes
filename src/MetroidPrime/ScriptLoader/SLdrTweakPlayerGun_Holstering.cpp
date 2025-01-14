#include "MetroidPrime/ScriptLoader/SLdrTweakPlayerGun_Holstering.hpp"

SLdrTweakPlayerGun_Holstering::SLdrTweakPlayerGun_Holstering() {
  gunHolsterTime = 0.15000001f;
  gunNotFiringTime = 30.0f;
  gunHolsteredAngle = 40.0f;
}

SLdrTweakPlayerGun_Holstering::~SLdrTweakPlayerGun_Holstering() {}

void LoadTypedefSLdrTweakPlayerGun_Holstering(SLdrTweakPlayerGun_Holstering& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7ee98ebb:
      sldrThis.gunHolsterTime = input.ReadFloat();
      break;
    case 0xec515cd5:
      sldrThis.gunNotFiringTime = input.ReadFloat();
      break;
    case 0x0448573f:
      sldrThis.gunHolsteredAngle = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
