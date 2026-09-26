#ifndef _CACTORMODELPARTICLES
#define _CACTORMODELPARTICLES

#include "types.h"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CActor;
class CEntity;
class CElementGen;
class CElectricDescription;
class CGenDescription;
class CParticleElectric;
class CRainSplashGenerator;
class CSkinnedModel;
class CStateManager;
class CTexture;
struct SSkinningWorkspace;

class CActorModelParticles {
public:
  enum ESystemTypes {
    kST_OnFire,
    kST_Ice,
    kST_Ash,
    kST_FirePop,
    kST_Electric,
    kST_IcePop,
    kST_BlackHole, // Guessed name; Effect_Blackhole resource.
    kST_Imploder,  // Guessed name; Effect_Imploder resource.
  };

  struct CSystem {
    rstl::vector< CToken > mTokens;
    int mRefCount;
    bool mLoaded;

    explicit CSystem(const char* name);
    void Update();
    void Unlock();
    void Lock();
    void DelRef();
    void AddRef();
  };

  class CItem {
    friend class CActorModelParticles;

  public:
    CItem(const CEntity& ent, CActorModelParticles& parent);
    ~CItem();
    void GeneratePoints(const CSkinnedModel& model, const SSkinningWorkspace& workspace, int count);
    bool Update(float dt, CStateManager& mgr);
    void DontUseType(ESystemTypes type);

  private:
    TUniqueId mId;
    TAreaId mAreaId;
    rstl::reserved_vector< rstl::pair< rstl::auto_ptr< CElementGen >, uint >, 8 > mOnFireGens;
    float mOnFireDelayTimer;
    bool mOnFire;
    CSfxHandle mSfx;
    rstl::auto_ptr< CElementGen > mAshGen;
    int mAshPointIterator;
    int mAshMaxParticles;
    int mAshQueuedParticles; // Guessed name; particles scheduled for the next model callback.
    uint mAshSeed;
    rstl::reserved_vector< rstl::auto_ptr< CElementGen >, 4 > mIceGens;
    int mIcePointIterator;
    uint mIceSeed;
    rstl::auto_ptr< CElementGen > mFirePopGen;
    rstl::auto_ptr< CParticleElectric > mElectricGen;
    int mElectricPointIterator;
    uint mElectricSeed;
    CColor mElectricColor;
    // Guessed names: shared state for the black-hole and imploder effects.
    rstl::auto_ptr< CElementGen > mImplosionGen;
    int mImplosionPointIterator;
    int mImplosionMaxParticles;
    int mImplosionQueuedParticles;
    uint mImplosionSeed;
    CVector3f mImplosionPoint;
    CPlane mImplosionClipPlane;
    rstl::auto_ptr< CRainSplashGenerator > mRainSplashGen;
    CToken mAshy;
    rstl::auto_ptr< CElementGen > mIcePopGen;
    CVector3f mParticleOffsetScale;
    CTransform4f mIceXf;
    CActorModelParticles* mParent;
    float mRemTime;
    uchar mLockDeps;

    bool UpdateOnFire(float dt, CActor* actor, CStateManager& mgr);
    bool UpdateBurn(float dt, const CActor* actor, CStateManager& mgr);
    // Guessed name.
    bool UpdateImplosion(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateAshGen(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateIcePop(float dt, const CActor* actor);
    bool UpdateFirePop(float dt, const CActor* actor);
    bool UpdateIce(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateElectric(float dt, const CActor* actor, CStateManager& mgr);
    bool UpdateRainSplash(float dt, const CActor* actor, CStateManager& mgr);
    void UseType(ESystemTypes type);
  };

  CActorModelParticles();
  CTexture* GetAshyTexture(const CActor& actor) const;
  void StopBurnDeath(CActor& actor); // Guessed name.
  void StartBurnDeath(CActor& actor, CStateManager& mgr);
  void Render(const CStateManager& mgr, const CActor& actor) const;
  void AddStragglersToRenderer(const CStateManager& mgr) const;
  rstl::list< CItem >::iterator FindSystem(TUniqueId uid);
  rstl::list< CItem >::const_iterator FindSystem(TUniqueId uid) const;
  rstl::list< CItem >::iterator FindOrCreateSystem(CActor& actor);
  void SetupHook(TUniqueId uid) const;
  static void PointGenerator(const CSkinnedModel& model, const SSkinningWorkspace& workspace,
                             void* context);
  void StopRainSplashes(CActor& actor);
  void StartRainSplashes(CActor& actor, CStateManager& mgr, int maxSplashes, int genRate,
                         float minZ);
  void StopFire(CActor& actor);
  void LightDudeOnFire(CActor& actor);
  void StopElectric(CActor& actor);
  void StartElectric(CActor& actor);
  void DoFirePop(CActor& actor);
  void StopImplosion(CActor& actor);                                          // Guessed name.
  void StartImplosion(CActor& actor, const CVector3f& point, bool blackHole); // Guessed name.
  void StartAsh(CActor& actor);
  void Update(float dt, CStateManager& mgr);

private:
  friend class CItem;

  TToken< CGenDescription > mOnFire;
  TToken< CGenDescription > mAsh;
  TToken< CGenDescription > mIceBreak;
  TToken< CGenDescription > mFirePop;
  TToken< CGenDescription > mIcePop;
  TToken< CGenDescription > mBlackHole; // Guessed name.
  TToken< CGenDescription > mImploder;  // Guessed name.
  TToken< CElectricDescription > mElectric;
  CToken mAshy;
  rstl::reserved_vector< CSystem, 8 > mDgrps;
  rstl::list< CItem > mItems;
  uchar mLoadingDeps;
  uchar mJustLoadedDeps;
  uchar mLoadedDeps;

  void UpdateSystemTypes();
  void DelTypeRef(ESystemTypes type);
  void AddTypeRef(ESystemTypes type);
  void InitializeSystemTypes();
  CElementGen* MakeIceGen();
  CElementGen* MakeOnFireGen();
  CParticleElectric* MakeElectricGen();
  CElementGen* MakeImploderGen();  // Guessed name.
  CElementGen* MakeBlackHoleGen(); // Guessed name.
  CElementGen* MakeIcePopGen();
  CElementGen* MakeFirePopGen();
  CElementGen* MakeAshGen();
};
NESTED_CHECK_SIZEOF(CActorModelParticles, CSystem, 0x18)
NESTED_CHECK_SIZEOF(CActorModelParticles, CItem, 0x16c)
CHECK_SIZEOF(CActorModelParticles, 0x128)

#endif // _CACTORMODELPARTICLES
