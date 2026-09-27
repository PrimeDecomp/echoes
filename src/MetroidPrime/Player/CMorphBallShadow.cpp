#include "MetroidPrime/Player/CMorphBallShadow.hpp"

#include "MetaRender/CCubeRenderer.hpp"

CMorphBallShadow::CMorphBallShadow(int width, int height, const TToken< CTexture >& ballFade)
: mTexture(kTF_I8, width, height, 1)
, mBallFade(ballFade)
, mWidth(width)
, mHeight(height)
, mShadowVolume(CAABox::MakeMaxInvertedBox())
, mHasIds(false) {
  mBallFade.Lock();
}

CMorphBallShadow::~CMorphBallShadow() { mTexture.ScheduleDeletion(); }

void CMorphBallShadow::RenderIdBuffer(const CAABox& aabb, CStateManager& mgr, CPlayer& player) {
  mShadowVolume = aabb;
  mActors.clear();
  mAreas.clear();
  mWorldModelBits = rstl::vector< uint >();

  gpRender->SetRequestRGBA6(true);
  if (!gpRender->IsRGBA6Current()) {
    mHasIds = false;
    return;
  }

  GatherAreas(mgr);
  // TODO: Gather eligible actors, render receiver IDs, then copy the alpha texture.
  mHasIds = false;
}

void CMorphBallShadow::Render(CStateManager& mgr, float alpha, const CTexture& shadowTexture) {
  if (!mHasIds || !AreasValid(mgr)) {
    return;
  }

  // TODO: Project receiver IDs using shadowTexture, optional ball fade, and actor/world geometry.
}

void CMorphBallShadow::GatherAreas(CStateManager& mgr) {
  mAreas.clear();
  // TODO: Record visible area IDs in the world's alive-chain order.
}

bool CMorphBallShadow::AreasValid(const CStateManager& mgr) const {
  // TODO: Compare the world's current visible alive-chain areas with mAreas in order.
  return false;
}
