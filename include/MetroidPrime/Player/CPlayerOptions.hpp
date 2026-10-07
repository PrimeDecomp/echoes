#ifndef _CPLAYEROPTIONS
#define _CPLAYEROPTIONS

#include "types.h"

class CBitStreamReader;
class CBitStreamWriter;

// Class/member spellings and qualifiers are reconstructed; no original export is established.
class CPlayerOptions {
public:
  CPlayerOptions();
  explicit CPlayerOptions(CBitStreamReader& in);
  CPlayerOptions(bool rumbleEnabled, bool invertYAxis)
  : mRumbleEnabled(rumbleEnabled), mInvertYAxis(invertYAxis) {}

  void PutTo(CBitStreamWriter& out) const;
  void SetRumbleEnabled(bool enabled);
  void SetInvertYAxis(bool value); // Target-derived: reverses multiplayer free-look pitch input.

  bool GetRumbleEnabled() const { return mRumbleEnabled; }
  bool GetInvertYAxis() const { return mInvertYAxis; }

private:
  bool mRumbleEnabled;
  bool mInvertYAxis;
};

CHECK_SIZEOF(CPlayerOptions, 2)

#endif // _CPLAYEROPTIONS
