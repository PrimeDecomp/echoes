#ifndef _CPASANIMSTATE
#define _CPASANIMSTATE

#include "types.h"

#include "Kyoto/Animation/CPASAnimInfo.hpp"
#include "Kyoto/Animation/CPASAnimParm.hpp"
#include "Kyoto/Animation/CPASParmInfo.hpp"
#include "Kyoto/Animation/CharacterCommon.hpp"

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CPASAnimState {
public:
  CPASAnimParm GetAnimParmData(int, unsigned int) const;

  pas::EAnimationState GetStateId() const { return mId; }
  bool HasAnims() const { return static_cast< uint >(mAnims.size()) != 0; }

private:
  pas::EAnimationState mId;
  rstl::vector< CPASParmInfo > mParms;
  rstl::vector< CPASAnimInfo > mAnims;
  rstl::vector< int > mSelectionCache;
};
CHECK_SIZEOF(CPASAnimState, 0x34)

#endif // _CPASANIMSTATE
