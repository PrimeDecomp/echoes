#ifndef _CSKINRULES
#define _CSKINRULES

#include "Kyoto/Animation/CVirtualBone.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CCharLayoutInfo;
class CFactoryFnReturn;
class CInputStream;
class CVParamTransfer;
struct SObjectTag;
class CSkinRules {
public:
  CSkinRules(CInputStream& in);
  ~CSkinRules();

  void BuildAccumulatedTransforms(const CPoseAsTransforms_Linear& pose,
                                  const CCharLayoutInfo& layoutInfo, CTransform4f* out) const;
  void LoadMatrixBank(int bank) const; // Guessed name.

  int GetNumPoints() const { return mVertexCount; }
  int GetNumVirtualBones() const { return mVirtualBones.size(); }
  const rstl::vector< CVirtualBone >& GetVirtualBones() const { return mVirtualBones; }
  const uchar* GetVertexToBoneMap() const { return mVertexToBone.get(); }

private:
  rstl::vector< CVirtualBone > mVirtualBones;
  rstl::vector< short > mMatrixIndices;
  int mVertexCount;
  rstl::single_ptr< uchar > mVertexToBone;
};
CHECK_SIZEOF(CSkinRules, 0x28)

CFactoryFnReturn FSkinRulesFactory(const SObjectTag& tag, CInputStream& in,
                                   const CVParamTransfer& params);

#endif // _CSKINRULES
