#ifndef _CSAFEZONEMANAGER
#define _CSAFEZONEMANAGER

#include "types.h"

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;
class CSphere;
class CStateManager;
class CTransform4f;

class CSafeZoneManager {
public:
  CSafeZoneManager();

  void AddOrUpdateSafeZone(CStateManager& mgr, const TUniqueId& id, const CVector3f& position,
                           const CVector3f& halfExtents, float scaleFactor);
  void RemoveSafeZone(const TUniqueId& id);
  bool IsObjectInSafeZone(const CActor& actor, const CStateManager& mgr) const;
  bool IsObjectInHurtfulSafeZone(const CActor& actor, const CStateManager& mgr) const;
  bool PointIsInSafeZone(const CStateManager& mgr, const CVector3f& point) const;
  bool PointIsInHurtfulSafeZone(const CStateManager& mgr, const CVector3f& point) const;
  TUniqueId PointIsInWhichSafeZone(const CStateManager& mgr, const CVector3f& point) const;
  TUniqueId SphereTouchingWhichSafeZone(const CStateManager& mgr, const CSphere& sphere) const;

  // Reconstructed names/signatures for the unexported native operations.
  void Update(float dt, CStateManager& mgr);
  void Render(CStateManager& mgr) const;
  float GetDarkWorldFilterAmount(const CTransform4f& cameraTransform) const;

private:
  // Reconstructed record/member names; the native record has no established export.
  struct SZone {
    SZone(const TUniqueId& id, const CVector3f& position, const CVector3f& halfExtents,
          float scaleFactor);

    TUniqueId mId;
    CVector3f mPosition;
    CVector3f mHalfExtents;
    float mScaleFactor;
  };

  int FindSafeZone(const TUniqueId& id) const;

  rstl::reserved_vector< SZone, 64 > mZones;
};
CHECK_SIZEOF(CSafeZoneManager, 0x804)

#endif // _CSAFEZONEMANAGER
