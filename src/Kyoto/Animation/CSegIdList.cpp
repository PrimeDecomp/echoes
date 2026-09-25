#include "Kyoto/Animation/CSegIdList.hpp"

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

CSegIdList::CSegIdList(CInputStream& in)
: mSegList(in) {
  CCharAnimMemoryMetrics::AddToTotalSize(mSegList.capacity(), CCharAnimMemoryMetrics::kASS_Two);  
}
