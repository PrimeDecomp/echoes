#ifndef _CWORLDTRANSMANAGER
#define _CWORLDTRANSMANAGER

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CGameCameraSpline.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDarkWorldInfo.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CAnimRes;
class CCharacterFactory;
class CGuiTextSupport;
class CPortalTransition;
class CStringTable;

class CWorldTransManager {
public:
  enum ETransType { kTT_Disabled, kTT_Enabled, kTT_Text, kTT_Portal };

  CWorldTransManager();
  ~CWorldTransManager();

  void SetSfx(ushort sfx, uchar volume, uchar panning);
  void SfxStart();
  void SfxStop();
  void EnableTransition(const CAnimRes& samusRes, bool renderGrapple, CAssetId platformRes,
                        const CVector3f& platformScale, CAssetId bgRes, const CVector3f& bgScale,
                        bool goingUp, const CGameCameraSpline* firstPassCamera,
                        const CGameCameraSpline* secondPassCamera,
                        const CTransform4f& cameraTransform,
                        rstl::optional_object< CToken > soundGroup,
                        const CDarkWorldInfo* darkWorldInfo);
  void EnableTransition(CAssetId fontId, CAssetId stringId, int stringIdx, bool fadeWhite,
                        float charFadeTime, float charFadeRate, float textStartTime,
                        float textEndDelay, float subtitleFadeInDelay, float subtitleFadeTime,
                        const rstl::string& audioStream, uchar volume, bool displaySubtitles,
                        bool introText);
  void EnableTransition(rstl::single_ptr< CPortalTransition >& transition, uchar volume);
  void DisableTransition();
  void StartTransition();
  void EndTransition();
  void StartTextFadeOut();
  void Update(float dt);
  void Draw() const;
  void TouchModels();
  bool WaitForModelsAndTextures();
  // Guessed name.
  void CheckIntroTextSeen();
  bool IsTransitionFinished() const { return mTransitionFinished; }
  bool IsIntroText() const { return mIntroText; } // Guessed name
  ETransType GetTransType() const { return mTransType; }

private:
  struct SModelDatas;

  void UpdateDisabled(float dt);
  void UpdateEnabled(float dt);
  void UpdateText(float dt);
  void UpdateLights(float dt);
  void DrawAllModels() const;
  void DrawFirstPass() const;
  void DrawSecondPass() const;
  void DrawEnabled() const;
  void DrawDisabled() const;
  void DrawText() const;
  // Guessed names for the Echoes-only transition paths and camera queries.
  void UpdatePortalTransition(float dt);
  void DrawPortalTransition() const;
  CTransform4f GetCameraTransform(int pass) const;
  float GetCameraFov(int pass) const;

  float mCurTime;
  rstl::single_ptr< SModelDatas > mModelData;
  rstl::single_ptr< CGuiTextSupport > mTextData;
  rstl::single_ptr< CGuiTextSupport > mSubtitleData;
  rstl::optional_object< TToken< CStringTable > > mStrTable;
  rstl::optional_object< CDarkWorldInfo > mDarkWorldInfo;
  float mBgOffset;
  float mLightOffset;
  float mBgHeight;
  float mLightHeight;
  CRandom16 mRandom;
  rstl::optional_object< CToken > mSoundGroup;
  ushort mSfx;
  CSfxHandle mSfxHandle;
  uchar mVolume;
  uchar mPanning;
  ETransType mTransType;
  float mStopTime;
  float mTextStartTime;
  float mTextEndDelay;
  float mSubtitleFadeInDelay;
  float mSubtitleFadeTime;
  float mSfxInterval;
  rstl::string mAudioStream;
  int mStrIdx;
  float mTextElapsedTime;
  float mIntroTextFadeTimer;
  float mPortalFade;
  rstl::optional_object< CGameCameraSpline > mFirstPassCamera;
  rstl::optional_object< CGameCameraSpline > mSecondPassCamera;
  CTransform4f mCameraTransform;
  rstl::optional_object< TLockedToken< CCharacterFactory > > mCharacterFactory;
  rstl::single_ptr< CPortalTransition > mPortalTransition;
  bool mTransitionFinished : 1;
  bool mStopSoon : 1;
  bool mGoingUp : 1;
  bool mFadeWhite : 1;
  bool mTextDirty : 1;
  bool mDisplaySubtitles : 1;
  bool mIntroText : 1;
  bool mIntroTextSeen : 1;
  bool mLongShaft : 1;
  bool mIntroAudioStopped : 1;
};
CHECK_SIZEOF(CWorldTransManager, 0x4b0)

#endif // _CWORLDTRANSMANAGER
