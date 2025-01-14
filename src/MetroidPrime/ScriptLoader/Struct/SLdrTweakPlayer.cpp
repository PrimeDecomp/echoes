#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakPlayer.hpp"

SLdrTweakPlayer::SLdrTweakPlayer() {}

SLdrTweakPlayer::~SLdrTweakPlayer() {}

void LoadTypedefSLdrTweakPlayer(SLdrTweakPlayer& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0xdfd08eba:
      LoadTypedefSLdrTweakPlayer_DarkWorld(sldrThis.darkWorld, input);
      break;
    case 0x45171a96:
      LoadTypedefSLdrTweakPlayer_GrappleBeam(sldrThis.grappleBeam, input);
      break;
    case 0x82cf4cf1:
      LoadTypedefSLdrTweakPlayer_Motion(sldrThis.motion, input);
      break;
    case 0x56a720c8:
      LoadTypedefSLdrTweakPlayer_Misc(sldrThis.misc, input);
      break;
    case 0x42a17438:
      LoadTypedefSLdrTweakPlayer_AimStuff(sldrThis.aimStuff, input);
      break;
    case 0x243ae038:
      LoadTypedefSLdrTweakPlayer_Orbit(sldrThis.orbit, input);
      break;
    case 0x20124c3d:
      LoadTypedefSLdrTweakPlayer_ScanVisor(sldrThis.scanVisor, input);
      break;
    case 0x30412440:
      LoadTypedefSLdrTweakPlayer_Grapple(sldrThis.grapple, input);
      break;
    case 0xc4d32ae5:
      LoadTypedefSLdrTweakPlayer_Collision(sldrThis.collision, input);
      break;
    case 0xd6155d4b:
      LoadTypedefSLdrTweakPlayer_FirstPersonCamera(sldrThis.firstPersonCamera, input);
      break;
    case 0xbcca767e:
      LoadTypedefSLdrTweakPlayer_Shield(sldrThis.shield, input);
      break;
    case 0x4d3b20b7:
      LoadTypedefSLdrTweakPlayer_Frozen(sldrThis.frozen, input);
      break;
    case 0xaeaff210:
      LoadTypedefSLdrTweakPlayer_SuitDamageReduction(sldrThis.suitDamageReduction, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
