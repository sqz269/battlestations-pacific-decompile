#include "bsp/native_render_queue_rows.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render queue rows require MSVC Win32 x87 operation ordering.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* address, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const void*>(
        reinterpret_cast<std::uintptr_t>(address) + offset), sizeof(value));
    return value;
}
template<class T> void write(void* address, std::uint32_t offset, T value) noexcept {
    std::memcpy(reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(address) + offset),
        &value, sizeof(value));
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void* row_at(void* data, std::uint32_t index) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(data) + index * 20u);
}
void release_string(void* row, NativeStringStorage& storage) noexcept {
    auto* const data = read<char*>(row, 4);
    if (data != nullptr) storage.release(data, read<std::uint32_t>(row, 0) + 1u);
    // Native inline destruction leaves both words untouched.
}
void copy_float_words(void* destination, const void* source) noexcept {
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax + 8]
        fstp dword ptr [edx + 8]
        fld dword ptr [eax + 0ch]
        fstp dword ptr [edx + 0ch]
        fld dword ptr [eax + 10h]
        fstp dword ptr [edx + 10h]
    }
}
} // namespace

void reserve_native_render_queue_rows_00b1db30(void* header,
    NativeStringStorage& storage, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (read<std::int32_t>(header, 8) >= requested) return;

    const auto bytes = static_cast<std::uint32_t>(requested) * 20u;
    void* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::pointer_slots, bytes, bytes});
    // No rollback guard: native CBCA80 calls 00401130, whose entire body is RET.
    for (std::uint32_t i = 0; signed_word(i) < read<std::int32_t>(header, 4); ++i) {
        void* const destination = row_at(replacement, i);
        if (destination != nullptr) {
            void* const source = row_at(read<void*>(header, 0), i);
            write<std::uint32_t>(destination, 0, 0);
            write<char*>(destination, 4, nullptr);
            if (destination != source) {
                resize_native_string_header_0041dd40(destination, storage,
                    read<std::uint32_t>(source, 0), true);
                if (read<std::uint32_t>(source, 0) != 0) {
                    const auto length = read<std::uint32_t>(destination, 0);
                    const auto* const source_data = read<char*>(source, 4);
                    auto* const destination_data = read<char*>(destination, 4);
                    std::memcpy(destination_data, source_data, length);
                }
            }
            copy_float_words(destination, source);
        }
    }
    for (std::uint32_t i = 0; signed_word(i) < read<std::int32_t>(header, 4); ++i)
        release_string(row_at(read<void*>(header, 0), i), storage);

    singleton_lifetime_free(read<void*>(header, 0));
    write<void*>(header, 0, replacement);
    write<std::int32_t>(header, 8, requested);
}

void resize_native_render_queue_rows_00b1e7f0(void* header,
    NativeStringStorage& storage, std::int32_t requested) {
    if (requested > read<std::int32_t>(header, 8))
        reserve_native_render_queue_rows_00b1db30(header, storage, requested);

    const auto old_count = read<std::int32_t>(header, 4);
    if (old_count < requested) {
        auto index = static_cast<std::uint32_t>(old_count);
        auto remaining = static_cast<std::uint32_t>(requested) - index;
        do {
            void* const row = row_at(read<void*>(header, 0), index);
            if (row != nullptr) {
                write<std::uint32_t>(row, 0, 0);
                write<char*>(row, 4, nullptr);
            }
            ++index;
        } while (--remaining != 0);
    }
    while (requested < read<std::int32_t>(header, 4)) {
        write<std::uint32_t>(header, 4, read<std::uint32_t>(header, 4) - 1u);
        const auto count = read<std::uint32_t>(header, 4);
        void* const data = read<void*>(header, 0);
        release_string(row_at(data, count), storage);
    }
    write<std::int32_t>(header, 4, requested);
}

void destroy_native_render_queue_rows_00b1f150(void* header, NativeStringStorage& storage) {
    resize_native_render_queue_rows_00b1e7f0(header, storage, 0);
    singleton_lifetime_free(read<void*>(header, 0));
}
} // namespace bsp
