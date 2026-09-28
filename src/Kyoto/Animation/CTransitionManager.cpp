#include "Kyoto/Animation/CTransitionManager.hpp"

#include "Kyoto/Animation/CTreeUtils.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CTransitionManager::GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                      const rstl::ncrc_ptr< CAnimTreeNode >& b) const {
  return CTreeUtils::GetTransitionTree(a, b, mContext);
}

rstl::rc_ptr< IMetaTrans >
CTransitionManager::GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                 const rstl::ncrc_ptr< CAnimTreeNode >& b) const {
  return CTreeUtils::GetMetaTrans(a, b, mContext);
}
