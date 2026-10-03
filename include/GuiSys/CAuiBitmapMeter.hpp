#ifndef _CAUIBITMAPMETER
#define _CAUIBITMAPMETER

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/reserved_vector.hpp"

class CTexture;

// Guessed name: the BMTR widget is a textured meter with a trailing shadow bar.
class CAuiBitmapMeter : public CGuiWidget {
public:
  CAuiBitmapMeter(const CGuiWidgetParms& parms, CSimplePool* pool, CAssetId textureId,
                  const rstl::reserved_vector< CVector3f, 4 >& coords,
                  const rstl::reserved_vector< CVector2f, 4 >& uvs, bool loadTexture);

  // CGuiObject
  ~CAuiBitmapMeter() override;

  // CGuiWidget
  FourCC GetWidgetTypeID() const override;
  EWidgetUsageFlags GetWidgetUsageFlags() const override;
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool, uint version);

  // Guessed names, derived from the native animation and drawing consumers.
  void SetTargetFraction(float fraction);
  void SetCurrentFraction(float fraction);
  float GetCurrentFraction() const;
  float GetShadowFraction() const;
  void SetShadowColor(const CColor& color);
  void SetIncreaseSpeed(float speed);
  void SetDecreaseSpeed(float speed);

private:
  rstl::reserved_vector< CVector3f, 4 > mCoords;
  rstl::reserved_vector< CVector2f, 4 > mUvs;
  CAssetId mTextureId;
  rstl::optional_object< TCachedToken< CTexture > > mTexture;
  CColor mShadowColor;
  float mTargetFraction;
  float mCurrentFraction;
  float mShadowFraction;
  float mIncreaseSpeed;
  float mDecreaseSpeed;
  float mShadowDrainSpeed;
};
CHECK_SIZEOF(CAuiBitmapMeter, 0x144)

#endif // _CAUIBITMAPMETER
