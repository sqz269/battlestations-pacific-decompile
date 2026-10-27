#pragma once
#include <utility>

// Added below are source interfaces for the newly defined 00D087C0 slots.
// The plane squadron as the object the AI commands: the member array at
// +3D0h, the flight leader, the groupable-combatant answer 009FE080 takes for
// class 18h, and the member fan-out the squadron's own vtable performs.
//
// Addresses: 007F2C60 (the constructor that zeroes the five member slots at
// 007F2DA3..007F2DBB), 004F0AD0 (the scene creator), 007F4580 (the slot-39
// attach that fills the array; 007F4B43..007F4B6E is the per-wing tail),
// 007F4735/007F4754/007F4778 (the WingCount rule), 007ED610 (the flight-leader
// rotation, vtable 00D087C0 slot +164h arm BEh), 007ECF80 (vtable slot +128h,
// the member broadcast), 007ECFD0 (vtable slot +114h, the AI command block at
// squadron+348h), 007EDA90 (the exclusion 009FE080 negates), 007F3970 /
// 007F39ED (the removal and its compaction), 007F3500 (the clone).
//
// docs/PLANE_SQUADRON_ENTITY.md carries the listing evidence. The object
// layout itself is docs/PLANE_SQUADRON.md's table and is not re-derived here;
// this header adds only the fields the AI path reads and the four rules that
// move planes in and out of the member array.
//
// Nothing here is ABI-compatible with the native 0x414-byte block. The field
// names carry the native offsets so a reader can follow them back.

#include <cstddef>
#include <cstdint>

#include "bsp/ai_command_lifetime.hpp"

namespace bsp {

// +C4h, stamped by 007F2DEC. docs/ENTITY_CLASS_IDS.md row 18.
inline constexpr int kPlaneSquadronClassId = 0x18;

// MPlaneKamikaze, the id 007EDAA0 pushes through the lead plane's
// vtable[+5Ch]. docs/ENTITY_CLASS_IDS.md row 17, class test 009534A0.
inline constexpr int kPlaneSquadronKamikazeKindId = 0x17;

// The plane base 0Fh, the class every member answers and the class 009FE080
// does NOT admit on its own (009FE0A5 pushes 6, the ship base, not 0Fh).
inline constexpr int kPlaneSquadronMemberKindId = 0x0F;

// 007F2DA3..007F2DBB zero exactly +3D0h, +3D4h, +3D8h, +3DCh and +3E0h, and
// 007F3885 CMP EBX,4 / 007F3888 JA is the second witness: five slots.
inline constexpr std::size_t kPlaneSquadronMaxWings = 5;

// 007F4735: the key is absent -> 3. 007F4754: present -> max(1, authored).
inline constexpr int kPlaneSquadronDefaultWingCount = 3;

// The squadron fields the AI path touches. Offsets are the native ones.
struct PlaneSquadronEntity {
    int class_id{kPlaneSquadronClassId};   // +C4h
    int wing_count{0};                     // +3C8h, WingCount
    int live_count{0};                     // +3CCh, the live member count
    void* members[kPlaneSquadronMaxWings]{};  // +3D0h..+3E0h
    int behaviour{-1};                     // +364h, seeded -1 by 007F2DE0
    int attack_mode{1};                    // +370h, seeded 1, cleared by 007F31A0
    int avoid_layer_secondary{0};          // +34Ch
    int avoid_layer_primary{0};            // +350h
    float morale{1.0f};                    // +3E8h
    bool dirty{false};                     // +3ECh
    void* ai_command_block{nullptr};       // +348h, vtable[+114h] returns it
};

// Tail thunks preserve the original argument/return category. The callback
// names the actual leader slot; +3D0h is read even when live_count is zero.
template<class F, class... A> decltype(auto) plane_squadron_leader_slot12c_007ecf70(
    const PlaneSquadronEntity& s, F&& f, A&&... a) {
    return std::forward<F>(f)(s.members[0], std::forward<A>(a)...);
}
template<class F, class... A> decltype(auto) plane_squadron_leader_slot38_007ed470(
    const PlaneSquadronEntity& s, F&& f, A&&... a) {
    return std::forward<F>(f)(s.members[0], std::forward<A>(a)...);
}
template<class F, class... A> decltype(auto) plane_squadron_leader_slotbc_007ed4a0(
    const PlaneSquadronEntity& s, F&& f, A&&... a) {
    return std::forward<F>(f)(s.members[0], std::forward<A>(a)...);
}
void* plane_squadron_leader_position_007ed450(const PlaneSquadronEntity&, void* out,
                                            void (*slot34)(void*, void*));
const char* plane_squadron_description_007ed480(const PlaneSquadronEntity&, const char* (*slot28)(void*));
float plane_squadron_leader_float_007ed4d0(const PlaneSquadronEntity&, double (*slot50)(void*));
double plane_squadron_heading_007ee290(const PlaneSquadronEntity&, double (*slotc8)(void*),
                                      void (*refresh)(void*), void* squadron,
                                      const float& world_ec, const float& world_f4);
bool plane_squadron_true_007efaa0() noexcept;
void plane_squadron_noop_007efab0(std::uint32_t unused) noexcept;
void plane_squadron_noop_007ed000(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept;
// ECX=squadron, float seconds on stack, RET4. Base first, then members in
// order; the loop re-reads live_count after every virtual call.
void plane_squadron_set_invincible_007f2290(PlaneSquadronEntity&, float seconds,
    void (*base)(void*, float), void* native_squadron, void (*member_slotf4)(void*, float));
struct PlaneSquadronPoseHost {
    virtual ~PlaneSquadronPoseHost() = default;
    virtual bool world_valid_c8(void* member) = 0;
    virtual void refresh_world_00414db0(void* member) = 0;
    virtual void copy_leader_world_to_local_004134f0(void* leader) = 0;
    virtual void invalidate_squadron_c8_10c() = 0;
    virtual void* first_child_48() = 0;
    virtual void invalidate_child_0042ed50(void*) = 0;
    virtual void* next_sibling_44(void*) = 0;
    virtual float invincible_until_308() = 0;
    virtual void clear_member_2f4(void*) = 0;
};
// Original ignores one stack word, RET4; comparison uses the x87 sum of a
// float game clock and a double tolerance. This model uses double for it.
void plane_squadron_follow_leader_pose_007f22e0(PlaneSquadronEntity&, const volatile float& game_time_f876a4,
                                              const volatile double& tolerance_d7a308, PlaneSquadronPoseHost&);

class Cc13Seg48Host;
// Raw-storage boundary for 00D087C0 slot+16Ch (007EFBA0). It reads genuine
// Win32 squadron/target fields; do not pass the projected PlaneSquadronEntity
// value above. All callbacks are required original operations, in image order.
struct PlaneSquadronCommandGateHost {
    virtual ~PlaneSquadronCommandGateHost() = default;
    // 00779D50: ECX=squadron, (command,target) stack, AL; RET8.
    virtual bool entity_may_follow_target_00779d50(void*, const char*, void*) = 0;
    // ECX=squadron, command stack, AL; RET4.
    virtual bool supports_command_vslot168(void*, const char*) = 0;
    // 00438E10: ECX=command, EDX=literal, EAX strcmp-like result.
    virtual int compare_insensitive_00438e10(const char*, const char*) = 0;
    virtual bool is_kind_vslot5c(void*, int kind) = 0;
    // 006BF050: ECX=target, DL=1; EAX block pointer.
    virtual void* air_ops_get_block_006bf050(void*, std::uint8_t flag) = 0;
    // 00803510: ECX=squadron party, EDX=target party, EAX relation.
    virtual int party_relative_00803510(std::uint32_t, std::uint32_t) = 0;
    virtual bool is_surface_00922dd0(void*) = 0;
    virtual bool is_airborne_00922b10(void*) = 0;
    virtual bool uses_small_turn_roll_limit_0047b880(void* leader) = 0;
    // ECX=squadron; target,1,1 on stack; EAX token, RET0Ch.
    virtual std::uint32_t choose_attack_command_007eec50(void*, void*, int, int) = 0;
};
// ECX=squadron, command/nullable target on stack; AL bool, RET8. Reuses the
// existing class-field predicate and squadron ordnance loops, without wiring.
bool plane_squadron_command_allowed_007efba0(void* native_squadron, const char* command,
    void* target, PlaneSquadronCommandGateHost&, Cc13Seg48Host& ordnance);

// 007F4735 / 007F4754 / 007F4778. `present` is whether the property bag
// carried `WingCount` (00CF8840); `authored` is the value at prop+0Ch.
int plane_squadron_wing_count_007f4754(bool present, int authored) noexcept;

// 007F4B43..007F4B6E, the per-wing tail of 007F4580's mode-1 loop:
//   plane+9D8h = squadron+3CCh   (initial index;007ED260 later reindexes)
//   plane+9D4h = squadron        (the back pointer)
//   squadron+3D0h[count] = plane
//   squadron+3CCh += 1
//   squadron+3ECh = 1
// The native store at 007F4B55 has NO bound test; this returns false instead
// of writing past the fifth slot, which is the only deliberate divergence.
// `spawn_index` receives the value 007F4B43 stamps.
bool plane_squadron_attach_plane_007f4b43(PlaneSquadronEntity& squadron, void* plane,
                                          int* spawn_index) noexcept;

// +3D0h, the flight leader. 007ED610 is what makes slot 0 mean "leader".
void* plane_squadron_flight_leader(const PlaneSquadronEntity& squadron) noexcept;

// 007ED610, vtable 00D087C0 slot +164h arm BEh. Rotates members[index] into
// slot 0 by shifting the prefix up one; 007ED614 rejects index <= 0 and
// 007ED618 rejects index >= +3CCh, both without touching the array.
bool plane_squadron_promote_flight_leader_007ed610(PlaneSquadronEntity& squadron,
                                                   int index) noexcept;

// 007F3970's compaction, 007F39E0..007F3A1D: move the tail down over the
// removed slot, decrement +3CCh, zero the now-unused last slot, set +3ECh.
bool plane_squadron_remove_plane_007f39ed(PlaneSquadronEntity& squadron,
                                          void* plane) noexcept;

// The three reads 007EDA90 takes, all on the FLIGHT LEADER at +3D0h and not on
// a carrier: 007EDA91 loads [squadron+3D0h], 007EDAA0 asks that object
// IsKindOf(17h) = MPlaneKamikaze, 007EDAAA tests the byte at leader+C24h,
// which docs/ATTACK_GATE_TAILS.md establishes is the authored `PilotFires`.
struct PlaneSquadronLeadPlaneFacts {
    bool has_lead_plane{false};        // [squadron+3D0h] != 0
    bool lead_is_kamikaze_17{false};   // leader->vtable[+5Ch](17h)
    bool lead_pilot_fires_0c24{false}; // the byte at leader+C24h
};

// 007EDA90 itself, expressed on the leader facts. True only when all three
// hold in the shape the listing tests them. This is the SAME rule
// ai_squadron_excluded_007eda90 models; this overload exists so a caller that
// holds a squadron can answer it without inventing a "carrier link".
bool plane_squadron_excluded_007eda90(const PlaneSquadronLeadPlaneFacts& lead) noexcept;

// The bridge into 009FE080: a squadron fills the plane-squadron arm, never the
// ship-base arm (009FE0A0 is only reached when IsKindOf(18h) is false).
AiGroupableCombatantFacts plane_squadron_combatant_facts(
    const PlaneSquadronLeadPlaneFacts& lead) noexcept;

// 007ECF80, vtable 00D087C0 slot +128h, __thiscall(squadron, arg), RET 4:
//   for (i = 0; i < squadron->+3CCh; ++i)
//       members[i]->vtable[+128h](arg)
// Slot +128h is the skill-level setter (0089539A in BSP_LuaBinding_SetSkillLevel,
// 0082388E in BSP_UnitInstance_SEntityInit), so on a squadron the slot means
// "apply to every live member". `visit` is called once per live member, in
// array order, and the count of members visited is returned.
std::size_t plane_squadron_broadcast_to_members_007ecf80(
    const PlaneSquadronEntity& squadron,
    void (*visit)(void* member, void* context), void* context) noexcept;

// Complete ordinary 007F4BA0..007F544F, squadron vtable 00D087C0 +A4h.
// Native ECX=squadron, no stack arguments, RET. Descriptive names are hypotheses.
// The caller supplies initialized bytes representing native local storage B+0Ch
// through B+5Bh: Lua handle conversion can leave the DWORD at B+0Ch untouched.
// No default value is invented for that slot. This C++ interface is unwired;
// native FH3 frame/unwind and binary ABI equivalence remain unproved.
struct PlaneSquadronInitScratch {
    alignas(4) std::uint8_t bytes[0x50];
};

// Native offsets used by the raw-storage init interface, named consistently
// with PlaneSquadronEntity and the saved-field keys in 007F50CF..007F5364.
// These constants do not claim a C++ object layout or binary ABI.
struct PlaneSquadronInitOffset {
    static constexpr std::size_t children = 0x48;
    static constexpr std::size_t local_transform = 0x74;
    static constexpr std::size_t state_holder = 0xc0;
    static constexpr std::size_t world_transform_valid = 0xc8;
    static constexpr std::size_t world_position = 0xfc;
    static constexpr std::size_t dependent_pose_valid = 0x10c;
    static constexpr std::size_t invincibility_seconds = 0x150;
    static constexpr std::size_t current_command = 0x344;
    static constexpr std::size_t ai_command_block = 0x348;
    static constexpr std::size_t avoid_layer_secondary = 0x34c;
    static constexpr std::size_t avoid_layer_primary = 0x350;
    static constexpr std::size_t leader_component_id = 0x354; // slot +00h result
    static constexpr std::size_t retreat_in_progress = 0x361;
    static constexpr std::size_t attack_mode = 0x370;
    static constexpr std::size_t eq_per = 0x374; // native saved key "EQper"
    static constexpr std::size_t automatic_drop_enabled = 0x378;
    static constexpr std::size_t countdowns = 0x37c;
    static constexpr std::size_t num_followers = 0x3bc;
    static constexpr std::size_t wing_count = 0x3c8;
    static constexpr std::size_t live_count = 0x3cc;
    static constexpr std::size_t members = 0x3d0;
    static constexpr std::size_t formation_shape = 0x3e4;
    static constexpr std::size_t home_base_observer = 0x3f0;
    static constexpr std::size_t home_base = 0x404;
    static constexpr std::size_t inside_home_base = 0x408;
    static constexpr std::size_t collected_damage = 0x40c;
    static constexpr std::size_t previous_collected_damage = 0x410;
};
struct PlaneSquadronInitGlobals {
    std::uint8_t* const& game_00e188a8;
};
struct PlaneSquadronInitHost {
    virtual ~PlaneSquadronInitHost() = default;
    virtual void call_0077fad0(void* squadron) = 0; // ECX, RET
    virtual std::uint32_t component_virtual_00(void* component) = 0; // ECX, RET
    // Native 007ED260 leaves ECX unchanged; 007F4EF5 reuses it for 007ED6E0.
    virtual void call_007ed260(void* squadron) = 0;
    virtual void* call_008f2260(void* bag, std::uint32_t key_address) = 0; // RET4
    virtual const char* call_008f0e20(void* property) = 0; // ECX, RET
    virtual void* call_00925a90(void* registry, const char* name) = 0; // RET4
    virtual void* call_006bcd20(void* entity, std::uint8_t require_active) = 0; // ECX,DL
    virtual void call_007f1c00(void* squadron, void* base, std::uint8_t flag) = 0; // RET8
    virtual void call_007ed1d0(void* squadron, const void* vector3) = 0; // RET4
    virtual void entity_virtual_1bc(void* entity, std::uint32_t value) = 0; // RET4
    virtual void call_0085dc80(void* matrix) = 0;
    virtual void call_0042ed50(void* node) = 0;
    virtual void call_007f2920(void* squadron) = 0;
    virtual bool call_007eda90(void* squadron) = 0;
    virtual void* block_virtual_50(void* block) = 0;
    virtual void block_virtual_54(void* block, std::uint32_t value) = 0;
    virtual void* call_0071be40(void* block) = 0;
    // Both target constructors are ECX=out, (entity/point,float) on stack, RET8.
    virtual void* call_00465080(void* out24, void* entity, float extra) = 0;
    virtual void call_00414db0(void* node) = 0;
    virtual void* call_007ef8f0(void* game, void* out3, const void* point3) = 0; // RET8
    virtual void* call_00468560(void* out24, const void* point3, float extra) = 0;
    virtual void call_0077d600(void* squadron, std::uint32_t command_address,
        const void* target24, std::uint32_t flags) = 0; // RET0Ch
    virtual void* call_00521e30(std::uint16_t handle) = 0; // CX, RET
    virtual void call_007f15f0(void* squadron, void* target) = 0; // RET4
    virtual void call_007ed6e0(void* squadron) = 0;
    virtual void call_009238a0(void* squadron, void* out_lua20) = 0; // RET4
    // ECX=reader; consumes/destructs a by-value 14h-byte LuaObject, RET14h.
    virtual void call_004425c0(void* reader20, void* consumed_lua20) = 0;
    virtual void call_00bd8e20(void* reader, std::uint32_t key_kind,
        std::uint32_t key_value) = 0; // RET8
    virtual bool call_00bd5eb0(void* reader, std::uint32_t key_kind,
        std::uint32_t key_value) = 0; // RET8, AL
    // Reader +10h and direct BD6830 both take key {tag,value}, field {tag,dest},
    // RET10h. Tags 1/2/3 store DWORD/float/byte. Tag4 writes a DWORD handle only
    // for an integral number or table; other values leave the destination alone.
    virtual void reader_virtual_10(void* reader, std::uint32_t key_kind,
        std::uint32_t key_value, std::uint32_t field_kind, void* destination) = 0;
    virtual void call_00bd6830(void* reader, std::uint32_t key_kind,
        std::uint32_t key_value, std::uint32_t field_kind, void* destination) = 0;
    virtual void call_00bd7a20(void* reader) = 0; // reader+4 vector pop, RET
    virtual void* call_00bf681b(std::uint32_t bytes) = 0; // CRT new binding only
    virtual void* call_0084d810(void* storage, void* squadron) = 0; // RET4
    virtual void call_006952a0(void* old_base, void* observer_owner) = 0; // ECX,EDX
    virtual void call_00694a60(void* new_base, void* observer_owner) = 0; // ECX,EDX
    virtual void* call_004c17d0() = 0;
    virtual void* call_0041df40(void* registry, float value, std::uint8_t angle) = 0; // RET8
    virtual void reader_virtual_04(void* reader, std::uint32_t key_kind,
        std::uint32_t key_value) = 0;
    virtual void reader_virtual_08(void* reader) = 0;
    virtual void call_007f0dc0(void* settings, void* reader) = 0; // RET4
    virtual void call_00441210(void* vector_at_reader_plus4) = 0;
    virtual bool entity_virtual_5c(void* entity, std::uint32_t kind) = 0; // RET4, AL
    virtual void* call_007f1de0(void* storage, void* parent_node, void* squadron) = 0; // RET8
    virtual void squadron_virtual_f4(void* squadron, float value) = 0; // RET4
};
void plane_squadron_sentity_init_007f4ba0(void* native_squadron,
    PlaneSquadronInitScratch&, const PlaneSquadronInitGlobals&, PlaneSquadronInitHost&);

}  // namespace bsp
