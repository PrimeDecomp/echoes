#include "MetroidPrime/ScriptLoader/SLdrTweakGame_CoinLimitChoices.hpp"

SLdrTweakGame_CoinLimitChoices::SLdrTweakGame_CoinLimitChoices() {
  coinLimit0 = 200;
  coinLimit1 = 400;
  coinLimit2 = 600;
  coinLimit3 = 800;
  coinLimit4 = 1000;
}

SLdrTweakGame_CoinLimitChoices::~SLdrTweakGame_CoinLimitChoices() {}

void LoadTypedefSLdrTweakGame_CoinLimitChoices(SLdrTweakGame_CoinLimitChoices& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x6b4cae26:
      sldrThis.coinLimit0 = input.ReadInt32();
      break;
    case 0xd3f0c943:
      sldrThis.coinLimit1 = input.ReadInt32();
      break;
    case 0xc14566ad:
      sldrThis.coinLimit2 = input.ReadInt32();
      break;
    case 0x79f901c8:
      sldrThis.coinLimit3 = input.ReadInt32();
      break;
    case 0xe42e3971:
      sldrThis.coinLimit4 = input.ReadInt32();
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}
