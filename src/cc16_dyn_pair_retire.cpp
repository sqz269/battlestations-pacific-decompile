#include "bsp/cc16_dyn_pair_retire.hpp"

namespace bsp::cc16_dyn_pair_retire {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered storage uses Win32 pointers");
U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept { return *reinterpret_cast<volatile U32*>(location); }
volatile U32& field(const void* base, U32 offset) noexcept { return word(address(base) + offset); }
bool signed_less(U32 left, U32 right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}
}  // namespace

U32 erase_pair_reference_00c40e50(void* endpoint, U32 index,
                               const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 data = field(endpoint, 0x30);
    U32 eax = field(endpoint, 0x34);
    eax = word(data + eax * 4u - 4u);
    word(data + index * 4u) = eax;  // Swap last into the selected slot first.
    const U32 desired = field(endpoint, 0x34) - 1u;
    if (desired > field(endpoint, 0x34)) {  // Unsigned JBE, not a size guard.
        if (desired > field(endpoint, 0x38)) {
            const U32 bytes = desired * 4u;
            field(endpoint, 0x38) = desired;
            const U32 grown = address(crt.operator_new_00bf55be(bytes));
            U32 copied = 0;
            U32 destination = grown;
            while (copied < field(endpoint, 0x34)) {
                if (destination != 0) {
                    word(destination) = word(field(endpoint, 0x30) + copied * 4u);
                }
                ++copied;
                destination += 4u;
            }
            const U32 old = field(endpoint, 0x30);
            if (old != 0) {
                crt.free_00bf6989(pointer(old));
            }
            field(endpoint, 0x30) = grown;
        }
        eax = field(endpoint, 0x34);
        while (eax < desired) {
            const U32 slot = field(endpoint, 0x30) + eax * 4u;
            if (slot != 0) {
                word(slot) = 0;
            }
            ++eax;
        }
    }
    field(endpoint, 0x34) = desired;
    return eax;
}

void* retire_endpoint_pair_00c4c5d0(void* first_endpoint, void* storage,
                                 void* other_endpoint,
                                 const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 manager = address(storage);  // EBX before the first search.
    const U32 other = address(other_endpoint);  // EBP retained across leaf calls.
    const U32 first_count = field(first_endpoint, 0x34);
    U32 pair = 0;
    if (signed_less(0, first_count)) {
        const U32 data = field(first_endpoint, 0x30);  // EBX cached for match reload.
        U32 scan = data;
        U32 index = 0;
        do {
            const U32 candidate = word(scan);
            if (word(candidate) == other || word(candidate + 4u) == other) {
                pair = word(data + index * 4u);
                break;
            }
            ++index;
            scan += 4u;
        } while (signed_less(index, first_count));
    }

    // Search again by the captured pair pointer, retaining the initial signed
    // count but reloading the first reference-array base, including pair==0.
    if (signed_less(0, first_count)) {
        U32 scan = field(first_endpoint, 0x30);
        U32 index = 0;
        do {
            if (word(scan) == pair) {
                erase_pair_reference_00c40e50(first_endpoint, index, crt);
                break;
            }
            ++index;
            scan += 4u;
        } while (signed_less(index, first_count));
    }

    // Other endpoint's size and data are fresh after any first-leaf callback.
    const U32 other_count = field(pointer(other), 0x34);
    if (signed_less(0, other_count)) {
        U32 scan = field(pointer(other), 0x30);
        U32 index = 0;
        do {
            if (word(scan) == pair) {
                erase_pair_reference_00c40e50(pointer(other), index, crt);
                break;
            }
            ++index;
            scan += 4u;
        } while (signed_less(index, other_count));
    }

    // Unconditional unlink/recycle. Retain both native reloads around the
    // first neighbor write; no missing-pair return or field clearing added.
    const U32 next = word(pair + 0x0cu);
    const U32 previous = word(pair + 0x08u);
    word(next + 0x08u) = previous;
    const U32 reloaded_next = word(pair + 0x0cu);
    const U32 reloaded_previous = word(pair + 0x08u);
    word(reloaded_previous + 0x0cu) = reloaded_next;
    const U32 free_head = word(manager + 0x34u);
    word(pair + 0x0cu) = free_head;
    word(manager + 0x58u) = word(manager + 0x58u) - 1u;
    word(manager + 0x34u) = pair;
    return pointer(reloaded_next);
}

}  // namespace bsp::cc16_dyn_pair_retire
