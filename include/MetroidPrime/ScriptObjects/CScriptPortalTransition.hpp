#ifndef _CSCRIPTPORTALTRANSITION
#define _CSCRIPTPORTALTRANSITION

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CEntity.hpp"

#include "rstl/single_ptr.hpp"

class CPortalTransition;
class CStateManager;

// Guessed name. The scripted settings and connections for a portal transition.
class CScriptPortalTransition : public CEntity {
public:
  CScriptPortalTransition(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                          int direction, const CAnimRes& samusRes, CAssetId soundGroupCommon,
                          CAssetId soundGroupDirectional, ushort startPortal, ushort inPortal1,
                          ushort inPortal2, uchar volume, uchar pan);

  // CEntity
  ~CScriptPortalTransition() override;
  CEntity* TypesMatch(int typeId) const override;

  // Guessed name.
  rstl::single_ptr< CPortalTransition > CreateTransition(CStateManager& mgr) const;

private:
  int mDirection;
  CAnimRes mSamusRes;
  CAssetId mSoundGroupCommon;
  CAssetId mSoundGroupDirectional;
  ushort mStartPortal;
  ushort mInPortal1;
  ushort mInPortal2;
  uchar mVolume;
  uchar mPan;
};
CHECK_SIZEOF(CScriptPortalTransition, 0x54)

#endif // _CSCRIPTPORTALTRANSITION
