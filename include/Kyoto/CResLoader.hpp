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
  bool ResourceExists(const SObjectTag& tag) const;

private:
  rstl::list< unkptr > mAramList;
  rstl::list< unkptr > mPakLoadedList;
  rstl::list< unkptr > mPakLoadingList;
  unkptr mCurPak;
  CAssetId mCachedResId;
  const CPakFile::SResInfo* mCachedResInfo;
  bool mForwardSeek;
};

#endif // _CRESLOADER
