#ifndef _CSCRIPTSPIDERBALLATTRACTIONSURFACE
#define _CSCRIPTSPIDERBALLATTRACTIONSURFACE

#include "MetroidPrime/CActor.hpp"

// Guessed name, correlated with Prime and the Echoes Spider Ball surface loader.
class CScriptSpiderBallAttractionSurface : public CActor {
public:
  CScriptSpiderBallAttractionSurface(TUniqueId uid, const rstl::string& name,
                                     const CEntityInfo& info, const CTransform4f& xf,
                                     const CVector3f& scale);

  // CEntity
  ~CScriptSpiderBallAttractionSurface() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& actor, CStateManager& mgr) override;

  const CVector3f& GetScale() const { return mScale; }

private:
  CVector3f mScale;
  CAABox mAabb;
};
CHECK_SIZEOF(CScriptSpiderBallAttractionSurface, 0x180)

#endif // _CSCRIPTSPIDERBALLATTRACTIONSURFACE
