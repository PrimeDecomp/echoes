#ifndef _CVISORFLARE
#define _CVISORFLARE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TAverage.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/vector.hpp"

class CActor;
class CStateManager;
class CVector3f;

class CVisorFlare {
public:
  enum EBlendMode { kBM_Additive, kBM_Blend };

  class CFlareDef {
    mutable TToken< CTexture > mTex;
    float mPos;
    float mScale;
    CColor mColor;

  public:
    CFlareDef(const TToken< CTexture >& tex, float pos, float scale, uint color);
    TToken< CTexture >& GetTexture() const { return mTex; }
    float GetPosition() const { return mPos; }
    float GetScale() const { return mScale; }
    CColor GetColor() const { return mColor; }
  };

  // Echoes retains the unused fade-time argument from Prime.
  CVisorFlare(EBlendMode blendMode, bool distanceScaled, float fadeTime, float angularFalloff,
              float rotationScale, uint darkVisorMode, uint combatVisorMode,
              const rstl::vector< CFlareDef >& flares, bool smallOcclusionTest,
              bool noOcclusionTest);
  ~CVisorFlare();

  void Update(float dt, const CVector3f& pos, const CActor* actor, CStateManager& mgr);
  void Render(const CVector3f& pos, const CActor& actor, const CStateManager& mgr);
  void UpdateFrustum(const CStateManager& mgr, const CVector3f& pos); // Guessed name

private:
  void SetupRenderState(const CStateManager& mgr) const;
  void ResetRenderState(const CStateManager& mgr) const; // Guessed name
  void DrawStreamed(const CColor& color, float sinScale, float cosScale) const;
  void RenderFlares(const CVector3f& pos, const CStateManager& mgr) const; // Guessed name
  void UpdateOcclusion(const CVector3f& pos, const CStateManager& mgr,
                       int playerIndex); // Guessed name

  EBlendMode mBlendMode;
  rstl::vector< CFlareDef > mFlareDefs;
  float mAngularFalloff;
  float mRotationScale;
  float mIntensity;
  int mDarkVisorMode;
  int mCombatVisorMode;
  bool mDistanceScaled : 1;
  // Guessed names for Echoes's framebuffer occlusion state.
  bool mSmallOcclusionTest : 1;
  bool mOutsideFrustum : 1;
  bool mNoOcclusionTest : 1;
  TAverage< float > mOcclusionAverage;
  uchar mOcclusionWarmupFrames;
  CTexture mSavedFramebuffer;
  CTexture mOcclusionTexture;
};

CHECK_SIZEOF(CVisorFlare, 0x110)
NESTED_CHECK_SIZEOF(CVisorFlare, CFlareDef, 0x14)

#endif // _CVISORFLARE
