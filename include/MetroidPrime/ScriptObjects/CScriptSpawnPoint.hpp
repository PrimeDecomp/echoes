#ifndef _CSCRIPTSPAWNPOINT
#define _CSCRIPTSPAWNPOINT

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

class CScriptSpawnPoint : public CEntity {
  CTransform4f m_xf;
  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > m_amountForItem;
  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > m_capacityForItem;
  bool m_firstSpawn : 1;
  bool m_morphed : 1;

public:
  CScriptSpawnPoint(TUniqueId, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf,
                    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& amountForItem,
                    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& capacityForItem,
                    bool firstSpawn, bool isMorphed);

  ~CScriptSpawnPoint() override;
  CEntity* TypesMatch(int typeId) const override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg&) override;

  const CTransform4f& GetTransform() const;
  int GetItemAmount(const CPlayerState::EItemType&) const;
  int GetItemCapacity(const CPlayerState::EItemType&) const;
};

#endif // _CSCRIPTSPAWNPOINT
