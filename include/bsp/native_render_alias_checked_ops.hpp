#pragma once

#include "bsp/native_render_resource_record.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Actual eight-byte Win32 checked iterator. owner_00 is the identity/address
// of the real twelve-byte list at record+8, not a copied list header.
struct NativeRenderAliasIterator {
    void* owner_00;
    NativeRenderResourceAliasNode* node_04;
};

// Complete 004BE820..004BE849: ECX left, stack right address, RET4, AL bool.
// A returning invalid-parameter handler is followed by current node loads.
bool native_render_alias_iterator_not_equal_004be820(
    const volatile NativeRenderAliasIterator& left,
    const volatile NativeRenderAliasIterator& right,
    const SingletonLifetimeCallbacks&);

// Complete 004BECC0..004BECE6: ECX iterator, RET0, EAX original iterator.
// Publish previous before checking it against the current owner's sentinel.
volatile NativeRenderAliasIterator* native_render_alias_iterator_previous_004becc0(
    volatile NativeRenderAliasIterator&, const SingletonLifetimeCallbacks&);

// Complete 004B9FF0..004BA018: ECX iterator, RET0, EAX original iterator.
// After the sentinel check/handler, reload the node before loading its next.
volatile NativeRenderAliasIterator* native_render_alias_iterator_next_004b9ff0(
    volatile NativeRenderAliasIterator&, const SingletonLifetimeCallbacks&);

// Complete 004D0990..004D0A0B: ECX destination list; stack output address,
// input owner, input node; RET0Ch; EAX output. Input is captured by value.
// Unlink and release the actual node/string, then decrement the destination's
// current count. Result stores next then INPUT owner. Owner equality is not
// checked. Reaching the destination sentinel skips free/count decrement.
// Bind the existing real string pool and invalid-parameter callback domain.
volatile NativeRenderAliasIterator* erase_native_render_alias_node_004d0990(
    void* actual_destination_owner, volatile NativeRenderAliasIterator& output,
    NativeRenderAliasIterator input_by_value, SizedStoragePool& actual_string_pool,
    const SingletonLifetimeCallbacks&);
// Actual owning-pool overload: every string operation uses the current 419CC0
// publication/gate/lifetime binding. Same native algorithm and exception limits.
volatile NativeRenderAliasIterator* erase_native_render_alias_node_004d0990(
    void* actual_destination_owner, volatile NativeRenderAliasIterator& output,
    NativeRenderAliasIterator input_by_value, ActualNativeStringPoolStorage& actual_string_pool,
    const SingletonLifetimeCallbacks&);

// These require native Win32 storage and a callable invalid_parameter binding.
// Returning handlers may repair storage; no validation is strengthened, no
// replacement header is made, and unsafe unrepaired continuations remain unsafe.
// These C++ interfaces do not reproduce register/stack or native exception ABI.


// Raw publication variant uses the current source CRT invalid-parameter
// service, which may return. Pool getter failures propagate through real catches.
volatile NativeRenderAliasIterator* erase_native_render_alias_node_004d0990(
    void*, volatile NativeRenderAliasIterator&, NativeRenderAliasIterator, NativeStringRawPoolContext&);

}
