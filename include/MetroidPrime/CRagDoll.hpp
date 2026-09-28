#ifndef _CRAGDOLL
#define _CRAGDOLL

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CActor;
class CCharLayoutInfo;
class CGameProjectile;
class CJointData_LinearStorage;
class CModelData;
class CProjectileTouchResult;
class CStateManager;

class CRagDoll {
public:
  virtual void Prime(CStateManager& mgr, const CTransform4f& xf, CModelData& modelData);
  virtual void Update(CStateManager& mgr, float dt, float waterTop);
  virtual void PreRender(const CVector3f& pos, CModelData& modelData);
  virtual void CheckStatic(float dt);

  CRagDoll(float normalGravity, float floatingGravity, float overTime, float damping,
           float restitution, uint flags);
  ~CRagDoll() {}

  CAABox CalculateRenderBounds() const;
  void PreRenderAllViewports(CActor& actor, float extent);
  // Guessed names.
  CAABox GetRenderBounds() const;
  void UpdateRenderBounds();
  CProjectileTouchResult ProjectileCollision(const CGameProjectile& projectile, TUniqueId actorId);

  bool IsPrimed() const { return mPrimed; }
  bool IsOver() const { return mOver; }
  bool WillContinueSmallMovements() const { return mContinueSmallMovements; }
  void SetContinueSmallMovements(bool value) { mContinueSmallMovements = value; }
  void SetNoOverTimer(bool value) { mNoOverTimer = value; }
  uint GetImpactCount() const { return mImpactCount; }

protected:
  class CRagDollParticle {
    friend class CRagDoll;

  public:
    CRagDollParticle(const CSegId& id, const CVector3f& curPos, float radius,
                     const CVector3f& prevPos)
    : mId(id)
    , mCurPos(curPos)
    , mRadius(radius)
    , mPrevPos(prevPos)
    , mAcceleration(CVector3f::Zero())
    , mImpactResponseDelta(CVector3f::Zero())
    , mImpactFrameVel(0.f)
    , mDamping(1.f)
    , mImpactPending(false)
    , x40_25_(false) {}

    const CSegId& GetBone() const { return mId; }
    const CVector3f& GetPosition() const { return mCurPos; }
    CVector3f& Position() { return mCurPos; }
    float GetRadius() const { return mRadius; }

  private:
    CSegId mId;
    CVector3f mCurPos;
    float mRadius;
    CVector3f mPrevPos;
    CVector3f mAcceleration;
    CVector3f mImpactResponseDelta;
    float mImpactFrameVel;
    float mDamping;
    bool mImpactPending : 1;
    bool x40_25_ : 1;
  };

  class CRagDollLengthConstraint {
  public:
    // Guessed names for the target's constraint modes.
    enum EInequality { kI_Equal, kI_Minimum, kI_Maximum };

    CRagDollLengthConstraint(CRagDollParticle* p1, CRagDollParticle* p2, float length,
                             EInequality inequality)
    : mP1(p1), mP2(p2), mLength(length), mInequality(inequality) {}

    void Update();
    float GetLength() const { return mLength; }

  private:
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    float mLength;
    EInequality mInequality;
  };

  class CRagDollJointConstraint {
  public:
    CRagDollJointConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                            CRagDollParticle* p4, CRagDollParticle* p5, CRagDollParticle* p6)
    : mP1(p1), mP2(p2), mP3(p3), mP4(p4), mP5(p5), mP6(p6) {}

    void Update();

  private:
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    CRagDollParticle* mP3;
    CRagDollParticle* mP4;
    CRagDollParticle* mP5;
    CRagDollParticle* mP6;
  };

  class CRagDollPlaneConstraint {
  public:
    CRagDollPlaneConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                            CRagDollParticle* p4, CRagDollParticle* p5)
    : mP1(p1), mP2(p2), mP3(p3), mP4(p4), mP5(p5) {}

    void Update();

  private:
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    CRagDollParticle* mP3;
    CRagDollParticle* mP4;
    CRagDollParticle* mP5;
  };

  // Guessed name, correlated with the exported AddKneeConstraint.
  class CRagDollKneeConstraint {
  public:
    CRagDollKneeConstraint(CRagDollParticle* p1, CRagDollParticle* p2, CRagDollParticle* p3,
                           CRagDollParticle* p4, float minimumDistance)
    : mP1(p1), mP2(p2), mP3(p3), mP4(p4), mMinimumDistance(minimumDistance) {}

    void Update();

  private:
    CRagDollParticle* mP1;
    CRagDollParticle* mP2;
    CRagDollParticle* mP3;
    CRagDollParticle* mP4;
    float mMinimumDistance;
  };

  void SetNumParticles(int num) { mParticles.reserve(num); }
  void SetNumLengthConstraints(int num) { mLengthConstraints.reserve(num); }
  void SetNumJointConstraints(int num) { mJointConstraints.reserve(num); }
  void SetNumKneeConstraints(int num) { mKneeConstraints.reserve(num); }
  void AddParticle(const CSegId& id, const CVector3f& prevPos, const CVector3f& curPos,
                   float radius);
  void AddLengthConstraint(int i1, int i2);
  void AddMinLengthConstraint(int i1, int i2, float length);
  void AddMaxLengthConstraint(int i1, int i2, float length);
  void AddJointConstraint(int i1, int i2, int i3, int i4, int i5, int i6);
  void AddKneeConstraint(int i1, int i2, int i3, int i4, float minimumDistance);
  CQuaternion BoneAlign(CJointData_LinearStorage& pose, const CCharLayoutInfo& layout, int i1,
                        int i2, const CQuaternion& rotation);
  void CalfAlign(CJointData_LinearStorage& pose, int i1, int i2, int i3,
                 const CVector3f& planeNormal);
  void SatisfyWorldConstraintsOnConstruction(CStateManager& mgr);
  void AccumulateForces(float dt, float waterTop);
  void Verlet(float dt);
  void SatisfyConstraints(CStateManager& mgr);
  bool SatisfyWorldConstraints(CStateManager& mgr, int pass);
  void ClearForces();
  float GetConstraintLength(int index) const { return mLengthConstraints[index].GetLength(); }

  rstl::vector< CRagDollLengthConstraint > mLengthConstraints;
  rstl::vector< CRagDollJointConstraint > mJointConstraints;
  rstl::vector< CRagDollPlaneConstraint > mPlaneConstraints;
  rstl::vector< CRagDollKneeConstraint > mKneeConstraints;
  float mNormalGravity;
  float mFloatingGravity;
  float mDamping;
  float mRestitution;
  float mAngTimer;
  CAABox mRenderBounds;
  bool mRenderBoundsValid : 1;
  bool mPrimed : 1;
  bool mContinueSmallMovements : 1;
  bool mNoAiCollision : 1;
  bool mHitByProjectile : 1;
  bool mNoOverTimer : 1;
  bool mPrevMovingSlowly : 1;
  bool mOver : 1;
  float mOverTimer;
  uint mImpactCount;
  float mStaticSpeedThreshold;
  float mImpactVel;
  CVector3f mAverageVel;
  rstl::vector< CRagDollParticle > mParticles;
};
CHECK_SIZEOF(CRagDoll, 0xa0)
NESTED_CHECK_SIZEOF(CRagDoll, CRagDollParticle, 0x44)
NESTED_CHECK_SIZEOF(CRagDoll, CRagDollLengthConstraint, 0x10)
NESTED_CHECK_SIZEOF(CRagDoll, CRagDollJointConstraint, 0x18)
NESTED_CHECK_SIZEOF(CRagDoll, CRagDollPlaneConstraint, 0x14)
NESTED_CHECK_SIZEOF(CRagDoll, CRagDollKneeConstraint, 0x14)

#endif // _CRAGDOLL
