#ifndef _CINGAMETWEAKMANAGER
#define _CINGAMETWEAKMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "rstl/string.hpp"

class CTweakValue {
public:
  struct Audio {
    float GetFadeIn() const { return mFadeIn; }
    float GetFadeOut() const { return mFadeOut; }
    float GetVolume() const { return mVolume; }
    const rstl::string& GetFileName() const { return mFileName; }

  private:
    float mFadeIn;
    float mFadeOut;
    float mVolume;
    rstl::string mFileName;
    CAssetId mResourceId;
  };

  const Audio& GetAudio() const { return mAudio; }

private:
  uint mType;
  rstl::string mKey;
  rstl::string mText;
  Audio mAudio;
  uint mValue;
};

NESTED_CHECK_SIZEOF(CTweakValue, Audio, 0x20)
CHECK_SIZEOF(CTweakValue, 0x48)

class CInGameTweakManager {
public:
  bool HasTweakValue(const rstl::string& key) const;
  const CTweakValue* GetTweakValue(const rstl::string& key) const;
  static rstl::string GetIdentifierForMusicEvent(CAssetId areaId, const rstl::string& name);
};

extern CInGameTweakManager* gpTweakManager;

#endif // _CINGAMETWEAKMANAGER
