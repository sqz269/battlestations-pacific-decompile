#pragma once

#include <cstdint>

namespace bsp {

struct PointEffectReferenceArray;

// Complete 008670A0 / 008672A0 bodies over the same pointer/count/capacity
// header used by PointEffectInstanceStorage. Original ABI: ECX=header, one
// signed stack DWORD, RET4; reconstructed C++ functions have a new typed ABI.
// Entries are canonical companions borrowing the pointed owner's actual +04
// count and virtual+00 terminal action, never a second diagnostic reference.
// Reserve clamps its request to one, retains copies before releasing old
// slots, frees the current backing pointer, then publishes pointer/capacity.
void reserve_point_effect_entry_array_008670a0(
    PointEffectReferenceArray&, std::int32_t requested_capacity);

// Growth writes null slots. Shrink decrements the live count first, releases
// the captured entry, then clears that captured slot after terminal reentry.
// The loop reloads actual count/storage, and finally stores requested_count.
void resize_point_effect_entry_array_008672a0(
    PointEffectReferenceArray&, std::int32_t requested_count);

// MSVC Win32 only. Caller supplies valid spans/lifetimes and a nonnegative
// resize count. Reserve's four-byte allocation product wraps exactly as the
// native DWORD multiply; no overflow guard or corrupt-header recovery is added.
// The shared BF55BE/BF6989 host CRT boundary allocates/frees the real backing
// storage. Terminal actions are nonthrowing under the canonical companion
// contract. No destructor, implicit rollback, or physical native vtable overlay.

} // namespace bsp
