#ifndef _SLDRTWEAKCAMERABOB
#define _SLDRTWEAKCAMERABOB

#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/string.hpp"

struct SLdrTweakCameraBob {
    SLdrTweakCameraBob();
    ~SLdrTweakCameraBob();

    rstl::string instanceName;
    float cameraBobExtentX;
    float cameraBobExtentY;
    float cameraBobPeriod;
    float unknown_0xa27bb5a7;
    float unknown_0xe3580b2b;
    float slowSpeedPeriodScale;
    float targetMagnitudeTrackingRate;
    float landingBobSpringConstant;
    float viewWanderRadius;
    float viewWanderSpeedMin;
    float viewWanderSpeedMax;
    float viewWanderRollVariation;
    float gunBobMagnitude;
    float helmetBobMagnitude;
};

void LoadTypedefSLdrTweakCameraBob(SLdrTweakCameraBob&, CInputStream&);

/* {
  SLdrTweakCameraBob sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0xe2a0b6f1:
      sldrThis.cameraBobExtentX = input.ReadFloat();
      break;
    case 0x29fc6554:
      sldrThis.cameraBobExtentY = input.ReadFloat();
      break;
    case 0x149d7339:
      sldrThis.cameraBobPeriod = input.ReadFloat();
      break;
    case 0xa27bb5a7:
      sldrThis.unknown_0xa27bb5a7 = input.ReadFloat();
      break;
    case 0xe3580b2b:
      sldrThis.unknown_0xe3580b2b = input.ReadFloat();
      break;
    case 0xb05dade7:
      sldrThis.slowSpeedPeriodScale = input.ReadFloat();
      break;
    case 0x6dc5d440:
      sldrThis.targetMagnitudeTrackingRate = input.ReadFloat();
      break;
    case 0xd16539a7:
      sldrThis.landingBobSpringConstant = input.ReadFloat();
      break;
    case 0xadbb0a42:
      sldrThis.viewWanderRadius = input.ReadFloat();
      break;
    case 0xe7f8f11b:
      sldrThis.viewWanderSpeedMin = input.ReadFloat();
      break;
    case 0x01985efa:
      sldrThis.viewWanderSpeedMax = input.ReadFloat();
      break;
    case 0xef19ba33:
      sldrThis.viewWanderRollVariation = input.ReadFloat();
      break;
    case 0x7f59be96:
      sldrThis.gunBobMagnitude = input.ReadFloat();
      break;
    case 0x38a82ac1:
      sldrThis.helmetBobMagnitude = input.ReadFloat();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKCAMERABOB
