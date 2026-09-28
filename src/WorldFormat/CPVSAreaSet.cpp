#include "WorldFormat/CPVSAreaSet.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

CPVSAreaSet::CPVSAreaSet(int numFeatures, int numLights, int num2ndLights, int numActors,
                         int leafSize, int lightIndexCount, const char* entityIds,
                         const char* lightLeaves, const char* octreeData)
: mNumFeatures(numFeatures)
, mNumLights(numLights)
, mNum2ndLights(num2ndLights)
, mNumActors(numActors)
, mLeafSize(leafSize)
, mLightIndexCount(lightIndexCount)
, mEntityIds(entityIds)
, mLightLeaves(lightLeaves)
, mOctree(CPVSVisOctree::MakePVSVisOctree(octreeData, 68)) {}

rstl::auto_ptr< CPVSAreaSet > CPVSAreaSet::MakeAreaSet(const char* data, int length) {
  CMemoryInStream in(data, length);
  const int numFeatures = in.ReadInt32();
  const int numLights = in.ReadInt32();
  const int num2ndLights = in.ReadInt32();
  const int numActors = in.ReadInt32();
  const int leafSize = in.ReadInt32();
  const int lightIndexCount = in.ReadInt32();

  data += in.GetReadPosition();
  const char* const lightLeaves = data + numActors * 4;
  const char* const octreeData = lightLeaves + lightIndexCount * leafSize;
  return rstl::auto_ptr< CPVSAreaSet >(rs_new CPVSAreaSet(numFeatures, numLights, num2ndLights,
                                                          numActors, leafSize, lightIndexCount,
                                                          data, lightLeaves, octreeData));
}

CPVSVisOctree& CPVSAreaSet::GetVisOctree() const { return mOctree; }

CPVSVisSet CPVSAreaSet::GetLightSet(int lightIndex) const {
  if (lightIndex >= mLightIndexCount) {
    return CPVSVisSet(kVSS_OutOfBounds);
  }

  rstl::auto_ptr< const char > leaf(mLightLeaves + mLeafSize * lightIndex);
  leaf.release();
  return CPVSVisSet(mOctree.GetNumObjects(), mOctree.GetNumLights(), leaf);
}

int CPVSAreaSet::GetEntityIdByIndex(uint index) const {
  return CBasics::SwapBytes(reinterpret_cast< const int* >(mEntityIds)[index]);
}
