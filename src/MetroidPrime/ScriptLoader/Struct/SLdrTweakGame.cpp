#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGame.hpp"

SLdrTweakGame::SLdrTweakGame()
  : instanceName()
  , pakFile()
  , asset()
  , fieldofView(55.0)
  , fieldofView2Player(45.0)
  , disableDebugMenu(false)
  , unknown_0x7262d27b(true)
  , developmentMode(true)
  , unknown_0xa3dcf42a(25.0)
  , unknown_0xb35c72be(1.0)
  , unknown_0x4a02103c(30.0)
  , unknown_0xe1fca71b(125.0)
  , unknown_0xfbce966a(150.0)
  , unknown_0x09c6ca10(300.0)
  , hardModeDamageMultiplier(1.53)
  , hardModeWeaponMultiplier(0.5)
  , unknown_0x5ab5812c(0.15000001)
  , unknown_0x53401390(0.15000001)
  , totalPercentage(102)
  , unknown_0x1d627808()
  , unknown_0xb2e8828d()
  , unknown_0x06af87bd()
  , unknown_0x1533ea4e()
  , unknown_0x40818220()
{}

SLdrTweakGame::~SLdrTweakGame() {}

void LoadTypedefSLdrTweakGame(SLdrTweakGame& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0x2bd13ab3:
      sldrThis.pakFile = rstl::string(input);
      break;
    case 0xf8be005a:
      sldrThis.asset = rstl::string(input);
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
      sldrThis.unknown_0x40818220 = SLdrSpline(input, propertySize);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}