#ifndef _CDVDKEEPALIVE
#define _CDVDKEEPALIVE

#include "Kyoto/CDvdFile.hpp"
#include "rstl/auto_ptr.hpp"

// Guessed name: maintains periodic disc reads while a front-end window is active.
class CDvdKeepAlive {
public:
  CDvdKeepAlive();
  ~CDvdKeepAlive();
  void Update();

private:
  CDvdFile mFile;
  rstl::auto_ptr< uchar > mBuffer;
  int mReadOffset;
  rstl::auto_ptr< CDvdRequest > mRequest;
};
CHECK_SIZEOF(CDvdKeepAlive, 0x3c)

#endif // _CDVDKEEPALIVE
