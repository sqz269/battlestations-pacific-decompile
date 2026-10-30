#include "bsp/native_lua_variant_retirement.hpp"

#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
namespace {

using Word = std::uint32_t;
using Result = NativeLuaVariantRetirementResult;
using Body = NativeLuaVariantRetirementBody;
using Phase = NativeLuaVariantRetirementPhase;
using Frame = NativeLuaVariantRetirementFrame;
using Operation = NativeLuaVariantRetirementOperation;
using CleanupAction = NativeLuaVariantCppCleanupAction;
using CleanupStage = NativeLuaVariantCppCleanupStage;
using CleanupRecord = NativeLuaVariantCppCleanupRecord;
using CleanupBacking = NativeLuaVariantCppCleanupBacking;

static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeLuaVariantRetirementPair) == 8);
static_assert(std::is_trivially_destructible_v<Frame>);
static_assert(std::is_trivially_destructible_v<Operation>);
static_assert(sizeof(Frame) == 120 && sizeof(Operation) == 24);
static_assert(std::is_trivially_destructible_v<CleanupRecord>);
static_assert(std::is_trivially_destructible_v<CleanupBacking>);
static_assert(sizeof(NativeLuaVariantCppCleanupIncoming) == 16);
static_assert(sizeof(NativeLuaVariantCppCleanupActionRecord) == 36);
static_assert(sizeof(CleanupRecord) == 56 && sizeof(CleanupBacking) == 8);
static_assert(offsetof(NativeLuaVariantRetirementPair, node) == 4);

void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
Word read_word(void* p, Word offset) noexcept {
    return *reinterpret_cast<volatile Word*>(at(p, offset));
}
void* read_pointer(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(read_word(p, offset));
}
std::uint8_t read_byte(void* p, Word offset) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(at(p, offset));
}
void write_word(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(at(p, offset)) = value;
}

// Metadata only. No heap allocation, callback, object-history policy or cleanup.
Frame* enter(Operation& o, NativeStringRawPoolContext& strings, Body body,
    void* receiver, void* owner) noexcept {
    if (o.frames == nullptr || o.active_depth >= o.capacity) {
        o.result = Result::metadata_backing_required;
        return nullptr;
    }
    if (o.active_depth == 0) {
        o.result = Result::completed;
        o.cpp_exception_seen = false;
    }
    Frame& f = o.frames[o.active_depth];
    f = Frame{};
    f.body = body;
    f.phase = Phase::active;
    f.result = Result::completed;
    f.strings = &strings;
    f.receiver = receiver;
    f.owner = owner;
    ++o.active_depth;
    if (o.peak_depth < o.active_depth) o.peak_depth = o.active_depth;
    return &f;
}
Result complete(Frame& f, Operation& o) noexcept {
    f.result = Result::completed;
    f.phase = Phase::complete;
    --o.active_depth;
    o.result = Result::completed;
    return Result::completed;
}
Result stop(Frame& f, Operation& o, Result result) noexcept {
    f.result = result;
    f.phase = Phase::source_stopped;
    o.result = result;
    return result; // active frames deliberately remain retained
}
void retain_cpp_failure(Operation& o, std::size_t first) noexcept {
    o.cpp_exception_seen = true;
    o.result = Result::cpp_exception_retained;
    for (std::size_t i = first; i < o.active_depth; ++i) {
        o.frames[i].phase = Phase::cpp_exception;
        o.frames[i].result = Result::cpp_exception_retained;
    }
}
void return_buffer(NativeStringRawPoolContext& strings, Frame& f,
    void* buffer, Word bytes, Word getter_site, Word return_site) {
    f.native_site = getter_site;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,
        strings.actual_manager_publication_01090aa0);
    f.captured_pool = pool;
    f.native_site = return_site;
    return_native_string_pool_00bd1510(pool, buffer, bytes,
        strings.actual_small_returns_disabled_01090aa4);
}


// Backing checks/record preparation occur only at entry, before native capture.
template<bool ApplyCppCleanup>
Frame* enter_mode(Operation& o, NativeStringRawPoolContext& strings, Body body,
    void* receiver, void* owner, const CleanupBacking* cleanup) noexcept {
    (void)cleanup;
    if constexpr (ApplyCppCleanup) {
        if (cleanup == nullptr || cleanup->records == nullptr ||
            o.active_depth >= cleanup->capacity) {
            o.result = Result::metadata_backing_required;
            return nullptr;
        }
    }
    [[maybe_unused]] const std::size_t slot = o.active_depth;
    Frame* const f = enter(o, strings, body, receiver, owner);
    if constexpr (ApplyCppCleanup) {
        if (f != nullptr) {
            CleanupRecord& r = cleanup->records[slot];
            r = CleanupRecord{};
            r.body = body;
            if (body == Body::destruct_00884af0)
                r.action.kind = CleanupAction::name_00c970d0;
            else if (body == Body::subtree_006ee890)
                r.action.kind = CleanupAction::key_00c830a0;
        }
    }
    return f;
}
template<bool ApplyCppCleanup>
CleanupRecord* record_for(const CleanupBacking* cleanup,
    std::size_t slot) noexcept {
    (void)cleanup;
    (void)slot;
    if constexpr (ApplyCppCleanup) return &cleanup->records[slot];
    else return nullptr;
}
template<bool ApplyCppCleanup>
void observe_incoming([[maybe_unused]] CleanupRecord* r,
    [[maybe_unused]] const Frame& f, [[maybe_unused]] bool armed,
    [[maybe_unused]] void* saved_home) noexcept {
    if constexpr (ApplyCppCleanup) {
        r->incoming.observed = true;
        r->incoming.armed = armed;
        r->incoming.native_site = f.native_site;
        r->incoming.cleanup_state = f.cleanup_state;
        r->incoming.saved_home = saved_home;
    }
}
void arm_action(CleanupRecord& r, void* saved_home) noexcept {
    r.action.stage = CleanupStage::armed;
    r.action.saved_home = saved_home;
}

// Necessary composition of the existing complete raw41DD20/6EE020 contracts.
// Records capture the ACTUAL action reads, not a pre-read around an opaque call.
// They never replace the original failure captures in Frame.
template<CleanupAction Action>
void run_action(CleanupRecord& r, NativeStringRawPoolContext& strings,
    void* saved_home) {
    static_assert(Action == CleanupAction::name_00c970d0 ||
        Action == CleanupAction::key_00c830a0);
    constexpr bool name = Action == CleanupAction::name_00c970d0;
    auto& a = r.action;
    a.entered = true;
    a.stage = CleanupStage::action_entered;
    a.native_site = name ? 0x0041dd20u : 0x006ee020u;
    void* const buffer = read_pointer(saved_home, name ? 4u : 8u);
    a.captured_buffer = buffer;
    a.stage = CleanupStage::buffer_captured;
    if (buffer != nullptr) {
        a.native_site = name ? 0x0041dd27u : 0x006ee027u;
        const Word length = read_word(saved_home, name ? 0u : 4u);
        const Word bytes = length + Word{1};
        a.captured_length = length;
        a.captured_size = bytes;
        a.stage = CleanupStage::length_captured;
        a.native_site = name ? 0x0041dd30u : 0x006ee031u;
        a.stage = CleanupStage::getter_entered;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        a.captured_pool = pool;
        a.stage = CleanupStage::pool_captured;
        a.native_site = name ? 0x0041dd37u : 0x006ee038u;
        a.stage = CleanupStage::return_entered;
        return_native_string_pool_00bd1510(pool, buffer, bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
    a.completed = true;
    a.stage = CleanupStage::completed;
}

template<bool ApplyCppCleanup>
Result clear_kernel(void*, NativeStringRawPoolContext&, Operation&,
    const CleanupBacking*);
template<bool ApplyCppCleanup>
Result destruct_kernel(void*, NativeStringRawPoolContext&, Operation&,
    const CleanupBacking*);
template<bool ApplyCppCleanup>
Result subtree_kernel(void*, void*, NativeStringRawPoolContext&, Operation&,
    const CleanupBacking*);

template<bool ApplyCppCleanup>
Result clear_kernel(
    void* actual_variant, NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking* cleanup) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter_mode<ApplyCppCleanup>(o, strings,
        Body::clear_00886920, actual_variant, nullptr, cleanup);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
    [[maybe_unused]] CleanupRecord* const record =
        record_for<ApplyCppCleanup>(cleanup, depth);
    [[maybe_unused]] Frame* range_failure = nullptr;
    [[maybe_unused]] CleanupRecord* range_record = nullptr;
    try {
        f.native_site = 0x00886927;
        const Word tag = read_word(actual_variant, 4);
        f.captured_tag = tag;
        if (tag == 5) {
            f.native_site = 0x00886938;
            void* const map = read_pointer(actual_variant, 8);
            f.captured_payload = map;
            if (map != nullptr) {
                // Prepare every fallible/backing-dependent metadata operation
                // BEFORE endpoint capture88693F. The interval below contains
                // only raw reads and pure retained-metadata stores/predicates.
                [[maybe_unused]] const std::size_t range_slot = o.active_depth;
                Frame* const prepared = enter_mode<ApplyCppCleanup>(o, strings,
                    Body::whole_range_library_contract, map, map, cleanup);
                if (prepared == nullptr) return stop(f, o, o.result);
                Frame& range = *prepared;
                if constexpr (ApplyCppCleanup) {
                    range_failure = &range;
                    range_record = record_for<ApplyCppCleanup>(cleanup, range_slot);
                }
                range.output_home = &f.output;

                f.native_site = 0x0088693f;
                void* const endpoint_head = read_pointer(map, 4);
                void* const first = read_pointer(endpoint_head, 0);
                range.captured_head = endpoint_head;
                range.first_owner = map;
                range.first_node = first;
                range.last_owner = map;
                range.last_node = endpoint_head;

                range.native_site = 0x006ee96f;
                void* const first_head = read_pointer(map, 4);
                void* const begin = read_pointer(first_head, 0);
                range.validation_head = first_head;
                range.validation_begin = begin;
                // Actual caller identities are map/map, already nonnull.
                // Their owner validations are discharged in this qualified
                // operation; begin/end are revalidated from current backing.
                if (first != begin) {
                    stop(range, o, Result::whole_range_domain_mismatch);
                    return stop(f, o, Result::whole_range_domain_mismatch);
                }
                range.native_site = 0x006ee98d;
                void* const end_head = read_pointer(map, 4);
                range.validation_head = end_head;
                if (endpoint_head != end_head) {
                    stop(range, o, Result::whole_range_domain_mismatch);
                    return stop(f, o, Result::whole_range_domain_mismatch);
                }
                range.native_site = 0x006ee9a1;
                void* const root_head = read_pointer(map, 4);
                void* const root = read_pointer(root_head, 4);
                range.captured_head = root_head;
                range.captured_root = root;

                range.native_site = 0x006ee9aa;
                const Result subtree = subtree_kernel<ApplyCppCleanup>(
                    map, root, strings, o, cleanup);
                if (subtree != Result::completed) {
                    stop(range, o, subtree);
                    return stop(f, o, subtree);
                }

                // Four distinct CURRENT-head reads after the callback.
                range.native_site = 0x006ee9af;
                void* const head1 = read_pointer(map, 4);
                range.captured_head = head1;
                write_word(head1, 4, reinterpret_cast<Word>(head1));
                range.native_site = 0x006ee9b5;
                void* const head2 = read_pointer(map, 4);
                range.captured_head = head2;
                write_word(map, 8, 0);
                write_word(head2, 0, reinterpret_cast<Word>(head2));
                range.native_site = 0x006ee9c1;
                void* const head3 = read_pointer(map, 4);
                range.captured_head = head3;
                write_word(head3, 8, reinterpret_cast<Word>(head3));
                range.native_site = 0x006ee9c7;
                void* const head4 = read_pointer(map, 4);
                range.captured_head = head4;
                void* const out_node = read_pointer(head4, 0);
                f.output.owner = map;
                f.output.node = out_node;
                complete(range, o);

                f.native_site = 0x00886954;
                void* const current_head = read_pointer(map, 4);
                f.captured_head = current_head;
                f.native_site = 0x00886958;
                singleton_lifetime_free(current_head);
                f.native_site = 0x0088695e;
                write_word(map, 4, 0);
                write_word(map, 8, 0);
                f.native_site = 0x0088696c;
                singleton_lifetime_free(map);
                f.native_site = 0x00886974;
                write_word(actual_variant, 8, 0);
                f.native_site = 0x0088697b;
                write_word(actual_variant, 4, 0xffffffffu);
                return complete(f, o);
            }
        } else if (tag == 1) {
            f.native_site = 0x00886988;
            void* const descriptor = read_pointer(actual_variant, 8);
            f.captured_payload = descriptor;
            if (descriptor != nullptr) {
                f.native_site = 0x0088698f;
                void* const buffer = read_pointer(descriptor, 4);
                f.captured_buffer = buffer;
                if (buffer != nullptr) {
                    f.native_site = 0x00886996;
                    const Word length = read_word(descriptor, 0);
                    f.captured_length = length;
                    return_buffer(strings, f, buffer, length + Word{1},
                        0x0088699f, 0x008869a6);
                }
                f.native_site = 0x008869ac;
                singleton_lifetime_free(descriptor);
                f.native_site = 0x008869b4;
                write_word(actual_variant, 8, 0);
            }
        }
        f.native_site = 0x008869bb;
        write_word(actual_variant, 4, 0xffffffffu);
        return complete(f, o);
    } catch (...) {
        retain_cpp_failure(o, depth);
        if constexpr (ApplyCppCleanup) {
            observe_incoming<ApplyCppCleanup>(record, f, false, nullptr);
            if (range_failure != nullptr && range_failure->phase != Phase::complete)
                observe_incoming<ApplyCppCleanup>(
                    range_record, *range_failure, false, nullptr);
        }
        throw;
    }
}

template<bool ApplyCppCleanup>
Result destruct_kernel(
    void* actual_variant, NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking* cleanup) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter_mode<ApplyCppCleanup>(o, strings,
        Body::destruct_00884af0, actual_variant, nullptr, cleanup);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
    [[maybe_unused]] CleanupRecord* const record =
        record_for<ApplyCppCleanup>(cleanup, depth);
    [[maybe_unused]] bool cleanup_armed = false;
    [[maybe_unused]] void* cleanup_home = nullptr;
    try {
        f.native_site = 0x00884b0d;
        write_word(actual_variant, 0, 0x00d0e6f4u);
        f.name_home = at(actual_variant, 0xc);
        f.cleanup_state = 0;
        if constexpr (ApplyCppCleanup) {
            cleanup_home = at(actual_variant, 0xc);
            cleanup_armed = true;
            arm_action(*record, cleanup_home);
        }
        f.native_site = 0x00884b1b;
        const Result cleared = clear_kernel<ApplyCppCleanup>(
            actual_variant, strings, o, cleanup);
        if (cleared != Result::completed) return stop(f, o, cleared);

        f.native_site = 0x00884b20;
        void* const buffer = read_pointer(actual_variant, 0x10);
        f.captured_buffer = buffer;
        f.cleanup_state = -1;
        if constexpr (ApplyCppCleanup) {
            cleanup_armed = false;
            record->action.stage = CleanupStage::normal_demoted;
        }
        if (buffer != nullptr) {
            f.native_site = 0x00884b2f;
            const Word length = read_word(actual_variant, 0xc);
            f.captured_length = length;
            return_buffer(strings, f, buffer, length + Word{1},
                0x00884b39, 0x00884b40);
        }
        return complete(f, o);
    } catch (...) {
        retain_cpp_failure(o, depth);
        if constexpr (ApplyCppCleanup) {
            observe_incoming<ApplyCppCleanup>(record, f, cleanup_armed, cleanup_home);
            if (cleanup_armed) {
                cleanup_armed = false;
                f.cleanup_state = -1;
                record->action.stage = CleanupStage::exceptional_demoted;
                try {
                    run_action<CleanupAction::name_00c970d0>(
                        *record, strings, cleanup_home);
                } catch (...) {
                    record->action.secondary_cpp_seen = true;
                    throw;
                }
            }
        }
        throw;
    }
}

template<bool ApplyCppCleanup>
Result subtree_kernel(
    void* owner, void* actual_node, NativeStringRawPoolContext& strings,
    Operation& o, const CleanupBacking* cleanup) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter_mode<ApplyCppCleanup>(o, strings,
        Body::subtree_006ee890, actual_node, owner, cleanup);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
    [[maybe_unused]] CleanupRecord* const record =
        record_for<ApplyCppCleanup>(cleanup, depth);
    [[maybe_unused]] bool cleanup_armed = false;
    [[maybe_unused]] void* cleanup_home = nullptr;
    try {
        void* node = actual_node;
        f.current_node = node;
        f.node_argument_home = node;
        f.native_site = 0x006ee8ab;
        std::uint8_t nil = read_byte(node, 0x31);
        while (nil == 0) {
            f.native_site = 0x006ee8b7;
            void* const right = read_pointer(node, 8);
            f.captured_right = right;
            f.native_site = 0x006ee8bd;
            const Result right_result =
                subtree_kernel<ApplyCppCleanup>(owner, right, strings, o, cleanup);
            if (right_result != Result::completed)
                return stop(f, o, right_result);

            f.native_site = 0x006ee8c2;
            void* const left = read_pointer(node, 0);
            f.captured_left = left;
            void* const key = at(node, 0xc);
            f.key_home = key;
            f.node_argument_home = key; // Native incoming-node home is rewritten.
            f.cleanup_state = 0;
            if constexpr (ApplyCppCleanup) {
                cleanup_home = key;
                cleanup_armed = true;
                arm_action(*record, cleanup_home);
            }
            f.native_site = 0x006ee8d6;
            const Result value = destruct_kernel<ApplyCppCleanup>(
                at(key, 0x10), strings, o, cleanup);
            if (value != Result::completed) return stop(f, o, value);

            f.native_site = 0x006ee8db;
            void* const buffer = read_pointer(key, 8);
            f.captured_buffer = buffer;
            f.cleanup_state = -1;
            if constexpr (ApplyCppCleanup) {
                cleanup_armed = false;
                record->action.stage = CleanupStage::normal_demoted;
            }
            if (buffer != nullptr) {
                f.native_site = 0x006ee8ea;
                const Word length = read_word(key, 4);
                f.captured_length = length;
                return_buffer(strings, f, buffer, length + Word{1},
                    0x006ee8f4, 0x006ee8fb);
            }
            f.native_site = 0x006ee901;
            singleton_lifetime_free(node);
            f.native_site = 0x006ee909;
            nil = read_byte(left, 0x31);
            node = left;
            f.current_node = node;
        }
        return complete(f, o);
    } catch (...) {
        retain_cpp_failure(o, depth);
        if constexpr (ApplyCppCleanup) {
            observe_incoming<ApplyCppCleanup>(record, f, cleanup_armed, cleanup_home);
            if (cleanup_armed) {
                cleanup_armed = false;
                f.cleanup_state = -1;
                record->action.stage = CleanupStage::exceptional_demoted;
                try {
                    run_action<CleanupAction::key_00c830a0>(
                        *record, strings, cleanup_home);
                } catch (...) {
                    record->action.secondary_cpp_seen = true;
                    throw;
                }
            }
        }
        throw;
    }
}

} // namespace

Result clear_native_lua_variant_00886920(void* actual_variant,
    NativeStringRawPoolContext& strings, Operation& o) {
    return clear_kernel<false>(actual_variant, strings, o, nullptr);
}
Result destruct_native_lua_variant_00884af0(void* actual_variant,
    NativeStringRawPoolContext& strings, Operation& o) {
    return destruct_kernel<false>(actual_variant, strings, o, nullptr);
}
Result destroy_native_lua_variant_subtree_006ee890(void* owner, void* actual_node,
    NativeStringRawPoolContext& strings, Operation& o) {
    return subtree_kernel<false>(owner, actual_node, strings, o, nullptr);
}
Result clear_native_lua_variant_00886920_cpp_cleanup(void* actual_variant,
    NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking& cleanup) {
    return clear_kernel<true>(actual_variant, strings, o, &cleanup);
}
Result destruct_native_lua_variant_00884af0_cpp_cleanup(void* actual_variant,
    NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking& cleanup) {
    return destruct_kernel<true>(actual_variant, strings, o, &cleanup);
}
Result destroy_native_lua_variant_subtree_006ee890_cpp_cleanup(void* owner,
    void* actual_node, NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking& cleanup) {
    return subtree_kernel<true>(owner, actual_node, strings, o, &cleanup);
}

NativeLuaVariantScalarDeleteResult
scalar_delete_native_lua_variant_00884fa0_cpp_cleanup(void* actual_variant,
    const volatile std::uint32_t& actual_flags_home,
    NativeStringRawPoolContext& strings, Operation& o,
    const CleanupBacking& cleanup) {
    static_assert(std::is_aggregate_v<NativeLuaVariantScalarDeleteResult>);
    static_assert(std::is_trivially_copyable_v<NativeLuaVariantScalarDeleteResult>);
    static_assert(sizeof(NativeLuaVariantScalarDeleteResult) == 8);
    static_assert(offsetof(NativeLuaVariantScalarDeleteResult,
        captured_receiver_bits) == 4);
    void* const captured_variant = actual_variant;
    const auto captured_bits = reinterpret_cast<std::uintptr_t>(captured_variant);
    const Result child = destruct_native_lua_variant_00884af0_cpp_cleanup(
        captured_variant, strings, o, cleanup);
    if (child != Result::completed) return {child, captured_bits};

    const auto* const flags_byte =
        reinterpret_cast<const volatile unsigned char*>(&actual_flags_home);
    if ((*flags_byte & 1u) != 0) singleton_lifetime_free(captured_variant);
    return {child, captured_bits};
}

} // namespace bsp
