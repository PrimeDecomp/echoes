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
    uint x0_size;
    rstl::single_ptr< CDvdRequest > x4_request;
    rstl::auto_ptr< uchar > x8_buffer;

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
  int GetStringCount() const { return x0_stringCount; }

private:
  // Echoes STRG files can name strings; names are sorted for binary search.
  struct SStringName {
    const char* x0_name;
    int x4_index;
  };

  int x0_stringCount;
  int x4_nameCount;
  rstl::single_ptr< uchar > x8_data;
  SStringName* xc_names;
  const wchar_t** x10_strings;
  rstl::single_ptr< SReloadData > x14_reloadData;
};
CHECK_SIZEOF(CStringTable, 0x18)
NESTED_CHECK_SIZEOF(CStringTable, SReloadData, 0x10)

extern CStringTable* gpStringTable;

#endif // _CSTRINGTABLE
