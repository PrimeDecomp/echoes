#ifndef _CSPLITTERBEAMEFFECT
#define _CSPLITTERBEAMEFFECT

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "MetroidPrime/CActor.hpp"

#include "rstl/single_ptr.hpp"

class CAbsAngle;
class CEntityInfo;
class CStateManager;
class CTransform4f;

// Guessed class: the cone-shaped beam effect that the splitter command module projects. The class
// name is corroborated by the Wii SEL export of the constructor. The cone is rendered by
// projecting the depth buffer seen from the cone apex into a small texture (see PreRender), which
// is then used to cut the cone against the world geometry (see Render).
class CSplitterBeamEffect : public CActor {
public:
  CSplitterBeamEffect(TUniqueId uid, const CEntityInfo& info, const rstl::string& name,
                      const CTransform4f& xf, int textureHeight, const CAbsAngle& angle,
                      float range, const CColor& cloudColor1, const CColor& cloudColor2,
                      const CColor& addColor1, const CColor& addColor2, float cloudScale,
                      float fadeOffSize, float openSpeed);

  // CEntity
  ~CSplitterBeamEffect() override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

  // Guessed names: driven by the command module while it scans.
  void SetOpening(bool opening) { mOpening = opening; }
  bool IsClosed() const { return !mOpening && mOpenFraction == 0.f; }

private:
  float mMaxAngle;                       // Guessed name
  float mRange;                          // Guessed name
  CColor mCloudColor1;                   // Guessed name
  CColor mCloudColor2;                   // Guessed name
  CColor mAddColor1;                     // Guessed name
  CColor mAddColor2;                     // Guessed name
  float mCloudScale;                     // Guessed name
  float mFadeOffSize;                    // Guessed name
  float mOpenSpeed;                      // Guessed name
  rstl::single_ptr< CTexture > mTexture; // Guessed name; scene depth seen from the cone apex
  float mCurrentAngle;                   // Guessed name; radians
  int mPreRenderCount;                   // Guessed name
  float mOpenFraction;                   // Guessed name
  CRandom16 mRandom;                     // Guessed name
  float mAddColorLerp;                   // Guessed name
  float mCloudColorLerp;                 // Guessed name
  bool mOpening;                         // Guessed name
};

CHECK_SIZEOF(CSplitterBeamEffect, 0x1a0)

#endif // _CSPLITTERBEAMEFFECT
