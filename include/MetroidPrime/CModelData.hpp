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
class CCharAnimTime;
class CSegId;
class CPlane;
class CTexture;
class CPlayerState;
struct SSkinningWorkspace;
struct SModelDataMultipassContext;

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
  CAssetId GetId() const { return mCmdlId; }
  const CVector3f& GetScale() const { return mScale; }
};
CHECK_SIZEOF(CStaticRes, 0x10)

class CModelData {
public:
  // Guessed names; values follow the Echoes visor-to-model selection.
  enum EWhichModel {
    kWM_Normal,
    kWM_Dark,
    kWM_Echo,
  };

  // TODO these probably aren't real
  bool HasNormalModel() const { return mNormalModel; }

  CModelData();
  CModelData(const CAnimRes&);
  CModelData(const CStaticRes&);
  CModelData(const CModelData& other);
  ~CModelData();

  CAdvancementDeltas AdvanceAnimation(float dt, CStateManager& mgr, TAreaId aid, bool advTree,
                                      float minParticleWeight = 0.f);
  CAdvancementDeltas AdvanceAnimation(float dt, CRandom16& random, bool advTree);
  void AdvanceParticles(const CTransform4f& xf, float dt, CStateManager& mgr);
  void RenderParticles(const CFrustumPlanes& planes) const;
  void RenderUnsortedParts(EWhichModel which, const CTransform4f& xf, const CActorLights* lights,
                           const CModelFlags& flags) const;
  void Render(const CStateManager&, const CTransform4f&, const CActorLights*,
              const CModelFlags&) const;
  void Render(EWhichModel, const CTransform4f&, const CActorLights*, const CModelFlags&) const;
  void RenderSolid(EWhichModel which, const CTransform4f& xf, bool unsortedOnly,
                   const CModelFlags& flags) const;
  void RenderModelMultipleTimesWithFlags(EWhichModel which, const CTransform4f& xf,
                                         const CActorLights* lights, const CModelFlags* flags,
                                         const u64* masks, const CColor* colors,
                                         const CPlane* planes, int count) const;
  void DisintegrateDraw(EWhichModel which, const CTransform4f& xf, const CTexture& texture,
                        const CColor& color, float amount) const;
  void DisintegrateDraw(const CStateManager& mgr, const CTransform4f& xf, const CTexture& texture,
                        const CColor& color, float amount) const;
  // Guessed names.
  void RenderNoise(EWhichModel which, const CTransform4f& xf, const CColor& color,
                   bool additive) const;
  void RenderNoise(const CStateManager& mgr, const CTransform4f& xf, const CColor& color,
                   bool additive) const;
  CSkinnedModel& PickAnimatedModel(EWhichModel which) const;
  const TLockedToken< CModel >& PickStaticModel(EWhichModel which) const;
  void Touch(const CStateManager& mgr, int) const;
  void Touch(EWhichModel which, int shaderIdx) const;
  void Touch() const;
  CAdvancementDeltas AdvanceAnimationIgnoreParticles(float dt, CRandom16& rand, bool advTree);
  int GetNumShaders() const;
  // Guessed name.
  void LockTextures();
  void SetupWorldSpacePortalPlane(const CTransform4f& xf, const CPlane& plane) const;

  const CAnimData* GetAnimationData() const { return mAnimData.get(); }
  CAnimData* AnimationData() { return mAnimData.get(); }
  CAABox GetBounds(const CTransform4f& xf) const;
  CAABox GetBounds() const;
  bool IsLoaded(int shaderIdx) const;
  bool IsDefinitelyOpaque(EWhichModel which) const;

  CTransform4f GetLocatorTransform(const rstl::string& name) const;
  CTransform4f GetLocatorTransform(const CSegId& id) const;
  CTransform4f GetLocatorTransformDynamic(const rstl::string& name,
                                          const CCharAnimTime* time) const;
  CTransform4f GetLocatorTransformDynamic(const CSegId& id, const CCharAnimTime* time) const;
  CTransform4f GetScaledLocatorTransform(const rstl::string& name) const;
  CTransform4f GetScaledLocatorTransform(const CSegId& id) const;
  CTransform4f GetScaledLocatorTransformDynamic(const rstl::string& name,
                                                const CCharAnimTime* time) const;
  CTransform4f GetScaledLocatorTransformDynamic(const CSegId& id, const CCharAnimTime* time) const;

  bool HasAnimation() const { return !mAnimData.null(); }
  bool IsNull() const { return mAnimData.null() && !mNormalModel; }

  // Guessed names: the alternate resources are Echo and Dark visor models.
  void SetEchoModel(const rstl::pair< CAssetId, CAssetId >& assets);
  void SetDarkModel(const rstl::pair< CAssetId, CAssetId >& assets);

  void SetAmbientColor(const CColor& color) { mAmbientColor = color; }
  // Guessed name.
  void SetRenderFullEchoModel(bool enabled) { mRenderFullEchoModel = enabled; }

  CVector3f GetScale() const { return mScale; }
  void SetScale(const CVector3f& scale);

  bool GetIsLoop() const;
  bool IsAnimating() const;
  float GetAnimationDuration(int anim) const;
  void EnableLooping(bool enable);
  static CModelData CModelDataNull();
  static EWhichModel GetRenderingModel(const CStateManager& mgr);
  static EWhichModel GetRenderingModel(const CStateManager& mgr, const CPlayerState& playerState);

private:
  // Guessed name. Echoes supplies a skinning workspace instead of Prime's raw arrays.
  static void MultipassDrawCallback(const SSkinningWorkspace& workspace,
                                    const SModelDataMultipassContext& context);
  CVector3f mScale;
  rstl::auto_ptr< CAnimData > mAnimData;
  mutable bool mRenderSorted : 1;
  // Guessed names, supported by the texture-lock and render paths.
  bool mTexturesLocked : 1;
  bool mRenderUnsortedParts : 1;
  bool mRenderFullEchoModel : 1;
  CColor mAmbientColor;
  rstl::optional_object< TLockedToken< CModel > > mNormalModel;
  rstl::optional_object< TLockedToken< CModel > > mEchoModel;
  rstl::optional_object< TLockedToken< CModel > > mDarkModel;
};
CHECK_SIZEOF(CModelData, 0x4c)

#endif // _CMODELDATA
