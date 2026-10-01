#ifndef _CRELFILEMANAGER
#define _CRELFILEMANAGER

#include "rstl/map.hpp"
#include "rstl/string.hpp"

class CRelFile;

// Guessed class and method names. Partial interface to the REL loading service.
class CRELFileManager {
public:
  CRELFileManager();
  ~CRELFileManager();
  void Update();
  void WaitForAllFiles(); // Guessed name.

private:
  rstl::map< rstl::string, CRelFile* > mFiles;
};
CHECK_SIZEOF(CRELFileManager, 0x14)

extern CRELFileManager* gpRelFileManager;

#endif // _CRELFILEMANAGER
