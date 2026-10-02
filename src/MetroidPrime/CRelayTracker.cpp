#include "MetroidPrime/CRelayTracker.hpp"

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorldSaveGameInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptMemoryRelay.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

CRelayTracker::CRelayTracker() { CMemory::OffsetFakeStatics(static_cast< int >(sizeof(*this))); }

CRelayTracker::CRelayTracker(CBitStreamReader& in, const CWorldSaveGameInfo& saveWorld) {
  rstl::vector< bool > relayStates(saveWorld.GetRelays().size(), false);
  for (int i = 0; i < relayStates.size(); ++i) {
    relayStates[i] = in.ReadBits(1);
  }

  for (int i = 0; i < relayStates.size(); ++i) {
    if (relayStates[i]) {
      mRelays.push_back(saveWorld.GetRelays()[i]);
    }
  }

  CMemory::OffsetFakeStatics(static_cast< int >(sizeof(*this)));
}

CRelayTracker::~CRelayTracker() { CMemory::OffsetFakeStatics(-static_cast< int >(sizeof(*this))); }

void CRelayTracker::PutTo(CBitStreamWriter& out, const CWorldSaveGameInfo& saveWorld) const {
  rstl::vector< bool > relayStates(saveWorld.GetRelays().size(), false);

  rstl::reserved_vector< TEditorId, 512 >::const_iterator it = mRelays.begin();
  for (; it != mRelays.end(); ++it) {
    TEditorId id = *it;
    relayStates[saveWorld.GetRelayIndex(id)] = true;
  }

  for (int i = 0; i < relayStates.size(); ++i) {
    out.WriteBits(relayStates[i] ? 1 : 0, 1);
  }
}

void CRelayTracker::SendMsgs(const TAreaId& areaId, CStateManager& mgr) {
  rstl::reserved_vector< TEditorId, 512 >::iterator it = mRelays.begin();
  for (; it != mRelays.end(); ++it) {
    if (it->AreaNum() == areaId.Value()) {
      if (CEntity* entity = mgr.ObjectById(mgr.GetIdForScript(*it))) {
        entity->SendScriptMsgs(kSS_Active, mgr, kInvalidUniqueId, kSM_None);
      }
    }
  }

  bool removed = true;
  while (removed) {
    removed = false;
    for (rstl::reserved_vector< TEditorId, 512 >::iterator it = mRelays.begin();
         it != mRelays.end(); ++it) {
      if (it->AreaNum() != areaId.Value()) {
        continue;
      }
      CEntity* entity = mgr.ObjectById(mgr.GetIdForScript(*it));
      if (entity != nullptr && static_cast< CScriptMemoryRelay* >(entity)->GetDefaultActive()) {
        mRelays.erase(it);
        removed = true;
        break;
      }
    }
  }
}

void CRelayTracker::AddRelay(const TEditorId& id) {
  rstl::reserved_vector< TEditorId, 512 >::iterator it = mRelays.begin();
  for (; it != mRelays.end(); ++it) {
    if (*it == id) {
      return;
    }
  }

  mRelays.push_back(id);
}

void CRelayTracker::RemoveRelay(const TEditorId& id) {
  rstl::reserved_vector< TEditorId, 512 >::iterator it = mRelays.begin();
  for (; it != mRelays.end(); ++it) {
    if (*it == id) {
      mRelays.erase(it);
      return;
    }
  }
}
