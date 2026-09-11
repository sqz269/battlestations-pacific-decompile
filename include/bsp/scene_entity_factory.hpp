#pragma once
// The scene entity class table and the per-entity generation gate.
// Addresses: 0046c550, 004f2800, 004ee250, 004ee1a0, 00468660, 00469690,
//            00468fb0, 0046bf20, 0046bf70, 0046be90, 0046c450, 004693c0.
//
// Every name here is a hypothesis, not a recovered symbol. docs/SCENE_ENTITY_FACTORY.md
// holds the evidence for each claim, including the disassembly the class table was
// decoded from. The per-class bodies (the 26 creator routines) are out of scope: this
// header records the id, the creator address, the instance size and how the creator
// obtains its instance, not what each class then does.
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/scene_file.hpp"

namespace bsp {

class PoseRefreshResolver;

// ---------------------------------------------------------------------------
// The class table (004F2800)
// ---------------------------------------------------------------------------

// 004F2800 runs 26 calls to 004EE250, each of the shape
//     SceneClassMap::Register(name, classId, create, postCreate)
// with ECX = *(00E18680) (the scene database) and the map at this+34h. 004EE250
// allocates a 0Ch-byte descriptor {classId, create, postCreate} and 004EE1A0
// links it into the 40h-bucket chained hash map under a copy of the name.
//
// 0046CF40 resolves an entity's class token through 00468FB0 and reads the
// descriptor out of node+8: descriptor[0] is the class id, descriptor[1] is
// called on the instantiate pass and descriptor[2] on both passes.
enum class SceneEntityCreatorKind {
    // The creator calls operator new(size) + memset 0 and constructs a fixed
    // engine object. `instance_size` is that size.
    FixedInstance,
    // The creator reads the `Type` property and hands it to the unit-class
    // factory at 00964790, then makes the instance through a virtual at +28h of
    // the returned class object. There is no fixed instance size.
    UnitClassFactory,
    // The creator reads its own type key out of the property bag and builds the
    // instance from that record without the unit-class factory.
    TypedResource,
};

struct SceneEntityClassRow {
    int class_id;                    // descriptor +0, the id the reader compares
    const char* name;                // the map key, case-insensitive
    std::uint32_t create_address;    // descriptor +4, run on the instantiate pass
    std::uint32_t register_address;  // descriptor +8, run on both passes
    std::uint32_t instance_size;     // operator new size, 0 when the creator does not allocate
    SceneEntityCreatorKind creator_kind;
    const char* type_key;            // the bag key the creator reads, or nullptr
};

inline constexpr int kSceneEntityClassCount = 26;

// The 26 rows in 004F2800's registration order.
const SceneEntityClassRow* scene_entity_class_table() noexcept;

const SceneEntityClassRow* find_scene_entity_class_by_id(int class_id) noexcept;
// Case-insensitive, matching 00438E10 inside 00468FB0. This is why the three
// `Landfort` entities in the shipped files resolve to `LandFort`.
const SceneEntityClassRow* find_scene_entity_class_by_name(const std::string& name) noexcept;

// 00469690: name -> id. Returns kSceneUnknownClassId when the name is not
// registered; the native returns 0 there, which is not a registered id either.
inline constexpr int kSceneUnknownClassId = -1;
int scene_entity_class_id_from_name(const std::string& name) noexcept;

// 00468660: id -> name. Returns nullptr when the id is not registered; the
// native returns 0 in that case, and returns the global at 00E18560 when a
// registered row has a null name pointer (004EE250 never produces one).
const char* scene_entity_class_name_from_id(int class_id) noexcept;

// 0046CF40 reads the descriptor as `*(node+8)` with no null check, so a class
// token that is not registered dereferences address 8 and faults. There is no
// unknown-class recovery in the native reader; this predicate is what a caller
// has to test before dispatching.
bool scene_entity_class_is_registered(const std::string& name) noexcept;

// ---------------------------------------------------------------------------
// The registration pass filter (0046D441..0046D57C inside 0046CF40)
// ---------------------------------------------------------------------------

// On the registration pass the reader keeps only entities whose class id is one
// of these six, plus the 4Dh/44h fallback below. 19h is not a registered class
// id, so that one comparison can never match.
inline constexpr int kSceneRegistrationPassClassIds[6] = {0x47, 0x19, 0x1b, 0x1c, 0x34, 0x4d};
bool scene_registration_pass_handles_class(int class_id) noexcept;

// The fallback taken when the `Type` property is absent or the six ids above
// did not match: SpawnPoint and Landscape still run descriptor[2].
inline constexpr int kSceneRegistrationFallbackClassIds[2] = {0x4d, 0x44};
bool scene_registration_fallback_class(int class_id) noexcept;

// ---------------------------------------------------------------------------
// The generation gate (0046C550)
// ---------------------------------------------------------------------------

// 004BCA50's return value, switched on at 0046C772 through the jump table at
// 0046CCE4. Values above 10 fall to the default and reject the entity.
enum class SceneGameMode : int {
    IslandCapture1v1 = 0,
    IslandCapture2v2 = 1,
    IslandCapture3v3 = 2,
    IslandCapture4v4 = 3,
    Duel = 4,
    Escort = 5,
    Siege = 6,
    Competitive = 7,
    InGameGeneration = 8,
    EngineMovie = 9,
    Unrestricted = 10,
};

// The play-area records live in an array of six-float rows at *(00E188A8)+705Ch
// with an 18h stride. Only four of the six floats are read: index 0 and 3 bound
// X, index 5 and 2 bound Z. The Z pair is stored high first, so index 2 is the
// larger value.
struct SceneModeArea {
    float bounds[6]{};
};
inline constexpr std::uint32_t kSceneModeAreaBase = 0x705c;
inline constexpr std::uint32_t kSceneModeAreaStride = 0x18;

// Slot order in that array is not the mode order: modes 0..3 use slots 0..3,
// Siege uses slot 4, Competitive slot 5, Duel slot 6 and Escort slot 7. Returns
// -1 for the three modes that have no area test.
int scene_mode_area_slot(SceneGameMode mode) noexcept;
// Byte offset of the mode's row from *(00E188A8), or 0 when it has none.
std::uint32_t scene_mode_area_offset(SceneGameMode mode) noexcept;

// The key the mode reads out of the entity's `MultiType` sub-bag once the point
// is inside the area, or nullptr for modes 8, 9 and 10.
const char* scene_mode_property_key(SceneGameMode mode) noexcept;

// FCOMIP/JBE: every bound is strict, so a point exactly on an edge is rejected.
bool scene_point_in_mode_area(const SceneModeArea& area, float x, float z) noexcept;

// The nested `"MultiType" { ... }` sub-bag. 126098 of the 133664 entities in the
// 259 installed .scn files carry one. Its absence, not its contents, selects the
// deferred-record branch of 0046C550.
inline constexpr const char* kSceneMultiTypeKey = "MultiType";
const ScenePropertyBlock* find_scene_property_block(const ScenePropertyBlock& bag,
                                                    const std::string& key) noexcept;
// `B` properties are authored as `false` / `true`; anything else reads as false.
bool scene_property_bool(const SceneProperty* prop) noexcept;

// The 5Ch record 0046C550 allocates at 0046C5F1 and 0046CB9C, filled by 008F41F0
// into 004693C0 and appended to the list at this+24h by 0046C450. The same
// record shape is what docs/SCENE_FILE_READER.md saw at 0046D68A.
struct SceneDeferredEntityRecord {
    std::string class_name;   // argument 1, from 00468660
    std::string entity_name;  // argument 2, the quoted name
    std::string party;        // the entity-level `Party` property, authored token
    float local_frame[16]{};  // argument 4
    float parent_frame[16]{}; // arguments 7..22, the composed parent frame
    std::string parent_name;  // parent->vtable[10h](), empty when there is no parent
};

// Remaining services reached by 0046C550 outside the parsed entity block.
// Parent-pose geometry is called directly through the canonical pose binding.
// No default implementation stands in for the remaining external behaviour.
struct SceneEntityGateHost {
    virtual ~SceneEntityGateHost() = default;

    // 004BCA50 with ECX = *(00E188A8).
    virtual SceneGameMode effective_game_mode() = 0;

    // One six-float row of the play-area array; `slot` is scene_mode_area_slot.
    virtual SceneModeArea mode_area(int slot) = 0;

    // arg3 == 0 at 0046C6E5: 00413920 multiplies the entity's local frame by the
    // parent frame and the product's +30h / +38h become X and Z.
    virtual void compose_world_frame(const float local_frame[16],
                                     const float parent_frame[16],
                                     float out[16]) = 0;

    // captured parent->vtable[10h](), used only to fill the deferred record.
    // The same original argument identity is passed even after other host calls.
    virtual std::string parent_name(void* captured_parent_identity) = 0;

    // operator new(5Ch), 008F41F0 + 004693C0, then 0046C450 onto this+24h.
    virtual void append_deferred_entity_record(const SceneDeferredEntityRecord& record) = 0;

    // 0046BF20 -> 0046BF70: the multiplayer stock walk over `PlaneStock %d`,
    // `Slot %d`, `Stock %d`, `NumSlots`, `AlliedList` and `JapanList`.
    virtual void register_multiplayer_stock(const ScenePropertyBlock& properties,
                                            const std::string& class_name) = 0;
};

// The parsed entity block as 0046CF40 hands it over.
struct SceneEntityGateInputs {
    std::string class_name;                     // argument 1
    std::string entity_name;                    // argument 2
    const ScenePropertyBlock* properties{nullptr};  // argument 5
    const float* local_frame{nullptr};          // argument 4, 16 floats
    const float* parent_frame{nullptr};         // arguments 7..22, 16 floats
    void* parent_identity{nullptr};            // actual nullable argument 3, borrowed
    bool record_already_built{false};           // argument 23 already non-null
};

// Which rule produced the answer. The native only returns a byte; this is for
// the reconstruction's own reporting and has no counterpart in the binary.
enum class SceneGateRule {
    NoMultiTypeBlock,      // 0046C5A6 not taken
    ClassAlwaysGenerated,  // the id is in the set at this+164h
    InsideModeArea,        // the area test passed; the answer is the mode's key
    OutsideModeArea,       // the area test failed
    GenerateInGame,        // mode 8 or 9 with `GenerateInGame` set
    EngineMovieFallback,   // mode 9 without it
    Unrestricted,          // mode 10
    UnknownGameMode,       // the jump table default
};

struct SceneEntityGateResult {
    bool generate{false};
    bool deferred_record_created{false};
    bool stock_registered{false};
    SceneGateRule rule{SceneGateRule::UnknownGameMode};
};

// Typed control-flow projection of 0046C550 with explicit remaining services.
// The parent-pose branch uses the actual identity and canonical pose resolver.
// `always_generate_class_ids` is the std::set at this+164h,
// searched at 0046C741 by 00468DB0; what fills it was not recovered, so it is an
// input here rather than a table.
SceneEntityGateResult scene_entity_generation_gate_0046c550(
    const SceneEntityGateInputs& inputs,
    const std::vector<int>& always_generate_class_ids,
    SceneEntityGateHost& host, PoseRefreshResolver& poses);

}  // namespace bsp
