#pragma once

#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant retirement requires MSVC Win32.
#endif

namespace bsp {

// New Source metadata/status, not an Original ABI or exception policy.
// A completed RETURN means that API entry finished all normal body effects.
// operation.result is the latest nested diagnostic, not root completion while
// active_depth is nonzero. The other returned
// values reject a Source backing/domain precondition and retain active frames.
// C++ exceptions escaping the real pool services propagate, without conversion
// to a status. Existing provider noexcept/CRT/manager policies stay unchanged.
enum class NativeLuaVariantRetirementResult : std::uint32_t {
    completed,
    metadata_backing_required,
    whole_range_domain_mismatch,
    cpp_exception_retained // diagnostic only; never returned instead of throw
};
enum class NativeLuaVariantRetirementBody : std::uint32_t {
    clear_00886920,
    destruct_00884af0,
    subtree_006ee890,
    whole_range_library_contract
};
enum class NativeLuaVariantRetirementPhase : std::uint32_t {
    unused, active, complete, source_stopped, cpp_exception
};

struct NativeLuaVariantRetirementPair {
    void* volatile owner{};
    void* volatile node{};
};

// Independent caller-owned diagnostic backing. No field here supplies a fake
// game object, global cell, allocator or callback. home is the VALUE delivered
// through the Native overwritten node-argument home, not its stack address.
// cleanup_state records the observed Native arm/demotion epoch; it does not
// install FH3/SEH or authorize automatic cleanup.
struct NativeLuaVariantRetirementFrame {
    volatile NativeLuaVariantRetirementBody body{};
    volatile NativeLuaVariantRetirementPhase phase{};
    volatile NativeLuaVariantRetirementResult result{};
    volatile std::uint32_t native_site{};
    volatile std::int32_t cleanup_state{-1};
    NativeStringRawPoolContext* volatile strings{};
    void* volatile receiver{};
    void* volatile owner{};
    void* volatile captured_payload{};
    void* volatile current_node{};
    void* volatile captured_right{};
    void* volatile captured_left{};
    void* volatile key_home{};
    void* volatile node_argument_home{};
    void* volatile name_home{};
    void* volatile captured_buffer{};
    void* volatile captured_pool{};
    volatile std::uint32_t captured_length{};
    volatile std::uint32_t captured_tag{};
    void* volatile captured_head{};
    void* volatile first_owner{};
    void* volatile first_node{};
    void* volatile last_owner{};
    void* volatile last_node{};
    void* volatile validation_head{};
    void* volatile validation_begin{};
    void* volatile captured_root{};
    void* volatile output_home{};
    NativeLuaVariantRetirementPair output;
};

// Supply live typed frame backing sufficient for the ACTUAL maximum active
// Source call chain, including nested maps. External callbacks that reenter
// retirement use an independent operation/span; they must not mutate this
// active metadata. No capacity is inferred
// from map.count, and no fixed maximum or allocation is introduced here.
// Backing/context/metadata must be disjoint from retired objects and remain
// independently live through all callbacks and any failure disposition.
// active_depth is zero for a root call; recursive calls share this operation.
// Completed frames can be reused normally. Failed/stopped frames are retained:
// caller resolves their actual resources before reusing that metadata. There
// is no destructor, address/history blacklist, rollback, retry or failed cleanup.
struct NativeLuaVariantRetirementOperation {
    NativeLuaVariantRetirementFrame* frames;
    std::size_t capacity;
    std::size_t active_depth{};
    std::size_t peak_depth{};
    NativeLuaVariantRetirementResult result{
        NativeLuaVariantRetirementResult::completed};
    bool cpp_exception_seen{};
};

// Complete normal 00886920..008869C7, explicit Source interface over genuine
// live 20-byte variant backing. Capture tag once; accept every DWORD tag.
// Only nonnull tag1/tag5 payloads are retired and then cleared. Other/null
// cases still store tag=-1 without an added payload clear.
//
// Tag1 descriptor8, tag5 map12 and node52 backing belong to the genuine
// singleton_lifetime_allocate/free Source domain. Buffers use the SAME actual
// pool01090AA8, manager01090AA0 and return-gate01090AA4 identities in strings,
// with the real qualified raw-manager/deletion domain kept live. Numeric
// profile words and matching sizes authenticate none of these requirements.
//
// Tag5 composes the reached qualified whole-range library operation. Its
// captured owner/begin/end must revalidate against current head without an
// intervening external callback/concurrent mutation. Rejection is Source-only
// non-completion, before traversal/free/reset, not a partial-erase substitute.
// Four current-head reloads/reset/output epochs follow real subtree return.
// No Source implementation of complete006EE960 or generic vendor erase/copy
// is claimed. map+0/count are never read to admit traversal.
NativeLuaVariantRetirementResult clear_native_lua_variant_00886920(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation);

// Complete normal 00884AF0..00884B54. Store numerical D0E6F4, arm state0,
// clear the real payload, then capture CURRENT name buffer+10h and demote
// before its late length+0Ch/pool return. No name clear or receiver free.
NativeLuaVariantRetirementResult destruct_native_lua_variant_00884af0(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation);

// Complete normal 006EE890..006EE925 / necessary library cleanup composition.
// owner is the Native opaque ECX identity, forwarded recursively, not read.
// node is genuine live raw backing: ANY nonzero nil BYTE31h stops. Recurse
// right; capture LEFT only after return; retire value atnode+1Ch, then CURRENT
// key buffer atnode+14h with conditional late length+10h; free captured node,
// then inspect captured LEFT's current nil byte. No null/cycle/default fix.
//
// Native name/key cleanup actions, argument-home rewrite and state demotions
// are recorded independently. Escaping C++ exceptions retain frames/propagate;
// no automatic Native FH3/SEH, rollback or equivalent action-frame delivery is
// supplied. SEH/nonlocal transfer need not pass through the C++ catch.
// No host/queue/payload/vector/scalar dispatch or producer adoption is added.
NativeLuaVariantRetirementResult destroy_native_lua_variant_subtree_006ee890(
    void* owner, void* actual_node, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation);

} // namespace bsp
