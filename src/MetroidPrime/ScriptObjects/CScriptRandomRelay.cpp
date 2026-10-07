#include "MetroidPrime/ScriptObjects/CScriptRandomRelay.hpp"

#include "MetroidPrime/CObjectList.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/ScriptLoader.hpp"
#include "MetroidPrime/ScriptLoader/SLdrRandomRelay.hpp"

#include "Kyoto/CRandom16.hpp"

#include "rstl/math.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"

CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, const rstl::string& name,
                                       const CEntityInfo& info, int sendSetSize,
                                       int sendSetVariance, const bool percentSize, const bool randomChance)
: CEntity(uid, info, name, 0)
, mSendSetSize(sendSetSize)
, mSendSetVariance(sendSetVariance)
, mPercentSize(percentSize)
, mRandomChance(randomChance) {
  if (percentSize && mSendSetSize > 100) {
    mSendSetSize = 100;
  }
}

CScriptRandomRelay::~CScriptRandomRelay() {}

void CScriptRandomRelay::AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) {
  CEntity::AcceptScriptMsg(mgr, msg);
  switch (msg.GetMessage()) {
  case kSM_SetToZero:
    if (GetActive()) {
      SendLocalScriptMsgs(kSS_Zero, mgr, msg.GetOriginator());
    }
    break;
  default:
    break;
  }
}

void CScriptRandomRelay::SendLocalScriptMsgs(EScriptObjectState state, CStateManager& mgr,
                                             TUniqueId originator) {
  switch (state) {
  case kSS_Zero: {
    if (mRandomChance) {
      if (mgr.Random()->Range(0, 100) <= mSendSetSize) {
        SendScriptMsgs(state, mgr, originator, kSM_None);
      }
      break;
    }

    rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > > objs;
    objs.reserve(16);

    rstl::vector< SConnection >::const_iterator conn = GetConnectionList().begin();
    for (; conn != GetConnectionList().end(); ++conn) {
      if (conn->state == state) {
        CObjectList& objList = mgr.ObjectListById(kOL_All);
        CStateManager::TIdListResult list = mgr.GetIdListForScript(conn->objId);
        if (!(list.first == list.second)) {
          for (CStateManager::TIdList::const_iterator it = list.first; it != list.second; ++it) {
            CEntity* ent = objList.GetObjectById(it->second);
            if (ent != nullptr && ent->GetActive()) {
              if (objs.size() == objs.capacity()) {
                objs.reserve(objs.size() * 2);
              }
              objs.push_back_unsafe(rstl::pair< CEntity*, EScriptObjectMessage >(ent, conn->msg));
            }
          }
        }
      }
    }

    int count = objs.size();
    int targetSetSize =
        mPercentSize ? int(0.5f + (float(count * mSendSetSize) / 100.f)) : mSendSetSize;
    const short variance = CCast::FtoS(float(mSendSetVariance) * (2.f * mgr.Random()->Float()));
    targetSetSize += variance - mSendSetVariance;
    targetSetSize = rstl::min_val(rstl::max_val(0, targetSetSize), 100);

    while (count > targetSetSize) {
      rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > >::iterator it = objs.begin();
      int removeIdx = int(mgr.Random()->Float() * float(objs.size()) * 0.99f);
      for (; removeIdx > 0; --removeIdx) {
        ++it;
        if (objs.end() == it) {
          break;
        }
      }
      if (objs.end() != it) {
        objs.erase(it);
        --count;
      }
    }

    for (rstl::vector< rstl::pair< CEntity*, EScriptObjectMessage > >::iterator it = objs.begin();
         objs.end() != it; ++it) {
      mgr.SendScriptMsg(it->first, GetUniqueId(), it->second, originator);
    }
    break;
  }
  default:
    SendScriptMsgs(state, mgr);
    break;
  }
}

CEntity* LoadRandomRelay(CStateManager& mgr, CInputStream& input, CEntityInfo& info) {
  SLdrRandomRelay sldrThis;
#include "MetroidPrime/ScriptLoader/SLdrRandomRelay.inc"

  return rs_new CScriptRandomRelay(mgr.AllocateUniqueId(), sldrThis.editorProperties.name,
                                   LdrToEntityInfo(info, sldrThis.editorProperties), sldrThis.count,
                                   sldrThis.randomAdjust, sldrThis.percentCount,
                                   sldrThis.isRandomChance);
}
