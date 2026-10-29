#include "bsp/cc16_dyn_sap_retire.hpp"

#include "bsp/cc16_dyn_pair_retire.hpp"
#include "bsp/cc16_dyn_sap_endpoints.hpp"

namespace bsp::cc16_dyn_sap_retire {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered raw addresses require Win32.");
U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile std::uint8_t& byte(U32 location) noexcept {
    return *reinterpret_cast<volatile std::uint8_t*>(location);
}
bool signed_less(U32 left, U32 right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}

void compact_pending(U32 manager, U32 endpoint) noexcept {
    const U32 count = word(manager + 0x198u);
    const U32 end_base = word(manager + 0x194u);
    const U32 end = end_base + count * 4u;
    U32 cursor = word(manager + 0x194u);
    while (cursor != end) {
        if (word(cursor) == endpoint) {
            break;
        }
        cursor += 4u;
    }
    const U32 compact_count = word(manager + 0x198u);
    const U32 compact_base = word(manager + 0x194u);
    U32 last = compact_base + compact_count * 4u - 4u;
    while (cursor != last) {
        word(cursor) = word(cursor + 4u);
        const U32 current_count = word(manager + 0x198u);
        const U32 current_base = word(manager + 0x194u);
        cursor += 4u;
        last = current_base + current_count * 4u - 4u;
    }
    word(manager + 0x198u) = word(manager + 0x198u) - 1u;
}

void recycle_endpoint(U32 manager, U32 endpoint, bool second_pool) noexcept {
    const U32 next = word(endpoint + 0x40u);
    const U32 previous = word(endpoint + 0x3cu);
    word(next + 0x3cu) = previous;
    const U32 reloaded_next = word(endpoint + 0x40u);
    const U32 reloaded_previous = word(endpoint + 0x3cu);
    word(reloaded_previous + 0x40u) = reloaded_next;
    const U32 head_offset = second_pool ? 0x104u : 0x68u;
    const U32 count_offset = second_pool ? 0x190u : 0xf4u;
    const U32 free_head = word(manager + head_offset);
    word(endpoint + 0x40u) = free_head;
    word(manager + count_offset) = word(manager + count_offset) - 1u;
    word(manager + head_offset) = endpoint;
}
}  // namespace

void retire_endpoint_pairs_00c4c550(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 storage = address(manager);
    const U32 owner = address(endpoint);
    U32 index = 0;
    while (signed_less(index, word(owner + 0x34u))) {
        const U32 data = word(owner + 0x30u);
        const U32 pair = word(data + index * 4u);
        U32 other = word(pair);
        if (other == owner) {
            other = word(pair + 4u);
        }
        const U32 other_count = word(other + 0x34u);
        if (signed_less(0u, other_count)) {
            U32 scan = word(other + 0x30u);
            U32 other_index = 0;
            do {
                if (word(scan) == pair) {
                    cc16_dyn_pair_retire::erase_pair_reference_00c40e50(
                        pointer(other), other_index, crt);
                    break;
                }
                ++other_index;
                scan += 4u;
            } while (signed_less(other_index, other_count));
        }
        const U32 next = word(pair + 0x0cu);
        const U32 previous = word(pair + 0x08u);
        word(next + 0x08u) = previous;
        // This helper reloads previous BEFORE next, unlike C4C5D0.
        const U32 reloaded_previous = word(pair + 0x08u);
        const U32 reloaded_next = word(pair + 0x0cu);
        word(reloaded_previous + 0x0cu) = reloaded_next;
        const U32 free_head = word(storage + 0x34u);
        word(pair + 0x0cu) = free_head;
        word(storage + 0x58u) = word(storage + 0x58u) - 1u;
        ++index;
        word(storage + 0x34u) = pair;
    }
}

void remove_and_recycle_endpoint_00c4cc90(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 storage = address(manager);
    const U32 owner = address(endpoint);
    if (byte(owner + 0x2cu) != 0u) {
        cc16_dyn_sap_endpoints::remove_endpoints_00c40800(endpoint, manager, 0u);
        cc16_dyn_sap_endpoints::remove_endpoints_00c40800(endpoint, manager, 1u);
        cc16_dyn_sap_endpoints::remove_endpoints_00c40800(endpoint, manager, 2u);
        retire_endpoint_pairs_00c4c550(manager, endpoint, crt);
    } else {
        compact_pending(storage, owner);
    }
    // Native branch selection is captured before the buffer read/free call.
    const bool second_pool = byte(owner + 0x1cu) != 0u;
    const U32 references = word(owner + 0x30u);
    if (references != 0u) {
        crt.free_00bf6989(pointer(references));
    }
    recycle_endpoint(storage, owner, second_pool);
}

}  // namespace bsp::cc16_dyn_sap_retire
