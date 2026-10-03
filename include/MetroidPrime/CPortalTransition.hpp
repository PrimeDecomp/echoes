#ifndef _CPORTALTRANSITION
#define _CPORTALTRANSITION

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CGameCameraSpline.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCharacterFactory;
class CParticleGen;

// Guessed name. The standalone animation/effect runtime for portal travel.
class CPortalTransition {
public:
  // Guessed names. The two optional camera paths run consecutively.
  enum ECameraPass { kCP_First, kCP_Second, kCP_None };

  CPortalTransition(const CAnimRes& samusRes, int suitCharIdx, bool renderGrapple,
                    const CTransform4f& samusTransform,
                    rstl::optional_object< CToken > firstEffectDescription,
                    const CTransform4f& firstEffectTransform, const CVector3f& firstEffectScale,
                    rstl::optional_object< CToken > secondEffectDescription,
                    const CTransform4f& secondEffectTransform, const CVector3f& secondEffectScale,
                    const CGameCameraSpline* firstPassCamera,
                    const CGameCameraSpline* secondPassCamera, const CTransform4f& cameraTransform,
                    rstl::optional_object< CToken > soundGroupCommon,
                    rstl::optional_object< CToken > soundGroupDirectional, ushort startPortal,
                    ushort inPortal1, ushort inPortal2, uchar volume, uchar pan, int direction);
  ~CPortalTransition();
  bool IsReady() const;
  void Draw() const;
  void Update(float dt);
  bool TouchModels();
  bool IsFinished() const;

private:
  // Guessed names.
  CTransform4f GetCameraTransform(ECameraPass pass) const;
  float GetCameraFov(ECameraPass pass) const;
  void UpdateLights();

  // Guessed field names, recovered from construction and runtime consumers.
  float mCurTime;
  int mDirection;
  CRandom16 mRandom;
  CAnimRes mSamusRes;
  int mSuitCharIdx;
  mutable CModelData mSamusModelData; // Draw prepares the animation's render cache.
  CModelFlags mModelFlags;
  CTransform4f mSamusTransform;
  CModelData mBeamModelData;
  CModelData mGrappleModelData;
  CTransform4f mGunTransform;
  CTransform4f mGrappleTransform;
  rstl::optional_object< CToken > mBeamModel;
  rstl::optional_object< CToken > mGrappleModel;
  rstl::optional_object< CToken > mSuitModel;
  rstl::optional_object< CToken > mSuitSkin;
  rstl::optional_object< TLockedToken< CCharacterFactory > > mCharacterFactory;
  rstl::optional_object< CToken > mFirstEffectDescription;
  rstl::single_ptr< CParticleGen > mFirstEffect;
  CTransform4f mFirstEffectTransform;
  rstl::optional_object< CToken > mSecondEffectDescription;
  rstl::single_ptr< CParticleGen > mSecondEffect;
  CTransform4f mSecondEffectTransform;
  ECameraPass mCameraPass;
  mutable rstl::optional_object< CGameCameraSpline > mFirstPassCamera;
  mutable rstl::optional_object< CGameCameraSpline > mSecondPassCamera;
  CTransform4f mCameraTransform;
  rstl::vector< CLight > mLights;
  rstl::optional_object< CToken > mSoundGroupCommon;
  rstl::optional_object< CToken > mSoundGroupDirectional;
  ushort mStartPortal;
  ushort mInPortal1;
  ushort mInPortal2;
  CSfxHandle mStartPortalHandle;
  CSfxHandle mInPortal1Handle;
  CSfxHandle mInPortal2Handle;
  uchar mVolume;
  uchar mPan;
};
CHECK_SIZEOF(CPortalTransition, 0x650)

#endif // _CPORTALTRANSITION
