#include "Kyoto/Text/CTextParser.hpp"

#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Text/CRasterFont.hpp"
#include "Kyoto/Text/CTextExecuteBuffer.hpp"
#include "rstl/StringExtras.hpp"
#include "rstl/algorithm.hpp"

CTextParser::CTextParser(IObjectStore& store) : mObjectStore(store) {}

void CTextParser::ParseText(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                            const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  int begin = 0;
  int end = 0;
  while (str[end] && (len == -1 || end < len)) {
    if (str[end] == L'&') {
      if ((len == -1 || end + 1 < len) && str[end + 1] != L'&') {
        if (end > begin) {
          buffer.AddString(str + begin, end - begin);
        }
        ++end;
        begin = end;
        while ((len == -1 || end < len) && str[end] && str[end] != L';') {
          ++end;
        }
        ParseTag(buffer, str + begin, end - begin, textureMap);
        begin = end + 1;
      } else {
        buffer.AddString(str + begin, end + 1 - begin);
        end += 2;
        begin = end;
      }
    } else {
      ++end;
    }
  }
  if (end > begin) {
    buffer.AddString(str + begin, end - begin);
  }
}

CAssetId CTextParser::GetAssetIdFromString(
    const rstl::string& text, const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  const rstl::wstring str = CStringExtras::ConvertToUNICODE(text);
  const CAssetId id = (static_cast< uint >(GetColorValue(str.data())) << 24) |
                      (GetColorValue(str.data() + 2) << 16) | (GetColorValue(str.data() + 4) << 8) |
                      GetColorValue(str.data() + 6);
  if (textureMap) {
    typedef rstl::pair< CAssetId, CAssetId > AssetPair;
    rstl::vector< AssetPair >::const_iterator it = rstl::binary_find(
        textureMap->begin(), textureMap->end(), id,
        rstl::pair_sorter_finder< AssetPair, rstl::less< CAssetId > >(rstl::less< CAssetId >()));
    if (it != textureMap->end()) {
      return it->second;
    }
  }
  return id;
}

TToken< CRasterFont > CTextParser::GetFont(const wchar_t* str, int len) {
  const CAssetId id = (static_cast< uint >(GetColorValue(str)) << 24) |
                      (GetColorValue(str + 2) << 16) | (GetColorValue(str + 4) << 8) |
                      GetColorValue(str + 6);
  return mObjectStore.GetObj(SObjectTag('FONT', id));
}

CFontImageDef
CTextParser::GetImage(const wchar_t* str, int len,
                      const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  // TODO: parse static/animated image tags, crop factors and remapped texture IDs.
  return CFontImageDef(rstl::vector< TToken< CTexture > >(), 0.f, CVector2f(1.f, 1.f));
}

uint CTextParser::HandleUserTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len) {
  return 0;
}

void CTextParser::ParseTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                           const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap) {
  // TODO: dispatch font/image, color, spacing, justification and state-stack tags.
}

bool CTextParser::BeginsWith(const wchar_t* str, int len, const wchar_t* prefix) {
  int i = 0;
  for (; prefix[i] && i < len; ++i) {
    if (str[i] != prefix[i]) {
      return false;
    }
  }
  return prefix[i] == L'\0';
}

bool CTextParser::Equals(const wchar_t* str, int len, const wchar_t* other) {
  int i = 0;
  for (; other[i] && i < len; ++i) {
    if (str[i] != other[i]) {
      return false;
    }
  }
  return other[i] == L'\0';
}

int CTextParser::ParseInt(const wchar_t* str, int len, bool allowSign) {
  bool negative = false;
  int pos = 0;
  if (allowSign && len > 0 && str[0] == L'-') {
    negative = true;
    pos = 1;
  }

  int value = 0;
  for (; pos < len; ++pos) {
    value = value * 10 + str[pos] - L'0';
  }
  return negative ? -value : value;
}

int CTextParser::FromHex(wchar_t c) {
  if (c >= L'0' && c <= L'9') {
    return c - L'0';
  }
  if (c >= L'A' && c <= L'F') {
    return c - L'A' + 10;
  }
  if (c >= L'a' && c <= L'f') {
    return c - L'a' + 10;
  }
  return 0;
}

int CTextParser::GetColorValue(const wchar_t* str) {
  return (FromHex(str[0]) << 4) + FromHex(str[1]);
}

CTextColor CTextParser::ParseColor(const wchar_t* str, int len) {
  const int r = GetColorValue(str + 1);
  const int g = GetColorValue(str + 3);
  const int b = GetColorValue(str + 5);
  const int a = len == 9 ? GetColorValue(str + 7) : 255;
  return CTextColor(r, g, b, a);
}
