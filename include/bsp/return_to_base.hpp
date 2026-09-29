#pragma once
// Packet cc9_squadron_land_task. The squadron's `returntobase` resolution:
// 007F16D0 BSP_Plane_ResolveReturnToBase (ECX = the squadron, stack: the
// command record out, RET 4) and its nearest-site search 006C0840
// (__fastcall: ECX = side, EDX = the member-array head plane; stack: distance
// out pointer, the head's class test 0047B850, local-only byte; RET 0Ch).
// docs/CONTROLLED_UNIT.md, "The squadron's returntobase resolution, recorded".
// New C++ interfaces over plain inputs; not ABI replacements.
#include <cstdint>
#include <vector>

namespace bsp {

// One node of the air-ops list at 00E19948 (next at node+BCh), in list order.
// node+4 is the air-ops block; block+7Ch its owner entity.
struct LandingSiteCandidate {
    std::uintptr_t node{0};
    bool block_present{false};     // node+4 != 0 (006C0990)
    bool owner_present{false};     // block+7Ch != 0 (006C099E)
    bool owner_dead_5e{false};     // owner+5Eh (006C09A6)
    bool owner_remote_5d{false};   // owner+5Dh (006C09C1)
    bool approach_bit_20{false};   // block+20h bit 1 (006C09D2..006C09D7)
    std::int32_t owner_side_54{0}; // owner+54h (006C09E3)
    // 006BC530(head position, 0) at 006C0A27. contract: unread body.
    bool accepts_6bc530{false};
    // The ordering key 006C0A34..006C0A9B builds (438B10 and 42BE90 for an
    // accepting block, 427E30 of 006BCC90's block-local offset otherwise).
    // The helpers are unread: `key_known` false means the host has no value.
    bool key_known{false};
    float key{0.0f};
};

// The head plane's own site, 00923810(1) = its scene parent +3Ch.
struct HeadOwnSite {
    bool present{false};
    bool is_airfield_45{false};    // vtable[5Ch](45h)
    bool is_carrier_09{false};     // vtable[5Ch](9)
    bool dead_5e{false};
    bool parent_present{false};    // its own 00923810(1)
    bool parent_dead_5e{false};
    std::int32_t side_54{0};
    std::uintptr_t airfield_block_7ac{0};   // site+7ACh
    std::uintptr_t carrier_block_1208{0};   // site+1208h
};

struct NearestLandingSiteInputs {
    std::int32_t side{0};            // ECX, the squadron's +54h
    bool head_present{false};        // EDX != 0
    bool head_control_9d4{false};    // [head+9D4h] != 0
    bool multiplayer_927c90{false};  // 00927C90(0) != 0
    HeadOwnSite own;
    bool need_approach_bit{false};   // stack 2: 0047B850(head)
    bool local_only{false};          // stack 3
};

struct NearestLandingSiteResult {
    std::uintptr_t node{0};          // EAX
    bool from_own_site{false};
    int candidates_passed{0};        // nodes past the filters
    bool key_unknown{false};         // two or more passed and a key was missing
    // The winner's key and class (006C0ACF-006C0ADB). 006C0AFF-006C0B2A turn them
    // into the stack-1 output: 0.0 for an accepting winner, else sqrt(key).
    float best_key{-1.0f};
    bool best_accepts{false};
};

// 006C0840. The walk keeps the best node: an accepting (006BC530) node beats a
// non-accepting one, and within a class the smaller key wins (006C0AA3..
// 006C0ADB). With a single node past the filters the key never decides.
NearestLandingSiteResult nearest_landing_site_006c0840(const NearestLandingSiteInputs& in,
    const std::vector<LandingSiteCandidate>& list);

// 007F16D0's outcome.
enum class ReturnToBaseArm : int {
    kNull = 0,           // 007F18EB: record [0] = 0, the zero position
    kLandAtHome = 1,     // 007F176E: 007F1000(land 00E08FA0, sq+404h)
    kLandAtSite = 2,     // 007F17D2: 007EF8B0(land, 00465080(block+7Ch, 0.0))
    kRetreat = 3,        // 007F1896: retreat 00E08F90 toward the zone's centre
};

struct ReturnToBaseInputs {
    bool class_descriptor_35c{false};     // sq+35Ch != 0
    float min_water_spd_198{0.0f};        // [sq+35Ch]+198h, compared == 0.0 (00D7A218)
    bool head_present{false};             // sq+3D0h != 0
    bool head_is_kamikaze_17{false};      // head vtable[5Ch](17h)
    bool head_c24{false};                 // head+C24h
    bool has_deck_369{false};             // sq+369h
    // 006BCD20(sq+404h, 1), 006C4790(sq) and 006BED30 (unread bodies): true when
    // the home arm would issue.
    bool home_arm_issues{false};
    // 006C0840's node, and the two reads 007F17A1..007F17AF make of it.
    std::uintptr_t site_node{0};
    bool site_block_present{false};       // node+4
    bool site_block_refuses_6bc120{true}; // 006BC120(block): owner null or +5Dh set
    // 004C7730(sq+FCh, sq+54h, 0, 0).
    bool zone_found{false};
    float zone_corners_xz[8]{};           // +10h,+18h,+1Ch,+24h,+28h,+30h,+34h,+3Ch
};

struct ReturnToBaseResult {
    ReturnToBaseArm arm{ReturnToBaseArm::kNull};
    // kRetreat: the record's +0Ch/+14h, the four corners summed from 0.0 in
    // listing order and times 0.25 (00D7A348), y (+10h) = 0.
    float retreat_x{0.0f};
    float retreat_z{0.0f};
};

ReturnToBaseResult resolve_return_to_base_007f16d0(const ReturnToBaseInputs& in);

}  // namespace bsp
