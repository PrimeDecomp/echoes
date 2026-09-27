#ifndef _CPROJECTEDSHADOW
#define _CPROJECTEDSHADOW

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"

class CModelData;
class CStateManager;
class CTransform4f;

class CProjectedShadow {
public:
  // Projection mode zero includes actor receivers; other modes are not yet identified.
  CProjectedShadow(int width, int height, uchar persistent, int projectionMode);
  ~CProjectedShadow();

  void Render(const CStateManager& mgr) const;
  void RenderShadowBuffer(CStateManager& mgr, const CModelData& model,
                          const CTransform4f& transform, int flags, const CVector3f& translation,
                          float scale, float zDistanceAdjust);
  void RenderShadowBuffer(CStateManager& mgr, int count, const CModelData* const* models,
                          const CTransform4f* const* transforms, int flags,
                          const CVector3f& translation, float scale, float zDistanceAdjust);
  // Guessed name. Overrides the bounds for the next shadow-buffer render.
  void SetBounds(const CAABox& bounds);

  void SetOpacity(float opacity) { mOpacity = opacity; }
  void SetNextShadow(CProjectedShadow* shadow) { mNextShadow = shadow; }
  CProjectedShadow* GetNextShadow() const { return mNextShadow; }

private:
  void ExpandBoundsForTexture();

  CTexture mTexture;
  CAABox mBounds;
  float mScale;
  CVector3f mTranslation;
  float mZDistanceAdjust;
  float mOpacity;
  bool mEnabled : 1;
  uchar mPersistent : 1;
  bool mOverrideBounds : 1;
  bool mProjectOnActors : 1;
  CProjectedShadow* mNextShadow;
};
CHECK_SIZEOF(CProjectedShadow, 0xa0)

#endif // _CPROJECTEDSHADOW
