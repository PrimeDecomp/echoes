#ifndef _COBBTREE
#define _COBBTREE

#include "Collision/COBBox.hpp"
#include "WorldFormat/CCollisionEdge.hpp"
#include "WorldFormat/CCollisionPrimitiveData.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class COBBTree : public CCollisionPrimitiveData {
public:
  enum EPreBuiltTrees {
    kPBT_UnitCube,
    kPBT_UnitSphereLow,
    kPBT_UnitSphereMedium,
    kPBT_UnitSphereHigh,
  };

  class CSimpleAllocator {
  public:
    explicit CSimpleAllocator(uint size);
    ~CSimpleAllocator();
    void* Alloc(size_t size);
    uint GetPoolMemSize() const { return mSize; }

  private:
    char* mBuffer;
    uint mSize;
    uint mOffset;
  };

  class CLeafData {
  public:
    explicit CLeafData(const rstl::vector< ushort >& surfaces);
    explicit CLeafData(CInputStream& in);
    const rstl::vector< ushort >& GetSurfaceVector() const { return mSurfaces; }
    uint GetMemoryUsage() const;

  private:
    rstl::vector< ushort > mSurfaces;
  };

  class CNode {
  public:
    CNode(const CTransform4f& xf, const CVector3f& extents, const CNode* left, const CNode* right,
          const CLeafData* leaf);
    explicit CNode(CInputStream& in);
    ~CNode();

    const COBBox& GetOBB() const { return mObb; }
    bool IsLeaf() const { return mIsLeaf; }
    const CNode* GetLeftNode() const { return mLeft; }
    const CNode* GetRightNode() const { return mRight; }
    const CLeafData* GetLeafData() const { return mLeaf; }
    void SetHit(bool hit) const { mHit = hit; }
    bool WasHit() const { return mHit; }
    uint GetMemoryUsage() const;

    static void SetAllocator(CSimpleAllocator* allocator);
    void* operator new(size_t size, const char* file, int line);
    void operator delete(void* ptr, size_t size);

  private:
    COBBox mObb;
    bool mIsLeaf;
    const CNode* mLeft;
    const CNode* mRight;
    const CLeafData* mLeaf;
    mutable bool mHit;

    static CSimpleAllocator* spAllocator;
  };

  struct SIndexData {
    explicit SIndexData(CInputStream& in);

    rstl::vector< u64 > mMaterials;
    rstl::vector< uchar > mVertMaterials;
    rstl::vector< uchar > mEdgeMaterials;
    rstl::vector< uchar > mSurfaceMaterials;
    rstl::vector< CCollisionEdge > mEdges;
    rstl::vector< ushort > mSurfaceIndices;
    rstl::vector< ushort > x60_; // Additional serialized index array; role unresolved.
    rstl::vector< CVector3f > mVertices;
  };

  explicit COBBTree(CInputStream& in);
  COBBTree(const SIndexData& indexData, const CNode* root);
  ~COBBTree();

  const CNode* GetRoot() const { return mRoot; }
  CAABox CalculateLocalAABox() const;

  static rstl::auto_ptr< COBBTree > BuildOrientedBoundingBoxTree(const CVector3f& extent,
                                                                 const CVector3f& center);
  // Guessed name; enum values follow the four resource registrations.
  static void SetPrebuiltTree(COBBTree* tree, EPreBuiltTrees which);
  static COBBTree* GetPrebuiltTree(EPreBuiltTrees which);

private:
  // Guessed name for the array-view setup in both constructors.
  void BindIndexData();

  uint mMagic;
  uint mVersion;
  uint mMemsize;
  CSimpleAllocator mAllocator;
  SIndexData mIndexData;
  const CNode* mRoot;

  static COBBTree* sPrebuiltTrees[4];
};
CHECK_SIZEOF(COBBTree, 0xd0)
NESTED_CHECK_SIZEOF(COBBTree, CSimpleAllocator, 0xc)
NESTED_CHECK_SIZEOF(COBBTree, CLeafData, 0x10)
NESTED_CHECK_SIZEOF(COBBTree, CNode, 0x50)
NESTED_CHECK_SIZEOF(COBBTree, SIndexData, 0x80)

#endif // _COBBTREE
