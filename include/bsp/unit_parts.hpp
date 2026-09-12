#pragma once
// Unit parts: the class-side part descriptors, the damage-state model instance
// at unit+360h, and the per-part detach state that lives on the unit motion
// controller at unit+1018h. docs/UNIT_PARTS.md, reports/unit_parts.json.
//
// Every name here is a hypothesis, not a recovered symbol. Nothing in this file
// is a binary-compatible replacement: the sequences below run over an injected
// host, one virtual per native call site, and the layouts are offset constants
// rather than structs laid over native memory.
//
// Offsets already established by docs/UNIT_HIT_PATH.md (the class part vector,
// unit+344h, unit+360h) live in bsp/unit_hit_path.hpp and are not repeated;
// unit+1018h is kUnitPartsObjectOffset in bsp/unit_damage.hpp and the
// controller's own layout is in bsp/unit_controller.hpp.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_controller.hpp"
#include "bsp/unit_damage.hpp"
#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Per-part state on the controller. 00939CB0 runs
// eh_vector_constructor_iterator(this+8Ch, 20h, 14h, 009318D0, 0092FD50), so
// there are 20 records of 20h bytes at kUnitControllerOffsetRecordArray.
// 00934150 reaches record i with SHL EAX,5 / LEA EDI,[EAX+ESI] (0093417A) and
// then reads [EDI+90h], which is record base + 4h.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitPartRecordStride = 0x20;
inline constexpr int kUnitPartRecordCount = 20; // the 14h count of the ctor iterator

// Two 10h-byte vectors inside one record; each keeps begin at +4h and end at
// +8h, matching the pair 00934150 reads at [EDI+90h]/[EDI+94h] and
// [EDI+A0h]/[EDI+A4h].
inline constexpr std::size_t kUnitPartRecordOffNodeVector = 0x00;   // begin +4h, end +8h
inline constexpr std::size_t kUnitPartRecordOffEffectVector = 0x10; // begin +14h, end +18h
inline constexpr std::size_t kUnitPartRecordVectorBeginDelta = 0x04;
inline constexpr std::size_t kUnitPartRecordVectorEndDelta = 0x08;

// Controller fields this packet adds. The three group vectors have the same
// shape: 10h-byte elements whose inner vector is {begin +4h, end +8h} over
// 14h-byte entries.
inline constexpr std::size_t kUnitPartsOffHullNodeVector = 0x008; // begin +8h, end +Ch (00935367)
inline constexpr std::size_t kUnitPartsOffPartHealth = 0x310;     // begin, end +314h (00935C80)
inline constexpr std::size_t kUnitPartsOffDebrisGroups = 0x320;   // 009343DA on
inline constexpr std::size_t kUnitPartsOffSecondGroups = 0x330;   // 0093919D writes
inline constexpr std::size_t kUnitPartsOffNodeGroups = 0x340;     // 0093438A on
inline constexpr std::size_t kUnitPartsOffGroupSlotBytes = 0x34C; // MOVSX at 00934356
inline constexpr std::size_t kUnitPartsGroupRecordStride = 0x10;  // SHL EBP,4 at 0093836E
inline constexpr std::size_t kUnitPartsGroupEntryStride = 0x14;   // the /14h divides

// The unit fields the detach and the class fields the sub-object rule read.
inline constexpr std::size_t kUnitOffEffectSlotArray = 0xA14;   // [[controller+1Ch]+A14h]
inline constexpr std::size_t kUnitOffPartsDetachedFlag = 0x1174; // set to 1 at 00935529
inline constexpr std::size_t kUnitOffModelRoot = 0x4A4;          // 00938117, the node lookups
inline constexpr std::size_t kEffectOffDebrisByte = 0x09;        // set to 1 at 00934250 and after the spawn
inline constexpr std::size_t kEffectOffRefCount = 0x04;          // InterlockedDecrement at 00934267
inline constexpr std::size_t kSceneNodeOffFlags = 0x5C;          // bit 1 skips the matrix refresh
inline constexpr std::size_t kSceneNodeOffHullWeight = 0xAC;     // compared against 0.0f in the tail

// The class descriptor fields the debris spawn and the sub-object rule read.
inline constexpr std::size_t kVehicleClassOffDebrisEffects = 0x7E8; // begin, end +7ECh
inline constexpr std::size_t kVehicleClassOffSubObjectFlag = 0x174; // byte, gates the owner damage
inline constexpr std::size_t kVehicleClassOffBreakTimerDefault = 0x190; // FLD at 00958AE8
inline constexpr std::size_t kVehicleClassOffBreakTimerKinded = 0x18C; // FLD at 00958B7A
inline constexpr std::size_t kVehicleClassOffSubObjectDamage = 0x194;  // FLD at 00958BB2
inline constexpr std::size_t kGameSettingsOffDebrisEffect = 0x3A8;     // 009348C4

// The 1ACh damage-state instance, as far as 007135C0 writes it.
inline constexpr std::size_t kPartInstanceOffOwnerAlias = 0x04C;
inline constexpr std::size_t kPartInstanceOffPartSet = 0x160;
inline constexpr std::size_t kPartInstanceOffOwner = 0x164;
inline constexpr std::size_t kPartInstanceOffStateVector = 0x16C; // begin, end +170h, cap +174h
inline constexpr std::size_t kPartInstanceOffSubObjectA = 0x17C;  // 007103A0
inline constexpr std::size_t kPartInstanceOffFlagByte = 0x184;
inline constexpr std::size_t kPartInstanceOffSubObjectB = 0x18C;  // 007103C0
inline constexpr std::size_t kPartInstanceOffSubObjectC = 0x198;  // 007103C0
inline constexpr std::size_t kPartInstanceStateStride = 0x10;     // the >> 4 in 0072AB10

// The sub-object fields 00958A30 reads that bsp/unit_damage.hpp does not name.
inline constexpr std::size_t kUnitOffSubObjectBrokenByte = 0x720; // CMP byte [ESI+720h],0 at 00958A6D
inline constexpr std::size_t kUnitOffSubObjectGuardField = 0x358; // tested at the later-break branch

// Values.
inline constexpr int kUnitPartNoGroup = -1;          // FFh written at 0093813A
inline constexpr float kUnitPartLiveThreshold = 0.0f; // 00935C70's COMISS against zero
inline constexpr std::uint8_t kUnitMessageKindDetachPart = 0x99; // 4Bh + index 4Eh
inline constexpr std::uint8_t kUnitMessageKindFirst = 0x4B;      // ADD EAX,-4Bh at 00821EA1
inline constexpr std::uint8_t kUnitMessageKindLast = 0xA0;       // CMP EAX,55h at 00821EA4
inline constexpr std::size_t kUnitMessageOffKind = 0x10;         // MOVZX at 00821E9D
inline constexpr std::size_t kUnitMessageOffPartIndex = 0x20;    // MOV ECX,[ESI+20h] at 00821FF0

// IsKindOf ids the sub-object rule pushes. Their names are unresolved; the ids
// are the literals at 00958B07 (1Bh), 00958B11 (46h) and 00958B40 (45h).
inline constexpr int kUnitKindSubObjectBreakA = 0x46;
inline constexpr int kUnitKindSubObjectBreakB = 0x45;
inline constexpr int kUnitKindSubObjectComposite = 0x1B;

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// 00935C70: a part is detachable while its controller+310h health is above
// zero; detaching stores kUnitPartDetachedHealth (bsp/unit_damage.hpp).
bool unit_part_is_live(float part_health) noexcept;

// 00934356 / 00934366: the signed byte at controller+34Ch+index selects the
// group slot; a negative byte means the model had no nodes for that part and
// the detach stops after releasing the part's nodes and effects.
int unit_part_group_slot(std::int8_t slot_byte) noexcept;
bool unit_part_detach_is_visual_only(int group_slot) noexcept;

// 0093836E: SHL EBP,4 turns the slot into the byte offset of the group record.
std::size_t unit_part_group_record_offset(int group_slot) noexcept;

// 0072AB10: the index of the last damage state of the instance at unit+360h,
// or 0 when the instance or its state vector is absent. state_bytes is
// [inst+170h] - [inst+16Ch].
int unit_last_damage_state_index(bool has_instance, std::size_t state_bytes) noexcept;

// The class ids that select the break timer at 00958AE8..00958B85. The owner's
// class descriptor supplies both floats. The short-circuit is load-bearing:
// vtable[204h] is only consulted for a composite (1Bh) sub-object.
struct SubObjectKindTests {
    bool is_kind_46{false};       // vtable[5Ch](46h)
    bool is_kind_45{false};       // vtable[5Ch](45h)
    bool is_composite{false};     // vtable[5Ch](1Bh)
    bool secondary_46{false};     // vtable[204h](46h), only read when is_composite
    bool secondary_45{false};     // vtable[204h](45h), only read when is_composite
};

struct SubObjectClassFields {
    float break_timer_default{0.0f}; // owner class +190h
    float break_timer_kinded{0.0f};  // owner class +18Ch
    float owner_damage{0.0f};        // owner class +194h
    bool damages_owner{false};       // the sub-object's own class +174h byte
};

bool unit_subobject_kind_selects_timer(const SubObjectKindTests& tests, int class_id) noexcept;
float unit_subobject_break_timer(const SubObjectKindTests& tests,
                                 const SubObjectClassFields& fields) noexcept;
// 00958B8F..00958BC2: the owner takes class+194h only for a composite
// sub-object whose own class byte +174h is set.
bool unit_subobject_damages_owner(const SubObjectKindTests& tests,
                                  const SubObjectClassFields& fields) noexcept;

// The sub-object state 00958A30 branches on.
struct SubObjectHealthState {
    float health{0.0f};      // +370h
    float max_health{0.0f};  // +36Ch
    bool broken{false};      // +720h, non-zero
    float break_timer{0.0f}; // +728h
    bool guard_set{false};   // +358h non-zero
    bool has_owner{false};   // +71Ch non-null
};

enum class SubObjectHealthAction {
    kNone,
    kDestroyRoot,     // 00958DBE, vtable[70h](1)
    kFirstBreak,      // 00958A7A..00958BC4
    kRepeatBreak,     // 00958BD5..00958CD3
    kRepairComplete,  // 00958CD4..00958DA6
};

SubObjectHealthAction unit_subobject_health_action(const SubObjectHealthState& state) noexcept;

// ---------------------------------------------------------------------------
// The detach. One virtual per native call site of 00934150, plus the two
// callers. Handles are native addresses; nothing is dereferenced here.
// ---------------------------------------------------------------------------

struct PartDetachGroupEntry {
    std::uint32_t node{0};          // entry +0h, the only field 00934150 dereferences
    std::uint32_t spec_field_4{0};  // entry +4h, copied into the debris spec at 00934F5x
    std::uint32_t spec_field_8{0};  // entry +8h..+10h, copied at 00934EE0 onwards
    std::uint32_t spec_field_c{0};
    std::uint32_t spec_field_10{0};
};

struct PartDetachRequest {
    int index{0};                 // the part index, 0..19
    int group_slot{kUnitPartNoGroup};
    float impulse[3]{0.0f, 0.0f, 0.0f}; // both native callers pass zero
};

struct UnitPartDetachHost {
    virtual ~UnitPartDetachHost() = default;

    // Step 2. 00B6DFA0 at 009341C1, per node of the record's node vector.
    virtual void release_part_node(std::uint32_t node) = 0;
    // Step 3. 00867B10 at 00934248 and the +9h byte written right after.
    virtual void stop_effect_children(std::uint32_t effect) = 0;
    virtual void mark_effect_orphaned(std::uint32_t effect) = 0;
    // Step 4. The InterlockedDecrement at 00934267 and the vtable[0] destructor.
    virtual void release_effect_ref(std::uint32_t effect) = 0;

    // Steps 7, 15 and 17. 00B6DA70, 00B6E0A0 and the node's vtable[2Ch].
    virtual void set_node_visibility(std::uint32_t node, float factor) = 0;
    virtual void node_world_position(std::uint32_t node, float out[3]) = 0;
    virtual void node_set_position(std::uint32_t node, const float position[3]) = 0;
    // Step 8. The node's vtable[48h], the bounds centre the centroid sums.
    virtual void node_bounds_centre(std::uint32_t node, float out[3]) = 0;
    // Steps 15 and 17. 00B6DB70 at 00934CD8/00935171/009353F8, guarded by
    // node+5Ch bit 1, then 0092E250 and the pair of 00427D10 calls.
    virtual void refresh_world_matrix(std::uint32_t node) = 0;
    virtual void apply_node_transform(std::uint32_t node) = 0;

    // Step 9. 008685E0 at 0093487A and 009348EA. The handle comes from the
    // class list at +7E8h when it is non-empty, otherwise from 00424C40()+3A8h.
    virtual std::uint32_t debris_effect_handle_from_class() = 0;
    virtual std::uint32_t debris_effect_handle_from_settings() = 0;
    virtual std::uint32_t spawn_debris_effect(std::uint32_t handle, const float position[3]) = 0;

    // Steps 10 to 14. The physics library and the game dynamics list; neither
    // is ported, so each is one contract call.
    virtual void physics_begin_debris() = 0;
    virtual std::uint32_t unit_detail_for_lod() = 0;
    virtual void register_debris_node(std::uint32_t node) = 0;
    virtual void add_game_dynamics_body(const PartDetachGroupEntry& entry,
                                        const float velocity[3], const float spin[3],
                                        int part_index, int entry_index) = 0;

    // Step 16 and the tail. 004A5C60 at 00935350, 0085BF20 at 00935444.
    virtual void flush_scene_batch() = 0;
    virtual bool hull_node_is_visible() = 0;

    // Step 18. [unit+1174h] = 1 at 00935529.
    virtual void set_unit_parts_detached_flag() = 0;

    // Enumeration of the native vectors. Reading them is not modelled.
    virtual int part_node_count(int part_index) = 0;
    virtual std::uint32_t part_node(int part_index, int i) = 0;
    virtual int part_effect_slot_count(int part_index) = 0;
    virtual std::uint32_t part_effect(int part_index, int i) = 0;
    virtual void clear_part_vectors(int part_index) = 0;
    virtual int group_entry_count(std::size_t group_vector_offset, int group_slot) = 0;
    virtual PartDetachGroupEntry group_entry(std::size_t group_vector_offset, int group_slot,
                                             int i) = 0;
    virtual int hull_node_count() = 0;
    virtual std::uint32_t hull_node(int i) = 0;
    virtual float hull_node_weight(std::uint32_t node) = 0;
};

// 00934150 as a sequence over the host. The centroid it computes is returned so
// a caller can check it against a trace.
struct PartDetachResult {
    bool released_nodes{false};
    bool spawned_debris{false};
    float centroid[3]{0.0f, 0.0f, 0.0f};
    float lowest_y{0.0f};
    int debris_bodies{0};
};

PartDetachResult unit_parts_detach_part_00934150(UnitPartDetachHost& host,
                                                 const PartDetachRequest& request);

// 00821E80 case 99h -> 0080E440 -> 00934150. The handler applies no test at
// all: the index is the message field and the impulse is zero.
bool unit_message_is_detach_part(std::uint8_t kind) noexcept;
PartDetachRequest unit_detach_request_from_message(std::uint8_t kind, int message_part_index,
                                                   std::int8_t group_slot_byte) noexcept;

} // namespace bsp
