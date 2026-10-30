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


// Additive qualified Source C++ cleanup; the normal-only APIs/metadata above
// retain their exact signatures/layouts/retain-and-rethrow behavior. These are
// passive caller-owned records, not Original frames or an execution journal.
enum class NativeLuaVariantCppCleanupAction : std::uint32_t {
    none, name_00c970d0, key_00c830a0
};
enum class NativeLuaVariantCppCleanupStage : std::uint32_t {
    unarmed, armed, normal_demoted, exceptional_demoted, action_entered,
    buffer_captured, length_captured, getter_entered, pool_captured,
    return_entered, completed
};
struct NativeLuaVariantCppCleanupIncoming {
    volatile bool observed{};
    volatile bool armed{};
    volatile std::uint32_t native_site{};
    volatile std::int32_t cleanup_state{-1};
    void* volatile saved_home{};
};
struct NativeLuaVariantCppCleanupActionRecord {
    volatile NativeLuaVariantCppCleanupAction kind{};
    volatile NativeLuaVariantCppCleanupStage stage{};
    volatile bool entered{};
    volatile bool completed{};
    volatile bool secondary_cpp_seen{};
    void* volatile saved_home{};
    void* volatile captured_buffer{};
    volatile std::uint32_t captured_length{};
    volatile std::uint32_t captured_size{};
    void* volatile captured_pool{};
    volatile std::uint32_t native_site{};
};
struct NativeLuaVariantCppCleanupRecord {
    volatile NativeLuaVariantRetirementBody body{};
    NativeLuaVariantCppCleanupIncoming incoming;
    NativeLuaVariantCppCleanupActionRecord action;
};
struct NativeLuaVariantCppCleanupBacking {
    NativeLuaVariantCppCleanupRecord* records;
    std::size_t capacity;
};

// Supply genuine typed/disjoint backing, sufficient for the ACTUAL active call
// chain including range/nested-map frames, for BOTH this sidecar span and the
// existing operation span. Metadata/context remain stable and independently
// live through all callbacks/failure disposition. No count/history-derived
// limit, allocation or automatic failed-state cleanup is supplied. Each lexical
// handler retains its own slot reference; failed active_depth is not its index.
// Sidecars record current scope/incoming/action captures independently; they do
// not drive cleanup, own exceptions, or authorize replay/retry/abandonment.
//
// All four internal cycle edges select the qualified mode. Genuine C++ escape
// from clear while destructor state0 invokes C970D0's actual41DD20 contract at
// saved V+Ch. Escape from value destruction while subtree state0 invokes
// C830A0's actual6EE020 contract at the rewritten key-home VALUE. Exceptional
// demotion precedes CURRENT action buffer/conditional late length capture.
// Normal demotion remains after the normal buffer capture. Actions use SAME
// actual AA8/AA0/AA4 raw pool services, add no object/node/map free/header clear,
// and do not resume the normal suffix. Metadata returned non-completion never
// triggers an action and is immediately propagated, outside the valid domain.
//
// If each actual action returns normally, bare rethrow preserves its incoming
// C++ exception. A cleanup getter can itself throw: record its distinct stage
// and propagate that escaping exception, without a new terminal/throw policy
// or promise to retain the incoming exception object. Existing noexcept/CRT/OS
// boundaries remain. /EHsc C++ handlers need not catch faults, SEH, longjmp or
// other nonlocal exits; compiler EH state over nonthrowing volatile operations
// does not establish Original fault/state epochs. No FH3/SEH/Original ABI or
// secondary-terminal equivalence, rollback or cleanup destructor is supplied.
// Saved K is the Source VALUE at the rewrite epoch; a mutable Native private
// home reload/replacement and arbitrary private-stack aliases are not supplied.
// Full host/vector/queue/payload/scalar/producer/adoption gates remain separate.
NativeLuaVariantRetirementResult clear_native_lua_variant_00886920_cpp_cleanup(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);
NativeLuaVariantRetirementResult destruct_native_lua_variant_00884af0_cpp_cleanup(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);
NativeLuaVariantRetirementResult
    destroy_native_lua_variant_subtree_006ee890_cpp_cleanup(
        void* owner, void* actual_node, NativeStringRawPoolContext& strings,
        NativeLuaVariantRetirementOperation& operation,
        const NativeLuaVariantCppCleanupBacking& cleanup);

// New ordinary Source result, not Native EAX/RET4 or a success-pointer policy.
// No default values or implicit conversion: a returned non-completion carries
// diagnostic receiver identity only. Completed bits can already be dangling.
struct NativeLuaVariantScalarDeleteResult {
    NativeLuaVariantRetirementResult completion;
    std::uintptr_t captured_receiver_bits;
};

// Complete 00884FA0..00884FBD (30 bytes/11 instructions), qualified Source API.
// Capture V; call the actual qualified884AF0 with the SAME context/metadata.
// Returned non-completion exits before flags/free. Genuine C++ escape has no
// ordinary returned result and skips flags/free; provider/secondary limits of
// the qualified child remain. No scalar-local action/frame/guard is added.
//
// actual_flags_home borrows a live DWORD HOME, never an entry-value snapshot.
// Only after child completion, read its CURRENT LOW BYTE once and test mask01.
// If set, actual singleton_lifetime_free receives captured V. Return captured
// pointer bits even after free; do not dereference them as a live receiver.
// V is genuine20-byte variant backing; a late free bit requires its exact root
// allocation base in the matching canonical malloc/free domain. Numerical
// profiles/flags/sizes authenticate neither receiver nor provider ownership.
// Flags HOME stays live/readable through its late read; supported raw-home/
// receiver aliases retain that order. Context, operation/sidecars and Source
// result metadata stay independently live/disjoint from mutable homes and raw
// payload through use/failure disposition. Legal aliases may change the flags
// HOME during the child; active metadata cannot be externally changed and
// reentry uses independent operation/backing.
//
// The reference/aggregate interface and compiler return storage supply no
// Original private-home alias, ECX/RET4, register/FH3/SEH/fault or secondary-
// terminal equivalence. No null/default/profile/retry policy, callable table,
// host/current-slot dispatch or activation is introduced.
NativeLuaVariantScalarDeleteResult
scalar_delete_native_lua_variant_00884fa0_cpp_cleanup(
    void* actual_variant,
    const volatile std::uint32_t& actual_flags_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup);

} // namespace bsp
