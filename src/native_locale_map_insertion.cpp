#include "bsp/native_locale_map_insertion.hpp"

#include "bsp/game_native_locale_pool.hpp"
#include "bsp/native_locale_map_node_recycling.hpp"

#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale map insertion requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::uint8_t) == 1);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

Word word(Word base, Word byte_offset = 0u) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}

void put_word(Word base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

void increment_word(Word base, Word byte_offset) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        add dword ptr [eax + edx], 1
    }
}

std::uint8_t read_byte(Word base, Word byte_offset) noexcept {
    Word result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        movzx eax, byte ptr [eax + edx]
        mov result, eax
    }
    return static_cast<std::uint8_t>(result);
}

// Actual F460 body only: no metadata permission/fence/callback framework.
// Its ordinary-C++ exceptions alone select the enclosing constructor cleanup.
void* copy_construct_key_00a9f460(void* actual_node, const void* actual_key,
    NativeStringRawPoolContext& strings) {
    const Word destination = address(actual_node);
    const Word source = address(actual_key);
    const bool self = destination == source; // native CMP BEFORE zero stores
    put_word(destination, 0u, 0u);
    put_word(destination, 4u, 0u);
    if (!self) {
        const Word requested = word(source);
        resize_native_string_header_0041dd40(actual_node, strings, requested, true);
        if (word(source) != 0u) { // CURRENT source length AFTER resize
            const Word count = word(destination);
            const Word source_data = word(source, 4u);
            const Word destination_data = word(destination, 4u);
            // Native BF7680 admits overlap. Keep the established raw-string
            // Source policy of omitting a zero-byte call (not its operands).
            if (count != 0u)
                std::memmove(pointer(destination_data), pointer(source_data), count);
        }
    }
    put_word(destination, 8u, 0u);
    put_word(destination, 0x0cu, 0u);
    return actual_node;
}
} // namespace

NativeLocaleMapInsertionFrame::NativeLocaleMapInsertionFrame(
    const NativeLocaleMapInsertionContext& context) noexcept
    : context_(context), allocation_(context.pool_owner) {
    progress_.context = &context;
    progress_.map = context.actual_map4004;
    progress_.key = context.actual_key8;
}

bool NativeLocaleMapInsertionFrame::has_normal_result() const noexcept {
    return progress_.phase == NativeLocaleMapInsertionPhase::returned &&
        progress_.normal_returned && !progress_.failed && !failure_;
}

void* NativeLocaleMapInsertionFrame::normal_result() const noexcept {
    return has_normal_result() ? progress_.value_header : nullptr;
}

[[noreturn]] void NativeLocaleMapInsertionFrame::reject(const char* message) {
    progress_.failed = true;
    progress_.phase = NativeLocaleMapInsertionPhase::failed;
    throw std::logic_error(message);
}

void NativeLocaleMapInsertionFrame::require_entered() {
    if (progress_.phase != NativeLocaleMapInsertionPhase::entered ||
        progress_.failed || failure_)
        reject("locale map insertion cannot resume after caught reentry/failure");
}

void NativeLocaleMapInsertionFrame::fail(std::exception_ptr failure) noexcept {
    progress_.failed = true;
    progress_.phase = NativeLocaleMapInsertionPhase::failed;
    if (!failure_) failure_ = failure;
}

void* NativeLocaleMapInsertionFrame::finish(void* value) {
    require_entered();
    progress_.value_header = value;
    progress_.normal_returned = true;
    progress_.phase = NativeLocaleMapInsertionPhase::returned;
    return value;
}

void* insert_or_get_native_locale_value_00a9fc30(
    const NativeLocaleMapInsertionContext& context, NativeLocaleMapInsertionFrame& frame) {
    auto& p = frame.progress_;
    try {
        if (&context != &frame.context_ || p.phase != NativeLocaleMapInsertionPhase::prepared ||
            p.attempted || p.failed || frame.failure_)
            frame.reject("locale map insertion requires its exact fresh prepared context/frame");
        p.attempted = true;
        p.phase = NativeLocaleMapInsertionPhase::entered;
        if (context.actual_map4004 == nullptr || context.actual_key8 == nullptr)
            frame.reject("locale map insertion requires actual live map/key backing");

        const Word map = address(context.actual_map4004);
        const Word key = address(context.actual_key8);
        const Word initial_length = word(key); // captured ONCE for hash
        Word hash = 0u;
        if (initial_length != 0u) {
            const Word initial_data = word(key, 4u); // captured ONCE for hash
            for (Word i = 0u; i < initial_length; ++i) {
                Word folded = read_byte(initial_data, i);
                if (static_cast<std::uint8_t>(folded - 0x41u) <= 0x19u)
                    folded += 0x20u;
                const Word signed_byte = (folded & 0x80u) != 0u
                    ? folded | 0xffffff00u : folded;
                hash = hash * 0x83u + signed_byte;
            }
        }
        const Word bucket = hash & 0xfffu;
        const Word bucket_offset = bucket * 4u;
        p.bucket_index = bucket;
        p.hash_completed = true;
        Word node = word(map, bucket_offset);
        while (node != 0u) {
            p.current_node = pointer(node);
            const Word node_length = word(node);
            const Word key_length = word(key);
            if (node_length == key_length) {
                bool equal;
                if (node_length == 0u) {
                    equal = key_length == 0u;
                } else if (key_length == 0u) {
                    equal = false;
                } else {
                    const Word key_data = word(key, 4u); // right BEFORE left
                    const Word node_data = word(node, 4u);
                    const int comparison = _stricmp(
                        static_cast<const char*>(pointer(node_data)),
                        static_cast<const char*>(pointer(key_data)));
                    frame.require_entered(); // Source fence, not F460 cleanup
                    equal = comparison == 0;
                }
                if (equal) {
                    p.duplicate_found = true;
                    return frame.finish(pointer(node + 8u));
                }
            }
            node = word(node, 0x14u); // CURRENT next AFTER comparison
        }

        increment_word(map, 0x4000u); // BEFORE actual allocation (no rollback)
        p.count_incremented = true;
        p.pool_borrow_entered = true;
        // Source admission observation, not a diagnostic-derived authority.
        // Keep duplicates getter-free; a rejection here retains count++.
        void* const actual_pool = context.pool_owner.pool_storage_00e1768c();
        p.actual_pool = actual_pool;
        p.pool_borrow_returned = true;
        frame.require_entered();
        p.allocation_entered = true;
        void* const allocated = allocate_native_locale_node_00a9f0b0(
            context.pool_owner, frame.allocation_);
        p.allocated_slot = allocated; // BEFORE any subsequent effect/fence
        p.allocation_returned = true;
        frame.require_entered(); // OUTSIDE selected constructor boundary
        if (!frame.allocation_.has_normal_result() ||
            frame.allocation_.normal_result() != allocated ||
            frame.allocation_.progress().pool != actual_pool)
            frame.reject("locale map insertion requires the exact allocator normal receipt");

        void* constructed = nullptr;
        if (allocated != nullptr) {
            p.constructor_entered = true;
            try {
                // ONLY actual F460 invocation. Preflight, allocator and
                // post-normal private fences are deliberately OUTSIDE.
                constructed = copy_construct_key_00a9f460(
                    allocated, context.actual_key8, context.strings);
            } catch (...) {
                const auto original = std::current_exception();
                frame.constructor_failure_ = original;
                p.constructor_failed = true;
                frame.fail(original); // sticky BEFORE real selected cleanup
                p.selected_return_entered = true;
                try {
                    return_native_locale_node_slot_00736090(actual_pool, allocated);
                    p.selected_return_returned = true;
                } catch (...) {
                    frame.selected_return_failure_ = std::current_exception();
                    p.selected_return_failed = true;
                    // Retain secondary failure metadata and ALL partial debt;
                    // preserve original constructor exception, no retry.
                }
                std::rethrow_exception(original);
            }
            p.constructed_node = constructed;
            p.constructor_returned = true;
            frame.require_entered(); // normal ctor debt is NOT failed-ctor cleanup
        }
        // Native NULL allocation skips F460 and still reaches these accesses.
        // No invented graceful NULL result, fallback or selected return.
        const Word completed_node = address(constructed);
        const Word old_head_for_next = word(map, bucket_offset);
        put_word(completed_node, 0x14u, old_head_for_next);
        p.next_stored = true;
        put_word(completed_node, 0x10u, 0u);
        p.previous_stored = true;
        const Word current_head = word(map, bucket_offset); // genuine SECOND read
        if (current_head != 0u) {
            put_word(current_head, 0x10u, completed_node);
            p.former_head_previous_stored = true;
        }
        put_word(map, bucket_offset, completed_node);
        p.head_published = true;
        return frame.finish(pointer(completed_node + 8u));
    } catch (...) {
        frame.fail(std::current_exception());
        throw; // no generic payload/header/count/lock/slot cleanup
    }
}
} // namespace bsp
