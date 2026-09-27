#ifndef _CTRANSITIONMANAGER
#define _CTRANSITIONMANAGER

#include "Kyoto/Animation/CAnimSysContext.hpp"

class CAnimTreeNode;
class CTransitionManager {
public:
  CTransitionManager(const CAnimSysContext& context);
  ~CTransitionManager();
  rstl::rc_ptr< CAnimTreeNode > GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                  const rstl::ncrc_ptr< CAnimTreeNode >& b) const;

private:
  CAnimSysContext mContext;
};

#endif // _CTRANSITIONMANAGER
