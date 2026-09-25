#ifndef _CPLAYERKNOCKBACKMGR
#define _CPLAYERKNOCKBACKMGR

#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"

// Wii SEL exports establish the class name; the remaining player-specific fields
// need semantic recovery. Its RULE_Player constructor is in a separate TU.
class CPlayerKnockBackMgr : public CKnockBackMgr {
public:
  CPlayerKnockBackMgr();
  ~CPlayerKnockBackMgr();

private:
  float x64_;
  float x68_;
  TUniqueId x6c_;
  float x70_;
  float x74_;
  float x78_;
  float x7c_;
  TUniqueId x80_;
  float x84_;
  float x88_;
  uchar x8c_;
  uchar x8d_;
};
CHECK_SIZEOF(CPlayerKnockBackMgr, 0x90)

#endif
