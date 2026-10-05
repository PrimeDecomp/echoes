#include "MetroidPrime/Player/CPlayerOptions.hpp"

#include "Kyoto/Streams/CBitStreamReader.hpp"
#include "Kyoto/Streams/CBitStreamWriter.hpp"

CPlayerOptions::CPlayerOptions() : mRumbleEnabled(true), mInvertYAxis(false) {}

CPlayerOptions::CPlayerOptions(CBitStreamReader& in)
: mRumbleEnabled(in.ReadPackedBool()), mInvertYAxis(in.ReadPackedBool()) {}

void CPlayerOptions::PutTo(CBitStreamWriter& out) const {
  out.WriteBits(mRumbleEnabled != 0, 1);
  out.WriteBits(mInvertYAxis != 0, 1);
}

void CPlayerOptions::SetRumbleEnabled(bool enabled) { mRumbleEnabled = enabled; }

void CPlayerOptions::SetInvertYAxis(bool value) { mInvertYAxis = value; }
