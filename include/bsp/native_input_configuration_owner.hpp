#pragma once

namespace bsp {

// Actual embedded configuration: game constructor4DDB90 passes game+3C at
// 4DDBE2; the next member starts at game+560. This borrowed span is524h, not a
// new allocation or a separately constructed settings singleton. The first
// 4C8h is the existing NativeLuaStateStorage AT OFFSET ZERO. Following it:
// bytes4C8..4CB, untouched DWORD4CC, five checked10h vector headers4D0/4E0/
// 4F0/500/510, and byte520; trailing bytes521..523 retain their preimage.
// Each vector has opaque+0 and pointer begin/end/capacity at+4/+8/+C. Outer
// 4D0 contains actual10h DWORD-vector rows; the other four contain DWORDs.

// 698680: ECX actual configuration, EAX original address, no stack args, RET.
// Calls concrete B66BD0 on the SAME base, then initializes only the fifteen
// pointer words and bytes4C8,4CB,4CA,4C9,520 in that order. Vector opaque words,
// DWORD4CC, Lua constructor preimages and tail padding stay untouched.
void* construct_native_input_configuration_00698680(void* actual_configuration) noexcept;

// 4DCEB0: ECX same embedded configuration, no stack args, tail B669A0; no
// semantic return. Release flat buffers510,500,4F0,4E0 and clear their triplets;
// destroy captured outer4D0 begin..end rows forward, reload/free its begin,
// clear its triplet, then close the actual Lua owner at offset ZERO.
// Uses the existing singleton CRT allocation/free domain and actual Lua API.
// This does NOT call698730 action reset, clear tail flags or free the embedded
// configuration. No private owner, registration, table copy or default script.
void destroy_native_input_configuration_004dceb0(void* actual_configuration);

// New C++ source ABI over valid original storage. Complete constructor and
// destruction schedules; the nested row cleanup is a source container adapter,
// not an original STL symbol reconstruction. Native FH3/SEH, hardware faults,
// asynchronous mutation, script configuration and application startup remain
// outside this interface. The caller controls the enclosing game's lifetime.
} // namespace bsp
