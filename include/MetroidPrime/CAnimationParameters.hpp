#ifndef _CANIMATIONPARAMETERS
#define _CANIMATIONPARAMETERS

#include "Kyoto/SObjectTag.hpp"

class CAnimationParameters {
public:
  CAnimationParameters(CAssetId ancs, uint character, uint initialAnimation)
  : mAncs(ancs), mCharacter(character), mInitialAnimation(initialAnimation) {}

  CAssetId GetACSFile() const { return mAncs; }
  uint GetCharacter() const { return mCharacter; }
  uint GetInitialAnimation() const { return mInitialAnimation; }

private:
  CAssetId mAncs;
  uint mCharacter;
  uint mInitialAnimation;
};
CHECK_SIZEOF(CAnimationParameters, 0xc)

#endif
