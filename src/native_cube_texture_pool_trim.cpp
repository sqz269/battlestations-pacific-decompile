#include "bsp/native_cube_texture_pool_trim.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cube-texture pool trimming requires MSVC Win32.
#endif

namespace bsp {
namespace {

void* volatile& pointer(void* storage, std::uint32_t byte_offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + byte_offset);
}
volatile std::uint32_t& word(void* storage, std::uint32_t byte_offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + byte_offset);
}
volatile std::uint16_t& free_count(void* slab) noexcept {
    return *reinterpret_cast<volatile std::uint16_t*>(static_cast<unsigned char*>(slab) + 0x6c0);
}

} // namespace

void trim_native_cube_texture_pool_00b3e690(void* pool) noexcept {
    std::uint32_t index = 0;
    while (index < word(pool, 0x2c)) {
        auto* const table = pointer(pool, 0x28);
        auto* const slab = pointer(table, index * 4u);
        if (free_count(slab) == 32) {
            singleton_lifetime_free(slab);
            // B3E6B6..B3E6EC is the proven returning-free continuation.
            auto* const current_table = pointer(pool, 0x28);
            auto* const final_slab = pointer(current_table, word(pool, 0x2c) * 4u - 4u);
            pointer(current_table, index * 4u) = final_slab;
            word(pool, 0x2c) = word(pool, 0x2c) - 1u;
            if (index < word(pool, 0x2c)) {
                auto* const moved_table = pointer(pool, 0x28);
                auto* const moved_slab = pointer(moved_table, index * 4u);
                for (std::uint32_t slot = 0; slot < 32; ++slot) {
                    word(moved_slab, 0x30u + slot * 0x34u) = index;
                }
            }
            --index;
        }
        ++index;
    }
    index = 0;
    const bool has_slabs = word(pool, 0x2c) != 0;
    word(pool, 0x34) = 0xffffffffu;
    if (has_slabs) {
        // B3E703 captures the cursor once; subsequent loop tests reread count.
        auto* cursor = static_cast<unsigned char*>(pointer(pool, 0x28));
        do {
            if (free_count(pointer(cursor, 0)) != 0) {
                word(pool, 0x34) = index;
                return;
            }
            ++index;
            cursor += 4;
        } while (index < word(pool, 0x2c));
    }
}

void bind_native_cube_texture_pool_trim_00d61944(
    void* pool, AllocatorListDomain& list) {
    list.bind_virtual0(*static_cast<AllocatorListElement*>(pool),
        {0x00d61944, 0x00b3e690, pool, &trim_native_cube_texture_pool_00b3e690});
}

} // namespace bsp
