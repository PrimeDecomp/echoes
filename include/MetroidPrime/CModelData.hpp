#ifndef _CMODELDATA
#define _CMODELDATA

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/IAnimReader.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/string.hpp"

class CAABox;
class CActorLights;
class CAnimData;
class CAnimRes;
class CFrustumPlanes;
class CModel;
class CModelFlags;
class CStateManager;
class CSkinnedModel;
class CRandom16;

// TODO move
#include "Kyoto/Math/CQuaternion.hpp"
struct CAdvancementDeltas {
public:
  CAdvancementDeltas(const CVector3f& posDelta, const CQuaternion& rotDelta)
  : mPosDelta(posDelta), mRotDelta(rotDelta) {}

  const CVector3f& GetOffsetDelta() const { return mPosDelta; }
  const CQuaternion& GetOrientationDelta() const { return mRotDelta; }

private:
  CVector3f mPosDelta;
  CQuaternion mRotDelta;
};
CHECK_SIZEOF(CAdvancementDeltas, 0x1c)

class CStaticRes {
  CAssetId mCmdlId;
  CVector3f mScale;

public:
  CStaticRes(CAssetId id, const CVector3f& scale) : mCmdlId(id), mScale(scale) {}
};

class CModelData {
public:
  enum EWhichModel {
    kWM_Normal,
    kWM_XRay,
    kWM_Thermal,
    kWM_ThermalHot,
  };

  // TODO these probably aren't real
  bool HasNormalModel() const { return mNormalModel; }

  CModelData();
  CModelData(const CAnimRes&);
  CModelData(const CStaticRes&);
  CModelData(const CModelData& other);
  ~CModelData();

  CAdvancementDeltas AdvanceAnimation(float dt, CStateManager& mgr, TAreaId aid, bool advTree);
  void AdvanceParticles(const CTransform4f& xf, float dt, CStateManager& mgr);
  void RenderParticles(const CFrustumPlanes& planes) const;
  void RenderUnsortedParts(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                           const CModelFlags& flags) const;
  void RenderThermal(const CTransform4f& xf, const CColor& mulColor, const CColor& addColor,
                     const CModelFlags& flags) const;
  void Render(const CStateManager&, const CTransform4f&, const CActorLights*,
              const CModelFlags&) const;
  void Render(EWhichModel, const CTransform4f&, const CActorLights*, const CModelFlags&) const;
  void FlatDraw(EWhichModel which, const CTransform4f& xf, bool unsortedOnly,
                const CModelFlags& flags) const;
  CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
  void Touch(const CStateManager& mgr, int) const;
  SAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool advTree);

  const CAnimData* GetAnimationData() const { return mAnimData.get(); }
  CAnimData* AnimationData() { return mAnimData.get(); }
  CAABox GetBounds(const CTransform4f& xf) const;
  CAABox GetBounds() const;
  bool IsLoaded(int shaderIdx) const;
  bool IsDefinitelyOpaque(EWhichModel which) const;

  CTransform4f GetLocatorTransform(const rstl::string& name) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& name) const;

  bool HasAnimation() const { return !mAnimData.null(); }
  bool IsNull() const { return mAnimData.null() && !mNormalModel; }

  void SetXRayModel(const rstl::pair< CAssetId, CAssetId >& assets);
  void SetInfraModel(const rstl::pair< CAssetId, CAssetId >& assets);

  void SetAmbientColor(const CColor& color) { mAmbientColor = color; }
  bool GetSortThermal() const { return mSortThermal; }
  void SetSortThermal(bool b) { mSortThermal = b; }

  CVector3f GetScale() const { return mScale; }
  void SetScale(const CVector3f& scale) { mScale = scale; }

  bool GetIsLoop() const;
  void EnableLooping(bool enable);
  static CModelData CModelDataNull();
  static EWhichModel GetRenderingModel(const CStateManager& mgr);

private:
  CVector3f mScale;
  rstl::auto_ptr< CAnimData > mAnimData;
  bool mRenderSorted : 1;
  bool mSortThermal : 1;
  CColor mAmbientColor;
  rstl::optional_object< TCachedToken< CModel > > mNormalModel;
  rstl::optional_object< TCachedToken< CModel > > mXrayModel;
  rstl::optional_object< TCachedToken< CModel > > mInfraModel;
};
CHECK_SIZEOF(CModelData, 0x4c)

#endif // _CMODELDATA
