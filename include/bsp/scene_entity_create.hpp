#pragma once
#include <cstdint>

// Creation of a mission entity instance by name: 0046D930 and its spawn-side
// wrapper 004C6BA0. Evidence and the full call-site table are in
// docs/SCENE_ENTITY_CREATE.md.
//
// This is a sequence over an injected host, not a binary-compatible replacement.
// Every native record this path touches (the scene database, the record in the
// map at SceneDatabase+18h, the 0Ch class descriptor, the property bag, the
// entity itself) keeps an opaque pointer because none of those layouts is
// reconstructed. The STL map, the CRT string and pool helpers, the generation
// gate 0046C550, the class creator and the scene-file reader are contracts.
//
// The scene-file Instantiate pass does NOT reach 0046D930. The two paths
// converge one level lower, at the class descriptor's creator: 0046D5A4 on the
// scene-file side, 0046DB4B here. See docs/SCENE_ENTITY_CREATE.md.

namespace bsp {

// ---------------------------------------------------------------------------
// Record and holder offsets
// ---------------------------------------------------------------------------

// Fields of the record the map at SceneDatabase+18h holds, read only where
// 0046D930 touches them. The class-name length at +8h is provisional: it is
// inferred from the {length, data} shape of the other two strings, not read.
inline constexpr std::uint32_t kSceneCreateRecordProperties = 0x04;
inline constexpr std::uint32_t kSceneCreateRecordClassNameData = 0x0c;
inline constexpr std::uint32_t kSceneCreateRecordLocalFrame = 0x14;
inline constexpr std::uint32_t kSceneCreateRecordParentNameLength = 0x54;
inline constexpr std::uint32_t kSceneCreateRecordParentNameData = 0x58;

// The only entity field 0046D930 writes (0046DB9B, 0046DBE0).
inline constexpr std::uint32_t kSceneCreateEntityPropertyBagRef = 0xc0;

// The 12-byte kind-1 property-bag holder built by 00922E20:
// {vtable +0, kind +4, bag +8}. This constructor makes a private bag clone.
// 00922DE0 dispatches on the kind; 00774DC0 constructs kind 2. +4 is not a
// reference count. The historical InitialRefs constant below names kind 1.
inline constexpr std::uint32_t kScenePropertyBagRefSize = 0x0c;
inline constexpr std::uint32_t kScenePropertyBagRefVtable = 0x00d03d94;
inline constexpr std::uint32_t kScenePropertyBagRefInitialRefs = 1;

// operator new size inside the clone 008F41F0.
inline constexpr std::uint32_t kScenePropertyBagInstanceSize = 0x114;

// The empty NativeString data 00468660 returns and 0046DA44 substitutes for a
// null parent name.
inline constexpr std::uint32_t kSceneEmptyStringData = 0x00e18560;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 0046DA5E..0046DB00: the parentFrame 0046C550 receives by value is a literal
// identity matrix, 1.0f (from 00D7A24C) at 0, 5, 10 and 15 and 0.0f elsewhere.
// It is rebuilt on the stack at every call; nothing from the record reaches it.
void scene_create_identity_frame(float out[16]) noexcept;

// 0046DA2D: the record supplies a parent only when its parent-name NativeString
// is non-empty. The length, not the data pointer, is what the branch tests.
bool scene_create_record_has_parent(std::uint32_t parent_name_length) noexcept;

// 0046DA3D..0046DA44: a null data pointer becomes the empty string rather than
// being passed through.
const char* scene_create_parent_name(const char* parent_name_data,
                                     const char* empty_string) noexcept;

// 0046DB55: which arm of the property-bag branch runs.
enum class SceneCreatePropertyArm {
    // 0046DBAF: wrap the record's own bag. 00922E20 clones it internally, so
    // the record's bag is still never handed to the entity.
    RecordBag,
    // 0046DB57: clone the record's bag, merge the overrides into the clone,
    // wrap the clone, then destroy it through its own vtable slot 0.
    RecordBagWithOverrides,
};
SceneCreatePropertyArm scene_create_property_arm(const void* overrides) noexcept;

// 004C6BBE: the wrapper reassigns party player slots only when game+1FE4h is
// non-zero. It does this whether or not the entity was created.
bool scene_spawn_assigns_party_slots(std::uint32_t game_field_1fe4) noexcept;

// Why 0046D930 returned what it did.
enum class SceneCreateOutcome {
    // 0046D9AB: the map node is the head, so no record is keyed by classKey.
    RecordNotFound,
    // 0046DB2E: 0046C550 returned AL == 0.
    GateRejected,
    // 0046DBF3: the creator's entity is returned.
    Created,
};

// ---------------------------------------------------------------------------
// Values crossing the host boundary
// ---------------------------------------------------------------------------

// The three stack arguments of 0046D930, in order. `overrides` may be null;
// the cloud scatter (004BAB12) and the Lua spawn bindings all pass null there.
struct SceneCreateRequest {
    const char* class_key{nullptr};     // +4h, keys the record map at SceneDatabase+18h
    const char* instance_name{nullptr}; // +8h, the created entity's name
    void* overrides{nullptr};           // +Ch, optional per-instance property overrides
};

// The record fields 0046D930 reads, at the offsets above.
struct SceneCreateRecordFields {
    void* properties{nullptr};            // +4
    const char* class_name{nullptr};      // +0Ch
    const float* local_frame{nullptr};    // +14h, 16 floats
    std::uint32_t parent_name_length{0};  // +54h
    const char* parent_name_data{nullptr};// +58h
};

// The 0046C550 argument block assembled at 0046DA6D..0046DB26. `unused_a6` and
// `out_deferred_record` are the two zeros the caller always pushes.
struct SceneCreateGateArgs {
    const char* class_name{nullptr};    // a1, from class_id_to_name
    const char* entity_name{nullptr};   // a2, the request's instance_name
    void* parent{nullptr};              // a3
    const float* local_frame{nullptr};  // a4, record+14h
    void* properties{nullptr};          // a5, record+4
    int unused_a6{0};                   // a6
    float parent_frame[16]{};           // a7..a22, by value
    void* out_deferred_record{nullptr}; // a23, always 0 from this caller
};

// The class creator's arguments at 0046DB4B. ECX carries the class id as data,
// EDX the entity name; it is not __thiscall.
struct SceneCreateCreatorArgs {
    int class_id{0};                   // ECX
    const char* entity_name{nullptr};  // EDX
    void* parent{nullptr};             // first stack argument
    const float* local_frame{nullptr}; // second
    void* properties{nullptr};         // third
    int trailing_zero{0};              // fourth, unread by every creator opened so far
};

struct SceneCreateResult {
    void* entity{nullptr};
    SceneCreateOutcome outcome{SceneCreateOutcome::RecordNotFound};
    // False when operator new(0Ch) returned null, in which case entity+C0h is
    // set to 0 and the entity is still returned (0046DB8F, 0046DBD6).
    bool property_bag_ref_attached{false};
};

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------

// One method per native callee of 0046D930 and 004C6BA0, in call order. There
// are no default implementations: nothing here stands in for unrecovered
// behaviour. Call sites for each method are in the comments and, one row per
// site, in reports/scene_entity_create.json.
struct SceneEntityCreateHost {
    virtual ~SceneEntityCreateHost() = default;

    // 0046D96F FUN_00468CD0 over the map at SceneDatabase+18h, followed by the
    // end check at 0046D9AB and the mapped-pointer read at 0046D9D9. Returns
    // null when the node is the map head. The temporary NativeString the native
    // code builds at 0046D95B and releases at 0046D987/0046D98E is an
    // implementation detail of this method.
    virtual void* find_scene_record(void* scene_database, const char* class_key) = 0;

    // Reads the record fields at the kSceneCreateRecord* offsets.
    virtual SceneCreateRecordFields read_record(void* record) = 0;

    // 0046D9FE BSP_SceneClassMap_FindNode over the map at SceneDatabase+34h,
    // then *(node+8) at 0046DA03: the 0Ch class descriptor. The temporary
    // string at 0046D9E4 / 0046DA21 / 0046DA28 belongs to this method.
    virtual void* find_class_row(void* scene_database, const char* class_name) = 0;

    // *classRow at 0046DB17 and 0046DB3E: the descriptor's class id.
    virtual int read_class_id(void* class_row) = 0;

    // 0046DA55 FUN_00925A90 on *(*(00E188A8)+19CCh): find a child by name.
    virtual void* find_parent_entity(const char* parent_name) = 0;

    // 0046DB1D BSP_SceneDatabase_ClassIdToName on *(00E18680). The routine
    // reloads that global rather than reusing `this`, so the database is passed
    // separately here and the two are not assumed to be the same object.
    virtual const char* class_id_to_name(int class_id) = 0;

    // 0046DB27 BSP_SceneEntity_ShouldGenerate, RET 5Ch. AL == 0 aborts the
    // creation and 0046D930 returns null.
    virtual bool should_generate(void* scene_database, const SceneCreateGateArgs& args) = 0;

    // 0046DB4B, an indirect call through *(class_row+4) -- the same
    // descriptor+4 creator the scene-file reader calls at 0046D5A4. Returns the
    // new entity. Every field of the instance except +C0h is written in here.
    virtual void* run_class_creator(void* class_row, const SceneCreateCreatorArgs& args) = 0;

    // 0046DB5A (and 00922E2D inside the holder) FUN_008F41F0: deep-copy a
    // property bag into a fresh 114h block.
    virtual void* clone_property_bag(void* bag) = 0;

    // 0046DB66 FUN_008F54F0(clone, overrides, 1): merge the override bag in.
    virtual void apply_property_overrides(void* clone, void* overrides) = 0;

    // 0046DB6D / 0046DBB1 operator new(0Ch) then 0046DB88 / 0046DBCF
    // FUN_00922E20. Returns null when the allocation failed, which the native
    // code tests at 0046DB83 and 0046DBC7. The holder clones the bag itself.
    virtual void* make_property_bag_ref(void* bag) = 0;

    // 0046DBAB, an indirect call through the clone's vtable slot 0 with the
    // deleting flag 1: destroys the temporary clone after it has been wrapped.
    virtual void destroy_property_bag(void* bag) = 0;

    // 0046DB9B / 0046DBE0: store the holder at entity+C0h.
    virtual void store_property_bag_ref(void* entity, void* ref) = 0;

    // 0046DBE8 BSP_SEntity_InitAll with CL = 0. Runs on both arms and on the
    // failed-allocation path, but not on the two early returns.
    virtual void init_all_entities(bool flag) = 0;

    // 004C6BB2 / 004BAAFC: *(00E18680), the scene database the wrapper and the
    // cloud scatter both substitute for their own `this`.
    virtual void* scene_database_singleton() = 0;

    // 004C6BBE: *(game+1FE4h).
    virtual std::uint32_t spawn_party_gate_field(void* game) = 0;

    // 004C6BCD BSP_Game_AssignPartyPlayerSlots(game, 0).
    virtual void assign_party_player_slots(void* game, int mode) = 0;
};

// ---------------------------------------------------------------------------
// The two sequences
// ---------------------------------------------------------------------------

// 0046D930. `scene_database` is the ECX argument. Returns the created entity
// with its outcome, or a null entity on either early return.
SceneCreateResult scene_entity_create_0046d930(SceneEntityCreateHost& host,
                                               void* scene_database,
                                               const SceneCreateRequest& request);

// 004C6BA0. Forwards the request to 0046D930 on the scene-database singleton,
// then reassigns party player slots on `game` when game+1FE4h is non-zero. It
// adds nothing to the entity and returns it on both paths, including the null
// a failed creation returns.
SceneCreateResult scene_entity_spawn_004c6ba0(SceneEntityCreateHost& host,
                                              void* game,
                                              const SceneCreateRequest& request);

} // namespace bsp
