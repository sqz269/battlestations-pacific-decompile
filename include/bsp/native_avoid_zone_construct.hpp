#pragma once

#include <cstdint>

namespace bsp {

// Complete 0041CCD0..0041D0B1. Original ECX actual24h zone, stacked path/layer,
// RET8, EAX owner. Source EDX borrows the ADDRESS of an actual live world-pointer
// cell. It is not an Original argument, a world snapshot or a default binding.
// The cell's CURRENT value is loaded at the native late point, after all point
// and allocation work. Reached raw world fields and actual virtual slots need
// backing; the current world and object/vtable ownership belong to the caller.
//
// The actual incoming path/layer argument cells remain live through the Source
// frame. Layer's word is reused for signed pointcount and path's word is
// reloaded inside the loop. All load/store/alias, DWORD arithmetic, x87/spill,
// allocation/growth, clip, winding and three-ancestor schedules are retained.
// One current C++ guard, armed at the Native state0 store, reads the CURRENT
// saved-receiver cell and invokes actual00412DC0 on unwind: free array/zero0
// only. No record rollback, pending-allocation guard or normal release is added.
//
// Source uses actual raw children and current heap/CRT/C++ EH. Extra Source
// frame/local addresses and exception policy are not Native FH3/SEH/private
// CRT/frame/vtable/ABI or gameplay identity. No noexcept promise is made.
void* __fastcall construct_native_avoid_zone_0041ccd0(
    void* actual_zone, void* const volatile* actual_world_pointer_cell,
    void* actual_path, std::uint32_t layer_word);

} // namespace bsp
