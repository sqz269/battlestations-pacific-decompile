#include "bsp/native_particle_record_resize.hpp"

#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle record resize requires MSVC Win32 pointer widths.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeResourceRecordVectorStorage) == 0x0c);
static_assert(sizeof(NativeRenderResourceRecord) == 0x2c);

void* at(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(static_cast<Word>(
        reinterpret_cast<std::uintptr_t>(base)) + offset);
}

volatile Word& word(void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}

Word load(void* base, Word offset = 0) noexcept {
    return word(base, offset);
}

void store(void* base, Word offset, Word value) noexcept {
    word(base, offset) = value;
}

void* pointer(void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(load(base, offset));
}

std::int32_t signed_bits(Word bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

void resize_with_public_slot(NativeResourceRecordVectorStorage& actual_vector,
    NativeParticleRecordResizeContext& context,
    volatile std::int32_t* const requested_slot) {
    void* const vector = &actual_vector;
    std::int32_t bound = *requested_slot; // 004DC429: original public stack word.
    if (bound > signed_bits(load(vector, 8))) {
        reserve_native_resource_record_vector_004da180(
            actual_vector, bound, context.strings, context.validation);
    }

    // 004DC43F is a fresh count read after reserve. Each growth iteration
    // recomputes its address from the current vector data pointer.
    Word index = load(vector, 4);
    while (signed_bits(index) < bound) {
        auto* const record = static_cast<NativeRenderResourceRecord*>(
            at(pointer(vector), index * 0x2cu));
        bool name_armed = false;
        try {
            if (record != nullptr) {
                ::new (record) NativeRenderResourceRecord; // No value writes.
                store(record, 0, 0);
                store(record, 4, 0);
                name_armed = true; // Native state 1 precedes 004C3020.

                auto* const sentinel =
                    allocate_native_render_alias_sentinel_004c3020();
                store(record, 0x0c, static_cast<Word>(
                    reinterpret_cast<std::uintptr_t>(sentinel)));
                store(record, 0x10, 0);
                bound = *requested_slot; // 004DC47F: same current public word.
                store(record, 0x24, 0);
                store(record, 0x20, 0);
                store(record, 0x1c, 0);
                store(record, 0x18, 0);
                store(record, 0x14, 0);
            }
        } catch (...) {
            // State 1 first returns the current name header. State 0 then
            // rereads the current vector data and computes the placement
            // record address; original 00401130 is a RET-only target.
            if (name_armed) {
                destroy_native_string_header_0041dd20(record, context.strings);
            }
            volatile Word placement_address =
                load(vector, 0) + index * 0x2cu;
            (void)placement_address;
            throw;
        }
        ++index;
        // Native state -1 is published after the increment, before the
        // next signed comparison. No completed-prefix rollback is invented.
    }

    // Native uses the last captured/reloaded bound here; record destructors
    // can mutate the current vector count and data, both reread each pass.
    while (bound < signed_bits(load(vector, 4))) {
        // MSVC lowers even a volatile C++ compound assignment to a separate
        // load/store here. Native 004DC4B0 is one memory ADD, so keep it one.
        __asm {
            mov eax, vector
            add dword ptr [eax + 4], -1
        }
        const Word current_index = load(vector, 4);
        auto* const record = static_cast<NativeRenderResourceRecord*>(
            at(pointer(vector), current_index * 0x2cu));
        destroy_native_resource_record_004d45a0(*record, context.strings);
    }
    store(vector, 4, static_cast<Word>(bound));
}
} // namespace

void __fastcall resize_native_particle_record_array_004dc410(
    NativeResourceRecordVectorStorage& actual_vector,
    NativeParticleRecordResizeContext& context,
    volatile std::int32_t requested_count) {
    // Address-taking must compile to the original third-argument stack slot;
    // CY checks the emitted Win32 adapter before claiming public-slot parity.
    resize_with_public_slot(actual_vector, context, &requested_count);
}

} // namespace bsp
