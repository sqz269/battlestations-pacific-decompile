#include "bsp/native_plane_squadron_insert.hpp"

#include <limits>
#include <stdexcept>

namespace bsp {

static_assert(sizeof(void*) == 4, "native squadron pointer slots require Win32");

NativePlaneSquadronInsertView native_plane_squadron_insert_view(
    const void* squadron, std::size_t backing_bytes,
    volatile std::int32_t& count, std::array<const void*, 5>& members,
    volatile std::uint8_t& dirty) {
    const auto root = reinterpret_cast<std::uintptr_t>(squadron);
    if (squadron == nullptr || backing_bytes < 0x3ed ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x3ed ||
        root % alignof(void*) != 0 ||
        reinterpret_cast<std::uintptr_t>(&count) != root + 0x3cc ||
        reinterpret_cast<std::uintptr_t>(members.data()) != root + 0x3d0 ||
        reinterpret_cast<std::uintptr_t>(&dirty) != root + 0x3ec) {
        throw std::logic_error("native insert requires live SAME squadron cells");
    }
    return {squadron, count, members.data(), dirty};
}

NativePlaneSquadronInsertPlaneView native_plane_squadron_insert_plane_view(
    const void* plane, std::size_t backing_bytes,
    const void* volatile& squadron, volatile std::int32_t& index) {
    const auto root = reinterpret_cast<std::uintptr_t>(plane);
    if (plane == nullptr || backing_bytes < 0x9dc ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x9dc ||
        root % alignof(void*) != 0 ||
        reinterpret_cast<std::uintptr_t>(&squadron) != root + 0x9d4 ||
        reinterpret_cast<std::uintptr_t>(&index) != root + 0x9d8) {
        throw std::logic_error("native insert requires live SAME plane cells");
    }
    return {plane, squadron, index};
}

void native_plane_squadron_insert_sorted_007ed0d0(
    const NativePlaneSquadronInsertView& squadron, const void* incoming_plane,
    std::int32_t requested_index, NativePlaneSquadronInsertViews& views) {
    squadron.dirty_3ec = 1; // 007ED0D7, also the null branch's sole effect.
    if (incoming_plane == nullptr) {
        return;
    }
    const auto incoming = views.insert_plane_view(incoming_plane); // PURE alias
    if (incoming.actual_plane != incoming_plane) {
        throw std::logic_error("native insert mapping changed incoming identity");
    }
    incoming.squadron_9d4 = squadron.actual_squadron; // 007ED0E6 before index
    incoming.index_9d8 = requested_index; // 007ED0EC before count capture
    const std::int32_t count = squadron.count_3cc; // 007ED0F2, captured once
    std::int32_t position = 0;
    while (position < count) {
        const void* current = squadron.members_3d0[position];
        const auto member = views.plane_leader_view(current); // PURE alias
        if (member.actual_plane != current) {
            throw std::logic_error("native insert mapping changed member identity");
        }
        if (member.field_9d8 > requested_index) { // 007ED107/10D, FRESH signed
            break;
        }
        ++position;
    }
    for (std::int32_t destination = count; destination > position; --destination) {
        squadron.members_3d0[destination] = squadron.members_3d0[destination - 1];
    }
    squadron.members_3d0[position] = incoming_plane; // 007ED13D
    ++squadron.count_3cc; // 007ED144: reload actual count, not captured count+1
}

} // namespace bsp
