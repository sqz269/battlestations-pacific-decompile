#pragma once

namespace bsp {

// 00922B10 and 00922C80, the two classifiers that answer `007EEC50`'s
// `target_is_air` and `target_is_surface`. docs/ATTACK_CAPABILITY_INPUTS.md
// Part 2 has the listing and both x87 comparisons.
//
// Both read the entity's `+5Dh` byte first, which docs/BOT_TASKS.md:273 names
// "marks a target no longer engageable". That is live state, not authored, and
// treating it as clear would make dead targets attackable - so it is an explicit
// input here rather than something the rule assumes.

// Everything the two classifiers ask of one entity. The `is_*` fields are
// `IsKindOf` answers; a host holding class ids and the id-to-parent table can
// fill all of them (docs/ATTACK_CAPABILITY_INPUTS.md Part 1 proves the chains
// are table-answerable and non-circular).
struct EntityTargetFacts {
    bool present{false};            // the null check both routines open with
    bool not_engageable{false};     // +5Dh != 0
    bool is_plane{false};           // IsKindOf(0Fh)
    bool is_plane_squadron{false};  // IsKindOf(18h) PlaneSquadronGen
    bool is_ship_family{false};     // IsKindOf(06h)
    bool is_submarine{false};       // IsKindOf(08h)
    bool is_airfield{false};        // IsKindOf(45h)
    bool is_shipyard{false};        // IsKindOf(46h)
    bool is_command_building{false};// IsKindOf(1Ch)
    bool is_dummy_target{false};    // IsKindOf(35h)
    bool is_land_fort{false};       // IsKindOf(1Bh)
    float world_y{0.0f};            // 00427EB0's +4h
    // settings+49Ch `Submarine.SubmarinePeriscopeLevel`, default 20.0.
    float submarine_periscope_level{20.0f};
    // vehicleClass+178h `FakedType`, whose authored default is 1Bh itself.
    // docs/ATTACK_GATE_TAILS.md traced the key and the default.
    int land_fort_faked_type{0x1b};
};

// 00922D08's multiplier, the double 0.25 at 00D7A348.
inline constexpr float kSubmarineSurfaceFraction = 0.25f;
// 00922D77's threshold, the double 50.0 at 00CE3938.
inline constexpr float kDummyTargetSurfaceHeight = 50.0f;

// 00922B10. `!entity ? false : +5Dh ? false : (plane || plane squadron)`.
inline bool entity_is_airborne_00922b10(const EntityTargetFacts& e) {
    if (!e.present) return false;
    if (e.not_engageable) return false;
    return e.is_plane || e.is_plane_squadron;
}

// 00922C80 reaches a call this host cannot make - 008DDF90
// BSP_SzurkeNyil_ContainsUnit, over a set selected through a singleton path that
// was never read. That branch sits AFTER the ship family, airfield, shipyard and
// command-building arms, so it is unreachable for the cases that matter here
// (an aircraft attacking a ship returns true at the ship arm). Rather than
// assume the set is empty, the rule reports when the walk reaches it.
enum class SurfaceTargetAnswer {
    kNo,
    kYes,
    // 008DDF90's arm, and everything after it. The caller has to decide; the
    // native would consult a set this process does not build.
    kUnreadSetBranch,
};

// 00922C80, `__fastcall(entity, bool allow_far)`. The `allow_far` argument is
// not taken here: the native consults it only after the 008DDF90 branch, so it
// belongs to the tail below rather than to this walk.
inline SurfaceTargetAnswer entity_is_surface_target_00922c80(const EntityTargetFacts& e) {
    if (!e.present) return SurfaceTargetAnswer::kNo;
    if (e.not_engageable) return SurfaceTargetAnswer::kNo;
    if (e.is_plane) return SurfaceTargetAnswer::kNo;           // IsKindOf(0Fh)
    if (e.is_plane_squadron) return SurfaceTargetAnswer::kNo;  // IsKindOf(18h)
    if (e.is_ship_family) {
        if (!e.is_submarine) return SurfaceTargetAnswer::kYes;
        // 00922CE8-00922D08: accept iff pos.y > -(periscope * 0.25). With the
        // shipped default of 20.0 a submarine is a surface target above -5.0.
        const float threshold = -(e.submarine_periscope_level * kSubmarineSurfaceFraction);
        return e.world_y > threshold ? SurfaceTargetAnswer::kYes : SurfaceTargetAnswer::kNo;
    }
    if (e.is_airfield) return SurfaceTargetAnswer::kYes;
    if (e.is_shipyard) return SurfaceTargetAnswer::kYes;
    if (e.is_command_building) return SurfaceTargetAnswer::kYes;
    // 008DDF90 and everything below it.
    return SurfaceTargetAnswer::kUnreadSetBranch;
}

// The tail 00922C80 runs when the set does NOT contain the entity, exposed
// separately so a host that later builds that set can complete the walk.
inline bool entity_surface_target_tail_00922c80(const EntityTargetFacts& e, bool allow_far) {
    if (!allow_far) return false;
    if (e.is_dummy_target) return e.world_y > kDummyTargetSurfaceHeight;
    return e.is_land_fort && e.land_fort_faked_type == 0x1b;
}

// ---------------------------------------------------------------------------
// 009229F0 and its tail 00922990 (packet cc7_target_still_attackable).
//
// `src/game_hosts_commands.cpp` calls 009229F0 `target_still_attackable` and
// `docs/ATTACK_CAPABILITY_INPUTS.md` called it "the shared target still attackable
// test". **It is not a liveness test.** The body is 33 instructions and contains no
// `+5Dh` read, no timer and nothing temporal: it is a class test with a LandFort
// `FakedType` fallback. Nothing in it needs live state, so a host with the class-id
// parent table and the authored `FakedType` can answer it outright.

// 00922990, reached by the tail JMP at 00922A2A. A hand-written membership table over
// the faked type - **not** a parent-chain walk, and it disagrees with one: the ship arm
// omits 09h MMothership and 0Ch MLandingShip, which `Entity_IsKindOf(faked, 6)` would
// include. Answering this with the parent table would get those two wrong.
inline bool faked_family_00922990(int faked_type, int kind) {
    if (kind == 0x06) {  // 00922990; the listing tests 8, 7, 0Dh, 0Bh, 0Ah, 0Eh in that order
        return faked_type == 0x07 || faked_type == 0x08 || faked_type == 0x0A ||
               faked_type == 0x0B || faked_type == 0x0D || faked_type == 0x0E;
    }
    if (kind == 0x0F) {  // 009229BC; 13h, 11h, 12h, 10h, 14h, 15h, 16h, 17h - all eight
        return faked_type >= 0x10 && faked_type <= 0x17;
    }
    return false;  // 009229EC, every other kind
}

// 009229F0 __fastcall(entity ECX, int kind EDX).
//   `is_kind`      = entity->vtable[5Ch](kind)   at 009229FC
//   `is_land_fort` = entity->vtable[5Ch](1Bh)    at 00922A0D
//   `faked_type`   = entity[+538h][+178h]        at 00922A1A/00922A20, the authored
//                    `FakedType` key whose producer is BSP_StructureClass_ReadLuaFields
//                    (docs/ATTACK_GATE_TAILS.md), default 1Bh.
inline bool entity_kind_or_faked_009229f0(bool entity_present, bool is_kind,
                                          bool is_land_fort, int faked_type, int kind) {
    if (!entity_present) return false;           // 009229F3 / 009229F8
    if (is_kind) return true;                    // 00922A04
    if (!is_land_fort) return false;             // 00922A18
    return faked_family_00922990(faked_type, kind);
}

// 007AC9D0 BSP_Entity_PathInterfaceForKind(entity), the sibling at the non-torpedo
// branch of the same call site. Not a predicate: it selects a sub-object by class and
// returns its address, or null. Returns the byte offset, or -1 for null.
inline int path_interface_offset_007ac9d0(bool entity_present, int class_id_matches_47,
                                          int class_id_matches_48, int class_id_matches_49,
                                          int class_id_matches_4a) {
    if (!entity_present) return -1;              // 007AC9D5
    if (class_id_matches_47) return 0x1E4;       // 007AC9E4
    if (class_id_matches_48) return 0x170;       // 007AC9FB
    if (class_id_matches_49) return 0x310;       // 007ACA12
    if (class_id_matches_4a) return 0x1E4;       // 007ACA27 jumps back to 007AC9E4
    return -1;                                   // 007ACA29
}

}  // namespace bsp
