#pragma once

// cc13_seg68: segment 68 (009F6090-009FE120) reference models. Packet cc13-seg68;
// evidence and the adoption notes for cc9 are in docs/CC13_SEG68.md.
//
// The squadron free-attack auto-target (its three target filters, the selector,
// the tick and the AttackLists loader), the weapon director's auto-target
// constructor, the approach target ref (the "precision" / "bullpos" / "error" /
// "projtime" / "vehicle" archive record) over the native observer storage, the
// plane gun object and its direction, the converging-leg lock singleton, and
// small unit and entity helpers.
//
// Every name is a hypothesis, not a recovered symbol. These are new C++
// interfaces, not ABI-compatible replacements; build-tested only, nothing is
// wired into the game. Every engine call the image makes is a named host hook
// invoked at the same point, named after the callee's address. `Handle` is a
// native pointer value the host resolves; 0 is null.
//
// Precision: gameplay x87 code runs at 24-bit precision on the main thread and
// at 53-bit on the frame-job workers (docs/CC13_FPU_PRECISION.md). Where every
// operand is a float, these models compute in float, which rounds each
// operation the way the 24-bit control word does (the exponent range aside).
// Each x87 site is listed in the doc with its operation order.

#include "bsp/band_009f_cc13.hpp"
#include "bsp/bot_fire_target.hpp"
#include "bsp/gun_aim_terms.hpp"
#include "bsp/native_land_state_lifetime.hpp"
#include "bsp/observer_lifetime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
namespace cc13_seg68 {

using Handle = std::uint32_t;

// The image's command-class singletons this segment compares against
// (docs/SCENE_COMMAND_TYPES.md, include/bsp/attack_commands.hpp).
inline constexpr Handle kCmdTorpedo = 0x00E08F18u;
inline constexpr Handle kCmdDiveBomb = 0x00E08F20u;
inline constexpr Handle kCmdLevelBomb = 0x00E08F28u;
inline constexpr Handle kCmdDropKamikaze = 0x00E08F30u;
inline constexpr Handle kCmdDepthCharge = 0x00E08F38u;
inline constexpr Handle kCmdStrafe = 0x00E08F40u;
inline constexpr Handle kCmdRocket = 0x00E08F48u;
inline constexpr Handle kCmdKamikaze = 0x00E08F50u;
inline constexpr Handle kCmdDogfight = 0x00E08F58u;
inline constexpr Handle kCmdMoveTo = 0x00E08F68u;   // issued by 009F85F4 and 009F7D6E
inline constexpr Handle kCmd8F88 = 0x00E08F88u;     // 009F85E3: clears, then moveto self

// ===========================================================================
// Weapon-director auto-target (the 40h selector at director+38h)
// ===========================================================================

struct WeaponDirectorAutoTargetHost {
    virtual ~WeaponDirectorAutoTargetHost() = default;
    virtual Handle director_unit_24c(Handle director) = 0;  // [director+24Ch]
    // The IsKindOf answers 009F65E0 asks of the owner unit.
    virtual AutoTargetOwnerKinds owner_kinds_009f65e0(Handle unit) = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange, ECX = stream.
    virtual float random_uniform_00bd2f10(int stream, float lo, float hi) = 0;
};

// The 40h object. The search state at +10h holds the priority vector
// (+14h..+1Ch), the range (+20h), the owner unit (+24h) and the best (+28h).
struct WeaponDirectorAutoTarget {
    std::uint32_t vtable_00{0};
    Handle owner_director_04{0};
    Handle unit_08{0};
    Handle director_0c{0};
    std::uint32_t search_vtable_10{0};       // never written by 009F6A20
    AutoTargetSearchConfig search;           // +14h..+1Ch vector, +20h range
    Handle search_owner_24{0};
    Handle best_candidate_28{0};
    float best_score_2c{0.0f};               // not written by the constructor
    bool issue_move_30{false};               // not written by the constructor
    AutoTargetState state;                   // +34h, +38h, +3Ch
};

inline constexpr std::uint32_t kAutoTargetVtableDerived = 0x00D21B48u;
inline constexpr std::uint32_t kAutoTargetVtableBase = 0x00D21B40u;

// 009F69C0 __thiscall(searchState, unit) RET 4.
void auto_target_search_init_009f69c0(WeaponDirectorAutoTargetHost& host,
                                      WeaponDirectorAutoTarget& self, Handle unit);
// 009F6A20 __thiscall(selector, director) RET 4, returns the selector.
WeaponDirectorAutoTarget& weapon_director_auto_target_construct_009f6a20(
    WeaponDirectorAutoTargetHost& host, WeaponDirectorAutoTarget& self, Handle director);
// 009F60C0 __fastcall(selector): the destructor body.
void weapon_director_auto_target_destruct_009f60c0(WeaponDirectorAutoTarget& self);

// 009F6AD0 __fastcall(controller) -> bool: [controller+3Ch] == 0.
bool command_controller_fire_forbidden_009f6ad0(std::uint8_t byte_3c) noexcept;
// ===========================================================================
// Squadron free-attack auto-target (40h, vtable 00D21C00)
// ===========================================================================

// Which of the three embedded filters a pointer names (009F6C80's answers).
using SquadronFilter = band009f::SquadronTargetFilter;

struct PathPoint {
    std::array<float, 3> p{};
    bool valid{false};
};

struct SquadronCandidateTrace;   // packet cc13-fapick, defined below

struct SquadronAutoTargetHost {
    virtual ~SquadronAutoTargetHost() = default;
    // Native reads.
    virtual std::uint8_t u8(Handle object, std::uint32_t offset) = 0;
    virtual std::uint32_t u32(Handle object, std::uint32_t offset) = 0;
    virtual float f32(Handle object, std::uint32_t offset) = 0;
    // Virtual calls.
    virtual bool is_kind_of_5c(Handle entity, int kind) = 0;          // entity vtable+5Ch
    virtual bool class_query_18(Handle class_object, int kind) = 0;    // class vtable+18h
    virtual Handle entity_vt140(Handle entity) = 0;                    // entity vtable+140h
    virtual int director_vt50(Handle director) = 0;                    // director vtable+50h
    // Direct calls, by callee address.
    virtual void refresh_pose_00414db0(Handle entity) = 0;
    virtual Handle ggame_00e188a8() = 0;                               // [00E188A8]
    virtual bool point_outside_map_0071c4f0(Handle ggame, Handle point) = 0;
    virtual bool command_blocks_auto_action_00811f50(Handle unit, int arg) = 0;
    virtual bool squadron_outside_map_detached_007ee8b0(Handle squadron) = 0;
    virtual Handle recon_slot_008053c0(Handle object) = 0;
    virtual bool sensors_detect_unit_00804350(Handle unit, Handle target, std::uint32_t mask) = 0;
    virtual Handle choose_attack_command_007eec50(Handle squadron, Handle target,
                                                  std::uint8_t flag, int one) = 0;
    virtual Handle ai_weights_block_004d6ce0() = 0;
    virtual PathPoint current_path_point_0071e4c0(Handle director) = 0;
    virtual Handle game_tuning_0042e740() = 0;
    virtual Handle command_target_resolve_00521ea0(Handle descriptor) = 0;
    virtual Handle static_zero_vector_00f87574() = 0;
    virtual SquadronFilter pick_filter_009f6c80(Handle self) = 0;
    virtual void revalidate_current_command_009f8160(Handle self) = 0;  // hosted by cc9
    virtual void ordnance_latch_009f7c90(Handle self) = 0;              // hosted by cc9
    virtual bool party_slot_is_ai_held_00927f10(Handle leader, std::uint32_t slot) = 0;
    virtual bool director_accepts_new_target_0071df70(Handle director) = 0;
    virtual void send_clear_commands_0071d880(Handle director) = 0;
    virtual Handle world_position_refreshed_00427eb0(Handle entity) = 0;
    // 00468560 / 00465080 build a command-target descriptor in a caller local
    // and return it; the handle stands for that local.
    virtual Handle command_target_init_position_00468560(Handle position, float zero) = 0;
    virtual Handle command_target_from_entity_00465080(Handle entity, float zero) = 0;
    virtual void issue_command_0071ecf0(Handle director, Handle command, Handle target) = 0;
    virtual void send_override_command_0071d980(Handle director, Handle command,
                                                Handle target) = 0;
    virtual void raise_override_stage_0071d9e0(Handle director, int stage) = 0;
    // Packet cc13-fapick, log-only: 009F7810 reports each recon-list entity once,
    // with where it dropped or its score. Not an image call; the default does nothing.
    virtual void trace_candidate_009f7810(Handle entity, const SquadronCandidateTrace& trace) {
        static_cast<void>(entity);
        static_cast<void>(trace);
    }
};

// Packet cc13-fapick: where 009F7810's loop left a candidate. The fields are
// written as the existing predicates answer; nothing is evaluated for them.
enum class SquadronCandidateNote {
    Scored,             // reached 009F78B2
    SinglePlane,        // 009F7491 IsKindOf(0Fh)
    OutsideMapBlocked,  // 009F74BD outside the map and 00811F50(0)
    SquadronDetached,   // 009F74E9 007EE8B0
    Slot140Blocks,      // 009F74FA / 009F7876 / 009F7010 [vtable[140h]()]+1D4h
    Kind1bCarrier,      // 009F752D inner 46h/45h
    GuardOut,           // filter C 009F7F40 on the entity, in 009F7480
    NotUnitOrSquadron,  // 009F7BB8/009F7BC7 neither 5 nor 18h (or no leader)
    NoDetect,           // 009F7BF5 00804350 false
    NoRank,             // 009F7C2B weight 0 (not in the chosen AttackLists vector)
    LeaderGuardOut,     // filter C 009F7F40 on the unit (a squadron: its leader), in 009F7BA0
};
struct SquadronCandidateTrace {
    SquadronCandidateNote note{SquadronCandidateNote::Scored};
    int rank{0};
    float score{0.0f};
};

struct SquadronAutoTarget;

// The selector at +2Ch: {owner, filter, current target, interval}.
struct SquadronSelector {
    SquadronAutoTarget* owner_00{nullptr};
    SquadronFilter filter_04{SquadronFilter::At14};
    Handle target_08{0};
    float interval_0c{0.0f};
    std::uintptr_t native_address{0};  // the selector's own address, read by 009F7816
};

struct SquadronAutoTarget {
    std::uint32_t vtable_00{0};
    Handle director_04{0};
    Handle squadron_08{0};
    std::uint32_t filter_a_vtable_0c{0};     // owner +10h = this
    std::uint32_t filter_b_vtable_14{0};     // owner +18h = this
    std::uint32_t filter_c_vtable_1c{0};     // owner +20h = this
    std::uint8_t filter_c_leader_class_13_24{0};
    std::uint8_t rockets_previous_28{0};
    SquadronSelector selector_2c;
    std::uint8_t latch_3c{0};
    std::uint8_t latch_3d{0};
    std::uint8_t latch_3e{0};
};

inline constexpr std::uint32_t kSquadronAutoTargetVtable = 0x00D21C00u;
inline constexpr std::uint32_t kFilterAVtable = 0x00D21BC8u;
inline constexpr std::uint32_t kFilterBVtable = 0x00D21B98u;
inline constexpr std::uint32_t kFilterCVtable = 0x00D21BE4u;
inline constexpr std::uint32_t kFilterRootVtable = 0x00D21B50u;

// 009F7590 __thiscall(this, director, squadron) RET 8; 009F7210 builds filter C.
SquadronAutoTarget& squadron_auto_target_construct_009f7590(
    SquadronAutoTargetHost& host, SquadronAutoTarget& self, Handle director,
    Handle squadron, std::uintptr_t selector_native_address);
// 009F7210 __thiscall(filterC, owner) RET 4: the byte at filter+8 (owner+24h).
void filter_c_construct_009f7210(SquadronAutoTargetHost& host, SquadronAutoTarget& self);
// 009F6DD0 __fastcall(this).
void squadron_auto_target_clear_latches_009f6dd0(SquadronAutoTarget& self) noexcept;
// 009F6D60 __thiscall(this, target, flag) RET 8.
Handle squadron_choose_attack_command_009f6d60(SquadronAutoTargetHost& host,
                                               const SquadronAutoTarget& self,
                                               Handle target, std::uint8_t flag);

// The filter vtable slots. Slot +4: enabled; +8: score; +0Ch: accept; +10h:
// a boolean; +14h: the AttackLists weight; +18h: the distance.
bool filter_enabled_slot4(SquadronFilter filter) noexcept;   // 009F6C10 / 009F7070
bool filter_slot10(SquadronFilter filter) noexcept;          // 009F70E0 / 009F7090 / 009F7270
float filter_score_slot8(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                         SquadronFilter filter, Handle target,
                         SquadronCandidateTrace* trace = nullptr);
bool filter_accept_slotc(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                         SquadronFilter filter, Handle target);
float filter_distance_slot18(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                             SquadronFilter filter, Handle target);

// 009F6B10 (root slot +8, also 009F7080 in filter B): 0.0.
float filter_default_score_009f6b10() noexcept;
// 009F7010 __stdcall-shaped (ECX unused) (entity) RET 4.
bool filter_accept_unit_009f7010(SquadronAutoTargetHost& host, Handle entity);
// 009F7090 / 009F7270: false.
bool filter_slot10_false_009f7090() noexcept;
bool filter_c_slot10_false_009f7270() noexcept;
// 009F7150 __thiscall(filterA, target) RET 4.
float filter_distance_to_target_009f7150(SquadronAutoTargetHost& host,
                                         const SquadronAutoTarget& self, Handle target);
// 009F72A0 __thiscall(filterC, target) RET 4.
float filter_c_distance_score_009f72a0(SquadronAutoTargetHost& host,
                                       const SquadronAutoTarget& self, Handle target);
// 009F7F40 __thiscall(filterC, target) RET 4. Not a Ghidra function when seg68 was
// written; Ghidra now has FUN_009f7f40 (body 009F7F40-009F7FDA), named
// BSP_AutoTargetFilterC_AcceptGuardCircle by cc13-nits. Game code, not library.
bool filter_c_accept_guard_circle_009f7f40(SquadronAutoTargetHost& host,
                                           const SquadronAutoTarget& self, Handle target);
// 009F7BA0 __thiscall(filter, target) RET 4.
float filter_score_target_009f7ba0(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                                   SquadronFilter filter, Handle target,
                                   SquadronCandidateTrace* trace = nullptr);
// 009F9770 __thiscall(filter, target, arg) RET 8.
int ai_attack_class_weight_009f9770(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                                    Handle target, std::uint32_t arg);
// 009F7B00 __thiscall(block, vector, entity) RET 8 (not a Ghidra function name).
int ai_attack_list_rank_009f7b00(SquadronAutoTargetHost& host, Handle vector, Handle entity);
// 009F6BE0 __fastcall(unit): IsKindOf(17h) and the byte [unit+C24h].
bool unit_is_kind17_with_flag_c24_009f6be0(SquadronAutoTargetHost& host, Handle unit);
// 009F7B80 __fastcall(unit): a kind-0Fh unit's [unit+9D4h], else the unit.
Handle unit_resolve_kind0f_owner_009f7b80(SquadronAutoTargetHost& host, Handle unit);
// 009F6AF0 __fastcall(ref) -> [[ref]+4].
Handle auto_target_ref_inner_owner_009f6af0(SquadronAutoTargetHost& host, Handle ref);

// 009F7480 __thiscall(selector, entity) RET 4.
bool selector_candidate_allowed_009f7480(SquadronAutoTargetHost& host,
                                         const SquadronSelector& selector, Handle entity,
                                         SquadronCandidateTrace* trace = nullptr);
// 009F7810 __thiscall(selector, float dt) RET 4 (dt unused).
void selector_pick_best_009f7810(SquadronAutoTargetHost& host, SquadronSelector& selector,
                                 float dt);
// 009F7910 __thiscall(selector, float dt) RET 4.
void selector_update_009f7910(SquadronAutoTargetHost& host, SquadronSelector& selector,
                              float dt);
// 009F7FE0 __thiscall(this, descriptor, float scale) RET 8.
bool bot_within_guard_distance_009f7fe0(SquadronAutoTargetHost& host,
                                        const SquadronAutoTarget& self, Handle descriptor,
                                        float scale);
// 009F8470 __thiscall(this, float dt) RET 4: vtable 00D21C00 slot 1.
void squadron_auto_target_tick_009f8470(SquadronAutoTargetHost& host, SquadronAutoTarget& self,
                                        Handle self_native, float dt);

// ===========================================================================
// The AttackLists loader (009F89F0) and its key formatter (009F6F40)
// ===========================================================================

// 009F6F40 __cdecl(buf, fmt, ...): vsprintf_s(buf, 4, fmt, args).
int format_string4_009f6f40(char* buffer, const char* format, ...);

struct FreeAttackLoaderHost {
    virtual ~FreeAttackLoaderHost() = default;
    // Lua state owner (a caller local), 00B66BD0 / 00B6A020 / 00B69D40 / 00B669A0.
    virtual void lua_state_construct_00b66bd0() = 0;
    virtual void lua_state_open_00b6a020(int flags) = 0;
    virtual void lua_run_script_00b69d40(const char* path, int arg) = 0;
    virtual void lua_state_close_00b669a0() = 0;
    // Lua objects: handles to caller locals. 00B67980 and 00B67800 construct
    // their result into `out` and return it.
    virtual Handle lua_globals_00b67980(Handle out) = 0;
    virtual Handle lua_get_by_name_00b67800(Handle object, Handle out, const char* key) = 0;
    virtual void lua_object_construct_00b65f50(Handle object) = 0;
    virtual void lua_object_assign_00b67690(Handle object, Handle from) = 0;
    virtual void lua_object_destruct_00b67700(Handle object) = 0;
    virtual bool lua_is_nil_00b65fb0(Handle object) = 0;
    virtual bool lua_is_string_00b660a0(Handle object) = 0;
    virtual const char* lua_get_string_00b662b0(Handle object) = 0;
    virtual int lua_get_integer_00b66290(Handle object) = 0;
    // The frame's Lua object locals: ESP+5Ch AttackLists, ESP+48h the list,
    // ESP+34h the value, ESP+1Ch the lookup temporary, ESP+B0h the globals.
    virtual Handle local_lists_object() = 0;
    virtual Handle local_list_object() = 0;
    virtual Handle local_value_object() = 0;
    virtual Handle local_temp_object() = 0;
    virtual Handle local_globals_object() = 0;
    // 00E0CD80[i], the 97-entry class-name table.
    virtual const char* class_name_00e0cd80(int index) = 0;
    // 009F8960, push_back on the vector at record+offset.
    virtual void push_back_009f8960(Handle record, std::uint32_t offset, int value) = 0;
};

inline constexpr int kAttackListKeyCount = 0x18;   // 009F8CE6
inline constexpr int kClassNameCount = 0x61;       // 009F8CBC

// 009F89F0 __fastcall(record), the 004D4A20 weights block.
void squadron_free_attack_targets_load_009f89f0(FreeAttackLoaderHost& host, Handle record);

// ===========================================================================
// Converging-leg lock and manager plumbing
// ===========================================================================

struct LegLock;

struct LegSingletonHost {
    virtual ~LegSingletonHost() = default;
    virtual Handle singleton_manager_get() = 0;                    // BSP_SingletonLifetime_GetManager
    virtual void singleton_register(Handle manager, Handle object) = 0;
    virtual Handle manager_section_10(Handle manager) = 0;         // [manager+10h]
    virtual void enter_critical_section(Handle section) = 0;
    virtual void leave_critical_section(Handle section) = 0;
    virtual void section_depth_add_18(Handle section, int delta) = 0;  // ++/-- [section+18h]
    virtual Handle operator_new(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(Handle block) = 0;
    virtual Handle critical_section_create() = 0;                  // BSP_CriticalSection_Create
    virtual void critical_section_destroy_owned(Handle lock) = 0;  // BSP_CriticalSection_DestroyOwned
    virtual void copy16(Handle destination, Handle source) = 0;    // the 10h-byte record copy
    virtual Handle& global_00f8a724() = 0;                         // the lock singleton
    virtual Handle& global_00f8a720() = 0;                         // the manager singleton
    virtual LegLock& lock_at(Handle object) = 0;            // the 8-byte block as a lock
};

struct Array16 {
    Handle data_00{0};
    std::uint32_t count_04{0};
    std::uint32_t capacity_08{0};
};

struct LegLock {
    std::uint32_t vtable_00{0};
    Handle section_04{0};
};

struct LegManager {
    std::uint32_t vtable_00{0};
    Array16 records_04;
};

inline constexpr std::uint32_t kLegLockVtable = 0x00D21C7Cu;
inline constexpr std::uint32_t kLegManagerVtable = 0x00D21C80u;
inline constexpr std::uint32_t kSingletonBaseVtable = 0x00CE3818u;

void array16_grow_009f9b60(LegSingletonHost& host, Array16& array);
void array16_push_back_009fa430(LegSingletonHost& host, Array16& array, Handle record);
void free_owned_buffer_009f9ad0(LegSingletonHost& host, Handle& buffer);
LegLock& leg_lock_construct_009fa9f0(LegSingletonHost& host, LegLock& self);
void leg_lock_reset_singleton_009f9a70(LegSingletonHost& host, LegLock& self);
void leg_lock_destruct_009fb120(LegSingletonHost& host, LegLock& self);
// 009FB010 __cdecl() -> the lock; builds it under the manager's section.
Handle leg_lock_get_009fb010(LegSingletonHost& host);
void leg_manager_construct_009faa40(LegManager& self) noexcept;
void leg_manager_destruct_009faa60(LegSingletonHost& host, LegManager& self);

// ===========================================================================
// Unit helpers and the world-edge overrun
// ===========================================================================

// 009F9BE0 __thiscall(unit, i) RET 4: float [unit+460h+4i].
float unit_blast_accumulator_get_009f9be0(const float* unit_460, int index) noexcept;
// 009F9BF0 __fastcall(unit): ([+474h] + [+464h]) + [+478h], stored float.
float unit_blast_accumulator_sum_009f9bf0(float f464, float f474, float f478) noexcept;

struct RandomHost {
    virtual ~RandomHost() = default;
    virtual float random_uniform_00bd2f10(int stream, float lo, float hi) = 0;
};
// 009F9C80 __fastcall(out[2]) -> out: (sin a, cos a), a = U1(0, 2pi).
float* random_unit_circle_direction_009f9c80(RandomHost& host, float* out);

// 009FA510 __thiscall(GGame, const float* p, float margin) RET 8: the largest
// margin overrun of (p.x, p.z) past the world rectangle at GGame+711Ch..7130h.
struct WorldEdges {
    float x_min_711c{0.0f};
    float z_max_7124{0.0f};
    float x_max_7128{0.0f};
    float z_min_7130{0.0f};
};
float ggame_world_edge_overrun_009fa510(const WorldEdges& edges, const float* p,
                                        float margin) noexcept;

// 009FE060 __fastcall(unit): a squadron's member count [+3CCh], else 1.
int unit_kind18_count_009fe060(SquadronAutoTargetHost& host, Handle unit);

// ===========================================================================
// The plane gun object (task+314h, built by 009FAAD0)
// ===========================================================================

struct BotTaskGun {
    Handle owner_00{0};
    GunAimWander wander_04;                    // +04h..+24h
    float f28{0.0f};                           // -1.0 (00D7A260)
    float f2c{0.0f}, f30{0.0f}, f34{0.0f}, f38{0.0f};
    float f3c{0.0f};                           // 009FAB60's last argument
    float cone_40{0.0f};
    float f44{0.0f};                           // 999999.0 (00CF87D0)
    std::uint8_t b48{0}, b49{0}, b4a{0}, b4b{0};
    float f4c{0.0f}, f50{0.0f};
    float untouched_54[5]{};
    std::array<float, 3> direction_68{};
    std::uint32_t u74{0};
};
static_assert(offsetof(BotTaskGun, f28) == 0x28);
static_assert(offsetof(BotTaskGun, f3c) == 0x3c);
static_assert(offsetof(BotTaskGun, f44) == 0x44);
static_assert(offsetof(BotTaskGun, b4b) == 0x4b);
static_assert(offsetof(BotTaskGun, direction_68) == 0x68);
static_assert(offsetof(BotTaskGun, u74) == 0x74);

struct GunHost {
    virtual ~GunHost() = default;
    virtual void reseed_aim_wander_009fa620(GunAimWander& wander) = 0;
    virtual float tangent_00412e20(float angle) = 0;
    virtual float vector_length_00419440(const std::array<float, 3>& v) = 0;
};

// 009FAAD0 __thiscall(gun, owner) RET 4.
BotTaskGun& bot_task_gun_construct_009faad0(GunHost& host, BotTaskGun& self, Handle owner);
// 009FAB60 __thiscall(gun, accel, scale, reach, f3c) RET 10h.
void bot_task_gun_init_aim_009fab60(GunHost& host, BotTaskGun& self, float accel, float scale,
                                    float reach, float f3c);
// 009FABE0 __thiscall(gun, heading, pitch) RET 8.
void bot_set_direction_from_heading_pitch_009fabe0(GunHost& host, BotTaskGun& self,
                                                   float heading, float pitch);

// ===========================================================================
// The approach target ref (vtable 00D21CB4) over the native observer storage
// ===========================================================================

struct NativeApproachTargetRef {
    NativeObserverOwnerStorage observer_00;           // vtable +0h, edges +4h..+0Ch
    std::uint8_t observer_byte_10{0};
    std::uint8_t pad_11[3]{};
    NativeObserverOwnerStorage* vehicle_14{nullptr};
    NativeObserverOwnerStorage* target_18{nullptr};
    std::array<float, 3> world_point_1c{};
    std::array<float, 3> body_offset_28{};
    std::array<float, 3> bias_34{};                   // "error"
    std::uint8_t has_target_40{0};
    std::uint8_t dirty_41{0};
    std::uint8_t pad_42[2]{};
    float projtime_44{0.0f};                          // "projtime"
    std::array<float, 3> spread_48{};                 // "precision"
    std::array<float, 3> centre_54{};                 // "bullpos"
    float repick_timer_60{0.0f};
    float section_chance_64{0.0f};
    float weight_68{0.0f};
    float weight_6c{0.0f};
    float weight_70{0.0f};
};
static_assert(offsetof(NativeApproachTargetRef, observer_byte_10) == 0x10);
static_assert(offsetof(NativeApproachTargetRef, vehicle_14) == 0x14);
static_assert(offsetof(NativeApproachTargetRef, world_point_1c) == 0x1c);
static_assert(offsetof(NativeApproachTargetRef, bias_34) == 0x34);
static_assert(offsetof(NativeApproachTargetRef, has_target_40) == 0x40);
static_assert(offsetof(NativeApproachTargetRef, projtime_44) == 0x44);
static_assert(offsetof(NativeApproachTargetRef, centre_54) == 0x54);
static_assert(offsetof(NativeApproachTargetRef, weight_70) == 0x70);
static_assert(sizeof(NativeApproachTargetRef) == 0x74);

inline constexpr std::uint32_t kApproachTargetRefVtable = 0x00D21CB4u;

struct ApproachRefHost {
    virtual ~ApproachRefHost() = default;
    virtual NativeObserverLifetime& observers() = 0;
    virtual bool is_kind_of_5c(NativeObserverOwnerStorage* entity, int kind) = 0;
    // entity vtable+34h(out): the velocity.
    virtual std::array<float, 3> entity_velocity_34(NativeObserverOwnerStorage* entity) = 0;
    // message vtable+4(): the subject the notice is about.
    virtual NativeObserverOwnerStorage* message_subject_04(Handle message) = 0;
    virtual void refresh_pose_00414db0(NativeObserverOwnerStorage* entity) = 0;
    virtual bool pose_valid_c8(NativeObserverOwnerStorage* entity) = 0;          // byte +C8h
    // 004142E0(point, out, entity+CCh).
    virtual std::array<float, 3> transform_affine_point_004142e0(
        const std::array<float, 3>& point, NativeObserverOwnerStorage* entity) = 0;
    virtual const std::array<float, 3>& static_zero_vector_00f87574() = 0;
    virtual float random_uniform_00bd2f10(int stream, float lo, float hi) = 0;
    virtual void pick_body_frame_aim_offset_009fa260(NativeApproachTargetRef& ref) = 0;
    virtual void update_009fada0(NativeApproachTargetRef& ref, float dt) = 0;
    // Archive writer (ref+8 of the by-value ArchiveWriterRef) vtable+0Ch.
    virtual void archive_write_0c(Handle writer, int zero, const char* key, int type,
                                  std::uintptr_t value) = 0;
    virtual void archive_writer_ref_destroy_004107b0(Handle ref) = 0;
    // Archive reader (ref+8) vtable+10h, and the ref's own vtable+10h for a
    // pointer field (an 8-byte {4, &slot} descriptor by value).
    virtual void archive_read_10(Handle reader, int zero, const char* key, int type,
                                 void* destination) = 0;
    virtual void archive_ref_read_pointer_10(Handle ref, int zero, const char* key,
                                             NativeObserverOwnerStorage** slot) = 0;
    virtual void archive_reader_ref_destroy_00410630(Handle ref) = 0;
};

// 009FA2E0 __thiscall(ref, out) RET 4 -> out.
std::array<float, 3> approach_ref_target_velocity_009fa2e0(ApproachRefHost& host,
                                                           const NativeApproachTargetRef& self);
// 009FA350 __fastcall(ref).
void approach_ref_reset_aim_offset_009fa350(ApproachRefHost& host, NativeApproachTargetRef& self);
// 009FA380 __thiscall(ref, const float* error) RET 4.
void approach_ref_set_aim_error_009fa380(NativeApproachTargetRef& self,
                                         const std::array<float, 3>& error) noexcept;
// 009FACC0 __thiscall(ref, ArchiveWriterRef by value) RET 0Ch.
void approach_ref_save_009facc0(ApproachRefHost& host, const NativeApproachTargetRef& self,
                                Handle writer_ref, Handle writer);
// 009FAF90 __thiscall(ref, message) RET 4: vtable 00D21CB4 slot 1.
void approach_ref_on_target_destroyed_009faf90(ApproachRefHost& host,
                                               NativeApproachTargetRef& self, Handle message);
// 009FB0F0 / 009FB6C0 __thiscall(ref, vehicle) RET 4 (009FB6C0 returns ref).
void approach_ref_set_vehicle_009fb0f0(ApproachRefHost& host, NativeApproachTargetRef& self,
                                       NativeObserverOwnerStorage* vehicle);
NativeApproachTargetRef& approach_ref_assign_vehicle_009fb6c0(
    ApproachRefHost& host, NativeApproachTargetRef& self, NativeObserverOwnerStorage* vehicle);
// 009FB190 __thiscall(ref, target) RET 4.
void approach_ref_bind_target_009fb190(ApproachRefHost& host, NativeApproachTargetRef& self,
                                       NativeObserverOwnerStorage* target);
// 009FB5E0 __thiscall(ref, target) RET 4.
void approach_ref_set_target_009fb5e0(ApproachRefHost& host, NativeApproachTargetRef& self,
                                      NativeObserverOwnerStorage* target);
// 009FB640 / 009FB670 __thiscall(ref, message) RET 4 (identical bodies).
void approach_ref_on_vehicle_destroyed_009fb640(ApproachRefHost& host,
                                                NativeApproachTargetRef& self, Handle message);
void approach_ref_on_vehicle_destroyed_009fb670(ApproachRefHost& host,
                                                NativeApproachTargetRef& self, Handle message);
// 009FB3E0 __thiscall(ref, ArchiveReaderRef by value) RET 0Ch -> ref.
NativeApproachTargetRef& approach_ref_construct_from_stream_009fb3e0(
    ApproachRefHost& host, NativeApproachTargetRef& self, Handle reader_ref, Handle reader);

// 009FCF90 __thiscall(holder, rocket) RET 4 -> holder: the observed pointer at
// holder+14h, the same shape as 009FB6C0 (holder = the opportunistic-rocket
// state +44h).
struct ObservedPointerHolder {
    NativeObserverOwnerStorage observer_00;
    std::uint8_t observer_byte_10{0};
    std::uint8_t pad_11[3]{};
    NativeObserverOwnerStorage* subject_14{nullptr};
};
static_assert(offsetof(ObservedPointerHolder, subject_14) == 0x14);
ObservedPointerHolder& opportunistic_rocket_set_rocket_ref_009fcf90(
    NativeObserverLifetime& observers, ObservedPointerHolder& self,
    NativeObserverOwnerStorage* rocket);

// 009FCFD0 __thiscall(vector, const element&) RET 4.
void land_state_observer_vector_push_back_009fcfd0(
    NativeLandStateObserverVector& vector, const NativeLandStateObserverElement24& source,
    NativeObserverLifetime& lifetime);

}  // namespace cc13_seg68
}  // namespace bsp
