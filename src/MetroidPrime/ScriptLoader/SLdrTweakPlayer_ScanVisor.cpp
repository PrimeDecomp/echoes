#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_ScanVisor.hpp"

SLdrTweakPlayer_ScanVisor::SLdrTweakPlayer_ScanVisor() {
  scanDistance = 50.0f;
  scanRetention = true;
  scanFreezesGame = true;
  scanLineOfSight = true;
  scanMaxTargetDistance = 100.0f;
  scanMaxLockDistance = 100.0f;
  scanCameraSpeed = 30.0f;
}

SLdrTweakPlayer_ScanVisor::~SLdrTweakPlayer_ScanVisor() {}

void LoadTypedefSLdrTweakPlayer_ScanVisor(SLdrTweakPlayer_ScanVisor& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0xb0a32e5b:
      sldrThis.scanDistance = input.ReadFloat();
      break;
    case 0x2a75f2b8:
      sldrThis.scanRetention = input.ReadBool();
      break;
    case 0x058284bb:
      sldrThis.scanFreezesGame = input.ReadBool();
      break;
    case 0x1e54f5ae:
      sldrThis.scanLineOfSight = input.ReadBool();
      break;
    case 0xadfa90fc:
      sldrThis.scanMaxTargetDistance = input.ReadFloat();
      break;
    case 0xf4db84a9:
      sldrThis.scanMaxLockDistance = input.ReadFloat();
      break;
    case 0x8a7b245f:
      sldrThis.scanCameraSpeed = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
