#include "bsp/native_menu_prompt_construction.hpp"

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_front_end_frame_construction.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector_append.hpp"
#include "bsp/pool_pool17_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
Word address(const void* p) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
void* at(void* p, Word offset) noexcept { return pointer(address(p) + offset); }
Word word(const void* p, Word offset) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<std::uintptr_t>(address(p) + offset));
}
void put(void* p, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(static_cast<std::uintptr_t>(address(p) + offset)) = value;
}
void byte(void* p, Word offset, unsigned char value) noexcept {
    *reinterpret_cast<volatile unsigned char*>(static_cast<std::uintptr_t>(address(p) + offset)) = value;
}

struct LabelSpec {
    Word literal, length, fingerprint, resize, copy, append, pool_get, pool_return;
};
// Evidence/validation metadata, not an owning copy of original literal DATA.
constexpr LabelSpec labels[14] = {
    {0xced4f0,11,0x1561e2ca,0x5331fe,0x533219,0x53322d,0x533246,0x53324d},
    {0xced4e4,10,0x0cc53830,0x533262,0x53327d,0x533291,0x5332aa,0x5332b1},
    {0xced4d4,14,0xe6bad85d,0x5332c6,0x5332e1,0x5332f5,0x53330e,0x533315},
    {0xced4bc,22,0x7bc1a14c,0x53332a,0x533345,0x533359,0x533372,0x533379},
    {0xced4a8,18,0x32d57e4b,0x53338e,0x5333a9,0x5333bd,0x5333d6,0x5333dd},
    {0xced494,16,0x498006fb,0x5333f2,0x53340d,0x533421,0x53343a,0x533441},
    {0xceb3a8,12,0xfb1503ac,0x533456,0x533471,0x533485,0x53349e,0x5334a5},
    {0xced480,18,0x8c66ba09,0x5334ba,0x5334d5,0x5334ec,0x533505,0x53350c},
    {0xced46c,17,0xb2a4d965,0x533521,0x53353c,0x533550,0x533569,0x533570},
    {0xced454,21,0x0e861800,0x533585,0x5335a0,0x5335b4,0x5335cd,0x5335d4},
    {0xced434,29,0xc2eb19a9,0x5335e9,0x533604,0x533618,0x533631,0x533638},
    {0xced418,25,0x5c8910b2,0x53364d,0x533668,0x53367c,0x533695,0x53369c},
    {0xced400,23,0x1e90895e,0x5336b1,0x5336cc,0x5336e0,0x5336f9,0x533700},
    {0xced3ec,19,0xd4a66c29,0x533715,0x533730,0x533744,0x53375d,0x533764},
};

// Exact4322C0 effects: header+0 is deliberately untouched.
void construct_empty_vector(void* header) noexcept {
    put(header, 4, 0); put(header, 8, 0); put(header, 12, 0);
}

// Exact530E30 ordinary body, including its two separate null tests. The real
// allocator throws on exhausted/new-handler failure. No nullable success is
// manufactured; a null return would retain the native address4 store/fault arm.
void* buy_list_head() {
    void* const head = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x50, 0x50});
    if (address(head) != 0) put(head, 0, address(head));
    void* const previous = at(head, 4);
    if (address(previous) != 0) put(previous, 0, address(head));
    return head;
}

// Actual record530F90 cleanup over raw headers, not FrontEndPromptRecord.
// Native D95920 has state1->0 (+10), state0->-1 (+0) cleanup actions.
void destroy_record(void* record, NativeStringRawPoolContext& strings,
    NativeMenuPromptConstructionOperation& operation) {
    int state = 1;
    operation.cleanup_record = record;
    operation.cleanup_record_state = state;
    try {
        destroy_native_string_header_0041dd20(at(record, 0x38), strings);
        state = 0;
        operation.cleanup_record_state = state;
        destroy_native_string_header_0041dd20(at(record, 0x10), strings);
        state = -1;
        operation.cleanup_record_state = state;
        destroy_native_string_header_0041dd20(record, strings);
    } catch (...) {
        if (state == 1) {
            operation.cleanup_record_state = 0;
            destroy_native_string_header_0041dd20(at(record, 0x10), strings);
        }
        if (state >= 0) {
            operation.cleanup_record_state = -1;
            destroy_native_string_header_0041dd20(record, strings);
        }
        throw;
    }
}

// Exact531F80/532190 composition. Head reloads remain after prior callbacks.
void tidy_list(void* list, NativeStringRawPoolContext& strings,
    NativeMenuPromptConstructionOperation& operation) {
    operation.cleanup_list = list;
    void* head = pointer(word(list, 4));
    void* cursor = pointer(word(head, 0));
    put(head, 0, address(head));
    head = pointer(word(list, 4));
    put(head, 4, address(head));
    const bool nonempty = address(cursor) != word(list, 4);
    put(list, 8, 0);
    if (nonempty) for (;;) {
        void* const next = pointer(word(cursor, 0));
        operation.cleanup_list_node = cursor;
        operation.cleanup_list_next = next;
        destroy_record(at(cursor, 8), strings, operation);
        singleton_lifetime_free(cursor);
        const bool more = address(next) != word(list, 4);
        cursor = next;
        if (!more) break;
    }
    void* const current_head = pointer(word(list, 4));
    operation.cleanup_list_head = current_head;
    singleton_lifetime_free(current_head);
    put(list, 4, 0);
}

// Inlined real4F71A0, used only at the constructor's actual state0 unwind.
void destruct_screen(void* receiver, volatile Word* bank) noexcept {
    put(receiver, 0, 0x00ceae54);
    for (Word i = 0; i != 95; ++i)
        if (bank[i] == address(receiver)) bank[i] = 0;
}

void unwind_construction(NativeMenuPromptConstructionContext& context,
    NativeMenuPromptConstructionOperation& operation) {
    void* const receiver = operation.captured_receiver;
    while (operation.unwind_state >= 0) {
        const int state = operation.unwind_state;
        // Move to the native parent state before invoking each cleanup action.
        operation.unwind_state = state >= 7 ? 6 : state - 1;
        if (state >= 7) {
            operation.native_site = 0x00c6c1a9 + static_cast<Word>(state - 7) * 8;
            destroy_native_string_header_0041dd20(operation.temporary_header, context.strings);
        } else switch (state) {
        case 6:
            operation.native_site = 0x00c6c1a1;
            tidy_list(at(receiver, 0x278), context.strings, operation);
            break;
        case 5:
            operation.native_site = 0x00c6c193;
            operation.cleanup_vector = at(receiver, 0x268);
            destroy_native_string_header_vector_004324a0(operation.cleanup_vector, context.strings);
            break;
        case 4:
            operation.native_site = 0x00c6c184;
            operation.cleanup_records_remaining = 7;
            for (Word remaining = 7; remaining != 0; ) {
                --remaining;
                operation.cleanup_records_remaining = remaining;
                destroy_record(at(receiver, 0x60 + remaining * 0x48), context.strings, operation);
            }
            break;
        case 3:
            operation.native_site = 0x00c6c16e;
            operation.cleanup_vectors_remaining = 2;
            for (Word remaining = 2; remaining != 0; ) {
                --remaining;
                operation.cleanup_vectors_remaining = remaining;
                operation.cleanup_vector = at(receiver, 0x40 + remaining * 0x10);
                destroy_native_string_header_vector_004324a0(operation.cleanup_vector, context.strings);
            }
            break;
        case 2:
            operation.native_site = 0x00c6c159;
            context.actual_prompt_publication_00e18e6c = nullptr;
            put(receiver, 0x0c, 0x00ce3818);
            break;
        case 1:
            operation.native_site = 0x00c6c14e;
            put(receiver, 8, 0x00ceb0fc); // Whole4F8F10 is this store and RET.
            break;
        case 0:
            operation.native_site = 0x00c6c143;
            destruct_screen(receiver, context.actual_screen_words_00e18b60);
            break;
        }
    }
    operation.unwind_complete = true;
}
} // namespace

NativeMenuPromptConstructionLiterals borrow_native_menu_prompt_literals(
    const game::GameNativeReadOnlyData& data) {
    std::array<const char*, 14> borrowed;
    for (std::size_t i = 0; i != borrowed.size(); ++i) {
        const auto& spec = labels[i];
        const auto* const bytes = static_cast<const unsigned char*>(data.data_at(spec.literal, spec.length + 1));
        Word fingerprint = 2166136261u;
        for (Word j = 0; j <= spec.length; ++j) fingerprint = (fingerprint ^ bytes[j]) * 16777619u;
        if (reinterpret_cast<std::uintptr_t>(bytes) != spec.literal ||
            bytes[spec.length] != 0 || fingerprint != spec.fingerprint)
            throw std::logic_error("MenuPrompt literal differs from authenticated original PE data");
        borrowed[i] = reinterpret_cast<const char*>(bytes);
    }
    return NativeMenuPromptConstructionLiterals(data, borrowed);
}

__declspec(noinline) void* construct_native_menu_prompt_screen_00533120(void* receiver,
    NativeMenuPromptConstructionContext& context, NativeMenuPromptConstructionOperation& operation) {
    if (operation.phase != NativeMenuPromptConstructionOperation::Phase::fresh)
        throw std::logic_error("MenuPrompt construction requires a fresh retained operation");
    operation.captured_receiver = receiver;
    operation.phase = NativeMenuPromptConstructionOperation::Phase::running;
    try {
        operation.native_site = 0x00533148;
        construct_native_front_end_screen_004f7180(receiver);
        operation.unwind_state = 0;
        put(receiver, 8, 0x00ceb0fc);
        put(receiver, 0x0c, 0x00ced2d8);
        put(receiver, 0x10, 0x00ceb110);
        operation.unwind_state = 2;
        put(receiver, 0, 0x00ced3c4);
        put(receiver, 8, 0x00ced3b0);
        put(receiver, 0x0c, 0x00ced3ac);
        put(receiver, 0x10, 0x00ced38c);
        byte(receiver, 0x3c, 1);
        operation.native_site = 0x0053319e;
        for (Word i = 0; i != 2; ++i) {
            construct_empty_vector(at(receiver, 0x40 + i * 0x10));
            operation.completed_vectors = i + 1;
        }
        operation.unwind_state = 3;
        operation.native_site = 0x005331ba;
        for (Word i = 0; i != 7; ++i) {
            pool_pool17_cc13::menu_prompt_record_construct_00530f40(at(receiver, 0x60 + i * 0x48));
            operation.completed_records = i + 1;
        }
        construct_empty_vector(at(receiver, 0x268));
        operation.unwind_state = 5;
        operation.native_site = 0x005331de;
        void* const head = buy_list_head();
        put(receiver, 0x27c, address(head));
        put(receiver, 0x280, 0);
        operation.unwind_state = 6;
        for (Word i = 0; i != 14; ++i) {
            const auto& spec = labels[i];
            put(operation.temporary_header, 0, 0);
            put(operation.temporary_header, 4, 0);
            operation.native_site = spec.resize;
            resize_native_string_header_0041dd40(operation.temporary_header, context.strings, spec.length, true);
            void* const captured_data = pointer(word(operation.temporary_header, 4));
            operation.captured_temporary_data = captured_data;
            if (captured_data) {
                const Word count = word(operation.temporary_header, 0) + 1u;
                operation.native_site = spec.copy;
                copy_native_crt_bytes_00bf7680(captured_data, context.literals.label(i), count,
                    context.actual_feature_word_0109eea4);
            }
            operation.unwind_state = static_cast<int>(i) + 7;
            operation.native_site = spec.append;
            append_native_string_header_vector_00450540(at(receiver, i < 7 ? 0x40 : 0x50), 0,
                operation.temporary_header, context.strings);
            operation.unwind_state = 6;
            if (captured_data) {
                const Word size = word(operation.temporary_header, 0) + 1u;
                operation.native_site = spec.pool_get;
                auto* const pool = native_string_pool_get_or_create_00419cc0(
                    context.strings.actual_published_01090aa8,
                    context.strings.actual_manager_publication_01090aa0);
                operation.native_site = spec.pool_return;
                return_native_string_pool_00bd1510(pool, captured_data, size,
                    context.strings.actual_small_returns_disabled_01090aa4);
            }
        }
        put(receiver, 0x284, 0xffffffffu);
        byte(receiver, 0x288, 0);
        operation.phase = NativeMenuPromptConstructionOperation::Phase::complete;
        return receiver;
    } catch (...) {
        operation.phase = NativeMenuPromptConstructionOperation::Phase::failed;
        operation.failure_native_site = operation.native_site;
        operation.failure_unwind_state = operation.unwind_state;
        try { unwind_construction(context, operation); }
        catch (...) { operation.unwind_failed = true; throw; }
        throw;
    }
}
} // namespace bsp
