#ifndef _CDARKSAMUSBATTLESTAGE
#define _CDARKSAMUSBATTLESTAGE

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/ScriptLoader/SLdrDarkSamusBattleStage.hpp"

// Guessed name; the class follows the DSBS loader and the DarkSamusBattleStage REL.
// It carries the stage's action tuning for the Dark Samus boss.
class CDarkSamusBattleStage : public CEntity {
public:
  CDarkSamusBattleStage(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                        const SLdrDSStageInfo& stageInfo);

  // CEntity
  ~CDarkSamusBattleStage() override;
  CEntity* TypesMatch(int typeId) const override;

  const SLdrDSStageInfo& GetStageInfo() const; // Guessed name

private:
  SLdrDSStageInfo mStageInfo; // Guessed name
};
CHECK_SIZEOF(CDarkSamusBattleStage, 0xac)

#endif // _CDARKSAMUSBATTLESTAGE
