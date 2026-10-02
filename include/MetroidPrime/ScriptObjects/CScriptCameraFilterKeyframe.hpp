#ifndef _CSCRIPTCAMERAFILTERKEYFRAME
#define _CSCRIPTCAMERAFILTERKEYFRAME

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"

#include "Kyoto/Graphics/CColor.hpp"

class CScriptCameraFilterKeyframe : public CEntity {
public:
  CScriptCameraFilterKeyframe(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                              CCameraFilterPass::EFilterType type,
                              CCameraFilterPass::EFilterShape shape, int filterStage,
                              int filterGroup, float colorR, float colorG, float colorB,
                              float colorA, float timeIn, float timeOut, CAssetId txtr);

  // CEntity
  ~CScriptCameraFilterKeyframe() override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

private:
  CCameraFilterPass::EFilterType mType;
  CCameraFilterPass::EFilterShape mShape;
  int mFilterStage;
  int mFilterGroup;
  CColor mColor;
  float mTimeIn;
  float mTimeOut;
  CAssetId mTxtr;
};
CHECK_SIZEOF(CScriptCameraFilterKeyframe, 0x44)

#endif // _CSCRIPTCAMERAFILTERKEYFRAME
