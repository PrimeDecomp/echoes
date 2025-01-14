#include "MetroidPrime/ScriptLoader/SLdrTDamageInfo.hpp"

SLdrTDamageInfo::SLdrTDamageInfo() {
  weaponType = 0;
  damageAmount = 10.0f;
  radiusDamageAmount = 5.0f;
  damageRadius = 1.0f;
  knockBackPower = 1.0f;
}

SLdrTDamageInfo::~SLdrTDamageInfo() {}

void LoadTypedefSLdrTDamageInfo(SLdrTDamageInfo& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x4d577910:
      sldrThis.weaponType = input.ReadInt32();
      break;
    case 0xf3ec8748:
      sldrThis.damageAmount = input.ReadFloat();
      break;
    case 0x37b6df3d:
      sldrThis.radiusDamageAmount = input.ReadFloat();
      break;
    case 0x0f598739:
      sldrThis.damageRadius = input.ReadFloat();
      break;
    case 0x56f98c49:
      sldrThis.knockBackPower = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
