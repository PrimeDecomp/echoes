#ifndef _CSNAKEWEEDSWARM
#define _CSNAKEWEEDSWARM

#include "types.h"

#include "Kyoto/Animation/CSkinnedModel.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CDamageInfo.hpp"

#include "Kyoto/Math/CVector2i.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CAnimRes;
class CElementGen;
class CGenDescription;

// Guessed class: a patch of snake weeds that rises from the ground on a grid, lowers when
// disturbed, and damages a player who stands among raised weeds. The class name is corroborated
// by the Wii SEL export of TypesMatch; the rest of the layout comes from the GameCube constructor.
class CSnakeWeedSwarm : public CActor {
public:
  enum EBoidPlacement { kBP_None, kBP_Ready, kBP_Invalid, kBP_Placed }; // Guessed names

  class CBoid {
  public:
    enum ESnakeWeedBoidState {
      kSWBS_Raised,
      kSWBS_Raising,
      kSWBS_Lowered,
      kSWBS_Lowering
    }; // Guessed names

    CBoid(const CVector3f& pos, float depth, float speed, float scale);
    void SetBoidState(ESnakeWeedBoidState state);
    ESnakeWeedBoidState GetBoidState() const;
    const CVector3f& GetLocation() const;
    float GetTimeOut() const;
    void SetTimeOut(float time);
    float GetDepth() const;
    void SetDepth(float depth);
    float GetSpeed() const;
    void SetSpeed(float speed);
    float GetSize() const;

  private:
    CVector3f mPos;             // Guessed name
    ESnakeWeedBoidState mState; // Guessed name
    float mTimeOut;             // Guessed name
    float mDepth;               // Guessed name
    float mSpeed;               // Guessed name
    float mUnknown1C_;          // Unknown; sits between the speed and size fields
    float mSize;                // Guessed name
  };

  CSnakeWeedSwarm(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CVector3f& pos, const CVector3f& scale, const CAnimRes& animRes,
                  const CActorParameters& actParms, float spacing, float height, float variance,
                  float weaponDamageRadius, float maxPlayerDistance, float loweredTime,
                  float loweredTimeVariation, float maxDepth, float speed, float speedVariation,
                  float slopeAngle, float scaleMin, float scaleMax, float distanceBelowGround,
                  const CDamageInfo& damageInfo, float f15, uint sfxId1, uint sfxId2, uint sfxId3,
                  uint particle1, uint particleCount, uint particle2, float f16);

  // CEntity
  ~CSnakeWeedSwarm() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CSnakeWeedSwarm
  void ApplyRadiusDamage(CVector3f pos, const CDamageInfo& info, CStateManager& mgr);
  void ScareSnakeWeeds(CStateManager& mgr, const CVector3f& pos, float radius);

private:
  void InitAnimBoids(CStateManager& mgr, CModelData::EWhichModel which);
  CAABox GetBoundingBox() const;
  void UpdateTouchBounds();
  void RenderBoid(uint index, const CBoid& boid, uint& posesToBuild) const;
  void CreateSwarm(CStateManager& mgr);
  CVector2i GetGridPosition(CVector3f pos);
  int GetGridWidth();
  int GetGridDepth();
  void PushBackPoint(CVector3f pos);
  void FloodFill(CStateManager& mgr, int count);
  float GetXVariance(const CVector3f& pos);
  float GetYVariance(const CVector3f& pos);
  bool PlaceSnakeWeedAtPoint(const CVector3f& pos, CStateManager& mgr);
  void AddContinuousParticles(const CVector3f& pos);
  void AddRetreatParticles(const CVector3f& pos);

  CVector3f mScale;             // Guessed name
  float mBoidSpacing;           // Guessed name
  float mHeight;                // Guessed name
  float mVariance;              // Guessed name
  float mWeaponDamageRadius;    // Guessed name
  float mMaxPlayerDistance;     // Guessed name
  float mLoweredTime;           // Guessed name
  float mLoweredTimeVariation;  // Guessed name
  float mMaxDepth;              // Guessed name
  float mSpeed;                 // Guessed name
  float mSpeedVariation;        // Guessed name
  float mCosSlopeAngle;         // Guessed name
  float mScaleMin;              // Guessed name
  float mScaleMax;              // Guessed name
  float mDistanceBelowGround;   // Guessed name
  uint x19c_;                   // Unknown; zeroed by the constructor
  rstl::vector< CBoid > mBoids; // Guessed name
  bool mHasGround : 1;          // Guessed name
  uchar mPlayerTouchMask;       // Guessed name; one bit per player touching the weeds
  CAABox mTouchBounds;          // Guessed name
  CDamageInfo mDamageInfo;      // Guessed name
  mutable rstl::reserved_vector< CSkinnedModelState, 4 >
      mSkinnedModelStates; // Guessed name; one stored pose per model
  rstl::reserved_vector< rstl::ncrc_ptr< CModelData >, 4 > mModelData; // Guessed name
  CModelData::EWhichModel mWhich;                                      // Guessed name
  rstl::single_ptr< rstl::vector< CVector3f > > mBoidPositions;        // Guessed name
  rstl::single_ptr< rstl::vector< EBoidPlacement > > mBoidPlacement;   // Guessed name
  ushort mSfx1;                                                        // Guessed name
  ushort mSfx2;                                                        // Guessed name
  ushort mSfx3;                                                        // Guessed name
  CSfxHandle mSfxHandle;                                               // Guessed name
  rstl::auto_ptr< TLockedToken< CGenDescription > > mParticleGenDescA; // Guessed name
  rstl::auto_ptr< TLockedToken< CGenDescription > > mParticleGenDescB; // Guessed name
  rstl::auto_ptr< CElementGen > mParticleGen1;                         // Guessed name
  rstl::auto_ptr< CElementGen > mParticleGen2;                         // Guessed name
  uint mParticleCount;                                                 // Guessed name
  float x200_;          // Unknown; constructor float argument
  float mParticleTimer; // Guessed name
};
NESTED_CHECK_SIZEOF(CSnakeWeedSwarm, CBoid, 0x24)
CHECK_SIZEOF(CSnakeWeedSwarm, 0x298)

#endif // _CSNAKEWEEDSWARM
