#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

// Mission world construction, 004DE610 BSP_Game_ConstructWorld.
//
// __fastcall void(GGame*), body 004DE610-004DFAFC, RET 0 (no stack arguments).
// Sole caller 004DFB70 BSP_Game_LoadMissionScene at 004E01DE, inside the "1_"
// VFS block and immediately after the first 0046DF00 scene pass. Torn down by
// 004D2BB0 BSP_Game_DestroyWorld (caller 004DA780).
//
// Every offset and size below was taken from the raw listing, not from the
// pseudocode: Ghidra aliases the allocation sizes into stack slots (piStack_ac,
// local_88) and drops the hidden this of the unprototyped callees, and it
// declares 004DF846 unreachable, which is exactly the block that materialises
// the "Ocean initialization failed" literal.
//
// docs/GAME_WORLD_CONSTRUCT.md, reports/game_world_construct.json.

namespace bsp {

// ---------------------------------------------------------------------------
// The world object at game+19CCh
// ---------------------------------------------------------------------------
// 4BCh from operator new 00BF55BE at 004DE651, memset to 0 at 004DE664 before
// the constructor 004CB030 runs, then 009037F0(world, 1, 1) at 004DE69C.
// The memset is load-bearing: the constructor never writes +4h or +8h, and +4h
// is the intrusive activation-chain head that 00903670 walks every frame
// (docs/GAME_WORLD_ENTITIES.md).

inline constexpr std::size_t kWorldObjectSize = 0x4BC; // push 4BCh, 004DE651
inline constexpr std::uint32_t kWorldVTable = 0x00CE7784; // 004CB04C
inline constexpr std::uint32_t kWorldScalarDeletingDestructor = 0x004CB0B0; // slot 0

// The eh_vector_constructor_iterator at 004CB076: 61h elements of stride 0Ch
// starting at +18h, element constructor 004C2D30 and destructor 004B7EC0. The
// array therefore ends at +18h + 61h*0Ch = +4A4h.
inline constexpr std::size_t kWorldSlotArrayOffset = 0x18;
inline constexpr std::size_t kWorldSlotStride = 0x0C;
inline constexpr std::size_t kWorldSlotCount = 0x61;
inline constexpr std::size_t kWorldSlotArrayEnd
    = kWorldSlotArrayOffset + kWorldSlotStride * kWorldSlotCount; // 4A4h

inline constexpr std::size_t kWorldActivationChainHeadOffset = 0x04; // memset only
inline constexpr std::size_t kWorldReadyFlagOffset = 0x4AC; // byte 1, 004CB098
inline constexpr std::size_t kWorldListOffset = 0x4B0; // _Myfirstiter
inline constexpr std::size_t kWorldListHeadOffset = 0x4B4; // = 004C3080(), 004CB091
inline constexpr std::size_t kWorldListSizeOffset = 0x4B8; // = 0, 004CB094

// Projection of what 004CB030 leaves behind. Fields the constructor does not
// write are shown at their memset value, which is what the readers observe.
struct WorldObjectLayout {
    std::uint32_t vtable{kWorldVTable};
    std::uint32_t activation_chain_head{0}; // +4h, memset
    std::uint32_t reserved_08{0}; // +8h, memset
    std::uint32_t field_0c{0}; // +0Ch, 004CB057
    std::uint32_t field_10{0}; // +10h, 004CB05A
    std::uint32_t field_14{0}; // +14h, 004CB05D
    std::size_t slot_count{kWorldSlotCount}; // +18h array, constructed elementwise
    bool ready_flag{true}; // +4ACh
    std::uint32_t list_head{0}; // +4B4h, non-null sentinel from 004C3080
    std::uint32_t list_size{0}; // +4B8h
};
WorldObjectLayout construct_world_object_004cb030() noexcept;

// ---------------------------------------------------------------------------
// The marker manager at game+21D4h
// ---------------------------------------------------------------------------
// 50h from operator new 00BF681B at 004DF9DE, constructor 006DECA0, follow-up
// 006D6200. The five 12-byte std::list members are at the five offsets the
// per-frame update 006DC1A0 walks; include/bsp/world_entities.hpp already names
// those offsets (kMarkerGroupListOffset and the four after it) and this header
// deliberately does not restate them.

inline constexpr std::size_t kMarkerManagerSize = 0x50; // push 50h, 004DF9DE
inline constexpr std::uint32_t kMarkerManagerVTable = 0x00CF8FF8; // 006DECD8
inline constexpr std::uint32_t kMarkerManagerScalarDeletingDestructor = 0x006DEDC0;
inline constexpr std::size_t kMarkerManagerListCount = 5;
inline constexpr std::size_t kMarkerManagerListStride = 0x0C; // {iter, head, size}

// One std::list member as 006DECA0 leaves it. _Myfirstiter is never written by
// the constructor; _Myhead is a self-linked sentinel from a per-value-type
// allocator and _Mysize is zero.
struct MarkerListInit {
    std::uint32_t head_allocator{0}; // 006D8590 / 006D85E0 / 006D8660
    std::size_t sentinel_flag_offset{0}; // node byte set to 1: 1Dh / 25h / 11h
    bool head_self_linked{true}; // node +0h, +4h and +8h all point at the node
    std::uint32_t size{0};
};

struct MarkerManagerLayout {
    std::uint32_t vtable{kMarkerManagerVTable};
    std::uint32_t field_04{0}; // 006DECAE
    std::uint32_t field_08{0}; // 006DECB1
    std::uint32_t field_0c{0}; // 006DECB4
    bool field_10{false}; // byte, 006DECB7
    MarkerListInit lists[kMarkerManagerListCount]{};
};
MarkerManagerLayout construct_marker_manager_006deca0() noexcept;

// ---------------------------------------------------------------------------
// Scene-record fields 004DE610 reads
// ---------------------------------------------------------------------------
// The record is game+5FCh, the block 0046DF00 filled on the pass before this
// routine. include/bsp/mission_scene_load.hpp already names the offsets that
// packet established; these are the ones only 004DE610 touches.

inline constexpr std::size_t kSceneRecordFogEntryOffset = 0xA20; // 004DF781
inline constexpr std::size_t kSceneRecordFogEntryStride = 0x10;
inline constexpr std::size_t kSceneRecordFogEntryCount = 4; // to A60h, 004DF79D
inline constexpr std::size_t kSceneRecordCameraNearOffset = 0xA78; // 004DE952
inline constexpr std::size_t kSceneRecordLightScalarOffset = 0xA84; // 004DF3FD
inline constexpr std::size_t kSceneRecordFoliageDensityOffset = 0xA88; // 004DE96E
inline constexpr std::size_t kSceneRecordFoliageRangeOffset = 0xA8C; // 004DE989
inline constexpr std::size_t kSceneRecordFoliageEnableOffset = 0xA90; // byte, 004DE9A4
inline constexpr std::size_t kSceneRecordCausticsScalar0Offset = 0xAB8; // 004DEF1F
inline constexpr std::size_t kSceneRecordCausticsScalar1Offset = 0xABC; // 004DEF3B
inline constexpr std::size_t kSceneRecordCausticsSourceOffset = 0xACC; // 004DECB9
inline constexpr std::size_t kSceneRecordWaterTracerColorOffset = 0xC44; // 004DF59B
inline constexpr std::size_t kSceneRecordOceanDescOffset = 0xC54; // 004DF457
inline constexpr std::size_t kSceneRecordOceanScalar0Offset = 0xC5C; // 004DF549
inline constexpr std::size_t kSceneRecordOceanScalar1Offset = 0xC60; // 004DF56B
inline constexpr std::size_t kSceneRecordSkyFlagOffset = 0xC20; // byte, 004DF7E1
inline constexpr std::size_t kSceneRecordSkyParam0Offset = 0xC18; // 004DF8A2
inline constexpr std::size_t kSceneRecordSkyParam1Offset = 0xC1C; // 004DF8C1
inline constexpr std::size_t kSceneRecordCloudDescOffset = 0x990; // 004DF878
inline constexpr std::size_t kSceneRecordTerrainVectorOffset = 0x100C; // 004DEA22
inline constexpr std::size_t kSceneRecordTerrainHandleOffset = 0x1044; // 004DE9C4
inline constexpr std::size_t kSceneRecordOceanVector0Offset = 0x1014; // 004DF495
inline constexpr std::size_t kSceneRecordOceanVector1Offset = 0x1024; // 004DF4D1
inline constexpr std::size_t kSceneRecordOceanVector2Offset = 0x1034; // 004DF50D
inline constexpr std::size_t kSceneRecordShoreHandleOffset = 0x104C; // 004DEAFD

// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------
inline constexpr const char* kWorldSceneRootName = "World"; // 00CE7E10
inline constexpr const char* kOperatorNodeName = "Operator"; // 00CE7E04
inline constexpr const char* kDefaultSkyName = "sky_001"; // 00CE7AB0
inline constexpr const char* kWaterTracerColorKey = "WaterTracerColor"; // 00CE7D64
inline constexpr const char* kCausticsTextureSourceKey = "CausticsTextureSource"; // 00CE7DEC
inline constexpr const char* kCausticsDayLightValue = "CausticsDayLight"; // 00CE77C4

// The three shore-wave pairs, in body order. Both branches install the same
// literals; only the caustics source differs between them.
inline constexpr std::size_t kShoreWaveLayerCount = 3;
inline constexpr const char* kShoreWaveSourceKeys[kShoreWaveLayerCount] = {
    "ShoreWaveTextureSource0", // 00CE7DC8, 004DED14 and 004DF0F1
    "ShoreWaveTextureSource1", // 00CE7DA4, 004DEDA3 and 004DF182
    "ShoreWaveTextureSource2", // 00CE7D80, 004DEE39 and 004DF20E
};
inline constexpr const char* kShoreWaveSourceValues[kShoreWaveLayerCount] = {
    "ShoreWaves0", // 00CE7DE0
    "ShoreWaves1", // 00CE7DBC
    "ShoreWaves2", // 00CE7D98
};

// 00CE7D3C. Built into a pooled buffer at 004DF829-004DF867 and released again
// without a reader; see ocean_failure_is_reported below.
inline constexpr const char* kOceanInitFailedLiteral = "Ocean initialization failed";
inline constexpr std::size_t kOceanInitFailedLength = 0x1B; // push 1Bh, 004DF82B

// The native site allocates a string of exactly this length, memcpy's the
// literal into it and hands the block straight back to the sized storage pool.
// Nothing reads it, and it is not conditioned on any ocean result: the only
// guard is game+5FCh being non-null. Returns false to state that plainly.
bool ocean_failure_is_reported() noexcept;

// ---------------------------------------------------------------------------
// Tail objects
// ---------------------------------------------------------------------------
// The loop at 004DF911-004DF959 makes 8 objects of 30h at game+21A4h..+21C0h,
// constructor 008DF900(index) and registration 008DA160. BSP_Game_DestroyWorld
// releases only the first three (004D2C9F sets the counter to 3), so five are
// left to whatever 008DA160 registered them with.
inline constexpr std::size_t kChannelObjectCount = 8; // cmp edi, 8 at 004DF956
inline constexpr std::size_t kChannelObjectSize = 0x30;
inline constexpr std::size_t kChannelSlotBase = 0x21A4;
inline constexpr std::size_t kChannelSlotsDestroyed = 3; // 004D2C9F

// One tail allocation: operator new of `size`, constructor when the block is
// non-null, store to the game slot, then a follow-up call on the new object.
struct TailConstruction {
    std::size_t game_offset;
    std::size_t size;
    std::uint32_t constructor;
    std::uint32_t follow_up;
};
inline constexpr std::size_t kTailConstructionCount = 6;
inline constexpr TailConstruction kTailConstructions[kTailConstructionCount] = {
    {0x21C8, 0x10, 0x008E2B90, 0x008E25F0}, // 004DF966
    {0x21CC, 0x10, 0x009221E0, 0x00920EC0}, // 004DF9A2
    {0x21D4, 0x50, 0x006DECA0, 0x006D6200}, // 004DF9DE, the marker manager
    {0x21DC, 0x70, 0x00707480, 0x007018C0}, // 004DFA1A
    {0x21E8, 0x58, 0x00735030, 0x00733030}, // 004DFA56
    {0x0000, 0x10, 0x00945820, 0x00941310}, // 004DFA92, stored to DAT_00F89B3C
};

// The input context the routine enables on the way out (004DFAD7: 004BEC00
// with 1Dh and 1, then 00A933F0). BSP_Game_DestroyWorld opens with the same
// pair and a 0, which is the clearest evidence of the pairing.
inline constexpr int kWorldInputContext = 0x1D;

// ---------------------------------------------------------------------------
// The sequence
// ---------------------------------------------------------------------------

// Which of the two configuration branches 004DE944 selects. The record is
// game+5FCh; the null branch installs literal defaults for the same settings.
enum class WorldConfigSource {
    SceneRecord, // 004DE952, record non-null
    Defaults, // 004DEF47
};

// The game slots 004DE610 fills, in the order it fills them. Zero means the
// allocation failed and the slot keeps a null, which is what the per-frame
// readers test (docs/GAME_WORLD_OCEAN.md gates the whole ocean block on
// game+19E8h being non-null).
struct WorldConstructResult {
    std::uint32_t world{0}; // +19CCh
    std::uint32_t scene_root{0}; // +19ECh
    std::uint32_t operator_node{0}; // +19FCh
    std::uint32_t operator_child{0}; // +1A00h, released again before return
    std::uint32_t ocean_owner{0}; // +19E8h
    std::uint32_t sky{0}; // +19F0h
    std::uint32_t atmosphere{0}; // the 94h object, released after handover
    std::uint32_t marker_manager{0}; // +21D4h
    WorldConfigSource config_source{WorldConfigSource::Defaults};
    bool ocean_from_scene_record{false}; // 004DF421
    bool input_context_enabled{false}; // 004DFAD7
    std::size_t channel_objects{0}; // 004DF911 loop
};

// Integration boundary. One method per native call site that this packet owns
// or has to reach; the scene-graph, terrain, ocean-render, lighting and
// resource internals behind them belong to other packets and are contracts
// here, not behaviour. Nothing has a default implementation.
struct WorldConstructHost {
    virtual ~WorldConstructHost() = default;

    // 004DE645: when game+719Dh is set, [00F8D394]+18h = 20000000h.
    virtual bool debug_render_flag() = 0; // game+719Dh
    virtual void set_renderer_budget(std::uint32_t value) = 0; // [00F8D394]+18h

    // 004DE651-004DE69C. The allocator zeroes the block before the constructor.
    virtual std::uint32_t create_world(const WorldObjectLayout& layout) = 0;
    virtual void world_post_construct(std::uint32_t world, int a, int b) = 0; // 009037F0

    // 004DE6A1 and 004DE73F. The scene root comes from the general allocator,
    // the Operator node from the node allocator 00B71930 with 458h in ECX.
    virtual std::uint32_t create_scene_node(std::size_t size, const std::string& name) = 0;
    virtual std::uint32_t create_operator_node(std::size_t size, const std::string& name) = 0;
    virtual void publish_operator_node(std::uint32_t node) = 0; // DAT_00E188B0
    virtual void set_camera_near_plane(std::uint32_t node, float value) = 0; // 00B6FBF0

    // 004DE7E4-004DE843: a 34h child handed to the Operator node, then released
    // through its own InterlockedDecrement, so the node holds the only
    // reference from here on.
    virtual std::uint32_t create_operator_child(std::size_t size) = 0; // 00B1F850
    virtual void attach_operator_child(std::uint32_t node, std::uint32_t child) = 0; // 00B71990
    virtual void release_ref(std::uint32_t object) = 0; // InterlockedDecrement + vtable[0]

    // 004DE936 and 004DE93F. 004C9EC0 is what creates the directional light at
    // game+19F8h (strings TestDirectionalLight and AllLights).
    virtual void construct_scene_services() = 0; // 004DCDF0
    virtual void construct_lighting() = 0; // 004C9EC0

    // 004DE944. Null selects the defaults branch at 004DEF47.
    virtual std::uint32_t scene_record() = 0; // game+5FCh
    virtual float record_float(std::size_t offset) = 0;
    virtual std::uint8_t record_byte(std::size_t offset) = 0;
    virtual std::uint32_t record_field(std::size_t offset) = 0; // &record[offset]

    // The parameter setter 00B1B830 on the singleton 00F8D434, used for the
    // caustics source and the three shore-wave pairs on both branches.
    virtual void set_world_parameter(const std::string& key, const std::string& value) = 0;
    virtual void set_world_parameter_from_record(
        const std::string& key, std::uint32_t record_field) = 0;

    // 004DF421-004DF461 and 004DF5EB-004DF631: the same constructor 00BBDFF0,
    // reached with the record's ocean description or with the sky_001 literal.
    virtual std::uint32_t create_ocean_owner(
        std::size_t size, std::uint32_t scene_root, std::uint32_t description) = 0;
    virtual std::uint32_t create_ocean_owner_named(
        std::size_t size, std::uint32_t scene_root, const std::string& name) = 0;
    virtual void ocean_set_light(std::uint32_t owner, std::uint32_t light) = 0; // 00BBCE30
    virtual void ocean_set_vector(std::size_t record_offset, int which) = 0;
    virtual void ocean_set_scalar(std::size_t record_offset, int which) = 0;
    virtual void ocean_set_quality(std::uint8_t quality) = 0; // 00BBCFE0, 00F889F4

    // 004DF6A3: a 94h atmosphere object handed to the ocean owner (00BBDF20)
    // and to the Operator node (00B71940), then released.
    virtual std::uint32_t create_atmosphere(std::size_t size) = 0; // 00B84E50
    virtual void ocean_set_atmosphere(std::uint32_t owner, std::uint32_t atmosphere) = 0;
    virtual void operator_set_atmosphere(std::uint32_t node, std::uint32_t atmosphere) = 0;
    virtual void atmosphere_add_layer(std::uint32_t atmosphere, std::size_t record_offset,
        std::size_t index) = 0; // 00B84FA0

    // 004DF7B5: the sky, B8h, constructor 0078DAA0(sceneRoot, flag).
    virtual std::uint32_t create_sky(
        std::size_t size, std::uint32_t scene_root, std::uint8_t flag) = 0;
    virtual void sky_configure(std::uint32_t sky, std::uint32_t scene_root) = 0;

    // 004DF829. The host is handed the literal so a reader can see it is
    // produced; run_world_construct discards the result, as the native code
    // does. See ocean_failure_is_reported.
    virtual void build_unused_literal(const std::string& text) = 0;

    // 004DF911 loop and the six tail constructions.
    virtual std::uint32_t create_channel_object(std::size_t size, std::size_t index) = 0;
    virtual void register_channel_object(std::uint32_t object) = 0; // 008DA160
    virtual std::uint32_t create_tail_object(const TailConstruction& spec) = 0;

    virtual void set_input_context(int context, bool enabled) = 0; // 004BEC00 + 00A933F0
};

// 004DE610 in full. Returns the slots it filled; the native routine returns
// void and the caller observes the world only through the game object.
WorldConstructResult run_world_construct(WorldConstructHost& host);

} // namespace bsp
