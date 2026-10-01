#include "MetroidPrime/CRELFileManager.hpp"

#include "MetroidPrime/CRelFile.hpp"

#include "Kyoto/Alloc/CMemory.hpp"

CRELFileManager::CRELFileManager() {}

CRELFileManager::~CRELFileManager() {}

CRelFile* CRELFileManager::GetFile(const rstl::string& name) {
  CRelFile* file;
  rstl::map< rstl::string, CRelFile* >::iterator it = mFiles.find(name);
  if (it != mFiles.end()) {
    file = it->second;
  } else {
    file = rs_new CRelFile(name);
    mFiles.insert(rstl::pair< rstl::string, CRelFile* >(name, file));
    file = mFiles.find(name)->second;
  }
  file->AddReference();
  return file;
}

void CRELFileManager::Update() {
  bool erased = true;
  while (erased) {
    erased = false;
    for (rstl::map< rstl::string, CRelFile* >::iterator it = mFiles.begin(); it != mFiles.end();
         ++it) {
      CRelFile* file = it->second;
      file->Update();
      if (file->IsDeletable()) {
        delete file;
        mFiles.erase(it);
        erased = true;
        break;
      }
    }
  }
}

void CRELFileManager::WaitForAllFiles() {
  while (mFiles.size() != 0) {
    Update();
    for (rstl::map< rstl::string, CRelFile* >::const_iterator it = mFiles.begin(); it != mFiles.end();
         ++it) {
      const CRelFile* file = it->second;
      if (file->GetLoadRequestCount() > 0 || file->GetReferenceCount() != 0) {
        break;
      }
    }
  }
}
