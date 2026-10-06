#ifndef _CSCRIPTSUBTITLE
#define _CSCRIPTSUBTITLE

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Text/CGuiTextSupport.hpp"
#include "Kyoto/Text/CStringTable.hpp"

// Guessed class name.
class CScriptSubtitle : public CActor {
public:
  CScriptSubtitle(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int positionX,
                  int positionY, int extentX, int extentY, const CColor& fontColor,
                  const CColor& outlineColor, const CColor& geometryColor, CAssetId font,
                  CAssetId stringTable, int initialStringIndex, float fadeInTime, float fadeOutTime,
                  const CGuiTextProperties& textProperties);

  // CEntity
  ~CScriptSubtitle() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;

private:
  // Guessed method names.
  void RefreshText();
  void SetStringIndex(CStateManager& mgr, int index);

  // Guessed member names.
  float mPositionX;
  float mPositionY;
  mutable CGuiTextSupport mTextSupport;
  CColor mGeometryColor;
  TLockedToken< CStringTable > mStringTable;
  int mStringIndex;
  float mFadeInTime;
  float mFadeOutTime;
  float mFadeOpacity;
  float mTargetFadeOpacity;
};
CHECK_SIZEOF(CScriptSubtitle, 0xe90)

#endif
