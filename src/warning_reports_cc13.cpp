#include "bsp/warning_reports_cc13.hpp"

#include "bsp/engine_seg49_64_78_cc13.hpp" // WarningObject, base constructors
#include "bsp/ship_ai_states.hpp" // party_relative_to_00803510

#include <cstdint>

namespace bsp {

namespace {

using cc13_seg49_64_78::WarningHost;
using cc13_seg49_64_78::WarningObject;
using cc13_seg49_64_78::WarningSlot;

// 005B7F60 (ECX = slot, stack entity, RET 4) and 00523B30 (the same with the
// entity read from another handle's +14h): nothing when unchanged; unlink the
// old entity (006952A0), store, link the new one (00694A60).
void set_slot(WarningHost& host, void*& current, WarningSlot slot, WarningEntity entity)
{
    void* const next = const_cast<void*>(entity);
    if (current == next) return;
    if (current != nullptr) host.observer_unlink_006952a0(current, slot);
    current = next;
    if (next != nullptr) host.observer_link_00694a60(next, slot);
}

// Kind suffix shared by several branches: ship / plane / other.
const char* ship_plane_other(WarningUnitQuery& q, WarningEntity e, bool squadron_is_plane)
{
    if (q.is_kind_5c(e, kWarningKindShip)) return "ship";
    if (squadron_is_plane && q.is_kind_5c(e, kWarningKindSquadron)) return "plane";
    if (q.is_kind_5c(e, kWarningKindPlane)) return "plane";
    return "other";
}

template <typename Host>
struct ScopedLock24 {
    Host& host;
    explicit ScopedLock24(Host& h) : host(h) { host.lock_24(); }
    ~ScopedLock24() { host.unlock_24(); }
};

// Shared opening guard of 00977DA0 and 009781E0 (00977DE9..00977E2C,
// 00978236..00978276): multiplayer, +D0h clear, slot in [0,8), clock > 4.0f.
bool capture_guard(CaptureReportHost& host)
{
    if (!host.network_session_1fe4()) return false;
    if (host.suppressed_d0()) return false;
    const std::int32_t slot = host.local_slot_18ec();
    if (slot < 0 || slot > 7) return false;
    return host.mission_clock_f876a4() > kWarningReportGrace; // COMISS / JA, JBE
}

} // namespace

bool is_local_side_unit_00968400(WarningUnitQuery& q, WarningEntity entity)
{
    return entity != nullptr && q.side_54(entity) == q.local_side_id();
}

WarningRelation local_relation_00968370(WarningUnitQuery& q, WarningEntity entity)
{
    if (entity == nullptr) return WarningRelation::Unknown;
    return q.relation_00803ce0(entity);
}

WarningObject& construct_hit_warning_00974c70(WarningObject& w, WarningHost& host,
    HitWarningQuery& q, WarningEntity target, WarningEntity attacker)
{
    // 00974C9F: 00973BA0(target, unused); the base stores +80h and links the
    // target with no null test.
    cc13_seg49_64_78::construct_entity_warning_base_00973ba0(w, host, const_cast<void*>(target));
    w.vtable = 0x00D1B308;  // 00974CB5
    w.word_6c = 0x00D1B2F0; // 00974CBB, the +6Ch observer's vtable
    w.id = "hit";           // 00974CC2 resize 3, memcpy 00D1B438
    w.area_exclusion = false; // 00974CE2 byte +84h = 0

    // 00974CF8..00974D29: the carrier (attacker+ACh) counts only when it is a
    // kind-2 object.
    WarningEntity carrier = nullptr;
    if (attacker != nullptr) {
        WarningEntity c = q.carrier_ac(attacker);
        if (c != nullptr && q.is_kind_5c(c, kWarningKindUnit)) carrier = c;
    }
    // 00974D31..00974D4B -> [ESP+11h].
    const bool attacker_local = attacker != nullptr && q.slot_1c(attacker) == q.local_slot_18ec();
    // 00974D50..00974D7E -> [ESP+12h].
    bool target_player = false;
    if (target != nullptr) {
        WarningEntity t = q.is_kind_5c(target, kWarningKindPlane) ? q.plane_owner_9d4(target) : target;
        target_player = t == q.controlled_unit_e188d8();
    }
    // 00974D83..00974DC3 -> [ESP+13h]: the attacker's +CCh is a kind-2Ah
    // object and the carrier is a local-side kind-18h squadron.
    const bool payload = attacker != nullptr
        && q.is_kind_5c(q.weapon_cc(attacker), kWarningKindPayloadWeapon)
        && carrier != nullptr && q.is_kind_5c(carrier, kWarningKindSquadron)
        && is_local_side_unit_00968400(q, carrier);

    // 00974DC8..00974ED6: a null target counts as relation 3.
    const WarningRelation relation = target == nullptr ? WarningRelation::Unknown
                                                       : q.relation_00803ce0(target);

    if (relation == WarningRelation::Own && !attacker_local) {
        // 00974EDC: our unit was hit by someone else.
        w.id += "weareunderattack";
        if (q.is_kind_5c(target, kWarningKindShip)) {
            w.id += "ship";
        } else if (q.is_kind_5c(target, kWarningKindSquadron)) {
            w.id += "planesquadron";
        } else if (q.is_kind_5c(target, kWarningKindCommandBuilding)) {
            // 00975006..0097507B
            if (!q.is_base_78c(target)) {
                w.id += "navalsupply";
            } else if (q.owner_slot_180(target) == q.local_slot_18ec()) {
                w.id += "commandbuildingplayer";
            } else if (q.count_command_buildings_of_slot(q.local_slot_18ec()) == 1) {
                w.id += "commandbuildinglast";
            }
        } else {
            w.id += "other";
        }
        // 009750C4..0097512F
        if (target_player) {
            w.id += "player";
        } else {
            set_slot(host, w.reporter, WarningSlot::Reporter54, target);   // 00975139 005B7F60
            set_slot(host, w.shortcut, WarningSlot::Shortcut24, w.reporter); // 00975142 00523B30
        }
        w.area_exclusion = true; // 00975112 byte +84h = 1
        return w;
    }

    if (attacker_local && relation != WarningRelation::Enemy) {
        // 00974DE9: the player's side hit something not hostile.
        w.id += "playermistake";
        if (relation != WarningRelation::Own) return w; // 00974E2E
        w.id += "friendlyfire";
        w.id += ship_plane_other(q, target, false);       // 00974E77..00975186
        set_slot(host, w.reporter, WarningSlot::Reporter54, target); // 009751A3 005B7F60
        return w;
    }

    // 009751BE: neither of the above.
    if (!payload) return w;
    w.id += "payloadhit";
    if (attacker_local) {
        w.id += "player";
        return w;
    }
    w.id += "plane";
    w.id += "ambient";
    set_slot(host, w.ambient, WarningSlot::Ambient3C, carrier); // 00975317 005B7F60
    return w;
}

WarningObject& construct_kill_warning_009744c0(WarningObject& w, WarningHost& host,
    KillWarningQuery& q, WarningEntity victim)
{
    cc13_seg49_64_78::construct_warning_base_00973a90(w, host); // 009744E7
    w.vtable = 0x00D1B2CC;                                      // 009744F1
    // 009744F7: +6Ch is the victim as a plain pointer (no observer handle).
    w.word_6c = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(victim));
    w.id = "kill"; // 00974506 resize 4, memcpy 00D18A84

    // 00974510..00974545: victim+2C4h when it is a kind-2 object.
    WarningEntity killer = q.killer_2c4(victim);
    if (killer != nullptr && !q.is_kind_5c(killer, kWarningKindUnit)) killer = nullptr;

    switch (q.relation_00803ce0(victim)) {
    case WarningRelation::Own:
        w.id += "own";
        if (q.is_kind_5c(victim, kWarningKindShip)) {
            w.id += "ship";
        } else if (q.is_kind_5c(victim, kWarningKindPlane)) {
            w.id += "plane";
            w.id += "ambient";
            set_slot(host, w.ambient, WarningSlot::Ambient3C, victim); // 009746EB 005B7F60
        } else if (q.is_kind_5c(victim, kWarningKindSquadron)) {
            w.id += "planesquadron";
        } else if (q.is_kind_5c(victim, kWarningKindAirfield)) {
            w.id += "airfield";
        }
        return w;

    case WarningRelation::Enemy: {
        w.id += "enemy";
        // 009747D5..009747FD: 00965810 is asked about the killer (may be null).
        const bool player_kill = q.is_player_controlled_00965810(killer)
            && q.local_slot_18ec() == q.slot_2d8(victim);
        const bool ship = q.is_kind_5c(victim, kWarningKindShip);
        const bool squadron = !ship && q.is_kind_5c(victim, kWarningKindSquadron);
        const bool airfield = !ship && !squadron && q.is_kind_5c(victim, kWarningKindAirfield);
        if (!ship && !squadron && !airfield) {
            if (!q.is_kind_5c(victim, kWarningKindPlane)) return w;
            w.id += "plane";
            w.id += "ambient";
            set_slot(host, w.ambient, WarningSlot::Ambient3C, killer); // 009748B0 005B7F60
            if (killer == nullptr) return w;
            w.id += ship_plane_other(q, killer, true);
            if (player_kill) w.id += "player";
            return w;
        }
        w.id += ship ? "ship" : squadron ? "planesquadron" : "airfield";
        // 00974A22..00974A33: only a killer of our own side is named.
        if (killer != nullptr && local_relation_00968370(q, killer) == WarningRelation::Own) {
            set_slot(host, w.reporter, WarningSlot::Reporter54, killer); // 00974A41 005B7F60
            w.id += ship_plane_other(q, killer, true);
            if (player_kill) w.id += "player";
            return w;
        }
        w.id += "unknown"; // 00CEB808
        return w;
    }

    case WarningRelation::Neutral:
        w.id += "neutral";
        w.id += "ambient";
        set_slot(host, w.ambient, WarningSlot::Ambient3C, victim); // 00974BB8 005B7F60
        if (q.is_kind_5c(victim, kWarningKindShip)) {
            w.id += "ship";
        } else if (q.is_kind_5c(victim, kWarningKindPlane)) {
            w.id += "plane";
        }
        return w;

    case WarningRelation::Unknown:
    default:
        return w;
    }
}

bool is_entity_reportable_009761d0(CaptureReportHost& host, WarningEntity entity)
{
    const WarningEntityFlags flags = host.entity_flags(entity);
    if (!flags.present_5c || flags.flag_5d || flags.flag_60 || flags.flag_5e) return false;
    return !host.entity_suppressed_0077edf0(entity);
}

std::vector<std::string> report_capture_ship_00977da0(CaptureReportHost& host,
    WarningEntity subject, WarningEntity point)
{
    std::vector<std::string> reported;
    ScopedLock24<CaptureReportHost> lock(host);
    if (!capture_guard(host)) return reported;
    const std::int32_t slot = host.local_slot_18ec();

    // 00977E36..00977EA0. With both arguments null the native reads
    // point+180h through a null pointer.
    if (subject == nullptr && host.owner_slot_180(point) == slot) {
        host.report_message_warning_00975ac0(kCaptureShipRangeId);
        reported.emplace_back(kCaptureShipRangeId);
    }
    if (point != nullptr || subject == nullptr) return reported; // 00977EC2..00977ECF

    // 00977ED5..00977F83
    const WarningVec3 at = host.world_position(subject);
    std::string id(kCaptureShipId);
    id += host.is_base_78c(subject) ? "base" : "supply";
    const float range = host.capture_range_218(subject); // FSTP dword: float

    // 00977F87..0097802B: the first list-6 unit within range that is not
    // kind 0Ch and belongs to the local slot.
    for (WarningEntity unit : host.unit_list_64()) {
        const WarningVec3 p = host.world_position(unit);
        // Each difference is stored as a float (FSTP dword), then
        // (dy*dy + dx*dx) + dz*dz on the x87 stack, stored as a float.
        const float dx = p.x - at.x;
        const float dy = p.y - at.y;
        const float dz = p.z - at.z;
        const float d2 = static_cast<float>(
            (static_cast<double>(dy) * dy + static_cast<double>(dx) * dx)
            + static_cast<double>(dz) * dz);
        if (!(range > d2)) continue; // FCOMIP range, d2; JBE skips
        if (host.is_kind_5c(unit, kWarningKindCaptureExcluded)) continue;
        if (host.owner_slot_180(unit) != slot) continue;
        // 00978036..00978054: subject+7B0h against the local side.
        id += host.side_7b0(subject) == host.local_side_id() ? "own" : "nmy";
        host.report_message_warning_00975ac0(id);
        reported.push_back(id);
        break;
    }
    return reported;
}

std::optional<CapturePointReport> report_capture_point_009781e0(CaptureReportHost& host,
    WarningEntity point, std::int32_t slot_arg, std::int32_t party_arg)
{
    ScopedLock24<CaptureReportHost> lock(host);
    if (!capture_guard(host)) return std::nullopt;
    if (!is_entity_reportable_009761d0(host, point)) return std::nullopt; // 00978282

    // 0097828F..009782BE
    const std::int32_t slot = host.local_slot_18ec();
    const std::int32_t side = host.local_side_id();
    const std::int32_t current = host.side_54(point);
    const std::int32_t previous = host.side_7b0(point);
    constexpr std::int32_t kNeutralParty = 2;

    CapturePointReport r;
    r.id = "capture"; // 00D1B654
    if (host.is_kind_5c(point, kWarningKindCommandBuilding)) {
        r.id += "commandbuilding"; // 00D1B644
        const bool base = host.is_base_78c(point);
        const bool neutralize = previous != kNeutralParty && party_arg != kNeutralParty
            && current == kNeutralParty;                         // 00978324..00978341
        const bool capture = current != kNeutralParty;           // 009785D6, 0097833E
        if (neutralize) {
            r.id += "neutralize";
            // 00E0CFE0 = {"own", "nmy", "neu", "unk"} indexed by the relation.
            const int rel = party_relative_to_00803510(side, party_arg);
            if (rel == 1) {
                r.id += "nmy";
                if (!base) {
                    r.id += "supply";
                } else {
                    r.id += "base";
                    // 009783FF..0097845E: the controlled unit is a ship
                    // within the point's range. LengthSquared is stored as a
                    // double (FSTP qword) before the compare.
                    WarningEntity player = host.controlled_unit_e188d8();
                    if (player != nullptr && host.is_kind_5c(player, kWarningKindShip)) {
                        const WarningVec3 a = host.world_position_00427eb0(point);
                        const WarningVec3 b = host.world_position_00427eb0(player);
                        const float dx = b.x - a.x;
                        const float dy = b.y - a.y;
                        const float dz = b.z - a.z;
                        const double d2 = static_cast<double>(dx) * dx
                            + static_cast<double>(dy) * dy + static_cast<double>(dz) * dz;
                        if (static_cast<double>(host.capture_range_218(point)) > d2) {
                            r.id += "player";
                        }
                    }
                }
            } else if (rel == 0) {
                r.id += "own";
                if (base) {
                    r.id += "base";
                    // 0097851E..009785B7
                    if (slot_arg == slot) {
                        const std::int32_t left = host.count_command_buildings_of_slot(slot);
                        r.id += left == 1 ? "oneleft" : left == 0 ? "last" : "player";
                    }
                }
            }
        } else if (capture && (previous == kNeutralParty || party_arg == kNeutralParty)) {
            r.id += "cap"; // 00CE8308
            r.id += previous == kNeutralParty ? "neut" : "nmy"; // 0097860E, 00978767
            if (current == side) {
                r.id += "own";
                if (!base) {
                    r.id += "supply";
                } else {
                    r.id += "base";
                    if (host.owner_slot_180(point) == slot) r.id += "player";
                }
            } else {
                r.id += "nmy";
                r.id += base ? "base" : "supply";
            }
        } else if (capture) {
            r.id += "cap"; // no suffix: 0097876A takes neither branch
        }
    }

    // 009788D0..00978953
    if (!host.is_message_id_known_0096dbb0(r.id)) {
        // The native passes the id's text pointer to "%d" (00D1B5F8), so the
        // printed value is an address; the id is then emptied.
        host.debug_print_004254b0("---WarningManager invalid Message--- %d");
        r.reported = false;
        return r;
    }
    host.debug_print_004254b0("Capture warning msg"); // 00D1B5E4
    host.report_entity_warning_00974150(point, r.id);
    r.reported = true;
    return r;
}

std::string report_command_building_00978a70(CaptureReportHost& host, WarningEntity record)
{
    std::string id = "hitweareunderattack"; // 00978AA1 resize 13h, 00D1B65C
    switch (host.record_state_18(record)) { // 00978AC9
    case 2: id += "navalsupply"; break;           // 00978AD5, 00D1B3F8
    case 0: id += "commandbuildingplayer"; break; // 00978B13, 00D1B3E0
    case 1: id += "commandbuildinglast"; break;   // 00978B7A, 00D1B3CC
    default: break;
    }
    // 00978BC7..00978BF9: new 6Ch, 00975AC0(id), 009763E0(manager, warning).
    host.report_message_warning_00975ac0(id);
    return id;
}

const void* message_from_0096e5e0(MessageFromHost& host, WarningEntity speaker)
{
    const std::string tail = "|.: ";                   // 0096E60D, 00D1B098
    const std::string name = host.name_10(speaker);    // 0096E63E vtable +10h
    const std::string head = "Message from |.";        // 0096E65F resize 0Fh, 00D1B088
    const std::string text = head + name + tail;       // 0096E695, 0096E6AB 004261A0
    return host.resolve_message_00705e00(text, 1);     // 0096E748
}

namespace {

// The virtual call at 009768C7..009768CF: [[warning]+4](reader). Slot +4 of
// each class vtable, read from the image: 00D1B4FC -> 0097B4D0,
// 00D1B270/00D1B348/00D1B384 -> 00976C60, 00D1B2AC -> 00976E40,
// 00D1B30C -> 00977000, 00D1B2D0/00D1B464/00D1B4B4 -> 009768F0.
void call_load_slot(WarningArchiveFactoryHost& host, WarningObject& w,
                    cc13_seg49_64_78::EventArchiveReader& reader)
{
    switch (w.vtable) {
    case 0x00D1B4F8: host.string_list_warning_load_0097b4d0(w, reader); break;
    case 0x00D1B26C:
    case 0x00D1B344:
    case 0x00D1B380: cc13_seg49_64_78::load_entity_warning_00976c60(w, reader, host); break;
    case 0x00D1B2A8: cc13_seg49_64_78::load_recon_warning_00976e40(w, reader, host); break;
    case 0x00D1B308: cc13_seg49_64_78::load_hit_warning_00977000(w, reader, host); break;
    case 0x00D1B2CC:
    case 0x00D1B460:
    case 0x00D1B4B0: cc13_seg49_64_78::load_warning_009768f0(w, reader, host); break;
    default: break;  // unreachable: every constructor below stores one of these nine
    }
}

} // namespace

WarningObject* warning_create_from_archive_009766d0(WarningArchiveFactoryHost& host,
                                                    cc13_seg49_64_78::EventArchiveReader& reader)
{
    // [ESP+0Ch] is not initialised before the read; a reader that leaves it
    // alone hands the switch stack contents. Modelled as -1 (no class).
    std::int32_t type = -1;
    reader.read_10(gui_lua_key_by_name("type"),                       // 00CE4AD0
                   gui_lua_field(GuiLuaFieldType::Int, &type));       // 00976711 vtable +10h
    WarningObject* w = nullptr;                                       // ESI = 0 at 0097670F
    switch (type) {
    case 0:
        w = host.operator_new_00bf681b(0x7C);                         // 0097671D
        if (w != nullptr) cc13_seg49_64_78::construct_string_list_warning_for_archive_00976230(*w, host);
        break;
    case 2:
        w = host.operator_new_00bf681b(0x9C);                         // 0097674B
        if (w != nullptr) cc13_seg49_64_78::construct_recon_warning_for_archive_00973ca0(*w, host);
        break;
    case 3:
        // 00973D30's body, inlined: base constructor then the class vtable.
        w = host.operator_new_00bf681b(0x70);                         // 0097677A
        if (w != nullptr) {
            cc13_seg49_64_78::construct_warning_base_00973a90(*w, host);  // 00976796
            w->vtable = 0x00D1B2CC;                                   // 0097679B
            w->type = 3;                                              // the class's vtable +10h
        }
        break;
    case 4:
        w = host.operator_new_00bf681b(0x88);                         // 009767B7
        if (w != nullptr) cc13_seg49_64_78::construct_hit_warning_for_archive_00973d90(*w, host);
        break;
    case 5:
        w = host.operator_new_00bf681b(0x84);                         // 009767E9
        if (w != nullptr) cc13_seg49_64_78::construct_exit_zone_warning_for_archive_00973e10(*w, host);
        break;
    case 1:
        w = host.operator_new_00bf681b(0x84);                         // 0097681B
        if (w != nullptr) cc13_seg49_64_78::construct_entity_warning_for_archive_00973c20(*w, host);
        break;
    case 6:
        w = host.operator_new_00bf681b(0x84);                         // 0097684A
        if (w != nullptr) cc13_seg49_64_78::construct_confirm_warning_for_archive_00973e90(*w, host);
        break;
    case 7:
        w = host.operator_new_00bf681b(0x6C);                         // 00976872
        if (w != nullptr) construct_new_unit_warning_009756a0(*w, host);
        break;
    case 8:
        w = host.operator_new_00bf681b(0x6C);                         // 0097689A
        if (w != nullptr) construct_powerup_warning_009757b0(*w, host, host.network_session_1fe4());
        break;
    default:
        break;                                                        // 00976896 JNZ 009768C7
    }
    // 009768C7: no null test; a null w faults here as the native does.
    call_load_slot(host, *w, reader);
    return w;                                                         // 009768D6 EAX = ESI
}

} // namespace bsp
