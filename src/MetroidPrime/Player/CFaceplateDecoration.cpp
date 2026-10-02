#include "MetroidPrime/Player/CFaceplateDecoration.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"

CFaceplateDecoration::CFaceplateDecoration(const CStateManager& mgr, int playerIndex)
: mPlayerIndex(playerIndex), mTextureId(kInvalidAssetId) {}

void CFaceplateDecoration::Update(const CStateManager& mgr) {
  CAssetId textureId = mgr.GetPlayer(mPlayerIndex)->GetVisorSteam().GetTextureId();
  if (textureId == kInvalidAssetId && mTexture.valid()) {
    mTexture->Unlock();
    mTextureId = textureId;
  }

  if (textureId != mTextureId && textureId != kInvalidAssetId) {
    mTextureId = textureId;
    mTexture = gpSimplePool->GetObj(SObjectTag('TXTR', mTextureId));
    if (mTexture.valid()) {
      mTexture->Lock();
    }
  }
}

void CFaceplateDecoration::Draw(const CStateManager& mgr) const {
  if (mTexture.valid() && mTexture->IsLoaded() && mTextureId != kInvalidAssetId) {
    CTexture* texture = *TLockedToken< CTexture >(*mTexture);
    float alpha = mgr.GetPlayer(mPlayerIndex)->GetVisorSteamAlpha();
    if (!close_enough(alpha, 0.f)) {
      CCameraFilterPass::DrawFilter(CCameraFilterPass::kFT_Blend,
                                   CCameraFilterPass::kFS_FullscreenQuarters,
                                   CColor::White().WithAlphaOf(alpha), texture, 1.f);
    }
  }
}
