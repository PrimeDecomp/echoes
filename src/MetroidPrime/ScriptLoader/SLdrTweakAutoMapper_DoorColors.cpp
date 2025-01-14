#include "MetroidPrime/ScriptLoader/SLdrTweakAutoMapper_DoorColors.hpp"

SLdrTweakAutoMapper_DoorColors::SLdrTweakAutoMapper_DoorColors() {
  numDoorColors = 8;
  blueDoorColor = CColor::Black();
  missileDoorColor = CColor::Black();
  darkBeamDoorColor = CColor::Black();
  unknown = CColor::Black();
  annihilatorBeamDoorColor = CColor::Black();
  lightBeamDoorColor = CColor::Black();
  superMissileDoorColor = CColor::Black();
  seekerDoorColor = CColor::Black();
  powerBombDoorColor = CColor::Black();
  greyDoorColor = CColor::Black();
  whiteDoorColor = CColor::Black();
}

SLdrTweakAutoMapper_DoorColors::~SLdrTweakAutoMapper_DoorColors() {}

void LoadTypedefSLdrTweakAutoMapper_DoorColors(SLdrTweakAutoMapper_DoorColors& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x4fceb904:
      sldrThis.numDoorColors = input.ReadInt32();
      break;
    case 0xb36aa9b7:
      sldrThis.blueDoorColor = CColor(input);
      break;
    case 0x9620d4a0:
      sldrThis.missileDoorColor = CColor(input);
      break;
    case 0x4e1a9a8d:
      sldrThis.darkBeamDoorColor = CColor(input);
      break;
    case 0xce986453:
      sldrThis.unknown = CColor(input);
      break;
    case 0x070f36f9:
      sldrThis.annihilatorBeamDoorColor = CColor(input);
      break;
    case 0xfada14b6:
      sldrThis.lightBeamDoorColor = CColor(input);
      break;
    case 0x172241dc:
      sldrThis.superMissileDoorColor = CColor(input);
      break;
    case 0x518bccdc:
      sldrThis.seekerDoorColor = CColor(input);
      break;
    case 0x2853fa91:
      sldrThis.powerBombDoorColor = CColor(input);
      break;
    case 0xe655a18e:
      sldrThis.greyDoorColor = CColor(input);
      break;
    case 0x0f08f35e:
      sldrThis.whiteDoorColor = CColor(input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
