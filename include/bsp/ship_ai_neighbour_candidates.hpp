#pragma once

#include "bsp/gameplay_settings.hpp"
#include <cstdint>

namespace bsp {

// Borrow fields of one canonical unit; no pose, class, candidate or list owner
// is created. The identities below remain opaque and must preserve equality.
struct ShipAiNeighbourUnitFields {
    const volatile std::uint8_t& world_valid_c8;
    const volatile float& world_x_fc;
    const volatile float& world_y_100;
    const volatile float& world_z_104;
    const volatile float& field_9c8;
};

class ShipAiNeighbourCandidateHost {
public:
    virtual ~ShipAiNeighbourCandidateHost() = default;
    // These are pure lookups of live borrowed fields, with no allocation,
    // defaulting, mutation, pose refresh or snapshot of the whole collection.
    virtual const void* current_self_aa8() const = 0;
    virtual ShipAiNeighbourUnitFields unit_fields(const void* unit) const = 0;
    virtual const void* unit_class_538(const void* unit) const = 0;
    virtual const volatile float& class_field_500(const void* captured_class) const = 0;
    virtual const void* world_list6_head_64() const = 0;
    virtual std::int32_t world_list6_count_60() const = 0;
    virtual const void* node_payload_08(const void* node) const = 0;
    virtual const void* node_next_04(const void* node) const = 0;

    // Real native call boundaries, not optional notifications. Refresh must
    // act on the same owner's actual pose. Settings may change between calls.
    virtual void refresh_pose_00414db0(const void* unit) = 0;
    virtual const GameplayTuningSettings& settings_00424c40() = 0;
    // 009F1A25: native ECX=brain+8 (nav), stack candidate, RET4; result ignored.
    // Bind the existing nav owner and the admission implementation here.
    virtual void admit_009f0d20(const void* candidate_identity) = 0;
};

// PARTIAL 009F1420: second timer's operations at009F15CC..15F5 and15F8..15FD.
// Borrow actual B50/B4C. The constructor randomizes both through stream1; no
// replacement defaults or extra draws are supplied. Does not run the first
// timer, preceding goal update, shared lock, candidate walk or later tails.
bool ship_ai_neighbour_timer_due_009f15cc(volatile float& countdown_b50,
    const volatile float& period_b4c, float step) noexcept;

// PARTIAL 009F1420: complete gated list6 candidate fragment009F1856..009F1A43.
// Entry due is the original local byte at ESP+17. Caller must already have the
// native parent lock/lifetime protection (brain+4, when nonnull), and retain it
// through the original later tails. This fragment does not acquire/release it.
// Captures signed count/head once; always reads each node's live next AFTER
// candidate callbacks, including the final iteration. No null/active/id filter.
// New C++ ABI, not a standalone original function or complete brain pre-pass.
void ship_ai_walk_neighbour_candidates_009f1856(bool due,
    ShipAiNeighbourCandidateHost&);

} // namespace bsp
