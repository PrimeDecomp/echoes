#ifndef _CSCANTREE
#define _CSCANTREE

#include "Kyoto/CRandom16.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CDvdRequest;
class CScanTreeNode;

// Guessed name. Minimal declaration of the scan-tree state embedded in CPauseScreen.
class CScanTree {
public:
  CScanTree();
  ~CScanTree();

private:
  int mSelectedNode;
  int mPreviousNode;
  float mTransition;
  float xc_;
  float x10_;
  rstl::auto_ptr< uchar > mBuffer;
  uint mBufferLength;
  rstl::auto_ptr< CDvdRequest > mLoadRequest;
  rstl::vector< rstl::rc_ptr< CScanTreeNode > > mNodes;
  int mRootNode;
  CRandom16 mRandom;
};
CHECK_SIZEOF(CScanTree, 0x40)

#endif // _CSCANTREE
