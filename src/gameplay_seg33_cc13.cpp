// Reference models of the bomb platforms and their device reload (segment 33, cc13-gpr33).
// Evidence per function is in docs/CC13_GPR_33.md; the header states each original address and
// ABI. Reconstructed and build-tested only: nothing here is wired into a host.
#include "bsp/gameplay_seg33_cc13.hpp"

#include <cmath>
#include <cstddef>
#include <iterator>
#include <list>

namespace bsp::gameplay_seg33_cc13 {
namespace {

inline bool authority(SessionMode mode) noexcept {
    // [00E188A8]+1FE4h tested as 0 or 1 (006E4222-006E4229, 006E453F-006E4549, 006E5DA4).
    return mode == SessionMode::Local || mode == SessionMode::Host;
}

inline std::size_t at(int slot) noexcept { return static_cast<std::size_t>(slot); }

}  // namespace

// ---------------------------------------------------------------------------------------------
// Single rack
// ---------------------------------------------------------------------------------------------

SingleRackState rack_construct_006e3c00() noexcept {
    // 006E3C00 after BSP_Gun_Construct: +480h = [00D7A260], +494h = 0, +484h = +488h = 1,
    // +498h = 0, +4F8h = 0, +48Ch = +490h = [00CE38C8]. The class id 25h at +C4h and the six
    // vtables are the record's, not modelled.
    return SingleRackState{};
}

void rack_set_ammo_006e3530(SingleRackState& rack, int n) noexcept {
    rack.ammo_484 = n;
    rack.org_ammo_488 = n;
}

void rack_set_reload_time_006e3a20(SingleRackState& rack, float reload_time) noexcept {
    // 006E3A24 FLD1 / FCOMIP / JBE: below 1.0 both fields take [00D7A24C]; NaN is kept.
    const float t = reload_time < 1.0f ? kRackReloadTimeFloor : reload_time;
    rack.reload_time_48c = t;
    rack.reload_left_490 = t;
}

int rack_round_count_006e3500(const SingleRackState& rack, int attached_rounds) noexcept {
    return attached_rounds + rack.ammo_484;  // 006E350F
}

bool rack_is_empty_006e3570(const SingleRackState& rack, int attached_rounds) noexcept {
    return attached_rounds + rack.ammo_484 == 0;
}

float rack_ammo_fraction_006e4130(const SingleRackState& rack, int attached_rounds) noexcept {
    if (rack.org_ammo_488 == 0) return 0.0f;  // 006E4134 / 006E413D FLDZ
    // 006E4158 FILD, 006E415C FIDIV, 006E4163 FSTP float.
    return static_cast<float>(static_cast<double>(attached_rounds + rack.ammo_484) /
                              static_cast<double>(rack.org_ammo_488));
}

float rack_remaining_drop_time_006e4170(const SingleRackState& rack, int attached_rounds,
                                        float repeat_time) noexcept {
    int n = attached_rounds + rack.ammo_484 - 1;  // 006E4186
    if (n < 0) n = 0;                             // SETL / SUB 1 / AND
    // 006E419C FLD class+E0h, 006E41A6 FIMUL, FSTP float: exact in double.
    return static_cast<float>(static_cast<double>(repeat_time) * n);
}

void rack_fire_006e3550(SingleRackState& rack, float delay) noexcept {
    rack.drop_bombs_498 = true;
    rack.to_repeat_494 = rack.to_repeat_494 + delay;  // FLD / FADD / FSTP float
}

bool rack_is_busy_006e3cd0(const SingleRackState& rack) noexcept { return rack.drop_bombs_498; }

RackReloadEvent rack_advance_reload_006e41c0(SingleRackState& rack, int attached_rounds,
                                             float dt, float reload_mul,
                                             SessionMode mode) noexcept {
    // 006E41D5-006E41E7: the scaled step is stored to the argument slot as a float.
    const float step = static_cast<float>(static_cast<double>(reload_mul) * dt);
    if (!rack_is_empty_006e3570(rack, attached_rounds)) return RackReloadEvent::None;  // 006E41F5
    const float left = static_cast<float>(static_cast<double>(rack.reload_left_490) - step);
    rack.reload_left_490 = left;  // 006E4209
    if (!(left < 0.0f)) return RackReloadEvent::None;  // FLDZ / FCOMIP / JBE 006E4215
    if (authority(mode)) {
        // 006E422B-006E4245: B2h with argument -1 routed (1, 0) from the rack; delivered by the
        // session pump to 006E4290, whose B2h arm is rack_rearm_006e3410.
        rack.reload_left_490 = rack.reload_time_48c;  // 006E424A
        return RackReloadEvent::RouteReloadMessage;
    }
    rack.reload_left_490 = 0.0f;  // 006E4268
    return RackReloadEvent::ClientZeroed;
}

bool rack_rearm_006e3410(SingleRackState& rack, bool round_attached) noexcept {
    rack.reload_left_490 = rack.reload_time_48c;  // +490h = +48Ch
    if (round_attached) {
        rack.ammo_484 = rack.org_ammo_488 - 1;
        return false;
    }
    rack.ammo_484 = rack.org_ammo_488;
    return true;  // 006E3448 JMP [vtable+224h]
}

void rack_attach_next_round_006e3e70(SingleRackState& rack, bool rack_flag_5d) noexcept {
    if (rack_flag_5d) return;  // 006E3E77
    rack.ammo_484 -= 1;        // 006E3F84, reached from the create's success and failure
}

RackReloadEvent rack_tick_reload_slice_006e57a4(SingleRackState& rack, int attached_rounds,
                                                float dt, bool device_reload_enabled,
                                                float reload_mul, SessionMode mode) noexcept {
    // 006E577F COMISS against -1.0 / JBE: only above -1.0.
    if (rack.to_repeat_494 > kRackRepeatIdle) {
        rack.to_repeat_494 = static_cast<float>(static_cast<double>(rack.to_repeat_494) - dt);
    }
    if (!device_reload_enabled) return RackReloadEvent::None;  // 006E57B6
    return rack_advance_reload_006e41c0(rack, attached_rounds, dt, reload_mul, mode);
}

bool rack_fire_window_open_006e57d4(const SingleRackState& rack, float dt) noexcept {
    return rack.drop_bombs_498 && dt > 0.0f && rack.to_repeat_494 < 0.0f;
}

bool rack_fire_window_step_006e5816(SingleRackState& rack, bool can_fire, bool owner_is_plane,
                                    bool gate, bool flag_3b8, float repeat_time) noexcept {
    if (!can_fire) {
        // 006E58D8-006E58E1: the rack stops dropping when +3B8h is set or ammo < 1.
        if (flag_3b8 || rack.ammo_484 < 1) rack.drop_bombs_498 = false;
        return false;
    }
    // Not a plane: the owner's vtable[1E8h] is asked and dropBombs cleared before the test.
    if (!owner_is_plane) rack.drop_bombs_498 = false;
    if (!gate) return false;
    rack.to_repeat_494 = repeat_time;  // class+E0h after the ADh route (006E58A5)
    return true;
}

// ---------------------------------------------------------------------------------------------
// Multiple rack
// ---------------------------------------------------------------------------------------------

MultiRackState multi_rack_construct_006e42d0() noexcept {
    MultiRackState rack;
    rack.base = rack_construct_006e3c00();
    rack.load_timer_680 = 0.0f;  // param_1[1A0h]
    rack.display_rounds_67c = 0;
    rack.slot_timer_5b4.fill(kMultiRackSlotIdle);
    rack.slot_loaded_654.fill(false);
    // +514h and +684h are not written by 006E42D0 (the allocation is not zeroed by it); a
    // cleared state is the model's assumption.
    return rack;
}

void multi_rack_set_ammo_006e3770(MultiRackState& rack, int n) noexcept {
    rack.base.ammo_484 = n;
    rack.base.org_ammo_488 = n;
    rack.ammo_684 = n;
}

bool multi_rack_any_slot_reloading_006e35c0(const MultiRackState& rack) noexcept {
    for (float t : rack.slot_timer_5b4) {
        if (t >= 0.0f) return true;  // 0.0 <= t
    }
    return false;
}

float multi_rack_min_reload_left_006e35f0(const MultiRackState& rack) noexcept {
    float best = 0.0f;
    int n = 0;
    for (float t : rack.slot_timer_5b4) {
        if (t > 0.0f) {
            if (n == 0 || t < best) best = t;
            ++n;
        }
    }
    return best;
}

int multi_rack_count_rounds_006e3730(const MultiRackState& rack, int real_rounds,
                                     bool has_round_of_kind) noexcept {
    if (rack.display_rounds_67c > 0 && (real_rounds > 0 || has_round_of_kind)) {
        return real_rounds + rack.display_rounds_67c;
    }
    return real_rounds;
}

bool multi_rack_has_round_006e45e0(const MultiRackState& rack, bool base_has_round,
                                   bool first_display_class_matches) noexcept {
    if (base_has_round) return true;  // 006E45EE
    for (bool occupied : rack.display_round_514) {
        if (occupied) return first_display_class_matches;  // 006E461D
    }
    return false;
}

float multi_rack_ammo_fraction_006e3700(const MultiRackState& rack, int round_count) noexcept {
    // 006E3714 FILD, FSTP float (exact), 006E3720 FIDIV orgAmmo, FSTP float.
    return static_cast<float>(static_cast<double>(round_count) /
                              static_cast<double>(rack.base.org_ammo_488));
}

int multi_rack_kill_display_round_006e3790(MultiRackState& rack, int slot) noexcept {
    if (rack.display_rounds_67c <= 0) return -1;
    int s = slot;
    if (slot < 0) {
        for (int i = 0; i < kMultiRackSlots; ++i) {
            if (rack.display_round_514[at(i)]) {
                s = i;
                break;
            }
        }
        if (s < 0) return -1;  // the image reads +514h[slot] (before the array); not modelled
    }
    if (!rack.display_round_514[at(s)]) return -1;
    // BSP_MissionEntity_Kill(0) on the entity, then the slot store and the decrement. Whether the
    // kill reaches the observer 006E3840 before the store (a second decrement) was not read; the
    // model assumes it does not (the kill list 00F899B4 is drained later in the step).
    rack.display_round_514[at(s)] = false;
    rack.display_rounds_67c -= 1;
    return s;
}

std::optional<int> multi_rack_reset_slot_timer_006e37f0(MultiRackState& rack, int slot) noexcept {
    int s = slot;
    if (slot < 0) {
        s = -1;
        for (int i = 0; i < kMultiRackSlots; ++i) {
            if (0.0f > rack.slot_timer_5b4[at(i)]) {  // COMISS 0 / JA
                s = i;
                break;
            }
        }
        if (s < 0) return std::nullopt;
    }
    rack.slot_loaded_654[at(s)] = false;
    rack.slot_timer_5b4[at(s)] = rack.base.reload_time_48c;
    return s;
}

void multi_rack_display_round_died_006e3840(MultiRackState& rack, int slot) noexcept {
    // The observer matches by entity pointer over all forty entries; one entity is in one slot.
    if (slot < 0 || slot >= kMultiRackSlots || !rack.display_round_514[at(slot)]) return;
    rack.display_round_514[at(slot)] = false;
    rack.display_rounds_67c -= 1;
}

std::vector<int> multi_rack_request_loads_006e4410(const MultiRackState& rack) noexcept {
    std::vector<int> slots;
    // The image's bound is orgAmmo (+488h), unclamped (an orgAmmo above 40 reads past the arrays);
    // the model clamps it to the slot count.
    const int bound = rack.base.org_ammo_488 < kMultiRackSlots ? rack.base.org_ammo_488
                                                               : kMultiRackSlots;
    for (int i = 0; i < bound; ++i) {
        if (!rack.slot_loaded_654[at(i)] && !rack.display_round_514[at(i)]) slots.push_back(i);
    }
    return slots;
}

std::optional<int> multi_rack_advance_reload_006e44c0(MultiRackState& rack, float dt,
                                                      float reload_mul,
                                                      SessionMode mode) noexcept {
    const float step = static_cast<float>(static_cast<double>(reload_mul) * dt);  // 006E44EE
    for (int i = 0; i < kMultiRackSlots; ++i) {
        float& t = rack.slot_timer_5b4[at(i)];
        if (t < 0.0f) continue;  // COMISS / JB 006E4514 (NaN skips too)
        const float left = static_cast<float>(static_cast<double>(t) - step);
        t = left;
        if (!(left < 0.0f)) continue;  // FCOMI 0 / JBE 006E452C
        t = kMultiRackSlotIdle;
        rack.ammo_684 += 1;
        if (authority(mode)) return i;  // B2h(slot) routed (1, 0), then RET
        rack.slot_loaded_654[at(i)] = true;
    }
    return std::nullopt;
}

std::vector<int> multi_rack_trim_006e4690(MultiRackState& rack, int real_rounds) noexcept {
    std::vector<int> killed;
    if (rack.display_rounds_67c > 0) {
        int keep = rack.ammo_684 - real_rounds;  // computed once, 006E46CA
        if (keep < 0) keep = 0;
        while (keep < rack.display_rounds_67c) {
            const int s = multi_rack_kill_display_round_006e3790(rack, -1);
            if (s < 0) break;  // the image spins here; see the header
            killed.push_back(s);
        }
    }
    const int want = rack.display_rounds_67c + real_rounds;
    int counted = 0;
    for (int i = 0; i < kMultiRackSlots; ++i) {
        if (!rack.display_round_514[at(i)] && !rack.slot_loaded_654[at(i)]) {
            if (0.0f < rack.slot_timer_5b4[at(i)]) {
                // still reloading: not counted (COMISS 0 / JB 006E4776)
            } else {
                rack.slot_loaded_654[at(i)] = true;
                ++counted;
            }
        } else {
            ++counted;
        }
        if (counted >= want) break;  // 006E4783
    }
    return killed;
}

bool multi_rack_generate_real_round_006e5d70(MultiRackState& rack, int slot, bool torn_down,
                                             SessionMode mode) noexcept {
    if (torn_down) return false;  // +5Eh, +5Dh, +5Fh, +60h
    if (slot < 0 || slot >= kMultiRackSlots) return false;
    const bool spawn = authority(mode);
    multi_rack_kill_display_round_006e3790(rack, slot);
    rack.load_timer_680 = kMultiRackLoadDelay;
    rack.slot_loaded_654[at(slot)] = true;
    rack.slot_timer_5b4[at(slot)] = kMultiRackSlotIdle;
    return spawn;
}

MultiRackPreTick multi_rack_pre_tick_006e5900(MultiRackState& rack, float dt, SessionMode mode,
                                              int real_rounds) noexcept {
    MultiRackPreTick out;
    if (mode == SessionMode::Client) {
        out.trimmed = multi_rack_trim_006e4690(rack, real_rounds);  // 006E5A1A
    } else if (kMultiRackLoadTimerCeiling > rack.load_timer_680) {  // FCOMIP / JBE 006E5952
        const float left = static_cast<float>(static_cast<double>(rack.load_timer_680) - dt);
        rack.load_timer_680 = left;
        if (left < 0.0f) {  // 006E5970
            // 006E5980 counts the real rounds and drops the answer.
            for (int i = 0; i < kMultiRackSlots; ++i) {
                if (rack.display_round_514[at(i)]) {
                    out.load_slot = i;  // B3h(slot) routed (1, 0)
                    break;
                }
            }
            if (!out.load_slot) {
                rack.display_rounds_67c = 0;
                rack.load_timer_680 = kMultiRackLoadParked;
            }
        }
    }
    rack.base.to_repeat_494 = kRackRepeatIdle;  // 006E5A32, every tick
    out.clear_drop_after_tick = rack.base.drop_bombs_498 && dt > 0.0f;  // 006E5A23-006E5A4D
    return out;
}

void multi_rack_on_fire_message_006e5f54(MultiRackState& rack, int real_rounds) noexcept {
    if (rack.display_rounds_67c > 0 && real_rounds <= kMultiRackRealRoundCap) {
        rack.load_timer_680 = kRackRepeatIdle;  // [00D7A260] = -1.0
    }
}

std::vector<int> multi_rack_on_drop_message_006e5f86(MultiRackState& rack, int real_rounds,
                                                     int dropped_slot) noexcept {
    std::vector<int> killed;
    if (rack.display_rounds_67c > 0) {
        rack.ammo_684 -= 1;
        killed = multi_rack_trim_006e4690(rack, real_rounds);
    }
    multi_rack_reset_slot_timer_006e37f0(rack, dropped_slot);  // vtable[1F4h]
    return killed;
}

// ---------------------------------------------------------------------------------------------
// Projectile record leaves
// ---------------------------------------------------------------------------------------------

float projectile_speed_006e7cd0(const Seg33Vec3& velocity) noexcept {
    // 006E7CFD-006E7D0B: FMULP/FADDP in the order (x*x + y*y) + z*z, then FSTP float.
    const double x = velocity.x, y = velocity.y, z = velocity.z;
    const float squared = static_cast<float>((x * x + y * y) + z * z);
    if (!(static_cast<double>(squared) > kProjectileSpeedFloorSquared)) return 0.0f;  // JBE
    return static_cast<float>(std::sqrt(static_cast<double>(squared)));  // 00BF7030, FSTP float
}

Seg33Vec3 projectile_extrapolate_position_006e2220(const Seg33Vec3& base,
                                                   const Seg33Vec3& velocity, float t) noexcept {
    const float dx = t * velocity.x;  // 006E2234 FSTP float
    const float dy = velocity.y * t;
    const float dz = t * velocity.z;
    return Seg33Vec3{base.x + dx, base.y + dy, base.z + dz};  // FADD, FSTP float each
}

// ---------------------------------------------------------------------------------------------
// Command classes
// ---------------------------------------------------------------------------------------------

int command_class_compare_006f7b30(int slot10_a, int slot10_b) noexcept {
    return slot10_a - slot10_b;  // 006F7B46 SUB EDI,EAX
}

std::vector<std::size_t> command_menu_sort_006f7c20(const std::vector<int>& keys) {
    std::list<std::size_t> nodes;
    for (std::size_t i = 0; i < keys.size(); ++i) nodes.push_back(i);
    if (nodes.size() < 2) {  // 006F7C27: head == tail
        return std::vector<std::size_t>(nodes.begin(), nodes.end());
    }
    auto cursor = nodes.begin();
    do {
        auto best = cursor;
        for (auto scan = std::next(cursor); scan != nodes.end(); ++scan) {
            if (command_class_compare_006f7b30(keys[*best], keys[*scan]) == -1) best = scan;
        }
        if (best == cursor) {
            ++cursor;  // 006F7CF8
        } else {
            nodes.splice(std::next(cursor), nodes, best);  // best relinked after the cursor
            nodes.splice(nodes.end(), nodes, cursor);      // the cursor appended at the tail
            cursor = std::next(best);
        }
    } while (cursor != std::prev(nodes.end()));
    return std::vector<std::size_t>(nodes.begin(), nodes.end());
}

}  // namespace bsp::gameplay_seg33_cc13
