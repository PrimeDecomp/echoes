#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"

uint CCharAnimMemoryMetrics::sTotalSize = 0;

void CCharAnimMemoryMetrics::AddToTotalSize(uint size, EAnimSubSystem subSystem) {
  sTotalSize += size;
}

void CCharAnimMemoryMetrics::SubtractFromTotalSize(uint size, EAnimSubSystem subSystem) {
  sTotalSize = sTotalSize > size ? sTotalSize - size : 0;
}
