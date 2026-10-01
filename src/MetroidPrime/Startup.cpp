#include "MetroidPrime/CSaveRegion.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Graphics/CGraphicsSys.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "dolphin/dvd.h"
#include "dolphin/os/OSModule.h"
#include "rstl/single_ptr.hpp"
#include "string.h"

#include "MetroidPrime/StartupFont.inc"

class CCubeRenderer;
class IController;

extern "C" void OSGetSavedRegion(void** start, void** end);
extern "C" void OSSetSaveRegion(void* start, void* end);
extern "C" void InvokeCMain(int argc, char** argv, COsContext* context, void* saveRegion,
                            CMemorySys* memorySys, void* unknownSubsystem);

void* CSaveRegion::mSaveBuffer;
const void* CSaveRegion::mNonVolatileSettingsBuf;
CCubeRenderer* gpRender;
const TToken< CRasterFont >* gpDefaultFont;
IController* gpController;
bool COsContext::mProgressiveMode;

// The native startup toggles this flag around graphics/main lifetime. Its owner is unknown.
static bool sUnknownSubsystemInitialized;

// Guessed name. The string-table buffer is retained by the OS until process exit.
class CRelDebugSupport {
public:
  CRelDebugSupport();
  void Update();
  bool IsLoaded() const { return mLoaded; }

private:
  rstl::single_ptr< CDvdRequest > mRequest;
  void* mStringTable;
  bool mLoaded;
};
CHECK_SIZEOF(CRelDebugSupport, 0xc)

// Guessed name; Prime's equivalent is CGameGlobalObjects::LoadDefaultFont.
static CRasterFont* LoadDefaultFont() {
  CLZOInputStream fontStream(rs_new CMemoryInStream(sDefaultFontData, sizeof(sDefaultFontData)),
                             sizeof(sDefaultFontData), 0x1660);
  CRasterFont* font = rs_new CRasterFont(fontStream, nullptr);
  CLZOInputStream textureStream(
      rs_new CMemoryInStream(sDefaultFontTexture, sizeof(sDefaultFontTexture)),
      sizeof(sDefaultFontTexture), 0xa34);
  font->SetTexture(rs_new CTexture(textureStream, CTexture::kAM_Zero, CTexture::kBK_Zero));
  return font;
}

// Guessed name; this restores only the progressive-mode bit before graphics startup.
static COsContext& RestoreProgressiveMode(COsContext& context) {
  if (CSaveRegion::GetNonVolatileSettingsBuffer() != nullptr) {
    CMemoryInStream stream(CSaveRegion::GetNonVolatileSettingsBuffer(),
                           CSaveRegion::kSaveBufferSize);
    CBitStreamReader bits(stream);
    COsContext::SetProgressiveMode(bits.ReadPackedBool());
  }
  return context;
}

CRelDebugSupport::CRelDebugSupport() : mRequest(nullptr), mStringTable(nullptr), mLoaded(false) {}

void CRelDebugSupport::Update() {
  if (!mLoaded && mRequest.null()) {
    if (CDvdFile::FileExists("_MetroidR.CWP.str")) {
      CDvdFile file("_MetroidR.CWP.str");
      const uint size = (file.Length() + 31) & ~31;
      mStringTable = CMemory::Alloc(size, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1,
                                    IAllocator::kTP_Heap,
                                    CCallStack(-1, "MetroWerks REL Debug Support", "__Ignore"));
      mRequest = file.SyncRead(mStringTable, size);
    } else {
      mLoaded = true;
      return;
    }
  }

  if (!mRequest.null() && mRequest->IsComplete()) {
    OSSetStringTable(mStringTable);
    mRequest = nullptr;
    mLoaded = true;
  }
}

CSaveRegion::CSaveRegion(COsContext& context) {
  void* start = nullptr;
  void* end;
  OSGetSavedRegion(&start, &end);
  OSSetSaveRegion(nullptr, nullptr);
  mSaveBuffer = context.AllocFromArena(kSaveBufferSize);
  if (start != nullptr) {
    memcpy(mSaveBuffer, start, kSaveBufferSize);
    mNonVolatileSettingsBuf = mSaveBuffer;
  }
}

int main(int argc, char** argv) {
  DVDSetAutoFatalMessaging(TRUE);
  SetErrorHandlers();
  COsContext context(true, true);
  CSaveRegion saveRegion(context);
  CMemorySys memorySys(RestoreProgressiveMode(context), CMemorySys::GetGameAllocator());

  // Only its address is passed; the native code never initializes this object's storage.
  uchar unknownSubsystem;
  if (sUnknownSubsystemInitialized != true) {
    sUnknownSubsystemInitialized = true;
  }
  {
    CGraphicsSys graphicsSys(context, memorySys, COsContext::GetProgressiveMode());
    TToken< CRasterFont > defaultFont(LoadDefaultFont());
    gpDefaultFont = &defaultFont;
    CRelDebugSupport debugSupport;
    while (!debugSupport.IsLoaded()) {
      debugSupport.Update();
    }

    InvokeCMain(argc, argv, &context, &saveRegion, &memorySys, &unknownSubsystem);
  }
  if (sUnknownSubsystemInitialized == true) {
    sUnknownSubsystemInitialized = false;
  }
  return 0;
}
