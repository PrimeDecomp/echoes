#ifndef _CSCRIPTDYNAMICLIGHT
#define _CSCRIPTDYNAMICLIGHT

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CGameSpline.hpp"
#include "MetroidPrime/CGameLight.hpp"

class CGameSplineDesc;

// Class spelling is corroborated by the Echoes Wii cast export.
class CScriptDynamicLight : public CGameLight {
public:
  // Reconstructed names; these domains differ from CLight's renderer type values.
  enum ELightKind { kLK_LocalAmbient, kLK_Directional, kLK_Point, kLK_Spot };
  enum ELightSet {
    kLS_LayerOne,
    kLS_LayerTwo,
    kLS_BothLayers,
    kLS_World,
    kLS_LayerOneAndWorld,
    kLS_LayerTwoAndWorld,
    kLS_All,
  };

  // Guessed record name, supported by construction, copies and runtime consumers.
  struct SDescription {
    SDescription(ELightKind kind, const CColor& color, const CMayaSpline& intensity,
                 float intensityDuration, bool intensityLoops, EFalloffType falloffType,
                 const CMayaSpline& falloff, float falloffDuration, bool falloffLoops,
                 const CMayaSpline& spotlight, float spotlightDuration, bool spotlightLoops,
                 ELightSet lightSet, const CVector3f& parentTranslation,
                 const CVector3f& parentRotation, const rstl::string& locator,
                 bool useParentRotation);
    ~SDescription();

    bool UsesLayerOne() const;
    bool UsesLayerTwo() const;
    bool UsesWorld() const;

    ELightKind mKind;
    CColor mColor;
    CMayaSpline mIntensitySpline;
    float mIntensityDuration;
    EFalloffType mFalloffType;
    CMayaSpline mFalloffSpline;
    float mFalloffDuration;
    CMayaSpline mSpotlightSpline;
    float mSpotlightDuration;
    CVector3f mParentTranslation;
    CVector3f mParentRotation;
    rstl::string mLocatorName;
    ELightSet mLightSet;
    bool mIntensityLoops : 1;
    bool mFalloffLoops : 1;
    bool mSpotlightLoops : 1;
    bool mUseParentRotation : 1;
  };

  CScriptDynamicLight(TUniqueId uid, TAreaId areaId, const CLight& light,
                      const SDescription& description, const rstl::string& name,
                      const CEntityInfo& info, const CTransform4f& xf,
                      const CGameSplineDesc& spline);

  // CEntity
  ~CScriptDynamicLight() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;

  bool UsesLayerOne() const { return mLayerOne; }
  bool UsesLayerTwo() const { return mLayerTwo; }
  bool UsesWorld() const { return mWorld; }

private:
  // Guessed private method names, recovered from the respective consumers.
  void FindLightReceivers(CStateManager& mgr);
  void FindParent(CStateManager& mgr);
  void FindTarget(CStateManager& mgr);
  void UpdateLight(float dt);
  void UpdateSpline(float dt);
  void UpdateParent(CStateManager& mgr);
  void UpdateTarget(CStateManager& mgr);

  SDescription mDescription;
  float mIntensity;
  float mIntensityTime;
  float mFalloffTime;
  float mSpotlightTime;
  float mSplineTime;
  CGameSpline mSpline;
  TUniqueId mParentId;
  CSegId mParentLocator;
  CTransform4f mParentTransform;
  TUniqueId mTargetId;
  bool mHasSpline : 1;
  bool mSplineLoops : 1;
  bool mWorld : 1;
  bool mLayerOne : 1;
  bool mLayerTwo : 1;
  bool mHasParent : 1;
  bool mUseParentLocator : 1;
};
CHECK_SIZEOF(CScriptDynamicLight, 0x448)

#endif // _CSCRIPTDYNAMICLIGHT
