#ifndef _CINGAMEGUIMANAGER
#define _CINGAMEGUIMANAGER

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CInGameGuiManagerCommon.hpp"
#include "MetroidPrime/Player/CFaceplateDecoration.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CArchitectureQueue;
class CColor;
class CAutoMapper;
class CDependencyGroup;
class CFinalInput;
class CGuiFrame;
class CGuiFrameLoader;
class CGuiWidget;
class CMessageScreen;
class CPauseScreen;
class CPauseScreenBlur;
class CPlayerVisor;
class CRandom16;
class CSamusFaceReflection;
class CSamusHud;
class CStateManager;
class CInGameQuitScreen; // Guessed name
class CTurretHud;        // Guessed name

// Guessed name: the Prime manager's gameplay responsibilities are per-player in Echoes.
class CInGameGuiManager {
public:
  CInGameGuiManager(const CStateManager& mgr, CGuiFrameLoader& hud, CGuiFrameLoader& memo,
                    CGuiFrameLoader* helmet, CGuiFrameLoader* darkMask, int playerIndex);
  ~CInGameGuiManager();

  bool GetIsGameDraw() const;
  CAutoMapper& GetAutoMapper() { return *mAutoMapper; }
  const CAutoMapper* GetAutoMapper() const { return mAutoMapper.get(); }
  void PreDraw(CStateManager& mgr, bool cameraActive);
  void Draw(const CStateManager& mgr) const;
  void Update(const CStateManager& mgr, float dt, CRandom16& random, CArchitectureQueue& queue,
              bool cameraActive);
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input,
                              CArchitectureQueue& queue);
  bool CheckLoadComplete(const CStateManager& mgr);
  void PauseGame(const CStateManager& mgr, EInGameGuiState state);
  void ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message, float time);
  void StopSounds(const CStateManager& mgr); // Guessed name
  void BeginStateTransition(EInGameGuiState state, const CStateManager& mgr);
  bool IsInPausedState() const; // Guessed name
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);
  // Guessed name; forwards the scan palette and camera direction to the renderer.
  void DrawScanVisor(float time, const CStateManager& mgr, const CColor& sweepColor,
                     const CColor& inactiveColor, const CColor& inactiveExternalColor,
                     const CColor* palette, int paletteSize, const CVector3f& direction) const;

private:
  typedef rstl::reserved_vector< TToken< CDependencyGroup >, 3 > TPauseScreenDGRPs;
  typedef rstl::pair< uint, TToken< CTexture > > TDumpedTexture;

  static TPauseScreenDGRPs LockPauseScreenDependencies();
  bool CheckDGRPLoadComplete();
  void InitializeDumpableARAMTextures();
  void DestroyAreaTextures(const CStateManager& mgr);
  uchar TryReloadAreaTextures();
  bool IsTextureInPauseScreen(CAssetId id) const;
  void EnsureStates(const CStateManager& mgr);
  void DoStateTransition(const CStateManager& mgr);
  void TryCompleteStateTransition();
  bool IsTransitionReady() const; // Guessed name
  void UpdateAutoMapper(const CStateManager& mgr, float dt);
  void DrawDarkVisorMask() const; // Guessed name

  int mPlayerIndex;
  bool mIsSinglePlayer;
  TCachedToken< CTexture > mDeathDot;
  CFaceplateDecoration mFaceplateDecoration;
  rstl::single_ptr< CPlayerVisor > mPlayerVisor;
  rstl::single_ptr< CSamusHud > mSamusHud;
  rstl::single_ptr< CAutoMapper > mAutoMapper;
  rstl::single_ptr< CSamusFaceReflection > mSamusReflection;
  rstl::single_ptr< CPauseScreenBlur > mPauseScreenBlur;
  rstl::single_ptr< CInGameQuitScreen > mQuitScreen;
  rstl::single_ptr< CMessageScreen > mMessageScreen;
  rstl::single_ptr< CPauseScreen > mPauseScreen;
  rstl::single_ptr< CTurretHud > mTurretHud;
  CAssetId mPauseGameHudMessage;
  float mPauseGameHudTime;
  TPauseScreenDGRPs mPauseScreenDGRPs;
  rstl::vector< TToken< CDependencyGroup > > mInGameGuiDGRPs;
  rstl::vector< CAssetId > mInGameTextureIds;
  rstl::vector< CToken > mPauseResources;
  rstl::list< TDumpedTexture > mDumpedTextures;
  EInGameGuiState mPrevState;
  EInGameGuiState mNextState;
  uint mHelmetVisMode;
  uint mEnableTargetingManager;
  uint mEnableAutoMapper;
  uint mHudVisMode;
  uint mEnablePlayerVisor;
  CQuaternion mAutoMapperRotation;
  CVector3f mAutoMapperOffset;
  CQuaternion mCameraRotation;
  CVector3f mCameraOffset;
  CTransform4f mMapCameraTransform;
  float mVisorStaticAlpha;
  CGuiWidget* mDarkOuterMask;
  rstl::auto_ptr< CGuiFrame > mDarkMaskFrame;
  bool mLoaded : 1;
  bool mPlayerAlive : 1;
  bool mDeferTransition : 1;
};
CHECK_SIZEOF(CInGameGuiManager, 0x150)

#endif // _CINGAMEGUIMANAGER
