#include "MetroidPrime/Weapons/CPowerBeam.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CStateManager.hpp"

extern "C" ushort lbl_8041E2E6;

CPowerBeam::CPowerBeam(TUniqueId playerId, const CVector3f& scale, int unk)
: CGunWeapon(kWT_Power, playerId, scale, unk)
, mShotSmoke()
, mPower2nd1()
, mSmokeTimer(0.f)
, mSmokeState(kSS_Inactive)
, x244_24(false)
, mLoaded(false) {}

CPowerBeam::~CPowerBeam() {}

void CPowerBeam::ReInitVariables() {
  mShotSmokeGen = nullptr;
  mPower2ndGen = nullptr;
  mSmokeTimer = 0.f;
  mSmokeState = kSS_Inactive;
  x244_24 = false;
  mLoaded = false;
  mEnabledSecondaryEffect = kSFT_None;
}

void CPowerBeam::PreRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  CTransform4f backupView = CGraphics::GetViewMatrix();

  CGraphics::SetViewPointMatrix(xf.GetInverse() * backupView);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  if (!mShotSmokeGen.null() && mSmokeState != kSS_Inactive)
    mShotSmokeGen->Render();

  CGraphics::SetViewPointMatrix(backupView);
}

void CPowerBeam::PostRenderGunFx(const CStateManager& mgr, const CTransform4f& xf) {
  if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null())
    mPower2ndGen->Render();
  CGunWeapon::PostRenderGunFx(mgr, xf);
}

void CPowerBeam::UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr,
                             const CTransform4f& xf) {
  switch (mSmokeState) {
  case kSS_Inactive:
    if (shotSmoke) {
      if (!mShotSmokeGen.null())
        mShotSmokeGen->SetParticleEmission(true);
      mSmokeTimer = 2.f;
      mSmokeState = kSS_Active;
    }
    break;
  case kSS_Active:
    if (mSmokeTimer > 0.f) {
      mSmokeTimer -= dt;
    } else {
      if (!mShotSmokeGen.null())
        mShotSmokeGen->SetParticleEmission(false);
      mSmokeState = kSS_Done;
    }
    // [[fallthrough]];
  case kSS_Done:
    if (!mShotSmokeGen.null()) {
      CTransform4f locator = mSolidModelData->GetScaledLocatorTransform(
          rstl::string_l(CGunWeapon::skMuzzleLocator));
      mShotSmokeGen->SetGlobalTranslation(locator.GetTranslation());
      mShotSmokeGen->Update(dt);
      if (mSmokeState == kSS_Done && mShotSmokeGen->GetSystemCount() == 0)
        mSmokeState = kSS_Inactive;
    } else {
      mSmokeState = kSS_Inactive;
    }
    break;
  }

  if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null()) {
    mPower2ndGen->SetGlobalOrientAndTrans(xf);
    mPower2ndGen->Update(dt);
  }

  CGunWeapon::UpdateGunFx(shotSmoke, dt, mgr, xf);
}

void CPowerBeam::Update(float dt, CStateManager& mgr) {
  CGunWeapon::Update(dt, mgr);
  if (IsLoaded())
    return;

  if (CGunWeapon::IsLoaded() && !mLoaded) {
    mLoaded = mShotSmoke->IsLoaded() && mPower2nd1->IsLoaded();
    if (mLoaded) {
      // x234_shotSmokeGen = rs_new CElementGen(x21c_shotSmoke);
      mShotSmokeGen = new CElementGen(*mShotSmoke);
      mShotSmokeGen->SetParticleEmission(false);
    }
  }
}

void CPowerBeam::Fire(CToken& token, bool underwater, float dt,
                      CPlayerState::EChargeStage chargeState, const CTransform4f& xf,
                      CStateManager& mgr, TUniqueId homingTarget, int unk1, ushort unk2,
                      TUniqueId id, CSfxHandle sfx, float chargeFactor1, float chargeFactor2) {

  if (unk2 == lbl_8041E2E6) {
    mgr.fn_80036F10();
  }

  CGunWeapon::Fire(token, underwater, dt, chargeState, xf, mgr, homingTarget, unk1, unk2, id, sfx,
                   chargeFactor1, chargeFactor2);
}

void CPowerBeam::Load(CStateManager& mgr, bool subtypeBasePose) {
  CGunWeapon::Load(mgr, subtypeBasePose);
  mShotSmoke->Lock();
  mPower2nd1->Lock();
}

void CPowerBeam::Unload(CStateManager& mgr) {
  CGunWeapon::Unload(mgr);
  if (!mgr.fn_80036F10()) {
    mPower2nd1->Unlock();
    mShotSmoke->Unlock();
  }
  ReInitVariables();
}

void CPowerBeam::Unk11(CStateManager& mgr) {
  CGunWeapon::Unk11(mgr);
  if (!mgr.fn_80036F10()) {
    mPower2nd1->Unlock();
    mShotSmoke->Unlock();
  }
  mShotSmokeGen = nullptr;
  mPower2ndGen = nullptr;
  mSmokeState = kSS_Inactive;
  x244_24 = false;
  mEnabledSecondaryEffect = kSFT_None;
}

bool CPowerBeam::IsLoaded() const { return CGunWeapon::IsLoaded() && mLoaded; }

void CPowerBeam::EnableSecondaryFx(ESecondaryFxType type) {
  switch (type) {
  case kSFT_None:
  case kSFT_ToCombo:
  case kSFT_CancelCharge:
    if (mEnabledSecondaryEffect != kSFT_None && !mPower2ndGen.null())
      mPower2ndGen->SetParticleEmission(false);
    mEnabledSecondaryEffect = kSFT_None;
    break;
  case kSFT_Charge:
    mPower2ndGen = new CElementGen(*mPower2nd1);
    mPower2ndGen->SetGlobalScale(mScale);
    mEnabledSecondaryEffect = type;
    break;
  default:
    break;
  }
}

void CPowerBeam::Unk9(CStateManager& mgr) {
  if (mSubtypeBasePose == 0) {
    CGunWeapon::Unk9(mgr);
    mShotSmoke = gpSimplePool->GetObj("ShotSmoke");
    mPower2nd1 = gpSimplePool->GetObj("Power2nd_1");
  }
}
