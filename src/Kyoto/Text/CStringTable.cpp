#include "Kyoto/Text/CStringTable.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/reserved_vector.hpp"

#include <string.h>

static const wchar_t skInvalidString[] = L"Invalid";
static const FourCC skLanguages[] = {'ENGL', 'GERM', 'FREN', 'SPAN', 'ITAL', 'DUTC', 'JAPN'};
static FourCC mCurrentLanguage = skLanguages[0];

struct SLanguageInfo {
  FourCC x0_language;
  uint x4_offset;
  uint x8_size;
};

struct SStringNameLess {
  template < typename T >
  bool operator()(const T& a, const T& b) const {
    return strcmp(a.x0_name, b.x0_name) < 0;
  }
};

void CStringTable::SetLanguage(int language) { mCurrentLanguage = skLanguages[language]; }

CStringTable::CStringTable(CInputStream& in)
: x0_stringCount(0)
, x4_nameCount(0)
, x8_data(nullptr)
, xc_names(nullptr)
, x10_strings(nullptr)
, x14_reloadData(nullptr) {
  Load(in);
}

CStringTable::~CStringTable() {}

void CStringTable::Load(CInputStream& in) {
  in.ReadInt32();
  uint version = in.ReadInt32();
  int langCount = in.Get(TType< int >());
  x0_stringCount = in.Get(TType< int >());

  rstl::reserved_vector< SLanguageInfo, 7 > languages;
  for (int i = 0; i < langCount; ++i) {
    FourCC language = in.Get(TType< FourCC >());
    uint offset = in.Get(TType< uint >());
    uint size = version != 0 ? in.Get(TType< uint >()) : 0;
    SLanguageInfo info = {language, offset, size};
    languages.push_back(info);
  }

  uint offset = languages[0].x4_offset;
  uint size = languages[0].x8_size;
  for (int i = 0; i < langCount; ++i) {
    if (languages[i].x0_language == mCurrentLanguage) {
      offset = languages[i].x4_offset;
      size = languages[i].x8_size;
      break;
    }
  }

  if (version != 0) {
    x4_nameCount = in.Get(TType< int >());
    uint namesSize = in.Get(TType< uint >());
    x8_data = rs_new uchar[namesSize + size];
    xc_names = reinterpret_cast< SStringName* >(x8_data.get());
    x10_strings = reinterpret_cast< const wchar_t** >(x8_data.get() + namesSize);
    in.ReadBytes(xc_names, namesSize);
  }

  for (uint i = 0; i < offset; ++i) {
    in.ReadInt8();
  }

  if (version == 0) {
    size = in.Get(TType< uint >());
    x8_data = rs_new uchar[size];
    x10_strings = reinterpret_cast< const wchar_t** >(x8_data.get());
  }
  in.ReadBytes(x10_strings, size);

  if (version != 0) {
    SStringName* entry = xc_names;
    for (int i = 0; i < x4_nameCount; ++i, ++entry) {
      entry->x0_name += reinterpret_cast< uint >(xc_names);
    }
  }

  uint* entry = reinterpret_cast< uint* >(x10_strings);
  for (int i = 0; i < x0_stringCount; ++i, ++entry) {
    *entry += reinterpret_cast< uint >(x10_strings);
  }
}

const wchar_t* CStringTable::GetString(int idx) const {
  if (idx < 0 || idx >= x0_stringCount) {
    return skInvalidString;
  }
  return x10_strings[idx];
}

int CStringTable::GetStringIndex(const char* name) const {
  SStringName* begin = xc_names;
  if (x4_nameCount > 0 && *name != '\0') {
    SStringName* end = begin + x4_nameCount;
    SStringName key = {name};
    SStringNameLess cmp;
    SStringName* it = rstl::lower_bound(begin, end, key, cmp);
    if (it != end && strcmp(it->x0_name, name) == 0) {
      return it->x4_index;
    }
  }
  return -1;
}

const wchar_t* CStringTable::GetString(const char* name) const {
  return GetString(GetStringIndex(name));
}

CStringTable::SReloadData::~SReloadData() {}

CFactoryFnReturn FStringTableFactory(const SObjectTag& tag, CInputStream& in,
                                     const CVParamTransfer& xfer) {
  return rs_new CStringTable(in);
}
