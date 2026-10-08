#ifndef _CFLYERSWARM
#define _CFLYERSWARM

#include "MetroidPrime/Enemies/CSwarmBasics.hpp"

#include "rstl/reserved_vector.hpp"

// Guessed name: a swarm of flying boids that follow the waypoint path instead of crawling on
// surfaces. The class name is corroborated by the Wii TypesMatch export.
class CFlyerSwarm : public CSwarmBasics {
public:
  CFlyerSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CVector3f& boundingBoxExtent, const CTransform4f& xf, const CAnimRes& animRes,
              CActorParameters actorParameters, const CBasicSwarmData& data, bool active,
              float surfaceAvoidance, float initialMoveSpeedModifier,
              float initialMoveSpeedModifierTime, float spawnSpread, float rollUprightSpeed,
              float rollUprightMinAngle);

  // CEntity
  ~CFlyerSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void Render(const CStateManager& mgr) const override;

  // CSwarmBasics
  void CreateBoid(CStateManager& mgr, int index) override;
  void UpdateBoid(CAreaCollisionCache& cache, CStateManager& mgr, float dt, CBoid& boid,
                  int partitionIndex) override;
  void ApplySteeringBehaviors(CStateManager& mgr, CBoid& boid, CVector3f& ahead,
                              const rstl::reserved_vector< CBoid*, 50 >& nearList) override;
  bool ShouldBuildAreaCollisionCacheForPartition(int partitionIndex) const override;

private:
  float mSurfaceAvoidance;                                   // Guessed name
  float mInitialMoveSpeedModifier;                           // Guessed name
  float mInitialMoveSpeedModifierTime;                       // Guessed name
  float mSpawnSpread;                                        // Guessed name; degrees
  float mRollUprightSpeed;                                   // Guessed name; radians per second
  float mRollUprightMinAngle;                                // Guessed name; radians
  rstl::reserved_vector< bool, 125 > mPartitionHasCollision; // Guessed name
};
CHECK_SIZEOF(CFlyerSwarm, 0x638)

#endif // _CFLYERSWARM
