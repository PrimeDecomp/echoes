#ifndef _SLDRTWEAKTARGETING
#define _SLDRTWEAKTARGETING

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTIcon_Configurations.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Charge_Gauge.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockDagger.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_LockFire.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_OuterBeamIcon.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_Scan.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakTargeting_VulnerabilityIndicator.hpp"
#include "rstl/string.hpp"

struct SLdrTweakTargeting {
    SLdrTweakTargeting();
    ~SLdrTweakTargeting();

    rstl::string instanceName;
    SLdrTweakTargeting_OuterBeamIcon unknown_0x5173932f;
    SLdrTIcon_Configurations unknown_0x23ff4be4;
    SLdrTIcon_Configurations unknown_0x92e98613;
    SLdrTIcon_Configurations unknown_0x64833842;
    SLdrTIcon_Configurations unknown_0x8dfd6e3c;
    SLdrTweakTargeting_Charge_Gauge chargeGauge;
    SLdrTweakTargeting_LockFire lockFire;
    SLdrTweakTargeting_LockDagger lockDagger;
    SLdrTweakTargeting_Scan scan;
    int unknown_0xc3410560;
    float unknown_0x3eb13041;
    float unknown_0x5e67cab0;
    float unknown_0xe0ca98ac;
    float lockOnConfirmReticleScale;
    float unknown_0x9f8d62c1;
    float unknown_0xff5eeeb9;
    float unknown_0x0d0b660d;
    float seekerTargetReticleScale;
    float unknown_0x03efc783;
    float unknown_0xdcbd7bf8;
    float unknown_0x2acd6b4b;
    float unknown_0xb4c6c331;
    float unknown_0x27d02089;
    float unknown_0xae89310e;
    float unknown_0xb27644df;
    float unknown_0x21e2d1cc;
    float orbitPointOccludedOpacity;
    float unknown_0x5c489cb5;
    float orbitPointZOffset;
    float unknown_0x61a6a38e;
    float unknown_0xfbdf31f9;
    float unknown_0xf76f7d0b;
    float unknown_0x810b3a08;
    float unknown_0x73fe1553;
    float unknown_0xc8aef6f2;
    float unknown_0x69b1e76c;
    float unknown_0x8a0dfd23;
    float unknown_0x8299e96e;
    float unknown_0xa18a3f25;
    float unknown_0x23be9bb2;
    float unknown_0x8d512b82;
    float unknown_0x39f1698d;
    float unknown_0xc768c1e9;
    float unknown_0x55c47b0e;
    float unknown_0xa009aea2;
    float unknown_0x38080bbf;
    float unknown_0x71017dbe;
    float unknown_0x4a996997;
    float unknown_0x9f9fa6f3;
    float unknown_0x932fea01;
    float unknown_0x165f0fa8;
    float unknown_0x6bd6b11f;
    CColor unknown_0x42420f6e;
    float flowerReticleScale;
    CColor flowerReticleColor;
    float unknown_0xb090e147;
    float unknown_0x4c73a43d;
    float unknown_0x6543d31b;
    float unknown_0x8cd2d1ce;
    CColor missileBracketColor;
    float unknown_0x45910e5d;
    float unknown_0x07b30fa0;
    CColor unknown_0x13ce8500;
    CColor unknown_0x9829f256;
    CColor unknown_0x77a613b9;
    CColor unknown_0xdfa81287;
    CColor lockOnConfirmReticleColor;
    CColor seekerReticleColor;
    float unknown_0x618d150a;
    float unknown_0x209a2a8c;
    float unknown_0xacb3f8f7;
    float unknown_0xeda4c771;
    float unknown_0xd3427574;
    float unknown_0x92554af2;
    float unknown_0x1e134e75;
    float unknown_0x5f0471f3;
    float unknown_0xe90548ac;
    float unknown_0xa812772a;
    float unknown_0x2d75c7be;
    float unknown_0x6c62f838;
    float unknown_0xf98e6242;
    float unknown_0xb8995dc4;
    float unknown_0x5009c614;
    float unknown_0x111ef992;
    float grappleIconScale;
    float grappleIconScaleInactive;
    float unknown_0x498d794a;
    float unknown_0x089a46cc;
    CColor grappleIconColor;
    CColor grappleIconColorInactive;
    CColor unknown_0x083b1cc8;
    float unknown_0x966982b1;
    CColor unknown_0xf9799f5f;
    float unknown_0x9b980788;
    CColor orbitPointModelColor;
    CColor crosshairsColor;
    float unknown_0x2ff52290;
    bool unknown_0x8a548cc9;
    CColor unknown_0x65d449e1;
    float unknown_0x42c7fbe4;
    float unknown_0x13820c03;
    float unknown_0x52953385;
    CColor xRaySeekerReticleColor;
    float unknown_0xdd8cf478;
    float unknown_0xcbac6d52;
    float unknown_0x980a75b6;
    float unknown_0xcd1e0e91;
    float unknown_0xce9cf241;
    float unknown_0xb6531e99;
    float unknown_0x77e5c6b5;
    float unknown_0xfef5668b;
    CColor healthColor;
    SLdrTweakTargeting_VulnerabilityIndicator powerVulnerabilityIndicator;
    SLdrTweakTargeting_VulnerabilityIndicator lightVulnerabilityIndicator;
    SLdrTweakTargeting_VulnerabilityIndicator darkVulnerabilityIndicator;
    SLdrTweakTargeting_VulnerabilityIndicator annihilatorVulnerabilityIndicator;
};

void LoadTypedefSLdrTweakTargeting(SLdrTweakTargeting&, CInputStream&);

/* {
  SLdrTweakTargeting sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0x5173932f:
      LoadTypedefSLdrTweakTargeting_OuterBeamIcon(sldrThis.unknown_0x5173932f, input);
      break;
    case 0x23ff4be4:
      LoadTypedefSLdrTIcon_Configurations(sldrThis.unknown_0x23ff4be4, input);
      break;
    case 0x92e98613:
      LoadTypedefSLdrTIcon_Configurations(sldrThis.unknown_0x92e98613, input);
      break;
    case 0x64833842:
      LoadTypedefSLdrTIcon_Configurations(sldrThis.unknown_0x64833842, input);
      break;
    case 0x8dfd6e3c:
      LoadTypedefSLdrTIcon_Configurations(sldrThis.unknown_0x8dfd6e3c, input);
      break;
    case 0xbc5a41b2:
      LoadTypedefSLdrTweakTargeting_Charge_Gauge(sldrThis.chargeGauge, input);
      break;
    case 0x00183589:
      LoadTypedefSLdrTweakTargeting_LockFire(sldrThis.lockFire, input);
      break;
    case 0xd20ecc07:
      LoadTypedefSLdrTweakTargeting_LockDagger(sldrThis.lockDagger, input);
      break;
    case 0x65ef9f2a:
      LoadTypedefSLdrTweakTargeting_Scan(sldrThis.scan, input);
      break;
    case 0xc3410560:
      sldrThis.unknown_0xc3410560 = input.ReadInt32();
      break;
    case 0x3eb13041:
      sldrThis.unknown_0x3eb13041 = input.ReadFloat();
      break;
    case 0x5e67cab0:
      sldrThis.unknown_0x5e67cab0 = input.ReadFloat();
      break;
    case 0xe0ca98ac:
      sldrThis.unknown_0xe0ca98ac = input.ReadFloat();
      break;
    case 0xc1e35abb:
      sldrThis.lockOnConfirmReticleScale = input.ReadFloat();
      break;
    case 0x9f8d62c1:
      sldrThis.unknown_0x9f8d62c1 = input.ReadFloat();
      break;
    case 0xff5eeeb9:
      sldrThis.unknown_0xff5eeeb9 = input.ReadFloat();
      break;
    case 0x0d0b660d:
      sldrThis.unknown_0x0d0b660d = input.ReadFloat();
      break;
    case 0x132dd09e:
      sldrThis.seekerTargetReticleScale = input.ReadFloat();
      break;
    case 0x03efc783:
      sldrThis.unknown_0x03efc783 = input.ReadFloat();
      break;
    case 0xdcbd7bf8:
      sldrThis.unknown_0xdcbd7bf8 = input.ReadFloat();
      break;
    case 0x2acd6b4b:
      sldrThis.unknown_0x2acd6b4b = input.ReadFloat();
      break;
    case 0xb4c6c331:
      sldrThis.unknown_0xb4c6c331 = input.ReadFloat();
      break;
    case 0x27d02089:
      sldrThis.unknown_0x27d02089 = input.ReadFloat();
      break;
    case 0xae89310e:
      sldrThis.unknown_0xae89310e = input.ReadFloat();
      break;
    case 0xb27644df:
      sldrThis.unknown_0xb27644df = input.ReadFloat();
      break;
    case 0x21e2d1cc:
      sldrThis.unknown_0x21e2d1cc = input.ReadFloat();
      break;
    case 0x8e3afef8:
      sldrThis.orbitPointOccludedOpacity = input.ReadFloat();
      break;
    case 0x5c489cb5:
      sldrThis.unknown_0x5c489cb5 = input.ReadFloat();
      break;
    case 0xef2c842e:
      sldrThis.orbitPointZOffset = input.ReadFloat();
      break;
    case 0x61a6a38e:
      sldrThis.unknown_0x61a6a38e = input.ReadFloat();
      break;
    case 0xfbdf31f9:
      sldrThis.unknown_0xfbdf31f9 = input.ReadFloat();
      break;
    case 0xf76f7d0b:
      sldrThis.unknown_0xf76f7d0b = input.ReadFloat();
      break;
    case 0x810b3a08:
      sldrThis.unknown_0x810b3a08 = input.ReadFloat();
      break;
    case 0x73fe1553:
      sldrThis.unknown_0x73fe1553 = input.ReadFloat();
      break;
    case 0xc8aef6f2:
      sldrThis.unknown_0xc8aef6f2 = input.ReadFloat();
      break;
    case 0x69b1e76c:
      sldrThis.unknown_0x69b1e76c = input.ReadFloat();
      break;
    case 0x8a0dfd23:
      sldrThis.unknown_0x8a0dfd23 = input.ReadFloat();
      break;
    case 0x8299e96e:
      sldrThis.unknown_0x8299e96e = input.ReadFloat();
      break;
    case 0xa18a3f25:
      sldrThis.unknown_0xa18a3f25 = input.ReadFloat();
      break;
    case 0x23be9bb2:
      sldrThis.unknown_0x23be9bb2 = input.ReadFloat();
      break;
    case 0x8d512b82:
      sldrThis.unknown_0x8d512b82 = input.ReadFloat();
      break;
    case 0x39f1698d:
      sldrThis.unknown_0x39f1698d = input.ReadFloat();
      break;
    case 0xc768c1e9:
      sldrThis.unknown_0xc768c1e9 = input.ReadFloat();
      break;
    case 0x55c47b0e:
      sldrThis.unknown_0x55c47b0e = input.ReadFloat();
      break;
    case 0xa009aea2:
      sldrThis.unknown_0xa009aea2 = input.ReadFloat();
      break;
    case 0x38080bbf:
      sldrThis.unknown_0x38080bbf = input.ReadFloat();
      break;
    case 0x71017dbe:
      sldrThis.unknown_0x71017dbe = input.ReadFloat();
      break;
    case 0x4a996997:
      sldrThis.unknown_0x4a996997 = input.ReadFloat();
      break;
    case 0x9f9fa6f3:
      sldrThis.unknown_0x9f9fa6f3 = input.ReadFloat();
      break;
    case 0x932fea01:
      sldrThis.unknown_0x932fea01 = input.ReadFloat();
      break;
    case 0x165f0fa8:
      sldrThis.unknown_0x165f0fa8 = input.ReadFloat();
      break;
    case 0x6bd6b11f:
      sldrThis.unknown_0x6bd6b11f = input.ReadFloat();
      break;
    case 0x42420f6e:
      // sldrThis.unknown_0x42420f6e = TODO
      break;
    case 0xa63229f1:
      sldrThis.flowerReticleScale = input.ReadFloat();
      break;
    case 0xbda45f1a:
      // sldrThis.flowerReticleColor = TODO
      break;
    case 0xb090e147:
      sldrThis.unknown_0xb090e147 = input.ReadFloat();
      break;
    case 0x4c73a43d:
      sldrThis.unknown_0x4c73a43d = input.ReadFloat();
      break;
    case 0x6543d31b:
      sldrThis.unknown_0x6543d31b = input.ReadFloat();
      break;
    case 0x8cd2d1ce:
      sldrThis.unknown_0x8cd2d1ce = input.ReadFloat();
      break;
    case 0xe42f6be0:
      // sldrThis.missileBracketColor = TODO
      break;
    case 0x45910e5d:
      sldrThis.unknown_0x45910e5d = input.ReadFloat();
      break;
    case 0x07b30fa0:
      sldrThis.unknown_0x07b30fa0 = input.ReadFloat();
      break;
    case 0x13ce8500:
      // sldrThis.unknown_0x13ce8500 = TODO
      break;
    case 0x9829f256:
      // sldrThis.unknown_0x9829f256 = TODO
      break;
    case 0x77a613b9:
      // sldrThis.unknown_0x77a613b9 = TODO
      break;
    case 0xdfa81287:
      // sldrThis.unknown_0xdfa81287 = TODO
      break;
    case 0xda752c50:
      // sldrThis.lockOnConfirmReticleColor = TODO
      break;
    case 0x7b544e14:
      // sldrThis.seekerReticleColor = TODO
      break;
    case 0x618d150a:
      sldrThis.unknown_0x618d150a = input.ReadFloat();
      break;
    case 0x209a2a8c:
      sldrThis.unknown_0x209a2a8c = input.ReadFloat();
      break;
    case 0xacb3f8f7:
      sldrThis.unknown_0xacb3f8f7 = input.ReadFloat();
      break;
    case 0xeda4c771:
      sldrThis.unknown_0xeda4c771 = input.ReadFloat();
      break;
    case 0xd3427574:
      sldrThis.unknown_0xd3427574 = input.ReadFloat();
      break;
    case 0x92554af2:
      sldrThis.unknown_0x92554af2 = input.ReadFloat();
      break;
    case 0x1e134e75:
      sldrThis.unknown_0x1e134e75 = input.ReadFloat();
      break;
    case 0x5f0471f3:
      sldrThis.unknown_0x5f0471f3 = input.ReadFloat();
      break;
    case 0xe90548ac:
      sldrThis.unknown_0xe90548ac = input.ReadFloat();
      break;
    case 0xa812772a:
      sldrThis.unknown_0xa812772a = input.ReadFloat();
      break;
    case 0x2d75c7be:
      sldrThis.unknown_0x2d75c7be = input.ReadFloat();
      break;
    case 0x6c62f838:
      sldrThis.unknown_0x6c62f838 = input.ReadFloat();
      break;
    case 0xf98e6242:
      sldrThis.unknown_0xf98e6242 = input.ReadFloat();
      break;
    case 0xb8995dc4:
      sldrThis.unknown_0xb8995dc4 = input.ReadFloat();
      break;
    case 0x5009c614:
      sldrThis.unknown_0x5009c614 = input.ReadFloat();
      break;
    case 0x111ef992:
      sldrThis.unknown_0x111ef992 = input.ReadFloat();
      break;
    case 0xec0ff888:
      sldrThis.grappleIconScale = input.ReadFloat();
      break;
    case 0xf229e05e:
      sldrThis.grappleIconScaleInactive = input.ReadFloat();
      break;
    case 0x498d794a:
      sldrThis.unknown_0x498d794a = input.ReadFloat();
      break;
    case 0x089a46cc:
      sldrThis.unknown_0x089a46cc = input.ReadFloat();
      break;
    case 0xf7998e63:
      // sldrThis.grappleIconColor = TODO
      break;
    case 0x3ce853e3:
      // sldrThis.grappleIconColorInactive = TODO
      break;
    case 0x083b1cc8:
      // sldrThis.unknown_0x083b1cc8 = TODO
      break;
    case 0x966982b1:
      sldrThis.unknown_0x966982b1 = input.ReadFloat();
      break;
    case 0xf9799f5f:
      // sldrThis.unknown_0xf9799f5f = TODO
      break;
    case 0x9b980788:
      sldrThis.unknown_0x9b980788 = input.ReadFloat();
      break;
    case 0x3523a47f:
      // sldrThis.orbitPointModelColor = TODO
      break;
    case 0x27358b4d:
      // sldrThis.crosshairsColor = TODO
      break;
    case 0x2ff52290:
      sldrThis.unknown_0x2ff52290 = input.ReadFloat();
      break;
    case 0x8a548cc9:
      sldrThis.unknown_0x8a548cc9 = input.ReadBool();
      break;
    case 0x65d449e1:
      // sldrThis.unknown_0x65d449e1 = TODO
      break;
    case 0x42c7fbe4:
      sldrThis.unknown_0x42c7fbe4 = input.ReadFloat();
      break;
    case 0x13820c03:
      sldrThis.unknown_0x13820c03 = input.ReadFloat();
      break;
    case 0x52953385:
      sldrThis.unknown_0x52953385 = input.ReadFloat();
      break;
    case 0x0cd8fe5d:
      // sldrThis.xRaySeekerReticleColor = TODO
      break;
    case 0xdd8cf478:
      sldrThis.unknown_0xdd8cf478 = input.ReadFloat();
      break;
    case 0xcbac6d52:
      sldrThis.unknown_0xcbac6d52 = input.ReadFloat();
      break;
    case 0x980a75b6:
      sldrThis.unknown_0x980a75b6 = input.ReadFloat();
      break;
    case 0xcd1e0e91:
      sldrThis.unknown_0xcd1e0e91 = input.ReadFloat();
      break;
    case 0xce9cf241:
      sldrThis.unknown_0xce9cf241 = input.ReadFloat();
      break;
    case 0xb6531e99:
      sldrThis.unknown_0xb6531e99 = input.ReadFloat();
      break;
    case 0x77e5c6b5:
      sldrThis.unknown_0x77e5c6b5 = input.ReadFloat();
      break;
    case 0xfef5668b:
      sldrThis.unknown_0xfef5668b = input.ReadFloat();
      break;
    case 0x5ce2a16f:
      // sldrThis.healthColor = TODO
      break;
    case 0x1e179603:
      LoadTypedefSLdrTweakTargeting_VulnerabilityIndicator(sldrThis.powerVulnerabilityIndicator, input);
      break;
    case 0x2b70cef8:
      LoadTypedefSLdrTweakTargeting_VulnerabilityIndicator(sldrThis.lightVulnerabilityIndicator, input);
      break;
    case 0x946cefde:
      LoadTypedefSLdrTweakTargeting_VulnerabilityIndicator(sldrThis.darkVulnerabilityIndicator, input);
      break;
    case 0x921c86e1:
      LoadTypedefSLdrTweakTargeting_VulnerabilityIndicator(sldrThis.annihilatorVulnerabilityIndicator, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKTARGETING
