#include "MetroidPrime/CSaveRegion.hpp"

#include "Kyoto/Alloc/CMemorySys.hpp"
#include "Kyoto/Basics/COsContext.hpp"
#include "Kyoto/Basics/RAssertDolphin.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequestManager.hpp"
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

#include "MetroidPrime/DefaultFontData.inc"
#include "MetroidPrime/DefaultFontTexture.inc"

extern const char MetroidBuildInfo[] = BUILD_INFO;

class CCubeRenderer;
class IController;

extern "C" void OSGetSavedRegion(void** start, void** end);
extern "C" void OSSetSaveRegion(void* start, void* end);
extern "C" void InvokeCMain(int argc, char** argv, COsContext* context, CSaveRegion* saveRegion,
                            CMemorySys* memorySys, CDvdRequestSys* dvdRequestSys);

void* CSaveRegion::mSaveBuffer;
const void* CSaveRegion::mNonVolatileSettingsBuf;
CCubeRenderer* gpRender;
const TToken< CRasterFont >* gpDefaultFont;
IController* gpController;
bool COsContext::mProgressiveMode;

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
  TToken< CTexture > texture(
      rs_new CTexture(textureStream, CTexture::kAM_Zero, CTexture::kBK_Zero));
  font->SetTexture(texture);
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
    if (CDvdFile::FileExists("/MetroidR_CWP.str")) {
      CDvdFile file("/MetroidR_CWP.str");
      const uint size = (file.Length() + 31) & ~31;
      mStringTable = CMemory::Alloc(size, IAllocator::kHI_RoundUpLen, IAllocator::kSC_Unk1,
                                    IAllocator::kTP_Heap,
                                    CCallStack(-1, "MetroWerks REL Debug Support", " - Ignore"));
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

  CDvdRequestSys dvdRequestSys;
  CGraphicsSys graphicsSys(context, memorySys, COsContext::GetProgressiveMode());
  TToken< CRasterFont > defaultFont(LoadDefaultFont());
  gpDefaultFont = &defaultFont;
  CRelDebugSupport debugSupport;
  while (!debugSupport.IsLoaded()) {
    debugSupport.Update();
  }

  InvokeCMain(argc, argv, &context, &saveRegion, &memorySys, &dvdRequestSys);
  return 0;
}
