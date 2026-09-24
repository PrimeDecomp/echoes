#ifndef _CRESLOADER
#define _CRESLOADER

#include "types.h"

#include "rstl/list.hpp"
#include "rstl/string.hpp"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/CPakFile.hpp"

class CARAMDvdRequest;

class CResLoader {
public:
  int GetPakCount() const;
  CPakFile& GetPakFile(int idx) const;
  void AddPakFileAsync(const rstl::string&, bool, bool);
  void AsyncIdlePakLoading();
  bool AreAllPaksLoaded() const;
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, char* extBuf);
  CInputStream* LoadNewResourceSync(const SObjectTag& tag, int, int, char* extBuf);
  CARAMDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int, int, char*);

  FourCC GetResourceTypeById(CAssetId) const;
  uint ResourceSize(const SObjectTag& tag) const;

private:
  rstl::list< unkptr > x0_aramList;
  rstl::list< unkptr > x18_pakLoadedList;
  rstl::list< unkptr > x30_pakLoadingList;
  unkptr x48_curPak;
  CAssetId x4c_cachedResId;
  const CPakFile::SResInfo* x50_cachedResInfo;
  bool x54_forwardSeek;
};

#endif // _CRESLOADER
