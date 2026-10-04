#ifndef _CSTATEMANAGERCONTAINER
#define _CSTATEMANAGERCONTAINER

#include "types.h"

#include "Kyoto/TOneStatic.hpp"
#include "MetroidPrime/CActorModelParticles.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CEnvFxManager.hpp"
#include "MetroidPrime/CFluidPlaneManager.hpp"
#include "MetroidPrime/CHintManager.hpp"
#include "MetroidPrime/CRumbleManager.hpp"
#include "MetroidPrime/CSafeZoneManager.hpp"
#include "MetroidPrime/CScriptObjectLoaderHelper.hpp"
#include "MetroidPrime/CSortedLists.hpp"
#include "MetroidPrime/CWeaponMgr.hpp"
#include "MetroidPrime/Cameras/CCameraShakerManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed class/member names, correlated with Prime's StateManager backing block.
class CStateManagerContainer : public TOneStatic< CStateManagerContainer > {
public:
  CStateManagerContainer();

private:
  friend class CStateManager;

  CCameraManager mCameraManager0;
  CCameraManager mCameraManager1;
  CCameraManager mCameraManager2;
  CCameraManager mCameraManager3;
  SL::CSortedListManager mSortedListManager;
  CWeaponMgr mWeaponManager;
  CFluidPlaneManager mFluidPlaneManager;
  CEnvFxManager mEnvFxManager;
  CActorModelParticles mActorModelParticles;
  CSafeZoneManager mSafeZoneManager;
  CRumbleManager mRumbleManager0;
  CRumbleManager mRumbleManager1;
  CRumbleManager mRumbleManager2;
  CRumbleManager mRumbleManager3;
  CScriptObjectLoaderHelper mScriptObjectLoader;
  // Descriptive names derived from native render-phase profiling strings.
  rstl::reserved_vector< TUniqueId, 20 > mRenderBeforeAreas;
  rstl::reserved_vector< TUniqueId, 20 > mRenderFirstSorted;
  rstl::reserved_vector< TUniqueId, 20 > mRenderLast;
  rstl::reserved_vector< TUniqueId, 20 > mRenderLastUnderGun;
  rstl::reserved_vector< TUniqueId, 20 > mRenderLastAfterCameraFilters;
};
CHECK_SIZEOF(CStateManagerContainer, 0x13fb4)

#endif // _CSTATEMANAGERCONTAINER
