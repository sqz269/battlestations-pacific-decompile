#include "bsp/native_pod8_vector_insert.hpp"
#include "bsp/native_pod8_failure.hpp"
#include "bsp/native_pod8_inplace_range.hpp"
#include "bsp/native_pod8_range.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

struct RawPod8 {
    std::uint32_t word0;
    std::uint32_t word1;
};
static_assert(sizeof(RawPod8) == 8);

// Raw x86 storage accesses, in the written order. The argument-cell address
// comes from the public entry's incoming ESP, not private C++ parameter copies.
__forceinline std::uint32_t bits(const void* pointer) noexcept {
    return reinterpret_cast<std::uint32_t>(pointer);
}
__forceinline void* pointer(std::uint32_t address) noexcept {
    return reinterpret_cast<void*>(address);
}
__forceinline std::uint32_t load(std::uint32_t address) noexcept {
    return *reinterpret_cast<volatile const std::uint32_t*>(address);
}
__forceinline void store(std::uint32_t address, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(address) = value;
}
__forceinline std::uint32_t sar3(std::uint32_t value) noexcept {
    return (value >> 3) | ((0u - (value >> 31)) << 29);
}
__forceinline std::uint32_t current_size(
    std::uint32_t owner, std::uint32_t captured_first) noexcept {
    return captured_first == 0 ? 0 : sar3(load(owner + 8) - captured_first);
}

// Native initializes only AL of a local tag. Read a real Source frame preimage
// through inline assembly, with the same byte-only initialization. Its upper
// bytes are not claimed to equal the original Native frame. The concrete copy
// kernel does not read this metadata, but the actual passed DWORD is retained.
__forceinline std::uint32_t source_frame_tag() noexcept {
    std::uint32_t tag;
    __asm {
        mov byte ptr tag, 0
        mov eax, dword ptr tag
        mov dword ptr tag, eax
    }
    return tag;
}

__forceinline void add_current_last(
    std::uint32_t owner, std::uint32_t amount) noexcept {
    __asm {
        mov ecx, owner
        mov eax, amount
        add dword ptr [ecx + 8], eax
    }
}

// These are fixed actual entry addresses, never caller-supplied providers.
// Dropping noexcept in the local pointer type is intentional: current MSVC
// must retain the recovered catch phases around these actual naked kernels.
using CopyEntry = void* (__fastcall*)(
    const void*, const void*, void*, std::uint32_t, std::uint32_t, std::uint32_t);
using FillDispatchEntry = void* (__fastcall*)(
    std::uint32_t, std::uint32_t, void*, std::uint32_t, const void*);
using FreeEntry = void (__cdecl*)(void*);

__declspec(noinline) void __cdecl insert_current_source_frames(
    void* actual_owner, void* actual_incoming_stack_cells) {
    const std::uint32_t owner = bits(actual_owner);
    const std::uint32_t cells = bits(actual_incoming_stack_cells);

    const std::uint32_t value_address = load(cells + 12);
    RawPod8 value;
    value.word1 = load(value_address + 4);
    value.word0 = load(value_address);
    const std::uint32_t first = load(owner + 4);
    const std::uint32_t capacity =
        first == 0 ? 0 : sar3(load(owner + 12) - first);
    const std::uint32_t count = load(cells + 8);
    if (count == 0) return;

    if (count > 0x1fffffffu - current_size(owner, first)) {
        throw_native_pod8_vector_length_error_004203a0();
    }

    if (capacity < current_size(owner, first) + count) {
        const std::uint32_t half = capacity >> 1;
        std::uint32_t grown_capacity =
            0x1fffffffu - half < capacity ? 0 : capacity + half;
        if (grown_capacity < current_size(owner, first) + count) {
            grown_capacity = current_size(owner, first) + count;
        }

        // Allocation is outside state0. Reload position/current first after it.
        void* const allocated = allocate_native_pod8_records_00413390(
            grown_capacity, nullptr);
        const std::uint32_t position = load(cells + 4);
        const std::uint32_t prefix_first = load(owner + 4);
        const std::uint32_t tag = source_frame_tag();
        const std::uint32_t prefix_metadata_count = load(cells + 8);
        store(cells + 12, bits(allocated));

        CopyEntry volatile copy_entry = copy_native_pod8_range_00419960;
        FillDispatchEntry volatile fill_entry = fill_n_native_pod8_dispatch_0041e3c0;
        FreeEntry volatile free_entry = singleton_lifetime_free;
        try {
            // Native state0: prefix, inserted run, suffix, old free, publication.
            void* const prefix_end = copy_entry(
                pointer(prefix_first), pointer(position), allocated,
                owner, prefix_metadata_count, tag);
            const std::uint32_t fill_count = load(cells + 8);
            void* const fill_end = fill_entry(owner, 0, prefix_end, fill_count, &value);
            const std::uint32_t old_last = load(owner + 8);
            *reinterpret_cast<volatile unsigned char*>(cells + 4) = 0;
            const std::uint32_t suffix_tag = load(cells + 4);
            const std::uint32_t suffix_metadata_count = load(cells + 8);
            copy_entry(pointer(position), pointer(old_last), fill_end,
                       owner, suffix_metadata_count, suffix_tag);

            const std::uint32_t old_first = load(owner + 4);
            const std::uint32_t old_size = current_size(owner, old_first);
            const std::uint32_t total = load(cells + 8) + old_size;
            if (old_first != 0) free_entry(pointer(old_first));
            const std::uint32_t current_allocation = load(cells + 12);
            store(owner + 12, current_allocation + grown_capacity * 8);
            store(owner + 8, current_allocation + total * 8);
            store(owner + 4, current_allocation);
        } catch (...) {
            // Native catch1: free the CURRENT reused value slot; no rollback.
            free_entry(pointer(load(cells + 12)));
            throw;
        }
        return;
    }

    const std::uint32_t old_last = load(owner + 8);
    const std::uint32_t position = load(cells + 4);
    if (sar3(old_last - position) < count) {
        const std::uint32_t count_bytes = count * 8;
        store(cells + 12, count_bytes);
        // This first copy precedes state2; its failure has no insertion catch.
        copy_native_pod8_dispatch_0041f4e0(
            owner, 0, pointer(position), pointer(old_last), pointer(position + count_bytes));
        const std::uint32_t current_last = load(owner + 8);
        const std::uint32_t current_count = load(cells + 8);
        const std::uint32_t excess = current_count - sar3(current_last - position);
        FillDispatchEntry volatile fill_entry = fill_n_native_pod8_dispatch_0041e3c0;
        try {
            // Native state2: excess fill, extend CURRENT last, prefix assignment.
            fill_entry(owner, 0, pointer(current_last), excess, &value);
            const std::uint32_t current_count_bytes = load(cells + 12);
            add_current_last(owner, current_count_bytes);
            const std::uint32_t fill_last = load(owner + 8) - current_count_bytes;
            fill_native_pod8_range_0041c680(pointer(position), pointer(fill_last), &value);
        } catch (...) {
            // Native catch2 is only the shared zero/zero rethrow tail.
            throw;
        }
        return;
    }

    const std::uint32_t count_bytes = count * 8;
    const std::uint32_t split = old_last - count_bytes;
    store(cells + 12, count_bytes);
    store(cells + 8, split);
    void* const new_last = copy_native_pod8_dispatch_0041f4e0(
        owner, 0, pointer(split), pointer(old_last), pointer(old_last));
    const std::uint32_t current_split = load(cells + 8);
    store(owner + 8, bits(new_last));
    copy_backward_native_pod8_range_0041dac0(
        pointer(position), pointer(current_split), pointer(old_last));
    const std::uint32_t current_count_bytes = load(cells + 12);
    fill_native_pod8_range_0041c680(
        pointer(position), pointer(position + current_count_bytes), &value);
}

} // namespace

__declspec(naked) void __fastcall insert_native_pod8_vector_00420f80(
    void*, void*, void*, void*, std::uint32_t, const void*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push ecx
        call insert_current_source_frames
        add esp, 8
        ret 10h
    }
}

} // namespace bsp
