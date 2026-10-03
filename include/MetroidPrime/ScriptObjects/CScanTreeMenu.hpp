#ifndef _CSCANTREEMENU
#define _CSCANTREEMENU

#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

class CInputStream;

// Class, method and member names are guessed.
class CScanTreeMenu : public CScanTreeNode {
public:
  // Enum names are guessed; unresolved values retain their indices.
  enum ESetting {
    kS_SurroundMode = 0,
    kS_HudLag = 1,
    kS_HintSystem = 2,
    kS_Unknown3 = 3,
    kS_InvertYAxis = 4,
    kS_SwapBeamControls = 5,
    kS_Rumble = 6,
    kS_Unknown8 = 8,
    kS_Unknown9 = 9,
    kS_Unknown10 = 10,
    kS_Unknown11 = 11
  };

  CScanTreeMenu(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                const rstl::string& nameStringName, ESetting setting, CAssetId optionStringTable,
                const rstl::string& option1, int value1, const rstl::string& option2, int value2,
                const rstl::string& option3, int value3, const rstl::string& option4, int value4);

  // CScanTreeNode
  ~CScanTreeMenu();
  void LockResources();
  void UnlockResources();
  bool AreResourcesLoaded();
  ENodeType GetNodeType() const;

  int GetOptionCount() const;
  rstl::wstring GetOptionName(int index) const;
  void ApplyOption(int index);
  int GetCurrentOptionIndex() const;
  void ApplySelectedOption();
  int GetSelectedOption() const;
  void RefreshSelectedOption();
  ESetting GetSetting() const;

private:
  ESetting mSetting;
  int mOptionValue;
  int mSelectedOption;
  rstl::auto_ptr< TCachedToken< CStringTable > > mOptionStringTable;
  rstl::reserved_vector< rstl::pair< rstl::string, int >, 4 > mOptions;
};
CHECK_SIZEOF(CScanTreeMenu, 0xCC)

// Guessed loader name.
CScanTreeMenu* LoadScanTreeMenu(int* id, CInputStream& input);

#endif // _CSCANTREEMENU
