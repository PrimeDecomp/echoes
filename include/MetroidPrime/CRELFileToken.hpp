#ifndef _CRELFILETOKEN
#define _CRELFILETOKEN

#include "rstl/string.hpp"

class CRelFile;
class CRELFileToken {
public:
  CRELFileToken(const rstl::string& name, int loadMode);
  CRELFileToken(const CRELFileToken& other);
  ~CRELFileToken();

  void Load();           // Guessed name
  bool IsLoaded() const; // Guessed name

private:
  CRelFile* mFile;
  bool mLoaded;
};
CHECK_SIZEOF(CRELFileToken, 0x8)

#endif // _CRELFILETOKEN
