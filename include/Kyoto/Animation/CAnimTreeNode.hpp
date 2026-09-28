#ifndef _CANIMTREENODE
#define _CANIMTREENODE

#include "Kyoto/Animation/IAnimReader.hpp"
#include "rstl/pair.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/reserved_vector.hpp"

class CAnimTreeNode : public IAnimReader {
public:
  explicit CAnimTreeNode(const rstl::string& name);

  // IAnimReader
  ~CAnimTreeNode() override;
  bool IsCAnimTreeNode() const override;

  virtual uint Depth() const = 0;
  virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const = 0;
  virtual uint VGetNumChildren() const = 0;
  virtual rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const = 0;
  virtual void VGetWeightedReaders(
      float weight, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;

  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const {
    return VGetContributionOfHighestInfluence();
  }

  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const { return VGetBestUnblendedChild(); }

  const rstl::string& GetPrimitiveName() const { return mName; }

protected:
  rstl::string mName;
};
CHECK_SIZEOF(CAnimTreeNode, 0x14)

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr);

#endif // _CANIMTREENODE
