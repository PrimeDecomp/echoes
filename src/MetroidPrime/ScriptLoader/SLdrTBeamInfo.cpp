#include "MetroidPrime/ScriptLoader/SLdrTBeamInfo.hpp"

SLdrTBeamInfo::SLdrTBeamInfo() {
  cooldown = 0.2f;
}

SLdrTBeamInfo::~SLdrTBeamInfo() {}

void LoadTypedefSLdrTBeamInfo(SLdrTBeamInfo& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x102e085f:
      sldrThis.cooldown = input.ReadFloat();
      break;
    case 0xfaa71e25:
      LoadTypedefSLdrTWeaponDamage(sldrThis.damageInfo, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
