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

  void PutTo(CBitStreamWriter& out) const;
  void SetRumbleEnabled(bool enabled);
  void SetUnknownFlag(bool value); // Guessed name; the second flag's meaning remains unresolved.

  bool GetRumbleEnabled() const { return mRumbleEnabled; }
  bool GetUnknownFlag() const { return x1_; }

private:
  bool mRumbleEnabled;
  bool x1_;
};

CHECK_SIZEOF(CPlayerOptions, 2)

#endif // _CPLAYEROPTIONS
