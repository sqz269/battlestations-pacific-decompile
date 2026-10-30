#include "bsp/native_locale_node_allocation.hpp"

#include "bsp/cc13_sweepC.hpp"
#include "bsp/game_native_locale_pool.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <stdexcept>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw locale node allocation requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::uint16_t) == 2);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}

void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

// Native-width accesses/RMWs and wrapping addresses, independent of typed
// pointer arithmetic or a projected C++ pool/table/container layout.
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

void add_word(Word base, Word byte_offset, Word value) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        add dword ptr [eax + edx], ecx
    }
}

std::uint16_t half(Word base, Word byte_offset) noexcept {
    std::uint16_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ax, word ptr [eax + edx]
        mov result, ax
    }
    return result;
}

void decrement_half(Word base, Word byte_offset) noexcept {
    __asm {
        mov eax, base
        mov edx, byte_offset
        add word ptr [eax + edx], 0ffffh
    }
}

void* initialize_block(void* actual_block, Word current_index) {
    // Existing genuine host-free E970 has a void Source result. The native
    // EAX was SAME block; only normalize that result after real normal return.
    cc13_sweepC::string_map_node_pool_init_block_00a9e970(actual_block, current_index);
    return actual_block;
}
} // namespace

NativeLocaleNodeAllocationFrame::NativeLocaleNodeAllocationFrame(
    game::GameNativeLocalePoolProcess& owner) noexcept : owner_(owner) {
    progress_.owner = &owner;
}

bool NativeLocaleNodeAllocationFrame::has_normal_result() const noexcept {
    return progress_.phase == NativeLocaleNodeAllocationPhase::returned &&
        progress_.normal_returned && !progress_.failed && !failure_;
}

void* NativeLocaleNodeAllocationFrame::normal_result() const noexcept {
    return has_normal_result() ? progress_.node : nullptr;
}

[[noreturn]] void NativeLocaleNodeAllocationFrame::reject(const char* message) {
    progress_.failed = true;
    progress_.phase = NativeLocaleNodeAllocationPhase::failed;
    throw std::logic_error(message);
}

void NativeLocaleNodeAllocationFrame::require_entered() {
    if (progress_.phase != NativeLocaleNodeAllocationPhase::entered ||
        progress_.failed || failure_)
        reject("locale node allocation cannot resume after caught reentry/failure");
}

void NativeLocaleNodeAllocationFrame::fail() noexcept {
    progress_.failed = true;
    progress_.phase = NativeLocaleNodeAllocationPhase::failed;
    if (!failure_) failure_ = std::current_exception();
}

void* allocate_native_locale_node_00a9f0b0(
    game::GameNativeLocalePoolProcess& owner, NativeLocaleNodeAllocationFrame& frame) {
    auto& p = frame.progress_;
    try {
        if (&owner != &frame.owner_ || p.phase != NativeLocaleNodeAllocationPhase::prepared ||
            p.attempted || p.failed || frame.failure_)
            frame.reject("locale node allocation requires its exact fresh prepared frame/owner");
        p.attempted = true; // consume BEFORE borrow/diagnostic/provider effects
        p.phase = NativeLocaleNodeAllocationPhase::entered;
        p.pool = owner.pool_storage_00e1768c();
        p.pool_borrow_returned = true;
        frame.require_entered();
        const Word pool = address(p.pool);
        const Word section = pool + 0x0cu;
        p.section = pointer(section);
        p.enter_entered = true;
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(pointer(section)));
        p.enter_returned = true;
        frame.require_entered();
        add_word(section, 0x18u, 1u);
        p.depth_incremented = true;

        if (word(pool, 0x34u) == 0xffffffffu) {
            const Word slab_index = word(pool, 0x2cu);
            put_word(pool, 0x34u, slab_index);
            p.first_free_published = true;
            p.slab_allocation_entered = true;
            void* const memory = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 0x7804u, 0x7804u});
            p.slab_allocation = memory; // capture BEFORE any subsequent effect
            p.slab_allocation_returned = true;
            frame.require_entered();
            void* block = nullptr;
            if (memory != nullptr) {
                const Word current_index = word(pool, 0x34u); // AFTER allocator
                p.block_initialization_entered = true;
                block = initialize_block(memory, current_index);
                p.constructed_slab = block;
                p.block_initialization_returned = true;
                frame.require_entered();
            }
            // Native NULL continues: no invented graceful NULL return/cleanup.
            const Word capacity = word(pool, 0x30u);
            if (word(pool, 0x2cu) == capacity) {
                const Word grown_capacity = capacity * 2u + 2u;
                const Word bytes = grown_capacity * 4u;
                put_word(pool, 0x30u, grown_capacity); // BEFORE allocation
                p.capacity_published = true;
                p.table_allocation_bytes = bytes;
                p.table_allocation_entered = true;
                void* const grown = singleton_lifetime_allocate(
                    {SingletonAllocationKind::object, bytes, bytes});
                p.table_allocation = grown;
                p.table_allocation_returned = true;
                frame.require_entered();
                const Word new_table = address(grown);
                Word destination = new_table;
                for (Word i = 0u; i < word(pool, 0x2cu); ++i, destination += 4u) {
                    if (destination != 0u) {
                        const Word current_table = word(pool, 0x28u);
                        const Word element = word(current_table, i * 4u);
                        put_word(destination, 0u, element);
                    }
                }
                const Word old_table = word(pool, 0x28u);
                p.old_table = pointer(old_table);
                if (old_table != 0u) {
                    p.old_table_free_entered = true;
                    singleton_lifetime_free(pointer(old_table));
                    p.old_table_free_returned = true;
                    frame.require_entered();
                }
                put_word(pool, 0x28u, new_table);
                p.table_published = true;
            }
            const Word append_index = word(pool, 0x2cu);
            const Word append_table = word(pool, 0x28u);
            const Word slot = append_table + append_index * 4u;
            if (slot != 0u) {
                put_word(slot, 0u, address(block));
                p.slab_store_performed = true;
            }
            add_word(pool, 0x2cu, 1u);
            p.slab_count_incremented = true;
        }

        const Word current_index = word(pool, 0x34u);
        const Word current_table = word(pool, 0x28u);
        const Word block = word(current_table, current_index * 4u);
        p.selected_slab = pointer(block);
        decrement_half(block, 0x7800u);
        p.free_count_decremented = true;
        const auto free_count = half(block, 0x7800u); // actual post-RMW reload
        const auto slot_index = half(block, 0x7000u + static_cast<Word>(free_count) * 2u);
        const Word node = block + static_cast<Word>(slot_index) * 0x1cu;
        p.node = pointer(node);
        if (free_count == 0u) {
            Word next = word(pool, 0x34u) + 1u;
            const bool has_next = next < word(pool, 0x2cu); // CMP before store
            put_word(pool, 0x34u, 0xffffffffu);
            if (has_next) {
                Word cursor = word(pool, 0x28u) + next * 4u; // captured ONCE
                do {
                    const Word candidate = word(cursor);
                    if (half(candidate, 0x7800u) != 0u) {
                        put_word(pool, 0x34u, next);
                        break;
                    }
                    ++next;
                    cursor += 4u;
                } while (next < word(pool, 0x2cu)); // CURRENT count, captured cursor
            }
        }
        add_word(section, 0x18u, 0xffffffffu);
        p.depth_decremented = true;
        p.leave_entered = true;
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(pointer(section)));
        p.leave_returned = true;
        frame.require_entered(); // rejected/caught reentry cannot issue receipt
        p.normal_returned = true;
        p.phase = NativeLocaleNodeAllocationPhase::returned;
        return pointer(node);
    } catch (...) {
        frame.fail(); // metadata ONLY; never unlock/free/rollback/retry
        throw;
    }
}
} // namespace bsp
