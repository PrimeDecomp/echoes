#ifndef _CJOINTDATA_LINEARSTORAGE
#define _CJOINTDATA_LINEARSTORAGE

#include "types.h"

class CJointData_LinearStorage {
public:
  const uchar* GetRotations() const { return x10_rotations; }
  const uchar* GetTranslations() const { return x14_translations; }
  const uchar* GetScales() const { return x18_scales; }
  int GetStride() const { return x1c_stride; }

private:
  int x0_;
  int x4_;
  int x8_;
  int xc_;
  uchar* x10_rotations;
  uchar* x14_translations;
  uchar* x18_scales;
  int x1c_stride;
};

#endif // _CJOINTDATA_LINEARSTORAGE
