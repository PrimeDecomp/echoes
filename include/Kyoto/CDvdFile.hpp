#ifndef _CDVDFILE
#define _CDVDFILE

#include "Kyoto/CDvdRequest.hpp"
#include "types.h"

#include "rstl/auto_ptr.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

enum ESeekOrigin { kSO_Set, kSO_Cur, kSO_End };

class CDvdFile;
struct CDvdFileARAM;
struct DVDFileInfo;
class CDvdFile {
public:
  CDvdFile(const char* name);
  ~CDvdFile();
  const rstl::string& GetFilename() const { return mFilename; }
  uint Length() { return mSize; }
  void HandleDVDInterrupt();
  void HandleARAMInterrupt();
  void PingARAMTransfer();
  void TryARAMFile();
  void PushARAMFileLoad();
  void PopARAMFileLoad();
  bool IsARAMFileLoaded();
  bool IsARAMFile() const { return mARAMAllocated; }
  void StartARAMFileLoad();
  void StallForARAMFile();
  CDvdRequest* SyncRead(void* buf, uint len);
  void SyncSeekRead(void* buf, uint len, ESeekOrigin, int offset);
  CDvdRequest* AsyncSeekRead(void* buf, uint len, ESeekOrigin, int offset);
  void CloseFile();
  void CalcFileOffset(int offset, ESeekOrigin origin);
  void UpdateFilePos(int pos);
  const int GetFileSize() const { return mSize; }

  static bool FileExists(const char*);
  static void DVDARAMXferCallback(s32, DVDFileInfo*);
  static void ARAMARAMXferCallback(u32 addr);
  static void internalCallback(s32, DVDFileInfo*);

private:
  int mFileEntry;
  uchar* mARAMBuffer;
  bool mARAMAllocated;
  bool mARAMPopped;
  rstl::single_ptr< CDvdFileARAM > mARAMFile;
  int mOffset;
  int mSize;
  rstl::string mFilename;
};
CHECK_SIZEOF(CDvdFile, 0x28)

#endif // _CDVDFILE
