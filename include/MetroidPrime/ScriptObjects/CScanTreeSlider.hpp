#ifndef _CSCANTREESLIDER
#define _CSCANTREESLIDER

#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"

class CInputStream;

// Class, method, enum and member names are guessed.
class CScanTreeSlider : public CScanTreeNode {
public:
  enum ESetting {
    kS_ScreenBrightness = 0,
    kS_ScreenPositionX = 1,
    kS_ScreenPositionY = 2,
    kS_ScreenStretch = 3,
    kS_SfxVolume = 4,
    kS_MusicVolume = 5,
    kS_HudAlpha = 6,
    kS_HelmetAlpha = 7
  };

  CScanTreeSlider(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                  const rstl::string& nameStringName, ESetting setting);

  // CScanTreeNode
  ~CScanTreeSlider();
  ENodeType GetNodeType() const;

  float GetNormalizedDefaultValue() const;
  float GetNormalizedValue() const;
  void SetNormalizedValue(float value);
  float GetSavedNormalizedValue() const;
  void RestoreSavedValue();
  void SaveValue();
  void ApplyNormalizedValue();
  void RefreshNormalizedValue();
  void SetOptionValue(int value);
  int GetDefaultOptionValue() const;
  int GetOptionValue() const;
  int GetMaxOptionValue() const;
  int GetMinOptionValue() const;

private:
  float mNormalizedValue;
  float mSavedNormalizedValue;
  ESetting mSetting;
};
CHECK_SIZEOF(CScanTreeSlider, 0x70)

// Guessed loader name.
CScanTreeSlider* LoadScanTreeSlider(int* id, CInputStream& input);

#endif // _CSCANTREESLIDER
