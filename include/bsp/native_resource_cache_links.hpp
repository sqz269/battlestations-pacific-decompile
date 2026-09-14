#pragma once

namespace bsp {
struct SingletonLifetimeCallbacks;

// Actual resource cache: tree head+4; head root+4; node1Ch links+0/+4/+8,
// key+C/+10, borrowed resource+14, color+18, nil+19. Actual iterator is the
// eight-byte {owner,node} pair. These helpers change links/iterator only.

// Complete B7CBD0..B7CC21[82] / B7D5F0..B7D63D[78]. Native ECX tree,
// stacked node, RET4; all return paths retain the captured pivot in EAX.
// Reload pivot's moved child after writing node's link, and capture current
// head only after publishing pivot.parent. Repair moved-child parent only
// when nonnil. Update root or current parent link, then pivot child/node parent.
// No recoloring, allocator/count change, payload access or sentinel validation.
void* rotate_native_resource_cache_right_00b7cbd0(void* actual_tree,
    void* actual_node) noexcept;
void* rotate_native_resource_cache_left_00b7d5f0(void* actual_tree,
    void* actual_node) noexcept;

// Complete B7CDF0..B7CE78[137]. ECX actual iterator, no stacked arguments,
// RET or tail JMP BF6713; no uniform result is specified. Null owner invokes
// returning CRT before reloading current node. End selects rightmost, checking
// the captured result after publication. Otherwise descend left/rightmost or
// climb current parents, publishing each ancestor before following parent+4.
// The final nil check rereads CURRENT iterator.node; both native tail-handler
// branches return if the callback returns, preserving its repairs.
void decrement_native_resource_cache_iterator_00b7cdf0(void* actual_iterator,
    const SingletonLifetimeCallbacks& invalid_parameters);

// Explicit-service MSVC Win32 APIs, not original stack/SEH ABI replacements.
// Borrow all actual storage; no resource references, ownership, insertion,
// manager publication or generic container substitution. Valid raw pointers
// remain caller requirements. Actual-storage aliases follow the stated read/
// store schedule; original stack/register spill aliases, concurrency, native
// exception equivalence and gameplay remain unproved. Similar registry/VFS
// helper addresses are not treated as aliases.
} // namespace bsp
