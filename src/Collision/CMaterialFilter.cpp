#include "Collision/CMaterialFilter.hpp"

const CMaterialFilter CMaterialFilter::skPassEverything;

bool CMaterialFilter::Passes(const CMaterialList& other) const {
  switch (mType) {
  case kFT_Always:
    return true;
  case kFT_Include:
    return other.SharesMaterials(mInclude);
  case kFT_Exclude:
    return !other.SharesMaterials(mExclude);
  case kFT_IncludeExclude:
    return other.SharesMaterials(mInclude) && !other.SharesMaterials(mExclude);
  case kFT_Never:
    return false;
  default:
    return true;
  }
}

CMaterialFilter CMaterialFilter::WithImplicitMaterials(const CMaterialList& materials) const {
  switch (mType) {
  case kFT_Always:
    return *this;
  case kFT_Include:
    if (mInclude.SharesMaterials(materials)) {
      return CMaterialFilter();
    }
    return *this;
  case kFT_Exclude:
    if (mExclude.SharesMaterials(materials)) {
      return CMaterialFilter(CMaterialList(), CMaterialList(0x00000000FFFFFFFF), kFT_Never);
    }
    return *this;
  case kFT_IncludeExclude:
    if (mInclude.SharesMaterials(materials)) {
      return CMaterialFilter(CMaterialList(0x00000000FFFFFFFF), mExclude, kFT_Exclude);
    }
    if (mExclude.SharesMaterials(materials)) {
      return CMaterialFilter(CMaterialList(), CMaterialList(0x00000000FFFFFFFF), kFT_Never);
    }
    return *this;
  case kFT_Never:
    return *this;
  }
  return CMaterialFilter();
}
