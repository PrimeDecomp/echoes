#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_OuterBeamIcon.hpp"
#include "Kyoto/Graphics/CColor.hpp"

SLdrTweakTargeting_OuterBeamIcon::SLdrTweakTargeting_OuterBeamIcon()
: butSettingsScale(CColor::Green())
{
  unknown_0x383e2b2d = 1.1f;
  unknown_0xeaac42d0 = 0.80000001f;
  butSettingsColor = 1.0f;
  butSettingsScale = CColor(0.29411799f, 0.494118f, 0.639216f, 0.627451f);
  unknown_0xe7d57d6a = 4;
}

SLdrTweakTargeting_OuterBeamIcon::~SLdrTweakTargeting_OuterBeamIcon() {}

void LoadTypedefSLdrTweakTargeting_OuterBeamIcon(SLdrTweakTargeting_OuterBeamIcon& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x383e2b2d:
      sldrThis.unknown_0x383e2b2d = input.ReadFloat();
      break;
    case 0xeaac42d0:
      sldrThis.unknown_0xeaac42d0 = input.ReadFloat();
      break;
    case 0x4967a63c:
      sldrThis.butSettingsColor = input.ReadFloat();
      break;
    case 0x52f1d0d7:
      sldrThis.butSettingsScale = CColor(input);
      break;
    case 0xe7d57d6a:
      sldrThis.unknown_0xe7d57d6a = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
