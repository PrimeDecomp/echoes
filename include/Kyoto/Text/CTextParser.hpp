#ifndef _CTEXTPARSER
#define _CTEXTPARSER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "Kyoto/Text/CFontImageDef.hpp"
#include "rstl/pair.hpp"
#include "rstl/vector.hpp"
#include <rstl/string.hpp>
#include <types.h>

#include <Kyoto/Text/CTextColor.hpp>

class IObjectStore;
class CTextExecuteBuffer;
class CRasterFont;

class CTextParser {
public:
  CTextParser(IObjectStore& store);
  void ParseText(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                 const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  static CAssetId
  GetAssetIdFromString(const rstl::string& str,
                       const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  TToken< CRasterFont > GetFont(const wchar_t* str, int len);
  CFontImageDef GetImage(const wchar_t* str, int len,
                         const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  uint HandleUserTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len);
  void ParseTag(CTextExecuteBuffer& buffer, const wchar_t* str, int len,
                const rstl::vector< rstl::pair< CAssetId, CAssetId > >* textureMap);
  static bool BeginsWith(const wchar_t* str1, int len, const wchar_t* str2);
  static bool Equals(const wchar_t* str1, int len, const wchar_t* str2);
  static int ParseInt(const wchar_t* str, int len, bool allowSign);
  static int FromHex(wchar_t c);
  static int GetColorValue(const wchar_t* str);
  CTextColor ParseColor(const wchar_t* str, int len);

private:
  IObjectStore& mObjectStore;
};

CHECK_SIZEOF(CTextParser, 0x4)

#endif // _CTEXTPARSER
