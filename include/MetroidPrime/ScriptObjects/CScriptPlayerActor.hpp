#ifndef _CSCRIPTPLAYERACTOR
#define _CSCRIPTPLAYERACTOR

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

class CModel;
class CPlayer;
class CProjectedShadow;
class CSkinRules;
class CSkinnedModel;

class CScriptPlayerActor : public CScriptActor {
public:
  CScriptPlayerActor(TUniqueId uid, const rstl::string& name, CEntityInfo& info,
                     const CTransform4f& xf, const CAnimRes& animRes, const CModelData& modelData,
                     int characterCount, const CAABox& bounds, bool setBoundingBox,
                     const CMaterialList& materials, float mass, float zMomentum,
                     const CHealthInfo& health, const CDamageVulnerability& vulnerability,
                     const CActorParameters& parameters, bool loop, uint flags,
                     CPlayerState::EBeamId beam, bool usePlayerBeamModel);

  // CEntity
  ~CScriptPlayerActor() override {}
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(const bool active) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;

  void TouchModels(CStateManager& mgr);

private:
  CTransform4f GetGunTransform() const;
  CTransform4f GetGrappleTransform() const;
  int GetSuitCharIdx(const CStateManager& mgr, CPlayerState::EPlayerSuit suit) const;
  int GetNextSuitCharIdx(const CStateManager& mgr) const;
  void LoadSuit(int charIdx);
  void LoadBeam(CPlayerState::EBeamId beam);
  void CancelBeamLoad();
  void CancelGrappleLoad(); // Guessed name
  void UpdateGrappleModel(CStateManager& mgr);
  void UsePlayerBeamModel(CPlayer& player);
  void PumpBeamModel(CStateManager& mgr);
  void PumpGrappleModel(CStateManager& mgr);
  void PumpSuitModel(CStateManager& mgr);
  void BuildBeamModelData();
  void BuildGrappleModelData();
  void SetupOfflineModelData();
  void SetupOnlineModelData(CStateManager& mgr);
  void TouchModels_Internal(const CStateManager& mgr) const;
  void RenderGrapple(const CStateManager& mgr) const;
  void RenderGun(const CStateManager& mgr) const;
  void SetupEnvFx(CStateManager& mgr, bool set);
  void SetIntoStateManager(CStateManager& mgr, bool set);
  bool HasGunModelData() const {
    return mBeamModelData.get() != nullptr &&
           (mBeamModelData->HasAnimation() || mBeamModelData->HasNormalModel());
  }
  bool HasGrappleModelData() const {
    return mGrappleModelData.get() != nullptr &&
           (mGrappleModelData->HasAnimation() || mGrappleModelData->HasNormalModel());
  }
  bool HasSuitModelData() const {
    return mSuitModelData.get() != nullptr &&
           (mSuitModelData->HasAnimation() || mSuitModelData->HasNormalModel());
  }

  CAnimRes mSuitRes;
  CPlayerState::EBeamId mBeam;
  CPlayerState::EPlayerSuit mSuit;
  CPlayerState::EBeamId mPreviousSetBeamId; // Guessed name
  CPlayerState::EBeamId mSetBeamId;
  int mLoadedCharIdx;
  rstl::single_ptr< CModelData > mBeamModelData;
  rstl::single_ptr< CModelData > mGrappleModelData; // Guessed name
  rstl::single_ptr< CModelData > mSuitModelData;
  rstl::single_ptr< TToken< CModel > > mBeamModel;
  rstl::single_ptr< TToken< CModel > > mGrappleModel; // Guessed name
  rstl::single_ptr< TCachedToken< CModel > > mSuitModel;
  rstl::single_ptr< TToken< CSkinRules > > mSuitSkin;
  rstl::optional_object< TLockedToken< CSkinnedModel > > mBackupModelData;
  rstl::single_ptr< CProjectedShadow > mProjectedShadow;
  int mDeallocateBackupCountdown;
  int mCharacterCount; // Guessed name
  uint mFlags;
  bool mSetBoundingBox : 1;
  bool mDeferOnlineModelData : 1;
  bool mDeferOfflineModelData : 1;
  bool mBeamModelLoading : 1;
  bool mGrappleModelLoading : 1; // Guessed name
  bool mSuitModelLoading : 1;
  bool mLoading : 1;
  bool mEnableLoading : 1;
  bool mDeferOnlineLoad : 1;
  bool mAreaTrackingLoad : 1;
  bool mUsePlayerBeamModel : 1; // Guessed name
  bool mWaitForIncrement : 1;   // Guessed name
  bool mSentArrived : 1;        // Guessed name
  bool mDrawGrapple : 1;        // Guessed name
  TUniqueId mNextPlayerActor;
};
CHECK_SIZEOF(CScriptPlayerActor, 0x408)

#endif // _CSCRIPTPLAYERACTOR
