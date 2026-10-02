#ifndef _CSPLASHSCREEN
#define _CSPLASHSCREEN

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/optional_object.hpp"

class CSplashScreen : public CIOWin {
public:
  // Guessed names; order recovered from the native texture table and startup flow.
  enum ESplashScreen {
    kSplashScreen_ProgressiveCheck,
    kSplashScreen_HealthWarning,
    kSplashScreen_Nintendo,
    kSplashScreen_Retro,
    kSplashScreen_Dolby,
    kSplashScreen_MAX,
  };

  explicit CSplashScreen(ESplashScreen splash);
  // CIOWin
  ~CSplashScreen() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;

private:
  // Guessed names.
  enum EProgressivePhase { kPP_Initial, kPP_Selection, kPP_Confirmation, kPP_Complete };
  ESplashScreen mSplash;
  float mSplashTimeout;
  float mProgressiveSelectionTimeout;
  EProgressivePhase mProgressivePhase;
  bool mProgressiveMode;
  bool mTexturesLoaded;
  rstl::optional_object< TCachedToken< CTexture > > mSplashTexture;
  rstl::optional_object< TCachedToken< CTexture > > mPressStartTexture;
};
CHECK_SIZEOF(CSplashScreen, 0x48)

#endif // _CSPLASHSCREEN
