#ifndef _SLDRTWEAKGAME
#define _SLDRTWEAKGAME

#include "Kyoto/Math/CMayaSpline.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_CoinLimitChoices.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_FragLimitChoices.hpp"
#include "MetroidPrime/ScriptLoader/SLdrTweakGame_TimeLimitChoices.hpp"
#include "rstl/string.hpp"

struct SLdrTweakGame {
    SLdrTweakGame();
    ~SLdrTweakGame();

    rstl::string instanceName;
    rstl::string pakFile;
    rstl::string asset;
    float fieldofView;
    float fieldofView2Player;
    bool disableDebugMenu;
    bool unknown_0x7262d27b;
    bool developmentMode;
    float unknown_0xa3dcf42a;
    float unknown_0xb35c72be;
    float unknown_0x4a02103c;
    float unknown_0xe1fca71b;
    float unknown_0xfbce966a;
    float unknown_0x09c6ca10;
    float hardModeDamageMultiplier;
    float hardModeWeaponMultiplier;
    float unknown_0x5ab5812c;
    float unknown_0x53401390;
    int totalPercentage;
    SLdrTweakGame_FragLimitChoices unknown_0x1d627808;
    SLdrTweakGame_TimeLimitChoices unknown_0xb2e8828d;
    SLdrTweakGame_CoinLimitChoices unknown_0x06af87bd;
    SLdrTweakGame_TimeLimitChoices unknown_0x1533ea4e;
    SLdrSpline unknown_0x40818220;
};

void LoadTypedefSLdrTweakGame(SLdrTweakGame&, CInputStream&);

/* {
  SLdrTweakGame sldrThis;
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      // sldrThis.instanceName = TODO
      break;
    case 0x2bd13ab3:
      // sldrThis.pakFile = TODO
      break;
    case 0xf8be005a:
      // sldrThis.asset = TODO
      break;
    case 0xfc93ceb8:
      sldrThis.fieldofView = input.ReadFloat();
      break;
    case 0x9fb2faa6:
      sldrThis.fieldofView2Player = input.ReadFloat();
      break;
    case 0xa9096914:
      sldrThis.disableDebugMenu = input.ReadBool();
      break;
    case 0x7262d27b:
      sldrThis.unknown_0x7262d27b = input.ReadBool();
      break;
    case 0xe943ba12:
      sldrThis.developmentMode = input.ReadBool();
      break;
    case 0xa3dcf42a:
      sldrThis.unknown_0xa3dcf42a = input.ReadFloat();
      break;
    case 0xb35c72be:
      sldrThis.unknown_0xb35c72be = input.ReadFloat();
      break;
    case 0x4a02103c:
      sldrThis.unknown_0x4a02103c = input.ReadFloat();
      break;
    case 0xe1fca71b:
      sldrThis.unknown_0xe1fca71b = input.ReadFloat();
      break;
    case 0xfbce966a:
      sldrThis.unknown_0xfbce966a = input.ReadFloat();
      break;
    case 0x09c6ca10:
      sldrThis.unknown_0x09c6ca10 = input.ReadFloat();
      break;
    case 0x4dfcd432:
      sldrThis.hardModeDamageMultiplier = input.ReadFloat();
      break;
    case 0xae1831d9:
      sldrThis.hardModeWeaponMultiplier = input.ReadFloat();
      break;
    case 0x5ab5812c:
      sldrThis.unknown_0x5ab5812c = input.ReadFloat();
      break;
    case 0x53401390:
      sldrThis.unknown_0x53401390 = input.ReadFloat();
      break;
    case 0xd09f373b:
      sldrThis.totalPercentage = input.ReadInt32();
      break;
    case 0x1d627808:
      LoadTypedefSLdrTweakGame_FragLimitChoices(sldrThis.unknown_0x1d627808, input);
      break;
    case 0xb2e8828d:
      LoadTypedefSLdrTweakGame_TimeLimitChoices(sldrThis.unknown_0xb2e8828d, input);
      break;
    case 0x06af87bd:
      LoadTypedefSLdrTweakGame_CoinLimitChoices(sldrThis.unknown_0x06af87bd, input);
      break;
    case 0x1533ea4e:
      LoadTypedefSLdrTweakGame_TimeLimitChoices(sldrThis.unknown_0x1533ea4e, input);
      break;
    case 0x40818220:
      // sldrThis.unknown_0x40818220 = TODO
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
} */
#endif // _SLDRTWEAKGAME
