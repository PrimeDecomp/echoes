#include "Kyoto/Animation/IAnimReader.hpp"

IAnimReader::~IAnimReader() {}

rstl::optional_object< rstl::ownership_transfer< IAnimReader > > IAnimReader::VSimplified() {
  return rstl::optional_object_null();
}

SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& time,
                                                        const CCharAnimTime&) const {
  return SAdvancementResults(time);
}

uint IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                                 uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetBoolPOIList(time, listOut, capacity, iterator, additive) : 0;
}

uint IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                                  uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetInt32POIList(time, listOut, capacity, iterator, additive) : 0;
}

uint IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                     uint capacity, uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetParticlePOIList(time, listOut, capacity, iterator, additive)
                                : 0;
}

uint IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                                  uint iterator, int additive) const {
  return time.GreaterThanZero() ? VGetSoundPOIList(time, listOut, capacity, iterator, additive) : 0;
}

bool IAnimReader::IsCAnimTreeNode() const { return false; }

void IAnimReader::VGetSegData(const CCharLayoutInfo&, CJointData_LinearStorage&,
                              const CCharAnimTime&) const {}

void IAnimReader::VGetSegData(const CCharLayoutInfo&, CJointData_LinearStorage&) const {}
