#ifndef _CSCRIPTPOINTOFINTEREST
#define _CSCRIPTPOINTOFINTEREST

#include "MetroidPrime/CActor.hpp"

class CScannableParameters;

// Prime's CScriptPointOfInterest: an invisible scannable point. Echoes replaces Prime's active
// flag with the editor's look-at flag and fixes the render bounds. Member names are unverified.
class CScriptPointOfInterest : public CActor {
public:
  CScriptPointOfInterest(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                         const CTransform4f& xf, const CScannableParameters& scanParms, bool lookAt,
                         float scanOffset);

  // CEntity
  ~CScriptPointOfInterest() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  bool GetLookAt() const { return mLookAt; }

private:
  float mScanOffset;
  bool mLookAt : 1;
};
CHECK_SIZEOF(CScriptPointOfInterest, 0x160)

#endif // _CSCRIPTPOINTOFINTEREST
