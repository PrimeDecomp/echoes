#include "Kyoto/Basics/CBasics.hpp"

#include <stdarg.h>
#include <stdio.h>

char* CBasics::Stringize(const char* fmt, ...) {
  static char stringizeBuffer[512];

  va_list args;
  va_start(args, fmt);
  vsnprintf(stringizeBuffer, sizeof(stringizeBuffer), fmt, args);
  va_end(args);

  return stringizeBuffer;
}
