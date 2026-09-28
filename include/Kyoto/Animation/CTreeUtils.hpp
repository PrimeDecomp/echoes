#ifndef _CTREEUTILS
#define _CTREEUTILS

#include "rstl/rc_ptr.hpp"

class CAnimTreeNode;
class CAnimSysContext;
class IMetaTrans;
class CTreeUtils {
public:
  // Guessed name; selects the transition between the highest-contributing animations.
  static rstl::rc_ptr< IMetaTrans > GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                 const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                 const CAnimSysContext& animCtx);
  static rstl::ncrc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                           const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                           const CAnimSysContext& animCtx);
};

#endif // _CTREEUTILS
