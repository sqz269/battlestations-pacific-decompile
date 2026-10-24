#include "bsp/pool_pool02_cc13.hpp"

#include <cfloat>
#include <cstddef>

// Packet cc13-pool02; evidence, original ABI and uncertainty per routine are in the header.
namespace bsp::pool_pool02_cc13 {
namespace {

template <class T>
T& at(Object object, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}

Object add(Object object, std::size_t off) noexcept { return static_cast<std::uint8_t*>(object) + off; }

// The world position +0FCh..+104h, refreshed (00414DB0) while the pose byte +0C8h is clear.
const float* refreshed_position(Object entity, Pool02Host& host) {
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    return &at<float>(entity, 0xFC);
}

}  // namespace

const char* bomb_class_get_name_006ea290() noexcept { return "MBomb"; }  // 00CFA4D4

bool bomb_class_is_kind_of_006ea2a0(std::int32_t kind) noexcept { return kind == 0x2A || kind == 0x29; }

void cargo_detach_from_world_lists_006eb460(Object cargo, Pool02Host& host) {
    host.unit_detach_from_mid_world_lists_006dffc0(cargo);
    Object const owner = at<Object>(cargo, 0x30);
    for (Object node = at<Object>(owner, 0xA0); node != nullptr; node = at<Object>(node, 4)) {
        if (at<Object>(node, 8) == cargo) {
            host.unit_list_erase_node_004837d0(add(owner, 0x9C), node);
            return;
        }
    }
}

void command_building_send_neutralize_006f2940(Object building, Pool02Host& host) {
    at<float>(building, 0x7A8) = 0.0f;
    if (at<std::int32_t>(building, 0x54) == 2) return;
    alignas(4) std::uint8_t message[0x24]{};
    host.session_message_construct_base_0075b430(message, 0xD3);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x00) = 0x00CFADBCu;
    at<std::int32_t>(message, 0x1C) = 9;
    at<std::uint8_t>(message, 0x20) = 0;
    host.session_route_message_0077c2a0(building, message, 7, 0);
}

void command_building_handle_message_d5_006f29d0(Object building, float progress, std::uint32_t value,
                                                 std::int32_t side, Pool02Host& host) {
    Object const game = host.game_00e188a8();
    Object const local = at<Object>(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC));
    if (side == at<std::int32_t>(local, 0x28)) {
        const float old_progress = at<float>(building, 0x7A8);
        // old == 0 (ordered), new != 0 or unordered, as the two LAHF parity tests read.
        if (old_progress == 0.0f && !(progress == 0.0f) && at<std::int32_t>(building, 0x54) == 2)
            host.warning_report_capture_ship_00977da0(building, 0);
    }
    at<std::int32_t>(building, 0x7AC) = side;
    at<float>(building, 0x7A8) = progress;
    at<std::uint32_t>(building, 0x7A4) = value;
}

Object command_building_take_landing_pad_006f2a50(Object site, Object ship, Pool02Host& host) {
    if (at<std::int32_t>(site, 0x798) == 0) return nullptr;
    host.enter_critical_section(at<Object>(site, 0x764));
    ++at<std::int32_t>(at<Object>(site, 0x764), 0x18);
    float best = FLT_MAX;  // 00D7A248
    Object chosen = nullptr;
    const float* const source = ship != nullptr ? refreshed_position(ship, host) : host.static_vector_00f87574();
    const float ref[3] = {source[0], source[1], source[2]};
    for (std::int32_t i = 0; i < at<std::int32_t>(site, 0x798); ++i) {
        Object const pad = at<Object*>(site, 0x794)[i];
        if (host.landing_pad_occupant_006ac220(pad) != nullptr) continue;
        if (ship == nullptr) {
            chosen = pad;
            break;
        }
        const float* const p = refreshed_position(pad, host);
        const float dx = p[0] - ref[0];
        const float dy = p[1] - ref[1];
        const float dz = p[2] - ref[2];
        const float xx = dx * dx;
        const float yy = dy * dy;
        const float xy = yy + xx;
        const float zz = dz * dz;
        const float dist = xy + zz;
        if (best > dist) {
            best = dist;
            chosen = pad;
        }
    }
    if (chosen != nullptr) host.landing_pad_set_occupant_006ac490(chosen, ship);
    --at<std::int32_t>(at<Object>(site, 0x764), 0x18);
    host.leave_critical_section(at<Object>(site, 0x764));
    return chosen;
}

bool land_fort_shows_on_minimap_006f57d0() noexcept { return false; }

std::int32_t command_class_base_slot10_006f7f50() noexcept { return 0; }

const char* command_class_settarget_name_006f8010() noexcept { return "settarget"; }  // 00CF8B0C

}  // namespace bsp::pool_pool02_cc13
