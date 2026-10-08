#include "bsp/native_plane_squadron_promote.hpp"

#include "bsp/native_plane_squadron_reindex.hpp"
#include "bsp/native_session_message_be.hpp"
#include <limits>
#include <stdexcept>

namespace bsp {

static_assert(sizeof(void*) == 4, "native squadron pointer cells require Win32");

NativePlaneSquadronPromotionView native_plane_squadron_promotion_view(
    void* squadron, std::size_t backing_bytes, volatile std::int32_t& count,
    std::array<const void*, 5>& members) {
    const auto root = reinterpret_cast<std::uintptr_t>(squadron);
    if (squadron == nullptr || backing_bytes < 0x3e4 ||
        root > (std::numeric_limits<std::uintptr_t>::max)() - 0x3e4 ||
        root % alignof(void*) != 0 ||
        reinterpret_cast<std::uintptr_t>(&count) != root + 0x3cc ||
        reinterpret_cast<std::uintptr_t>(members.data()) != root + 0x3d0) {
        throw std::logic_error("native promotion requires live SAME squadron cells");
    }
    return {squadron, count, members.data()};
}

void native_plane_squadron_promote_007ed610(
    const NativePlaneSquadronPromotionView& squadron, std::int32_t index) {
    if (index <= 0) return; // 007ED614/616, no count observation yet.
    if (squadron.count_3cc <= index) return; // 007ED618/61E, signed.
    const void* const selected = squadron.members_3d0[index]; // 007ED621
    for (; index > 0; --index) {
        squadron.members_3d0[index] = squadron.members_3d0[index - 1];
    }
    squadron.members_3d0[0] = selected; // 007ED63F before raw publications.
    native_plane_squadron_reindex_007ed260(squadron.actual_squadron, nullptr);
}

bool native_plane_squadron_receive_leader_promotion_be_007f0077(
    const NativePlaneSquadronPromotionView& squadron,
    const volatile NativeSessionMessageBE& message) {
    native_plane_squadron_promote_007ed610(squadron, message.member_index_1c);
    return true; // Native AL=1 at007F0083, including rejected index.
}

} // namespace bsp
