#ifndef _CINGAMETWEAKMANAGER
#define _CINGAMETWEAKMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "rstl/string.hpp"

class CTweakValue {
public:
  struct Audio {
    float GetFadeIn() const { return x0_fadeIn; }
    float GetFadeOut() const { return x4_fadeOut; }
    float GetVolume() const { return x8_volume; }
    const rstl::string& GetFileName() const { return xc_fileName; }

  private:
    float x0_fadeIn;
    float x4_fadeOut;
    float x8_volume;
    rstl::string xc_fileName;
    CAssetId x1c_resourceId;
  };

  const Audio& GetAudio() const { return x24_audio; }

private:
  uint x0_type;
  rstl::string x4_key;
  rstl::string x14_text;
  Audio x24_audio;
  uint x44_value;
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
