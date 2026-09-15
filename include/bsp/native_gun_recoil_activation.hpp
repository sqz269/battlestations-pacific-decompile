#pragma once

#include <cstdint>

namespace bsp {

struct NativeDamageableClassModelContext;
class NativeDamageableClassModelAcquired;

// Complete 155-byte00730CB0..00730D4A ordinary body. Native ECX is the
// actual device class; no stacked arguments; AL=1, plain RET. The exact
// SSE/x87 recoil loop leaves class fields untouched and retains its native
// unordered branches, floating status effects, control word and float stores.
// Then a nonzero DWORD+38 invokes the existing full00879590 with enemy=0.
// The callee may publish class+50; this wrapper adds no publication or latch.
//
// The caller owns the existing model context and acquired frame. If+38 is
// zero, the frame is not used. Otherwise it must be fresh and belong to that
// context; existing model failure/ownership rules apply without a replay.
std::uint8_t finalise_native_gun_class_00730cb0(void* actual_class,
    NativeDamageableClassModelContext&, NativeDamageableClassModelAcquired&);

// Explicit MSVC Win32 C++ service interface, not an original callable ABI or
// FH3/SEH bridge. Constants are exact local copies of the original positive
// zero cells00D7A218(float) and00D7A258(double), not live-address bindings.
// No loop limit, NaN sanitization, null-class fallback or exception conversion.
// Descriptive names are hypotheses; no executable/gameplay claim.

} // namespace bsp
