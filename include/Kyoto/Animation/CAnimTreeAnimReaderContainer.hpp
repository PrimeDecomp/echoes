#ifndef _CANIMTREEANIMREADERCONTAINER
#define _CANIMTREEANIMREADERCONTAINER

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeAnimReaderContainer : public CAnimTreeNode {
public:
  CAnimTreeAnimReaderContainer(const rstl::ownership_transfer< IAnimReader >& reader,
                               const rstl::string& name, uint animDbIdx)
  : CAnimTreeNode(name), mReader(reader), mAnimDbIdx(animDbIdx) {}

  // IAnimReader
  ~CAnimTreeAnimReaderContainer() override {}
  SAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int additive) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int additive) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int additive) const override;
  bool VGetBoolPOIState(uint nameHash) const override;
  s32 VGetInt32POIState(uint nameHash) const override;
  CParticleData::EParentedMode VGetParticlePOIState(uint nameHash) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data,
                   const CCharAnimTime& time) const override;
  void VGetSegData(const CCharLayoutInfo& layout, CJointData_LinearStorage& data) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
  SAdvancementResults VGetAdvancementResults(const CCharAnimTime& time,
                                             const CCharAnimTime& startOffset) const override;

  // CAnimTreeNode
  uint Depth() const override { return 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override { return 0; }
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float weight,
      rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

private:
  rstl::object_owner< IAnimReader, rstl::call_deep_clone< IAnimReader > > mReader;
  uint mAnimDbIdx;
};
CHECK_SIZEOF(CAnimTreeAnimReaderContainer, 0x20)

#endif // _CANIMTREEANIMREADERCONTAINER
