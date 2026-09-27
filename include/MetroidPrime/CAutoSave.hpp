#ifndef _CAUTOSAVE
#define _CAUTOSAVE

#include "types.h"

#include "Kyoto/TToken.hpp"
#include "MetroidPrime/CDvdKeepAlive.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/single_ptr.hpp"

class CAudioGroupSet;
class CSaveGameScreen;

class CAutoSave : public CIOWin {
public:
  CAutoSave();

  ~CAutoSave() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Draw() const override;
  bool GetIsContinueDraw() const override;

private:
  enum EState { kS_LoadAudio, kS_Saving };

  EState mState;
  rstl::single_ptr< CSaveGameScreen > mSaveGameScreen;
  TCachedToken< CAudioGroupSet > mAudioGroup;
  CDvdKeepAlive mDvdKeepAlive;
};
CHECK_SIZEOF(CAutoSave, 0x64)

#endif // _CAUTOSAVE
