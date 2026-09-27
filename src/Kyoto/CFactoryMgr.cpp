#include "Kyoto/CFactoryMgr.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/Streams/CLZOInputStream.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"

#include <dolphin/os.h>

#include "ctype.h"
#include "string.h"

static const uint sTypeTable[] = {
    'CLSN', 'CSPP', 'CMDL', 'CSKR', 'ANIM', 'CINF', 'TXTR', 'PLTT', 'FONT', 'ANCS', 'ANMS', 'MADF',
    'MLVL', 'MREA', 'MAPW', 'MAPA', 'SAVW', 'SAVA', 'PART', 'WPSC', 'SWHC', 'DPSC', 'ELSC', 'CRSC',
    'SPSC', 'SRSC', 'AFSM', 'DCLN', 'AGSC', 'ATBL', 'CSNG', 'STRG', 'SCAN', 'PATH', 'DGRP', 'HMAP',
    'PTLA', 'STLC', 'EGMC', 'RULE', 'FSM2', 'CTWK', 'FRME', 'HINT', 'MAPU', 'DUMB',
};

CFactoryMgr::CFactoryMgr() {}

CFactoryMgr::~CFactoryMgr() {}

void CFactoryMgr::AddFactory(FourCC type, FFactoryFunc factory) {
  rstl::map< int, FFactoryFunc >::iterator it = mFactories.find(type);
  if (it != mFactories.end()) {
    return;
  }
  mFactories.insert(rstl::pair< int, FFactoryFunc >(type, factory));
}

void CFactoryMgr::AddFactory(FourCC type, FMemFactoryFunc factory) {
  rstl::map< int, FMemFactoryFunc >::iterator it = mMemFactories.find(type);
  if (it != mMemFactories.end()) {
    return;
  }
  mMemFactories.insert(rstl::pair< int, FMemFactoryFunc >(type, factory));
}

bool CFactoryMgr::CanMakeMemory(const SObjectTag& tag) const {
  return mMemFactories.find(tag.type) != mMemFactories.end();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObject(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& params) {
  rstl::map< int, FFactoryFunc >::iterator it = mFactories.find(tag.type);
  return it->second(tag, in, params).GetObjForTransfer();
}

rstl::auto_ptr< IObj > CFactoryMgr::MakeObjectFromMemory(const SObjectTag& tag,
                                                         const rstl::auto_ptr< uchar >& buffer,
                                                         int size, bool compressed,
                                                         const CVParamTransfer& params) {
  rstl::map< int, FMemFactoryFunc >::const_iterator memIt = mMemFactories.find(tag.type);
  if (memIt != mMemFactories.end()) {
    FMemFactoryFunc factory = memIt->second;
    if (compressed) {
      rstl::auto_ptr< CInputStream > in(
          rs_new CMemoryInStream(buffer.get(), size, CMemoryInStream::kOS_NotOwned));
      const uint length = in->ReadInt32();
      CLZOInputStream lzo(in, size - in->GetReadPosition(), length);
      rstl::auto_ptr< uchar > data(lzo.ReleaseBuffer());
      return factory(tag, data, length, params).GetObjForTransfer();
    }
    rstl::auto_ptr< uchar > data(buffer);
    if (!data.owner()) {
      data = rstl::auto_ptr< uchar >(
          static_cast< uchar* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen)));
      memcpy(data.get(), buffer.get(), size);
      DCStoreRange(data.get(), size);
    }
    return factory(tag, data, size, params).GetObjForTransfer();
  }

  FFactoryFunc factory = mFactories.find(tag.type)->second;
  if (compressed) {
    rstl::auto_ptr< CInputStream > in(
        rs_new CMemoryInStream(buffer.get(), size, CMemoryInStream::kOS_NotOwned));
    const uint length = in->ReadInt32();
    CLZOInputStream lzo(in, size - in->GetReadPosition(), length);
    return factory(tag, lzo, params).GetObjForTransfer();
  }
  CMemoryInStream in(buffer.get(), size, CMemoryInStream::kOS_NotOwned);
  return factory(tag, in, params).GetObjForTransfer();
}

uint CFactoryMgr::TypeIdxToFourCC(uint idx) { return sTypeTable[idx]; }

uint CFactoryMgr::FourCCToTypeIdx(uint fcc) {
  char* type = reinterpret_cast< char* >(&fcc);
  type[0] = toupper(type[0]);
  type[1] = toupper(type[1]);
  type[2] = toupper(type[2]);
  type[3] = toupper(type[3]);
  for (uint i = 0; i < sizeof(sTypeTable) / sizeof(sTypeTable[0]); ++i) {
    if (fcc == sTypeTable[i]) {
      return i;
    }
  }
  return -1;
}
