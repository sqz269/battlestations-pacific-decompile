#include "bsp/native_input_tree_head_builders.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

// Fixed private family, instantiated only by the four literal ordinary entries.
template<Word Bytes, Word ColorOffset, Word NilOffset>
void* buy_head() {
    void* const block = singleton_lifetime_allocate({
        SingletonAllocationKind::object, Bytes, Bytes});
    Word const base = reinterpret_cast<Word>(block);
    if (base != 0) {
        *reinterpret_cast<volatile Word*>(base) = 0;
    }
    Word const parent = base + 4U;
    if (parent != 0) {
        *reinterpret_cast<volatile Word*>(parent) = 0;
    }
    Word const right = base + 8U;
    if (right != 0) {
        *reinterpret_cast<volatile Word*>(right) = 0;
    }
    // These stores are unconditional, including the native anomalous-null path.
    *reinterpret_cast<volatile std::uint8_t*>(base + ColorOffset) = 1;
    *reinterpret_cast<volatile std::uint8_t*>(base + NilOffset) = 0;
    return block;
}
} // namespace

void* __cdecl buy_native_input_device_tree_head_0069d1f0() {
    return buy_head<0x9CU, 0x98U, 0x99U>();
}

void* __cdecl buy_native_input_preset_tree_head_0069d240() {
    return buy_head<0x2CU, 0x28U, 0x29U>();
}

void* __cdecl buy_native_input_default_scalar_tree_head_0069d290() {
    return buy_head<0x18U, 0x14U, 0x15U>();
}

void* __cdecl buy_native_input_controller_tree_head_0069d320() {
    return buy_head<0x24U, 0x20U, 0x21U>();
}

} // namespace bsp
