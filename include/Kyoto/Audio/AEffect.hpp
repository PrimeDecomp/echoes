#ifndef _AEFFECT
#define _AEFFECT

#include "types.h"

// SDK-correlated names; the selected target uses the older 0x90-byte VstP ABI.
struct AEffect;
typedef long (*AudioMasterCallback)(AEffect* effect, long opcode, long index, long value, void* ptr,
                                    float option);
typedef long (*AEffectDispatcher)(AEffect* effect, long opcode, long index, long value, void* ptr,
                                  float option);
typedef void (*AEffectProcess)(AEffect* effect, float** inputs, float** outputs, long sampleFrames);
typedef void (*AEffectSetParameter)(AEffect* effect, long index, float value);
typedef float (*AEffectGetParameter)(AEffect* effect, long index);

struct AEffect {
  long mMagic;
  AEffectDispatcher mDispatcher;
  AEffectProcess mProcess;
  AEffectSetParameter mSetParameter;
  AEffectGetParameter mGetParameter;
  long mNumPrograms;
  long mNumParameters;
  long mNumInputs;
  long mNumOutputs;
  long mFlags;
  long mReserved1;
  long mReserved2;
  long mInitialDelay;
  long mRealtimeQualities;
  long mOfflineQualities;
  float mIORatio;
  void* mObject;
  void* mUser;
  long mUniqueId;
  long mVersion;
  AEffectProcess mProcessReplacing;
  char mFuture[60]; // Reserved storage in the native callback ABI, not class padding.
};
CHECK_SIZEOF(AEffect, 0x90)

enum EEffectFlags {
  kEF_HasEditor = 1,
  kEF_HasClip = 2,
  kEF_HasVu = 4,
  kEF_CanMono = 8,
  kEF_CanReplacing = 16,
  kEF_ProgramChunks = 32,
  kEF_IsSynth = 256,
  kEF_NoSoundInStop = 512,
  kEF_Async = 1024,
  kEF_ExternalBuffer = 2048
};

enum EEffectOpcode {
  kEO_Open,
  kEO_Close,
  kEO_SetProgram,
  kEO_GetProgram,
  kEO_SetProgramName,
  kEO_GetProgramName,
  kEO_GetParameterLabel,
  kEO_GetParameterDisplay,
  kEO_GetParameterName,
  kEO_GetVu,
  kEO_SetSampleRate,
  kEO_SetBlockSize,
  kEO_MainsChanged,
  kEO_EditGetRect,
  kEO_EditOpen,
  kEO_EditClose,
  kEO_EditDraw,
  kEO_EditMouse,
  kEO_EditKey,
  kEO_EditIdle,
  kEO_EditTop,
  kEO_EditSleep,
  kEO_Identify,
  kEO_GetChunk,
  kEO_SetChunk,
  kEO_ProcessEvents,
  kEO_CanBeAutomated,
  kEO_StringToParameter,
  kEO_GetNumCategories,
  kEO_GetProgramNameIndexed,
  kEO_CopyProgram,
  kEO_ConnectInput,
  kEO_ConnectOutput,
  kEO_GetInputProperties,
  kEO_GetOutputProperties,
  kEO_GetPlugCategory,
  kEO_GetCurrentPosition,
  kEO_GetDestinationBuffer,
  kEO_OfflineNotify,
  kEO_OfflinePrepare,
  kEO_OfflineRun,
  kEO_ProcessVariableIo,
  kEO_SetSpeakerArrangement,
  kEO_SetBlockSizeAndSampleRate,
  kEO_SetBypass,
  kEO_GetEffectName,
  kEO_GetErrorText,
  kEO_GetVendorString,
  kEO_GetProductString,
  kEO_GetVendorVersion,
  kEO_VendorSpecific,
  kEO_CanDo,
  kEO_GetTailSize,
  kEO_Idle,
  kEO_GetIcon,
  kEO_SetViewPosition,
  kEO_GetParameterProperties,
  kEO_KeysRequired,
  kEO_GetVstVersion
};

// Pointees are only passed through these recovered interfaces; no invented layouts.
// SDK-correlated host callback opcodes, independently verified against native wrappers.
enum EAudioMasterOpcode {
  kAM_Automate = 0,
  kAM_Version = 1,
  kAM_CurrentId = 2,
  kAM_Idle = 3,
  kAM_PinConnected = 4,
  kAM_WantMidi = 6,
  kAM_GetTime = 7,
  kAM_TempoAt = 10,
  kAM_GetNumAutomatableParameters = 11,
  kAM_GetParameterQuantization = 12,
  kAM_IOChanged = 13,
  kAM_NeedIdle = 14,
  kAM_SizeWindow = 15,
  kAM_GetSampleRate = 16,
  kAM_GetBlockSize = 17,
  kAM_GetInputLatency = 18,
  kAM_GetOutputLatency = 19,
  kAM_GetPreviousPlug = 20,
  kAM_GetNextPlug = 21,
  kAM_WillReplaceOrAccumulate = 22,
  kAM_GetCurrentProcessLevel = 23,
  kAM_GetAutomationState = 24,
  kAM_OfflineStart = 25,
  kAM_OfflineRead = 26,
  kAM_OfflineWrite = 27,
  kAM_OfflineGetCurrentPass = 28,
  kAM_OfflineGetCurrentMetaPass = 29,
  kAM_SetOutputSampleRate = 30,
  kAM_GetSpeakerArrangement = 31,
  kAM_GetVendorString = 32,
  kAM_GetProductString = 33,
  kAM_GetVendorVersion = 34,
  kAM_VendorSpecific = 35,
  kAM_CanDo = 37,
  kAM_GetLanguage = 38,
  kAM_OpenWindow = 39,
  kAM_CloseWindow = 40,
  kAM_GetDirectory = 41,
  kAM_UpdateDisplay = 42
};

struct ERect;
struct VstEvents;
struct VstTimeInfo;
struct VstPinProperties;
struct VstOfflineTask;
struct VstAudioFile;
struct VstSpeakerArrangement;
struct VstWindow;
struct VstVariableIo;
struct VstParameterProperties;

enum VstPlugCategory { kPlugCategUnknown = 0, kPlugCategSynth = 2 };
enum VstOfflineOption {
  kVstOfflineAudio,
  kVstOfflinePeaks,
  kVstOfflineParameter,
  kVstOfflineMarker,
  kVstOfflineCursor,
  kVstOfflineSelection,
  kVstOfflineQueryFiles
};

#endif // _AEFFECT
