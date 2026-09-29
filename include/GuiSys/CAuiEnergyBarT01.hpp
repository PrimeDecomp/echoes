#ifndef _CAUIENERGYBART01
#define _CAUIENERGYBART01

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"

class IObjectStore;

class CAuiEnergyBarT01 : public CGuiWidget {
public:
  enum ESetMode { kSM_Normal, kSM_Wrapped, kSM_Instant };

  typedef rstl::pair< CVector3f, CVector3f > (*FCoordFunc)(float t);

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);
  CAuiEnergyBarT01(const CGuiWidgetParms& parms, IObjectStore* pool, CAssetId textureId,
                   bool loadTexture);

  // CGuiObject
  ~CAuiEnergyBarT01() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  EWidgetUsageFlags GetWidgetUsageFlags() const override;
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;

  void SetMaxEnergy(float maxEnergy);
  void SetCurrEnergy(float energy, ESetMode mode);

  float GetLaggedEnergy() const { return mFilledEnergy; }
  float GetActualEnergy() const { return mSetEnergy; }
  float GetMaxEnergy() const { return mMaxEnergy; }
  void SetFilledColor(const CColor& color) { mFilledColor = color; }
  void SetShadowColor(const CColor& color) { mShadowColor = color; }
  void SetEmptyColor(const CColor& color) { mEmptyColor = color; }
  void SetCoordFunc(FCoordFunc func) { mCoordFunc = func; }
  void SetTesselation(float tesselation) { mTesselation = tesselation; }
  void SetFilledDrainSpeed(float speed) { mFilledSpeed = speed; }
  void SetShadowDrainSpeed(float speed) { mShadowSpeed = speed; }
  void SetShadowDrainDelay(float delay) { mShadowDrainDelay = delay; }
  void SetIsAlwaysResetTimer(bool reset) { mAlwaysResetDelayTimer = reset; }

private:
  CAssetId mTextureId;
  rstl::optional_object< TCachedToken< CTexture > > mTexture;
  CColor mEmptyColor;
  CColor mFilledColor;
  CColor mShadowColor;
  FCoordFunc mCoordFunc;
  float mTesselation;
  float mMaxEnergy;
  float mFilledSpeed;
  float mShadowSpeed;
  float mShadowDrainDelay;
  bool mAlwaysResetDelayTimer;
  bool mWrapping;
  float mSetEnergy;
  float mFilledEnergy;
  float mShadowEnergy;
  float mShadowDrainDelayTimer;
};
CHECK_SIZEOF(CAuiEnergyBarT01, 0x108)

#endif // _CAUIENERGYBART01
