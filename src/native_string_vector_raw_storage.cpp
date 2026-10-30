#include "bsp/native_string_vector_raw_storage.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw0Ch string vectors require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Word) == 4 && sizeof(std::int32_t) == 4);

Word address(const void* value) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(value));
}
void* pointer(Word bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
std::int32_t signed_word(Word bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
Word word(Word base, Word byte_offset) noexcept {
    Word value;
    _ReadWriteBarrier();
    std::memcpy(&value, pointer(base + byte_offset), sizeof(value));
    _ReadWriteBarrier();
    return value;
}
void put_word(Word base, Word byte_offset, Word value) noexcept {
    _ReadWriteBarrier();
    std::memcpy(pointer(base + byte_offset), &value, sizeof(value));
    _ReadWriteBarrier();
}
void add_word(Word base, Word byte_offset, Word value) noexcept {
    _ReadWriteBarrier();
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov ecx, value
        add dword ptr [eax + edx], ecx
    }
    _ReadWriteBarrier();
}

// Native callers inline this constructor. Keep its complete reached header
// accesses fenced here; do not infer transitive fences from another facade.
void copy_header(Word destination, Word source, NativeStringRawPoolContext& strings) {
    const bool self = destination == source; // CMP BEFORE both zero stores
    put_word(destination, 0u, 0u);
    put_word(destination, 4u, 0u);
    if (!self) {
        const Word requested = word(source, 0u);
        resize_native_string_header_0041dd40(pointer(destination), strings, requested, true);
        if (word(source, 0u) != 0u) {
            const Word count = word(destination, 0u);
            const Word source_data = word(source, 4u);
            const Word destination_data = word(destination, 4u);
            // Existing raw-pool BF7680 overlap/zero-call qualification.
            if (count != 0u)
                std::memmove(pointer(destination_data), pointer(source_data), count);
        }
    }
}

void return_header(Word header, NativeStringRawPoolContext& strings) {
    const Word data = word(header, 4u);
    if (data != 0u) {
        const Word bytes = word(header, 0u) + 1u; // operands BEFORE getter
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
        return_native_string_pool_00bd1510(pool, pointer(data), bytes,
            strings.actual_small_returns_disabled_01090aa4);
    }
}
} // namespace

void reserve_native_string_vector_00426520(void* actual_vector,
    std::int32_t requested, NativeStringRawPoolContext& strings) {
    if (requested < 1) requested = 1;
    const Word vector = address(actual_vector);
    if (signed_word(word(vector, 8u)) >= requested) return;
    const Word bytes = static_cast<Word>(requested) * 8u;
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes}); // BF55BE -> BF681B
    const Word captured_fresh = address(fresh);
    for (Word i = 0u; signed_word(i) < signed_word(word(vector, 4u)); ++i) {
        const Word destination = captured_fresh + i * 8u;
        if (destination != 0u) {
            const Word source = word(vector, 0u) + i * 8u; // CURRENT data
            copy_header(destination, source, strings);
        }
    }
    for (Word i = 0u; signed_word(i) < signed_word(word(vector, 4u)); ++i) {
        const Word current_header = word(vector, 0u) + i * 8u;
        return_header(current_header, strings);
    }
    const Word current_old_data = word(vector, 0u);
    singleton_lifetime_free(pointer(current_old_data)); // even null; BEFORE publication
    put_word(vector, 0u, captured_fresh);
    put_word(vector, 8u, static_cast<Word>(requested));
}

void resize_native_string_vector_00427110(void* actual_vector,
    std::int32_t requested, NativeStringRawPoolContext& strings) {
    const Word vector = address(actual_vector);
    if (requested > signed_word(word(vector, 8u)))
        reserve_native_string_vector_00426520(actual_vector, requested, strings);
    for (Word i = word(vector, 4u); signed_word(i) < requested; ++i) {
        const Word header = word(vector, 0u) + i * 8u; // CURRENT data, local index
        if (header != 0u) {
            put_word(header, 0u, 0u);
            put_word(header, 4u, 0u);
        }
    }
    while (requested < signed_word(word(vector, 4u))) {
        add_word(vector, 4u, 0xffffffffu); // actual count RMW BEFORE return
        const Word count = word(vector, 4u);
        const Word data = word(vector, 0u);
        return_header(data + count * 8u, strings);
    }
    put_word(vector, 4u, static_cast<Word>(requested));
}

void append_native_string_vector_004cdc20(void* actual_vector,
    const void* actual_source, NativeStringRawPoolContext& strings) {
    const Word vector = address(actual_vector);
    const Word capacity = word(vector, 8u); // captured BEFORE current count
    if (word(vector, 4u) == capacity) {
        const Word doubled = capacity * 2u;
        const auto requested = signed_word(doubled) > 1 ? signed_word(doubled) : 1;
        reserve_native_string_vector_00426520(actual_vector, requested, strings);
    }
    const Word count = word(vector, 4u);
    const Word data = word(vector, 0u);
    const Word destination = data + count * 8u;
    if (destination != 0u)
        copy_header(destination, address(actual_source), strings);
    add_word(vector, 4u, 1u); // ONLY after actual copy return or null skip
}

void destroy_native_string_vector_004d0fa0(
    void* actual_vector, NativeStringRawPoolContext& strings) {
    resize_native_string_vector_00427110(actual_vector, 0, strings);
    const Word current_data = word(address(actual_vector), 0u);
    singleton_lifetime_free(pointer(current_data)); // header data/cap remain stale
}
} // namespace bsp
