#include "Kyoto/Animation/CTreeUtils.hpp"

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CTransitionDatabase.hpp"
#include "Kyoto/Animation/IMetaTrans.hpp"

rstl::ncrc_ptr< CAnimTreeNode >
CTreeUtils::GetTransitionTree(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                              const rstl::ncrc_ptr< CAnimTreeNode >& b,
                              const CAnimSysContext& animCtx) {
  rstl::rc_ptr< IMetaTrans > trans = GetMetaTrans(a, b, animCtx);
  return trans->GetTransitionTree(a, b, animCtx);
}

rstl::rc_ptr< IMetaTrans > CTreeUtils::GetMetaTrans(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                                    const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                                    const CAnimSysContext& animCtx) {
  CAnimTreeEffectiveContribution contribA = a->GetContributionOfHighestInfluence();
  CAnimTreeEffectiveContribution contribB = b->GetContributionOfHighestInfluence();
  return animCtx.GetTransitionDatabase().NonConstCopy()->GetMetaTrans(
      contribA.GetAnimDatabaseIndex(), contribB.GetAnimDatabaseIndex());
}
