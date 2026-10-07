#ifndef _CFISHCLOUD
#define _CFISHCLOUD

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/SwarmRenderHelpers.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CActorParameters;
class CGenDescription;

class CFishCloud : public CActor {
public:
  // Member names follow Prime's CFishCloud; Echoes-only members use guessed names.
  class CModifierSource {
  public:
    CModifierSource(const TUniqueId& source, bool repulsor, bool swirl, float radius,
                    float priority);
    void SetAffectPriority(float priority) { mPriority = priority; }
    void SetAffectRadius(float radius) { mRadius = radius; }
    float GetAffectPriority() const { return mPriority; }
    float GetAffectRadius() const { return mRadius; }
    bool IsRepulsor() const { return mIsRepulsor; }
    bool IsSwirl() const { return mIsSwirl; }
    const TUniqueId& GetSource() const { return mSource; }
    bool operator<(const CModifierSource& other) const;

    TUniqueId mSource;
    float mRadius;
    float mPriority;
    bool mIsRepulsor;
    bool mIsSwirl;
  };

  class CBoid {
    friend class CFishCloud;

  public:
    CBoid(const CVector3f& pos, const CVector3f& vel, float scale);
    CVector3f& Translation() { return mPos; }
    const CVector3f& GetTranslation() const { return mPos; }

  private:
    CVector3f mPos;
    CVector3f mVel;
    float mScale;
    CBoid* mNext;
    bool mActive;
  };

  CFishCloud(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
             const CVector3f& scale, const CTransform4f& xf, const CModelData& mData,
             const CAnimRes& aRes, int numBoids, float speed, float separationRadius,
             float cohesionMagnitude, float alignmentWeight, float separationMagnitude,
             float weaponRepelMagnitude, float playerRepelMagnitude, float containmentMagnitude,
             float scatterVel, float maxScatterAngle, float weaponRepelDampingSpeed,
             float playerRepelDampingSpeed, float playerBallPriority, float playerBallDistance,
             float containmentRadius, int updateShift, const CColor& color, bool killable,
             float weaponKillRadius, CAssetId part1, int partCount1, CAssetId part2, int partCount2,
             CAssetId part3, int partCount3, CAssetId part4, int partCount4, int deathSfx,
             bool repelFromThreats, const CActorParameters& actParms);
  ~CFishCloud() override;

  // CEntity
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  bool AddRepulsor(TUniqueId source, bool swirl, float radius, float priority);
  bool AddAttractor(TUniqueId source, bool swirl, float radius, float priority);
  void RemoveRepulsor(TUniqueId source);
  void RemoveAttractor(TUniqueId source);

private:
  typedef rstl::vector< CModifierSource > TModifierSourceVector;
  typedef rstl::vector< CBoid* > TBoidPtrVector;
  typedef rstl::vector< CBoid > TBoidVector;

  void InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which);
  CAABox GetBoundingBox() const;
  CAABox GetUntransformedBoundingBox() const;
  bool PointInBox(const CAABox& aabb, const CVector3f& point) const;
  CPlane FindClosestPlane(const CAABox& aabb, const CVector3f& point) const;
  CPlane FindFarthestPlane(const CAABox& aabb, const CVector3f& point) const; // Guessed name.
  void PlaceBoid(CStateManager& mgr, CBoid& boid, const CAABox& aabb);
  void CreatePartitionList();
  void RenderBoid(int idx, const CBoid& boid, uint& drawMask) const;
  void KillBoid(CBoid& boid);
  void UpdatePartitionList();
  CBoid* GetListAt(const CVector3f& pos);
  void BuildBoidNearList(const CVector3f& pos, float radius,
                         rstl::reserved_vector< CBoid*, 25 >& nearList);
  void OldBuildBoidNearList(const CVector3f& pos, float radius,
                            rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyRotation(CBoid& boid, float magnitude, const CVector3f& point, float radius,
                     bool clockwise);
  void ApplyAlignment(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyWander(CStateManager& mgr, CBoid& boid);
  void ApplyCohesion(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplyCohesion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplySeparation(CBoid& boid, const rstl::reserved_vector< CBoid*, 25 >& nearList);
  void ApplySeparation(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyAttraction(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyRepulsion(CBoid& boid, const CVector3f& point, float radius, float magnitude);
  void ApplyContainment(CBoid& boid, const CAABox& aabb);
  void AddParticles(const CVector3f& pos);
  void UpdateParticles(float dt);
  void RenderParticles() const;

  TBoidVector mBoids;                                                                   // 0x158
  TBoidPtrVector mBoidPartitionLists;                                                   // 0x168
  TModifierSourceVector mModifierSources;                                               // 0x178
  int mThinkCounter;                                                                    // 0x188
  int mUpdateMask;                                                                      // 0x18c
  CVector3f mScale;                                                                     // 0x190
  float mRandomMovementTimer;                                                           // 0x19c
  float mSpeed;                                                                         // 0x1a0
  int mNumBoids;                                                                        // 0x1a4
  float mSeparationRadius;                                                              // 0x1a8
  float mCohesionMagnitude;                                                             // 0x1ac
  float mAlignmentWeight;                                                               // 0x1b0
  float mSeparationMagnitude;                                                           // 0x1b4
  float mWeaponRepelMagnitude;                                                          // 0x1b8
  float mPlayerRepelMagnitude;                                                          // 0x1bc
  float mScatterVel;                                                                    // 0x1c0
  float mMaxScatterAngle;                                                               // 0x1c4
  float mContainmentMagnitude;                                                          // 0x1c8
  float mPlayerBallPriority;                                                            // 0x1cc
  float mPlayerBallDistance;                                                            // 0x1d0
  float mPlayerRepelDampingSpeed;                                                       // 0x1d4
  float mWeaponRepelDampingSpeed;                                                       // 0x1d8
  float mPlayerRepelDamping;                                                            // 0x1dc
  float mWeaponRepelDamping;                                                            // 0x1e0
  CColor mColor;                                                                        // 0x1e4
  float mWeaponKillRadius;                                                              // 0x1e8
  float mContainmentRadius;                                                             // 0x1ec
  rstl::reserved_vector< SwarmRenderHelpers::CSwarmSkinnedModelState, 4 > mModelStates; // 0x1f0
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 4 > mModels;                     // 0x284
  rstl::reserved_vector< TLockedToken< CGenDescription >, 4 > mParticleDescs;           // 0x2a8
  rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mParticleGens;              // 0x2dc
  rstl::reserved_vector< int, 4 > mDeathParticleCounts;                                 // 0x300
  rstl::single_ptr< SwarmRenderHelpers::CSwarmDisplayList > mDisplayList;               // 0x314
  ushort mDeathSfx;                                                                     // 0x318
  CVector3f mPartitionPitch;                                                            // 0x31c
  CVector3f mOoPartitionPitch;                                                          // 0x328
  bool mRandomMovement : 1;
  bool mWorldSpace : 1;
  bool mEnableWeaponRepelDamping : 1;
  bool mValidModel : 1;
  bool mKillable : 1;
  bool mRepelFromThreats : 1;
  bool mEnablePlayerRepelDamping : 1;
  bool mUpdateWithoutPartitions : 1;
};
CHECK_SIZEOF(CFishCloud, 0x338)

class CFishCloudModifier : public CActor {
public:
  CFishCloudModifier(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CVector3f& pos, bool isRepulsor, bool swirl, float radius,
                     float priority);
  ~CFishCloudModifier() override;

  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  void AddSelf(CStateManager& mgr);
  void RemoveSelf(CStateManager& mgr);

private:
  float mRadius;
  float mPriority;
  bool mIsRepulsor;
  bool mSwirl;
};
CHECK_SIZEOF(CFishCloudModifier, 0x168)

#endif // _CFISHCLOUD
