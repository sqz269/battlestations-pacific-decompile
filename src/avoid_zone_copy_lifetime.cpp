#include "bsp/avoid_zone_copy_lifetime.hpp"

#include <cstdint>
#include <cstring>
#include <limits>

namespace bsp {
namespace {
std::uint32_t count_bits(const AvoidZoneNativeStorage& zone) noexcept {
    std::uint32_t result;
    std::memcpy(&result, &zone.corners.count, sizeof(result));
    return result;
}
void set_count_bits(AvoidZoneNativeStorage& zone, std::uint32_t count) noexcept {
    std::memcpy(&zone.corners.count, &count, sizeof(count));
}
void copy_float_words(const void* source, void* destination,
    std::uint32_t words) noexcept {
    // Only called with 1, 3 or 9 words; no arithmetic or default construction.
    // Native bounds/record stores are x87 FLD/FSTP, not bitwise float copies.
    __asm {
        mov eax, source
        mov edx, destination
        mov ecx, words
    next_word:
        fld dword ptr [eax]
        fstp dword ptr [edx]
        add eax, 4
        add edx, 4
        dec ecx
        jnz next_word
    }
}
void free_pointer_buffer(AvoidZoneNativeStorage& zone,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    if (zone.corners.records) {
        allocation.free_array_00bf6989(allocation.context, zone.corners.records);
        zone.corners.records = nullptr;
    }
}
void reserve_slots(AvoidZoneNativeStorage& zone, std::uint32_t requested,
    const AvoidZoneAllocationAccess& allocation) {
    if (zone.capacity >= requested) return;
    zone.capacity = requested; // Native store precedes allocation, including throw.
    const auto bytes = requested > 0x3fffffffu
        ? std::numeric_limits<std::uint32_t>::max() : requested * 4u;
    auto** replacement = static_cast<ShipAiPathLateralRecord**>(
        allocation.allocate_array_00bf55be(allocation.context, bytes));
    if (zone.corners.records) {
        const auto* old_bytes = reinterpret_cast<const unsigned char*>(zone.corners.records);
        auto* new_bytes = reinterpret_cast<unsigned char*>(replacement);
        for (std::uint32_t i = 0; i < count_bits(zone); ++i) {
            // Native MOV words can include untouched allocation bytes. Do not
            // evaluate an indeterminate prefix word as a C++ pointer value.
            std::memcpy(new_bytes + i * 4u, old_bytes + i * 4u, 4u);
        }
        allocation.free_array_00bf6989(allocation.context, zone.corners.records);
    }
    zone.corners.records = replacement;
}
} // namespace

AvoidZoneNativeStorage& avoid_zone_copy_construct_0041a030(
    AvoidZoneNativeStorage& destination, const AvoidZoneNativeStorage& source,
    const AvoidZoneAllocationAccess& allocation) {
    destination.corners.records = nullptr;
    destination.corners.count = 0;
    destination.capacity = 0;
    std::memcpy(&destination.associated_entity, &source.associated_entity, 4u);
    destination.layer = source.layer;
    copy_float_words(&source.min_x, &destination.min_x, 1); // 0041A06A/6D.
    try { // 0041A070 sets EH state 0 after the first bound store.
        copy_float_words(&source.min_z, &destination.min_z, 3); // 0041A074..83.
        const auto original_count = count_bits(source);
        reserve_slots(destination, original_count, allocation); // 0041A0A4.
        set_count_bits(destination, original_count); // 0041A0E1: NOT zero.
        for (std::uint32_t i = 0; i < original_count; ++i) {
            auto* clone = static_cast<ShipAiPathLateralRecord*>(
                allocation.allocate_record_00bf681b(allocation.context, 0x24u));
            if (clone) copy_float_words(source.corners.records[i], clone, 9);
            if (count_bits(destination) == destination.capacity) {
                const auto grown = destination.capacity * 2u + 2u; // Native wrap.
                reserve_slots(destination, grown, allocation); // 0041A16C.
            }
            destination.corners.records[count_bits(destination)] = clone;
            set_count_bits(destination, count_bits(destination) + 1u);
        }
    } catch (...) {
        // D84218 state0 -> D84210 -> C5E130 -> 00412DC0. Deliberately no
        // record destruction, count/capacity reset or just-created-clone free.
        free_pointer_buffer(destination, allocation);
        throw;
    }
    return destination;
}

void avoid_zone_destroy_owned_storage_0041a1e0(AvoidZoneNativeStorage& zone,
    const AvoidZoneAllocationAccess& allocation) noexcept {
    avoid_zone_clear_records_004167c0(zone, allocation); // 0041A1E3.
    free_pointer_buffer(zone, allocation); // 0041A1EF; preserves capacity/metadata.
}
} // namespace bsp
