#include "MetroidPrime/ScriptLoader/SLdrTWeaponDamage.hpp"

SLdrTWeaponDamage::SLdrTWeaponDamage() {}

SLdrTWeaponDamage::~SLdrTWeaponDamage() {}

void LoadTypedefSLdrTWeaponDamage(SLdrTWeaponDamage& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x8ac4278a:
      LoadTypedefSLdrTDamageInfo(sldrThis.normal, input);
      break;
    case 0xc9ac01d2:
      LoadTypedefSLdrTDamageInfo(sldrThis.charged, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
