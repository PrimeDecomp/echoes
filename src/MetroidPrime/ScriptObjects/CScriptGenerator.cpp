#include "MetroidPrime/ScriptObjects/CScriptGenerator.hpp"

#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrGenerator.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Enemies/CSwarmBasics.hpp"
#include "MetroidPrime/TCastTo.hpp"

namespace {
// The generator's connection state is GRNT in G2ME01, distinct from GENR.
const EScriptObjectState kGeneratorConnectionState = static_cast< EScriptObjectState >(0x47524e54);
}

CScriptGenerator::CScriptGenerator(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                                   int spawnCount, bool noReuseFollowers, const CVector3f& offset,
                                   bool noInheritTransform, bool unknown, float minScale,
                                   float maxScale)
: CEntity(uid, info, name, 0)
, mSpawnCount(spawnCount)
, mNoReuseFollowers(noReuseFollowers)
, mNoInheritTransform(noInheritTransform)
, x28_26_(unknown)
, mOffset(offset)
, mMinScale(minScale)
, mMaxScale(maxScale) {}

CScriptGenerator::~CScriptGenerator() {}

void CScriptGenerator::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  switch (msg.GetMessage()) {
  case kSM_SetToZero: {
    if (!GetActive()) {
      break;
    }
    rstl::vector< TUniqueId > followers;
    followers.reserve(GetConnectionList().empty() ? 1 : GetConnectionList().size());
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state != kGeneratorConnectionState || it->msg != kSM_Follow) {
        continue;
      }
      const TUniqueId uid = mgr.GetIdForScript(it->objId);
      if (uid != kInvalidUniqueId) {
        const CEntity* follower = mgr.GetObjectById(uid);
        if (follower && follower->GetActive()) {
          followers.push_back_unsafe(uid);
        }
      }
    }
    if (followers.empty()) {
      if (x28_26_) {
        followers.push_back_unsafe(msg.GetOriginator());
      } else {
        followers.push_back_unsafe(msg.GetSenderId());
      }
    }

    rstl::vector< TEditorId > activations;
    activations.reserve(GetConnectionList().size());
    for (rstl::vector< SConnection >::const_iterator it = GetConnectionList().begin();
         it != GetConnectionList().end(); ++it) {
      if (it->state != kGeneratorConnectionState) {
        continue;
      }
      if (it->msg == kSM_Activate) {
        activations.push_back_unsafe(it->objId);
      } else {
        mgr.SendScriptMsg(mgr.GetIdForScript(it->objId), GetUniqueId(), it->msg, kInvalidUniqueId);
      }
    }

    if (activations.empty()) {
      break;
    }

    for (int i = 0; i < mSpawnCount; ++i) {
      if (activations.empty() || followers.empty()) {
        break;
      }
      int activationIndex = static_cast< int >(0.99f * (mgr.Random()->Float() * activations.size()));
      const int followerIndex = static_cast< int >(0.99f * (mgr.Random()->Float() * followers.size()));

      CScriptObjectLoaderHelper& loader = mgr.ScriptObjectLoaderHelper();
      for (int j = 0; j < activations.size(); ++j) {
        const rstl::pair< const CScriptObjectLoaderHelper::SScriptObjectStream*, TEditorId > build =
            loader.GetBuildForScript(activations[j]);
        if (build.first && build.first->mType == 'SOND') {
          activationIndex = j;
          break;
        }
      }

      const TEditorId activationId = activations[activationIndex];
      CEntity* follower = mgr.GetObjectByIdFromListAll(followers[followerIndex]);
      if (!follower) {
        break;
      }
      const CScriptObjectLoaderHelper::SGeneratedObject generated =
          loader.GenerateScriptObject(activationId, mgr);
      if (generated.mUniqueId != kInvalidUniqueId) {
        CEntity* generatedEntity = mgr.ObjectById(generated.mUniqueId);
        CActor* generatedActor = TCastToPtr< CActor >(generatedEntity);
        const CActor* followerActor = TCastToConstPtr< CActor >(follower);
        const CSwarmBasics* followerSwarm = TCastToConstPtr< CSwarmBasics >(follower);

        if (generatedActor && followerSwarm) {
          if (!mNoInheritTransform) {
            generatedActor->SetTransform(followerSwarm->GetTransform());
          }
          generatedActor->SetTranslation(followerSwarm->GetLastKilledOffset() + mOffset);
        } else if (generatedActor && followerActor) {
          if (!mNoInheritTransform) {
            generatedActor->SetTransform(followerActor->GetTransform());
          }
          generatedActor->SetTranslation(followerActor->GetTranslation() + mOffset);
        }

        if (generatedEntity) {
          generatedEntity = mgr.ObjectById(generated.mUniqueId);
          generatedActor = TCastToPtr< CActor >(generatedEntity);
          followerActor = TCastToConstPtr< CActor >(follower);
          followerSwarm = TCastToConstPtr< CSwarmBasics >(follower);
          if (generatedActor) {
            if (followerSwarm) {
              if (!mNoInheritTransform) {
                generatedActor->SetTransform(followerSwarm->GetTransform());
              }
              generatedActor->SetTranslation(followerSwarm->GetLastKilledOffset() + mOffset);
            } else if (followerActor) {
              if (!mNoInheritTransform) {
                generatedActor->SetTransform(followerActor->GetTransform());
              }
              generatedActor->SetTranslation(followerActor->GetTranslation() + mOffset);
            }

            const float scale = mgr.Random()->Range(mMinScale, mMaxScale);
            if (generatedActor->HasModelData()) {
              generatedActor->ModelData()->SetScale(scale * generatedActor->ModelData()->GetScale());
            }
          }
          mgr.DeliverScriptMsg(CScriptMsg(GetUniqueId(), kInvalidUniqueId, generated.mUniqueId,
                                          kSM_Activate, kSS_InvalidState));
        }
      }

      activations.erase(activations.begin() + activationIndex);
      if (mNoReuseFollowers) {
        followers.erase(followers.begin() + followerIndex);
      }
    }
    break;
  }
  default:
    break;
  }
  CEntity::AcceptScriptMsg(mgr, msg);
}

CEntity* LoadGenerator(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrGenerator sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrGenerator.inc"

  return rs_new CScriptGenerator(
      mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
      LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.randomCount,
      sldrThis.uniqueLocations, sldrThis.offset, sldrThis.keepOrientation,
      sldrThis.useOriginatorTransform, sldrThis.randomScaleMin, sldrThis.randomScaleMax);
}
