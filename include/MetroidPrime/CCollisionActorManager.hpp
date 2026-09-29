#ifndef _CCOLLISIONACTORMANAGER
#define _CCOLLISIONACTORMANAGER

#include "Kyoto/Math/CTransform4f.hpp"
#include "MetroidPrime/Collision/CJointCollisionDescription.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/vector.hpp"

class CAnimData;
class CMaterialList;
class CStateManager;

class CCollisionActorManager {
public:
  enum EUpdateOptions { kUO_ObjectSpace, kUO_WorldSpace };

  CCollisionActorManager(CStateManager& mgr, TUniqueId owner, TAreaId areaId,
                         const rstl::vector< CJointCollisionDescription >& descriptions,
                         bool active);
  ~CCollisionActorManager();

  bool GetActive() const { return mActive; }
  void SetPhysicsActive(CStateManager& mgr, bool active);
  void SetActive(CStateManager& mgr, bool active);
  void Destroy(CStateManager& mgr) const;
  void Update(float dt, CStateManager& mgr, EUpdateOptions options);
  int GetCollisionDescIndexFromUniqueId(TUniqueId id) const;
  const CJointCollisionDescription& GetCollisionDescFromIndex(uint index) const;
  uint GetNumCollisionActors() const;
  void AddMaterialList(CStateManager& mgr, const CMaterialList& materials);
  void RemoveMaterialList(CStateManager& mgr, const CMaterialList& materials);

  static CTransform4f GetWRLocatorTransform(const CAnimData& animData, CSegId id,
                                            const CTransform4f& worldXf,
                                            const CTransform4f& scaleXf);

private:
  rstl::vector< CJointCollisionDescription > mJointDescriptions;
  TUniqueId mOwnerId;
  bool mActive;
  mutable bool mDestroyed;
  bool mPhysicsActive;
};

CHECK_SIZEOF(CCollisionActorManager, 0x18)

#endif // _CCOLLISIONACTORMANAGER
