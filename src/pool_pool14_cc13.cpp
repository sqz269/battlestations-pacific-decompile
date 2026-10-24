#include "bsp/pool_pool14_cc13.hpp"

#include "bsp/ship_ai_avoidance_request.hpp"

#include <cstddef>

// Packet cc13-pool14; evidence, original ABI and uncertainty per routine are in the header.
namespace bsp::pool_pool14_cc13 {
namespace {

template <class T>
T& at(Object object, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}

Object add(Object object, std::size_t off) noexcept { return static_cast<std::uint8_t*>(object) + off; }

// An observer handle: {vtable, +4h, +8h, +0Ch = 0, +10h byte 1, +14h observed = 0}.
void observer_handle_init(Object handle, std::uint32_t vtable) noexcept {
    at<std::uint32_t>(handle, 0x4) = 0;
    at<std::uint32_t>(handle, 0x8) = 0;
    at<std::uint32_t>(handle, 0xC) = 0;
    at<Object>(handle, 0x14) = nullptr;
    at<std::uint8_t>(handle, 0x10) = 1;
    at<std::uint32_t>(handle, 0x0) = vtable;
}

// The inlined observer assignment: unchanged does nothing; else unregister the old one,
// store, register the new one when it is set.
void observer_handle_assign(Object handle, Object observed, Pool14Host& host) {
    Object const old = at<Object>(handle, 0x14);
    if (old == observed) return;
    if (old != nullptr) host.observer_unregister_pair_006952a0(old, handle);
    at<Object>(handle, 0x14) = observed;
    if (observed != nullptr) host.observer_register_pair_00694a60(observed, handle);
}

}  // namespace

void dive_bomb_done_prepare_enter_009c7240(Object state, Pool14Host& host) {
    at<std::uint8_t>(state, 0x9C) = 0;
    at<float>(state, 0x98) = -1.0f;  // 00D7A260
    host.bot_state_follow_enter_009bed80(state);
}

void dive_bomb_done_prepare_tick_009c7270(Object state, float dt, Pool14Host& host) {
    host.bot_state_follow_tick_009c1fd0(state, dt);
}

ShipAiPathPlanBlock& ship_ai_path_plan_construct_009d9cc0(ShipAiPathPlanBlock& plan) noexcept {
    plan.revalidate_delay = 0.5f;           // 00CE3800
    plan.field_18 = 0.0f;
    plan.search_context = 0;                // +2Ch, stored as the float 0.0
    plan.capture_radius_sq = 1600.0f;       // 00D05A40
    plan.published_width = 20.0f;           // 00CE3930
    plan.field_04 = 20.0f;
    plan.vtable = kShipAiPathPlanVtable;    // 00D2152C
    plan.goal_node = nullptr;
    plan.head = nullptr;
    plan.search_state = 0;
    plan.node_count = 0;
    plan.owner = nullptr;
    plan.goal_drift_sq = 2500.0f;           // 00D20278
    plan.zone_layer = 0;
    plan.field_48 = 0;
    plan.goal_clearance = 1.0e7f;           // 00CF58EC
    return plan;
}
static_assert(kShipAiPathPublishedWidth == 20.0f && kShipAiPathCaptureRadiusSq == 1600.0f &&
              kShipAiPathGoalDriftSq == 2500.0f && kShipAiPathRevalidateInitial == 0.5f &&
              kShipAiPathNoClearance == 1.0e7f);

Object ship_ai_contact_track_construct_009eaca0(Object track, float a0, float a1, float a2, Object observed,
                                                float a4, float a5, std::uint8_t kind, Pool14Host& host) {
    Object const handle_a = add(track, 0x34);
    Object const handle_b = add(track, 0x4C);
    observer_handle_init(handle_a, 0x00D21868u);
    observer_handle_init(handle_b, 0x00CF5C94u);
    at<float>(track, 0x0) = a0;
    at<float>(track, 0xC) = a2;
    at<float>(track, 0x4) = static_cast<float>(static_cast<double>(a1) * 0.5);  // 00D7A280
    at<float>(track, 0x8) = a1;
    at<float>(track, 0x1C) = a5;
    at<std::uint8_t>(track, 0x64) = 0;
    at<float>(track, 0x10) = a4;
    Object const settings = host.game_settings_00424c40();
    const float base = at<float>(settings, kind != 0 ? 0x1F0 : 0x194);
    at<float>(track, 0x14) = static_cast<float>(static_cast<double>(base) + 1.0);  // 00D7A210
    observer_handle_assign(kind != 0 ? handle_a : handle_b, observed, host);
    host.ship_ai_refresh_contact_track_009dc060(track);
    return track;
}

bool ship_ai_obstacle_side_accepted_009ec770(Object blk, int other_party, Pool14Host& host) {
    const int party_filter = at<std::int32_t>(blk, 0x3F0);
    if (party_filter < 0) return false;
    if (at<std::uint8_t>(host.unit_weapon_director_0080e160(at<Object>(blk, 0x3FC)), 0x241) == 0) return false;
    if (at<std::uint8_t>(host.game_settings_00424c40(), 0x4) == 0) return false;
    return ship_ai_avoidance_party_accepted_009ec770(party_filter, other_party, true, true);
}

}  // namespace bsp::pool_pool14_cc13
