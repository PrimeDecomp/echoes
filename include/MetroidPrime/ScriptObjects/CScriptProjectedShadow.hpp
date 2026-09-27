#ifndef _CSCRIPTPROJECTEDSHADOW
#define _CSCRIPTPROJECTEDSHADOW

#include "MetroidPrime/CActor.hpp"
#include "rstl/single_ptr.hpp"

class CProjectedShadow;

class CScriptShadowProjector : public CActor {
public:
  CScriptShadowProjector(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& transform, const CVector3f& offset, bool persistent,
                         float scale, float zOffsetAdjust, float opacity, float opacityChange,
                         int textureSize);

  // CEntity
  ~CScriptShadowProjector() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;

private:
  float mScale;
  CVector3f mOffset;
  float mZOffsetAdjust;
  float mOpacity;
  float mOpacityRecip;
  TUniqueId mTarget;
  rstl::single_ptr< CProjectedShadow > mProjectedShadow;
  uint mTextureSize;
  bool mPersistent : 1;
  bool mShadowInvalidated : 1;
};
CHECK_SIZEOF(CScriptShadowProjector, 0x188)

#endif // _CSCRIPTPROJECTEDSHADOW
