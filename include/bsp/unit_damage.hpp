#pragma once
// Unit damage, invincibility and death. docs/UNIT_DAMAGE_AND_DEATH.md,
// reports/unit_damage.json. Packet cc2_unit_damage, Ghidra read-only.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The rules
// below are projections of the native routines named in each comment; they are
// not binary-compatible replacements and no native layout is reproduced.
#include <cstdint>
#include <cstddef>

namespace bsp {

// ---------------------------------------------------------------------------
// Entity fields the damage path touches. Offsets are on the object whose vtable
// is the root 00CE6290; for a ship that is the 0x1188 unit instance of
// include/bsp/unit_instance_layout.hpp with vtable 00CFC3D0. The flag bytes
// +5Ch..+60h already have constants in include/bsp/lua_binding_entity_lookup.hpp
// (kEntityFlagActive, kEntityFlagReleased, kEntityFlagReleaseRequested,
// kEntityFlagUnreadGate) and are not redeclared here.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kUnitInvincibilityOffset = 0x150;  // 0042ED8F writes, 008790D0 reads
inline constexpr std::size_t kUnitFullHealthMarkerOffset = 0x2E8; // 00877C20 stores -1
inline constexpr std::size_t kUnitMaxHealthOffset = 0x36C;      // 00877BD9, 008790F7, 00876267
inline constexpr std::size_t kUnitHealthOffset = 0x370;         // 00877C04 is the only writer
inline constexpr std::size_t kUnitReplicatedHealthByteOffset = 0x374; // 00877C84
inline constexpr std::size_t kUnitKilledFlagOffset = 0x5F;      // 00926D90, 009274D6
inline constexpr std::size_t kUnitDeathCauseOffset = 0x70;      // 00926C80, 00926D90, read 0077D1DB
inline constexpr std::size_t kUnitSubobjectOwnerOffset = 0x71C; // 0095DA03, 00958A4B
inline constexpr std::size_t kUnitSubobjectLockoutOffset = 0x728; // 0095DA0C
inline constexpr std::size_t kUnitPartsObjectOffset = 0x1018;   // 0080E490 returns it
inline constexpr std::size_t kUnitSinkAttachmentOffset = 0x740; // 00811126, released and nulled
inline constexpr std::size_t kUnitSinkClearedFieldA = 0x828;    // 00811116
inline constexpr std::size_t kUnitSinkClearedFieldB = 0x82C;    // 0081111E

// Vtable byte offsets on 00CE6290. Slot values are the base implementations;
// the unit vtable 00CFC3D0 overrides +70h, +110h, +1ACh and +1B0h.
inline constexpr std::size_t kEntityVtableSlotDestroy = 0x70;          // base 00926C80, unit 0077D1A0
inline constexpr std::size_t kEntityVtableSlotOnPendingDestroy = 0x74; // base 00926390, dispatched 0092747F
inline constexpr std::size_t kEntityVtableSlotOnKilled = 0x80;         // base 00928C80, dispatched 0092751F
inline constexpr std::size_t kEntityVtableSlotSetInvincible = 0xF4;    // 0042ED80, not overridden
inline constexpr std::size_t kEntityVtableSlotHealthFraction = 0x110;  // base 0042BB50, unit 00876260
inline constexpr std::size_t kEntityVtableSlotAddDamage = 0x1AC;       // base 0042B130, unit 0095DA00
inline constexpr std::size_t kEntityVtableSlotHealthChanged = 0x1B0;   // base 0042B140, ship 00827A90

// ---------------------------------------------------------------------------
// Constants read from the listing.
// ---------------------------------------------------------------------------
inline constexpr float kUnitInvincibleFull = 1.0f;      // 00D7A24C, Lua true
inline constexpr float kUnitInvincibleOff = 0.0f;       // 00897A50 else branch, Lua false
inline constexpr double kUnitFullHealthEpsilon = 1.0;   // 00D7A210, FSUB at 00877C12
inline constexpr double kUnitHealthByteScale = 256.0;   // 00D0DEE0, FMUL at 00877C58
inline constexpr int kUnitHealthByteMax = 0xFF;         // 00877C6B
inline constexpr double kUnitSinkPivotScale = 0.5;      // 00D7A280
inline constexpr float kUnitPartDetachedHealth = -10000.0f; // 00D19620, stored by 00935C70

// Session mode at [00E188A8+1FE4h]. 0 leaves the difficulty scaling on, 1 sends
// the replicated health byte, 2 suppresses every local damage application.
enum class UnitSessionMode : int {
    campaign = 0,       // 0095DA22, 0087D73C
    multiplayer_host = 1, // 00877C48
    multiplayer_client = 2, // 0087909B, 00877C2F
};

// The Kill binding's second Lua argument becomes 1 or 2 (008AC6FE..008AC71B).
// 00926D90 remaps the incoming cause 7 to 2 before storing it at +70h.
enum class UnitKillCause : int {
    normal = 1,
    hard = 2,
    remapped_seven = 7, // 00926DA1: stored as hard
};
int normalized_kill_cause_00926d90(int cause) noexcept;
UnitKillCause kill_cause_from_lua_008ac5c0(bool has_second_argument, bool hard) noexcept;

// ---------------------------------------------------------------------------
// The health pair and the rules over it. Pure functions with explicit inputs;
// nothing here touches an entity pointer.
// ---------------------------------------------------------------------------
struct UnitHealth {
    float max_health{0.0f};       // +36Ch
    float current_health{0.0f};   // +370h
    float invincibility{0.0f};    // +150h, a fraction of max_health
};

// 00876260 (00CFC3D0+110h): FLD [+370h] / FDIV [+36Ch]. No zero guard in the
// native code, so a zero maximum divides by zero there as well.
float health_fraction_00876260(const UnitHealth& health) noexcept;

// 00923BE0: a released entity (+5Dh set) reports 0.0 without dispatching.
float entity_health_00923be0(const UnitHealth& health, bool released) noexcept;

// 00877C53..00877C77: clamp((int)(fraction * 256.0), 0, 255).
int replicated_health_byte_00877b90(float health_fraction) noexcept;

// 008790E3..008790FD, written exactly as the x87 does it: the intermediate
// 1.0f - invincibility is rounded through a float store before the second
// subtraction, so this is not simply invincibility * max_health.
float invincibility_floor_00879070(const UnitHealth& health) noexcept;

// The result of the damage rule at 00879070. `applied` is what reaches the
// setter; `refused` says the routine returned before writing anything.
struct UnitDamageOutcome {
    bool refused{false};
    float clamped_amount{0.0f};
    float new_health{0.0f};
};

// Gates on the caller's side of the rule, in the order the listing tests them.
struct UnitDamageGates {
    UnitSessionMode session_mode{UnitSessionMode::campaign}; // 00879085
    bool release_requested{false};                           // +5Eh, 008790A1
};

// 00879070 body, steps 1-5 of docs/UNIT_DAMAGE_AND_DEATH.md. A non-positive
// amount is a repair: it skips the dead test and the invincibility floor.
UnitDamageOutcome apply_damage_00879070(const UnitHealth& health,
                                        const UnitDamageGates& gates,
                                        float amount) noexcept;

// 0095DA00 gate: a subobject whose lockout is still running takes nothing.
bool subobject_blocks_damage_0095da00(std::uint32_t owning_unit, float lockout) noexcept;

// 0095DA36..0095DA7F and 0087D74C..0087D79D. Both multipliers come from a
// difficulty table indexed by [00E188A8+6ACh], or by the constant 2 outside a
// campaign (kNonCampaignReportedDifficulty in include/bsp/lua_binding_core.hpp),
// and both are skipped in multiplayer. `table` is the caller's copy of the
// config vector; the native routines call the CRT range throw at 0095DA6E and
// 0087D785 when the index is past its end, which `out_of_range` reports.
struct UnitDifficultyScale {
    bool out_of_range{false};
    float amount{0.0f};
};
UnitDifficultyScale scale_damage_by_difficulty(float amount, const float* table,
                                               std::size_t count, std::size_t level,
                                               bool enabled) noexcept;

// The result of 00877B90. `health_changed` drives the vtable[1B0h] dispatch and
// `replicated_byte_changed` drives the host's session message.
struct UnitHealthWrite {
    bool wrote{false};
    float stored_health{0.0f};
    bool full_health_marker{false}; // +2E8h set to -1
    bool dispatch_health_changed{false};
    bool replicated_byte_changed{false};
    int replicated_byte{0};
};

UnitHealthWrite set_health_00877b90(const UnitHealth& health,
                                    float requested,
                                    UnitSessionMode session_mode,
                                    int last_replicated_byte,
                                    bool released) noexcept;

// 00827AB0 / 00958DAA: health at or below zero is dead.
bool unit_is_dead(const UnitHealth& health) noexcept;

// 008110F0 gate: released, or any non-zero invincibility, refuses the sink.
bool sink_is_refused_008110f0(bool released, float invincibility) noexcept;

// 00891CAF..00891D3C. `corner` is the Lua integer; the descriptor half-extents
// are [desc+A4h] (x) and [desc+A0h] (z).
struct UnitSinkPivot {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};
UnitSinkPivot sink_pivot_00891b20(float descriptor_half_width,
                                  float descriptor_half_length,
                                  int corner) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary. One virtual method per native call site the sequences
// below reach. No default implementations: nothing here stands in for
// unrecovered game behaviour.
// ---------------------------------------------------------------------------
struct UnitDamageHost {
    virtual ~UnitDamageHost() = default;

    // 00888AA0 at 0088E0FF and the equivalent site in each of the six handlers:
    // the entity behind the Lua table argument, or 0.
    virtual std::uint32_t entity_from_lua_table(int argument_index) = 0;

    // The Lua argument readers of docs/LUA_OBJECT_API.md, as used by these six
    // handlers only.
    virtual int lua_argument_count() = 0;                    // 00B663F0 at 008AC6DF
    virtual float lua_number_argument(int index) = 0;        // 0088E0DE
    virtual bool lua_boolean_argument(int index) = 0;        // 008AC70B
    virtual bool lua_argument_is_boolean(int index) = 0;     // 00897B6F
    virtual void lua_push_boolean(bool value) = 0;           // 00897DD8 in IsInvincible

    // Reads of the entity the handler resolved.
    virtual UnitHealth read_health(std::uint32_t entity) = 0;
    virtual bool entity_released(std::uint32_t entity) = 0;          // +5Dh
    virtual bool entity_release_requested(std::uint32_t entity) = 0; // +5Eh
    virtual bool entity_killed(std::uint32_t entity) = 0;            // +5Fh
    virtual bool entity_destroyed(std::uint32_t entity) = 0;         // +60h
    virtual std::uint32_t entity_subobject_owner(std::uint32_t entity) = 0; // +71Ch
    virtual float entity_subobject_lockout(std::uint32_t entity) = 0;       // +728h
    virtual std::uint32_t entity_class_descriptor(std::uint32_t entity) = 0; // +538h
    virtual int entity_death_cause(std::uint32_t entity) = 0;                // +70h
    virtual int last_replicated_health_byte(std::uint32_t entity) = 0;       // +374h

    // Writes 00877B90 and the kill/destroy routines make.
    virtual void write_health(std::uint32_t entity, float health) = 0;        // 00877C04
    virtual void write_full_health_marker(std::uint32_t entity) = 0;          // 00877C20
    virtual void write_replicated_health_byte(std::uint32_t entity, int byte) = 0; // 00877C84
    virtual void write_invincibility(std::uint32_t entity, float value) = 0;  // 0042ED8F
    virtual void write_killed_flag(std::uint32_t entity) = 0;                 // 00926D90
    virtual void write_destroyed_flag(std::uint32_t entity) = 0;              // 00926C80
    virtual void write_death_cause(std::uint32_t entity, int cause) = 0;      // +70h

    // Session and difficulty state.
    virtual UnitSessionMode session_mode() = 0;                    // [00E188A8+1FE4h]
    virtual std::size_t difficulty_level() = 0;                    // [00E188A8+6ACh]
    virtual bool is_local_player_role(std::uint32_t entity) = 0;   // 00927F30 at 0095DA2D
    virtual bool is_current_player_unit(std::uint32_t entity) = 0; // 0087D74C comparison
    virtual float unit_damage_multiplier(std::size_t level) = 0;   // config+50h at 0095DA76
    virtual float player_damage_multiplier(std::size_t level) = 0; // config+20h at 0087D79D

    // Virtual dispatches, each named by the slot it goes through.
    virtual void dispatch_add_damage(std::uint32_t entity, float amount) = 0;    // 0088E15B
    virtual void dispatch_set_invincible(std::uint32_t entity, float value) = 0; // 00897C63, 0042EDB4
    virtual void dispatch_health_changed(std::uint32_t entity) = 0;              // 00877C40
    virtual void dispatch_destroy(std::uint32_t entity, int recurse) = 0;        // 00958DBE, 00811111, 00926E05
    virtual void dispatch_kill(std::uint32_t entity, int cause) = 0;             // 008AC756, 00926E19, 007ED3A3, 00742275
    virtual void dispatch_on_killed(std::uint32_t entity) = 0;                   // 0092751F
    virtual void dispatch_pending_destroy(std::uint32_t entity) = 0;             // 0092747F
    virtual bool class_id_test(std::uint32_t entity, int class_id) = 0;          // 008AC729 vtable[5Ch]

    // Hierarchy walks. +48h head / +44h next for children, +3Ch for the parent.
    virtual std::uint32_t first_child(std::uint32_t entity) = 0;
    virtual std::uint32_t next_sibling(std::uint32_t entity) = 0;
    virtual std::uint32_t hierarchy_parent(std::uint32_t entity) = 0;
    virtual bool destroy_child_predicate(std::uint32_t child, std::uint32_t parent) = 0; // 00926D0E vtable[78h]

    // Deferred queues. 00926C80 pushes onto the list at 00F899A8 and 00926D90
    // onto the one at 00F899B4; 009273A0 drains both.
    virtual void queue_pending_destroy(std::uint32_t entity) = 0; // 00926D4E
    virtual void queue_pending_kill(std::uint32_t entity) = 0;    // 00926E40
    virtual void enter_entity_lock() = 0;                         // 009248D0 + 00CE2218
    virtual void leave_entity_lock() = 0;                         // 00CE2210

    // Contracts: named, not reconstructed.
    virtual void telemetry_event(std::uint32_t entity, const char* kind,
                                 float old_fraction, float new_fraction) = 0; // 00986B00
    virtual void send_health_message(std::uint32_t entity, int health_byte) = 0; // 00876D30 + 0077C2A0
    virtual void send_death_message(std::uint32_t entity, int message_id) = 0;   // 0075B430 + 0077C2A0
    virtual void send_breakup_message(std::uint32_t entity) = 0;                 // 00761310 + 0077C2A0
    virtual bool breakup_roll(std::uint32_t entity) = 0;  // 0092BEA0/0092BE90/00424C40/00BD2F10
    virtual float descriptor_death_selector(std::uint32_t descriptor) = 0; // [desc+B0h]
    virtual float descriptor_half_width(std::uint32_t descriptor) = 0;     // [desc+A4h]
    virtual float descriptor_half_length(std::uint32_t descriptor) = 0;    // [desc+A0h]
    virtual void send_destroy_message(std::uint32_t entity, int cause, int recurse) = 0; // 0077D1E4..0077D236
    virtual bool destroy_broadcast_enabled() = 0;   // byte at 00E0AF20
    virtual bool telemetry_enabled() = 0;           // dword at 00F8A0C4
    virtual void clear_sink_fields(std::uint32_t entity) = 0;       // 00811116, 0081111E
    virtual void release_sink_attachment(std::uint32_t entity) = 0; // 00811135
    virtual std::uint32_t parts_object(std::uint32_t entity) = 0;   // 0080E490
    virtual std::size_t part_count(std::uint32_t parts) = 0;        // ([+314h]-[+310h])/4
    virtual float part_health(std::uint32_t parts, std::size_t index) = 0;
    virtual void write_part_health(std::uint32_t parts, std::size_t index, float value) = 0;
    virtual void detach_part(std::uint32_t parts, std::size_t index) = 0; // 00934150
    virtual std::size_t group_member_count(std::uint32_t group) = 0;      // [+3CCh]
    virtual std::uint32_t group_member(std::uint32_t group, std::size_t index) = 0; // [+3D0h+i*4]
    virtual std::size_t group_vector_size(std::uint32_t group) = 0;       // ([+39Ch]-[+398h])/4
    virtual std::uint32_t group_vector_member(std::uint32_t group, std::size_t index) = 0;
    virtual void clear_group_vector_member(std::uint32_t group, std::size_t index) = 0;
};

// Class ids the Kill binding tests through vtable[5Ch] at 008AC729 / 008AC740.
inline constexpr int kKillGroupArrayClassId = 0x18;  // -> 007ED380
inline constexpr int kKillGroupVectorClassId = 0x1A; // -> 00742210

// Session message ids seen on the death path.
inline constexpr int kUnitDeathSessionMessageId = 0x9A;   // 00827AE5
inline constexpr int kUnitDestroySessionMessageId = 0x4E; // 0077D1E4
inline constexpr int kUnitDeathSessionChannel = 7;        // 00827B72
inline constexpr int kUnitHealthSessionChannel = 4;       // 00877C91

// ---------------------------------------------------------------------------
// The six binding handlers and the native routines, as sequences over the host.
// ---------------------------------------------------------------------------
int lua_add_damage_0088e000(UnitDamageHost& host);
int lua_kill_008ac5c0(UnitDamageHost& host);
int lua_sink_00891b20(UnitDamageHost& host);
int lua_set_invincible_00897a50(UnitDamageHost& host);
int lua_is_invincible_00897cb0(UnitDamageHost& host);
int lua_explode_to_parts_0088e1b0(UnitDamageHost& host);

void unit_add_damage_0095da00(UnitDamageHost& host, std::uint32_t entity, float amount);
void scale_damage_for_player_unit_0087d730(UnitDamageHost& host, std::uint32_t entity, float amount);
void unit_apply_damage_00879070(UnitDamageHost& host, std::uint32_t entity, float amount);
void unit_set_health_00877b90(UnitDamageHost& host, std::uint32_t entity, float health);
void ship_on_health_changed_00827a90(UnitDamageHost& host, std::uint32_t entity);
void unit_react_to_health_change_00958a30(UnitDamageHost& host, std::uint32_t entity);
void entity_set_invincible_0042ed80(UnitDamageHost& host, std::uint32_t entity, float value);
void unit_sink_008110f0(UnitDamageHost& host, std::uint32_t entity);
// Only recurse's low byte gates children. After each accepting child predicate,
// reread the parent's cause and clear the child's cause if that value is zero.
void entity_destroy_00926c80(UnitDamageHost& host, std::uint32_t entity, int recurse);
void entity_kill_00926d90(UnitDamageHost& host, std::uint32_t entity, int cause);
void unit_destroy_and_broadcast_0077d1a0(UnitDamageHost& host, std::uint32_t entity, int recurse);
void group_kill_members_array_007ed380(UnitDamageHost& host, std::uint32_t group, int cause);
void group_kill_members_vector_00742210(UnitDamageHost& host, std::uint32_t group, bool hard);
void parts_detach_all_live_00935c70(UnitDamageHost& host, std::uint32_t parts);

// 009273A0's second loop, the only on-killed dispatch found. The flag writes at
// 009274CE..009274DA happen before the dispatch and are the host's job.
void flush_pending_kill_dispatch_0092751f(UnitDamageHost& host, std::uint32_t entity);

}  // namespace bsp
