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

class CHUDBillboardEffect : public CEffect {
public:
  CHUDBillboardEffect(const rstl::optional_object< TToken< CGenDescription > >& particle,
                      const rstl::optional_object< TToken< CElectricDescription > >& electric,
                      TUniqueId uid, bool active, const rstl::string& name, float dist,
                      const CVector3f& scale0, int playerIndex, const CColor& color,
                      const CVector3f& scale1, const CVector3f& translation, bool adjustDepthRange);

  // CEntity
  ~CHUDBillboardEffect() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  CParticleGen* GetParticleGen() const { return mGenerator.get(); }
  void SetFinishing() { mFinishing = true; }

  static float GetNearClipDistance(const CStateManager& mgr, int playerIndex);
  static const CVector3f& GetScaleForPOV(const CStateManager& mgr);

private:
  static float CalcGenRate();

  static int g_BillboardCount;
  static int g_IndirectTexturedBillboardCount;

  rstl::single_ptr< CParticleGen > mGenerator; // 0x158
  uint mPlayerIndex;                           // 0x15c
  CVector3f mTranslation;                      // 0x160
  CVector3f mLocalScale;                       // 0x16c
  bool mRenderAsParticleGen : 1;
  bool mEnableRender : 1;
  bool mIsElementGen : 1;
  bool mRunIndefinitely : 1;
  bool mAdjustDepthRange : 1;
  bool mFinishing : 1;
  float mTimeoutTimer; // 0x17c
};
CHECK_SIZEOF(CHUDBillboardEffect, 0x180)

#endif // _CHUDBILLBOARDEFFECT
