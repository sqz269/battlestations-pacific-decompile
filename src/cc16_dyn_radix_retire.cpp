#include "bsp/cc16_dyn_radix_retire.hpp"

namespace bsp::cc16_dyn_radix_retire {
namespace {
using U32 = std::uint32_t;
using U16 = std::uint16_t;
static_assert(sizeof(void*) == 4, "The recovered raw addresses require Win32.");
U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U16& half(U32 location) noexcept {
    return *reinterpret_cast<volatile U16*>(location);
}
}  // namespace

void remove_radix_slot_00c40cc0(void* manager, void* object,
                              const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 owner = address(manager);
    const U32 index = half(address(object) + 0x20u);
    const U32 base = word(owner + 4u);
    const U32 count = word(owner + 8u);
    const U32 last_key = word(base + count * 8u - 8u);
    const U32 destination = base + index * 8u;
    word(destination) = last_key;
    const U32 moved_object = word(base + count * 8u - 4u);
    word(destination + 4u) = moved_object;
    half(moved_object + 0x20u) = static_cast<U16>(index);

    const U32 next_count = word(owner + 8u) - 1u;
    // Keep both unsigned gates. Count zero can enter this wrap/growth arm;
    // there is no native empty-vector guard to replace it with a no-op.
    if (next_count > word(owner + 8u) && next_count > word(owner + 0x0cu)) {
        word(owner + 0x0cu) = next_count;
        const U32 grown = address(crt.operator_new_00bf55be(next_count * 8u));
        U32 cursor = 0;
        U32 output = grown;
        while (cursor < word(owner + 8u)) {
            if (output != 0u) {
                const U32 input_base = word(owner + 4u);
                word(output) = word(input_base + cursor * 8u);
                word(output + 4u) = word(input_base + cursor * 8u + 4u);
            }
            ++cursor;
            output += 8u;
        }
        const U32 old = word(owner + 4u);
        if (old != 0u) {
            crt.free_00bf6989(pointer(old));
        }
        word(owner + 4u) = grown;
    }
    word(owner + 8u) = next_count;
}

void retire_radix_object_00c40d50(void* manager, void* object,
                                const cc15_dyn_live::DynLiveCrt& crt,
                                ObjectFree00BF65AC free_object) {
    remove_radix_slot_00c40cc0(manager, object, crt);
    free_object(object);
}

}  // namespace bsp::cc16_dyn_radix_retire
