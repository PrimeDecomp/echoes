#include "MetroidPrime/CGameLight.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CStateManager.hpp"

CGameLight::CGameLight(TUniqueId uid, TAreaId areaId, bool active, const rstl::string& name,
                       const CTransform4f& xf, TUniqueId parentId, const CLight& light,
                       uint sourceId, uint priority, float lifeTime, const CEntityInfo* info)
: CActor(uid, name, info ? *info : CEntityInfo(areaId, NullConnectionList, active), 0, xf,
         CModelData(), CMaterialList(kMT_NoStepLogic), CActorParameters(), kInvalidUniqueId)
, mParentId(parentId)
, mLight(light)
, mSourceId(sourceId)
, mPriority(priority)
, mLifeTime(lifeTime) {
  mLight.GetRadius();
  mLight.GetIntensity();
  SetLightPriorityAndId();
}

void CGameLight::SetLight(const CLight& light) {
  mLight = light;
  mLight.GetRadius();
  mLight.GetIntensity();
  SetLightPriorityAndId();
}

CLight CGameLight::GetLight() const {
  CLight light(mLight);
  if (light.GetType() != kLT_Directional) {
    light.SetPosition(GetTransform() * mLight.GetPosition());
  }
  if (light.GetType() != kLT_Point) {
    light.SetDirection(GetTransform().Rotate(mLight.GetDirection()).AsNormalized());
  }
  return light;
}

void CGameLight::Think(float dt, CStateManager& mgr) {
  if (mLifeTime > 0.f) {
    mLifeTime -= dt;
    if (mLifeTime <= 0.f) {
      mgr.DeleteObjectRequest(GetUniqueId());
    }
  }
}

void CGameLight::SetLightPriorityAndId() {
  mLight.SetPriority(mPriority);
  mLight.SetLightId(mSourceId);
}
