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

static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
static_assert(sizeof(NativeLuaVariantRetirementPair) == 8);
static_assert(std::is_trivially_destructible_v<Frame>);
static_assert(std::is_trivially_destructible_v<Operation>);
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

} // namespace

Result clear_native_lua_variant_00886920(
    void* actual_variant, NativeStringRawPoolContext& strings, Operation& o) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter(o, strings, Body::clear_00886920,
        actual_variant, nullptr);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
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
                Frame* const prepared = enter(o, strings,
                    Body::whole_range_library_contract, map, map);
                if (prepared == nullptr) return stop(f, o, o.result);
                Frame& range = *prepared;
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
                const Result subtree = destroy_native_lua_variant_subtree_006ee890(
                    map, root, strings, o);
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
        throw;
    }
}

Result destruct_native_lua_variant_00884af0(
    void* actual_variant, NativeStringRawPoolContext& strings, Operation& o) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter(o, strings, Body::destruct_00884af0,
        actual_variant, nullptr);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
    try {
        f.native_site = 0x00884b0d;
        write_word(actual_variant, 0, 0x00d0e6f4u);
        f.name_home = at(actual_variant, 0xc);
        f.cleanup_state = 0;
        f.native_site = 0x00884b1b;
        const Result cleared = clear_native_lua_variant_00886920(
            actual_variant, strings, o);
        if (cleared != Result::completed) return stop(f, o, cleared);

        f.native_site = 0x00884b20;
        void* const buffer = read_pointer(actual_variant, 0x10);
        f.captured_buffer = buffer;
        f.cleanup_state = -1;
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
        throw;
    }
}

Result destroy_native_lua_variant_subtree_006ee890(
    void* owner, void* actual_node, NativeStringRawPoolContext& strings,
    Operation& o) {
    const std::size_t depth = o.active_depth;
    Frame* const entered = enter(o, strings, Body::subtree_006ee890,
        actual_node, owner);
    if (entered == nullptr) return o.result;
    Frame& f = *entered;
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
                destroy_native_lua_variant_subtree_006ee890(owner, right, strings, o);
            if (right_result != Result::completed)
                return stop(f, o, right_result);

            f.native_site = 0x006ee8c2;
            void* const left = read_pointer(node, 0);
            f.captured_left = left;
            void* const key = at(node, 0xc);
            f.key_home = key;
            f.node_argument_home = key; // Native incoming-node home is rewritten.
            f.cleanup_state = 0;
            f.native_site = 0x006ee8d6;
            const Result value =
                destruct_native_lua_variant_00884af0(at(key, 0x10), strings, o);
            if (value != Result::completed) return stop(f, o, value);

            f.native_site = 0x006ee8db;
            void* const buffer = read_pointer(key, 8);
            f.captured_buffer = buffer;
            f.cleanup_state = -1;
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
        throw;
    }
}

} // namespace bsp
