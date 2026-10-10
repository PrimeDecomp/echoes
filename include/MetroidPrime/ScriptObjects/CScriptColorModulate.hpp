#ifndef _CSCRIPTCOLORMODULATE
#define _CSCRIPTCOLORMODULATE

#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Math/CMayaSpline.hpp"
#include "MetroidPrime/CEntity.hpp"

class CScriptColorModulate : public CEntity {
public:
  enum EBlendMode { kBM_Alpha, kBM_Additive, kBM_Additive2, kBM_Opaque, kBM_OpaqueAdd };
  enum EFadeState { kFS_AtoB, kFS_BtoA };

  CScriptColorModulate(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CColor& colorA, const CColor& colorB, EBlendMode blendMode,
                       float timeA2B, float timeB2A, bool doReverse, bool resetTargetWhenDone,
                       bool depthCompare, bool depthUpdate, bool depthBackwards, bool autoStart,
                       bool updateTime, bool loopForever, bool externalTime,
                       bool copyModelColorToColorA, const SLdrSpline& controlSpline);

  // CEntity
  ~CScriptColorModulate() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  static TUniqueId FadeInHelper(CStateManager& mgr, TUniqueId obj, float fadeTime);
  static TUniqueId FadeOutHelper(CStateManager& mgr, TUniqueId obj, float fadeTime);
  void SetTargetFlags(CStateManager& mgr, const CModelFlags& flags);
  void End(CStateManager& mgr);
  CModelFlags CalculateFlags(const CColor& color) const;
  void SetExternalTime(float time, CStateManager& mgr); // Guessed name
  void CopyTargetColor(CStateManager& mgr); // Guessed name

private:
  TUniqueId mParent;
  EFadeState mFadeState;
  float mCurTime;
  CColor mColorA;
  CColor mColorB;
  EBlendMode mBlendMode;
  float mTimeA2B;
  float mTimeB2A;
  CMayaSpline mControlSpline;
  bool mDoReverse : 1;
  bool mResetTargetWhenDone : 1;
  bool mDepthCompare : 1;
  bool mDepthUpdate : 1;
  bool mDepthBackwards : 1;
  bool mReversing : 1;
  bool mEnable : 1;
  bool mDieOnEnd : 1;
  bool mIsFadeOutHelper : 1;
  bool mUpdateTime : 1;
  bool mAutoStart : 1;
  bool mLoopForever : 1;
  bool mExternalTime : 1;
  bool mCopyModelColorToColorA : 1;
};
CHECK_SIZEOF(CScriptColorModulate, 0x8c)

#endif // _CSCRIPTCOLORMODULATE
