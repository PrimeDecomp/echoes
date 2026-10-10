#include "MetroidPrime/TStateMachineState.hpp"

extern "C" void fn_80194054(int obj, float f) {
    *(float*)(obj + 0x34) = f;
}

// Provisional TU owner: GetDelay and SetDelay remain header-defined and emit through callers.
// The original template spelling and emitting TU are not established.

extern "C" float fn_8019404C(int obj) {
    return *(float*)(obj + 0x34);
}
