#ifndef _CSCRIPTSPIDERBALLWAYPOINT
#define _CSCRIPTSPIDERBALLWAYPOINT

#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"

#include "Kyoto/Math/CAABox.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CScriptSpiderBallWaypoint : public CScriptWaypoint {
public:
  enum ECheckActiveWaypoint {
    kCAW_Check,
    kCAW_SkipCheck,
  };

  CScriptSpiderBallWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                            const CTransform4f& xf, uint flags);

  // CEntity
  ~CScriptSpiderBallWaypoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  void ClearWaypoints();
  void BuildWaypointListAndBounds(CStateManager& mgr);
  void AddPreviousWaypoint(TUniqueId uid);
  void AddPointToTouchBounds(const CVector3f& point);
  TUniqueId NextWaypoint(const CStateManager& mgr, ECheckActiveWaypoint check) const;
  TUniqueId PreviousWaypoint(const CStateManager& mgr, ECheckActiveWaypoint check) const;
  void GetClosestPointAlongWaypoints(CStateManager& mgr, const CVector3f& ballPos,
                                     float maxPointToBallDist,
                                     const CScriptSpiderBallWaypoint** closestWaypoint,
                                     CVector3f& closestPoint, CVector3f& deltaBetweenPoints,
                                     float deltaBetweenInterpPoints,
                                     CVector3f& interpDeltaBetweenPoints) const;

private:
  uint mFlags;
  rstl::vector< TUniqueId > mWaypoints;
  rstl::optional_object< CAABox > mAabox;
};
CHECK_SIZEOF(CScriptSpiderBallWaypoint, 0x188)

#endif // _CSCRIPTSPIDERBALLWAYPOINT
