#include "MetroidPrime/CMFGameLoader.hpp"

#include "MetroidPrime/CArchitectureQueue.hpp"
#include "MetroidPrime/CInGameGuiManagerSet.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMemoryCard.hpp"
#include "MetroidPrime/CMFGame.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Decode.hpp"
#include "MetroidPrime/Player/CGameMode.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/vector.hpp"
#include "string.h"

extern int gResFactoryUnknown;

static const SObjectTag skDefaultWorld('MLVL', 0x3bfa3eff);
static const char* const skGunPakSets[3][3] = {
    {"aram:samusgun", "aram:samgunfx", ""},
    {"aram:samusgunlow", "aram:samgunfxlow", "aram:samgunfxmulti"},
    {"aram:samusgunlow", "aram:samgunfxlow", "aram:samgunfxmulti"}};

static rstl::vector< rstl::string > BuildGunPakList();

CMFGameLoader::CMFGameLoader()
: CIOWin(rstl::string_l("CMFGameLoader"))
, mInitialized(false)
, mTransitionFinished(false) {
  gResFactoryUnknown = 1;
  CModel::DisableTextureTimeout();

  const bool showWorldName =
      gpMain->GetRestartMode() == CMain::kRM_Default ||
      (gpMain->GetRestartMode() == CMain::kRM_None &&
       gpGameState->GetGameMode().GetGameModeType() == 'SNGL' &&
       gpGameState->GetGameModeType() == 'FRND');
  const bool introText = showWorldName && gpGameState->GetInitPowerupsAtFirstSpawn() &&
                         gpGameState->CurrentWorldAssetId() == skDefaultWorld.GetId();
  if (introText) {
    const SObjectTag* tag = gpResourceFactory->GetResourceIdByName("STRG_IntroLevelLoad");
    if (tag != nullptr) {
      gpGameState->WorldTransitionManager()->EnableTransition(
          kInvalidAssetId, tag->GetId(), 1, false, 0.1f, 16.f, 1.f, 0.f, 2.f, 3.f,
          rstl::string_l(""), 0x5a, false, true);
    }
  } else if (showWorldName) {
    const CAssetId world = gpGameState->CurrentWorldAssetId();
    if (gpMemoryCard->HasSaveWorldMemory(world)) {
      const CSaveWorldMemory& memory = gpMemoryCard->GetSaveWorldMemory(world);
      const CAssetId name =
          gpGameState->GetIsDarkWorld() ? memory.GetDarkWorldNameId() : memory.GetWorldNameId();
      if (name != kInvalidAssetId) {
        gpGameState->WorldTransitionManager()->EnableTransition(
            kInvalidAssetId, name, 1, false, 0.1f, 16.f, 1.f, 0.f, 0.f, 0.f,
            rstl::string_l(""), 0x5a, false, false);
      }
    }
  }

  UpdateGunPaks();
  mInitialized = false;
}

CMFGameLoader::~CMFGameLoader() {
  CGraphics::SetIsBeginSceneClearFb(true);
  gResFactoryUnknown = 2;
}

CIOWin::EMessageReturn CMFGameLoader::OnMessage(const CArchitectureMessage& message,
                                              CArchitectureQueue& queue) {
  rstl::rc_ptr< CWorldTransManager >& transition = gpGameState->WorldTransitionManager();
  switch (message.GetType()) {
  case kAM_UserInput: {
    const CArchMsgParmUserInput parm = MakeMsg::GetParmUserInput(message);
    const CFinalInput& input = parm.GetUserInput();
    if (input.ControllerNumber() == 0 && input.PStart() && !transition.IsNull() &&
        transition->IsIntroText() && !mStateManager.IsNull() &&
        mStateManager->IsFullyInitialized()) {
      transition->CheckIntroTextSeen();
    }
    break;
  }
  case kAM_TimerTick: {
    const float dt = MakeMsg::GetParmTimerTick(message).GetReal();
    CResLoader& loader = gpResourceFactory->GetResLoader();
    if (!loader.AreAllPaksLoaded()) {
      loader.AsyncIdlePakLoading();
      return kMR_Exit;
    }
    if (!mInitialized) {
      transition->StartTransition();
      mInitialized = true;
      return kMR_Exit;
    }
    transition->Update(dt);

    if (mStateManager.IsNull()) {
      transition->WaitForModelsAndTextures();
      // TODO: Nonfunctional until the Echoes five-argument StateManager constructor is
      // recovered: mailbox, map info, per-player owners, transition and world layers.
      return kMR_Exit;
    }
    if (!mStateManager->IsFullyInitialized()) {
      // TODO: Nonfunctional pending the shared StateManager::InitializeState interface.
      return kMR_Exit;
    }
    if (mGuiManager.IsNull()) {
      gpGameState->CurrentWorldState().SetDesiredAreaAssetId(kInvalidAssetId);
      mGuiManager = rs_new CInGameGuiManagerSet(*mStateManager, queue);
    }
    if (!mGuiManager->CheckLoadComplete(*mStateManager)) {
      return kMR_Exit;
    }
    transition->StartTextFadeOut();
    mTransitionFinished = transition->IsTransitionFinished();
    return kMR_Exit;
  }
  case kAM_FrameEnd:
    if (mTransitionFinished) {
      CIOWin* game = rs_new CMFGame(mStateManager, mGuiManager, queue);
      queue.Push(MakeMsg::CreateCreateIOWin(kAMT_IOWinManager, kMFGameMsgPriority,
                                          kMFGameDrawPriority, game));
      CModel::EnableTextureTimeout();
      return kMR_RemoveIOWinAndExit;
    }
    break;
  default:
    break;
  }
  return kMR_Exit;
}

void CMFGameLoader::Draw() const { gpGameState->WorldTransitionManager()->Draw(); }

void CMFGameLoader::UpdateGunPaks() {
  ScanLoadedGunPaks();
  SelectGunPakSet();
}

void CMFGameLoader::ScanLoadedGunPaks() {
  mLoadedGunPakSets = 0;
  CResLoader& loader = gpResourceFactory->GetResLoader();
  const int pakCount = loader.GetPakCount();
  const rstl::vector< rstl::string > names = BuildGunPakList();
  for (int i = 0; i < pakCount; ++i) {
    const CPakFile* pak = loader.sub_802FBB64(i);
    if (!pak->IsARAMPak()) {
      continue;
    }
    const rstl::string& filename = pak->GetDvdFile().GetFilename();
    const rstl::string name = CStringExtras::CreatePrefix(
        CStringExtras::ConvertToLowerCase(filename), filename.length() - 4);
    const rstl::vector< rstl::string >::const_iterator found =
        rstl::binary_find(names.begin(), names.end(), name);
    if (found == names.end()) {
      continue;
    }
    for (int set = 0; set < 3; ++set) {
      if (*found == rstl::string_l(skGunPakSets[set][0]) ||
          *found == rstl::string_l(skGunPakSets[set][1])) {
        MarkGunPakSetLoaded(set);
        return;
      }
    }
  }
}

void CMFGameLoader::MarkGunPakSetLoaded(int set) { mLoadedGunPakSets |= 1 << set; }

void CMFGameLoader::ClearGunPakSetLoaded(int set) { mLoadedGunPakSets &= ~(1 << set); }

bool CMFGameLoader::IsGunPakSetLoaded(int set) const {
  return (mLoadedGunPakSets & (1 << set)) != 0;
}

void CMFGameLoader::SelectGunPakSet() {
  int selected = 0;
  if (gpGameState->GetGameMode().GetGameModeType() != 'SNGL') {
    selected = gpGameState->GetGameMode().GetNumPlayers() <= 2 ? 1 : 2;
  }
  for (int set = 0; set < 3; ++set) {
    if (set == selected) {
      if (!IsGunPakSetLoaded(set)) {
        LoadGunPakSet(set);
      }
    } else if (IsGunPakSetLoaded(set)) {
      UnloadGunPakSet(set);
    }
  }
}

void CMFGameLoader::LoadGunPakSet(int set) {
  CResLoader& loader = gpResourceFactory->GetResLoader();
  loader.AddPakFileAsync(rstl::string_l(skGunPakSets[set][0]), true, false);
  loader.AddPakFileAsync(rstl::string_l(skGunPakSets[set][1]), true, false);
  if (strlen(skGunPakSets[set][2]) != 0) {
    loader.AddPakFileAsync(rstl::string_l(skGunPakSets[set][2]), true, false);
  }
  MarkGunPakSetLoaded(set);
}

void CMFGameLoader::UnloadGunPakSet(int set) {
  CResLoader& loader = gpResourceFactory->GetResLoader();
  loader.RemovePakFile(rstl::string_l(skGunPakSets[set][0]));
  loader.RemovePakFile(rstl::string_l(skGunPakSets[set][1]));
  if (strlen(skGunPakSets[set][2]) != 0) {
    loader.RemovePakFile(rstl::string_l(skGunPakSets[set][2]));
  }
  ClearGunPakSetLoaded(set);
}

static rstl::vector< rstl::string > BuildGunPakList() {
  rstl::vector< rstl::string > names;
  names.reserve(9);
  for (int set = 0; set < 3; ++set) {
    names.push_back(rstl::string_l(skGunPakSets[set][0]));
    names.push_back(rstl::string_l(skGunPakSets[set][1]));
    if (strlen(skGunPakSets[set][2]) != 0) {
      names.push_back(rstl::string_l(skGunPakSets[set][2]));
    }
  }
  rstl::sort(names.begin(), names.end());
  return names;
}
