#ifndef _CSPACEPIRATE
#define _CSPACEPIRATE

#include "MetroidPrime/Enemies/CPatterned.hpp"

class CSpacePirate : public CPatterned {
public:
  // CEntity
  CEntity* TypesMatch(int typeId) const override;

  bool AttachActorToPirate(TUniqueId id);
  void DetachActorFromPirate();

private:
  uchar x7c0_[0x2c4];
  TUniqueId mAttachedActor; // Guessed member name.
  uchar xa86_[0xaa];
  void* xb30_;
  uchar xb34_[0x13c];
};
CHECK_SIZEOF(CSpacePirate, 0xc70)

#endif
