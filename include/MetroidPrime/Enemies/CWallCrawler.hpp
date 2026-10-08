#ifndef _CWALLCRAWLER
#define _CWALLCRAWLER

#include "types.h"

#include "Collision/CCollidableSphere.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

class CScriptWaypoint;

// Wii SEL class name (WallCrawler REL). Base class of the wall-walking enemies; Prime's CWallWalker
// is the closest relative. Member names follow Prime where the layout lines up.
class CWallCrawler : public CPatterned {
public:
  enum EType { kT_PillBug = 5, kT_WallWalker = 9 }; // Guessed names; other values are not recovered.

  CWallCrawler(EPatternedAI character, TUniqueId uid, const rstl::string& name, EFlavorType flavor,
               CEntityInfo& info, const CTransform4f& xf, const CModelData& mData,
               const CPatternedInfo& pInfo, EMovementType moveType, EColliderType colType,
               EBodyType bodyType, const CActorParameters& actParms, float sphereRadius,
               float collisionCloseMargin, float alignAngVel, float advanceWpRadius,
               float playerObstructionMinDist, EType type, bool disableMove, float f1, float f2,
               float f3, float f4, float f5, float f6);

  // CEntity
  ~CWallCrawler() override;
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;

  // CWallCrawler
  virtual TUniqueId GetNextWaypoint(CStateManager& mgr, const CScriptWaypoint* waypoint,
                                    bool reverse); // Guessed name; slot 0x148

  void SetConstraint(CStateManager& mgr, int constraint);
  void UpdateConstraintPlane(CStateManager& mgr);
  bool UpdateWPDestination(CStateManager& mgr);
  void AlignToFloor(CStateManager& mgr, float radius, const CVector3f& newPos, float dt);
  void AlignToPlane(const CUnitVector3f& normal, float dt);
  const CPlane& GetConstraintPlane() const;
  static CVector3f ProjectVectorToPlane(const CVector3f& vec, const CVector3f& planeDir);
  static CVector3f GetClosestPointOnPlane(const CVector3f& point, const CPlane& plane); // Guessed name

protected:
  CCollisionSurface mAlignSurface;
  CCollidableSphere mColSphere;
  float mCollisionCloseMargin;
  float mAlignAngVel;
  float mTumbleAngle;
  float mPatrolPauseRemTime;
  float mAdvanceWpRadius;
  float mPlayerObstructionMinDist;
  float mBendingHackWeight;
  EType mType;
  short mThinkCounter;
  int x834_;
  CVector3f x838_;
  float x844_;
  float x848_;
  float x84c_;
  float x850_;
  float x854_;
  float x858_;
  float x85c_;
  bool mAlignToFloor : 1;
  bool mHasAlignSurface : 1;
  bool mPlayerObstructed : 1;
  bool mDisableMove : 1;
  bool x860_27_ : 1;
  bool x860_28_ : 1;
  bool x860_29_ : 1;
  bool x860_30_ : 1;
  bool x861_24_ : 1;
};
CHECK_SIZEOF(CWallCrawler, 0x868)

#endif // _CWALLCRAWLER
