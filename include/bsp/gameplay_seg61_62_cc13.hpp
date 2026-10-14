#pragma once

// Segments 61 (0092E0B0-00951F20) and 62 (00951F40-00966890) reference models:
// the vehicle-class family (descriptor constructors, IsKindOf sets, plane
// instance constructors), the ship repair task's setters and adders, the
// InferiorFailure property and its session messages 7Dh/7Eh, the MT_ROLEOWNER
// device walks, the unit skill level, the unit pose setters, the controlled-unit
// audio flag, the launched-child slots, the vehicle-class reference swap, the
// submarine periscope break, the frozen-hull step, the class registry cache, the
// current-player test and three small math helpers. Packet cc13-gpr61;
// docs/CC13_GPR_61.md.
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw;
// x87 from the bytes with sc2_x87.py), not native layouts or ABI-compatible
// replacements. Field names are hypotheses; a "_NNN" suffix is the native byte
// offset. Every engine service the image calls is a callback or a returned
// action, so nothing here invents a global or stubs a call. Build-tested only;
// not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_61.md section 2):
// - Replaces: mostly nothing that exists. The host keeps the repair adders in
//   src/game_hosts_gunnery.cpp:11150 (damage_control_add_seconds, without the
//   +3Ch fire total, the +40h water copy, the +44h clear and the warning
//   statistics), the skill level as a bare index (src/game_hosts_script_orders.cpp
//   :3118), InferiorFailure as a units-host byte (src/game_hosts_lua.cpp:6836,
//   src/game_hosts_units.cpp:12864), and the class IsKindOf ids as a data table
//   (src/local_player_unit_lists.cpp:12). The role-owner walks, pose setters,
//   audio flag, launched-child slots and class reference swap have no host code.
// - Inputs: unit fields named per model below; the host gets them from its unit
//   slot (position, health +370h, repair task at unit+A20h, device lists).
// - RNG: none. No function in this file draws from 00BD2F10 on any stream.
// - OFF-switch prediction: each model is behind nothing yet; per-model
//   predictions are in the doc's table. Bound with the switch off nothing moves.

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace bsp {
namespace gameplay_seg61_62_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// 1. The vehicle-class family
// ---------------------------------------------------------------------------

// One descriptor constructor: `__thiscall(this)`, RET 0. Each calls its base
// constructor, stores the primary vtable at +0h and the secondary at +6Ch, zeroes
// the listed fields and returns this.
struct ClassDescriptorCtorRow {
    std::uint32_t ctor;
    const char* lua_type;           // the Lua class key in vehicle_class.cpp
    std::uint32_t base_ctor;        // 007D1E50 plane, 00963380 ship, 00749050 vehicle,
                                    // 00749180 command building
    std::uint32_t vtable;           // MOV [ESI], imm32
    std::uint32_t vtable_6c;        // MOV [ESI+6Ch], imm32
    std::array<std::uint16_t, 5> zeroed_dwords;  // offsets stored 0 after the vtables
    std::uint8_t zeroed_count;
};
extern const std::array<ClassDescriptorCtorRow, 21> kClassDescriptorCtors;
const ClassDescriptorCtorRow* class_descriptor_ctor(std::uint32_t ctor) noexcept;

// One IsKindOf: `__thiscall(this, id)`, RET 4. Every body is a flat CMP/JE chain
// over constants (no base-class call), so the set below is the whole function.
struct IsKindOfRow {
    std::uint32_t address;
    const char* family;
    std::array<std::uint8_t, 5> ids;  // in compare order
    std::uint8_t id_count;
};
extern const std::array<IsKindOfRow, 24> kIsKindOfRows;
const IsKindOfRow* is_kind_of_row(std::uint32_t address) noexcept;
// The body of every row: true when `id` equals one of the row's constants.
bool is_kind_of(const IsKindOfRow& row, std::int32_t id) noexcept;

// The three plane instance constructors in segment 62 (`__thiscall(this, a)`,
// RET 4): 007CFD20(this, a, 0), eight vtables, class id +C4h and +AACh.
struct PlaneInstanceCtorRow {
    std::uint32_t ctor;
    const char* m_class;
    std::array<std::uint32_t, 8> vtables;  // at kPlaneInstanceVtableOffsets
    std::uint32_t class_id_c4;
    std::uint32_t value_aac;
};
inline constexpr std::array<std::uint16_t, 8> kPlaneInstanceVtableOffsets = {
    0x000, 0x010, 0x024, 0x170, 0x1E4, 0x310, 0x38C, 0x72C};
extern const std::array<PlaneInstanceCtorRow, 3> kPlaneInstanceCtors;

// Class-name getters: MOV EAX, imm32 ; RET.
struct ClassNameGetterRow {
    std::uint32_t address;
    std::uint32_t string_address;
    const char* name;
};
extern const std::array<ClassNameGetterRow, 3> kClassNameGetters;

// 009635D0 ShipClass underwater armour: FLD [ECX+6B4h] ; RET.
float ship_class_underwater_armour_009635d0(float field_6b4) noexcept;
// 00963C70 LandingShipClass: XOR EAX,EAX ; CMP [ECX+809h], AL ; SETE AL.
bool landing_ship_class_slot_c12_clear_00963c70(std::uint8_t byte_809) noexcept;
// 00951F20: CALL 00876180 (MOV AL,1 ; RET) ; TEST AL,AL ; SETNE AL. Always true.
bool vehicle_class_derive_constants_default_00951f20() noexcept;

// The unit-controller variants, `__thiscall(this, unit)`, RET 4: 00939CB0 with
// the argument, then the vtable. 00939E50 (the submarine's, sole caller 00852210)
// also zeroes +394h/+398h and, when the unit's model ([[this+1Ch]+360h]+160h)
// has the note node "periszkop" (00D0C1F0, 0071AD50), keeps at +394h the first
// shape of the body chain ([this+2Ch] -> 00C31DC0, next +208h) whose 00C32450
// flags have bit 2.
struct ControllerVariantRow {
    std::uint32_t ctor;
    std::uint32_t vtable;
};
inline constexpr std::array<ControllerVariantRow, 3> kControllerVariants = {{
    {0x00939E50, 0x00D196EC}, {0x00939F20, 0x00D196F0}, {0x00939F40, 0x00D196F4}}};
// 00939F20 (variant C, sole caller 00857C20) and 00939F40 (variant D), both
// __thiscall(this, unit), RET 4, nine instructions: PUSH [ESP+4]; CALL 00939CB0
// with ECX = this; MOV [this], vtable; return this. `self` is the object's
// first dword (the vptr). 00939CB0 is not reconstructed: it runs through the hook.
struct ControllerVariantCtorHost {
    std::function<void(std::uint32_t* self, std::uintptr_t unit)> base_construct;  // 00939CB0
};
std::uint32_t* controller_construct_variant_c_00939f20(std::uint32_t* self, std::uintptr_t unit,
                                                       const ControllerVariantCtorHost& host);
std::uint32_t* controller_construct_variant_d_00939f40(std::uint32_t* self, std::uintptr_t unit,
                                                       const ControllerVariantCtorHost& host);
// Returns the index of the shape stored at +394h, or -1 (left 0).
int controller_b_periscope_shape_00939e50(bool model_has_periszkop,
                                          const std::vector<std::uint32_t>& shape_flags);

// ---------------------------------------------------------------------------
// 2. The ship repair task (the object at unit+A20h)
// ---------------------------------------------------------------------------

struct RepairTask {
    float water_tick_2c{0.0f};        // multiplies the water seconds in the test
    float fire_tick_30{0.0f};         // multiplies the fire seconds in the test
    float water_seconds_34{0.0f};     // 00939FA0 / 0093A4F0
    float fire_seconds_38{0.0f};      // 00939F90 / 0093A470
    float fire_total_3c{0.0f};        // 0093A470 only: a running sum of the adds
    float water_last_40{0.0f};        // 0093A4F0 only: a copy of the new +34h
    std::uint8_t byte_44{0};          // both adders clear it
    bool hull_repair_enabled_45{false};
    bool failure_repair_enabled_46{false};
};

void repair_set_fire_seconds_00939f90(RepairTask& task, float seconds) noexcept;
void repair_set_water_seconds_00939fa0(RepairTask& task, float seconds) noexcept;
void repair_set_hull_repair_enabled_00939fd0(RepairTask& task, bool enabled) noexcept;
void repair_set_failure_repair_enabled_00939fe0(RepairTask& task, bool enabled) noexcept;

// The adders' side effects, in image order, as host callbacks.
struct RepairAddHost {
    // 0093A484 00983780 / 0093A513 009832F0 on [00F8A0C4], (unit, seconds).
    std::function<void(float seconds)> warning_fire_channel_00983780;
    std::function<void(float seconds)> warning_leak_channel_009832f0;
    // Tail call 0090E6C0(ECX = [[00E188A8]+21A0h], unit) when the pending damage
    // exceeds the health.
    std::function<void()> pending_exceeds_health_0090e6c0;
};
// 0093A470 `__thiscall(task, float seconds)`, RET 4. Returns whether the tail
// call ran.
bool repair_add_fire_seconds_0093a470(RepairTask& task, float seconds, float unit_health_370,
                                      const RepairAddHost& host);
// 0093A4F0 `__thiscall(task, float seconds)`, RET 4.
bool repair_add_water_seconds_0093a4f0(RepairTask& task, float seconds, float unit_health_370,
                                       const RepairAddHost& host);

// ---------------------------------------------------------------------------
// 3. InferiorFailure (vehicle base, vtable +194h / +198h / clear)
// ---------------------------------------------------------------------------

// Session messages 7Dh (set) and 7Eh (clear): 009532E0 / 00953360 build them,
// 00953DA0 / 00953EA0 build the same fields on the stack.
struct InferiorFailureMessage {
    std::uint32_t kind{0};            // 7Dh or 7Eh, through 0075B430
    std::uint32_t vtable{0};          // 00D1A57C or 00D1A590
    std::uint32_t field_4{1};
    std::uint16_t word_18{0};
    std::uint8_t byte_1a{0};
    std::uint8_t byte_1c{0};
    float float_20{256.0f};           // [00CF8B3C]
};
InferiorFailureMessage inferior_failure_message_009532e0() noexcept;   // 7Dh
InferiorFailureMessage inferior_failure_message_00953360() noexcept;   // 7Eh
// 00953330 / 009533B0: {7Dh|7Eh, 62h, 49h, 46h}.
bool inferior_failure_message_is_kind_of(std::uint32_t kind, std::int32_t id) noexcept;

// "InferiorFailure" (00CF0B74) under _stricmp (00BF7FBF); a null name is false.
bool is_inferior_failure_name(const char* name) noexcept;
// 00953DA0 `__thiscall(unit, property, value)`, RET 8: on a match it routes the
// 7Dh message through 0077C2A0(msg, 7, 0) and returns true. The value argument
// is never read. `route` receives the message.
bool unit_set_inferior_failure_00953da0(const char* name,
    const std::function<void(const InferiorFailureMessage&, int channel, int flags)>& route);
// 00953EA0, the same with 7Eh.
bool unit_clear_inferior_failure_00953ea0(const char* name,
    const std::function<void(const InferiorFailureMessage&, int channel, int flags)>& route);
// 00953E60 `__thiscall(unit, property, frame, out)`, RET 0Ch: on a match
// *out = unit+720h and true; otherwise false and *out untouched.
bool unit_query_inferior_failure_00953e60(const char* name, std::uint8_t byte_720,
                                          std::uint8_t* out) noexcept;

// ---------------------------------------------------------------------------
// 4. MT_ROLEOWNER device walks (arm 0095AC1D)
// ---------------------------------------------------------------------------

// A unit keeps twelve typed device lists, 12 bytes each from +394h: +394h is
// non-zero when the list is not empty and +398h is the first node; a node's
// next is +4h and its device +8h (00952080 first / 009520E0 next). The slot
// tables are int arrays ending at a value >= 12: 00E0A510 {1,5,6},
// 00E0A520 {7}, 00E0A528 {8,9}.
inline constexpr std::array<std::int32_t, 3> kSlotTable_00e0a510 = {1, 5, 6};
inline constexpr std::array<std::int32_t, 1> kSlotTable_00e0a520 = {7};
inline constexpr std::array<std::int32_t, 2> kSlotTable_00e0a528 = {8, 9};

// One device on the unit's flat device list (+48h, next +44h).
struct RoleDevice {
    bool is_kind_20h{false};          // vtable[5Ch](20h)
    std::int32_t descriptor_kind_80{0};  // [[device+3F4h]+80h]
};
// The message fields the walks read.
struct RoleOwnerMessage {
    std::int32_t slot_28{0};
    std::int32_t take_2c{0};          // 1 take, else release
};
enum class RoleAction : std::uint8_t {
    Release_00729f70,                 // BSP_Gun_ReleasePlayerControl(device)
    Assign_vtable154,                 // device->vtable[154h](0, slot)
    SelectFireMode_0072d5b0,          // BSP_Gun_SelectFireMode(device, 0, 0)
    Activate_vtable1e8,               // device->vtable[1E8h](0)
};
struct RoleStep {
    std::size_t device{0};            // index into the input list
    RoleAction action{RoleAction::Release_00729f70};
    std::int32_t argument{0};         // the slot for Assign
};
// 00953F60: release only (msg+2Ch == 0). A device of kind 1/5/6 is released,
// except a kind-6 device when unit+1BCh equals msg+28h.
std::vector<RoleStep> role_owner_walk_00953f60(const std::vector<RoleDevice>& devices,
    const RoleOwnerMessage& msg, std::int32_t unit_1bc);
// 00954000: devices of kind 2/3/4/6. Take: Assign(slot), then kind 6 also
// SelectFireMode(0, 0). Release: Release.
std::vector<RoleStep> role_owner_walk_00954000(const std::vector<RoleDevice>& devices,
    const RoleOwnerMessage& msg);
// 009540D0 (slot table 00E0A520) and 00954170 (00E0A528): every device on those
// typed lists, in list order. Take: Assign(slot). Release: Assign(8), Activate.
std::vector<RoleStep> role_owner_walk_slots_009540d0(std::size_t device_count,
    const RoleOwnerMessage& msg);

// ---------------------------------------------------------------------------
// 5. Unit state setters
// ---------------------------------------------------------------------------

// 009565A0 `__thiscall(unit+38Ch, level)`, RET 4. Nothing when unit+5Dh (dead)
// is set. Otherwise unit+390h = level, the +6DCh object's vtable[18h](level),
// and vtable[1C4h](level) on every device that answers IsKindOf(20h).
struct SkillApplyPlan {
    bool applied{false};
    std::int32_t level_390{0};
    bool notify_object_6dc{false};
    std::vector<std::size_t> devices;  // indices that get vtable[1C4h](level)
};
SkillApplyPlan unit_skill_apply_level_009565a0(bool dead_5d, std::int32_t level,
    bool has_object_6dc, const std::vector<RoleDevice>& devices);

// 009583C0 / 009584D0 / 009585B0: the pose setters. The plan lists the image's
// order; a host replays it.
struct PoseSetPlan {
    bool broadcast_relocate{false};   // 007EDF00 / 007EDEB0 + 0077C7B0(msg, 0)
    bool call_vtable_114{false};      // 009583C0 only, before the stores
    bool rotate_heading_0047f200{false};  // 009584D0 only
    bool copy_matrix_004134f0{false}; // 009585B0 only, into +74h
    bool set_translation_a4{false};   // 009583C0 only: +A4h..+ACh = position
    bool set_previous_6a4{true};      // +6A4h..+6ACh = the new position
    // Then, always: +C8h = 0, +10Ch = 0, 0042ED50 on every device (+48h list),
    // the +310h sub-object's vtable[0Ch], and vtable[D8h].
};
// Whether the relocate message is sent: not a ship (IsKindOf 6), not a plane
// (IsKindOf 0Fh), world+1FE4h == 1 and the byte world+218Ch == 0.
bool pose_setter_broadcasts(bool is_ship, bool is_plane, std::int32_t world_1fe4,
                            std::uint8_t world_218c) noexcept;
PoseSetPlan unit_set_world_position_009583c0(bool is_ship, bool is_plane,
    std::int32_t world_1fe4, std::uint8_t world_218c) noexcept;
PoseSetPlan unit_set_heading_009584d0(bool is_ship, bool is_plane,
    std::int32_t world_1fe4, std::uint8_t world_218c) noexcept;
PoseSetPlan unit_set_world_matrix_009585b0() noexcept;

// 00954990 `__thiscall(unit, bool flag)`, RET 4: when IsKindOf(6) and the
// +BBCh object exists, its vtable[20h](flag ? 0.0f : 1.0f); then 0072B540(flag)
// on every node of the +428h list (device +8h, next +4h); then +6B4h = flag.
struct AudioFlagPlan {
    bool call_bbc{false};
    float bbc_value{0.0f};
    std::uint8_t byte_6b4{0};
};
AudioFlagPlan unit_broadcast_controlled_audio_flag_00954990(bool is_ship, bool has_bbc,
                                                            bool flag) noexcept;

// 00957450 `__thiscall(unit, child)`, RET 4: ten 18h-byte slots from +53Ch whose
// child pointer is the slot's +14h (+550h). The first empty slot takes the child
// (the 006952A0 detach of a previous child cannot run on an empty slot; 00694A60
// registers the pair only for a non-null child), then +630h = slot and +62Ch = 0,
// also for a null child. With no empty slot nothing is stored. Either way a
// positive +638h is decremented.
struct LaunchedChildSlots {
    std::array<std::uintptr_t, 10> child_550{};
    std::int32_t last_slot_630{0};
    std::uint8_t byte_62c{0};
    std::int32_t pending_638{0};
};
// Returns the slot index taken, or -1.
int unit_add_launched_child_00957450(LaunchedChildSlots& slots, std::uintptr_t child,
    const std::function<void(int slot, std::uintptr_t child)>& register_pair_00694a60);

// 009553D0 `__thiscall(unit, cls)`, RET 4: unit+538h takes `cls` with an
// interlocked reference on cls+4h; the previous class is released with
// InterlockedDecrement and destroyed through vtable[0] when the count reaches 0.
struct RefCountedClass {
    long refs_4{0};
    bool destroyed{false};
};
void unit_set_vehicle_class_009553d0(RefCountedClass*& slot_538, RefCountedClass* cls);

// 00956240 / 00956390 / 009564E0, the Kamikaze / DiveBomber / TorpedoBomber
// class's CreateUnit `__thiscall(cls, a)`, RET 4: operator new(E94h) (00BF55BE),
// memset 0 (00BF79F0), the instance constructor (00951D20 / 00951B60 / 00951C40)
// with `a`, the 009553D0 swap inlined (unit+538h = cls), then unit+354h = cls.
// A failed allocation is not guarded: the inlined swap reads [0+538h].
struct CreatedPlaneUnit {
    std::uint32_t instance_ctor{0};
    RefCountedClass* class_538{nullptr};
    RefCountedClass* class_354{nullptr};
};
std::uint32_t plane_create_unit_instance_ctor(std::uint32_t create_unit) noexcept;
CreatedPlaneUnit plane_class_create_unit(std::uint32_t create_unit, RefCountedClass* cls);

// 00956200 / 00956350 / 009564A0 `__thiscall(unit)`: 0074E610 (the plane base
// unregister), then the first node of one owner list whose payload is the unit
// is erased (004837D0). The owner is [unit+30h]; the list (head = list+4h, next
// +4h, payload +8h) is +12Ch for the Kamikaze, +F0h for the DiveBomber and +E4h
// for the TorpedoBomber. Returns the erased index, or -1.
inline constexpr std::array<std::uint16_t, 3> kPlaneOwnerListOffsets = {0x12C, 0xF0, 0xE4};
int plane_unregister_from_world_lists(std::vector<std::uintptr_t>& owner_list,
                                      std::uintptr_t unit);

// 00953250 `__thiscall(unit, id)`, RET 4: unit+6B8h = id.
inline void unit_store_dummy_object_id_00953250(std::int32_t& field_6b8, std::int32_t id) noexcept {
    field_6b8 = id;
}

// 00951F40 `__thiscall(plane, bool visible)`, RET 4: nothing without the +4A4h
// node. Visible: 00710AD0 on the +360h part then 00B6DA70(+4A4h, 1.0, 0).
// Hidden: 00710B80 then 00B6DA70(+4A4h, 0.0, 0).
struct PlaneVisiblePlan {
    bool acts{false};
    bool attach{false};
    float visibility{0.0f};
};
PlaneVisiblePlan plane_set_visible_00951f40(bool has_node_4a4, bool visible) noexcept;

// ---------------------------------------------------------------------------
// 5b. Submarine periscope break and the frozen-hull step (segment 61)
// ---------------------------------------------------------------------------

// 009373C0 `__thiscall(controller)` and its body twin 009327F0 `__thiscall(unit)`.
// 009373C0 does nothing unless the controller kept a periscope shape at +394h
// (00939E50). It then hides the mast node [unit+1214h] (00B6DA70(0.0, 0)), runs
// the common stores, and calls 00C47FC0(2) on the shape. 009327F0 has only the
// common stores: unit+122Ch = 2, +125Ch = 0.0, +1258h = clock [00F876A4],
// +1254h = f(settings+4C4h + clock).
struct PeriscopeBreakPlan {
    bool acts{false};
    bool hide_mast_node_1214{false};
    std::int32_t state_122c{0};
    float clock_fast_125c{0.0f};
    float clock_1258{0.0f};
    float deadline_1254{0.0f};
    bool shape_collision_c47fc0_2{false};
};
PeriscopeBreakPlan submarine_break_periscope_009373c0(bool has_periscope_shape_394,
                                                      float clock_f876a4,
                                                      float periscope_repair_time_4c4) noexcept;
PeriscopeBreakPlan submarine_break_periscope_009327f0(float clock_f876a4,
                                                      float periscope_repair_time_4c4) noexcept;

// 00937630 `__thiscall(controller, float dt)`, RET 4 (no direct caller; a
// vtable slot). With the unit's byte +1189h clear it is 009329C0(dt), the hydro
// forces. With it set the body's linear and angular velocities are set to zero
// (00C37E50, 00C37E20 on [controller+2Ch]) and 0080E170(0.0) runs on the unit.
struct FrozenForcesPlan {
    bool hydro_forces_009329c0{false};
    bool zero_body_velocities{false};
    bool order_ring_param_0080e170_zero{false};
};
FrozenForcesPlan controller_apply_frozen_forces_00937630(std::uint8_t unit_byte_1189) noexcept;

// ---------------------------------------------------------------------------
// 5c. The vehicle-class registry (00437F50 singleton)
// ---------------------------------------------------------------------------

// The registry keeps a class cache (registry+4h, a vector of reference-counted
// class pointers) and per-class party rows (registry+4010h, 3-byte elements).
struct ClassRegistryModel {
    std::vector<RefCountedClass*> cache_4;
    std::vector<std::array<std::uint8_t, 3>> party_rows_4010;
};
// 0095B9E0 `__thiscall(index)`: grow the cache to index+1 (00437BD0) when
// needed, release the entry (destroy at a zero count) and store null.
void class_registry_release_cached_0095b9e0(ClassRegistryModel& reg, std::int32_t index);
// 0095BA30: both vectors resized to 0 (00437BD0(0), 004359E0(0)).
void class_registry_clear_0095ba30(ClassRegistryModel& reg);
// 0095BAB0 `__thiscall(index)` -> bool: false unless the local player index
// world+18ECh is 0..7; grows the party rows to index+1 (004359E0) and answers
// party_rows[index][player+28h] == 0.
bool class_registry_party_row_clear_0095bab0(ClassRegistryModel& reg, std::int32_t index,
                                             std::int32_t local_player_index,
                                             std::int32_t player_party_28);

// 00965810 `__thiscall(unit)` -> bool: false for null; a plane (IsKindOf 0Fh)
// compares its +9D4h with the current player unit [00E188D8], anything else
// compares itself.
bool unit_is_current_player_unit_00965810(std::uintptr_t unit, bool is_plane,
                                          std::uintptr_t plane_owner_9d4,
                                          std::uintptr_t current_player_unit) noexcept;

// --- class registry loaders (cc13-gprx) ------------------------------------------

// 00964020, no caller, no vtable dword (an unreferenced eager preload), __cdecl(),
// bare RET, AL = 1 when every entry was built, 0 on the first unknown type
// (00964727..00964785). For each pair of the global "VehicleClass" table
// (IterateFirst/Next) it reads value."Type" and picks the row below in this order:
// Destroyer, Cruiser, Cargo, BattleShip are tested with __stricmp (00BF7FBF) on
// the raw string and only when it is non-null (00964142 JE 009642C5); the rest
// with 00425850 (case-insensitive) on the native string. A match allocates the
// object, runs the constructor, sets class+70h = the integer key (00964638) and
// calls class->vtable[8](value) (00964642, the class's own Lua reader), then puts
// it into the registry cache 00437F50()+4h at that key: the cache grows to key+1
// (00437BD0), a previous entry is released (InterlockedDecrement, destroyed
// through vtable[0] at zero) and replaced (00964644..00964689).
// The four ship arms inline 00963380 plus the two vtable stores; the rest call the
// descriptor constructor of kClassDescriptorCtors. "LandFort" builds the bare
// 00749180 base (size 180h), not a descriptor of that table. LandingShip,
// SmallReconPlane, LargeReconPlane and DummyTargetVehicle have no arm, so a
// table holding one stops the preload.
enum class PreloadAlloc : std::uint8_t {
    crt_new_memset,   // 00BF55BE(size) then 00BF79F0(p, 0, size)
    pool_00470b80,    // 00470B80(ECX = size)
};
struct ClassPreloadRow {
    const char* type;           // the "Type" string
    std::uint32_t size;
    PreloadAlloc alloc;
    std::uint32_t ctor;         // the constructor called (00963380 for the inlined ship arms)
    std::uint32_t vtable;       // the inlined vtable store, or 0 when the ctor stores it
    std::uint32_t vtable_6c;
    bool raw_stricmp;           // tested on the raw string before the native compare chain
};
extern const std::array<ClassPreloadRow, 18> kClassPreloadRows;
// The first row whose type matches, in the image's compare order; null when none.
const ClassPreloadRow* class_preload_row_00964020(const char* type) noexcept;
struct ClassPreloadEntry {
    std::int32_t key = 0;       // the integer table key (class+70h, registry index)
    std::string type;
};
struct ClassPreloadResult {
    bool completed = false;     // AL
    std::vector<std::pair<std::int32_t, const ClassPreloadRow*>> built;  // key, row, in order
};
ClassPreloadResult class_preload_00964020(const std::vector<ClassPreloadEntry>& table);

// 009653B0, __fastcall(ECX = index) -> class or null, RET 0, body 009653B0-00965466.
// Callers 00A06840 and 00A072D0. Answers 00964790(index, readRace = 1)
// (0096544C MOV DL,1) when VehicleClass[index] is a table, else null.
std::uintptr_t class_get_or_create_from_lua_index_009653b0(std::int32_t index,
    bool entry_is_table,
    const std::function<std::uintptr_t(std::int32_t, bool)>& get_or_create_00964790);

// 00965480, __fastcall(ECX = NativeString* name) -> class or null, RET 0. Caller
// 00487470. A lookup by name, not a list walk (the earlier ledger text):
// 1. each non-null registry cache entry 00437F50()+4h[0..+8h) in index order whose
//    name +54h matches is returned at once;
// 2. otherwise each pair of the global "VehicleClass" table with an integer key and
//    a table value whose "Name" (00CE8ED0) is a string that matches resolves through
//    00964790(key, readRace = 1) (0096562A MOV DL,1) and is returned;
// 3. else null.
// The match (both phases): a null name data matches a null or empty class name; a
// non-null name data against a null class name matches only when the name's length
// word (+0h) is 0; otherwise __stricmp.
bool class_name_matches_00965480(const char* name_data, std::int32_t name_length,
                                 const char* class_name) noexcept;
struct LuaClassNameEntry {
    bool key_is_integer = false;
    std::int32_t key = 0;
    bool value_is_table = false;
    bool name_is_string = false;
    const char* name = nullptr;
};
struct ClassByNameResult {
    std::uintptr_t cls = 0;
    bool from_cache = false;
    std::int32_t key = -1;      // the table key resolved in phase 2
};
ClassByNameResult class_find_by_name_00965480(const char* name_data, std::int32_t name_length,
    const std::vector<std::pair<std::uintptr_t, const char*>>& cache_with_names,
    const std::vector<LuaClassNameEntry>& vehicle_class_table,
    const std::function<std::uintptr_t(std::int32_t, bool)>& get_or_create_00964790);

// --- section failure effect (cc13-gprx) ------------------------------------------

// 0093AA90, __thiscall(RepairTask* = unit+A20h, uint section, int key_b, bool on),
// RET 0Ch, body 0093AA90-0093AC1A; the 9Bh MT_SHIP_SECTIONFAILUREEFX arm at
// 00822023. The task's slot vector is +8h..+0Ch (one effect pointer per section,
// the vector object at +4h); +0h is the unit.
// - Either way an index at or past the vector end calls 00BF6713 (the CRT invalid
//   parameter report; it does not return).
// - off: a held effect gets 00867B10 (stop its children), its +9h byte = 1, and the
//   slot is released to null (00484620); an empty slot does nothing.
// - on: with the slot empty, 008782A0(unit, section, key_b) finds the damage
//   section (an element of unit+348h with +4h == section and +8h == key_b); when it
//   and its record are non-null, its effect name +24h is set and the unit has a
//   scene node +4A4h, a point effect is created on that node from the record +18h
//   (008689C0, flags 1, 0, 0) and stored in the slot (0080E620, 004845D0).
//   A held effect is kept.
// Presentation: no gameplay state is written.
enum class SectionFailureEffectAction : std::uint8_t {
    invalid_index,   // 00BF6713
    none,
    stop_and_release,
    create,
};
struct SectionFailureEffectInputs {
    bool on = false;
    std::uint32_t section = 0;
    std::size_t slot_count = 0;
    bool slot_held = false;          // slots[section] != 0
    bool section_found = false;      // 008782A0 != 0 and its record != 0
    bool section_effect_name = false;  // record +24h != 0
    bool unit_node_4a4 = false;
};
SectionFailureEffectAction repair_set_section_failure_effect_0093aa90(
    const SectionFailureEffectInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 6. Math
// ---------------------------------------------------------------------------

// 0093EEE0 `__stdcall(x, step)`, RET 8: f(step * ftol(x / step + 0.5)),
// ftol = 00BF7420 (_ftol2_sse, CVTTSD2SI on the double), so it truncates toward 0.
float round_to_step_0093eee0(float x, float step) noexcept;
// 0093EFC0 `__thiscall(a, b)`, RET 4: true when any of the 16 floats compares
// unequal or unordered (FUCOMIP, TEST AH,44h, JP).
bool matrix44_not_equal_0093efc0(const float (&a)[16], const float (&b)[16]) noexcept;
// 00952340 `__fastcall(out, pivot, axis, point, angle)`, RET 0Ch: d = point - pivot
// (float stores), 0085C3F0 rotates d about axis by angle, out = pivot + d.
Vec3 rotate_about_point_00952340(const Vec3& pivot, const Vec3& axis, const Vec3& point,
    float angle, const std::function<Vec3(const Vec3& v, const Vec3& axis, float angle)>&
                     rotate_about_axis_0085c3f0);

}  // namespace gameplay_seg61_62_cc13
}  // namespace bsp
