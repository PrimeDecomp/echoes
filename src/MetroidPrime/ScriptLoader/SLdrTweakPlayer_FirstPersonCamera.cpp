#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_FirstPersonCamera.hpp"

SLdrTweakPlayer_FirstPersonCamera::SLdrTweakPlayer_FirstPersonCamera() {
  unknown_0xba5eb7f5 = 60.0f;
  cameraElevation = 0.0f;
  unknown_0xb400ebd6 = 0.1f;
  unknown_0xfd26b7b9 = 4.0f;
  unknown_0x97b14dc6 = 73.0f;
  unknown_0xeb59925a = 0.30000001f;
  unknown_0xa1d73380 = 4.0f;
  unknown_0xc8e8344a = 73.0f;
  unknown_0xd40c480e = 10.0f;
}

SLdrTweakPlayer_FirstPersonCamera::~SLdrTweakPlayer_FirstPersonCamera() {}

void LoadTypedefSLdrTweakPlayer_FirstPersonCamera(SLdrTweakPlayer_FirstPersonCamera& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xba5eb7f5:
      sldrThis.unknown_0xba5eb7f5 = input.ReadFloat();
      break;
    case 0x0d41e3ba:
      sldrThis.cameraElevation = input.ReadFloat();
      break;
    case 0xb400ebd6:
      sldrThis.unknown_0xb400ebd6 = input.ReadFloat();
      break;
    case 0xfd26b7b9:
      sldrThis.unknown_0xfd26b7b9 = input.ReadFloat();
      break;
    case 0x97b14dc6:
      sldrThis.unknown_0x97b14dc6 = input.ReadFloat();
      break;
    case 0xeb59925a:
      sldrThis.unknown_0xeb59925a = input.ReadFloat();
      break;
    case 0xa1d73380:
      sldrThis.unknown_0xa1d73380 = input.ReadFloat();
      break;
    case 0xc8e8344a:
      sldrThis.unknown_0xc8e8344a = input.ReadFloat();
      break;
    case 0xd40c480e:
      sldrThis.unknown_0xd40c480e = input.ReadFloat();
      break;
    case 0x7960c3a0:
      // sldrThis.unknown_0x7960c3a0 = TODO
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
