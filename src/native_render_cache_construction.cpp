#include "bsp/native_render_cache_construction.hpp"
#include "bsp/camera_plane_initialization.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render cache construction requires MSVC Win32.
#endif

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(CameraPlaneSet) == 0x144);
static_assert(alignof(CameraPlaneSet) == 4);

// Raw stores preserve the native widths and sequence without constructing a
// typed cache overlay or coalescing individually ordered fields into a memset.
__forceinline void store_word(void* address, std::uint32_t value = 0) noexcept {
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}
__forceinline void store_half(void* address) noexcept {
    __asm {
        mov eax, address
        xor edx, edx
        mov word ptr [eax], dx
    }
}
__forceinline void store_byte(void* address) noexcept {
    __asm {
        mov eax, address
        mov byte ptr [eax], 0
    }
}

void clear_bank_fields(std::byte* bank) noexcept {
    for (std::size_t offset = 0; offset != 0x20; offset += 4)
        store_word(bank + offset);
    store_byte(bank + 0x20);
}

void initialize_records(std::byte* cache) noexcept {
    for (std::size_t i = 0; i != 20; ++i) {
        auto* const record = cache + 0x1198 + i * 0x48;
        store_word(record);
        store_word(record + 4);
        store_word(record + 8);
        store_half(record + 0x0c);
    }
}

void clear_tail_values(std::byte* cache) noexcept {
    // Native MOVSS order starts18D8,18E8,18DC,18E0,18E4, then18EC..1934.
    constexpr std::size_t offsets[] = {
        0x18d8, 0x18e8, 0x18dc, 0x18e0, 0x18e4, 0x18ec,
        0x18f0, 0x18f4, 0x18f8, 0x18fc, 0x1900, 0x1904,
        0x1908, 0x190c, 0x1910, 0x1914, 0x1918, 0x191c,
        0x1920, 0x1924, 0x1928, 0x192c, 0x1930, 0x1934,
    };
    for (const auto offset : offsets) store_word(cache + offset);
}

// B241C0 construction-only fragment, NOT a general release implementation.
// Private sole production caller below proves all26 read owner cells null.
// Their native null branches neither release nor write the owner slot. Thus
// only this exact field-write schedule is reached. No terminal fallback exists.
void clear_constructed_cache_all_null_00b241c0_fragment(void* actual_cache) noexcept {
    auto* const cache = static_cast<std::byte*>(actual_cache);
    std::memset(cache + 0x0c, 0, 0xd2);
    store_word(cache + 0x1738);
    store_word(cache + 0x173c);
    store_word(cache + 0x1788, 0xffffffffu);
    for (std::size_t i = 0; i != 4; ++i) {
        auto* const stream = cache + 0x1740 + i * 0x10;
        store_word(stream + 4);
        store_word(stream + 8);
        store_word(stream + 0x0c);
    }
    // Reset visits only16 banks; the constructor initializes20.
    for (std::size_t i = 0; i != 16; ++i)
        clear_bank_fields(cache + 0x428 + i * 0xac);
    initialize_records(cache);
    clear_tail_values(cache);
}

} // namespace

void* construct_native_render_cache_banks_00b27e80(void* actual_banks) noexcept {
    auto* const banks = static_cast<std::byte*>(actual_banks);
    for (std::size_t i = 0; i != 20; ++i) {
        auto* const bank = banks + i * 0xac;
        store_word(bank + 0xa8);
        clear_bank_fields(bank);
        // Original reload/conditional release sees the zero just stored. All
        // intervening writes are within00..20 and there is no callable edge.
    }
    return actual_banks;
}

void* construct_native_render_cache_00b29430(void* actual_cache,
    const volatile std::uint32_t& live_one_00d7a24c) {
    auto* const cache = static_cast<std::byte*>(actual_cache);
    store_word(cache);
    store_word(cache + 4);
    store_word(cache + 8);
    construct_native_render_cache_banks_00b27e80(cache + 0x428);
    initialize_records(cache);

    // This is the actual embedded object whose C++ lifetime the caller began
    // before entry. It is not a local replacement or a default-initialization.
    auto& planes = *reinterpret_cast<CameraPlaneSet*>(cache + 0x178c);
    construct_camera_plane_set_00b659d0(planes, live_one_00d7a24c);

    store_word(cache + 0x1938); // Gamma: separate from reset's preserved tail.
    store_word(cache + 0x1738);
    store_word(cache + 0x173c);
    store_word(cache + 0x1780);
    store_word(cache + 0x1784);
    for (std::size_t i = 0; i != 4; ++i) {
        auto* const stream = cache + 0x1740 + i * 0x10;
        store_word(stream);
        store_word(stream + 4);
        store_word(stream + 8);
    }
    store_word(cache + 0x18d4);
    store_word(cache + 0x18d0);
    clear_tail_values(cache);

    // 3 initial +2 layout/index +4 streams +16 banks +1 target =26 null owners.
    // The only intervening helper wrote178C..18CF, disjoint from these cells,
    // under its existing finite-CRT contract. No missing cleanup is invoked.
    clear_constructed_cache_all_null_00b241c0_fragment(actual_cache);
    return actual_cache;
}

} // namespace bsp
