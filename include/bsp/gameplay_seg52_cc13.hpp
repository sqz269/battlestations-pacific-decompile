#pragma once

// Segment 52 (008288D0-00851CB0) reference models: the weapon director and the
// squadron command block, the shipyard hangars, the turning guns' fire request,
// the support site and the stationary props. Packet cc13-gpr52;
// docs/CC13_GPR_52.md, docs/CC13_GP_0080_0084.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw)
// and the Ghidra pseudocode, not native layouts or ABI-compatible replacements.
// Field names are hypotheses; the "_NNN" suffix is the native byte offset. Every
// engine service the image calls (virtual queries, target resolution, the RNG)
// is an input or a host callback, so nothing here invents a global or stubs a
// call. Gameplay runs at 24-bit x87 precision (docs/CC13_FPU_PRECISION.md); the
// one float sum here (0084F020) is modelled in float. Build-tested only; not
// wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is docs/CC13_GPR_52.md section 2):
// - Replaces:
//   * 0084C4E0 / 0084C5B0: the host's MRTGun immediate request
//     (src/game_hosts_gunnery.cpp:7793, `immediate_4d4`) and its FireIfReady arm
//     (:7813, modelled as 00727E30). Both turning-gun vtables put 0084C4E0, not
//     00727E30, in slot 1DCh (dwords 00CFC134 and 00CFC36C), and 0084C4E0 clears
//     gun+4D4h after a shot. The host clears it only on a dropped latch (:7728).
//   * 00844A10: the shipyard aim point, labelled NOT-MODELLED at
//     src/game_hosts_gunnery.cpp:1116 (kArtilleryAirfieldAimSlotBound's comment).
//   * 00835F60: nothing. The host's director step (src/game_hosts_commands.cpp:2805)
//     starts at the pre-pass 00836941, after this call.
//   * 00835B40, 0084DAE0, 0084DB90, 0084DC50, 0084DD20: nothing. No host runs the
//     derived director's retarget slot or the plane squadron block's queue slots.
//   * 00836040: the host's normalize_self_target override
//     (src/game_hosts_commands.cpp:1120), which always answers 0.
//   * 0084D050, 0084D170: nothing (the support manager is not hosted;
//     00675C40 is cc9's).
//   * 0084F020: nothing. No host routes a hit to a stationary prop.
// - Inputs: the entity live bytes +5Ch/+5Dh/+5Eh/+60h; command classes as the
//   image's class addresses (00E08F68 moveto and so on); the virtual answers
//   (kind 1/2, IsKind, +13Ch) passed in as values; the support site's authored
//   "Stock %d" bags through a lookup callback.
// - RNG: one draw. 00844AB4: U(0, n - 1) on STREAM 1, truncated by _ftol
//   (00BF7420), only when n >= 2 live hangars. Every other model draws nothing.
// - OFF-switch prediction: OFF changes nothing. ON:
//   * 0084C4E0 binds the one-shot clear: an MRTGun that fired once on its delayed
//     request stops sending the extra 0ADh every step and fires only through
//     0072D130's gates, so rapid turning guns fire fewer ungated shots;
//   * 00844A10 shifts every later stream-1 consumer by one draw per artillery
//     aim refresh at a shipyard with two or more live hangars;
//   * the rest change nothing the hosts run until a director or support-site
//     host is bound.

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace bsp {
namespace gameplay_seg52_cc13 {

struct Vec3f {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// The command classes the models compare against, by their image addresses
// (docs/COMMAND_CLASSES.md). These are identities, not globals the models read.
inline constexpr std::uint32_t kCmdMoveTo = 0x00E08F68u;
inline constexpr std::uint32_t kCmdAttackMove = 0x00E08F78u;
inline constexpr std::uint32_t kCmdMoveOnPath = 0x00E08F80u;
inline constexpr std::uint32_t kCmdStop = 0x00E08F88u;
inline constexpr std::uint32_t kCmdRetreat = 0x00E08F90u;
inline constexpr std::uint32_t kCmdReturnToBase = 0x00E08F98u;
inline constexpr std::uint32_t kCmdLand = 0x00E08FA0u;

// The live test the image repeats inline (00845794, 00844A5x, 00835FA3): +5Ch
// set, then +5Dh, +60h and +5Eh clear, in that order.
struct EntityLiveBytes {
    bool b5c = false;
    bool dead_5d = false;
    bool b5e = false;
    bool b60 = false;
};
bool entity_live(const EntityLiveBytes& e) noexcept;

// ---------------------------------------------------------------------------
// Shipyard hangars (+780h..+788h, 10h records, +4 the hangar Object; filled by
// 00849F70 from the "Hangar %d" properties).

struct ShipyardHangarSlot {
    bool has_object = false;  // record+4 != 0
    EntityLiveBytes object;
};

// 00845750 BSP_Shipyard_FirstLiveHangarObject, __fastcall(ECX = shipyard) -> EAX.
// The first record whose object is live, else none. The image does not test
// record+4 for null (00845794 dereferences it); a null object is taken as not
// live here and the difference is recorded.
std::optional<std::size_t> shipyard_first_live_hangar_00845750(
    const std::vector<ShipyardHangarSlot>& slots) noexcept;

// 00845870 BSP_Shipyard_GetLaunchPosition, __thiscall(shipyard, float3* out), RET 4.
// The first live hangar's vtable +C4h position (host callback), else the
// shipyard's own world position +FCh..+104h (after 00414DB0 when +C8h is clear).
Vec3f shipyard_launch_position_00845870(const std::vector<ShipyardHangarSlot>& slots,
    const std::function<Vec3f(std::size_t)>& hangar_position_c4,
    const Vec3f& shipyard_world_position_0fc);

// 00844A10, MShipyard vtable 00D0B870 slot 100h (the artillery aim point; the
// ledger's name "PickLaunchSlotPose" predates cc9's reading at
// src/game_hosts_gunnery.cpp:1093). Counts the live records with a non-null
// object; with n >= 2 draws k = _ftol(U(0, n - 1)) on STREAM 1 (00844AAF
// MOV ECX,1; 00844AB4) and takes the k-th live record (0-based; with none left
// it falls back to the first). The caller turns the record's object world
// position +FCh into the shipyard's frame through the cached inverse +110h
// (00B63D50 / 004142E0); with no live record the point is 00F87574 (zero).
struct ShipyardAimPick {
    std::optional<std::size_t> slot;
    int draws = 0;  // 0 or 1, all on stream 1
};
ShipyardAimPick shipyard_aim_point_pick_00844a10(const std::vector<ShipyardHangarSlot>& slots,
    const std::function<float(int stream, float lo, float hi)>& uniform);

// 00845AC0, MShipyard vtable 00D0B770 slot 1A8h (dword 00D0B918), the
// shipyard's copy of the airfield destruction rule 006D40F0 (the airfield's slot
// 1A8h, src/game_hosts_units.cpp:5797 kAirfieldDestructionRuleBound). Nothing
// while any record's object is live (00845794 tests +5Ch/+5Dh/+60h/+5Eh, no null
// test). Otherwise a shipyard adopted by a CommandBuilding (+71Ch != 0) gets
// vtable +194h("InferiorFailure", 0); any other gets vtable +70h(1) (00845B55
// PUSH 1), where the airfield passes 0. The caller of the shipyard's slot 1A8h
// was not found: the shipyard tick 00846320 does not load it, unlike the
// airfield tick at 006D2526.
// The +194h argument is an eight-byte native string (dword length, dword buffer)
// built on the stack: zeroed, 0041DD40(15, 1), memcpy of 16 bytes from 00CF0B74
// ("InferiorFailure") when a buffer came back; after the call the buffer goes back
// to the sized pool (00419CC0, then 00BD1510(buffer, length + 1, 1)).
struct ShipyardNativeString {
    std::uint32_t length = 0;
    char* data = nullptr;
};
struct ShipyardHangarsLostHost {
    virtual ~ShipyardHangarsLostHost() = default;
    // 0041DD40 __thiscall(string, length, 1) RET 8.
    virtual void native_string_resize_0041dd40(ShipyardNativeString& s, std::uint32_t length, std::int32_t flag) = 0;
    // Shipyard vtable +194h __thiscall(shipyard, const string*, 0) RET 8.
    virtual void shipyard_vslot194(const ShipyardNativeString* name, std::int32_t arg) = 0;
    // Shipyard vtable +70h __thiscall(shipyard, 1) RET 4.
    virtual void shipyard_vslot70(std::int32_t arg) = 0;
    virtual void* pool_singleton_00419cc0() = 0;
    // 00BD1510 __thiscall(pool, block, size, 1) RET 0Ch.
    virtual void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, std::int32_t flag) = 0;
};
void shipyard_hangars_lost_00845ac0(
    const std::vector<ShipyardHangarSlot>& slots, bool adopted_71c, ShipyardHangarsLostHost& host);

// 00847030 BSP_Shipyard_HandleLaunchRequest, __thiscall(shipyard, msg), RET 4.
// Jump table on msg+10h - A7h; every arm passes (msg+14h, msg+20h, byte msg+24h)
// except AAh, which takes the message, and each answers 1.
enum class ShipyardMessageArm {
    ChangeSlotClass_00844d60,      // A7h
    ChangeSlotEquipment_008436f0,  // A8h
    LaunchSlot_00846d90,           // A9h
    ApplySpawnMessage_008437d0,    // AAh
    UnitHandleMessage_0095abe0,    // any other kind (its answer is returned)
};
ShipyardMessageArm shipyard_message_arm_00847030(std::uint8_t kind) noexcept;

// ---------------------------------------------------------------------------
// Turning guns: the delayed-fire request gun+4D4h.

// 0084C4E0, slot 1DCh of MRTGun 00CFBF58 (dword 00CFC134) and of 00CFC190
// (dword 00CFC36C). 00727E30 FireIfReady; on a shot clears +4D4h and answers 1.
// `fired` is 00727E30's answer.
bool turning_gun_fire_if_ready_0084c4e0(bool fired, bool& request_4d4) noexcept;

// 0084C5B0, slot 8 of the +310h sub-object, __thiscall(gun+310h, float dt), RET 4.
// Runs 0072D130 (the host's tick) and then, if +4D4h is still set (0084C5DB reads
// it through the sub-object as +1C4h), builds message 0ADh (vtable 00CF9628) and
// routes it on the gun with (msg, 1, 0) (0084C625). Answers whether that extra
// 0ADh is sent. `request_4d4_after_tick` is +4D4h after 0072D130 returns: when
// the routed 0ADh of 0072D130 is delivered at once, its 0084C4E0 may already have
// cleared the request.
bool turning_gun_extra_fire_request_0084c5b0(bool request_4d4_after_tick) noexcept;

// ---------------------------------------------------------------------------
// Weapon director.

// 00835F60, base director vtable 00D09EC0 slot 7Ch, __fastcall(director); the
// first call of the derived step 00836920. With a fire target at +238h that is
// not IsKind(41h) (vtable +5Ch), it routes a message 5Eh (vtable 00D02E98)
// on the owner +34h with (msg, 7, 0) unless the target is live, of another party (+54h against [+34h]+54h) and
// either IsKind(1Ch) or 005457C0's answer (owner party != target party and the
// target party != 2). Answers whether the message is sent.
struct FireTargetView {
    bool present = false;          // director+238h != 0
    bool is_kind_41 = false;       // vtable +5Ch (41h)
    EntityLiveBytes live;
    std::int32_t party_54 = 0;
    bool is_kind_1c = false;       // vtable +5Ch (1Ch), asked only when reached
};
bool director_release_fire_target_00835f60(const FireTargetView& target,
                                           std::int32_t owner_party_54) noexcept;

// 00835B40, derived director vtable 00D09F58 slot 70h, __thiscall(director,
// const SceneCommandTarget*, char primary), RET 8. The image ALWAYS calls the
// base 0071EDD0(target, primary) and returns 0 when it does; the stage raise
// needs the same resolved object (00521EA0 on the incoming target and on +58h
// for primary, +18Ch otherwise), session mode [00E188A8]+1FE4h != 2, and the
// slot command +54h (primary) / +188h (otherwise) moveto or attackmove.
struct RetargetInputs {
    bool primary = false;
    bool same_object = false;          // 00521EA0(incoming) == 00521EA0(slot target)
    bool base_answer_0071edd0 = false;
    std::int32_t session_mode_1fe4 = 0;
    std::uint32_t slot_command = 0;    // +54h or +188h
};
struct RetargetResult {
    bool answer = false;
    bool raise_primary_stage_2 = false;    // 0071D810(2)
    bool raise_override_stage_2 = false;   // 0071D9E0(2)
};
RetargetResult director_retarget_command_slot_00835b40(const RetargetInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Plane squadron command block, vtable 00D0BD98 (built by 0084D810). Slots are
// 1Ch records from +54h: command class at +54h + 1Ch*i, target at +58h + 1Ch*i.

struct CommandSlotView {
    std::uint32_t command = 0;
    bool target_resolves = false;  // 00521EA0(slot target) != 0
    std::int32_t kind = 0;         // command vtable +0Ch
};

// 0084DAE0, slot +20h, __thiscall(block, int slot), RET 4.
// 0071DEE0(slot) must answer; slot 0 holding a moveto whose target resolves is
// refused when the owner +224h and the target answer the same vtable +13Ch.
struct SlotStillValidInputs {
    std::int32_t slot = 0;
    bool has_active_order_0071dee0 = false;
    std::uint32_t slot0_command = 0;
    bool slot0_target_resolves = false;
    std::int32_t owner_13c = 0;    // [block+224h] vtable +13Ch
    std::int32_t target_13c = 0;   // resolved target vtable +13Ch
};
bool squadron_slot_order_still_valid_0084dae0(const SlotStillValidInputs& in) noexcept;

// 0084DB90, slot +34h, __thiscall(block, command), RET 4. `last` is the slot
// 0071BE60 - 1 (the last filled); kinds are 006BC030's "vtable +0Ch is 1 or 2".
struct AcceptQueuedInputs {
    std::uint32_t command = 0;
    bool command_kind_1_or_2 = false;
    bool has_free_slot_0071c0c0 = false;
    std::int32_t filled_0071be60 = 0;
    CommandSlotView last;
};
bool squadron_accepts_queued_command_0084db90(const AcceptQueuedInputs& in) noexcept;

// 0084DC50, slot +30h, __thiscall(block, command, target), RET 8. The base
// 0071E550 first; then the last filled slot is dropped through 0071D900(last)
// when both kinds are 1/2, or the last is a moveto with a resolving target and
// the incoming command is kind 1/2 or a resolving moveto.
struct MakeRoomInputs {
    bool base_answer_0071e550 = false;
    std::uint32_t command = 0;
    std::int32_t command_kind = 0;
    bool command_target_resolves = false;
    std::int32_t filled_0071be60 = 0;
    CommandSlotView last;
};
struct MakeRoomResult {
    bool answer = false;
    std::optional<std::int32_t> drop_slot;  // 0071D900's argument
};
MakeRoomResult squadron_make_room_for_command_0084dc50(const MakeRoomInputs& in) noexcept;

// 0084DD20, __thiscall(block, command, SceneCommandTarget*), RET 8. A target
// whose byte +0 is set and which resolves to the block's own unit +34h: a moveto
// answers 1 unchanged; a stop rewrites the target to the zero point (00F87574..7C)
// and clears the rest, answering 1; anything else answers 0.
struct SceneCommandTargetView {
    std::uint8_t b0 = 0;
    std::uint16_t w2 = 0;
    std::uint32_t d4 = 0;
    Vec3f point_08;
    float f14 = 0.0f;
};
bool squadron_filter_self_target_command_0084dd20(std::uint32_t command,
    SceneCommandTargetView& target, bool resolves_to_own_unit,
    const Vec3f& zero_point_f87574) noexcept;

// 00836040, the ship director's slot 14h (the same shape; 0071E6C0 refuses the
// push when it answers 1 and a slot is filled, 0071E73C/0071E742). Here cruise
// (00E08F70) and stop (00E08F88) are rewritten to the empty target and answer 1;
// every other command answers 0, a moveto included. The host's override
// (src/game_hosts_commands.cpp:1120) always answers 0: no target resolves to the
// director's endpoint +34h there.
inline constexpr std::uint32_t kCmdCruise = 0x00E08F70u;
bool director_normalize_self_target_00836040(std::uint32_t command,
    SceneCommandTargetView& target, bool resolves_to_own_unit,
    const Vec3f& zero_point_f87574) noexcept;

// ---------------------------------------------------------------------------
// Support site (the support-manager sites of docs/SHIP_AI_OPEN_ITEMS.md 52).

struct SupportStockEntry {      // the 0Ch list payload built at 0084D2C2..0084D2F2
    std::int32_t type = 0;      // "Type" +0Ch
    std::int32_t count = 0;     // "Count" +0Ch, never 0 here
    std::int32_t squad_size = 3;  // "SquadSize" +0Ch, default 3
};
struct SupportSiteConfig {
    std::int32_t max_planes_334 = 4;   // "maxPlanes"
    std::int32_t max_ships_338 = 2;    // "maxShips"
    std::int32_t angle_33c = 230;      // "Angle", default 0E6h
    bool sp_active_340 = true;         // "SPActive"; 0084D050 needs it
    std::vector<SupportStockEntry> japan_31c;   // "JapanList"
    std::vector<SupportStockEntry> allied_328;  // "AlliedList"
    std::int32_t field_344 = 0;        // zeroed at the end
    std::vector<std::int32_t> class_loads;  // 00964790 types, in call order
};
// One "Stock %d" sub-bag as the lookup sees it: absent, not a bag (type != 6) or
// a null bag ends the walk.
struct SupportStockBag {
    bool bag = false;
    std::optional<std::int32_t> count;     // "Count" (the image assumes it exists)
    std::int32_t type = 0;                 // "Type" (likewise)
    std::optional<std::int32_t> squad_size;
};
struct SupportSiteProperties {
    std::optional<std::int32_t> max_planes, max_ships, angle;
    std::optional<bool> sp_active;
    // list: 0 = JapanList, 1 = AlliedList; index from 1.
    std::function<SupportStockBag(int list, std::int32_t index)> stock;
    std::int32_t effective_game_mode = 0;  // 004BCA50; 4 skips the class loads
};
// 0084D170, support-site vtable slot (no direct caller), __fastcall(ECX = site),
// after 0077F0E0. The AlliedList walk continues the "Stock %d" counter where the
// JapanList walk stopped (EBX is not reset: 0084D37E PUSH EBX), so with Japan
// stocks 1..N authored the Allied walk starts at "Stock N+1".
SupportSiteConfig support_site_activate_0084d170(const SupportSiteProperties& props);

// 0084CE60, __thiscall(site, byte): +340h = byte.
void support_site_set_active_0084ce60(SupportSiteConfig& site, bool active) noexcept;

// 0084D050, __thiscall(site, classId) -> AL, RET 4. Needs +340h; walks the map
// +314h (node +0Ch class, +10h enabled byte) and refuses on a matching entry whose
// byte is clear (a class with no entry passes); then a present owner
// (00923810(1)) must have 00923BE0 health > 0 (FCOMIP/JBE: NaN refuses).
struct SupplyClassEntry {
    std::int32_t class_id = 0;
    bool enabled = false;
};
bool support_site_can_supply_class_0084d050(bool active_340,
    const std::vector<SupplyClassEntry>& entries, std::int32_t class_id,
    bool owner_present, float owner_health) noexcept;

// 0084D560 (cc13-gprx), __thiscall(site, list* out, int team, bool flag) -> out,
// RET 0Ch, body 0084D560-0084D7B0. Builds the classes the site offers:
// - 00923810(1) is the site's owner (+3Ch). A present owner that is not live
//   (+5Ch clear, or +5Dh/+60h/+5Eh set) gives an empty list (0084D5A2..0084D5CF).
// - team 0 copies the AlliedList +328h, any other team the JapanList +31Ch.
// - In a session (game+1FE4 != 0) with flag set, the local player's unit
//   (game+18F0[game+18C8]) is read: its vtable[1Ch]() is the key (default 61h).
// - With an adopting CommandBuilding (+344h), 006F4A00(46h) / 006F4A00(45h) ask
//   whether any garrisoned shipyard / airfield reports "InferiorFailure" (the
//   failure 00845AC0 and 006D40F0 raise once every hangar is lost).
// - Each entry's class is 00964790(type, readRace = 1). A shipyard failure drops
//   classes answering vtable[18h](6); an airfield failure drops those answering
//   vtable[18h](0Fh) (0084D6C1..0084D71C). A kept class whose vtable[1Ch]()
//   equals the key marks the list.
// - A marked list gets {type = player unit +70h, count = -1, squad = 3} appended
//   at the tail (0084D747..0084D78A; inserted before the head node).
// The class kinds 6 and 0Fh are not named here (vehicle-class kind codes).
struct SupplyClassView {
    bool kind_6 = false;     // class vtable[18h](6)
    bool kind_0f = false;    // class vtable[18h](0Fh)
    std::int32_t key_1c = 0; // class vtable[1Ch]()
};
struct SupplyClassListInputs {
    const SupportSiteConfig* site = nullptr;
    std::int32_t team = 0;
    bool flag = false;
    bool owner_present = false;         // 00923810(1) != 0
    EntityLiveBytes owner;              // read only when present
    bool session = false;               // game+1FE4 != 0
    bool player_unit_present = false;   // game+18F0[game+18C8] != 0
    std::int32_t player_unit_key_1c = 0;
    std::int32_t player_unit_class_70 = 0;
    bool command_building_344 = false;
    bool shipyard_failure_46 = false;   // 006F4A00(46h), read only with +344h
    bool airfield_failure_45 = false;   // 006F4A00(45h), likewise
    std::function<SupplyClassView(std::int32_t type)> class_of;  // 00964790(type, 1)
};
std::vector<SupportStockEntry> support_site_build_supply_class_list_0084d560(
    const SupplyClassListInputs& in);

// ---------------------------------------------------------------------------
// Shipyard stock registration.

// 008433C0 (cc13-gprx), __thiscall(ScenePropertyBag* ECX), bare RET, body
// 008433C0-00843456. Reads "Party" first and takes its +0Ch with no null check
// (008433D3). Then for every k in 1..20 (00843449 CMP EDI,14h / JLE; a missing
// number does not stop the loop, unlike 0084D170's walk) a "Stock k" property
// that is a bag (type 6, non-null) whose "Type" value is non-zero calls
// 0095BA60(type, party) then [00E18680]->0046BE90(type).
struct ShipyardStockRegistration {
    std::int32_t stock_index = 0;
    std::int32_t type = 0;
    std::int32_t party = 0;
};
std::vector<ShipyardStockRegistration> shipyard_register_stock_008433c0(
    std::int32_t party,
    const std::function<std::optional<std::int32_t>(std::int32_t index)>& stock_type);

// ---------------------------------------------------------------------------
// Stationary props.

// 0084F020, stationary vtable slot (no direct caller), __thiscall(stationary,
// HitRecord*), RET 4 -> 1. A dead prop (+5Dh) is untouched. Otherwise the hit's
// damage against the armour +180h is 004706D0 (part index +34h == -1, the worst
// part) or 00470510 (any other index); COMISS against 0.0f (00D7A218) / JBE adds
// only a damage > 0 (NaN skips) to +1A0h in float, and 0084EB80 then runs either
// way (stationary_check_destroyed_0084eb80 in band_0080_cc13.hpp).
struct StationaryHitResult {
    bool applied = false;     // the +5Dh test passed
    bool damage_added = false;
    int check_result = 0;     // static_cast of StationaryDamageResult; 0 when not applied
};
StationaryHitResult stationary_apply_hit_0084f020(bool dead_5d, float hit_damage,
    float protection_150, float max_damage_17c, float& damage_1a0) noexcept;

}  // namespace gameplay_seg52_cc13
}  // namespace bsp
