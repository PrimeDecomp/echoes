#ifndef _CEXPLOSION
#define _CEXPLOSION

#include "MetroidPrime/CEffect.hpp"
#include "rstl/single_ptr.hpp"

class CParticleGen;
class CGenDescription;
class CElectricDescription;

class CExplosion : public CEffect {
public:
  CExplosion(const TLockedToken< CGenDescription >& particle, TUniqueId uid,
             const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf, uint flags,
             const CVector3f& scale, const CColor& color, int playerIndex);
  CExplosion(const TLockedToken< CElectricDescription >& electric, TUniqueId uid,
             const CEntityInfo& info, const rstl::string& name, const CTransform4f& xf, uint flags,
             const CVector3f& scale, const CColor& color, int playerIndex);

  // CEntity
  ~CExplosion() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

private:
  rstl::single_ptr< CParticleGen > mParticleGen;
  TUniqueId mExplosionLight;
  CAssetId mSourceId;
  CVector3f mScale;
  uint mFlags;
  int mPlayerIndex; // Guessed name; suppresses the owner's first-person rendering.
  bool mHasRenderBounds : 1;
  bool mUnknownFlag : 1;
  bool mFixedTimeStep : 1; // Guessed name; updates the generator at 1/60 second per call.
  float mTime;
};
CHECK_SIZEOF(CExplosion, 0x180)

#endif // _CEXPLOSION
