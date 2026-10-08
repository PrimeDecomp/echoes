#ifndef _CSCRIPTRIFTPORTAL
#define _CSCRIPTRIFTPORTAL

#include "types.h"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/CModelData.hpp"

// Guessed class: a rift portal that draws a swirling background, an incandescent glow and a line
// effect around its main model, and that pulls in or destroys projectiles that come close.
class CScriptRiftPortal : public CActor {
public:
  CScriptRiftPortal(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CModelData& model, const CModelData& backgroundModel,
                    const CModelData& incandescentModel, const CModelData& lineModel,
                    const CTransform4f& xf, const CVector3f& scale, bool ripPortal,
                    int projectileAttraction, float projectileBoxWidth, float projectileAngle,
                    float projectileDestructionRadius);

  // CEntity
  ~CScriptRiftPortal() override;
  CEntity* TypesMatch(int typeId) const override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(CStateManager& mgr, const CScriptMsg& msg) override;
  void SetActive(bool active) override;

  // CActor
  void PreRender(CStateManager& mgr) override;
  void AddToRenderer(const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  bool CanRenderUnsorted(const CStateManager& mgr) const override;
  void PreRenderAllViewports(CStateManager& mgr) override;

private:
  void AdvanceModel(CModelData& model, CStateManager& mgr, float dt); // Guessed name
  void UpdateProjectiles(CStateManager& mgr, float dt);               // Guessed name

  CVector3f mScale;                   // Guessed name
  CModelData mBackgroundModel;        // Guessed name
  CModelData mIncandescentModel;      // Guessed name
  CModelData mLineModel;              // Guessed name
  float mProjectileBoxWidth;          // Guessed name
  float mProjectileAngle;             // Guessed name
  int mProjectileAttraction;          // Guessed name
  float mProjectileDestructionRadius; // Guessed name
  CAABox mBounds;                     // Guessed name
  bool mOpening : 1;                  // Guessed name
  bool mOpen : 1;                     // Guessed name
  bool mRipPortal : 1;                // Guessed name
  bool x270_27_ : 1;
  bool x270_28_ : 1;
  mutable bool mDrawnOnce : 1; // Guessed name
};
CHECK_SIZEOF(CScriptRiftPortal, 0x278)

#endif // _CSCRIPTRIFTPORTAL
