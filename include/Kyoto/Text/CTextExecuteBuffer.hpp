#ifndef _CTEXTEXECUTEBUFFER
#define _CTEXTEXECUTEBUFFER

#include "rstl/list.hpp"
#include "rstl/rc_ptr.hpp"

#include "Kyoto/Text/CSaveableState.hpp"

class CInstruction;
class CBlockInstruction;
class CLineInstruction;

class CTextExecuteBuffer {
  typedef rstl::list< rstl::ncrc_ptr< CInstruction > > InstList;

public:
  CTextExecuteBuffer();

private:
  InstList mInstructions;
  CSaveableState mState;
  CBlockInstruction* mCurrentBlock;
  CLineInstruction* mCurrentLine;
  InstList::const_iterator mCurrentWord;
  int mCurrentY;
  int mCurrentX;
  int mCurrentWordX;
  int mCurrentWordY;
  int mSpaceDistance;
  bool mImageBaseline;
  rstl::list< CSaveableState > mStateStack;
};

CHECK_SIZEOF(CTextExecuteBuffer, 0xe0)

#endif // _CTEXTEXECUTEBUFFER
