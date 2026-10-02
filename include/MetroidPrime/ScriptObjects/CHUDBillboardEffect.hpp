#ifndef _CHUDBILLBOARDEFFECT
#define _CHUDBILLBOARDEFFECT

#include "MetroidPrime/CEffect.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CGenDescription;
class CElectricDescription;

// Layout only; the methods live in an unsplit range (0x800ECCDC-0x800EE0A0) and are not ported.
class CHUDBillboardEffect : public CEffect {
public:
  CHUDBillboardEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                      const rstl::optional_object< TToken< CElectricDescription > >& electric,
                      TUniqueId uid, bool active, const rstl::string& name, float dist,
                      const CVector3f& scale0, int playerIndex, const CColor& color, const CVector3f& scale1,
                      const CVector3f& translation, int unknown);

  CParticleGen* GetParticleGen() const { return mGenerator.get(); }
  void SetFinishing() { mFinishing = true; }

  static float GetNearClipDistance(const CStateManager& mgr, int playerIndex);
  static const CVector3f& GetScaleForPOV(const CStateManager& mgr);

private:
  rstl::single_ptr< CParticleGen > mGenerator; // 0x158
  CVector3f mTranslation; // 0x15c
  CVector3f mLocalScale; // 0x168
  float mTimeoutTimer; // 0x174
  bool mRenderAsParticleGen : 1;
  bool mEnableRender : 1;
  bool mIsElementGen : 1;
  bool mRunIndefinitely : 1;
  bool mUnknown4 : 1;
  bool mFinishing : 1; // 0x178 & 0x04
  int x17c_;
};
CHECK_SIZEOF(CHUDBillboardEffect, 0x180)

#endif // _CHUDBILLBOARDEFFECT
