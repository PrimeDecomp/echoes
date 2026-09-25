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
  FourCC mLanguage;
  uint mOffset;
  uint mSize;
};

struct SStringNameLess {
  template < typename T >
  bool operator()(const T& a, const T& b) const {
    return strcmp(a.mName, b.mName) < 0;
  }
};

void CStringTable::SetLanguage(int language) { mCurrentLanguage = skLanguages[language]; }

CStringTable::CStringTable(CInputStream& in)
: mStringCount(0)
, mNameCount(0)
, mData(nullptr)
, mNames(nullptr)
, mStrings(nullptr)
, mReloadData(nullptr) {
  Load(in);
}

CStringTable::~CStringTable() {}

void CStringTable::Load(CInputStream& in) {
  in.ReadInt32();
  uint version = in.ReadInt32();
  int langCount = in.Get(TType< int >());
  mStringCount = in.Get(TType< int >());

  rstl::reserved_vector< SLanguageInfo, 7 > languages;
  for (int i = 0; i < langCount; ++i) {
    FourCC language = in.Get(TType< FourCC >());
    uint offset = in.Get(TType< uint >());
    uint size = version != 0 ? in.Get(TType< uint >()) : 0;
    SLanguageInfo info = {language, offset, size};
    languages.push_back(info);
  }

  uint offset = languages[0].mOffset;
  uint size = languages[0].mSize;
  for (int i = 0; i < langCount; ++i) {
    if (languages[i].mLanguage == mCurrentLanguage) {
      offset = languages[i].mOffset;
      size = languages[i].mSize;
      break;
    }
  }

  if (version != 0) {
    mNameCount = in.Get(TType< int >());
    uint namesSize = in.Get(TType< uint >());
    mData = rs_new uchar[namesSize + size];
    mNames = reinterpret_cast< SStringName* >(mData.get());
    mStrings = reinterpret_cast< const wchar_t** >(mData.get() + namesSize);
    in.ReadBytes(mNames, namesSize);
  }

  for (uint i = 0; i < offset; ++i) {
    in.ReadInt8();
  }

  if (version == 0) {
    size = in.Get(TType< uint >());
    mData = rs_new uchar[size];
    mStrings = reinterpret_cast< const wchar_t** >(mData.get());
  }
  in.ReadBytes(mStrings, size);

  if (version != 0) {
    SStringName* entry = mNames;
    for (int i = 0; i < mNameCount; ++i, ++entry) {
      entry->mName += reinterpret_cast< uint >(mNames);
    }
  }

  uint* entry = reinterpret_cast< uint* >(mStrings);
  for (int i = 0; i < mStringCount; ++i, ++entry) {
    *entry += reinterpret_cast< uint >(mStrings);
  }
}

const wchar_t* CStringTable::GetString(int idx) const {
  if (idx < 0 || idx >= mStringCount) {
    return skInvalidString;
  }
  return mStrings[idx];
}

int CStringTable::GetStringIndex(const char* name) const {
  SStringName* begin = mNames;
  if (mNameCount > 0 && *name != '\0') {
    SStringName* end = begin + mNameCount;
    SStringName key = {name};
    SStringNameLess cmp;
    SStringName* it = rstl::lower_bound(begin, end, key, cmp);
    if (it != end && strcmp(it->mName, name) == 0) {
      return it->mIndex;
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
