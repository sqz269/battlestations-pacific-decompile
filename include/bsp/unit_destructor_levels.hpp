// The unit's own destructor levels above GameEntity: the four class bodies between
// the concrete unit class and 009287B0, as one ordered release sequence.
//
// Packet cc2_unit_destructor_levels, worktree agent/cc2-unit-destructor-levels.
// Ghidra was read-only for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/UNIT_DESTRUCTOR_LEVELS.md carries the evidence and
// reports/unit_destructor_levels.json the call rows and the flow gaps.
//
// The chain, each level found from slot 0 of its own +0h vtable and confirmed by the
// vptr set it rewrites (docs/UNIT_INSTANCE_LAYOUT.md has the constructor side):
//
//   level 6  MDestroyer          no destructor body; 006FE570 calls the level-5 body
//   level 5  0081F3A0            00D09678, eight vptrs, body 0081F3A0-0081F8AD
//   level 4  00959940            00D1A698, seven vptrs
//   level 3  0087A410            00D0DF70, six vptrs
//   level 2  0077E380            00D03E80, five vptrs, body 0077E380-0077E490
//   level 1  009287B0            bsp/entity_lifecycle_tails.hpp
//   level 0  00925780            bsp/entity_lifecycle_tails.hpp
//
// No offset constants are declared here on purpose. The fields this sequence touches
// already have canonical names in other headers (kUnitOffEffectHandles 0x758 and
// kUnitOffController 0x1018 in bsp/unit_instance.hpp, kUnitOffGunneryPass 0x6DC,
// kUnitOffTickElement 0x310 and kUnitOffCategoryRecords 0x394 in
// bsp/unit_gunnery_pass.hpp, kUnitOffWeaponDirector 0x738 in bsp/unit_weapons.hpp,
// kUnitOffLeakModel 0x10D4 in bsp/unit_forces.hpp, kUnitOffAllGunsRecord 0x424 in
// bsp/gunnery_tables.hpp and others). Each step below names its offset in a comment
// so no second, possibly conflicting, declaration of the same field is added.
//
// Not modelled: the bodies of 00932840, 0093B920, 0081EC00, 004C9550, 006E0860,
// 0077D930, 0077B980, 004B7EF0 and 00955EB0, read only far enough to establish the
// entry contract; the concrete targets of the four virtual slots the chain calls.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// The levels, by the depth docs/UNIT_INSTANCE_LAYOUT.md's +C4h class id uses.
// ---------------------------------------------------------------------------
enum class UnitDestructorLevel : std::uint8_t {
    most_derived = 6,        // no body; 006FE570 and 00822700 both call level 5
    unit_vehicle_base = 5,   // 0081F3A0
    unit_game_object = 4,    // 00959940
    unit_tickable = 3,       // 0087A410
    unit_owner_entity = 2,   // 0077E380
    game_entity = 1,         // 009287B0, bsp/entity_lifecycle_tails.hpp
    entity_root = 0,         // 00925780, bsp/entity_lifecycle_tails.hpp
};

// ---------------------------------------------------------------------------
// One enumerator per step of the rule tables in docs/UNIT_DESTRUCTOR_LEVELS.md,
// in native execution order. The comment is the call or store instruction.
// ---------------------------------------------------------------------------
enum class UnitReleaseStep : std::uint8_t {
    // Level 5, 0081F3A0
    l5_rewrite_vptrs,                    // 0081F3C2..0081F408, the 00D096xx set
    l5_stop_and_release_point_effects,   // 0081F43A per element of +758h, count +75Ch
    l5_release_motion_physics_proxy,     // 0081F488 -> 0092CFD0, unit+1018h
    l5_destroy_motion_controller,        // 0081F499 -> 00932840
    l5_free_motion_controller,           // 0081F49F
    l5_free_field_73c,                   // 0081F4B2
    l5_destroy_field_740,                // 0081F4CF, vtable[10h]
    l5_free_field_117c,                  // 0081F4E2, then +117Ch/+1180h/+1184h = 0
    l5_release_ref_range_1114,           // 0081F517 -> 0081C940 over [+1118h, +111Ch)
    l5_free_ref_range_buffer_1114,       // 0081F520, then +1118h/+111Ch/+1120h = 0
    l5_destroy_subobject_10d4,           // 0081F53C -> 0074E7F0
    l5_destroy_subobject_10a0,           // 0081F54C -> 0081EC00
    l5_release_live_effect_refs_ffc,     // 0081F55F -> 004C9550, then free at 0081F567
    l5_rewrite_subobject_vptr_bd0,       // 0081F56C, 00D09480
    l5_leave_critical_section_bd4,        // 0081F595, once per held recursion
    l5_delete_critical_section_bd4,       // 0081F59D
    l5_free_critical_section_bd4,         // 0081F5A4, then +BD4h = 0
    l5_release_ref_slots_ba4_to_bc0,      // 0081F5CB..0081F6EA, eight slots
    l5_destroy_ref_slot_array_b54,        // 0081F705, five elements, dtor 00440A30
    l5_destroy_ref_slot_array_b44,        // 0081F71F, four elements
    l5_destroy_repair_task_a20,           // 0081F72F -> 0093B920
    l5_release_live_effect_refs_a14,      // 0081F742 -> 004C9550, then free at 0081F74A
    l5_destroy_ref_slot_array_a00,        // 0081F767, four elements
    l5_release_ref_slots_9e8_to_9f4,      // 0081F77F..0081F802, four slots
    l5_free_point_effect_array_758,       // 0081F85F, after a release pass step 2 emptied
    l5_free_field_74c,                    // 0081F874
    l5_reset_owned_ref_slot_72c,          // 0081F887 -> 00809650, destroys +734h

    // Level 4, 00959940
    l4_rewrite_vptrs,                     // 0095995F..00959991, the 00D1A6xx set
    l4_release_field_538,                 // 009599B3
    l4_clear_current_object_global,        // 009599DF -> 004BCA80, [00E188DC] = 0
    l4_release_field_4a4,                  // 009599F2
    l4_destroy_field_6f4,                  // 00959A1B, vtable[4h](1)
    l4_destroy_field_724,                  // 00959A33, vtable[0h](1)
    l4_drain_list_424,                     // 00959A7E per node, back-edge 00959A8C
    l4_drain_gun_category_lists_394,       // 00959AD1 per node, twelve records
    l4_return_pooled_string_718,           // 00959B02 then 00959B09
    l4_return_pooled_string_6d0,           // 00959B2A then 00959B31
    l4_release_field_670,                  // 00959B49
    l4_free_field_664,                     // 00959B6C, then +664h/+668h/+66Ch = 0
    l4_destroy_record_array_53c,           // 00959B9B, ten 0x18-byte records
    l4_clear_gun_category_list_424,        // 00959BA6 -> 00955EB0
    l4_destroy_gun_category_array_394,     // 00959BBF, twelve 0Ch-byte records

    // Level 3, 0087A410
    l3_rewrite_vptrs,                      // 0087A435..0087A45D, the 00D0DFxx set
    l3_free_vector_380,                    // 0087A474, then +380h/+384h/+388h = 0
    l3_free_parts_descriptor_vector_348,   // 0087A499, then +348h/+34Ch/+350h = 0
    l3_destroy_tick_element_310,           // 0087A4B5 -> 00875490 -> 00874F00

    // Level 2, 0077E380
    l2_rewrite_vptrs,                      // 0077E3A0..0077E3BE, the 00D03Exx set
    l2_send_release_session_message,       // 0077E3F3 then 0077E404, payload entity+70h
    l2_erase_from_global_container,        // 0077E417 -> 0077BEA0, ECX = 00F87194
    l2_destroy_subobject_2b0,              // 0077E427 -> 006E0860
    l2_clear_list_2a4,                     // 0077E434 -> 0077D930
    l2_free_list_buffer_2a8,               // 0077E43D, then +2A8h = 0 at 0077E44B
    l2_clear_list_298,                     // 0077E452 -> 0077B980
    l2_destroy_recon_detection_records_1e8,// 0077E46C, three 0x34-byte records

    // The delegation out of this packet and the instance free.
    delegate_to_game_entity,               // 0077E47B -> 009287B0
    l6_free_instance,                      // 006FE580 / 00822710, gated on flags & 1
};

// ---------------------------------------------------------------------------
// The state the four levels branch on. Every field names the instruction that
// reads it. A field the native code does not test is not represented.
// ---------------------------------------------------------------------------
struct UnitReleaseState {
    // Level 5
    std::uint32_t point_effect_count{0};   // 0081F412 / 0081F47A, [unit+75Ch]
    bool has_point_effect_array{false};    // 0081F808, [unit+758h] != 0
    bool has_motion_controller{false};     // 0081F493, [unit+1018h] != 0
    bool has_field_73c{false};             // 0081F4AD
    bool has_field_740{false};             // 0081F4C6
    bool has_field_117c{false};            // 0081F4DD
    bool has_ref_range_1114{false};        // 0081F505, [unit+1118h] != 0
    bool has_critical_section_bd4{false};  // 0081F57F, [unit+BD4h] != 0
    std::int32_t critical_section_enter_count{0};  // 0081F583, [cs+18h]
    std::uint8_t live_ref_slots_ba4_to_bc0{0};     // bit per slot, 0081F5C5.. tests
    std::uint8_t live_ref_slots_9e8_to_9f4{0};     // bit per slot, 0081F779.. tests
    bool has_field_74c{false};             // 0081F86F
    bool has_owned_ref_734{false};         // 0080965C, inside 00809650

    // Level 4
    bool has_field_538{false};             // 009599A3
    bool field_4a4_is_current_global{false};  // 009599D5, CMP [00E188DC],EAX
    bool has_field_4a4{false};             // 009599EA
    bool has_field_6f4{false};             // 00959A10
    bool has_field_724{false};             // 00959A29
    std::uint32_t list_424_nodes{0};       // 00959A3B / 00959A86
    std::array<std::uint32_t, 12> gun_category_nodes{};  // 00959AA0 / 00959AD9
    bool has_pooled_string_718{false};     // 00959AEB
    bool has_pooled_string_6d0{false};     // 00959B14
    bool has_field_670{false};             // 00959B3C
    bool has_field_664{false};             // 00959B67

    // Level 3
    bool has_vector_380{false};            // 0087A46B
    bool has_parts_descriptor_vector{false};  // 0087A494, [unit+348h] != 0

    // Level 2
    bool session_message_gate{false};      // 0077E3C8..0077E3E9, all three tests
};

// The ordered steps the release performs for `state`, levels 5 down to 2 plus the
// delegation and the instance free. Pure: no host, no storage. Loop steps appear
// once per iteration, so a two-effect unit emits l5_stop_and_release_point_effects
// twice. `free_instance` is emitted only when `free_flag` is set, which is the
// scalar deleting destructor's `flags & 1` at 006FE578.
std::vector<UnitReleaseStep> unit_release_steps_0081f3a0(const UnitReleaseState& state,
                                                         bool free_flag);

// The levels a release runs through, most derived first. Always the same seven
// entries, with level 6 present but contributing no body; returned so a caller can
// assert the chain rather than hard-code it.
std::vector<UnitDestructorLevel> unit_destructor_chain_006fe570();

// ---------------------------------------------------------------------------
// 0081F583..0081F59A: the recursive critical section at unit+BD4h.
// The object is { CRITICAL_SECTION cs; std::int32_t enter_count; } - 00402F50
// writes 0 to +18h right after InitializeCriticalSection, which is what fixes the
// layout. The teardown leaves the section once per held recursion, then deletes it.
// The loop is a do/while entered only when the count is already positive, so a
// negative count leaves nothing.
// ---------------------------------------------------------------------------
std::int32_t recursive_critical_section_leave_count_0081f583(std::int32_t enter_count) noexcept;

// ---------------------------------------------------------------------------
// 004BB440 BSP_Game_ClaimParticipantRecord, the producer of the player+8h gate byte
// docs/ENTITY_LIFECYCLE_TAILS.md section 5 left unread. Eight 0x118-byte records at
// session+748h; the scan takes the first whose +8h byte is 0 and writes 1 into it
// (004BB47D). Returns the claimed index, or -1 when all eight are taken, which is
// the EAX = 0 the native returns from 004BB464.
// ---------------------------------------------------------------------------
int claim_participant_record_004bb440(std::array<std::uint8_t, 8>& claimed_bytes) noexcept;

// 008CDF58 and 008CDF5E, the gate the six readers share: an explicit slot is
// accepted only when +8h is non-zero and +9h is zero. +9h has no writer anywhere in
// the image (see the doc), so in the shipped build this reduces to "+8h is set".
bool participant_slot_accepted_008cdf58(std::uint8_t byte8, std::uint8_t byte9) noexcept;

// ---------------------------------------------------------------------------
// The release as a sequence over an injected host: one method per native call site.
// Nothing here stands in for unrecovered behaviour; a step the host cannot perform
// is simply not called.
// ---------------------------------------------------------------------------
class UnitReleaseHost {
  public:
    virtual ~UnitReleaseHost() = default;

    // Each level's own vptr set, in the order the stores appear.
    virtual void rewrite_vptrs(UnitDestructorLevel level) = 0;

    // Level 5.
    // 0081F43A -> 00867B10, then the +9h flag store at 0081F43F, then the
    // InterlockedDecrement release at 0081F454 and the two null stores.
    virtual void stop_and_release_point_effect(std::uint32_t index) = 0;
    virtual void release_motion_physics_proxy() = 0;   // 0081F488 -> 0092CFD0
    virtual void destroy_motion_controller() = 0;      // 0081F499 -> 00932840
    virtual void free_motion_controller() = 0;         // 0081F49F
    virtual void free_field(std::size_t field_offset) = 0;  // the plain free steps
    virtual void destroy_through_vtable(std::size_t field_offset, std::size_t slot) = 0;
    virtual void release_ref_range_1114() = 0;         // 0081F517 -> 0081C940
    virtual void destroy_subobject(std::size_t field_offset) = 0;  // 0074E7F0, 0081EC00, 006E0860
    virtual void release_live_effect_refs(std::size_t field_offset) = 0;  // 004C9550
    virtual void leave_critical_section_bd4() = 0;     // 0081F595
    virtual void delete_critical_section_bd4() = 0;    // 0081F59D
    virtual void release_ref_slot(std::size_t field_offset) = 0;  // the +BC0h.. and +9F4h.. slots
    virtual void destroy_ref_slot_array(std::size_t field_offset, std::uint32_t count) = 0;
    virtual void destroy_repair_task_a20() = 0;        // 0081F72F -> 0093B920
    virtual void reset_owned_ref_slot_72c() = 0;       // 0081F887 -> 00809650

    // Level 4.
    virtual void clear_current_object_global() = 0;    // 009599DF -> 004BCA80
    virtual void drain_list_node(std::size_t head_offset) = 0;  // 00959A7E, 00959AD1
    virtual void return_pooled_string(std::size_t field_offset) = 0;  // 00419CC0 then 00BD1510
    virtual void destroy_record_array(std::size_t field_offset, std::uint32_t count,
                                      std::size_t element_size) = 0;  // 00BF7C6E
    virtual void clear_gun_category_list(std::size_t field_offset) = 0;  // 00955EB0

    // Level 3.
    virtual void destroy_tick_element_310() = 0;       // 0087A4B5 -> 00875490 -> 00874F00

    // Level 2.
    // 0077E3F3 -> 00779780 with the payload read at 0077E3EB, then 0077E404 ->
    // BSP_Session_RouteMessage 0077C2A0 with the extra arguments 4 and 0.
    virtual void send_release_session_message(std::uint32_t dead_meat_mark) = 0;
    virtual void erase_from_global_container() = 0;    // 0077E417 -> 0077BEA0
    virtual void clear_list(std::size_t field_offset) = 0;  // 0077D930, 0077B980

    // Out of this packet.
    virtual void delegate_to_game_entity() = 0;        // 0077E47B -> 009287B0
    virtual void free_instance() = 0;                  // 006FE580 / 00822710
};

// Runs unit_release_steps_0081f3a0 against `host`. `dead_meat_mark` is the value at
// entity+70h, read at 0077E3EB and forwarded only when state.session_message_gate.
void run_unit_release_0081f3a0(UnitReleaseHost& host, const UnitReleaseState& state,
                               bool free_flag, std::uint32_t dead_meat_mark);

}  // namespace bsp
