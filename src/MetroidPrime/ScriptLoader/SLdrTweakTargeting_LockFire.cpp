#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockFire.hpp"

SLdrTweakTargeting_LockFire::SLdrTweakTargeting_LockFire() {
  lockFireReticleScale = 1.0f;
  lockFireAnimTime = 0.30000001f;
  lockFireColor = CColor::Black();
}

SLdrTweakTargeting_LockFire::~SLdrTweakTargeting_LockFire() {}

void LoadTypedefSLdrTweakTargeting_LockFire(SLdrTweakTargeting_LockFire& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xd4e59e59:
      sldrThis.lockFireReticleScale = input.ReadFloat();
      break;
    case 0xb79de7e8:
      sldrThis.lockFireAnimTime = input.ReadFloat();
      break;
    case 0xf5e9899f:
      sldrThis.lockFireColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
