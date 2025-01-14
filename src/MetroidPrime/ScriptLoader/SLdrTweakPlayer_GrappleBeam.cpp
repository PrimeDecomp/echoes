#include "MetroidPrime/ScriptLoader/SLdrTweakPlayer_GrappleBeam.hpp"

SLdrTweakPlayer_GrappleBeam::SLdrTweakPlayer_GrappleBeam() {
  travelSpeed = 30.0f;
  xWaveAmplitude = 0.25f;
  zWaveAmplitude = 0.125f;
  anglePhaseDelta = 0.875f;
}

SLdrTweakPlayer_GrappleBeam::~SLdrTweakPlayer_GrappleBeam() {}

void LoadTypedefSLdrTweakPlayer_GrappleBeam(SLdrTweakPlayer_GrappleBeam& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x14849367:
      sldrThis.travelSpeed = input.ReadFloat();
      break;
    case 0x6c6e6f3c:
      sldrThis.xWaveAmplitude = input.ReadFloat();
      break;
    case 0x88aa6e41:
      sldrThis.zWaveAmplitude = input.ReadFloat();
      break;
    case 0x2aab8dda:
      sldrThis.anglePhaseDelta = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
