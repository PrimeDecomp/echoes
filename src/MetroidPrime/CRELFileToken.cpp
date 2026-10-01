#include "MetroidPrime/CRELFileToken.hpp"

#include "MetroidPrime/CRELFileManager.hpp"
#include "MetroidPrime/CRelFile.hpp"

static rstl::string GetRelFilePath(const rstl::string& name) {
  static const char* skRelDirectory = "RELProd/";
  rstl::string path = skRelDirectory + name;
  return path;
}

CRELFileToken::CRELFileToken(const rstl::string& name, int loadMode)
: mFile(gpRelFileManager->GetFile(GetRelFilePath(name))), mLoaded(false) {
  if (loadMode == 0) {
    Load();
  }
}

CRELFileToken::CRELFileToken(const CRELFileToken& other) : mFile(other.mFile), mLoaded(false) {
  mFile->AddReference();
  if (other.mLoaded) {
    Load();
  }
}

CRELFileToken::~CRELFileToken() {
  Unload();
  mFile->RemoveReference();
}

void CRELFileToken::Load() {
  if (!mLoaded) {
    mFile->AddLoadRequest();
    mLoaded = true;
  }
}

void CRELFileToken::Unload() {
  if (mLoaded) {
    mFile->RemoveLoadRequest();
    mLoaded = false;
  }
}

bool CRELFileToken::IsLoaded() const { return mFile->IsLoaded(); }

const rstl::string& CRELFileToken::GetFileName() const { return mFile->GetFileName(); }
