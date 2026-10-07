#ifndef _CSCRIPTDESTRUCTIBLEBARRIER
#define _CSCRIPTDESTRUCTIBLEBARRIER

#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CVector3i.hpp"
#include "Kyoto/TFunctor.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "WorldFormat/COBBTree.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCollidableOBBTreeGroup;
class CElementGen;
class CModel;
class CModelFlags;

struct SBarrierSection;

// Guessed name. Health grid of the barrier's chunks; x runs along the width, y along the depth and
// z upwards.
class CBarrierChunkGrid {
public:
  typedef TFunctor1< const CVector3i& > ChunkCallback;

  CBarrierChunkGrid(const CVector3i& dims, const CVector3f& chunkSize, float chunkHealth);

  rstl::optional_object< SBarrierSection > SplitOffTop();
  rstl::optional_object< CVector3i > GetChunkAt(const CVector3f& pos, float radius) const;
  bool ApplyDamage(CStateManager& mgr, const CVector3f& pos, float damage, float radius,
                   const rstl::optional_object< ChunkCallback >& callback);
  rstl::pair< bool, float > DamageChunk(int x, int y, int z, float damage);
  static int SplitBox(const CVector3i& min, const CVector3i& size, CVector3i& minA,
                      CVector3i& sizeA, CVector3i& minB, CVector3i& sizeB);
  void AddBoxGeometry(COBBTree::SIndexData& data, rstl::vector< ushort >& surfaces,
                      const CVector3f& center, const CVector3f& extent) const;
  COBBTree::CNode* BuildBoxNode(COBBTree::SIndexData& data, const CVector3i& min,
                                const CVector3i& size) const;
  COBBTree::CNode* BuildRowNode(COBBTree::SIndexData& data, const CVector3i& row) const;
  COBBTree::CNode* BuildNode(COBBTree::SIndexData& data, const CVector3i& min,
                             const CVector3i& size) const;
  rstl::auto_ptr< CCollidableOBBTreeGroup > BuildCollision(const CMaterialList& material) const;
  void Render(const CTransform4f& xf, const CModelFlags& flags, const CModel* left,
              const CModel* center, const CModel* right, const CModel* fullRow,
              const CModel* fullLayer) const;

  const CAABox& GetBounds() const { return mBounds; }
  int GetChunkIndex(int x, int y, int z) const { return x + (y + z * mDims.GetY()) * mDims.GetX(); }
  float& ChunkHealth(int x, int y, int z) { return mChunkHealths[GetChunkIndex(x, y, z)]; }
  int GetNumDestroyed() const { return mNumDestroyed; }

  CVector3f GetExtent(const CVector3i& size) const {
    return CVector3f(size.GetX() * mChunkSize.GetX(), size.GetY() * mChunkSize.GetY(),
                     size.GetZ() * mChunkSize.GetZ());
  }

  CVector3i mDims;
  CVector3f mChunkSize;
  CVector3f mInvChunkSize;
  CAABox mBounds;
  int mNumDestroyed;
  float mChunkHealth;
  rstl::vector< int > mLayerCounts;
  rstl::vector< int > mRowCounts;
  rstl::vector< float > mChunkHealths;
};
CHECK_SIZEOF(CBarrierChunkGrid, 0x74)

// Guessed name. A disconnected top part of the barrier that falls as a separate actor.
struct SBarrierSection {
  SBarrierSection(CBarrierChunkGrid* grid, const CVector3f& spawnPos, const CVector3f& pivot,
                  int layer)
  : mGrid(grid), mSpawnPos(spawnPos), mPivot(pivot), mLayer(layer), mActorId(kInvalidUniqueId) {}

  rstl::auto_ptr< CBarrierChunkGrid > mGrid;
  CVector3f mSpawnPos;
  CVector3f mPivot;
  int mLayer;
  TUniqueId mActorId;
};
CHECK_SIZEOF(SBarrierSection, 0x28)

class CScriptDestructibleBarrier : public CPhysicsActor {
public:
  CScriptDestructibleBarrier(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CActorParameters& actParms,
                             const CHealthInfo& healthInfo, const CDamageVulnerability& dVuln,
                             const CVector3i& dims, const CVector3f& chunkSize,
                             const CModelData& leftModel, const CModelData& centerModel,
                             const CModelData& rightModel, const CModelData& fullRowModel,
                             const CModelData& fullLayerModel, const CModelData& baseModel,
                             CAssetId particle1, int particle1Count, CAssetId particle2,
                             int particle2Count, CAssetId particle3, int particle3Count,
                             CAssetId particle4, int particle4Count, int sfxChunkGenerated,
                             int sfxChunkDestroyed, int sfxMoveDown, int sfxMoveUp, int sfxStop,
                             bool startLowered, float lowerPercent, float lowerDelay,
                             float moveSpeed);

  // CEntity
  ~CScriptDestructibleBarrier();
  CEntity* TypesMatch(int typeId) const override;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  CHealthInfo* HealthInfo() override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;
  CVector3f GetOrbitPosition(const CStateManager& mgr) const override;
  CVector3f GetAimPosition(const CStateManager& mgr, float dt) const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;

  void OnChunkDestroyed(const CVector3i& chunk);
  void TakeDamage(CStateManager& mgr);
  void SetDamageVulnerability(const CDamageVulnerability& dVuln);
  void UpdateTransforms();
  void RenderChunks(const CStateManager& mgr) const;

private:
  CHealthInfo mHealthInfo;
  CHealthInfo mChunkHealthInfo;
  CDamageVulnerability mDamageVulnerability;
  CBarrierChunkGrid mGrid;
  rstl::auto_ptr< CCollidableOBBTreeGroup > mCollision;
  CVector3i mDims;
  CVector3f mChunkSize;
  CModelData mLeftModel;
  CModelData mCenterModel;
  CModelData mRightModel;
  CModelData mFullRowModel;
  CModelData mFullLayerModel;
  CModelData mBaseModel;
  rstl::single_ptr< CElementGen > mParticle1;
  int mParticle1Count;
  rstl::single_ptr< CElementGen > mParticle2;
  int mParticle2Count;
  rstl::single_ptr< CElementGen > mParticle3;
  int mParticle3Count;
  rstl::single_ptr< CElementGen > mParticle4;
  int mParticle4Count;
  int mSfxChunkGenerated;
  int mSfxChunkDestroyed;
  int mSfxMoveDown;
  int mSfxMoveUp;
  int mSfxStop;
  float mLowerPercent;
  float mLowerDelay;
  float mMoveSpeed;
  float mTimeSinceDamage;
  float mTimeSinceDamage2;
  float mLowerOffset;
  float mHeight;
  CTransform4f mRenderXf;
  CTransform4f mInvRenderXf;
  CAABox mTouchBounds;
  rstl::reserved_vector< SBarrierSection, 4 > mSections;
  CSfxHandle mMoveSfx;
  int mState;
  int mTargetState;
  uint mTouchedByPlayer : 1;
  uint mHasTransparency : 1;
  uint mPlayedSfx : 1;
};
CHECK_SIZEOF(CScriptDestructibleBarrier, 0x718)

#endif // _CSCRIPTDESTRUCTIBLEBARRIER
