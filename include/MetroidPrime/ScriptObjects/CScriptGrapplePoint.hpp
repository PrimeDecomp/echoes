#ifndef _CSCRIPTGRAPPLEPOINT
#define _CSCRIPTGRAPPLEPOINT

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CGrappleParameters.hpp"

// Original Wii export name and signature, independently correlated with the GameCube GRAP loader.
class CScriptGrapplePoint : public CActor {
public:
  CScriptGrapplePoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                      const CTransform4f& xf, const CGrappleParameters& parameters);

  // CEntity
  ~CScriptGrapplePoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  const CGrappleParameters& GetGrappleParameters() const { return mParameters; }

private:
  CGrappleParameters mParameters;
  CVector3f mPreviousPosition;
  uint mActivationFrame; // Guessed name; set on activation and XALD.
};
CHECK_SIZEOF(CScriptGrapplePoint, 0x198)

#endif // _CSCRIPTGRAPPLEPOINT
