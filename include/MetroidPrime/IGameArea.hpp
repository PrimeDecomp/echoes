#ifndef _IGAMEAREA
#define _IGAMEAREA

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "MetroidPrime/TGameTypes.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class IGameArea {
public:
  class Dock {
  public:
    struct SDockReference {
      TAreaId mArea;
      short mDock;
      short mLoadOther : 1;
      short x6_1_ : 1;

      SDockReference(TAreaId area, short dock)
      : mArea(area), mDock(dock), mLoadOther(false), x6_1_(false) {}
    };

    Dock(CInputStream& in, const CTransform4f& xf);
    bool IsReferenced() const;
    int GetReferenceCount() const;
    void SetReferenceCount(int count);
    TAreaId GetConnectedAreaId(int other) const;
    int GetOtherDockNumber(int other) const;
    void SetShouldLoadOther(int other, bool should);
    bool GetShouldLoadOther(int other) const;
    // TODO: identify the second per-reference flag.
    void fn_80056EE0(int other, bool value);
    bool fn_80056EA0(int other) const;

    const rstl::reserved_vector< CVector3f, 4 >& GetPlaneVertices() const { return mPlaneVertices; }

  private:
    int mReferenceCount;
    rstl::vector< SDockReference > mDockReferences;
    rstl::reserved_vector< CVector3f, 4 > mPlaneVertices;
    bool mIsReferenced;
  };

  virtual ~IGameArea();
  virtual const CTransform4f& IGetTM() const = 0;
  virtual CAssetId IGetStringTableAssetId() const = 0;
  virtual uint IGetNumAttachedAreas() const = 0;
  virtual TAreaId IGetAttachedAreaId(int index) const = 0;
  virtual bool IIsActive() const = 0;
  virtual CAssetId IGetAreaAssetId() const = 0;
  virtual int IGetAreaSaveId() const = 0;
  // Guessed name. Echoes replaces Prime's scripting-memory virtual with this string result.
  virtual rstl::string IGetInternalAreaName() const = 0;
};
CHECK_SIZEOF(IGameArea, 0x4)
NESTED_CHECK_SIZEOF(IGameArea, Dock, 0x4c)

#endif // _IGAMEAREA
