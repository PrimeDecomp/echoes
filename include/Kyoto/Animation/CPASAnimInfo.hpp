#ifndef _CPASANIMINFO
#define _CPASANIMINFO

#include "Kyoto/Animation/CPASAnimParm.hpp"

#include "rstl/reserved_vector.hpp"

class CPASAnimInfo {
public:
  CPASAnimInfo(int id);
  CPASAnimInfo(int id, const rstl::reserved_vector<CPASAnimParm::UParmValue, 8>& parms);

  int GetAnimId() const { return mId; }
  CPASAnimParm GetAnimParmData(uint idx, CPASAnimParm::EParmType type) const;
  const CPASAnimParm::UParmValue& GetAnimParmValue(uint idx) const;
private:
  uint mId;
  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > mParms;
};

#endif // _CPASANIMINFO
