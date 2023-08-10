#ifndef _SLDRANIMATIONPARAMETERS
#define _SLDRANIMATIONPARAMETERS

#include "Kyoto/SObjectTag.hpp"

struct SLdrAnimationParameters {
  SLdrAnimationParameters();
  ~SLdrAnimationParameters();

  CAssetId ancs;
  int character_index;
  int initial_anim;
};

void LoadTypedefSLdrAnimationParameters(SLdrAnimationParameters&, CInputStream&);

#endif // _SLDRANIMATIONPARAMETERS
