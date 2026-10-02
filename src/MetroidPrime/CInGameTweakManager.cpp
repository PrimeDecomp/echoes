#include "MetroidPrime/CInGameTweakManager.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/CResFactory.hpp"

// The retail string pool retains text from the developer tweak-file tools.
static const char* const skTweakFileText[] = {
    "String",
    "Int",
    "Real",
    "Bool",
    "Audio",
    "??(??)",
    "MidiObject",
    "World %8.8x",
    "Area %8.8x MusicObject: %s",
    "World %8.8x Area %8.8x MidiObject: %s",
    "WorldDefault: %8.8x",
    ".adp",
    "Audio/",
    "MIDI: ",
    "TweakFile\n",
    "Version %d\n",
    "Value Count: %d\n",
    "__BAD_TOKEN__",
    "Value\n{\n   Name: %s\n   Type: %s\n   Value: %s\n}\n",
    "TweakFile",
    "Version %d",
    "Value Count: %d",
    "",
    "True",
    ";",
    "%d",
    "%f",
    "False",
    "%f;%f;%f;%s;%d",
    "Value",
    "{",
    "Name:",
    "Type:",
    "Value:",
    " \t\n\r\"",
};

rstl::string CInGameTweakManager::GetIdentifierForMusicEvent(CAssetId area,
                                                             const rstl::string& name) {
  return rstl::string(CBasics::Stringize("Area %8.8x MusicObject: %s", area, name.c_str()));
}

rstl::string CInGameTweakManager::GetIdentifierForMidiEvent(CAssetId world, CAssetId area,
                                                            const rstl::string& name) {
  // The retail formatter swaps the world and area IDs.
  return rstl::string(
      CBasics::Stringize("World %8.8x Area %8.8x MidiObject: %s", area, world, name.c_str()));
}

rstl::string CInGameTweakManager::GetIdentifierForWorldDefaultMusic(CAssetId world) {
  return rstl::string(CBasics::Stringize("WorldDefault: %8.8x", world));
}

rstl::vector< rstl::pair< rstl::string, SObjectTag > >
CResFactory::GetResourceIdToNameList() const {
  return mResLoader.GetResourceIdToNameList();
}

bool CInGameTweakManager::ReadFromMemoryCard(const rstl::string&) { return false; }

const CTweakValue* CInGameTweakManager::GetTweakValue(const rstl::string& name) const {
  for (AUTO(it, mValues.begin()); it != mValues.end(); ++it) {
    if (rstl::operator==(rstl::istring(it->GetName().c_str()), rstl::istring(name.c_str()))) {
      return &*it;
    }
  }

  return nullptr;
}

bool CInGameTweakManager::HasTweakValue(const rstl::string& name) const {
  for (AUTO(it, mValues.begin()); it != mValues.end(); ++it) {
    if (rstl::operator==(rstl::istring(it->GetName().c_str()), rstl::istring(name.c_str()))) {
      return true;
    }
  }

  return false;
}

CInGameTweakManager::CInGameTweakManager() {}
