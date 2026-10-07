#ifndef _TGAMETYPES
#define _TGAMETYPES

#include "rstl/construct.hpp"
#include "rstl/pair.hpp"
#include "types.h"

class CInputStream;
class COutputStream;

struct TAreaId;
struct TEditorId;
struct TUniqueId;

extern const TAreaId kInvalidAreaId;
extern const TEditorId kInvalidEditorId;
extern const TEditorId kUnkId;
extern const TUniqueId kInvalidUniqueId;
extern const float kDefaultGravityAccel; // Guessed name; 9.81 * 2.5 in TGameTypes .sdata2

struct TAreaId {
  int value;

  TAreaId() : value(-1) {}
  TAreaId(int value) : value(value) {}
  int Value() const { return value; }

  bool operator==(const TAreaId& other) const { return value == other.value; }
  bool operator!=(const TAreaId& other) const { return value != other.value; }
};
CHECK_SIZEOF(TAreaId, 0x4)

struct TEditorId {
  uint value;

  TEditorId(uint value) : value(value) {}
  TEditorId(CInputStream& in);
  // TODO
  uint Value() const { return value & 0x3FFFFFF; }
  uint Id() const { return value & 0xffff; }
  int AreaNum() const { return (value >> 16) & 0x3ff; }
  int LayerNum() const { return (value >> 26) & 0x3f; }

  void PutTo(COutputStream&) const;

  bool operator==(const TEditorId& other) const { return Value() == other.Value(); }
  bool operator!=(const TEditorId& other) const { return Value() != other.Value(); }
  bool operator<(const TEditorId& other) const { return Value() < other.Value(); }
};
CHECK_SIZEOF(TEditorId, 0x4)

struct TUniqueId {
  ushort value;

  explicit TUniqueId(ushort packed) : value(packed) {}
  TUniqueId(const ushort version, const ushort id) : value(id | (version << 10)) {}

  ushort Value() const { return value & 0x3FF; }
  ushort Version() const { return (value >> 10) & 0x3F; }

  bool operator==(const TUniqueId& other) const { return value == other.value; }
  bool operator!=(const TUniqueId& other) const { return value != other.value; }
  bool operator<(const TUniqueId& other) const { return value < other.value; }

private:
};
CHECK_SIZEOF(TUniqueId, 0x2)

namespace rstl {
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TUniqueId)
RSTL_DECLARE_TRIVIALLY_CONSTRUCTIBLE(TEditorId)

// The native pickup, seeker and area-damage containers use the conservative
// element policy for these combinations. Other ID pairs inherit the member traits.
template <>
struct is_trivially_destructible< pair< int, TEditorId > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< int, TEditorId > > {
  enum { value = false };
};

template <>
struct is_trivially_destructible< pair< TUniqueId, int > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< TUniqueId, int > > {
  enum { value = false };
};

template <>
struct is_trivially_destructible< pair< TUniqueId, float > > {
  enum { value = false };
};

template <>
struct use_assignment_for_construction< pair< TUniqueId, float > > {
  enum { value = false };
};
} // namespace rstl

// struct TGameScriptId {
//   TEditorId editorId;
//   bool b;
// };
// CHECK_SIZEOF(TGameScriptId, 0x8)

typedef ushort TSfxId;
struct TLayerId {
  explicit TLayerId(int value) : mValue(value) {}
  int Value() const { return mValue; }

private:
  int mValue;
};
CHECK_SIZEOF(TLayerId, 0x4)

const TSfxId InvalidSfxId = 0xFFFFu;

#define ALIGN_UP(x, a) (((x) + (a - 1)) & ~(a - 1))

#endif // _TGAMETYPES
