#pragma once
#include <cstdint>
#include <vector>

// Entity identity: the two u16 handle tables behind entity+174h, the
// backslash-path entity lookup 009251F0, and the Cloud scene creator's field
// writes. Evidence per claim in docs/ENTITY_IDENTITY.md. Every descriptive name
// here is a hypothesis, not a recovered symbol.
//
// The tables are modelled by slot index, not by pointer: the native stores in a
// node's +4h the *address of the link that points at it*, which makes unlink
// branch-free but is a representation detail, not a layout claim. The offset
// constants below are the layout claim.
namespace bsp {
// Shared null-safe original fastcall comparison rule, with a new source ABI.
int compare_insensitive_00438e10(const char* left, const char* right);

// ---------------------------------------------------------------------------
// Layout (00951660 allocates 10h per slot; 00951560 builds the lists)
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kEntityIdSlotStride = 0x10;   // 009517E3 SHL EDX,0x4
inline constexpr std::uint32_t kEntityIdSlotOffNext = 0x00;  // 009517F6
inline constexpr std::uint32_t kEntityIdSlotOffPrev = 0x04;  // 009517EA
inline constexpr std::uint32_t kEntityIdSlotOffId = 0x08;    // 009517D7, a u16
inline constexpr std::uint32_t kEntityIdSlotOffEntity = 0x0C;  // 009517F2

inline constexpr std::uint32_t kEntityIdTableOffFirstId = 0x04;    // 00951660 param_1[1]
inline constexpr std::uint32_t kEntityIdTableOffIdCount = 0x08;    // 00951660 param_1[2]
inline constexpr std::uint32_t kEntityIdTableOffFreeList = 0x0C;   // 00951560
inline constexpr std::uint32_t kEntityIdTableOffLiveList = 0x2C;   // 00951560
inline constexpr std::uint32_t kEntityIdTableOffSlots = 0x4C;      // 00951660 param_1[0x13]
inline constexpr std::uint32_t kEntityIdTableOffFreeCount = 0x50;  // 00951660 param_1[0x14]
inline constexpr std::uint32_t kEntityIdTableSize = 0x54;          // 00927970 LEA ECX,[ESI+0x54]

// A list is an embedded 10h terminator node plus the head pointer at +10h.
inline constexpr std::uint32_t kEntityIdListOffTail = 0x04;  // terminator's prev
inline constexpr std::uint32_t kEntityIdListOffHead = 0x10;  // 00951560 *(this+1Ch), *(this+3Ch)
inline constexpr std::uint32_t kEntityIdListSize = 0x20;

// The two tables are one object: A at 00F89A08 with ids [0, A.count), B at
// 00F89A5C = 00F89A08+54h with ids [A.count, A.count+B.count). 00927940 passes
// A's count as B's first id, which is what lets every reader select with a
// single `id < A.count` test (00521E33, 009287F5).
inline constexpr std::uint32_t kEntityIdNoSlot = 0xFFFFFFFFu;

enum class EntityIdTableKind {
    Primary,    // 00F89A08, the low range
    Alternate,  // 00F89A5C, the high range
};

struct EntityIdSlot {
    std::uint16_t id{0};            // +8h, first_id + index; slot 0 is stored as 0
    const void* entity{nullptr};    // +0Ch, null while free
    std::uint32_t next{kEntityIdNoSlot};  // +0h, toward the list terminator
    std::uint32_t prev{kEntityIdNoSlot};  // +4h, toward the list head
    bool in_live_list{false};       // modelling only: the native derives it from +4h
};

struct EntityIdList {
    std::uint32_t head{kEntityIdNoSlot};  // list+10h
    std::uint32_t tail{kEntityIdNoSlot};  // list+4h, the terminator's prev
};

struct EntityIdTable {
    std::uint16_t first_id{0};   // +4h
    std::uint32_t id_count{0};   // +8h
    std::vector<EntityIdSlot> slots;  // +4Ch, operator_new(id_count * 10h)
    EntityIdList free_list;      // +0Ch
    EntityIdList live_list;      // +2Ch
    std::int32_t free_count{0};  // +50h
};

// ---------------------------------------------------------------------------
// The allocation, release and resolve rules
// ---------------------------------------------------------------------------

// 00951660: first id, slot count, free count = count - 1, then 00951560.
EntityIdTable entity_id_table_construct_00951660(std::uint16_t first_id, std::uint32_t id_count);

// 00951560: slot i carries the id first_id + i (slot 0 carries 0 and stays out
// of every list), the free list runs from slot[count-1] down to slot[1], and the
// live list is empty. Called again per mission at 004E01E3 and 004E3F22.
void entity_id_table_reset_00951560(EntityIdTable& table);

struct EntityIdSweepResult {
    std::uint32_t reclaimed{0};          // slots pushed on the free list
    std::uint32_t out_of_range_skipped{0};  // see below
};

// 00951720, the exhaustion path of the allocator: for i = count-1 down to 1,
// a slot whose entity pointer is null is relinked onto the free list.
//
// The native tests slot i (00951737, EDI = i*10h) but relinks slot i - first_id
// (00951741, SUB EAX,[ECX+4]). The two agree only for the table whose first id
// is 0. For the high table the native relinks the wrong slot, and below
// i == first_id it addresses memory before the array; this projection counts
// those iterations in out_of_range_skipped instead of reproducing the fault.
EntityIdSweepResult entity_id_table_sweep_00951720(EntityIdTable& table);

// 009517C0 __thiscall(table, uint16 requestedId, void* entity) -> uint16, RET 8.
// requested_id == 0 takes the free list's tail (the lowest never-used id first,
// then released ids in FIFO order); a non-zero requested_id takes that exact
// slot, unchecked in the native. Returns 0 where the native would address
// outside the array; 0 is the reserved id, so no caller can confuse it with a
// success.
std::uint16_t entity_id_table_allocate_009517c0(EntityIdTable& table,
                                                std::uint16_t requested_id,
                                                const void* entity);

// 009516D0 __thiscall(table, uint16 id), RET 4: clear the payload, unlink, push
// at the head of the free list, free_count += 1. Called from the entity
// destructor 009287B0 at 00928810.
void entity_id_table_release_009516d0(EntityIdTable& table, std::uint16_t id);

// The slot payload for an id, i.e. the read half of 00521E30.
const void* entity_id_table_lookup(const EntityIdTable& table, std::uint16_t id);

struct EntityHandleTablePair {
    EntityIdTable primary;    // 00F89A08
    EntityIdTable alternate;  // 00F89A5C
};

// 00927940: 00951660(this, 0, low_count) then 00951660(this+54h, low_count,
// high_count). The capacities are the caller's arguments and no caller of
// 00927940 exists in the image, so no capacity is claimed here.
EntityHandleTablePair entity_handle_table_construct_00927940(std::uint32_t low_count,
                                                             std::uint32_t high_count);

// 00521E30 BSP_EntityHandleTable_Resolve, __fastcall(uint16) -> entity or null.
const void* entity_handle_table_resolve_00521e30(const EntityHandleTablePair& pair,
                                                 std::uint16_t id);

// 009287EB..0092880B, the destructor's table choice: id < primary.id_count.
EntityIdTableKind entity_handle_table_kind_for_id_009287eb(const EntityHandleTablePair& pair,
                                                           std::uint16_t id);

// 009286B8 CMP byte ptr [ESP+0x28],BL with BL = 0 (00928660): the construct
// flag selects the alternate table when it is zero. Scene creators pass 0
// (004E9542), BSP_UnitOwnerEntity_Construct passes 1 (0077EEF9).
EntityIdTableKind entity_construct_table_kind_009286b8(int construct_flag);

// ---------------------------------------------------------------------------
// 009251F0, the backslash-path entity lookup
// ---------------------------------------------------------------------------

// component '\' component ... '\' leaf. Each component must equal a node name in
// full and case-insensitively; the first component names the node the search
// starts from, which for BSP_EntityRegistry_FindEntityByName 00925A90 is a party.
inline constexpr char kEntityQualifiedNameSeparator = '\\';  // 00925237 PUSH 0x5C

// The node interface 009251F0 uses. vtable+10h is the name getter; the child
// list is node+48h with its count at node+50h, walked by 00467F30.
struct EntityNameNode {
    virtual ~EntityNameNode() = default;
    virtual const char* node_name() const = 0;                    // vtable +10h
    virtual int child_count() const = 0;                          // node +50h
    virtual const EntityNameNode* child_at(int index) const = 0;  // 00467F30
};

// 009251F0 __stdcall(node, name) -> node or null, RET 8. The incoming ECX is
// stored at 009251FB and never read; it carries nothing.
const EntityNameNode* entity_find_by_qualified_name_009251f0(const EntityNameNode& node,
                                                             const char* name);

// ---------------------------------------------------------------------------
// 004E9E40 / 004E9920, the Cloud creators
// ---------------------------------------------------------------------------

inline constexpr const char* kSceneCloudTypePropertyKey = "CloudType";  // 00CE5FF4
inline constexpr int kScenePropertyTagStringA = 4;  // 004E9E77 CMP EAX,4
inline constexpr int kScenePropertyTagStringB = 2;  // 004E9E7C CMP EAX,2

// One virtual per native call site of 004E9E40 / 004E9920. Nothing here has a
// default: no method stands in for unrecovered game behaviour.
struct SceneCloudCreatorHost {
    virtual ~SceneCloudCreatorHost() = default;
    // 008F2260(bag, key) -> property record or null.
    virtual const void* find_property(const void* property_bag, const char* key) = 0;
    // [record+4h], the property's type tag.
    virtual int property_type_tag(const void* property) = 0;
    // 008F0DF0(record) -> the value as a C string.
    virtual const char* property_string_value(const void* property) = 0;
    // 0041E870 builds a temporary string, 00479970 resolves the class and falls
    // back to the Lua global table CloudClass on a miss. The temporary-string
    // plumbing (00419CC0 / 00BD1510 release) is a contract, not projected.
    virtual const void* acquire_entity_class(const char* type_name) = 0;
    // 0047BBA0 on [[00E188A8]+21D0h], a forwarder to that manager's vtable[8].
    virtual void* create_entity_from_class(const void* entity_class) = 0;
    // entity->vtable[98h](parent, entityRegistry, frameBlock) at 004E9F4B;
    // entityRegistry is [[00E188A8]+19CCh], the registry 009251F0 searches.
    virtual void attach_entity_to_scene(void* entity, void* parent, void* entity_registry,
                                        void* frame_block) = 0;
    // 0048D6C0(entity, name): resize the string at entity+154h and memcpy into
    // entity+158h. See kUnitInstanceNameLengthOffset in scene_unit_creators.hpp.
    virtual void set_entity_name(void* entity, const char* name) = 0;
};

// The creator ABI, confirmed in the body: ECX arrives holding the class id and
// is dead (004E9E46 reloads it from the third stack argument, the property bag),
// EDX is the entity name, and the stack carries parent, &frameBlock, properties
// and an unread 0 (RET 0x10 at 004E9F67).
struct SceneCloudCreatorArgs {
    const void* property_bag{nullptr};  // stack argument 3, and the reloaded ECX
    const char* entity_name{nullptr};   // EDX
    void* parent{nullptr};              // stack argument 1
    void* frame_block{nullptr};         // stack argument 2
    void* entity_registry{nullptr};     // [[00E188A8]+19CCh]
};

// 004E9E40: read CloudType, gate on the tag, resolve the class twice (the first
// result is discarded natively), create, attach, name. Returns null on the gate.
// No store to entity+174h occurs here: the id is written by 00928630 at
// 009286ED inside the constructor the manager's vtable[8] reaches.
void* scene_cloud_instantiate_004e9e40(SceneCloudCreatorHost& host,
                                       const SceneCloudCreatorArgs& args);

// 004E9920 __thiscall(bag), RET 0: the same read and gate, one resolve whose
// result is discarded. It creates nothing; it forces the class to load.
bool scene_cloud_register_004e9920(SceneCloudCreatorHost& host, const void* property_bag);

} // namespace bsp
