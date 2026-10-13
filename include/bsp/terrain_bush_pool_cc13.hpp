#pragma once
// The BushGroup generated-model pool (packet cc13-terrain2, docs/CC13_TERRAIN_1.md):
// one more instance of the Mit slab pool engine that the GUI widget pools use
// (include/bsp/native_gui_widget_pools_cc13.hpp, src/native_material_pools.cpp).
//
// Evidence: a capstone comparison of every member against the GUI Widget pool with
// immediates masked (local/t2/t2_cmp.py in the cc13-terrain2 tree): Allocate 00AE1870,
// Free 00AE13D0, Trim 00AE17D0, Destroy 00AE12F0, Construct 00AE16F0, InitBlock 00AE07C0
// and FreeThunk 00AE15D0 match instruction for instruction; the only differences are the
// constants (slot 18Ch, 8 slots, free stack C60h, free count C70h, slab C74h, slab index at
// slot+188h, vtable 00D5D4FC) and the stride division (IMUL A57EB503h, SAR 8). The global
// 00F8C228 is loaded by the free thunk, the allocate stub 00AE1A00 (MOV ECX; JMP Allocate),
// the CRT initializer 00CD77D0 (CALL Construct) and the exit stub 00CE0B60 (JMP Destroy).
//
// Status: reconstructed over the shared engine, build-tested; payloads are never
// constructed or destroyed here (the native pool does not either). Not game-validated.

#include <cstddef>
#include <cstdint>

namespace bsp {

struct BushGroupModelPoolTraits {
    static constexpr std::uint32_t native_global = 0x00F8C228;
    static constexpr std::uint32_t native_vtable = 0x00D5D4FC;
    static constexpr std::uint32_t native_virtual0 = 0x00AE17D0;  // Trim
    static constexpr std::size_t slot_bytes = 0x18C;
    static constexpr std::size_t object_bytes = slot_bytes - 4;
    static constexpr std::size_t slot_slab_index_offset = slot_bytes - 4;
    static constexpr std::uint32_t slots_per_slab = 8;
    static constexpr std::size_t free_indices_offset = slot_bytes * slots_per_slab;                  // C60h
    static constexpr std::size_t free_count_offset = slot_bytes * slots_per_slab + 2 * slots_per_slab; // C70h
    static constexpr std::size_t slab_bytes = free_count_offset + 4;                                  // C74h
    static constexpr std::uint32_t construct_address = 0x00AE16F0;
    static constexpr std::uint32_t allocate_address = 0x00AE1870;
    static constexpr std::uint32_t free_address = 0x00AE13D0;
    static constexpr std::uint32_t trim_address = 0x00AE17D0;
    static constexpr std::uint32_t destroy_address = 0x00AE12F0;
    static constexpr std::uint32_t init_block_address = 0x00AE07C0;
    static constexpr std::uint32_t free_thunk_address = 0x00AE15D0;
    static constexpr std::uint32_t allocate_stub_address = 0x00AE1A00;
};
static_assert(BushGroupModelPoolTraits::free_indices_offset == 0xC60);
static_assert(BushGroupModelPoolTraits::free_count_offset == 0xC70);
static_assert(BushGroupModelPoolTraits::slab_bytes == 0xC74);

} // namespace bsp
