#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property lifetime requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
namespace game {
class GameNativeStringProcess;
class GameNativePhysicalPoolProcess;
}

// Source-only borrowed binding. Both input processes have private constructors;
// use their canonical accessors. Complete explicit CC8A30 startup BEFORE this
// constructor; its guarded getter captures that SAME actual E175B0 pool once.
// The raw string context contains the same real manager/pool/gate cells used
// for key publication. This owns nothing and accepts no raw pool or callback.
class NativeScenePropertyLifetimeContext final {
public:
    NativeScenePropertyLifetimeContext(game::GameNativeStringProcess& actual_strings,
        game::GameNativePhysicalPoolProcess& actual_physical);

    NativeStringRawPoolContext& keys() const noexcept { return keys_; }
    void* property_pool() const noexcept { return property_pool_; }

private:
    NativeStringRawPoolContext& keys_;
    void* const property_pool_;
};

// Six complete ordinary successful Native bodies, reconstructed together.
// Actual CE89D4 records bind directly to the complete record scalar below;
// actual D16504 children bind directly to the complete bag scalar. Phase
// DWORDs remain Original DATA, never callable Source vtables/numeric targets.
// Require genuine accepted raw61 bags, raw58/type6 or other admitted record
// producers and SAME-current-CRT owning allocations. A phase number alone is
// not provenance. Nodes are unique live14h slots of the SAME initialized
// E175B0 pool/E188B4 list; their first8B are real canonical pooled key headers.
// Keep allocator+10 membership and all key/root/buffer owners coherent. Owning
// graphs are finite, unshared and acyclic; child+110 is a non-owning backlink.
// No concurrent mutation/trim/reentry, aliasing, overflow, faults or allocation
// failure; allow enough stack for recursion. Finish payloads/keys/nodes BEFORE
// CD9260 pool teardown; canonical processes/cells/list outlive every borrower.
// Respect the live small-return gate: raw key return repeats the actual getter,
// large blocks free, disabled small returns skip the ring without extra free.
// Enabled small returns require the same live pool generation as allocation.
//
// These context-bearing C++ APIs do NOT supply Original ECX/RET4/register/FS/
// SEH transport, class ABI, historical CRT or exceptional-unwind parity. The
// scalar result is the original Win32 root BITS, captured before optional free.
// Evidence and exact native boundaries: CC12_PROPERTY_RECURSIVE_LIFETIME_SOURCE.

// Whole008F3F30..008F402E. Borrow actual108h map at bag+4; save next before
// mapped release, zero mapped only after return, release key then real slot,
// zero all64 heads after their chains, finally zero count. No interior free.
void clear_native_scene_property_record_map_008f3f30(void* actual_map,
    NativeScenePropertyLifetimeContext& context);

// Whole004E6730..004E674E. Always destroy; only flags&1 frees the root.
std::uint32_t scalar_delete_native_scene_property_record_004e6730(void* actual_record,
    std::uint32_t flags, NativeScenePropertyLifetimeContext& context);

// Whole008F0DE0..008F0DEB. Publish CE89D4 then complete per-type release.
void destroy_native_scene_property_record_008f0de0(void* actual_record,
    NativeScenePropertyLifetimeContext& context);

// Whole008F0640..008F06DE. Exact Native table: 2 owns+C,5 owns+1C,6 owns child,
// 8..11 own array+20; ALL others (including7) take only the common reset.
// Retain conditional+C clears and ordered tail stores +4,+C,+28,+8.
void release_native_scene_property_record_value_008f0640(void* actual_record,
    NativeScenePropertyLifetimeContext& context);

// Whole008F59E0..008F59FE. Always destroy; only flags&1 frees the root.
std::uint32_t scalar_delete_native_scene_property_bag_008f59e0(void* actual_bag,
    std::uint32_t flags, NativeScenePropertyLifetimeContext& context);

// Whole008F5410..008F5468. Outer D16504, full clear(map+4), inner D162C4,
// another unconditional full clear(same map). No root free/owner reset.
void destroy_native_scene_property_bag_008f5410(void* actual_bag,
    NativeScenePropertyLifetimeContext& context);
} // namespace bsp
