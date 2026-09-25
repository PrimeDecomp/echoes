#ifndef _CSCRIPTGENERATOR
#define _CSCRIPTGENERATOR

#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CEntity.hpp"

class CScriptGenerator : public CEntity {
public:
  CScriptGenerator(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, int spawnCount,
                   bool noReuseFollowers, const CVector3f& offset, bool noInheritTransform,
                   bool unknown, float minScale, float maxScale);

  // CEntity
  ~CScriptGenerator() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  int mSpawnCount;
  bool mNoReuseFollowers : 1;
  bool mNoInheritTransform : 1;
  bool x28_26_ : 1;
  CVector3f mOffset;
  float mMinScale;
  float mMaxScale;
};
CHECK_SIZEOF(CScriptGenerator, 0x40)

#endif // _CSCRIPTGENERATOR
