#ifndef _CSCANTREECATEGORY
#define _CSCANTREECATEGORY

#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"
#include "rstl/vector.hpp"

class CInputStream;

// Class, method and member names are guessed.
class CScanTreeCategory : public CScanTreeNode {
public:
  CScanTreeCategory(int id, const rstl::vector< int >& children, const SLdrTransform& transform,
                    CAssetId nameStringTable, const rstl::string& nameStringName);

  // CScanTreeNode
  ~CScanTreeCategory();
  ENodeType GetNodeType() const;

  void SetSelectedChild(int node);
  int GetSelectedChild() const;
  int GetChild(int index) const;
  int GetChildCount() const;

private:
  rstl::vector< int > mChildren;
  int mSelectedChild;
};
CHECK_SIZEOF(CScanTreeCategory, 0x78)

// Guessed loader name.
CScanTreeCategory* LoadScanTreeCategory(int* id, const rstl::vector< int >& children,
                                        CInputStream& input);

#endif // _CSCANTREECATEGORY
