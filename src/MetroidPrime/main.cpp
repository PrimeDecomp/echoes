#include "MetroidPrime/CMain.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CPakFile.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "MetroidPrime/Tweaks/CTweakGame.hpp"
#include "MetroidPrime/CGameGlobalObjects.hpp"
#include "MetroidPrime/Player/CGameState.hpp"

class CCharacterFactoryBuilder;
class CGameState;
class CMemoryCard;
class CInGameTweakManager;

CResFactory* gpResourceFactory;
CSimplePool* gpSimplePool;
CCharacterFactoryBuilder* gpCharacterFactoryBuilder;
CStringTable* gpStringTable;
CMain* gpMain;
unkptr gpController;
CGameState* gpGameState;
CMemoryCard* gpMemoryCard;
CInGameTweakManager* gpTweakManager;

bool CMain::CheckReset() {
  
}

void CMain::FillInAssetIDs() {
  gpSimplePool->fn_8029c7e8(*gpResourceFactory->GetResourceIdByName("sound_lookup_ATBL"));
}

int CMain::RsMain(int argc, const char* const* argv) {}

void CMain::AsyncIdle(uint time) {
  if (time < 500) {
    uint total = 0;
    for (int i = 0; i < frameTimes.capacity(); ++i) {
      total += frameTimes[i];
    }
    if (total < 500 * frameTimes.capacity()) {
      time = 500;
    } else {
      time = 0;
    }
  }
  frameTimes[frameTimeIdx] = time;
  frameTimeIdx = frameTimeIdx + 1;
  if (frameTimeIdx >= frameTimes.capacity()) {
    frameTimeIdx = 0;
  }

  time = (time <= 5000) ? time : 5000;
  if (time < frameTimeMinimum) {
    time = frameTimeMinimum;
  }
  frameTimeMinimum = 0;
  bool flag = fn_80008A1C();
  if (flag) {
    time = 1000000;
  }

  if (time != 0) {
    gpResourceFactory->AsyncIdle(time, flag);
  }
}

void CMain::AddWorldPaks() {
  rstl::string basePath = gpTweakGame->GetPakFile();
  for (int i = 0; i < 16; ++i) {
    rstl::string pak =
        basePath +
        (i == 0 ? rstl::string_l("") : rstl::string(CBasics::Stringize("%d", i)));
    if (CDvdFile::FileExists((pak + rstl::string_l(".pak")).data())) {
      gpResourceFactory->GetResLoader().AddPakFileAsync(pak, false, true);
    }
  }
}

void CMain::EnsureWorldPaksReady() {
  CResLoader& resLoader = gpResourceFactory->GetResLoader();
  for (int i = 0; i < resLoader.GetPakCount(); ++i) {
    CPakFile& file = resLoader.GetPakFile(i);
    if (file.IsWorldPak()) {
      file.EnsureWorldPakReady();
    }
  }
}

void CMain::StreamNewGameState(CInputStream& in, int saveIdx) {
  // TODO
  gameGlobalObjects->GameState() = nullptr;
  gpGameState = nullptr;
  gameGlobalObjects->GameState() = new CGameState(in, saveIdx);
  gpGameState = gameGlobalObjects->GameState().get();
  // gpGameState->HintOptions().SetHintNextTime();
}
