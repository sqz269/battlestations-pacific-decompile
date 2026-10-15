#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD vector range erase requires MSVC Win32.
#endif

namespace bsp {

// Complete 004685A0..004685F9. Native ECX control, five stacked DWORDs
// {output,first_owner,first_node,last_owner,last_node}, RET14h/EAX output.
// New ordinary Source binds these LIVE cells at offsets0/4/8/C/10 in the
// actual_parameters20 backing, with stable/disjoint binding metadata. Only
// reached cells/fields and stores need valid backing; no hidden Source/native
// private-frame or argument-home alias, concurrency or native fault ABI is added.
//
// Capture first_owner before the real returnable diagnostic; if captured owner
// is zero, skip last_owner's read. Do not require owner==control. Then capture
// CURRENT first_node then last_node. Equal nodes skip control.end access/move/
// publication. Otherwise compose the accepted signed-positive move kernel,
// including its zero/negative wrapped result, and publish end even then.
// Read CURRENT output only after that step; store captured node BEFORE captured
// pre-diagnostic owner, then return output. Source kernel reuse is not a native
// call claim; the original body's two calls target BF6713 and BF67A7 directly.
void* erase_native_scene_dword_range_004685a0(
    void* actual_control, const void* actual_parameters20);

// Actual current CRT/SDK reporting, returning and throwing behavior is composed.
// No new guard, callback, ownership, cleanup, rollback or original register/
// stack/private-frame/CRT/SEH/fault/instruction/binary ABI/game identity is supplied.

} // namespace bsp
