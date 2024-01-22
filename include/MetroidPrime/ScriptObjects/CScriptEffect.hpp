#ifndef _CSCRIPTEFFECT
#define _CSCRIPTEFFECT

#include "MetroidPrime/CActor.hpp"

#include "Kyoto/Math/CMayaSpline.hpp"

class CScriptEffect : public CActor {
public:
  struct ParamStruct {
    ParamStruct(const SLdrSpline& spline, int unk1, float unk2, bool unk3);
    ~ParamStruct();

    SLdrSpline m_spline;
    int m_unk1;
    float m_unk2;
    bool m_unk3 : 1;
  };

  CScriptEffect(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CVector3f& scale, CAssetId elscId, int, int, int, int,
                float rateInverseCamDist, float rateInverseCamDistRate, float duration,
                float durationResetWhileVisible, bool useRateCamDistRange,
                float rateCamDistRangeMin, float rateCamDistRangeMax, float rateCamDistRangeFarRate,
                bool combatVisorVisible, bool thermalVisorVisible, bool xrayVisorVisible,
                const CLightParameters& lParms, bool, const ParamStruct& param_25, bool, bool, bool,
                int);

  bool GetFlagAt0x2c8() { return m_flag_0x2c8; }

private:
  char m_pad1[0x170]; // 0x158
  bool m_flag_0x2c8;  // 0x2c8
};

CScriptEffect::ParamStruct::~ParamStruct() {}

#endif // _CSCRIPTEFFECT
