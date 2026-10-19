// Segment 59 (008EE670..00922240), packet cc13-seg59 (marker cc13_seg59).
//
// What segment 59 still lacked after cc13-seg59A / cc13-seg59b / seg59c: the world destructor
// 00904C40, which the classifier tagged CG_vector_deleting_dtor although it is game code. The
// other open rows of the segment are library or compiler bodies, bound by ledger record only.
// docs/CC13_SEG59.md has the evidence and the census.
//
// Model: the routine works on the original object as raw bytes (Win32 layout, image offsets)
// and reaches every callee through Seg59Host, whose methods name the image routine they stand
// for. A new C++ interface, not a drop-in binary replacement; nothing here is bound into
// bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG59_HPP
#define BSP_CC13_SEG59_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg59 {

// World layout read by 00904C40.
inline constexpr std::uint32_t kWorldVtable = 0x00CE7784;   // stored at 00904C5F
inline constexpr std::size_t kWorldEntityHeader = 0x04;     // ptr to header; [header] = first entity
inline constexpr std::size_t kWorldMissionHeader = 0x08;    // ptr to header; [header] = first mission entity
inline constexpr std::size_t kWorldParentList = 0x0C;       // 12-byte raw parent header (004BF8E0)
inline constexpr std::size_t kWorldHeaderArray = 0x18;      // 97 raw headers of 12 bytes (004C2D30)
inline constexpr std::uint32_t kWorldHeaderArrayCount = 0x61;
inline constexpr std::uint32_t kWorldHeaderArrayStride = 0x0C;
inline constexpr std::size_t kWorldRefObject = 0x4A8;       // refcounted, count at +4h
inline constexpr std::size_t kWorldActiveByte = 0x4AC;
inline constexpr std::size_t kWorldListHead = 0x4B4;        // std::list sentinel node
inline constexpr std::size_t kWorldListSize = 0x4B8;
// Entity fields.
inline constexpr std::size_t kMissionEntityNext = 0x44;
inline constexpr std::size_t kEntityNext = 0x38;
inline constexpr std::size_t kEntityParent = 0x3C;
inline constexpr std::size_t kEntityAlive5C = 0x5C;
inline constexpr std::size_t kEntityByte5E = 0x5E;
inline constexpr std::size_t kEntityKilled6C = 0x6C;
inline constexpr std::int32_t kMissionKillCause = 7;        // PUSH 7 at 00904C84

class Seg59Host {
public:
    virtual ~Seg59Host() = default;
    // [00E0AF20] = 0 (byte), 00904C6D.
    virtual void clear_byte_00e0af20() = 0;
    // 00926D90 BSP_MissionEntity_Kill, __thiscall(entity, int cause), RET 4.
    virtual void mission_entity_kill_00926d90(void* entity, std::int32_t cause) = 0;
    // 00874D00 BSP_Game_RunExtraFixedStep, __fastcall(char mode in CL), RET.
    virtual void game_run_extra_fixed_step_00874d00(char mode) = 0;
    // 00922FD0 BSP_SceneNode_Kill, __fastcall(node), RET.
    virtual void scene_node_kill_00922fd0(void* node) = 0;
    // The entity's vtable slot +DCh, __thiscall(entity, float), RET 4.
    virtual void entity_vcall_dc(void* entity, float value) = 0;
    // 00904600 BSP_World_UpdateMatrixInterpolators, __thiscall(world, float), RET 4.
    virtual void world_update_matrix_interpolators_00904600(void* world, float dt) = 0;
    // 00BF65AC, the CRT _free jump.
    virtual void crt_free_00bf65ac(void* block) = 0;
    // [00CE2220] KERNEL32 InterlockedDecrement; returns the decremented value.
    virtual std::int32_t interlocked_decrement_ce2220(volatile std::int32_t* count) = 0;
    // The object's vtable slot +0h, __thiscall(object), no stack argument.
    virtual void object_vcall_0(void* object) = 0;
    // 00BF7C6E `eh vector destructor iterator'(first, size, count, 004C2D30): destroys the
    // elements from the last to the first with the 004C2D30 raw-header callback.
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* first, std::uint32_t size,
                                                        std::uint32_t count) = 0;
    // 004BF8E0 BSP_ParentList_ClearRawHeaderStorage, ECX = header, RET.
    virtual void parent_list_clear_raw_header_storage_004bf8e0(void* header) = 0;
};

// 00904C40, the world destructor (body 00904C40..00904E49), __thiscall(ECX = world), RET.
// Called only by 004CB0B0 BSP_World_ScalarDeletingDestructor (00904C40's one caller).
void world_destruct_00904c40(Seg59Host& host, void* world);

// The three property-map destructors the Unwind funclets call (tagged CG_adjustor_thunk):
// store the class vtable at +0h, then tail-JMP to the map's clear routine.
inline constexpr std::uint32_t kScenePropertyRecordMapVtable = 0x00D162C4;
inline constexpr std::uint32_t kEnumSymbolMapVtable = 0x00D162C0;
inline constexpr std::uint32_t kPropertyLibraryMapVtable = 0x00D162C8;

class Seg59MapHost {
public:
    virtual ~Seg59MapHost() = default;
    // 008F3F30 BSP_ScenePropertyRecordMap_ClearOwnedValues, ECX = object.
    virtual void scene_property_record_map_clear_008f3f30(void* object) = 0;
    // 008F4B60 BSP_EnumSymbolMap_Clear, ECX = object.
    virtual void enum_symbol_map_clear_008f4b60(void* object) = 0;
    // 008F4C50 BSP_PropertyLibraryMap_ClearOwnedBags, ECX = object.
    virtual void property_library_map_clear_008f4c50(void* object) = 0;
};

// 008F4160, __thiscall(ECX = object): [object] = 00D162C4; JMP 008F3F30.
void scene_property_record_map_destruct_008f4160(Seg59MapHost& host, void* object);
// 008F4D50, __thiscall(ECX = object): [object] = 00D162C0; JMP 008F4B60.
void enum_symbol_map_destruct_008f4d50(Seg59MapHost& host, void* object);
// 008F4D60, __thiscall(ECX = object): [object] = 00D162C8; JMP 008F4C50.
void property_library_map_destruct_008f4d60(Seg59MapHost& host, void* object);

// The game+21A0h scoring manager (14B8h bytes, vtable 00D18ACC): 8 MissionScoreRecords of
// 284h bytes at +4h, std::_Tree objects at +1428h, +1434h, +1440h and +1488h (each: +0h
// allocator word, +4h head node, +8h size), and a pool-backed string at +147Ch (length) /
// +1480h (block).
inline constexpr std::uint32_t kScoringManagerVtable = 0x00D18ACC;
inline constexpr std::size_t kScoringRecords = 0x4;
inline constexpr std::uint32_t kScoringRecordStride = 0x284;
inline constexpr std::uint32_t kScoringRecordCount = 8;
inline constexpr std::size_t kScoringTree1428 = 0x1428;
inline constexpr std::size_t kScoringTree1434 = 0x1434;
inline constexpr std::size_t kScoringTree1440 = 0x1440;
inline constexpr std::size_t kScoringPoolLength = 0x147C;
inline constexpr std::size_t kScoringPoolBlock = 0x1480;
inline constexpr std::size_t kScoringTree1488 = 0x1488;

class Seg59ScoringHost {
public:
    virtual ~Seg59ScoringHost() = default;
    // 00BF7CD1 `eh vector constructor iterator'(first, 284h, 8, 0091CE90, 00593570).
    virtual void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size,
                                                         std::uint32_t count) = 0;
    // 00BF7C6E `eh vector destructor iterator'(first, 284h, 8, 00593570).
    virtual void eh_vector_destructor_iterator_00bf7c6e(void* first, std::uint32_t size,
                                                        std::uint32_t count) = 0;
    // The _Tree head-node allocators, __thiscall(ECX = tree), RET, EAX = node.
    virtual void* tree_buy_head_node_00582840(void* tree) = 0;   // +1428h
    virtual void* tree_buy_head_node_00907cf0(void* tree) = 0;   // +1434h, +1440h
    virtual void* tree_buy_head_node_0063b110(void* tree) = 0;   // +1488h
    // The _Tree erase(first, last) instantiations, __thiscall(ECX = tree, iterator* out,
    // tree, first, tree, last), RET 14h.
    virtual void tree_erase_range_0063fb30(void* tree, void* first, void* last) = 0;  // +1488h
    virtual void tree_erase_range_00911b30(void* tree, void* first, void* last) = 0;  // +1440h, +1434h
    virtual void tree_erase_range_0058ae10(void* tree, void* first, void* last) = 0;  // +1428h
    // 00419CC0 BSP_SizedStoragePool_GetSingleton (no arguments, EAX = pool).
    virtual void* sized_storage_pool_get_singleton_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(pool, block, size, unused), RET 0Ch.
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* block,
                                                          std::uint32_t size,
                                                          std::uint32_t unused) = 0;
    // 00BF65AC, the CRT _free jump.
    virtual void crt_free_00bf65ac(void* block) = 0;
};

// 0091D640, __thiscall(ECX = manager), RET, EAX = manager. The constructor 004E3E74 runs on
// a 14B8h-byte allocation before storing it at game+21A0h (tagged CG_array_ctor_helper).
void* scoring_manager_construct_0091d640(Seg59ScoringHost& host, void* manager);
// 0091CCD0, __thiscall(ECX = manager), RET. The destructor; its one caller is the scalar
// deleting destructor 0091D760 (tagged CG_vector_deleting_dtor).
void scoring_manager_destruct_0091ccd0(Seg59ScoringHost& host, void* manager);

}  // namespace bsp::cc13_seg59

#endif  // BSP_CC13_SEG59_HPP
