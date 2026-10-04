#ifndef _CSCRIPTWORLDTELEPORTER
#define _CSCRIPTWORLDTELEPORTER

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/SObjectTag.hpp"

class CScriptWorldTeleporter : public CEntity {
public:
  // Elevator transition.
  CScriptWorldTeleporter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         CAssetId worldId, CAssetId areaId, CAssetId playerAncs, int defaultAnim,
                         int charIdx, const CVector3f& playerScale, CAssetId platformModel,
                         const CVector3f& platformScale, CAssetId backgroundModel,
                         const CVector3f& backgroundScale, bool upElevator, CAssetId soundGroup,
                         ushort soundId, uchar volume, uchar panning);
  // Text transition.
  CScriptWorldTeleporter(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         CAssetId worldId, CAssetId areaId, ushort soundId, uchar volume,
                         uchar panning, CAssetId fontId, CAssetId stringId, bool fadeWhite,
                         const rstl::string& audioStream, bool displaySubtitles, bool introText,
                         float charFadeTime, float charsPerSecond, float startDelay,
                         float endDelay, float subtitleFadeInDelay, float subtitleFadeTime);

  // CEntity
  ~CScriptWorldTeleporter() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void StartTransition(CStateManager& mgr);
  CAssetId GetWorldId() const { return mWorldId; }
  CAssetId GetAreaId() const { return mAreaId; }

private:
  enum ETeleporterType { kTT_NoTransition, kTT_Elevator, kTT_Text };

  CAssetId mWorldId;
  CAssetId mAreaId;
  ETeleporterType mType;
  bool mUpElevator : 1;
  bool mInTransition : 1;
  bool x30_26_ : 1;
  bool mFadeWhite : 1;
  bool mDisplaySubtitles : 1;
  bool mIntroText : 1;
  float mCharFadeTime;
  float mCharsPerSecond;
  float mStartDelay;
  float mEndDelay;
  float mSubtitleFadeInDelay;
  float mSubtitleFadeTime;
  CAssetId mPlayerAncs;
  uint mPlayerDefaultAnim;
  int mPlayerCharIdx;
  CVector3f mPlayerScale;
  CAssetId mPlatformModel;
  CVector3f mPlatformScale;
  CAssetId mBackgroundModel;
  CVector3f mBackgroundScale;
  CAssetId mSoundGroup;
  ushort mSoundId;
  uchar mVolume;
  uchar mPanning;
  CAssetId mFontId;
  CAssetId mStringId;
  rstl::string mAudioStream;
};
CHECK_SIZEOF(CScriptWorldTeleporter, 0xa4)

#endif // _CSCRIPTWORLDTELEPORTER
