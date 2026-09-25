#ifndef _CPARTICLEDATABASE
#define _CPARTICLEDATABASE

// TODO: check for Echoes

#include "types.h"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/map.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"
#include "rstl/optional_object.hpp"

class CElectricDescription;
class CGenDescription;
class CParticleGenInfo;
class CStateManager;
class CSwooshDescription;
class CAABox;

class CParticleDatabase {
public:
  typedef rstl::map< rstl::string, rstl::auto_ptr< CParticleGenInfo > > DrawMap;

  void SetParticleEffectState(const rstl::string& name, bool active, CStateManager& mgr);
  void RenderSystemsToBeDrawnFirst() const;
  void RenderSystemsToBeDrawnLast() const;
  void DeleteAllLights(CStateManager& mgr);
  rstl::optional_object<CAABox> GetBounds() const;

  bool AreAnySystemsDrawnWithModel() const { return mAnySystemsDrawnWithModel; }

private:
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CGenDescription > > > mParticleDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CSwooshDescription > > > mSwooshDescs;
  rstl::map< CAssetId, rstl::rc_ptr< TLockedToken< CElectricDescription > > > mElectricDescs;
  DrawMap mRendererDrawLoop;
  DrawMap mFirstDrawLoop;
  DrawMap mLastDrawLoop;
  DrawMap mRendererDraw;
  DrawMap mFirstDraw;
  DrawMap mLastDraw;
  bool mUpdatesEnabled : 1;
  bool mAnySystemsDrawnWithModel : 1;
};
CHECK_SIZEOF(CParticleDatabase, 0xb8)

#endif // _CPARTICLEDATABASE
