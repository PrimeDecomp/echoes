#include "MetroidPrime/Player/CPlayerOptions.hpp"

#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

CPlayerOptions::CPlayerOptions() : mRumbleEnabled(true), x1_(false) {}

CPlayerOptions::CPlayerOptions(CBitStreamReader& in)
: mRumbleEnabled(in.ReadPackedBool()), x1_(in.ReadPackedBool()) {}

void CPlayerOptions::PutTo(CBitStreamWriter& out) const {
  out.WriteBits(mRumbleEnabled != 0, 1);
  out.WriteBits(x1_ != 0, 1);
}

void CPlayerOptions::SetRumbleEnabled(bool enabled) { mRumbleEnabled = enabled; }

void CPlayerOptions::SetUnknownFlag(bool value) { x1_ = value; }
