#ifndef _CRELFILEDEBUGINFO
#define _CRELFILEDEBUGINFO

#include "types.h"

// Guessed class and method names. Address range of a linked REL file, kept in a global list so
// the exception handler can report addresses as offsets into the file.
class CRelFileDebugInfo {
public:
  CRelFileDebugInfo();
  ~CRelFileDebugInfo();

  void Register(const char* name, const void* start, uint size);
  void Unregister();
  bool Contains(int address) const;

  const char* GetName() const { return mName; }
  uint GetStart() const { return mStart; }

  static CRelFileDebugInfo* FindByAddress(int address);

private:
  const char* mName;
  uint mStart;
  uint mSize;
  CRelFileDebugInfo* mNext;
  CRelFileDebugInfo* mPrev;

  static CRelFileDebugInfo* mspHead;
};
CHECK_SIZEOF(CRelFileDebugInfo, 0x14)

#endif // _CRELFILEDEBUGINFO
