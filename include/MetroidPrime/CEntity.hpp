#ifndef _CENTITY
#define _CENTITY

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CValidEntityPredicate.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CStateManager;

class CEntity {
public:
  virtual ~CEntity();
  virtual CEntity* TypesMatch(int typeId) const;
  virtual void PreThink(float dt, CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&);
  void SendActive(CStateManager& mgr, bool active);
  virtual void SetActive(const bool active);

  CEntity(TUniqueId id, const CEntityInfo& info, const rstl::string& name, const uint castFlags);

  void SendScriptMsgs(EScriptObjectState state, CStateManager& mgr,
                      TUniqueId uid = kInvalidUniqueId, EScriptObjectMessage msg = kSM_None);
  void SendScriptMsgs(EScriptObjectState state, CStateManager& mgr, EScriptObjectMessage msg) {
    SendScriptMsgs(state, mgr, kInvalidUniqueId, msg);
  }
  // static inline void SendScriptMsg(CStateManager& mgr, CEntity* to, TUniqueId sender,
  //                                  EScriptObjectMessage msg) {
  //   mgr.SendScriptMsg(to, sender, msg);
  // }
  TUniqueId GetUniqueId() const { return mUniqueId; }
  TEditorId GetEditorId() const { return mEditorId; }
  TAreaId GetAreaIdForPersistence() const;
  TAreaId GetCurrentAreaId() const { return mAreaId; }
  // Reconstructed setter name; current-area reassignment is separate from persistence.
  void SetCurrentAreaId(TAreaId area) { mAreaId = area; }
  const bool GetActive() const { return mActive; }
  bool IsNotInArea() const { return mNotInArea; } // Guessed name
  bool GetUpdateWhileOccluded() const { return mUpdateWhileOccluded; }
  bool GetUpdateDuringCinematicSkip() const { return mUpdateDuringCinematicSkip; }
  void SetUpdateDuringCinematicSkip(bool update) { mUpdateDuringCinematicSkip = update; }
  uint GetCastFlags() const { return mCastFlags; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return mConnections; }
  const rstl::vector< SConnection >& GetConnectionList() const { return mConnections; }

  static rstl::vector< SConnection > NullConnectionList;
  static CEntityInfo NullEntityInfo;

  TUniqueId FindConnectedObject(const CStateManager&, EScriptObjectState,
                                EScriptObjectMessage) const;
  TUniqueId FindConnectedObject_if(const CStateManager&, EScriptObjectState, EScriptObjectMessage,
                                   const CValidEntityPredicate&) const;
  rstl::vector< TUniqueId > FindConnectedObjects(const CStateManager&, EScriptObjectState,
                                                 EScriptObjectMessage) const;
  rstl::vector< TUniqueId > FindConnectedObjects_if(const CStateManager&, EScriptObjectState,
                                                    EScriptObjectMessage,
                                                    const CValidEntityPredicate&) const;
  TUniqueId CheckConnectedObject(const CStateManager&, EScriptObjectState,
                                 EScriptObjectMessage) const;
  TUniqueId CheckConnectedObject_if(const CStateManager&, EScriptObjectState, EScriptObjectMessage,
                                    const CValidEntityPredicate&) const;

private:
  TAreaId mAreaId;
  TUniqueId mUniqueId;
  TEditorId mEditorId;
  rstl::vector< SConnection > mConnections;
  uint mActive : 1;
  uint mNotInArea : 1;
  uint mCastFlags : 4;
  // Guessed names, based on the update dispatch.
  uint mUpdateWhileOccluded : 1;
  uint mUpdateDuringCinematicSkip : 1;
};
CHECK_SIZEOF(CEntity, 0x24)

#endif // _CENTITY
