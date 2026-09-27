#include "MetroidPrime/CProjectedShadow.hpp"

#include "MetroidPrime/CModelData.hpp"

CProjectedShadow::CProjectedShadow(int width, int height, uchar persistent, int projectionMode)
: mTexture(kTF_I4, width, height, 1)
, mBounds(CAABox::MakeMaxInvertedBox())
, mScale(1.f)
, mTranslation(CVector3f::Zero())
, mZDistanceAdjust(0.f)
, mOpacity(1.f)
, mEnabled(false)
, mPersistent(persistent)
, mOverrideBounds(false)
, mProjectOnActors(projectionMode == 0)
, mNextShadow(nullptr) {}

CProjectedShadow::~CProjectedShadow() { mTexture.ScheduleDeletion(); }

void CProjectedShadow::ExpandBoundsForTexture() {
  const float texelScale = 3.f / (mTexture.GetWidth() - 2);
  const CVector3f offset(texelScale * mBounds.GetWidth(), texelScale * mBounds.GetHeight(), 0.f);
  mBounds = CAABox(mBounds.GetMinPoint() - offset, mBounds.GetMaxPoint() + offset);
}

// Guessed name.
void CProjectedShadow::SetBounds(const CAABox& bounds) {
  mBounds = bounds;
  mOverrideBounds = true;
}

void CProjectedShadow::RenderShadowBuffer(CStateManager& mgr, const CModelData& model,
                                          const CTransform4f& transform, int flags,
                                          const CVector3f& translation, float scale,
                                          float zDistanceAdjust) {
  const CModelData* modelPtr = &model;
  const CTransform4f* transformPtr = &transform;
  RenderShadowBuffer(mgr, 1, &modelPtr, &transformPtr, flags, translation, scale, zDistanceAdjust);
}

void CProjectedShadow::RenderShadowBuffer(CStateManager& mgr, int count,
                                          const CModelData* const* models,
                                          const CTransform4f* const* transforms, int flags,
                                          const CVector3f& translation, float scale,
                                          float zDistanceAdjust) {
  if (count < 1) {
    return;
  }

  if (!mOverrideBounds) {
    mBounds = models[0]->GetBounds(*transforms[0]);
    for (int i = 1; i < count; ++i) {
      mBounds.Include(models[i]->GetBounds(*transforms[i]));
    }
  } else {
    mOverrideBounds = false;
  }
  mScale = scale;
  mTranslation = translation;
  mZDistanceAdjust = zDistanceAdjust;
  mEnabled = true;
  ExpandBoundsForTexture();

  // TODO: Render silhouettes into the texture, restore graphics state and enqueue the shadow.
}

CAABox ScaleAndTranslateBounds(const CAABox& bounds, const CVector3f& translation, float scale) {
  const CVector3f extent = bounds.GetMaxPoint() - bounds.GetMinPoint();
  const CVector3f padding = (extent * scale - extent) * 0.5f;
  return CAABox(bounds.GetMinPoint() - padding + translation,
                bounds.GetMaxPoint() + padding + translation);
}

void CProjectedShadow::Render(const CStateManager& mgr) const {
  // TODO: Project the texture onto world geometry and optional actor receivers.
}
