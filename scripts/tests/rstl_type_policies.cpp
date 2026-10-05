// Compile-only coverage for the configured G2ME01 compiler.
#include "Kyoto/Animation/CAdditiveAnimationInfo.hpp"
#include "Kyoto/Animation/CLayoutDescription.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/TGameTypes.hpp"

struct CopyOnly {
  CopyOnly(const CopyOnly&);

private:
  CopyOnly& operator=(const CopyOnly&);
};

struct Owner {
  Owner(const Owner&);
  ~Owner();
};

namespace rstl {
RSTL_DECLARE_TRIVIALLY_DESTRUCTIBLE(CopyOnly)
} // namespace rstl

template < typename T, bool Destruction, bool Assignment >
struct CheckPolicy {
  typedef char
      CheckDestruction[rstl::is_trivially_destructible< T >::value == Destruction ? 1 : -1];
  typedef char
      CheckAssignment[rstl::use_assignment_for_construction< T >::value == Assignment ? 1 : -1];
};

CheckPolicy< rstl::pair< TEditorId, bool >, true, true > editorBool;
CheckPolicy< rstl::pair< TEditorId, TUniqueId >, true, true > editorUnique;
CheckPolicy< rstl::pair< TUniqueId, TUniqueId >, true, true > uniqueIds;
CheckPolicy< rstl::pair< CSegId, CSegId >, true, true > segmentIds;
CheckPolicy< rstl::pair< uint, CAdditiveAnimationInfo >, true, true > additiveInfo;
CheckPolicy< CLayoutDescription::CScaledLayoutDescription::ScaleInfo, true, true > scaleInfo;
CheckPolicy< rstl::pair< rstl::pair< int, bool >, const Owner* >, true, true > nested;
CheckPolicy< CWeaponTypeVulnerability, true, true > vulnerability;
CheckPolicy< SObjectTag, false, true > constructionOnly;
CheckPolicy< rstl::pair< CopyOnly, bool >, true, false > destructionOnly;
CheckPolicy< rstl::pair< Owner, bool >, false, false > owning;
CheckPolicy< rstl::pair< const int, bool >, false, false > constMember;
CheckPolicy< rstl::pair< int&, bool >, false, false > referenceMember;
CheckPolicy< rstl::pair< int, float >, false, false > legacyScalarPair;
CheckPolicy< rstl::pair< int, TEditorId >, false, false > legacyPickupPair;
CheckPolicy< rstl::pair< TUniqueId, int >, false, false > legacyAreaDamagePair;
CheckPolicy< rstl::pair< TUniqueId, float >, false, false > legacySeekerPair;

// These must instantiate copy construction without requiring an accessible
// assignment operator, even though CopyOnly's destructor can be omitted.
void CopyNonassignable(void* dest, const rstl::pair< CopyOnly, bool >& src) {
  rstl::construct(dest, src);
}

void CopyConstMember(void* dest, const rstl::pair< const int, bool >& src) {
  rstl::construct(dest, src);
}

void CopyReferenceMember(void* dest, const rstl::pair< int&, bool >& src) {
  rstl::construct(dest, src);
}

void CopyOwner(void* dest, const rstl::pair< Owner, bool >& src) { rstl::construct(dest, src); }

void CopyNested(void* dest, const CLayoutDescription::CScaledLayoutDescription::ScaleInfo& src) {
  rstl::construct< CLayoutDescription::CScaledLayoutDescription::ScaleInfo >(dest, src);
}
