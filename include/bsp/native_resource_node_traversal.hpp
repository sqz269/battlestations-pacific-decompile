#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>
namespace bsp {
// Actual node24h: profile0/ref4/reader8/parentC/tag10,14/depth18/declared1C/
// remaining20. Child borrows parent and reader. Header transfers debit the
// current parent budget; child payload is debited by later detach/destruction.
// Original ECX node, stacked parent, EAX node, RET4.
void* construct_native_resource_child_00bea250(void* node, void* parent,
    NativeResourceStreamReadContext&);
// ECX parent handle, stacked output handle, EAX output, RET4. Allocate24h,
// reload parent handle after allocation, construct, then publish. Allocation
// failure publishes null; construction failure leaves output unchanged.
void* create_native_resource_child_00bea680(void* parent_handle, void* output,
    NativeResourceStreamReadContext&);
// ECX reader, stack budget, EAX DWORD, RET4. Current stream slot38, actual
// initially reader-address bits, wrapping debit after the scalar returns.
std::uint32_t read_native_resource_control_dword_00bf02a0(void* reader,
    std::uint32_t* budget, NativeResourceStreamReadContext&);
// ECX reader, stack low offset/unused pointer, forwarded EAX, RET8. Invoke
// current stream slot1C with (low,0,1); the second stacked pointer is unconsumed.
std::uint32_t seek_native_resource_relative_00bf03e0(void* reader,
    std::uint32_t low_offset, void* ignored, NativeResourceStreamReadContext&);
// ECX node handle, RET. Captures reader before scalar dispatch; stores returned
// control at that same reader+64 even when the node's reader changes in a call.
void read_native_resource_node_control_00be9a40(void* handle,
    NativeResourceStreamReadContext&);
// ECX node handle, RET. Debit parent declared payload; seek remaining if nonzero;
// clear remaining after success; pop CURRENT reader path and detach. No attached
// or double-skip guard. Seek failure leaves the prior parent debit observable.
void skip_native_resource_node_00be9c40(void* handle,
    NativeResourceStreamReadContext&);
// ECX handle, EAX0/1, RET. The handle itself must be valid. No attachment guard.
bool native_resource_node_has_remaining_00715bf0(const void* handle) noexcept;
// New C++ interfaces, not original binary ABI. Private stack/EH spill aliases,
// native FH3/SEH identity and hardware-fault behavior are outside this surface.
}
