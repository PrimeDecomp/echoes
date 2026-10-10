#ifndef _CINGEXITHOSTEFFECT
#define _CINGEXITHOSTEFFECT

#include "types.h"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/PathFinding/CPathFindSearch.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"

// Guessed class: the swarm of Ing that streams out of the host the Ing is leaving. It follows a
// curved arc to the exit position and then lingers until its particles die out.
class CIngExitHostEffect : public CActor {
public:
  CIngExitHostEffect(TUniqueId uid, const CEntityInfo& info, const CTransform4f& xf,
                     const CVector3f& exitPosition, const CVector3f& scale, CAssetId swarmEffect,
                     CAssetId trailEffect, float trailLength, float speed, TUniqueId targetId,
                     float homingTime, float homingStrength, ushort sound);

  // CEntity
  ~CIngExitHostEffect() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  // CIngExitHostEffect
  bool HasArrived() const { return mArrived; } // Guessed name

private:
  void RemoveSfxEmitter();                             // Guessed name
  void UpdateSfxEmitter();                             // Guessed name
  void BuildArc(CStateManager& mgr);                   // Guessed name
  float CalculateArcLength() const;                    // Guessed name
  CVector3f RandomArcOffset(CStateManager& mgr) const; // Guessed name
  void UpdateMovement(float dt, CStateManager& mgr);   // Guessed name
  void UpdateParticles(float dt, CStateManager& mgr);  // Guessed name
  CParticleGen* CreateParticle(const CVector3f& scale, const CVector3f& translation,
                               const CToken& token); // Guessed name

  CTransform4f mSpawnTransform;                            // Guessed name
  CPathFindSearch mPathFindSearch;                         // Guessed name
  CVector3f mExitPosition;                                 // Guessed name
  CVector3f mScale;                                        // Guessed name
  float mElapsedTime;                                      // Guessed name
  float mSpeed;                                            // Guessed name
  TUniqueId mTargetId;                                     // Guessed name
  float mHomingTime;                                       // Guessed name
  float mHomingTimeRemaining;                              // Guessed name
  float mHomingStrength;                                   // Guessed name
  CToken mSwarmToken;                                      // Guessed name
  rstl::auto_ptr< CParticleGen > mSwarmParticle;           // Guessed name
  CToken mTrailToken;                                      // Guessed name
  rstl::auto_ptr< CParticleGen > mTrailParticle;           // Guessed name
  float mTrailLength;                                      // Guessed name
  rstl::reserved_vector< CVector3f, 4 > mArcPoints;        // Guessed name
  float mArcLength;                                        // Guessed name
  float mArcSpeedScale;                                    // Guessed name
  rstl::reserved_vector< CVector3f, 60 > mPositionHistory; // Guessed name
  int mHistoryIndex;                                       // Guessed name
  CSfxHandle mSfxHandle;                                   // Guessed name
  ushort mSound;                                           // Guessed name
  bool mArrived : 1;                                       // Guessed name
};
CHECK_SIZEOF(CIngExitHostEffect, 0x5e8)

#endif // _CINGEXITHOSTEFFECT
