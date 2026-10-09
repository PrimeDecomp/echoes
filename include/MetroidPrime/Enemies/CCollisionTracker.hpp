#ifndef _CCOLLISIONTRACKER
#define _CCOLLISIONTRACKER

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CFoldySurface.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TFunctor.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CEffect.hpp"
#include "WorldFormat/CCollisionCache.hpp"

#include "rstl/construct.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CElementGen;
class CGenDescription;

// Guessed name: one straight piece of a path that was laid along collision surfaces.
struct SSurfacePathSegment {
  SSurfacePathSegment(const CQuaternion& orientation, float length, const CVector3f& position,
                      const CVector3f& direction);

  CQuaternion mOrientation;
  CMatrix3f mTransform;
  CVector3f mPosition;
  CVector3f mDirection;
  float mLength;
};
CHECK_SIZEOF(SSurfacePathSegment, 0x50)
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(SSurfacePathSegment)
RSTL_DECLARE_BITWISE_CONSTRUCTION(SSurfacePathSegment)
} // namespace rstl

// Guessed name: the triangle payload of a collision-cache slot.
struct STriangleRecord {
  STriangleRecord(const CCollisionSurface& surface, ushort index)
  : mSurface(surface), mIndex(index) {}

  CCollisionSurface mSurface;
  ushort mIndex;
};

// Guessed name: a collision triangle with its plane, laid out like a packed collision-cache slot.
struct STriangleSlot {
  STriangleSlot(const CCollisionSurface& surface, ushort index)
  : mTriangle(surface, index)
  , mPlane(mTriangle.mSurface.GetVert(0), mTriangle.mSurface.GetVert(1),
           mTriangle.mSurface.GetVert(2)) {}

  STriangleRecord mTriangle;
  CPlane mPlane;
};
CHECK_SIZEOF(STriangleSlot, 0x48)
namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(STriangleSlot)
RSTL_DECLARE_BITWISE_CONSTRUCTION(STriangleSlot)
} // namespace rstl

// Guessed name: a polyline that slides along the triangles of a collision cache.
class CSurfacePath {
public:
  typedef TFunctor2< const CVector3f&, const CVector3f& > TStepCallback;

  explicit CSurfacePath(int reserveCount);

  void Simplify();
  bool Trace(float distance, const CVector3f& start, const CVector3f& direction,
             CCollisionCache& cache, int maxSteps,
             const rstl::optional_object< TStepCallback >& cb);
  void AddSegment(const STriangleSlot* slot, const CVector3f& start, const CVector3f& step,
                  bool isLast);

  int GetSegmentCount() const { return mSegments.size(); }
  const SSurfacePathSegment& GetSegment(int index) const { return mSegments[index]; }
  float GetTotalLength() const { return mTotalLength; }

private:
  bool FindNearestTriangle(float radius, const CVector3f& position, const STriangleSlot* exclude,
                           const CVector3f& velocity, CCollisionCache& cache,
                           const STriangleSlot** triangle, CVector3f* barycentric,
                           CVector3f* closest, CCollisionCacheIterator& foundIterator);

  rstl::vector< SSurfacePathSegment > mSegments;
  float mTotalLength;
  CRandom16 mRandom;
};
CHECK_SIZEOF(CSurfacePath, 0x18)

// Guessed name: a location along a CSurfacePath, as a segment index and a distance into it.
typedef rstl::pair< int, float > SPathPosition; // segment index, distance into the segment

// Guessed name: one moving piece of the effect, a window sliding along its own path.
struct SBlobStrand {
  SBlobStrand()
  : mPathDistance(0.f)
  , mSegmentDistance(0.f)
  , mSpeed(0.f)
  , mStartLength(1.f)
  , mEndLength(0.f)
  , mAge(0.f)
  , mLifetime(0.f)
  , mSegmentIndex(0)
  , mPath(8) {}

  float GetCurrentLength() const;
  SPathPosition GetPathPosition(float offset) const; // Guessed name
  bool IsFinished() const;

  float mPathDistance;
  float mSegmentDistance;
  float mSpeed;
  float mStartLength;
  float mEndLength;
  float mAge;
  float mLifetime;
  int mSegmentIndex;
  CSurfacePath mPath;
};
CHECK_SIZEOF(SBlobStrand, 0x38)

// Registered by the GeomBlobV2 REL (module 25). The class name and the accessor names come from the
// exports of the Wii build (orig/R3ME01/files/MP2/RSO/Production/selfile.sel, SHA1
// 521af7e86335e3e9314b7acefbd565444926e631): the constructor signature and the exports
// SetParticleEmissionRateScalar, SetMinimumPathLength, GetParticleSystem, ParticleSystem and
// LoadGeomBlobV2/SetSGeomBlobV2_FuncPtrs. The Wii accessors are in the reverse order of the
// GameCube binary; which of ParticleSystem/GetParticleSystem is at 0x2544 and which at 0x254C is a
// guess (identical bodies). The GameCube build has no symbol map.
class CCollisionTracker : public CEffect {
public:
  static const float skDefaultExtents;

  CCollisionTracker(const TLockedToken< CGenDescription >& desc, TUniqueId uid, TAreaId area,
                    bool active, const rstl::string& name, const CTransform4f& xf, TUniqueId owner,
                    uint flags, float trackRadius);

  // CEntity
  ~CCollisionTracker() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  // CActor
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  // CCollisionTracker
  CElementGen* ParticleSystem() const;
  CElementGen* GetParticleSystem() const;
  void SetMinimumPathLength(float length);
  float GetMinimumPathLength() const;          // Guessed name
  float GetParticleEmissionRateScalar() const; // Guessed name
  void SetParticleEmissionRateScalar(float scalar);

private:
  void RenderStrands() const;
  CTransform4f BuildStrandTransform(const SPathPosition& start, const CVector3f& startPoint,
                                    const SPathPosition& end, const CVector3f& endPoint,
                                    const SBlobStrand& strand, float scale) const;

  CRandom16 mRandom;
  rstl::single_ptr< CElementGen > mParticleSystem;
  CFoldySurface mModel;
  rstl::single_ptr< CCollisionCache > mCollisionCache;
  TUniqueId mLightId;
  CAssetId mParticleAssetId;
  float mParticleEmissionRateScalar;
  float mSpawnRemainder;
  float mMinimumPathLength;
  int x19c_unknown;
  TUniqueId mOwner;
  rstl::vector< SBlobStrand > mStrands;
  CAABox mTrackBounds;
  float mTrackRadius;
  rstl::optional_object< CVector3f > mLastCachePosition;
};
CHECK_SIZEOF(CCollisionTracker, 0x1e0)

CEffect* LoadGeomBlobV2(const TLockedToken< CGenDescription >& desc, TUniqueId uid, TAreaId area,
                        bool active, const rstl::string& name, const CTransform4f& xf,
                        TUniqueId owner, uint flags);

#endif // _CCOLLISIONTRACKER
