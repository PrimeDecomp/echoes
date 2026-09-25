#ifndef _CGAMELIGHT
#define _CGAMELIGHT

#include "Kyoto/Graphics/CLight.hpp"
#include "MetroidPrime/CActor.hpp"

class CGameLight : public CActor {
public:
  CGameLight(TUniqueId uid, TAreaId areaId, bool active, const rstl::string& name,
             const CTransform4f& xf, TUniqueId parentId, const CLight& light, uint sourceId,
             uint priority, float lifeTime, const CEntityInfo* info = nullptr);

  // CEntity
  ~CGameLight() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  void SetLightPriorityAndId();
  CLight GetLight() const;
  void SetLight(const CLight& light);
  TUniqueId GetParentId() const { return mParentId; }

private:
  TUniqueId mParentId;
  CLight mLight;
  uint mSourceId;
  uint mPriority;
  float mLifeTime;
};
CHECK_SIZEOF(CGameLight, 0x1b8)

#endif // _CGAMELIGHT
