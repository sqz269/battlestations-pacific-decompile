#pragma once

namespace bsp {

// Complete 00467BA0..00467BCA. Native: one stacked DWORD at entry+4, RET4,
// incoming ECX/EDX unused; raw EAX allocation, or zero if the null tail returns.
// No caller establishes a nominal payload class or semantic return type.
//
// actual_parameter4 binds the actual LIVE parameter DWORD, with a stable backing
// address and valid reached accesses. Real current-CRT allocation of 0Ch bytes
// precedes every argument read. On nonnull: zero DWORD0, zero DWORD4, THEN read
// the current argument and store it at DWORD8. Parameter/node aliases retain
// that schedule. The entire null tail reads the current argument, writes it to
// literal address8 and returns zero if that write returns; no recovery is added.
//
// Binding metadata/Source argument homes and native private frames have no
// implicit aliases to the described cells. Current allocation is nonnull-or-
// throw; original CRT/global/exception/ABI/fault/game identity is not claimed.
void* allocate_native_raw_dword_list_node_00467ba0(const void* actual_parameter4);

} // namespace bsp
