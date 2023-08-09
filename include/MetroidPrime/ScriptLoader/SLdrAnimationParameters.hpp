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

#endif // _SLDRANIMATIONPARAMETERS
