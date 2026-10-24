// The squadron rules the AI path depends on. docs/PLANE_SQUADRON_ENTITY.md
// carries the listing for every address named here.

#include "bsp/plane_squadron_entity.hpp"
#include <cmath>
#include <cstring>
#include "bsp/cc13_seg48.hpp"
#include "bsp/cc13_sweepB.hpp"

namespace bsp {
namespace {
template<class T> T squadron_gate_read(const void* object, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(object) + offset, sizeof(value));
    return value;
}
}
bool plane_squadron_command_allowed_007efba0(void* s, const char* command, void* target,
    PlaneSquadronCommandGateHost& h, Cc13Seg48Host& ordnance) {
    if (squadron_gate_read<std::uint8_t>(s, 0x3b0)) return false;
    if (h.entity_may_follow_target_00779d50(s, command, target)) return true;
    if (!h.supports_command_vslot168(s, command)) return false;
    const auto equal = [&](const char* literal) { return h.compare_insensitive_00438e10(command, literal) == 0; };
    const auto relative = [&]() {
        // The image loads target+54h before squadron+54h on every query.
        const auto target_party = squadron_gate_read<std::uint32_t>(target, 0x54);
        const auto self_party = squadron_gate_read<std::uint32_t>(s, 0x54);
        return h.party_relative_00803510(self_party, target_party);
    };
    if (!target) return equal("moveto") || equal("cleartarget");
    if (equal("land")) {
        if (cc13_sweepB_field198_is_zero_0060c5b0(squadron_gate_read<void*>(s, 0x35c))) {
            if (!h.is_kind_vslot5c(target, 6)) return false;
            return relative() == 0;
        }
        if (!h.air_ops_get_block_006bf050(target, 1)) return false;
        if (relative() != 0) return false;
        void* cls = squadron_gate_read<void*>(s, 0x35c);
        if (!squadron_gate_read<std::uint8_t>(cls, 0x160) && !h.is_kind_vslot5c(target, 0x45)) return false;
        return true;
    }
    if (equal("moveto")) {
        if (h.is_surface_00922dd0(target) && relative() == 0) return true;
        if (!h.is_airborne_00922b10(target)) return false;
        void* leader = squadron_gate_read<void*>(s, 0x3d0);
        return leader && h.uses_small_turn_roll_limit_0047b880(leader);
    }
    if (equal("settarget")) return h.choose_attack_command_007eec50(s, target, 1, 1) != 0;
    if (h.is_surface_00922dd0(target)) {
        // Keep the repeated moveto comparison: it is present at 007EFD4F.
        if (relative() == 0 && equal("moveto")) return true;
        if (squadron_gate_read<std::uint8_t>(target, 0x5d)) return false;
        if (equal("strafe")) return true;
        if (equal("bomb") && cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(s, 0, ordnance)) return true;
        if (!equal("torpedo")) return false;
        return cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(s, 0, ordnance);
    }
    if (h.is_kind_vslot5c(target, 0x1b) || h.is_kind_vslot5c(target, 0x1a) || h.is_kind_vslot5c(target, 0x46)) {
        if (squadron_gate_read<std::uint8_t>(target, 0x5d)) return false;
        if (equal("strafe")) return true;
        return equal("bomb") && cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(s, 0, ordnance);
    }
    if (!h.is_kind_vslot5c(target, 0x18)) return false;
    if (equal("moveto")) return true;
    void* leader = squadron_gate_read<void*>(s, 0x3d0);
    if (!leader || h.is_kind_vslot5c(leader, 0x10)) return false;
    return equal("strafe") || equal("dogfight");
}
void* plane_squadron_leader_position_007ed450(const PlaneSquadronEntity& s, void* out,
                                            void (*slot34)(void*, void*)) {
    slot34(s.members[0], out); return out;
}
const char* plane_squadron_description_007ed480(const PlaneSquadronEntity& s, const char* (*slot28)(void*)) {
    return s.members[0] ? slot28(s.members[0]) : "empty squadron desc";
}
float plane_squadron_leader_float_007ed4d0(const PlaneSquadronEntity& s, double (*slot50)(void*)) {
    return s.members[0] ? static_cast<float>(slot50(s.members[0])) : 0.0f;
}
double plane_squadron_heading_007ee290(const PlaneSquadronEntity& s, double (*slotc8)(void*),
                                      void (*refresh)(void*), void* squadron,
                                      const float& world_ec, const float& world_f4) {
    if (s.members[0]) return slotc8(s.members[0]);
    refresh(squadron);
    // Native atan2 register ABI has ST1=y=EC, ST0=x=F4; spills to float.
    return -static_cast<float>(std::atan2(static_cast<double>(world_ec), static_cast<double>(world_f4)));
}
bool plane_squadron_true_007efaa0() noexcept { return true; }
void plane_squadron_noop_007efab0(std::uint32_t) noexcept {}
void plane_squadron_noop_007ed000(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept {}
void plane_squadron_set_invincible_007f2290(PlaneSquadronEntity& s, float seconds,
    void (*base)(void*, float), void* native_squadron, void (*member_slotf4)(void*, float)) {
    base(native_squadron, seconds);
    for (int i = 0; i < s.live_count; ++i) member_slotf4(s.members[i], seconds);
}
void plane_squadron_follow_leader_pose_007f22e0(PlaneSquadronEntity& s, const volatile float& now,
                                              const volatile double& tolerance, PlaneSquadronPoseHost& h) {
    if (s.live_count == 0) return;
    void* leader = s.members[0];
    if (!h.world_valid_c8(leader)) h.refresh_world_00414db0(leader);
    h.copy_leader_world_to_local_004134f0(leader);
    h.invalidate_squadron_c8_10c();
    for (void* child = h.first_child_48(); child; child = h.next_sibling_44(child))
        h.invalidate_child_0042ed50(child);
    const float until = h.invincible_until_308();
    if (until != 0.0f && static_cast<double>(until) < static_cast<double>(now) + tolerance)
        for (int i = 0; i < s.live_count; ++i) h.clear_member_2f4(s.members[i]);
}
}

namespace bsp {

int plane_squadron_wing_count_007f4754(bool present, int authored) noexcept {
    // 007F4735: 008F2260 returned 0 for `WingCount`, so the immediate 3 at
    // 007F473B is what reaches +3C8h.
    if (!present) return kPlaneSquadronDefaultWingCount;
    // 007F4754 reads prop+0Ch and 007F4760..007F4770 clamps it up to 1.
    return authored < 1 ? 1 : authored;
}

bool plane_squadron_attach_plane_007f4b43(PlaneSquadronEntity& squadron, void* plane,
                                          int* spawn_index) noexcept {
    if (plane == nullptr) return false;
    if (squadron.live_count < 0) return false;
    // 007F4B55 MOV [ESI+EDI*4+3D0h],EAX has no bound test; the authored
    // WingCount enum (" 1".." 5") is what keeps a sixth wing unreachable. This
    // process refuses instead of writing over +3E4h.
    if (static_cast<std::size_t>(squadron.live_count) >= kPlaneSquadronMaxWings) return false;
    // 007F4B43: initial plane+9D8h = the CURRENT +3CCh. Native007ED260
    // later rewrites that index after changes to the member order.
    if (spawn_index != nullptr) *spawn_index = squadron.live_count;
    squadron.members[static_cast<std::size_t>(squadron.live_count)] = plane;  // 007F4B55
    squadron.live_count += 1;                                                 // 007F4B60
    squadron.dirty = true;                                                    // 007F4B6E
    return true;
}

void* plane_squadron_flight_leader(const PlaneSquadronEntity& squadron) noexcept {
    // 007EDA91 MOV ESI,[ECX+3D0h]: slot 0, whatever the live count.
    return squadron.live_count > 0 ? squadron.members[0] : nullptr;
}

bool plane_squadron_promote_flight_leader_007ed610(PlaneSquadronEntity& squadron,
                                                   int index) noexcept {
    // 007ED614 TEST EDX,EDX / JLE and 007ED618 CMP [ECX+3CCh],EDX / JLE.
    if (index <= 0 || index >= squadron.live_count) return false;
    if (static_cast<std::size_t>(index) >= kPlaneSquadronMaxWings) return false;
    // 007ED621 saves members[index], 007ED630..007ED63D shifts the prefix up
    // one slot from the top down, 007ED63F writes the saved plane into slot 0.
    void* promoted = squadron.members[static_cast<std::size_t>(index)];
    for (int i = index; i > 0; --i) {
        squadron.members[static_cast<std::size_t>(i)] =
            squadron.members[static_cast<std::size_t>(i - 1)];
    }
    squadron.members[0] = promoted;
    // Rotation-only projection. Native007ED645 then calls007ED260 to publish
    // live plane indices; plane_squadron_promote_and_reindex_007ed610 supplies
    // that required provider call (docs/LANDING_BE_PROFILE_CC11.md).
    return true;
}

bool plane_squadron_remove_plane_007f39ed(PlaneSquadronEntity& squadron,
                                          void* plane) noexcept {
    if (plane == nullptr || squadron.live_count <= 0) return false;
    int slot = -1;
    for (int i = 0; i < squadron.live_count; ++i) {
        if (squadron.members[static_cast<std::size_t>(i)] == plane) { slot = i; break; }
    }
    if (slot < 0) return false;
    // 007F39D0's memmove of the tail towards the removed slot.
    for (int i = slot; i + 1 < squadron.live_count; ++i) {
        squadron.members[static_cast<std::size_t>(i)] =
            squadron.members[static_cast<std::size_t>(i + 1)];
    }
    squadron.live_count -= 1;                    // 007F39ED ADD [ESI+3CCh],-1
    squadron.members[static_cast<std::size_t>(squadron.live_count)] = nullptr;  // 007F39FA
    squadron.dirty = true;                       // 007F3A1D
    return true;
}

bool plane_squadron_excluded_007eda90(const PlaneSquadronLeadPlaneFacts& lead) noexcept {
    if (!lead.has_lead_plane) return false;       // 007EDA99 JZ
    if (!lead.lead_is_kamikaze_17) return false;  // 007EDAA8 JZ
    if (lead.lead_pilot_fires_0c24) return false; // 007EDAB1 JNZ
    return true;                                  // 007EDAB3 MOV AL,1
}

AiGroupableCombatantFacts plane_squadron_combatant_facts(
    const PlaneSquadronLeadPlaneFacts& lead) noexcept {
    AiGroupableCombatantFacts facts;
    // 009FE088 PUSH 18h answers true for every squadron, so 009FE0A0's
    // ship-base tail is never reached and is_ship_base stays false.
    facts.is_plane_squadron = true;
    facts.is_ship_base = false;
    facts.squadron_has_flight_leader = lead.has_lead_plane;
    facts.leader_is_kamikaze_kind_17 = lead.lead_is_kamikaze_17;
    facts.leader_pilot_fires_0c24 = lead.lead_pilot_fires_0c24;
    return facts;
}

std::size_t plane_squadron_broadcast_to_members_007ecf80(
    const PlaneSquadronEntity& squadron,
    void (*visit)(void* member, void* context), void* context) noexcept {
    // 007ECF86 CMP [EBX+3CCh],ESI / 007ECF8C JLE: nothing to do at zero.
    if (visit == nullptr || squadron.live_count <= 0) return 0;
    std::size_t reached = 0;
    const int live = squadron.live_count < static_cast<int>(kPlaneSquadronMaxWings)
        ? squadron.live_count : static_cast<int>(kPlaneSquadronMaxWings);
    // 007ECFA0..007ECFB9: EDI walks &members[0] upwards, ESI counts against
    // +3CCh re-read every iteration.
    for (int i = 0; i < live; ++i) {
        void* member = squadron.members[static_cast<std::size_t>(i)];
        if (member == nullptr) continue;
        visit(member, context);
        ++reached;
    }
    return reached;
}

namespace {
template<class T> void squadron_init_store(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(object) + offset, &value, sizeof(value));
}
void* squadron_init_add(void* object, std::size_t offset) {
    return static_cast<std::uint8_t*>(object) + offset;
}
}

// 007F4BA0..007F544F: all 606 listing instructions reviewed, including RET.
// ECX=squadron, no stack arguments. Normal reader cleanup is included; FH3
// registration/unwind is excluded. Boundaries below are inspected native calls,
// not replacements for their bodies. No game/host wiring is supplied.
void plane_squadron_sentity_init_007f4ba0(void* s, PlaneSquadronInitScratch& scratch,
    const PlaneSquadronInitGlobals& g, PlaneSquadronInitHost& h) {
    auto* temp = scratch.bytes; // native B+0Ch, reused without invented defaults
    const auto get_pointer = [&](const void* object, std::size_t offset) {
        return squadron_gate_read<void*>(object, offset);
    };
    const auto property = [&](std::uint32_t key) {
        void* holder = get_pointer(s, 0xc0);
        return h.call_008f2260(get_pointer(holder, 8), key);
    };
    const auto mode = [&]() {
        return squadron_gate_read<std::int32_t>(g.game_00e188a8, 0x1fe4);
    };
    const auto cache_leader_component = [&]() {
        if (void* leader = get_pointer(s, 0x3d0)) {
            const auto result = h.component_virtual_00(squadron_init_add(leader, 0x170));
            squadron_init_store(s, 0x354, result);
        }
    };
    h.call_0077fad0(s);
    squadron_init_store(s, 0x3c8, squadron_gate_read<std::uint32_t>(s, 0x3cc));
    cache_leader_component();
    squadron_init_store(s, 0x3e4, std::uint32_t{1});
    h.call_007ed260(s);
    void* holder = get_pointer(s, 0xc0);
    if (squadron_gate_read<std::int32_t>(holder, 4) == 1) {
        void* p = property(0x00cf8818); // State
        std::int32_t state = 7;
        if (p && squadron_gate_read<std::uint32_t>(p, 4) == 0)
            state = squadron_gate_read<std::int32_t>(p, 0xc);
        temp[0] = static_cast<std::uint8_t>(state <= 1);
        void* home = nullptr;
        p = property(0x00cf8820); // HomeBase
        if (p && squadron_gate_read<std::uint32_t>(p, 4) == 5) {
            const char* first = h.call_008f0e20(p);
            if (std::strlen(first) != 0) {
                const char* second = h.call_008f0e20(p);
                home = h.call_00925a90(get_pointer(g.game_00e188a8, 0x19cc), second);
            }
        }
        p = property(0x00ce56b8); // SpawnPoint
        if (p && squadron_gate_read<std::uint32_t>(p, 4) == 5) {
            const char* first = h.call_008f0e20(p);
            if (std::strlen(first) != 0) {
                const char* second = h.call_008f0e20(p);
                void* spawn = h.call_00925a90(get_pointer(g.game_00e188a8, 0x19cc), second);
                void* parent = get_pointer(spawn, 0x3c); // no native null guard
                if (h.call_006bcd20(parent, 0)) {
                    temp[0] = 1;
                    home = parent;
                }
            }
        }
        h.call_007f1c00(s, home, temp[0]); // callee consumes only the low flag byte
        p = property(0x00cfc85c); // Vel3D
        if (p && squadron_gate_read<std::uint32_t>(p, 4) == 7) {
            std::memcpy(temp + 0xc, squadron_init_add(p, 0xc), 12);
            h.call_007ed1d0(s, temp + 0xc);
        }
        p = property(0x00cfc840); // DummyObjectID
        if (p && squadron_gate_read<std::uint32_t>(p, 4) == 0) {
            void* leader = get_pointer(s, 0x3d0);
            h.entity_virtual_1bc(leader, squadron_gate_read<std::uint32_t>(p, 0xc));
        }
        h.call_0085dc80(squadron_init_add(s, 0x74));
        void* child = get_pointer(s, 0x48);
        squadron_init_store(s, 0xc8, std::uint8_t{0});
        squadron_init_store(s, 0x10c, std::uint8_t{0});
        while (child) {
            h.call_0042ed50(child);
            child = get_pointer(child, 0x44);
        }
        if (!squadron_gate_read<std::uint8_t>(s, 0x408)) h.call_007f2920(s);
        if (mode() != 2) {
            void* current = get_pointer(get_pointer(s, 0x348), 0x38);
            squadron_init_store(s, 0x344, current);
            if (current && h.call_007eda90(s)) {
                if (h.block_virtual_50(get_pointer(s, 0x348)))
                    h.block_virtual_54(get_pointer(s, 0x348), 0);
            }
        }
        if (!h.call_0071be40(get_pointer(s, 0x348)) && mode() != 2) {
            void* leader = get_pointer(s, 0x3d0);
            if (leader && squadron_gate_read<std::int32_t>(leader, 0x900) == 6) {
                if (!squadron_gate_read<std::uint8_t>(s, 0xc8)) h.call_00414db0(s);
                void* point = h.call_007ef8f0(g.game_00e188a8, temp + 0xc,
                    squadron_init_add(s, 0xfc));
                const void* target = h.call_00468560(temp + 0x38, point, 0.0f);
                h.call_0077d600(s, 0x00e08f88, target, 1);
            } else if (void* base = get_pointer(s, 0x404)) {
                const void* target = h.call_00465080(temp + 0xc, base, 0.0f);
                h.call_0077d600(s, 0x00e08f68, target, 1);
            }
        }
        const auto current_mode = mode();
        if (current_mode == 0 || current_mode == 1) {
            p = property(0x00cf87ec); // AutoAttackTarget
            if (p && squadron_gate_read<std::uint32_t>(p, 4) == 0) {
                const auto id = squadron_gate_read<std::int32_t>(p, 0xc);
                if (id >= 0) {
                    void* target = h.call_00521e30(static_cast<std::uint16_t>(id));
                    h.call_007f15f0(s, target);
                }
            }
        }
    } else if (squadron_gate_read<std::uint8_t>(s, 0x408)) {
        // 007ED260 preserves ECX=s, so 007F4EF5 needs no explicit ECX reload.
        h.call_007ed6e0(s);
    }
    holder = get_pointer(s, 0xc0);
    if (holder && squadron_gate_read<std::int32_t>(holder, 4) == 3) {
        alignas(4) std::uint8_t root20[0x14];
        void* reader = temp + 0x24; // native B+30h, exactly 14h bytes
        h.call_009238a0(s, root20);
        h.call_004425c0(reader, root20); // consumes the by-value root; no extra release
        h.call_00bd8e20(reader, 0, 0x00d08ad4);
        h.call_00bd8e20(reader, 0, 0x00cf8880); // planes
        std::uint32_t index = 0;
        while (h.call_00bd5eb0(reader, 1, index)) {
            h.reader_virtual_10(reader, 1, index, 4, temp);
            const auto count = squadron_gate_read<std::uint32_t>(s, 0x3cc);
            const auto slot = static_cast<std::uint32_t>(0x3d0 + count * 4);
            squadron_init_store(s, slot, squadron_gate_read<std::uint32_t>(temp, 0));
            squadron_init_store(s, 0x3cc, squadron_gate_read<std::uint32_t>(s, 0x3cc) + 1u);
            ++index; // no added five-member bound or count reset
        }
        h.call_00bd7a20(reader);
        void* storage = h.call_00bf681b(0x22c);
        void* block = storage ? h.call_0084d810(storage, s) : nullptr;
        squadron_init_store(s, 0x348, block);
        squadron_init_store(s, 0x344, get_pointer(block, 0x38)); // native unchecked read
        h.call_00bd6830(reader, 0, 0x00d08ac8, 1, temp);
        squadron_init_store(s, 0x370, squadron_gate_read<std::uint32_t>(temp, 0));
        h.call_00bd6830(reader, 0, 0x00d08ab8, 1, squadron_init_add(s, 0x3bc));
        h.call_00bd6830(reader, 0, 0x00d08aac, 1, squadron_init_add(s, 0x3c8));
        h.reader_virtual_10(reader, 0, 0x00d08aa0, 1, temp);
        squadron_init_store(s, 0x3e4, squadron_gate_read<std::uint32_t>(temp, 0));
        h.reader_virtual_10(reader, 0, 0x00d08a94, 4, temp); // homeBase
        void* old_home = get_pointer(s, 0x404);
        void* new_home = get_pointer(temp, 0);
        void* observer = squadron_init_add(s, 0x3f0);
        if (old_home != new_home) {
            if (old_home) h.call_006952a0(old_home, observer);
            squadron_init_store(observer, 0x14, new_home);
            if (new_home) h.call_00694a60(new_home, observer);
        }
        h.call_00bd6830(reader, 0, 0x00d08a84, 2, squadron_init_add(s, 0x40c));
        h.call_00bd6830(reader, 0, 0x00d08a70, 2, squadron_init_add(s, 0x410));
        h.call_00bd6830(reader, 0, 0x00d08a5c, 3, squadron_init_add(s, 0x361));
        h.call_00bd6830(reader, 0, 0x00d08a44, 2, temp + 4);
        // 007F5231 FLD / 5235 FSTP: explicit float32 spill before singleton call.
        squadron_init_store(temp, 0, squadron_gate_read<float>(temp, 4));
        void* registry = h.call_004c17d0();
        const float slope = squadron_gate_read<float>(temp, 0);
        void* layer = h.call_0041df40(registry, slope, 0);
        squadron_init_store(s, 0x350, layer);
        registry = h.call_004c17d0();
        layer = h.call_0041df40(registry, 1.5f, 1); // 00CE380C = 3FC00000
        void* captured_block = get_pointer(s, 0x348);
        squadron_init_store(s, 0x34c, layer);
        h.reader_virtual_04(reader, 0, 0x00cf8e4c); // gameUnit
        // 007F5296 calls 0071BDC0: actual single RET4, no ordinary side effects.
        // Its ECX is the block captured before the preceding reader callback.
        static_cast<void>(captured_block);
        h.reader_virtual_08(reader);
        h.call_00bd6830(reader, 0, 0x00d08a30, 3, squadron_init_add(s, 0x408));
        h.call_007f0dc0(squadron_init_add(s, 0x37c), reader);
        h.call_00bd6830(reader, 0, 0x00d08a08, 2, squadron_init_add(s, 0x374));
        h.call_00bd6830(reader, 0, 0x00d089f8, 3, squadron_init_add(s, 0x378));
        h.call_00bd7a20(reader);
        squadron_init_store(reader, 0, std::uint32_t{0x00ce44fc});
        h.call_00441210(squadron_init_add(reader, 4));
        squadron_init_store(reader, 0, std::uint32_t{0x00ce374c});
    }
    void* home = get_pointer(s, 0x404); // retain EDI across both kind tests/allocation
    if (home && squadron_gate_read<std::int32_t>(get_pointer(s, 0xc0), 4) != 2) {
        bool launch = h.entity_virtual_5c(home, 9);
        if (!launch) launch = h.entity_virtual_5c(home, 0x45);
        if (launch) {
            if (void* storage = h.call_00bf681b(0x38))
                h.call_007f1de0(storage, squadron_init_add(home, 0x310), s);
        }
    }
    h.squadron_virtual_f4(s, squadron_gate_read<float>(s, 0x150));
    cache_leader_component(); // reload leader after the F4 callback
}

}  // namespace bsp
