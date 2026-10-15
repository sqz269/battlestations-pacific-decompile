#pragma once

#include "bsp/native_scene_property_recursive_lifetime.hpp"

#include <cstdint>

namespace bsp {

// Source-only borrowed binding. Both process types have private constructors;
// use their canonical accessors. Complete explicit CC8A30 startup first.
// Capture the SAME actual E175B0 pool, raw key context and permanent property/
// query fallback cells. This context creates no owner, pool or callback.
class NativeScenePropertyInsertionContext final {
public:
    NativeScenePropertyInsertionContext(game::GameNativeStringProcess& actual_strings,
        game::GameNativePhysicalPoolProcess& actual_physical);
    NativeScenePropertyInsertionContext(const NativeScenePropertyInsertionContext&) = delete;
    NativeScenePropertyInsertionContext& operator=(const NativeScenePropertyInsertionContext&) = delete;

    NativeScenePropertyLifetimeContext& lifetimes() noexcept { return lifetimes_; }
    const char* property_empty() const noexcept { return property_empty_; }
    const char* query_empty() const noexcept { return query_empty_; }

private:
    NativeScenePropertyLifetimeContext lifetimes_;
    const char* const property_empty_;
    const char* const query_empty_;
};

// Two complete ordinary successful Native bodies, with new context-bearing
// C++ interfaces. Names are descriptive hypotheses, not recovered symbols.
//
// Require an actual raw61-produced 114h bag / its actual 108h map at bag+4.
// Keys are stable valid closed NUL-free ASCII, lengths match their buffers,
// null header data is allowed only when empty, and current CRT locale is C.
// Nodes are unique live14h slots of the SAME initialized E175B0/E188B4 domain;
// their first8B are canonical owning pooled key headers. All nonnull mapped
// records come from admitted real raw producers and same-current-CRT owning
// allocations. CE89D4 phase bits alone do not establish that provenance.
// Owning record/bag graphs are finite, unshared and acyclic. The incoming
// record/graph is disjoint from the replaced owner, query/key, node and map.
// Keep all owners, canonical processes, fallback cells, pool generation and
// allocator membership live. Finish records/keys/nodes before pool teardown.
// No concurrent mutation/trim, reentry, invalid memory, overflow, allocation
// failure or provider throw is admitted. The caller transfers the incoming
// mapped ownership to the map on successful publication.
//
// These APIs supply no Original ECX/RET8/register/FS/SEH transport, arbitrary
// profile dispatch, historical CRT, exception cleanup or game/class ABI.

// Whole [008F28F0,008F299C), 172 bytes / 65 instructions, including hit and
// miss. Original ECX=map, stack=query header + incoming record DWORD; RET8.
// A zero incoming word is allowed here. Lookup writes the bucket first.
// A nonnull old mapped record is scalar-deleted with flags1; ONLY after that
// return is mapped+8 cleared, then the incoming word is published. There is
// no same-pointer shortcut. Hit preserves key/chain/head/count and reloads
// count. Miss owns a real copied key, preserves allocator+10 and publishes
// mapped8, nextC, bucket head, then incremented count, in that order.
std::uint32_t insert_or_replace_native_scene_property_map_008f28f0(
    void* actual_map, const void* actual_query_header,
    std::uint32_t incoming_record_word, NativeScenePropertyInsertionContext& context);

// Whole [008F33F0,008F348B), 155 bytes / 46 instructions. Original ECX=bag,
// stack=nonnull NUL-terminated C string + actual nonnull38h record; RET8.
// The real constructor initializes an actual8B temporary key, then the full
// map operation runs, then the key is returned through the SAME raw pool.
// Test record+34 BEFORE publishing owner+30. For zero, increment bag+10C
// before storing its old value to record+34; otherwise preserve both values.
// Zero remains both the first ordinal and the unstamped sentinel. No max,
// deduplication, uniqueness fix, owner reset or second key destructor occurs.
// Return the actual bag. Exceptional cleanup/rollback is not invented.
void* insert_native_scene_property_bag_008f33f0(
    void* actual_bag, const char* actual_key, void* actual_incoming_record,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
