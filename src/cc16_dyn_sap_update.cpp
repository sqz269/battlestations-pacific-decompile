#include "bsp/cc16_dyn_sap_update.hpp"

#include "bsp/cc16_dyn_pair_retire.hpp"
#include "bsp/cc16_dyn_storage.hpp"

namespace bsp::cc16_dyn_sap_update {
namespace {
using U32 = std::uint32_t;
using U16 = std::uint16_t;
using U8 = std::uint8_t;
static_assert(sizeof(void*) == 4, "The recovered raw storage uses Win32 pointers");

U32 address(const void* value) noexcept { return reinterpret_cast<U32>(value); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U16& half(U32 location) noexcept {
    return *reinterpret_cast<volatile U16*>(location);
}
volatile U8& byte(U32 location) noexcept {
    return *reinterpret_cast<volatile U8*>(location);
}
bool signed_less(U32 left, U32 right) noexcept {
    return (left ^ 0x80000000u) < (right ^ 0x80000000u);
}

void grow_axis_entries(U32 axis_base, U32 old_capacity,
                       const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 capacity = old_capacity * 2u + 2u;
    word(axis_base + 0x0cu) = capacity;
    const U32 grown = address(crt.operator_new_00bf55be(capacity * 8u));
    U32 index = 0;
    U32 destination = grown;
    while (index < word(axis_base + 8u)) {
        if (destination != 0) {
            const U32 old = word(axis_base + 4u);
            word(destination) = word(old + index * 8u);
            word(destination + 4u) = word(old + index * 8u + 4u);
        }
        ++index;
        destination += 8u;
    }
    const U32 old = word(axis_base + 4u);
    if (old != 0) {
        crt.free_00bf6989(pointer(old));
    }
    word(axis_base + 4u) = grown;
}

bool half_exceeds(U32 left, U32 right) noexcept {
    const U16 first = half(left);
    const U16 second = half(right);
    return first > second;
}

bool overlaps_other_axes(U32 moving, U32 other, U32 axis) noexcept {
    // Keep both flag reads and the four unsigned word comparisons ordered.
    if (byte(moving + 0x1cu) != 0) {
        if (byte(other + 0x1cu) != 0) {
            return false;
        }
    }
    const U32 first_axis = (1u << (axis & 31u)) & 3u;
    const U32 second_axis = (1u << first_axis) & 3u;
    if (half_exceeds(moving + first_axis * 2u + 0x20u,
                     other + first_axis * 2u + 0x26u)) {
        return false;
    }
    if (half_exceeds(other + first_axis * 2u + 0x20u,
                     moving + first_axis * 2u + 0x26u)) {
        return false;
    }
    if (half_exceeds(moving + second_axis * 2u + 0x20u,
                     other + second_axis * 2u + 0x26u)) {
        return false;
    }
    return !half_exceeds(other + second_axis * 2u + 0x20u,
                          moving + second_axis * 2u + 0x26u);
}

void cross_endpoint(U32 storage, U32 moving, U32 copied_tag, U32 axis,
                    bool moving_minimum, bool forward,
                    const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 other = copied_tag & 0xfffffffeu;
    const bool other_minimum = (copied_tag & 1u) != 0;
    const U32 index_location = other + axis * 2u + (other_minimum ? 0x20u : 0x26u);
    half(index_location) = static_cast<U16>(half(index_location) + (forward ? 0xffffu : 1u));
    if (other_minimum != moving_minimum && overlaps_other_axes(moving, other, axis)) {
        if (moving_minimum == forward) {
            // 00C4C83C / 00C4CB55: EAX=moving, push other, push storage.
            (void)cc16_dyn_pair_retire::retire_endpoint_pair_00c4c5d0(
                pointer(moving), pointer(storage), pointer(other), crt);
        } else {
            // 00C4C938 / 00C4CA4A: EDI=storage, push other, push moving.
            (void)cc16_dyn_storage::add_endpoint_pair_00c40a30(
                pointer(storage), pointer(moving), pointer(other), crt);
        }
    }
}

void publish_endpoint(U32 table, U32 moving, U32 tag_bit, U32 key,
                      U32 position, U32 index_offset) noexcept {
    half(moving + index_offset) = static_cast<U16>(position);
    const U32 offset = position * 8u;
    const U32 tagged_destination = word(table) + offset + 4u;
    word(tagged_destination) = moving | tag_bit;
    // Native reloads the data word between the tag store and key store.
    const U32 key_destination = word(table) + offset;
    word(key_destination) = key;
}

void order_endpoint(U32 table, U32 storage, U32 endpoint, U32 axis,
                    bool minimum, const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 index_offset = axis * 2u + (minimum ? 0x20u : 0x26u);
    const U32 original = half(endpoint + index_offset);
    const U32 selected = word(table) + original * 8u;
    const U32 tagged = word(selected + 4u);
    const U32 key = word(selected);
    const U32 moving = tagged & 0xfffffffeu;
    const U32 tag_bit = tagged & 1u;
    U32 position = original;
    U32 successor = original + 1u;
    // Forward size checks are signed JGE/JL; key ordering is unsigned.
    while (signed_less(successor, word(table + 4u))) {
        const U32 current = word(table) + position * 8u;
        if (word(current + 8u) >= key) {
            break;
        }
        word(current) = word(current + 8u);
        word(current + 4u) = word(current + 12u);
        const U32 copied_tag = word(word(table) + position * 8u + 4u);
        cross_endpoint(storage, moving, copied_tag, axis, minimum, true, crt);
        ++successor;
        ++position;
    }
    if (position != original) {
        publish_endpoint(table, moving, tag_bit, key, position, index_offset);
        return;
    }
    while (position != 0) {
        const U32 current = word(table) + position * 8u;
        if (word(current - 8u) <= key) {
            break;
        }
        word(current) = word(current - 8u);
        word(current + 4u) = word(current - 4u);
        const U32 copied_tag = word(word(table) + position * 8u + 4u);
        cross_endpoint(storage, moving, copied_tag, axis, minimum, false, crt);
        --position;
    }
    if (position != original) {
        publish_endpoint(table, moving, tag_bit, key, position, index_offset);
    }
}

U32 sort_key(U32 bits) noexcept {
    return ((0u - (bits >> 31u)) | 0x80000000u) ^ bits;
}
}  // namespace

U32 append_axis_endpoints_00c40900(
    void* storage, void* endpoint, U32 axis,
    const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 axis_base = address(storage) + axis * 12u;
    const U32 initial_count = word(axis_base + 8u);
    U32 capacity = word(axis_base + 0x0cu);
    const U32 tagged_endpoint = address(endpoint) | 1u;
    if (initial_count == capacity) {
        grow_axis_entries(axis_base, capacity, crt);
    }
    const U32 first_index = word(axis_base + 8u);
    const U32 first_data = word(axis_base + 4u);
    const U32 first = first_data + first_index * 8u;
    if (first != 0) {
        word(first) = initial_count;
        word(first + 4u) = tagged_endpoint;
    }
    word(axis_base + 8u) = word(axis_base + 8u) + 1u;
    capacity = word(axis_base + 0x0cu);
    if (word(axis_base + 8u) == capacity) {
        grow_axis_entries(axis_base, capacity, crt);
    }
    const U32 second_data = word(axis_base + 4u);
    const U32 second_index = word(axis_base + 8u);
    const U32 second = second_data + second_index * 8u;
    if (second != 0) {
        word(second) = initial_count;
        word(second + 4u) = address(endpoint);
    }
    word(axis_base + 8u) = word(axis_base + 8u) + 1u;
    half(address(endpoint) + axis * 2u + 0x20u) = static_cast<U16>(initial_count);
    const U32 observed_eax = initial_count + 1u;
    half(address(endpoint) + axis * 2u + 0x26u) = static_cast<U16>(observed_eax);
    return observed_eax;
}

void update_axis_endpoints_00c4c700(
    void* storage, void* endpoint, U32 axis,
    const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 endpoint_address = address(endpoint);
    const U32 minimum_source = endpoint_address + axis * 4u + 4u;
    // 00C4C70B occurs before the table/index reads. Keep the value on the
    // ambient x87 stack until the original destination has been computed.
    __asm {
        mov eax, minimum_source
        fld dword ptr [eax]
    }
    const U32 table = address(storage) + axis * 12u + 4u;
    const U32 minimum_index = half(endpoint_address + axis * 2u + 0x20u);
    const U32 minimum_destination = word(table) + minimum_index * 8u;
    __asm {
        mov eax, minimum_destination
        fstp dword ptr [eax]
    }
    const U32 minimum_bits = word(minimum_destination);
    word(minimum_destination) = sort_key(minimum_bits);

    const U32 maximum_index = half(endpoint_address + axis * 2u + 0x26u);
    const U32 maximum_source = endpoint_address + axis * 4u + 0x10u;
    __asm {
        mov eax, maximum_source
        fld dword ptr [eax]
    }
    const U32 maximum_destination = word(table) + maximum_index * 8u;
    __asm {
        mov eax, maximum_destination
        fstp dword ptr [eax]
    }
    const U32 maximum_bits = word(maximum_destination);
    word(maximum_destination) = sort_key(maximum_bits);

    order_endpoint(table, address(storage), endpoint_address, axis, true, crt);
    // Fresh maximum index/entry reads follow all minimum moves and calls.
    order_endpoint(table, address(storage), endpoint_address, axis, false, crt);
}

}  // namespace bsp::cc16_dyn_sap_update
