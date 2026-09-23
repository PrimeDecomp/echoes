#include "rstl/StringExtras.hpp"
#include "rstl/math.hpp"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CFactoryMgr.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

#include <string.h>

extern "C" char* strchr(const char*, int);

int CStringExtras::IndexOfSubstring(const rstl::string& left, const rstl::string& right) {
  int rightSize = right.length();
  if (rightSize == 0) {
    return 0;
  }
  int leftSize = left.length();
  if (leftSize == 0) {
    return -1;
  }

  for (int i = 0; i < (leftSize - rightSize) + 1; ++i) {
    int j = 0;
    for (; j < rightSize; ++j) {
      const int index = i + j;
      const int leftUpper = ConvertToUpperCase(left.at(index));
      const int rightUpper = ConvertToUpperCase(right.at(j));
      if (rightUpper != leftUpper) {
        break;
      }
    }
    if (j == rightSize) {
      return i;
    }
  }
  return -1;
}

int CStringExtras::CompareCaseInsensitive(const rstl::string& left, const rstl::string& right) {
  int leftLength = left.length();
  int rightLength = right.length();
  int commonLength = rstl::min_val(leftLength, rightLength);

  for (int idx = 0; idx < commonLength; ++idx) {
    int rightUpper = ConvertToUpperCase(right.at(idx));
    if (ConvertToUpperCase(left.at(idx)) < rightUpper) {
      return -1;
    }

    rightUpper = ConvertToUpperCase(right.at(idx));
    if (ConvertToUpperCase(left.at(idx)) > rightUpper) {
      return 1;
    }
  }
  if (leftLength < rightLength) {
    return -1;
  } else if (leftLength > rightLength) {
    return 1;
  } else {
    return 0;
  }
}

rstl::string CStringExtras::ConvertToLowerCase(const rstl::string& str) {
  rstl::string ret(str);
  for (int i = 0; i < ret.length(); ++i) {
    const unsigned char* before = reinterpret_cast< const unsigned char* >(ret.data());
    ret.reserve(ret.length());
    char* after = const_cast< char* >(ret.data());
    after[i] = ConvertToLowerCase(before[i]);
  }
  return ret;
}

rstl::string CStringExtras::CreatePrefix(const rstl::string& str, int count) {
  const char* data = str.data();
  if (str.length() < count) {
    count = str.length();
  }
  return rstl::string(data, count);
}

char CStringExtras::ConvertToUpperCase(char c) {
  if (c >= CCast::ToChar('a') && c <= CCast::ToChar('z')) {
    return (c - CCast::ToChar(' ')) & 0xFF;
  }

  return c;
}

char CStringExtras::ConvertToLowerCase(char c) {
  if (c >= CCast::ToChar('A') && c <= CCast::ToChar('Z')) {
    return (c + CCast::ToChar(' ')) & 0xFF;
  }

  return c;
}

rstl::string CStringExtras::CreateFromInteger(int v) {
  int magnitude = v < 0 ? -v : v;
  int count = 0;
  signed char digits[24];
  if (v == 0) {
    return rstl::string_l("0");
  }

  while (magnitude > 0) {
    digits[count] = '0' + magnitude % 10;
    ++count;
    magnitude /= 10;
  }

  rstl::string ret;
  if (v < 0) {
    ret.append("-", -1);
  }

  for (int i = 0; i < count; ++i) {
    ret.append(1, digits[count - i - 1]);
  }

  return ret;
}

rstl::string CStringExtras::ConvertToANSI(const rstl::wstring& str) {
  rstl::string ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
    const char c = static_cast< char >(str.at(i));
    if (c != 0) {
      ret.append(1, c);
    }
  }
  return ret;
}

rstl::wstring CStringExtras::ConvertToUNICODE(const rstl::string& str) {
  if (str.length() == 0) {
    return rstl::wstring();
  }

  rstl::wstring ret;
  ret.reserve(str.size() + 1);

  for (int i = 0; i < static_cast< int >(str.size()); ++i) {
    ret.append(1, static_cast< wchar_t >(str.at(i)));
  }
  return ret;
}

rstl::string CStringExtras::ReadString(CInputStream& in) {
  rstl::string ret;
  int strLen = in.ReadInt32();
  ret.reserve(strLen);
  while (strLen > 0) {
    int len = strLen > 512u ? 512 : strLen;
    char tmp[512];
    in.ReadBytes(tmp, len);
    ret.append(tmp, len);
    strLen -= len;
  }

  return ret;
}

rstl::vector< rstl::string > CStringExtras::TokenizeString(const rstl::string& string,
                                                           const char* delims, int expectedSize) {
  rstl::vector< rstl::string > ret;
  if (expectedSize > 0) {
    ret.reserve(expectedSize);
  }

  const int size = string.length();
  int pos = 0;
  while (pos < size) {
    while (pos < size && strchr(delims, string[pos]) != nullptr) {
      ++pos;
    }
    if (pos == size) {
      break;
    }

    int end = pos + 1;
    while (end < size && strchr(delims, string[end]) == nullptr) {
      ++end;
    }
    ret.push_back_unsafe(string.substr(pos, end - pos));
    pos = end + 1;
  }

  return ret;
}

CFactoryFnReturn FSTLCFactory(const SObjectTag&, CInputStream& in, const CVParamTransfer&) {
  return rs_new rstl::vector< rstl::string >(in);
}
