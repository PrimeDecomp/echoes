#ifndef _CMATERIALFILTER
#define _CMATERIALFILTER

#include "types.h"

#include "Collision/CMaterialList.hpp"

class CMaterialFilter {
  static const CMaterialFilter skPassEverything;

public:
  enum EFilterType {
    kFT_Always,
    kFT_Include,
    kFT_Exclude,
    kFT_IncludeExclude,
    kFT_Never, // Guessed name.
  };

  CMaterialFilter() : mInclude(0x00000000FFFFFFFF), mExclude(0), mType(kFT_Always) {}
  CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type)
  : mInclude(include), mExclude(exclude), mType(type) {}

  static CMaterialFilter MakeInclude(const CMaterialList& include) {
    return CMaterialFilter(include, CMaterialList(), kFT_Include);
  }
  static CMaterialFilter MakeExclude(const CMaterialList& exclude) {
    return CMaterialFilter(CMaterialList(0x00000000FFFFFFFF), exclude, kFT_Exclude);
  }
  static CMaterialFilter MakeIncludeExclude(const CMaterialList& include,
                                            const CMaterialList& exclude) {
    return CMaterialFilter(include, exclude, kFT_IncludeExclude);
  }

  static const CMaterialFilter& GetPassEverything() { return skPassEverything; }

  bool Passes(const CMaterialList& other) const;
  const CMaterialList& GetIncludeList() const { return mInclude; }
  const CMaterialList& GetExcludeList() const { return mExclude; }
  CMaterialList& IncludeList() { return mInclude; }
  CMaterialList& ExcludeList() { return mExclude; }
  EFilterType GetType() const { return mType; }

  // Guessed name; simplify the filter for geometry carrying these fixed materials.
  CMaterialFilter WithImplicitMaterials(const CMaterialList& materials) const;

private:
  CMaterialList mInclude;
  CMaterialList mExclude;
  EFilterType mType;
};
CHECK_SIZEOF(CMaterialFilter, 0x18)

#endif // _CMATERIALFILTER
