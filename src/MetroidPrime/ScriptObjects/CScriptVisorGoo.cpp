#include "MetroidPrime/ScriptObjects/CScriptVisorGoo.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CCameraManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrVisorGoo.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"

#include "rstl/math.hpp"

CScriptVisorGoo::CScriptVisorGoo(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                 const CTransform4f& xf, CAssetId particle, CAssetId electric,
                                 const CColor& color, float minRange, float maxRange,
                                 float chanceMinRange, float chanceMaxRange, int sfx,
                                 bool noViewCheck, bool persistent, bool deleteOnDeactivate)
: CActor(uid, name, info, 0, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
, mParticleDesc(nullptr)
, mElectricDesc(nullptr)
, mSfx(sfx)
, mParticleId(particle)
, mElectricId(electric)
, mEffectId(kInvalidUniqueId)
, mMinRange(minRange)
, mMaxRange(rstl::max_val(maxRange, minRange + 0.01f))
, mChanceMinRange(chanceMinRange)
, mChanceMaxRange(chanceMaxRange)
, mColor(color)
, mViewCheck(!noViewCheck)
, mPersistent(persistent)
, mDeleteOnDeactivate(deleteOnDeactivate) {
  if (particle != kInvalidAssetId) {
    mParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', particle));
  }
  if (electric != kInvalidAssetId) {
    mElectricDesc = gpSimplePool->GetObj(SObjectTag('ELSC', electric));
  }
}

void CScriptVisorGoo::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_Activate:
    if (!GetActive()) {
      if (mParticleId != kInvalidAssetId) {
        mParticleDesc.Lock();
      }
      if (mElectricId != kInvalidAssetId) {
        mElectricDesc.Lock();
      }
    }
    break;
  case kSM_Deactivate:
    if (GetActive() && mEffectId != kInvalidUniqueId) {
      CEntity* effect = mgr.ObjectById(mEffectId);
      if (effect != nullptr) {
        if (mDeleteOnDeactivate) {
          mgr.SendScriptMsg(effect, GetUniqueId(), kSM_Delete);
        } else {
          CHUDBillboardEffect* hud = static_cast< CHUDBillboardEffect* >(effect);
          hud->GetParticleGen()->SetParticleEmission(false);
          hud->SetFinishing();
        }
      }
      mEffectId = kInvalidUniqueId;
    }
    break;
  default:
    break;
  }
  CActor::AcceptScriptMsg(mgr, msg);
}

void CScriptVisorGoo::Think(float, CStateManager& mgr) {
  if (GetActive() && mEffectId == kInvalidUniqueId) {
    bool loaded = false;
    if (mParticleId != kInvalidAssetId) {
      if (mParticleDesc.IsLoaded()) {
        if (mElectricId != kInvalidAssetId) {
          if (mElectricDesc.IsLoaded()) {
            loaded = true;
          }
        } else {
          loaded = true;
        }
      }
    } else if (mElectricDesc.IsLoaded()) {
      loaded = true;
    }
    if (loaded) {
      for (int i = 0; i < mgr.GetNumPlayers(); ++i) {
        CPlayer* player = mgr.GetPlayer(i);
        if (player->GetCameraState() != CPlayer::kCS_FirstPerson) {
          continue;
        }
        bool showGoo = false;
        const CVector3f eyeToGoo = GetTranslation() - player->GetEyePosition();
        const float eyeToGooDist = eyeToGoo.Magnitude();
        if (eyeToGooDist >= mMinRange && eyeToGooDist <= mMaxRange) {
          if (mViewCheck) {
            const CVector3f colNorm = player->GetCameraManager()
                                          ->GetCurrentCameraTransform(mgr, true)
                                          .GetColumn(kDY)
                                          .AsNormalized();
            float angleThresh = 45.f;
            const float dot = CMath::Limit(CVector3f::Dot(eyeToGoo.AsNormalized(), colNorm), 1.f);
            const float angle = CMath::Rad2Rev(CMath::FastArcCosR(dot)) * 360.f;
            if (eyeToGooDist < 4.f) {
              angleThresh *= 4.f / eyeToGooDist;
              angleThresh = rstl::min_val(angleThresh, 90.f);
            }
            if (angle <= angleThresh) {
              showGoo = true;
            }
          } else {
            showGoo = true;
          }
          if (showGoo) {
            const float t = (mMaxRange - eyeToGooDist) / (mMaxRange - mMinRange);
            const float prob = t * mChanceMinRange + (1.f - t) * mChanceMaxRange;
            if (mgr.Random()->Float() * 100.f <= prob) {
              mEffectId = mgr.AllocateUniqueId();
              mgr.AddObject(rs_new CHUDBillboardEffect(
                  mParticleId != kInvalidAssetId
                      ? rstl::optional_object< TToken< CGenDescription > >(GetParticleDesc())
                      : rstl::optional_object_null(),
                  mElectricId != kInvalidAssetId
                      ? rstl::optional_object< TToken< CElectricDescription > >(GetElectricDesc())
                      : rstl::optional_object_null(),
                  mEffectId, true, rstl::string_l("VisorGoo"),
                  CHUDBillboardEffect::GetNearClipDistance(mgr, i),
                  CHUDBillboardEffect::GetScaleForPOV(mgr), i, mColor, CVector3f::One(),
                  CVector3f::Zero(), 0));
              CSfxManager::SfxStart(mSfx, 0x7f, 0x40, CSfxManager::kAllAreas, false, false,
                                    CSfxManager::kMedPriority);
            }
          }
        }
      }
      if (!mPersistent) {
        mgr.DeleteObjectRequest(GetUniqueId());
      }
    }
  }
}

void CScriptVisorGoo::Touch(CActor&, CStateManager&) {}

rstl::optional_object< CAABox > CScriptVisorGoo::GetTouchBounds() const {
  return rstl::optional_object_null();
}

void CScriptVisorGoo::Render(const CStateManager&) const {}

void CScriptVisorGoo::AddToRenderer(const CStateManager&) const {}

CEntity* LoadVisorGoo(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrVisorGoo sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrVisorGoo.inc"

  return rs_new CScriptVisorGoo(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), LdrToTransform4f(sldrThis.editorProperties),
      sldrThis.particle, sldrThis.electric, sldrThis.color, sldrThis.minRange, sldrThis.maxRange,
      sldrThis.chanceAtMinRange, sldrThis.chanceAtMaxRange, sldrThis.sound_HitSound,
      sldrThis.noViewCheck, sldrThis.persistent, sldrThis.unknown_0xcb9a3009);
}

CScriptVisorGoo::~CScriptVisorGoo() {}
