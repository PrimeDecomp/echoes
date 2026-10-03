#ifndef _CSCANTREESCAN
#define _CSCANTREESCAN

#include "MetroidPrime/ScriptObjects/CScanTreeNode.hpp"

class CInputStream;

// Class, method and member names are guessed.
class CScanTreeScan : public CScanTreeNode {
public:
  CScanTreeScan(int id, const SLdrTransform& transform, CAssetId nameStringTable,
                CAssetId scannableInfo, const rstl::string& nameStringName);

  // CScanTreeNode
  ~CScanTreeScan();
  ENodeType GetNodeType() const;

  CAssetId GetScannableInfo() const;

private:
  CAssetId mScannableInfo;
};
CHECK_SIZEOF(CScanTreeScan, 0x68)

// Guessed loader name.
CScanTreeScan* LoadScanTreeScan(int* id, CInputStream& input);

#endif // _CSCANTREESCAN
