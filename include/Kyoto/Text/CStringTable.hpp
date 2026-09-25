#ifndef _CSTRINGTABLE
#define _CSTRINGTABLE

#include "types.h"

#include "Kyoto/SObjectTag.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/single_ptr.hpp"

class CInputStream;
class CResFactory;
class CDvdRequest;

class CStringTable {
public:
  struct SReloadData {
    uint mSize;
    rstl::single_ptr< CDvdRequest > mRequest;
    rstl::auto_ptr< uchar > mBuffer;

    SReloadData(CAssetId id, CResFactory& factory);
    ~SReloadData();
  };

  CStringTable(CInputStream& in);
  ~CStringTable();

  void Load(CInputStream& in);

  static void SetLanguage(int language);

  const wchar_t* GetString(int idx) const;
  const wchar_t* GetString(const char* name) const;
  int GetStringIndex(const char* name) const;
  int GetStringCount() const { return mStringCount; }

private:
  // Echoes STRG files can name strings; names are sorted for binary search.
  struct SStringName {
    const char* mName;
    int mIndex;
  };

  int mStringCount;
  int mNameCount;
  rstl::single_ptr< uchar > mData;
  SStringName* mNames;
  const wchar_t** mStrings;
  rstl::single_ptr< SReloadData > mReloadData;
};
CHECK_SIZEOF(CStringTable, 0x18)
NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;

#endif // _CSTRINGTABLE
