// The kill-credit chain: how a damaging hit stamps an attribution block on the victim
// (0077CE60) and how the unit's destruction turns that block into scoring-record entries,
// per-type tallies and awards (0091BDA0). Also the two projectile flags the impact-effect
// step and the flak burst bias are gated on.
//
// Packet cc2_kill_credit, worktree agent/cc2-kill-credit. Ghidra was READ-ONLY for this
// packet. Every name here is a hypothesis, not a recovered symbol.
//
// docs/KILL_CREDIT.md carries the evidence. The 284h scoring record, its manager arithmetic
// and the two kill trees come from docs/SCORING_BINDING_TABLE.md and
// include/bsp/scoring_bodies.hpp; the hit record from include/bsp/unit_hit_path.hpp; the
// flak projectile offsets from include/bsp/projectile_helpers.hpp. All three are reused.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "bsp/projectile_helpers.hpp"
#include "bsp/scoring_bodies.hpp"
#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Class ids the two routines test through vtable[5Ch] (IsKindOf)
// ---------------------------------------------------------------------------
// Ids are docs/ENTITY_CLASS_IDS.md's; the names are that table's.
inline constexpr int kKillCreditKindAttributionBase = 0x02; // the class carrying +2B0h..+2E8h
inline constexpr int kKillCreditKindShipBase = 0x06;
inline constexpr int kKillCreditKindDestroyer = 0x07;       // MDestroyer
inline constexpr int kKillCreditKindMothership = 0x09;      // MMothership
inline constexpr int kKillCreditKindCruiser = 0x0A;         // MCruiser
inline constexpr int kKillCreditKindBattleship = 0x0D;      // MBattleship
inline constexpr int kKillCreditKindPlaneBase = 0x0F;
inline constexpr int kKillCreditKindKamikazePlane = 0x17;   // MPlaneKamikaze
inline constexpr int kKillCreditKindLandFort = 0x1B;        // MLandFort

// 0077D0DE forces the attacker class to this value on the kamikaze branch.
inline constexpr int kKillCreditKamikazeAttackerClass = 0x0C;

// 0091BF86 `CMP EAX,0x7` then `JA`, an unsigned test, on both slot fields.
inline constexpr int kKillCreditMaxPlayerSlot = 7;

// 0077CFB7 `CMP EBX,0x8` then `JBE`: the credited slot falls back only above 8, not above 7.
inline constexpr int kKillCreditSlotFallbackAbove = 8;

// ---------------------------------------------------------------------------
// The attribution block on the victim, offsets fixed by their writer 0077CE60
// ---------------------------------------------------------------------------
inline constexpr std::size_t kUnitAttributionRefOffset = 0x2B0;          // 0077CF7E
inline constexpr std::size_t kUnitAttributionAttackerOffset = 0x2C4;     // ref +14h
inline constexpr std::size_t kUnitAttributionAttackerIdOffset = 0x2C8;   // ref +18h, u16
inline constexpr std::size_t kUnitAttributionAttackerClassOffset = 0x2CC;// 0077CFBA
inline constexpr std::size_t kUnitAttributionAttackerSideOffset = 0x2D0; // 0077CF3E
inline constexpr std::size_t kUnitAttributionOrdnanceKindOffset = 0x2D4; // 0077CF29
inline constexpr std::size_t kUnitAttributionCreditedSlotOffset = 0x2D8; // 0077CFC5
inline constexpr std::size_t kUnitAttributionOriginSlotOffset = 0x2DC;   // 0077CFDD
inline constexpr std::size_t kUnitAttributionAttackerTypeOffset = 0x2E0; // 0077D07E, u16
inline constexpr std::size_t kUnitAttributionStampOffset = 0x2E4;        // 0077CED7, float
inline constexpr std::size_t kUnitAttributionSoleFlagOffset = 0x2E8;     // 0077CF58/0077CF73

// The four victim bytes gating the whole routine, tested at 0077CE66..0077CE88 and again at
// 0077CEFB..0077CF1D. The first must be set, the other three clear.
inline constexpr std::size_t kUnitAttributionGateSetOffset = 0x5C;
inline constexpr std::size_t kUnitAttributionGateClearAOffset = 0x5D;
inline constexpr std::size_t kUnitAttributionGateClearBOffset = 0x5E;
inline constexpr std::size_t kUnitAttributionGateClearCOffset = 0x60;

// ---------------------------------------------------------------------------
// The shot-side source record, offsets fixed by the shot base constructor 006E22D0
// ---------------------------------------------------------------------------
inline constexpr std::size_t kShotOffDescriptor = 0x04;      // +8h of it is the ordnance kind
inline constexpr std::size_t kShotOffImpactEffectFlag = 0x45;// 006E2353 sets it to 1
inline constexpr std::size_t kShotOffOriginSlot = 0x1C;      // 0077CFC2, 0077D08A
inline constexpr std::size_t kShotOffOwnerRef = 0x98;        // 006E2304, copied to victim+2B0h
inline constexpr std::size_t kShotOffOwnerUnit = 0xAC;       // ownerRef +14h
inline constexpr std::size_t kShotOffOwnerId = 0xB0;         // ownerRef +18h, u16, must be != 0
inline constexpr std::size_t kShotOffOwnerEntity = 0xCC;     // 0077CF2F, its +54h is the side
inline constexpr std::size_t kShotOffKamikazeSlotSource = 0x1AC; // 0077D0D5

// The weak-reference record 0077CC80 assigns, two live fields out of the 1Ah bytes it spans.
inline constexpr std::size_t kAttributionRefTargetOffset = 0x14;
inline constexpr std::size_t kAttributionRefIdOffset = 0x18;

// The attacker unit's per-ordnance-category slot array, indexed by 00779A00's category.
inline constexpr std::size_t kUnitOrdnanceSlotArrayOffset = 0x1AC; // 0077D009
inline constexpr std::size_t kUnitPlaneSlotOffset = 0x1B0;         // 0077CF97, category 1
inline constexpr std::size_t kUnitOwningSlotOffset = 0x180;        // 0077CFA3, 009FFD20
inline constexpr std::size_t kUnitTypeIdOffset = 0x174;            // u16, 0091C392
inline constexpr std::size_t kLandFortOwnerUnitOffset = 0x71C;     // 0077D057

// ---------------------------------------------------------------------------
// The scoring-record offsets 0091BDA0 writes besides the two kill trees
// ---------------------------------------------------------------------------
// Record-relative, like scoring_bodies.hpp. The manager expression is
// `manager + slot*284h + offset + 4`.
inline constexpr std::size_t kKillCreditCountersMapOffset = 0x6C;   // 0091C117, name -> int
inline constexpr std::size_t kKillCreditPlaneKillsOffset = 0x198;   // 0091C0C3, typeId -> int
inline constexpr std::size_t kKillCreditShipKillsOffset = 0x1A4;    // 0091C085, typeId -> int
inline constexpr std::size_t kKillCreditKillListOffset = 0x1BC;     // 0091C3B5, key -> entry
inline constexpr std::size_t kKillCreditHitTallyOffset = 0x12C;     // 0090ECA3, from 0077CE60

// The kill-list entry fields 0091BDA0 writes; the entry itself is 2Ch bytes (0090E89F's
// `MOV ECX,0xB` REP MOVSD prototype).
inline constexpr std::size_t kKillListEntryOrdnanceKindOffset = 0x18; // 0091C3DC
inline constexpr std::size_t kKillListEntryCreditedSlotOffset = 0x20; // 0091C3CB
inline constexpr std::size_t kKillListEntrySoleFlagOffset = 0x2A;     // 0091C3F5

// ---------------------------------------------------------------------------
// Award tokens, from the string literals the writer pushes
// ---------------------------------------------------------------------------
// Keys into the achievement registry docs/AWARD_GRANT.md establishes at [00E19900]+10h.
inline constexpr const char* kKillCreditCounterRuaDu = "Counter_RUA_DU"; // 00D187E4
inline constexpr const char* kKillCreditAwardRuaDu = "RUA_DU";          // 00D18AC4
inline constexpr const char* kKillCreditAwardRuaBu = "RUA_BU";          // 00D18ABC
inline constexpr const char* kKillCreditAwardGaWy = "GA_WY";            // 00D18AB4

// The ordnance kind the RUA_DU counter requires, compared against victim+2D4h at 0091C0E9.
inline constexpr int kKillCreditRuaDuOrdnanceKind = 0x0A;

// 0091C498 `FMUL double ptr [0x00D7A220]`; the constant is 100.0.
inline constexpr double kKillCreditHealthPercentScale = 100.0;

// The session mode that disables the whole kill writer, 0091BDC4 `CMP [EAX+1FE4h],2`.
inline constexpr int kKillCreditDisabledSessionMode = 2;

// ---------------------------------------------------------------------------
// 0077CE60 as a rule
// ---------------------------------------------------------------------------

// What the routine reads out of the shot's source record (`hit+4h`'s vtable[108h] result).
struct KillAttributionSource {
    int ordnance_kind{0};    // [[src+4h]+8h]
    int ordnance_category{0};// 00779A00([src+4h]), the index into attacker+1ACh
    int owner_side{0};       // [[src+CCh]+54h]
    int origin_slot{0};      // [src+1Ch]
    int owner_id{0};         // u16 [src+B0h]; zero abandons the block
    bool has_owner_unit{false};// [src+ACh] != 0
};

// What it reads off the attacker unit, when there is one.
struct KillAttributionAttacker {
    int unit_class{0};       // virtual +0h on attacker+170h
    int owning_slot{0};      // attacker+180h
    int plane_slot{0};       // attacker+1B0h
    int category_slot{0};    // attacker[+1ACh + category*4]
    int type_id{0};          // u16 attacker+174h
    bool is_plane{false};    // IsKindOf(0Fh)
    bool is_ship{false};     // IsKindOf(06h)
    bool is_land_fort{false};// IsKindOf(1Bh)
    bool fort_has_owner{false};   // land fort only: [fort+71Ch] != 0
    int fort_owner_slot{0};  // [[fort+71Ch]+180h]
};

// The kamikaze override, decided on the shot entity `hit+4h`, not on the attacker.
struct KillAttributionKamikaze {
    bool shot_is_plane{false};       // IsKindOf(0Fh)
    bool shot_drops_kamikaze{false}; // 007B93E0(shot, 0)
    bool shot_is_kamikaze_plane{false};// IsKindOf(17h)
    bool shot_is_ship{false};        // IsKindOf(06h)
    bool shot_can_ram{false};        // 00779AA0(shot)
    int shot_kamikaze_slot{0};       // [shot+1ACh]
};

// The block 0077CE60 leaves on the victim. Fields it does not reach keep their prior value,
// which is why the rule below takes the prior block and returns the updated one. The three
// `*_written` flags say how far the routine got: the stamp lands on any damaging hit, the
// side and kind once the shot's source record resolves, the rest only when the source's
// owner id at +B0h is non-zero.
struct KillAttributionFields {
    bool stamp_written{false};
    bool side_and_kind_written{false};
    bool block_written{false};
    int attacker_class{0};    // +2CCh
    int attacker_side{0};     // +2D0h
    int ordnance_kind{0};     // +2D4h
    int credited_slot{0};     // +2D8h
    int origin_slot{0};       // +2DCh
    int attacker_type_id{0};  // +2E0h
    int attacker_id{0};       // +2C8h
    int sole_attacker{0};     // +2E8h, seeded to 1 only from a negative prior value
    bool kamikaze_override{false}; // attacker_class forced to 0Ch, source slot rewritten
};

// The whole rule table of 0077CE60's attribution block, with no host. `has_source` is
// `hit+4h != 0 && hit+4h->vtable[108h]() != 0`; `prior_attacker_matches` is the test at
// 0077CF6E, `+2C4h == 0 || +2C4h == [src+ACh]`.
KillAttributionFields kill_attribution_0077ce60(const KillAttributionFields& prior,
                                                bool has_source,
                                                const KillAttributionSource& source,
                                                const KillAttributionAttacker& attacker,
                                                const KillAttributionKamikaze& kamikaze,
                                                bool prior_attacker_matches) noexcept;

// 00779A00, `int __fastcall(descriptor)`: `[desc+8h] - 2` through a 0Fh-entry jump table.
// Out-of-range kinds answer 0. Category 1 is never produced; attacker+1B0h holds that slot.
int kill_credit_ordnance_category_00779a00(int ordnance_kind) noexcept;

// 009FFD20, `int __fastcall(unit)`: the slot a unit's losses are counted against.
// `unit+180h` when 7 or below, -1 at exactly 8, and in single player 0 or 4 by side.
int kill_credit_owning_slot_009ffd20(int unit_owning_slot,
                                     bool multiplayer,
                                     int unit_side,
                                     int local_player_side) noexcept;

// ---------------------------------------------------------------------------
// 0091BDA0 as a rule
// ---------------------------------------------------------------------------

// The per-slot loss counter pass over the eight player records (0091BE90..0091BF7A).
// Side 0 counts into +1ECh (allied losses), side 1 into +1F8h (Japanese); any other side
// counts nowhere. Reuses scoring_losses_offset from scoring_bodies.hpp for the offset.
struct KillCreditLossPass {
    bool counted{false};
    bool allied_map{false}; // true selects +1ECh
};

KillCreditLossPass kill_credit_loss_pass_0091bda0(int victim_side,
                                                  int victim_owning_slot,
                                                  int slot,
                                                  int slot_side) noexcept;

// The per-type tallies at +198h/+1A4h, both keyed by the victim's u16 type id and both
// indexed with the ORIGINATING slot, not the credited one (0091C072, 0091C0B0).
struct KillCreditTypeTally {
    bool ship_map{false};  // +1A4h, attacker IsKindOf(06h)
    bool plane_map{false}; // +198h, attacker IsKindOf(0Fh)
    int slot{0};
    int victim_type_id{0};
};

KillCreditTypeTally kill_credit_type_tally_0091bda0(int originating_slot,
                                                    bool attacker_is_ship,
                                                    bool attacker_is_plane,
                                                    int victim_type_id) noexcept;

// The kill-list key at +1BCh: `(victimTypeId << 16) | attackerTypeId` (0091C3AC `SHL EDX,0x10`
// then `OR EDX,EAX`). Both halves are the u16 at each unit's +174h.
constexpr std::uint32_t kill_credit_kill_list_key(int victim_type_id,
                                                  int attacker_type_id) noexcept
{
    return (static_cast<std::uint32_t>(victim_type_id & 0xFFFF) << 16) |
           static_cast<std::uint32_t>(attacker_type_id & 0xFFFF);
}

// The three awards, each decided from the attribution block plus one registry threshold.
struct KillCreditAwardInputs {
    int credited_slot{0};       // victim+2D8h
    int originating_slot{0};    // victim+2DCh
    int attacker_side{0};       // victim+2D0h, the side recorded at hit time
    int attacker_unit_side{0};  // [victim+2C4h]+54h, the attacker's live side (0091C44D)
    int victim_side{0};         // victim+54h
    int ordnance_kind{0};       // victim+2D4h
    bool attacker_is_destroyer{false};  // IsKindOf(07h)
    bool attacker_is_battleship{false}; // IsKindOf(0Dh)
    bool attacker_is_cruiser{false};    // IsKindOf(0Ah)
    bool attacker_is_mothership{false}; // IsKindOf(09h)
    bool victim_is_ship{false};         // IsKindOf(06h)
    bool attacker_has_plane_slot{false};// attacker+1B0h != 0
    float kill_distance{0.0f};  // |victim pos - attacker pos|, 0042B2F0 at 0091C271
    float attacker_health{0.0f};// 00923BE0(attacker), a 0..1 fraction
    int rua_du_counter_after{0};// the +6Ch counter value after the increment
    int rua_du_threshold{0};    // Params[0] of the "RUA_DU" registry row
    int rua_bu_threshold{0};    // Params[0] of "RUA_BU"
    int ga_wy_threshold{0};     // Params[0] of "GA_WY"
};

struct KillCreditAwardDecision {
    bool counter_rua_du{false}; // increment the +6Ch "Counter_RUA_DU" entry
    bool grant_rua_du{false};
    bool grant_rua_bu{false};
    int rua_bu_value{0};        // CVTTSS2SI of the distance, 0091C349
    bool grant_ga_wy{false};
};

// The guard on the "Counter_RUA_DU" increment at 0091C0DD..0091C0FF, split out because the
// sequence has to increment before it can compare the counter with its threshold.
bool kill_credit_counts_rua_du(const KillCreditAwardInputs& in) noexcept;

KillCreditAwardDecision kill_credit_awards_0091bda0(const KillCreditAwardInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
// One virtual per native call site the sequence below reaches. No defaults.
struct KillCreditHost {
    virtual ~KillCreditHost() = default;

    // [00e188a8]+1FE4h. 0091BDA0 returns at once when this is kKillCreditDisabledSessionMode.
    virtual int session_mode() = 0;

    // manager+1484h, 0091BDD8. When set, a victim on the local player's side is skipped.
    virtual bool skip_friendly_losses() = 0;
    virtual int local_player_side() = 0;

    // 0090C170 on the map at manager+1488h, an entity -> int `find`. The kill is abandoned
    // when the victim's root entity (its vtable[140h] result) is already a key (0091BE3F).
    virtual bool root_entity_already_scored() = 0;

    // 005070C0 on record+1ECh / record+1F8h, keyed by the victim's vtable[28h] name.
    virtual void add_loss(int slot, bool allied_map, const std::string& victim_name) = 0;

    // 00667EB0 on record+198h / record+1A4h, an int -> int `operator[]`, then `ADD [EAX],1`.
    virtual void add_type_kill(int slot, std::size_t map_offset, int victim_type_id) = 0;

    // 0062C170 / 0062BB60 / 00625900, the same three-level chain scoring_bodies.hpp models.
    virtual int& kill_tree_leaf(int slot, std::size_t tree_offset,
                                const ScoringKillKey& key) = 0;

    // 005070C0 on record+6Ch. Returns the value AFTER the increment, which is what the
    // RUA_DU threshold compare at 0091C196 reads back.
    virtual int add_named_counter(int slot, const std::string& name) = 0;

    // 0050FC30 on [00E19900]+10h then vector::at(0) on the row's +44h. The row is the
    // achievement table docs/AWARD_GRANT.md recovered; +44h is the `Params` column.
    virtual int award_threshold(const std::string& token) = 0;

    // 0090EDE0, RET 10h: `(slot, &token, value, 0)`. Applies locally through 0090C000 and
    // may forward a type-14h session message to the owning peer.
    virtual void grant_award(int slot, const std::string& token, int value) = 0;

    // 0090E850 on record+1BCh, a `uint32 -> 2Ch-byte entry` operator[] that inserts a
    // default entry on a miss. The three writes follow at 0091C3CB, 0091C3DC and 0091C3F5.
    virtual void update_kill_list(int slot, std::uint32_t key, int ordnance_kind,
                                  int credited_slot, bool sole_attacker) = 0;
};

// What 0091BDA0 needs about the destroyed unit beyond its attribution block.
struct UnitKillInputs {
    ScoringKillAttribution attribution{}; // the +2C4h..+2DCh block, plus the victim's own side
    std::string victim_name;              // victim->vtable[28h]()
    int victim_type_id{0};                // u16 victim+174h
    int victim_owning_slot{0};            // 009FFD20(victim)
    bool victim_is_root{true};            // victim->vtable[140h]() == victim, or not a fort
    bool has_attacker{false};             // victim+2C4h != 0
    bool attacker_is_ship_base{false};    // IsKindOf(06h), selects the +1A4h tally
    bool attacker_is_plane_base{false};   // IsKindOf(0Fh), selects the +198h tally
    int attacker_type_id{0};              // u16 attacker+174h
    int ordnance_kind{0};                 // victim+2D4h, written into the kill-list entry
    int sole_attacker{0};                 // victim+2E8h
    KillCreditAwardInputs awards{};
    int slot_sides[8]{};                  // [game+18CCh + i*4]+28h
};

// 0091BDA0's whole sequence over the host, in listing order.
void kill_credit_record_unit_kill_0091bda0(KillCreditHost& host, const UnitKillInputs& in);

} // namespace bsp
