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

  CMaterialFilter() : include(0x00000000FFFFFFFF), exclude(0), type(kFT_Always) {}
  CMaterialFilter(const CMaterialList& include, const CMaterialList& exclude, EFilterType type)
  : include(include), exclude(exclude), type(type) {}

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
  const CMaterialList& GetIncludeList() const { return include; }
  const CMaterialList& GetExcludeList() const { return exclude; }
  EFilterType GetType() const { return type; }

  // Guessed name; simplify the filter for geometry carrying these fixed materials.
  CMaterialFilter WithImplicitMaterials(const CMaterialList& materials) const {
    switch (type) {
    case kFT_Always:
    case kFT_Never:
      return *this;
    case kFT_Include:
      return include.SharesMaterials(materials) ? GetPassEverything() : *this;
    case kFT_Exclude:
      if (exclude.SharesMaterials(materials)) {
        return CMaterialFilter(CMaterialList(), CMaterialList(0x00000000FFFFFFFF), kFT_Never);
      }
      return *this;
    case kFT_IncludeExclude:
      if (include.SharesMaterials(materials)) {
        return CMaterialFilter(CMaterialList(0x00000000FFFFFFFF), exclude, kFT_Exclude);
      }
      if (exclude.SharesMaterials(materials)) {
        return CMaterialFilter(CMaterialList(), CMaterialList(0x00000000FFFFFFFF), kFT_Never);
      }
      return *this;
    }
    return GetPassEverything();
  }

private:
  CMaterialList include;
  CMaterialList exclude;
  EFilterType type;
};
CHECK_SIZEOF(CMaterialFilter, 0x18)

#endif // _CMATERIALFILTER
