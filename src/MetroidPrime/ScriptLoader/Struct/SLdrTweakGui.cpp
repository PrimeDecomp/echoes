#include "MetroidPrime/ScriptLoader/Struct/SLdrTweakGui.hpp"

SLdrTweakGui::SLdrTweakGui()
  : instanceName()
  , misc()
  , scannableObjectDownloadTimes()
  , unknown()
  , echoVisor()
  , scanVisor()
  , logBook()
  , credits()
  , completion()
  , movieVolumes()
{}

SLdrTweakGui::~SLdrTweakGui() {}

void LoadTypedefSLdrTweakGui(SLdrTweakGui& sldrThis, CInputStream& input) {
  int propertyCount = input.ReadUint16();
  for (int i = 0; i < propertyCount; ++i) {
    uint propertyId = (uint)input.ReadInt32();
    u16 propertySize = input.ReadUint16();

    switch (propertyId) {
    case 0x7fda1466:
      sldrThis.instanceName = rstl::string(input);
      break;
    case 0xd45f7663:
      LoadTypedefSLdrTweakGui_Misc(sldrThis.misc, input);
      break;
    case 0x80b13e60:
      LoadTypedefSLdrTweakGui_ScannableObjectDownloadTimes(sldrThis.scannableObjectDownloadTimes, input);
      break;
    case 0x102aa38d:
      LoadTypedefSLdrTweakGui_DarkVisor(sldrThis.unknown, input);
      break;
    case 0x2b698e45:
      LoadTypedefSLdrTweakGui_EchoVisor(sldrThis.echoVisor, input);
      break;
    case 0x40ffb3c4:
      LoadTypedefSLdrTweakGui_ScanVisor(sldrThis.scanVisor, input);
      break;
    case 0x97b8a76a:
      LoadTypedefSLdrTweakGui_LogBook(sldrThis.logBook, input);
      break;
    case 0x77393416:
      LoadTypedefSLdrTweakGui_Credits(sldrThis.credits, input);
      break;
    case 0x02149892:
      LoadTypedefSLdrTweakGui_Completion(sldrThis.completion, input);
      break;
    case 0xa4f61e92:
      LoadTypedefSLdrTweakGui_MovieVolumes(sldrThis.movieVolumes, input);
      break;
    default:
      input.ReadBytes(nullptr, propertySize);
      break;
    }
  }
}