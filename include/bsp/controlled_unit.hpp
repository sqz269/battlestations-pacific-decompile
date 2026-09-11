#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/unit_instance.hpp" // ControlledUnitQuery, ControlledUnitBind, kUnitTrait*

// Making a unit the player's controlled unit, docs/CONTROLLED_UNIT.md.
//
// Two routines, not one. 004C0890 is small: it writes the global, resolves which object
// is actually being driven, and republishes one handle into the render-resources
// singleton. Everything a player would call "taking control" - leaving the old unit,
// the eligibility test, the observer pair, the two audio broadcasts - lives in its
// caller 00645600. The packet brief named 004C4300 (input contexts) and 00651760 (the
// HUD unit view) as part of this path; neither is reachable from either routine. See
// the Corrections section of docs/CONTROLLED_UNIT.md.
//
// The resolution rule itself is already reconstructed as resolve_controlled_unit_004c0890
// in bsp/unit_instance.hpp and is reused rather than repeated. What this header adds is
// the *sequence*: the global writes, the listener publish, and the caller's ordering.
//
// Descriptive names are hypotheses, not recovered symbols. Nothing here is a drop-in
// binary replacement.

namespace bsp {

// ---------------------------------------------------------------------------
// Globals, from 004C0890's own accesses.
// ---------------------------------------------------------------------------

// The controlled unit. 004C0893 is the only WRITE in the image's xref set; every other
// reference reads it. 004E4A80 inlines the whole body behind
// `DAT_00E188DC == 0 && DAT_00E188D8 != 0` (docs/GAME_ON_MOVE_MAP.md step 5).
inline constexpr std::uint32_t kGameControlledUnitAddress = 0x00E188D8u; // 004C0893

// The handle the driven object publishes. Written at 004C0900 and cleared at 004C0914.
inline constexpr std::uint32_t kGameControlledListenerAddress = 0x00E188DCu;

// The object the handle is pushed into: the render-resources singleton published by
// 00B0F076 (docs/APP_INIT_RENDERER.md). 00B0D7B0 stores at +1C0h and then kicks +30h.
inline constexpr std::uint32_t kRenderResourcesGlobalAddress = 0x00F8D39Cu; // 004C08F9
inline constexpr std::size_t kRenderResourcesListenerSlot = 0x1C0;          // 00B0D7B4

// Vtable slot 18h on the driven object; its return value becomes the published handle.
inline constexpr std::size_t kUnitVtableListenerHandleSlot = 0x18; // 004C08F2

// ---------------------------------------------------------------------------
// 004C0890, the setter itself.
// ---------------------------------------------------------------------------

// The two globals, so a caller can observe the writes without a process image.
struct ControlledUnitGlobals {
    bool unit_present{false};    // DAT_00E188D8 != 0 after the call
    bool listener_present{false}; // DAT_00E188DC != 0 after the call
};

// One method per native call site 004C0890 makes, in call order. The IsKindOf probes
// are supplied through ControlledUnitQuery (bsp/unit_instance.hpp) because the
// resolution rule is already reconstructed against it.
struct SetControlledUnitHost {
    virtual ~SetControlledUnitHost() = default;

    // 004C0893: the unconditional store, before any test. A null unit is stored too.
    virtual void store_controlled_unit(bool unit_present) = 0;

    // 004C08F7: vtable +18h on the resolved driven object. Its result is the handle.
    // Reached only when that object answers IsKindOf(0Fh) or IsKindOf(18h).
    virtual bool driven_listener_handle() = 0;

    // 004C0900 / 004C0914: DAT_00E188DC = handle, then 00B0D7B0 with ECX
    // DAT_00F8D39C pushes the same value into the singleton at +1C0h. Both the
    // publish and the clear go through the same two stores, so one method carries
    // the value that differs between them (rule 3 of the verification checklist).
    virtual void publish_listener(bool handle_present) = 0;
};

// 004C0890, __thiscall(unit), RET 0, body 004C0890..004C0924. No stack arguments.
//
// DAT_00E188D8 = unit unconditionally; then
//   target = unit->IsKindOf(5)    ? unit
//          : unit->IsKindOf(18h)  ? [unit+3D0h]
//          : null;
//   if (target && (target->IsKindOf(0Fh) || target->IsKindOf(18h)))
//        publish(target->vtable[18h]());
//   else publish(0);
//
// The zero path is not an error path: three of the five call sites reach 004C0890 with
// ECX already zero (007F3AF2 and 009595FE zero it explicitly, 00645600 zeroes ESI when
// the candidate fails the eligibility test), so clearing the controlled unit is the
// routine's second contract.
ControlledUnitGlobals set_controlled_unit_004c0890(bool unit_present,
                                                   ControlledUnitQuery& query,
                                                   SetControlledUnitHost& host);

// ---------------------------------------------------------------------------
// 00645060, the eligibility test 00645600 runs first.
// ---------------------------------------------------------------------------

// Trait ids the test queries beyond the three 004C0890 uses. Ids 2Ah, 46h and 45h are
// *rejections*: a unit answering any of them is never selectable.
inline constexpr int kUnitTraitSelectableBase = 0x02;   // 006450CA, required
inline constexpr int kUnitTraitRejectedA = 0x2A;        // 006450E6
inline constexpr int kUnitTraitRejectedB = 0x46;        // 006450F5
inline constexpr int kUnitTraitRejectedC = 0x45;        // 00645104

// Unit bytes the test reads, all as plain non-zero tests.
inline constexpr std::size_t kUnitOffAliveFlag = 0x5C;      // 00645091, must be set
inline constexpr std::size_t kUnitOffOutOfAction = 0x5D;    // 0064509B, must be clear
inline constexpr std::size_t kUnitOffRejectFlagE = 0x5E;    // 006450AF, must be clear
inline constexpr std::size_t kUnitOffRejectFlag60 = 0x60;   // 006450A5, must be clear
inline constexpr std::size_t kUnitOffTeamPointer = 0x54;    // 006450B9
inline constexpr std::size_t kUnitOffSpectateKind = 0x188;  // 00645133
inline constexpr std::size_t kTeamOffOwnerPointer = 0x28;   // 006450BC
inline constexpr std::size_t kTeamOffLocalFlag = 0x19;      // 0064512E
inline constexpr std::size_t kGameOffTeamTable = 0x18CC;    // 00645084, stride 4
inline constexpr std::size_t kGameOffSpectateOverride = 0x2194; // 00645071

// 00645133 compares the spectate kind against 8 and keeps the unit only when it differs.
inline constexpr int kUnitSpectateKindBlocked = 8; // 00645133

// The readable half of the test. Everything here is a plain flag or pointer compare in
// the listing; the two virtual probes (vtable +5Ch and vtable +124h) and the team query
// 00927C50 are the host's job because their bodies were not read for this packet.
struct UnitSelectableInputs {
    bool alive_5c{false};         // unit+5Ch
    bool out_of_action_5d{false}; // unit+5Dh
    bool reject_5e{false};        // unit+5Eh
    bool reject_60{false};        // unit+60h
    bool team_matches_owner{false}; // [unit+54h] == [team+28h]
    bool is_kind_2{false};
    bool is_kind_0f{false};
    bool is_kind_2a{false};
    bool is_kind_46{false};
    bool is_kind_45{false};
    bool vtable_124_allows{false}; // unit->vtable[124h]() at 00645110
    bool team_query_00927c50{false}; // 00645121, with the team index in EDI
    bool spectate_allowed{false};  // BL: the caller's flag, or 0 when game+2194h is set
    bool team_is_local{false};     // [team+19h]
    int spectate_kind{0};          // [unit+188h]
};

// 00645060, bool __fastcall(unit = ECX, teamIndex = EDX, bool allowSpectate = [ESP+4]),
// RET 4, body 00645060..00645159. Complete: every branch of the body is covered.
bool unit_is_selectable_00645060(const UnitSelectableInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 00645600, the HUD root's "control this unit" sequence.
// ---------------------------------------------------------------------------

// The unit-side broadcasts the sequence makes. Both take a bool and both turn it into
// the float `flag ? 0.0f : 1.0f` before handing it on (00817380 at 008173A4, 00954990
// at 009549BB), so the *controlled* unit receives 0.0f and a released one 1.0f.
inline constexpr std::size_t kUnitOffPartArray = 0xFFC;      // 00817386, begin
inline constexpr std::size_t kUnitOffPartCount = 0x1000;     // 00817392, element count
inline constexpr std::size_t kUnitOffPartBroadcastFlag = 0x1008; // 0081738C
inline constexpr std::size_t kUnitOffChildListHead = 0x428;  // 009549DF, link at +4
inline constexpr std::size_t kUnitOffControlledAudioFlag = 0x6B4; // 00954A00

// One method per native call site 00645600 makes, in call order.
struct SelectControlledUnitHost {
    virtual ~SelectControlledUnitHost() = default;

    // 0064560D: the *current* controlled unit's trait 6 probe, guarding the release.
    virtual bool current_is_kind_6() = 0;
    // 00645622: 00817380(current, false) - the per-part broadcast, released value.
    virtual void release_unit_parts() = 0;
    // 0064562D: 0080E290(current) - walks [current+48h] by +44h and calls 0085AD00 on
    // every node answering IsKindOf(22h).
    virtual void release_unit_nodes() = 0;

    // 00645637: hudRoot+1Ch = 0.
    virtual void clear_hud_slot() = 0;

    // 0064564D: 00645060(candidate, [game+18ECh], 1). The eligibility test.
    virtual bool candidate_is_selectable() = 0;
    // 0064565F / 00645673: candidate->IsKindOf(1) and candidate->vtable[124h]().
    virtual bool candidate_is_kind_1() = 0;
    virtual bool candidate_vtable_124() = 0;

    // 00645699 / 006456D3: 006952A0 and 00694A60, the observer pair at hudRoot+8h.
    virtual void unregister_observer() = 0;
    virtual void register_observer() = 0;

    // 006456B9 / 006456FA: 00954990(unit, flag). The audio broadcast; the released unit
    // gets flag 0 (value 1.0f) and the newly controlled one flag 1 (value 0.0f).
    virtual void set_unit_controlled_audio(bool controlled) = 0;

    // 004C0890 itself, at 00645687 and 006456C0.
    virtual void set_controlled_unit(bool candidate_present) = 0;

    // The global read back after each setter call: DAT_00E188D8 != 0.
    virtual bool controlled_unit_present() = 0;
    // 006456AB / 006456E5: the global's trait 5 probe, guarding both audio calls.
    virtual bool controlled_is_kind_5() = 0;
};

// What one pass of 00645600 did.
struct SelectControlledUnitResult {
    bool released_previous{false};  // the kind-6 release block ran
    bool candidate_rejected{false}; // the eligibility filter zeroed the candidate
    bool refreshed_in_place{false}; // the 00645687 call: current == candidate
    bool unregistered{false};
    bool registered{false};
};

// 00645600, __thiscall(hudRoot, Unit* candidate), RET 4, body 00645600..00645702.
//
// In order:
//  1. if (current && current->IsKindOf(6)) { 00817380(current, 0); 0080E290(current); }
//  2. hudRoot+1Ch = 0
//  3. candidate survives only when 00645060 passes and (!IsKindOf(1) || vtable[124h]())
//  4. if (current == candidate) 004C0890(candidate)      <- a republish, not a change
//  5. if (g_controlled) { 006952A0(g_controlled, hudRoot+8h);
//                         if (g_controlled->IsKindOf(5)) 00954990(g_controlled, 0); }
//  6. 004C0890(candidate)
//  7. if (g_controlled) { 00694A60(g_controlled, hudRoot+8h);
//                         if (g_controlled->IsKindOf(5)) 00954990(g_controlled, 1); }
//
// The global is re-read at 0064567B, 0064568C, 0064569E, 006456B1, 006456C5, 006456D8
// and 006456F4, so step 5 acts on the outgoing unit and step 7 on the incoming one.
// Step 7 is a tail jump to 00954990 at 006456FA.
//
// Step 4 is written as the listing has it (CMP ECX,ESI / JNZ past the call at 00645683).
// It fires only when the controlled unit is not changing, and its effect is to
// republish the listener handle; no reading of the surrounding code explains why, so it
// is recorded and not rationalised.
SelectControlledUnitResult select_controlled_unit_00645600(bool candidate_present,
                                                           SelectControlledUnitHost& host);

} // namespace bsp
