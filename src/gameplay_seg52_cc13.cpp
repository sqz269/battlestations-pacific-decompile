// Segment 52 (008288D0-00851CB0) reference models. Packet cc13-gpr52;
// include/bsp/gameplay_seg52_cc13.hpp carries the per-function evidence and the
// adoption note, docs/CC13_GPR_52.md the tables.
// Reconstructed and build-tested only: not wired, not game-run.

#include "bsp/gameplay_seg52_cc13.hpp"

#include <cstring>

#include "bsp/band_0080_cc13.hpp"  // stationary_check_destroyed_0084eb80

namespace bsp {
namespace gameplay_seg52_cc13 {

namespace {

constexpr float kZero = 0.0f;  // 00D7A218

bool kind_1_or_2(std::int32_t kind) noexcept {
    // 006BC030 and the inline copies at 0084DC7A / 0084DCC4: CMP 1, CMP 2.
    return kind == 1 || kind == 2;
}

}  // namespace

bool entity_live(const EntityLiveBytes& e) noexcept {
    return e.b5c && !e.dead_5d && !e.b60 && !e.b5e;
}

// --- shipyard ---------------------------------------------------------------

std::optional<std::size_t> shipyard_first_live_hangar_00845750(
    const std::vector<ShipyardHangarSlot>& slots) noexcept {
    // 00845770..008457BC: the walk, 10h per record; 008457C8 returns record+4.
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (slots[i].has_object && entity_live(slots[i].object)) return i;
    }
    return std::nullopt;  // 008457CF: EAX = 0
}

Vec3f shipyard_launch_position_00845870(const std::vector<ShipyardHangarSlot>& slots,
    const std::function<Vec3f(std::size_t)>& hangar_position_c4,
    const Vec3f& shipyard_world_position_0fc) {
    // 00845873 CALL 00845750; 00845884 the object's vtable +C4h(out).
    if (const auto first = shipyard_first_live_hangar_00845750(slots)) {
        return hangar_position_c4(*first);
    }
    // 00845893..008458BE: 00414DB0 when +C8h is clear, then +FCh..+104h.
    return shipyard_world_position_0fc;
}

ShipyardAimPick shipyard_aim_point_pick_00844a10(const std::vector<ShipyardHangarSlot>& slots,
    const std::function<float(int stream, float lo, float hi)>& uniform) {
    ShipyardAimPick pick;
    // First walk: n live records with a non-null object, the first one kept.
    std::size_t n = 0;
    std::optional<std::size_t> first;
    for (std::size_t i = 0; i < slots.size(); ++i) {
        if (!slots[i].has_object || !entity_live(slots[i].object)) continue;
        if (!first) first = i;
        ++n;
    }
    pick.slot = first;
    // 00844A94: CMP n, 1; JLE skips the draw.
    if (n > 1) {
        // 00844A9A..00844AB9: FLDZ (lo), CVTSI2SS n - 1 (hi), ECX = 1, 00BD2F10,
        // then _ftol 00BF7420 (truncation).
        const float u = uniform(1, kZero, static_cast<float>(static_cast<int>(n) - 1));
        pick.draws = 1;
        int k = static_cast<int>(u);
        // Second walk: the k-th live record; with none left the first one stands.
        pick.slot = first;
        for (std::size_t i = 0; i < slots.size(); ++i) {
            if (!slots[i].has_object || !entity_live(slots[i].object)) continue;
            if (k < 1) {  // 00844B15: JLE breaks with this record
                pick.slot = i;
                break;
            }
            --k;
        }
    }
    return pick;
}

void shipyard_hangars_lost_00845ac0(
    const std::vector<ShipyardHangarSlot>& slots, bool adopted_71c, ShipyardHangarsLostHost& host) {
    // The walk returns at the first live object; the image does not test record+4
    // for null, a null object is taken as not live here.
    if (shipyard_first_live_hangar_00845750(slots)) return;
    // 00845B4E: CMP [ESI+71Ch], 0; JE to vtable +70h(1) at 00845BEA (the PUSH 1 at
    // 00845B55 is the +70h argument on that arm and the resize flag on the other).
    if (!adopted_71c) {
        host.shipyard_vslot70(1);
        return;
    }
    // 00845B5D..00845BD3: the temporary "InferiorFailure" (00CF0B74), +194h(&name, 0),
    // then the pool release of its buffer.
    static constexpr char kInferiorFailure[] = "InferiorFailure";
    ShipyardNativeString name;
    host.native_string_resize_0041dd40(name, 0xF, 1);
    if (name.data != nullptr) std::memcpy(name.data, kInferiorFailure, name.length + 1);
    host.shipyard_vslot194(&name, 0);
    if (name.data != nullptr) {
        void* pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, name.data, name.length + 1, 1);
    }
}

ShipyardMessageArm shipyard_message_arm_00847030(std::uint8_t kind) noexcept {
    // 00847034..00847043: MOVZX kind; ADD -A7h; CMP 3; JA default; table 008470A4.
    switch (kind) {
    case 0xA7: return ShipyardMessageArm::ChangeSlotClass_00844d60;
    case 0xA8: return ShipyardMessageArm::ChangeSlotEquipment_008436f0;
    case 0xA9: return ShipyardMessageArm::LaunchSlot_00846d90;
    case 0xAA: return ShipyardMessageArm::ApplySpawnMessage_008437d0;
    default: return ShipyardMessageArm::UnitHandleMessage_0095abe0;
    }
}

// --- turning guns -------------------------------------------------------------

bool turning_gun_fire_if_ready_0084c4e0(bool fired, bool& request_4d4) noexcept {
    // 0084C4E3 CALL 00727E30; 0084C4EC MOV byte [ESI+4D4h], 0 on a shot.
    if (!fired) return false;
    request_4d4 = false;
    return true;
}

bool turning_gun_extra_fire_request_0084c5b0(bool request_4d4_after_tick) noexcept {
    // 0084C5D4 CALL 0072D130; 0084C5DB CMP byte [ESI+1C4h], 0; JE skips the send.
    return request_4d4_after_tick;
}

// --- weapon director ------------------------------------------------------------

bool director_release_fire_target_00835f60(const FireTargetView& target,
                                           std::int32_t owner_party_54) noexcept {
    // 00835F84: no target, nothing. 00835F93: IsKind(41h) keeps it.
    if (!target.present || target.is_kind_41) return false;
    // 00835FA3..00835FC0: live and of another party, else release.
    if (entity_live(target.live) && target.party_54 != owner_party_54) {
        // 00835FC9: IsKind(1Ch) keeps it.
        if (target.is_kind_1c) return false;
        // 00835FDC: 005457C0 with ECX = owner: 1 when the owner's party differs
        // and the target's party is not 2.
        const bool hostile = owner_party_54 != target.party_54 && target.party_54 != 2;
        if (hostile) return false;
    }
    return true;  // 00835FE5..0083602A: message 5Eh, routed (7, 0)
}

RetargetResult director_retarget_command_slot_00835b40(const RetargetInputs& in) noexcept {
    RetargetResult out;
    // 00835B6C..00835B77: 0071EDD0(target, primary), for both outcomes of the compare.
    if (!in.base_answer_0071edd0) return out;
    out.answer = true;
    // 00835B93..00835BC5: BL (same object), mode != 2, moveto or attackmove.
    if (in.same_object && in.session_mode_1fe4 != 2
        && (in.slot_command == kCmdMoveTo || in.slot_command == kCmdAttackMove)) {
        if (in.primary) {
            out.raise_primary_stage_2 = true;   // 00835BCF CALL 0071D810
        } else {
            out.raise_override_stage_2 = true;  // 00835BDD CALL 0071D9E0
        }
    }
    return out;
}

// --- plane squadron command block ---------------------------------------------------

bool squadron_slot_order_still_valid_0084dae0(const SlotStillValidInputs& in) noexcept {
    // 0084DAE9 CALL 0071DEE0(slot).
    if (!in.has_active_order_0071dee0) return false;
    // 0084DAF9..0084DB3D: slot 0, moveto, the target resolves, and +13Ch equal.
    if (in.slot == 0 && in.slot0_command == kCmdMoveTo && in.slot0_target_resolves
        && in.target_13c == in.owner_13c) {
        return false;
    }
    return true;
}

bool squadron_accepts_queued_command_0084db90(const AcceptQueuedInputs& in) noexcept {
    // 0084DB99 CALL 0071C0C0(command).
    if (!in.has_free_slot_0071c0c0) return false;
    // 0084DBAA CALL 0071BE60; JLE answers 1 with nothing queued.
    if (in.filled_0071be60 <= 0) return true;
    const std::uint32_t last = in.last.command;
    // 0084DBC9..0084DBDF: never behind land, returntobase or retreat.
    if (last == kCmdLand || last == kCmdReturnToBase || last == kCmdRetreat) return false;
    // 0084DBE1..0084DC18: behind a moveto whose target resolves.
    if (last == kCmdMoveTo && in.last.target_resolves) {
        if (in.command_kind_1_or_2) return true;
        return in.command == kCmdMoveTo || in.command == kCmdMoveOnPath;
    }
    // 0084DC22..0084DC2F: behind a kind-1/2 command only a kind-1/2 command queues.
    if (!kind_1_or_2(in.last.kind)) return true;
    return in.command_kind_1_or_2;
}

MakeRoomResult squadron_make_room_for_command_0084dc50(const MakeRoomInputs& in) noexcept {
    MakeRoomResult out;
    // 0084DC5F CALL 0071E550(command, target).
    if (in.base_answer_0071e550) {
        out.answer = true;
        return out;
    }
    const bool incoming_1_or_2 = kind_1_or_2(in.command_kind);
    // 0084DC7A..0084DC99: otherwise only a moveto with a resolving target goes on.
    if (!incoming_1_or_2 && (in.command != kCmdMoveTo || !in.command_target_resolves)) {
        return out;
    }
    // 0084DC9D CALL 0071BE60; JLE answers 0.
    if (in.filled_0071be60 <= 0) return out;
    const std::int32_t last = in.filled_0071be60 - 1;
    // 0084DCC4..0084DCDF: both kinds 1/2 drop the last slot. The image asks the
    // incoming command's vtable +0Ch a second time here (0084DCD5).
    bool drop = kind_1_or_2(in.last.kind) && incoming_1_or_2;
    // 0084DCE1..0084DCFD: a resolving moveto in the last slot is dropped.
    if (!drop && in.last.command == kCmdMoveTo && in.last.target_resolves) drop = true;
    if (drop) {
        out.drop_slot = last;  // 0084DD02 CALL 0071D900(last)
        out.answer = true;
    }
    return out;
}

bool squadron_filter_self_target_command_0084dd20(std::uint32_t command,
    SceneCommandTargetView& target, bool resolves_to_own_unit,
    const Vec3f& zero_point_f87574) noexcept {
    // 0084DD25: CMP byte [target], 0; 0084DD2F..0084DD37: 00521EA0 == block+34h.
    if (target.b0 == 0 || !resolves_to_own_unit) return false;
    if (command == kCmdMoveTo) return true;  // 0084DD42
    if (command != kCmdStop) return false;   // 0084DD49
    // 0084DD4B..0084DD8A.
    target.point_08 = zero_point_f87574;
    target.b0 = 0;
    target.w2 = 0;
    target.d4 = 0;
    target.f14 = 0.0f;
    return true;
}

bool director_normalize_self_target_00836040(std::uint32_t command,
    SceneCommandTargetView& target, bool resolves_to_own_unit,
    const Vec3f& zero_point_f87574) noexcept {
    // 00836045 / 0083604F..00836057: byte +0 set and 00521EA0 == director+34h.
    if (target.b0 == 0 || !resolves_to_own_unit) return false;
    // 0083605D / 00836064: cruise or stop.
    if (command != kCmdCruise && command != kCmdStop) return false;
    // 0083606B..008360AB.
    target.point_08 = zero_point_f87574;
    target.b0 = 0;
    target.w2 = 0;
    target.d4 = 0;
    target.f14 = 0.0f;
    return true;
}

// --- support site -------------------------------------------------------------------

SupportSiteConfig support_site_activate_0084d170(const SupportSiteProperties& props) {
    SupportSiteConfig site;
    // Defaults 4, 2, 1 and 0E6h when the property is absent.
    if (props.max_planes) site.max_planes_334 = *props.max_planes;
    if (props.max_ships) site.max_ships_338 = *props.max_ships;
    if (props.sp_active) site.sp_active_340 = *props.sp_active;
    if (props.angle) site.angle_33c = *props.angle;

    // 0084D25D: EBX = 1, once, for both walks.
    std::int32_t index = 1;
    const auto walk = [&](int list, std::vector<SupportStockEntry>& out) {
        for (;; ++index) {
            const SupportStockBag bag = props.stock ? props.stock(list, index) : SupportStockBag{};
            if (!bag.bag) break;  // absent, not type 6, or a null bag
            const std::int32_t count = bag.count.value_or(0);
            if (count == 0) continue;
            SupportStockEntry entry;
            entry.type = bag.type;
            entry.count = count;
            entry.squad_size = bag.squad_size.value_or(3);
            // 0084D2F6..0084D304: 00964790(type) unless the effective mode is 4.
            if (props.effective_game_mode != 4) site.class_loads.push_back(entry.type);
            out.push_back(entry);
        }
    };
    walk(0, site.japan_31c);   // "JapanList" -> +31Ch
    walk(1, site.allied_328);  // "AlliedList" -> +328h, from the same index
    site.field_344 = 0;
    return site;
}

void support_site_set_active_0084ce60(SupportSiteConfig& site, bool active) noexcept {
    site.sp_active_340 = active;
}

bool support_site_can_supply_class_0084d050(bool active_340,
    const std::vector<SupplyClassEntry>& entries, std::int32_t class_id,
    bool owner_present, float owner_health) noexcept {
    if (!active_340) return false;
    for (const SupplyClassEntry& e : entries) {
        if (e.class_id == class_id && !e.enabled) return false;
    }
    if (owner_present) {
        // 0084D104..0084D10C: FLDZ; FXCH; FCOMIP health, 0; JBE refuses <= 0 and NaN.
        if (!(owner_health > 0.0f)) return false;
    }
    return true;
}

std::vector<SupportStockEntry> support_site_build_supply_class_list_0084d560(
    const SupplyClassListInputs& in) {
    std::vector<SupportStockEntry> out;
    if (in.owner_present && !entity_live(in.owner)) return out;  // 0084D5B1..0084D5CF -> 0084D78C
    if (!in.site) return out;
    // 0084D5D5..0084D5E1: team 0 -> +328h (AlliedList), else +31Ch (JapanList).
    const std::vector<SupportStockEntry>& src = in.team == 0 ? in.site->allied_328 : in.site->japan_31c;

    std::int32_t key = 0x61;  // 0084D617
    if (in.session && in.flag && in.player_unit_present) key = in.player_unit_key_1c;  // 0084D649
    const bool drop_kind_6 = in.command_building_344 && in.shipyard_failure_46;   // 0084D659
    const bool drop_kind_0f = in.command_building_344 && in.airfield_failure_45;  // 0084D675

    bool marked = false;
    for (const SupportStockEntry& e : src) {
        const SupplyClassView cls = in.class_of ? in.class_of(e.type) : SupplyClassView{};  // 0084D6BC
        if (drop_kind_6 && cls.kind_6) continue;     // 0084D6CF..0084D6D7 -> unlink 0084D6FF
        if (drop_kind_0f && cls.kind_0f) continue;   // 0084D6E5..0084D6ED
        out.push_back(e);
        if (cls.key_1c == key) marked = true;        // 0084D728..0084D734
    }
    if (marked) {
        // 0084D74D: the player unit's +70h; with no unit (key 61h matched) the image
        // dereferences null here, so the model needs the unit present.
        SupportStockEntry added;
        added.type = in.player_unit_class_70;
        added.count = -1;
        added.squad_size = 3;
        out.push_back(added);
    }
    return out;
}

// --- shipyard stock registration -------------------------------------------------

std::vector<ShipyardStockRegistration> shipyard_register_stock_008433c0(
    std::int32_t party,
    const std::function<std::optional<std::int32_t>(std::int32_t index)>& stock_type) {
    std::vector<ShipyardStockRegistration> out;
    for (std::int32_t k = 1; k <= 20; ++k) {  // 008433D6, 00843446..0084344C
        const std::optional<std::int32_t> type = stock_type ? stock_type(k) : std::nullopt;
        if (!type || *type == 0) continue;     // 008433FF..0084340E, 0084341C
        out.push_back({k, *type, party});      // 00843435 0095BA60, 00843441 0046BE90
    }
    return out;
}

// --- stationary props ---------------------------------------------------------------

StationaryHitResult stationary_apply_hit_0084f020(bool dead_5d, float hit_damage,
    float protection_150, float max_damage_17c, float& damage_1a0) noexcept {
    StationaryHitResult out;
    if (dead_5d) return out;  // 0084F023
    out.applied = true;
    // 0084F052..0084F06B: COMISS damage, 0.0f; JBE; FLD; FADD [+1A0h]; FSTP float.
    if (hit_damage > kZero) {
        damage_1a0 = hit_damage + damage_1a0;
        out.damage_added = true;
    }
    // 0084F073 CALL 0084EB80.
    out.check_result = static_cast<int>(cc13_band0080::stationary_check_destroyed_0084eb80(
        protection_150, max_damage_17c, damage_1a0));
    return out;
}

}  // namespace gameplay_seg52_cc13
}  // namespace bsp
