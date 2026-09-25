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

  bool GetIsGameDraw() const;
  void PreDraw(CStateManager& mgr, bool cameraActive);
  void Draw(const CStateManager& mgr) const;
  void Update(const CStateManager& mgr, float dt, CRandom16& random, CArchitectureQueue& queue,
              bool cameraActive);
  void ProcessControllerInput(const CStateManager& mgr, const CFinalInput& input, float dt);
  bool CheckLoadComplete(const CStateManager& mgr);
  void PauseGame(const CStateManager& mgr, EInGameGuiState state);
  void ShowPauseGameHudMessage(const CStateManager& mgr, CAssetId message, float time);
  void StopSounds();            // Guessed name
  bool IsInPausedState() const; // Guessed name
  void PrepareScanDisplay(const CStateManager& mgr, int playerIndex);
  void fn_80225a30(float, const CStateManager&, const CColor&, const CColor&, const CColor&,
                   const CColor*, int, const CVector3f&) const;

private:
  typedef rstl::reserved_vector< TToken< CDependencyGroup >, 3 > TPauseScreenDGRPs;
  typedef rstl::pair< uint, TToken< CTexture > > TDumpedTexture;

  static TPauseScreenDGRPs LockPauseScreenDependencies();
  bool CheckDGRPLoadComplete();
  void InitializeDumpableARAMTextures();
  void DestroyAreaTextures(const CStateManager& mgr);
  bool TryReloadAreaTextures();
  bool IsTextureInPauseScreen(CAssetId id) const;
  void EnsureStates(const CStateManager& mgr);
  void BeginStateTransition(EInGameGuiState state, const CStateManager& mgr);
  void DoStateTransition(const CStateManager& mgr);
  void TryCompleteStateTransition();
  bool IsTransitionReady() const; // Guessed name
  void UpdateAutoMapper(const CStateManager& mgr, float dt);
  void DrawDarkVisorMask() const; // Guessed name

  int mPlayerIndex;
  bool mIsSinglePlayer;
  TCachedToken< CTexture > mDeathDot;
  CFaceplateDecoration mFaceplateDecoration;
  // Owning pointers. Construction/cleanup awaits the corresponding shared class interfaces.
  CPlayerVisor* mPlayerVisor;
  CSamusHud* mSamusHud;
  CAutoMapper* mAutoMapper;
  CSamusFaceReflection* mSamusReflection;
  CPauseScreenBlur* mPauseScreenBlur;
  CInGameQuitScreen* mQuitScreen;
  CMessageScreen* mMessageScreen;
  CPauseScreen* mPauseScreen;
  CTurretHud* mTurretHud;
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
