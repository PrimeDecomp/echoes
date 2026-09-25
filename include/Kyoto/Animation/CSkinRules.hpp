#ifndef _CSKINRULES
#define _CSKINRULES

#include "Kyoto/Animation/CVirtualBone.hpp"

class CInputStream;
class CSkinRules {
public:
  CSkinRules(CInputStream& in);
  ~CSkinRules();

private:
  rstl::vector<CVirtualBone> mVirtualBones;
  uint mVertexCount;
  uint mNormalCount;
};

#endif // _CSKINRULES
