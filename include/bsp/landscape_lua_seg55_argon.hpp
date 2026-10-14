#pragma once

// Segment 55 (00878530-0088B3D0) reconstructions: UnitTickableEntity,
// DamageableSection / DamageableClass, STL tree/vector helpers, Landscape
// terrain/height/normal/ocean-wave-damp, MissionLuaHost / MissionLuaResultSink /
// MissionLuaDeferredCall / MissionLuaCallQueue, and LuaObject / LuaValue /
// MissionEntity helpers.
// Packet argon-seg55; docs/ARGON_SEG55_LANDSCAPE_LUA.md.
//
// Reconstructed and build-tested C++ models read from Ghidra pseudocode and
// verified against x86/x87 assembly. Field names and C++ interfaces are
// hypotheses, not recovered symbols or drop-in binary ABI replacements.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp::landscape_lua_seg55_argon {

struct Vec2 {
    float x{0.0f};
    float z{0.0f};
};

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct Mat4x4 {
    float m[16]{};
};

// ============================================================================
// 1. STL Tree & Vector Helpers + DamageableSection / DamageableClass /
//    UnitTickableEntity (00878530..0087C700)
// ============================================================================

struct DamageableTreeNode {
    DamageableTreeNode* left{nullptr};    // +00h
    DamageableTreeNode* parent{nullptr};  // +04h
    DamageableTreeNode* right{nullptr};   // +08h
    float local_matrix[16]{};             // +10h..+4Fh
    void* section_model_owner{nullptr};   // +50h
    bool color_black{false};              // +54h
    bool is_nil{false};                   // +55h
};

struct DamageableTree {
    std::uint32_t proxy_or_comp{0};       // +00h
    DamageableTreeNode* head{nullptr};    // +04h
    std::uint32_t size{0};                // +08h
};

struct DamageableTreeIterator {
    const DamageableTree* container{nullptr}; // +00h
    DamageableTreeNode* node{nullptr};        // +04h
};

struct DamageableTreeHost {
    virtual ~DamageableTreeHost() = default;
    // 008772B0 STL_DamageableFrameTree_Advance_008772b0
    virtual void advance_iterator_008772b0(DamageableTreeIterator& it) = 0;
    // 00877FA0 std_Tree_Buynode
    virtual DamageableTreeNode* buy_head_node_00877fa0() = 0;
};

// 00878530 STL_Tree_Inc_00878530
// __thiscall(ECX = iterator*), RET. Calls 008772B0 and returns `it`.
DamageableTreeIterator* stl_tree_inc_00878530(
    DamageableTreeIterator* it,
    DamageableTreeHost& host);

// 00878610 STL_Tree_InitHead_00878610
// __thiscall(ECX = tree*), RET. Allocates head via 00877FA0, sets +55h = 1,
// self-links parent/left/right, zeroes size (+08h).
void stl_tree_init_head_00878610(
    DamageableTree& tree,
    DamageableTreeHost& host);

// 00878A90 STL_Tree_Construct_00878A90
// __thiscall(ECX = tree*, comp, alloc), RET 8. Same body as 00878610, returns `&tree`.
DamageableTree* stl_tree_construct_00878a90(
    DamageableTree& tree,
    DamageableTreeHost& host);

// 0087C2A0 STL_Tree_Construct_0087C2A0
// __thiscall(ECX = tree*), RET. Same body as 00878A90 with plain RET, returns `&tree`.
DamageableTree* stl_tree_construct_0087c2a0(
    DamageableTree& tree,
    DamageableTreeHost& host);

// 008786C0 STL_UnitPartPointerVector_CopyBackward
// __cdecl(first, last, dest_end), RET. Copies dwords backward and returns
// `dest_end - ((last - first) >> 2)`.
std::uint32_t* stl_unit_part_pointer_vector_copy_backward_008786c0(
    const std::uint32_t* first,
    const std::uint32_t* last,
    std::uint32_t* dest_end) noexcept;

struct UnitPartPointerVector {
    std::uint32_t alloc_tag{0};        // +00h
    std::uint32_t* first{nullptr};     // +04h
    std::uint32_t* last{nullptr};      // +08h
    std::uint32_t* end_cap{nullptr};   // +0Ch
};

struct UnitPartPointerVectorHost {
    virtual ~UnitPartPointerVectorHost() = default;
    // 00878190 STL_UnitPartPointerVector_FillConstructLoop
    virtual void fill_construct_loop_00878190(
        std::uint32_t* dest,
        std::size_t count,
        const std::uint32_t* val_slot) = 0;
    // 008776B0 STL_UnitPartPointerVector_CopyConstructRange
    virtual std::uint32_t* copy_construct_range_008776b0(
        const std::uint32_t* first,
        const std::uint32_t* last,
        std::uint32_t* dest) = 0;
    // 008780F0 STL_UnitPartPointerVector_AssignRange
    virtual void assign_range_008780f0(
        std::uint32_t* first,
        std::uint32_t* last,
        const std::uint32_t* val_slot) = 0;
    // 00876A80 STL_UnitPartPointerVector_Allocate
    virtual std::uint32_t* allocate_00876a80(std::size_t capacity) = 0;
    // 00BF65AC _free
    virtual void free_00bf65ac(std::uint32_t* ptr) = 0;
    // 0087A7E0 STL_xlen_throw_0087a7e0
    virtual void on_xlen_0087a7e0() = 0;
};

// 00878BB0 STL_UnitPartPointerVector_FillConstructCount
// __thiscall(ECX = vec, dest, count, val_ptr), RET 0Ch.
// Forwards to 00878190(ECX = dest, EDX = count) and returns `dest + count`.
std::uint32_t* stl_unit_part_pointer_vector_fill_construct_count_00878bb0(
    std::uint32_t* dest,
    std::size_t count,
    const std::uint32_t* val_slot,
    UnitPartPointerVectorHost& host);

// 008793C0 STL_UnitPartPointerVector_CopyConstructWrapper
// __thiscall(ECX = vec, first, last, dest), RET 0Ch.
// Forwards to 008776B0(ECX = first, EDX = last, dest) and returns `dest + (last - first)`.
std::uint32_t* stl_unit_part_pointer_vector_copy_construct_wrapper_008793c0(
    const std::uint32_t* first,
    const std::uint32_t* last,
    std::uint32_t* dest,
    UnitPartPointerVectorHost& host);

// 0087AF40 STL_UnitPartPointerVector_InsertCopies
// __thiscall(ECX = vec, unused_iter_proxy, where_ptr, count, val_ptr), RET 10h.
// Captures `*val_ptr` by value, enforces 0x3FFFFFFF max_size (0087A7E0), grows
// capacity by 1.5x via 00876A80 when needed, or shifts/fills in place via
// 008793C0 / 00878BB0 / 008786C0 / 008780F0.
void stl_unit_part_pointer_vector_insert_copies_0087af40(
    UnitPartPointerVector& vec,
    std::uint32_t* where_ptr,
    std::size_t count,
    const std::uint32_t* val_ptr,
    UnitPartPointerVectorHost& host);

// Damageable fake-effect row: +00h float, +04h/+08h dwords, +0Ch intrusive-refcounted
// owner (count at owner+4, release through owner vslot 0).
struct FakeEffectRow16 {
    std::uint32_t words[4]{};
};

// 00878BF0 STL_Copy16_00878BF0
// __cdecl(first, last, dest), RET. Calls 00878820 (refcounting row assignment) and returns
// `dest + ((last - first) >> 4) * 16`.
FakeEffectRow16* stl_copy16_00878bf0(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest);

// 00878D10 STL_CopyBackward16_00878D10
// __cdecl(first, last, dest_end), RET. Calls 008788D0 (refcounting row assignment,
// backward) and returns `dest_end - ((last - first) >> 4) * 16`.
FakeEffectRow16* stl_copy_backward16_00878d10(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest_end);

// 008793F0 STL_CopyBackward16_008793F0
// __cdecl(first, last, dest_end), RET. Identical COMDAT twin of 00878D10.
FakeEffectRow16* stl_copy_backward16_008793f0(
    const FakeEffectRow16* first,
    const FakeEffectRow16* last,
    FakeEffectRow16* dest_end);

struct DamageableSection {
    std::uint32_t vptr{0};          // +00h (00D0DF04)
    std::uint32_t words_04_20[8]{}; // +04h..+20h
    std::uint32_t field_24{0};      // +24h
    float threshold_28{0.0f};       // +28h (10.0f from 00CE38B8)
    float threshold_2c{0.0f};       // +2Ch (10.0f from 00CE38B8)
};

struct DamageableSectionHost {
    virtual ~DamageableSectionHost() = default;
    // 00878B40 BSP_DamageableSection_CopyConstruct
    virtual void copy_construct_00878b40(
        DamageableSection& place,
        const DamageableSection& src) = 0;
};

// 00878E10 STL_Construct_DamageableSection_00878E10
// __fastcall(ECX = place, EDX = src), RET. If `place != nullptr`, calls 00878B40(place, src).
void stl_construct_damageable_section_00878e10(
    DamageableSection* place,
    const DamageableSection* src,
    DamageableSectionHost& host);

// 00879470 STL_Construct_DamageableSection_00879470
// __stdcall(place, src), RET 8. Stack-argument twin of 00878E10.
void stl_construct_damageable_section_00879470(
    DamageableSection* place,
    const DamageableSection* src,
    DamageableSectionHost& host);

// 00878EB0 BSP_DamageableSection_ConstructDefault
// __fastcall(ECX = section), RET. Stores vptr 00D0DF04, zeroes +24h, then calls
// _memset(section, 0, 0x30) (which zeroes +00h..+2Fh including the vptr in the
// shipped instruction order!) and writes 10.0f (00CE38B8) to +28h and +2Ch.
DamageableSection* damageable_section_construct_default_00878eb0(
    DamageableSection& section) noexcept;

struct RefCountedObject {
    virtual ~RefCountedObject() = default;
    std::int32_t ref_count{1}; // +04h
    virtual void destroy_vslot0() = 0;
};

// 008794C0 TMPL_RefPtr_ReleaseAt0C_008794C0
// __fastcall(ECX = owner), RET. Releases refcounted pointer at `owner + 0Ch`
// via InterlockedDecrement(+04h) and vslot 0 when 0, then clears `+0Ch`.
void tmpl_ref_ptr_release_at_0c_008794c0(RefCountedObject*& slot_at_0c);

struct DamageableOwnerVector {
    std::uint32_t alloc_tag{0};    // +00h
    void* first{nullptr};          // +04h
    void* last{nullptr};           // +08h
    void* end_cap{nullptr};        // +0Ch
};

struct DamageableClassState {
    std::uint32_t vptr{0};                   // +00h (restored to 00D0E13C)
    std::int32_t ref_count{1};               // +04h
    DamageableOwnerVector owner_rows{};      // +08h..+14h (first at +0Ch, last at +10h, cap at +14h)
    int points_count{0};                     // +2Ch (dword[0Bh])
    int points_capacity_sign{0};             // +30h (dword[0Ch])
    void* points_buf{nullptr};               // +28h (dword[0Ah])
    std::uint32_t str_38_len{0};             // +38h (dword[0Eh])
    void* str_3c_data{nullptr};              // +3Ch (dword[0Fh])
    RefCountedObject* ref_50{nullptr};       // +50h (dword[14h])
    void* buf_54{nullptr};                   // +54h (dword[15h])
    std::uint32_t str_58_len{0};             // +58h (dword[16h])
    void* str_5c_data{nullptr};              // +5Ch (dword[17h])
    DamageableTree frame_tree{};             // +60h..+68h (head at +64h, size at +68h)
};

struct DamageableClassHost {
    virtual ~DamageableClassHost() = default;
    // 0087AB30 BSP_DamageableClass_ReleaseOwnerRows(ECX = first, EDX = last, vec)
    virtual void release_owner_rows_0087ab30(void* first, void* last, DamageableOwnerVector& vec) = 0;
    // 00BF65AC / 00BF6989 _free
    virtual void free_mem(void* ptr) = 0;
    // 0087AE70 STL_Tree_EraseRange_0087ae70
    virtual void erase_frame_tree_0087ae70(DamageableTree& tree) = 0;
    // 00419CC0 + 00BD1510 SizedStoragePool return block
    virtual void return_pool_block_00bd1510(void* data, std::uint32_t len_plus_one) = 0;
    // 0074D190 BSP_PointsArray_ReserveCapacity(0)
    virtual void reserve_points_capacity_0074d190(int cap) = 0;
    // 00879240 BSP_DamageableClass_DestroyEffectVector(this + 18h)
    virtual void destroy_effect_vector_00879240() = 0;
    // 00BD30F0 BSP_RefCounted_DestroyBase
    virtual void destroy_refcounted_base_00bd30f0() = 0;
};

// 0087B8B0 STL_Vector_Tidy_0087B8B0
// __fastcall(ECX = vec), RET. If `vec.first != nullptr`, calls 0087AB30 and
// 00BF65AC `_free(vec.first)`, then zeroes `first`, `last`, `end_cap`.
void stl_vector_tidy_0087b8b0(
    DamageableOwnerVector& vec,
    DamageableClassHost& host);

// 0087C700 BSP_DamageableClass_Destruct
// __thiscall(ECX = damageable_class), RET. Restores vptr 00D0E13C, frees +54h,
// releases refcounted +50h, erases and frees tree at +60h..+68h, returns pool
// strings at +58h/+5Ch and +38h/+3Ch, clears points array at +28h..+30h, calls
// 00879240 on +18h, releases owner rows at +08h..+14h, and calls 00BD30F0.
void damageable_class_destruct_0087c700(
    DamageableClassState& self,
    DamageableClassHost& host);

struct UnitSceneNode {
    virtual ~UnitSceneNode() = default;
    virtual void scalar_delete_vslot4(int flags) = 0;
    virtual void set_local_matrix_vslot38(const float* matrix16) = 0;
};

struct UnitTickableEntityState {
    std::uint32_t vptr_00{0};           // +00h (00D0DF70)
    std::uint32_t vptr_10{0};           // +10h (00D0DF54)
    std::uint32_t vptr_24{0};           // +24h (00D0DF4C)
    UnitTickableEntityState* parent_3c{nullptr};      // +3Ch (dword[0Fh])
    UnitTickableEntityState* next_sibling_44{nullptr};// +44h (dword[11h])
    UnitTickableEntityState* first_child_48{nullptr}; // +48h (dword[12h])
    int saved_lua_mode{0};              // +C0h->+04h (3 when saved Lua data is active)
    bool has_saved_lua_holder{false};   // +C0h != nullptr
    std::uint32_t field_c4{0};          // +C4h (dword[31h])
    std::uint32_t vptr_170{0};          // +170h (00D0DF48)
    std::uint32_t vptr_1e4{0};          // +1E4h (00D0DF40)
    std::uint32_t vptr_310{0};          // +310h (00D0DF28)
    std::vector<void*> part_descriptors{}; // +348h..+350h (dwords 0D2h..0D4h)
    DamageableClassState* damageable_class{nullptr}; // +354h (dword[0D5h])
    std::uint32_t field_35c{0};         // +35Ch (dword[0D7h])
    UnitSceneNode* damage_state_instance{nullptr}; // +360h (dword[0D8h])
    float field_364{0.0f};              // +364h (dword[0D9h])
    std::uint32_t field_374{0};         // +374h (dword[0DDh])
    std::vector<UnitSceneNode*> scene_nodes{}; // +380h..+388h (dwords 0E0h..0E2h)
    bool is_kind_4{false};              // vslot +5Ch(4)
    bool is_kind_8{false};              // vslot +5Ch(8)
    void* primary_render_node{nullptr}; // vslot +18h()
};

struct UnitTickableEntityHost {
    virtual ~UnitTickableEntityHost() = default;
    // 008781F0 BSP_UnitTickableEntity_SetSceneNodesVisible
    virtual void set_scene_nodes_visible_008781f0(UnitTickableEntityState& ent, bool visible) = 0;
    // 0077D270 BSP_UnitInstance_OnKilledLua
    virtual void on_killed_lua_0077d270(UnitTickableEntityState& ent) = 0;
    // 004BCA80 BSP_ControlledListener_Publish
    virtual void controlled_listener_publish_004bca80() = 0;
    // 009277F0 TRIV_body_00923060
    virtual void base_release_009277f0(UnitTickableEntityState& ent) = 0;
    // 0077EED0 BSP_UnitOwnerEntity_Construct
    virtual void construct_owner_entity_0077eed0(UnitTickableEntityState& ent, std::uint32_t flag) = 0;
    // 00875890 BSP_TickRegistration_Construct(ECX = this + 310h, this, 0)
    virtual void construct_tick_registration_00875890(UnitTickableEntityState& ent, int group) = 0;
    // 00875490 BSP_UnitTickElement_Destruct(ECX = this + 310h)
    virtual void destruct_tick_element_00875490(UnitTickableEntityState& ent) = 0;
    // 0077E380 BSP_UnitOwnerEntity_Destruct
    virtual void destruct_owner_entity_0077e380(UnitTickableEntityState& ent) = 0;
    // 0077FAD0 BSP_NetEntity_ReadReplicationState
    virtual void read_replication_state_0077fad0(UnitTickableEntityState& ent) = 0;
    // Saved Lua "_fragile" reader hooks for 0087BF80:
    virtual int read_saved_damaged_gfx_level(UnitTickableEntityState& ent) = 0;
    virtual void set_model_group_state_00876ec0(UnitTickableEntityState& ent, int level) = 0;
    virtual bool has_saved_damage_entry(UnitTickableEntityState& ent, std::size_t part_index) = 0;
    virtual float read_saved_damage_fire(UnitTickableEntityState& ent, std::size_t part_index) = 0;
    // Frame-tree clone & attach hooks for 0087BF80:
    virtual UnitSceneNode* clone_section_scene_node(void* section_model_owner, int arg0, float scale) = 0;
    virtual void set_node_parent_00b6e680(UnitSceneNode* child, void* parent_render_node) = 0;
};

// 00878990 BSP_UnitTickableEntity_OnKilled
// __thiscall(ECX = ent), RET. Calls 008781F0(0) and tail-jumps 0077D270.
void unit_tickable_entity_on_killed_00878990(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host);

// 00878F80 BSP_UnitTickableEntity_ReleaseSceneNodes
// __fastcall(ECX = ent), RET. Deletes each non-null node in `ent.scene_nodes`
// (+380h..+384h) via vslot +04h(1), clears the vector, and recurses on children
// at `+48h` (walking `+44h`) whose vslot `+5Ch(4)` returns non-zero.
void unit_tickable_entity_release_scene_nodes_00878f80(
    UnitTickableEntityState& ent);

// 008797B0 BSP_UnitInstance_ReleaseDamageStateInstance
// __thiscall(ECX = ent), RET. If `vslot_18() != nullptr` and equals
// `controlled_render_node` (`[00E188DC]`), calls 004BCA80; calls 00878F80;
// deletes `ent + 360h` via vslot +04h(1) and zeroes it; tail-jumps 009277F0.
void unit_instance_release_damage_state_instance_008797b0(
    UnitTickableEntityState& ent,
    const void* controlled_render_node_00e188dc,
    UnitTickableEntityHost& host);

// 0087B670 BSP_UnitTickableEntity_Construct
// __thiscall(ECX = ent, flag), RET 4. Calls 0077EED0(flag) and
// 00875890(ECX = ent+310h, ent, 0), installs the six vtables, zeroes +348h..+350h
// and +380h..+388h, sets +35Ch = -1, +364h = 99.9f (00D0E12C), +374h = 0x3F,
// and +C4h = 4.
UnitTickableEntityState* unit_tickable_entity_construct_0087b670(
    UnitTickableEntityState& ent,
    std::uint32_t flag,
    UnitTickableEntityHost& host);

// 0087A410 BSP_UnitTickableEntity_Destruct
// __thiscall(ECX = ent), RET. Restores the six vtables, frees +380h..+388h and
// +348h..+350h, calls 00875490(ent+310h), and calls 0077E380(ent).
void unit_tickable_entity_destruct_0087a410(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host);

// 0087BF80 BSP_UnitTickableEntity_SEntityInitSlotA4
// __thiscall(ECX = ent), RET. Calls 0077FAD0; when `[ent+C0h]+04h == 3`, reads
// `_fragile.damagedGFXLevel` (-> 00876EC0 when non-zero) and walks `_fragile.damage[i].fire`;
// then when `damageable_class->frame_tree.size != 0` and neither `ent` nor `ent->parent_3c`
// reports `vslot_5C(8)`, clones each frame-tree section node with scale 1.0f, pushes to
// `ent.scene_nodes`, attaches to `ent->vslot_18()` via 00B6E680, and sets local matrix (+10h).
void unit_tickable_entity_sentity_init_slot_a4_0087bf80(
    UnitTickableEntityState& ent,
    UnitTickableEntityHost& host,
    DamageableTreeHost& tree_host);

// ============================================================================
// 2. Landscape Terrain, HeightField Forwarders & Collision Subobject
//    (0087F9A0..00883BB0)
// ============================================================================

struct GeneratedModelNode;

struct TerrainHeightFieldInterface {
    virtual ~TerrainHeightFieldInterface() = default;
    // vslot +0Ch: query world AABB (min Vec3 at +00h, max Vec3 at +0Ch)
    virtual void get_world_bounds_vslot0c(Vec3& out_min, Vec3& out_max) = 0;
    // vslot +1Ch: local grid to world XZ (00ADA0D0)
    virtual Vec2* local_grid_to_world_xz_vslot1c(Vec2* out, float a, float b) = 0;
    // vslot +24h: local height (00ADA160)
    virtual float local_height_vslot24(float a, float b) = 0;
    // vslot +28h: height at world XZ
    virtual float height_at_vslot28(float x, float z) = 0;
    // vslot +2Ch: height at world XZ (00ADA2C0)
    virtual float height_at_world_vslot2c(float x, float z) = 0;
    // vslot +34h: local normal (00ADA1C0)
    virtual Vec3* local_normal_vslot34(Vec3* out, float x, float z) = 0;
    // vslot +3Ch: segment trace against heightfield (returns hit bool, writes hit_pos)
    virtual bool trace_segment_vslot3c(const void* ray, const void* filter, Vec3& out_hit_pos) = 0;
    // vslot +40h: the terrain render root (also the decal target). __thiscall(hf), no stack
    // arguments: 0087F9BD, 00883075 and 008830BA call it with nothing pushed for it.
    virtual GeneratedModelNode* get_terrain_render_root_vslot40() = 0;
};

struct HitRecordState {
    std::uint32_t header_00[2]{};      // +00h..+04h
    Vec3 hit_pos{};                    // +08h..+10h
    void* entity{nullptr};             // set via 00470370
    std::uint32_t material_kind{0};    // +30h (set to 10 = 0xA)
    std::int32_t part_index{0};        // +34h (set to -1)
};

struct LandscapeConvexHull {
    int kind{0};                       // +08h (0 for static hull body)
    Vec3 local_offset{};               // +14h..+1Ch
};

struct GeneratedModelMeshElement {
    std::string effect_name{};         // from *(*(elem+20h)+7Ch) via 00B172D0
    std::vector<std::string> registered_params{};
};

struct GeneratedModelNode {
    bool matches_current_type{true};   // vslot +0Ch(00B74310())
    std::vector<std::vector<GeneratedModelMeshElement>> geometries{};
    std::vector<LandscapeConvexHull*> convex_hulls{}; // +50h..+54h
    std::vector<GeneratedModelNode*> children{};      // +34h child list walking +3Ch
};

struct TerrainGridCellDesc {
    bool present{false};
    int shape_kind{0};                 // +0Ch (0 or 2)
    std::uint32_t data_ptr{0};         // +34h (kind 0) or +40h (kind 2)
    std::uint32_t extra_2c{0};         // +2Ch (kind 2)
    std::uint32_t extra_30{0};         // +30h (kind 2)
};

struct LandscapeState {
    std::uint32_t vptr_00{0};          // +00h (00CEA090)
    std::uint32_t vptr_10{0};          // +10h (00CEA078)
    std::uint32_t vptr_24{0};          // +24h (00CEA070)
    std::uint32_t field_58{0};         // +58h
    Mat4x4 entity_matrix_74{};         // +74h
    int scene_load_mode{1};            // [this+C0h]+04h
    bool world_pose_valid{false};      // +C8h (byte[200])
    Mat4x4 world_matrix_cc{};          // +CCh..+10Bh (translation at +FCh, +100h, +104h)
    std::uint32_t decal_owner_id{0};   // +168h
    std::uint32_t vptr_170{0};         // +170h (00CEA06C)
    std::uint32_t vptr_1e4{0};         // +1E4h (00CEA05C)
    LandscapeState* spatial_owner_230{nullptr}; // +230h
    std::vector<void*> spatial_shapes{}; // +2B4h.. (+2DCh count)
    std::uint32_t spatial_shape_count_2dc{0}; // +2DCh
    void* buf_2e0{nullptr};            // +2E0h (dword[0B8h])
    std::uint32_t vptr_344{0};         // +344h (00CEA050 collision subobject)
    std::vector<void*> grid_bodies_374{}; // +374h..+380h
    std::vector<void*> vec_368{};      // +368h..+370h (dwords 0DAh..0DCh)
    std::vector<void*> vec_378{};      // +378h..+380h (dwords 0DEh..0E0h)
    Mat4x4 spatial_matrix_384{};       // +384h (900)
    std::string file_path_3c4{};       // +3C4h..+3C8h
    void* terrain_render_node_3cc{nullptr}; // +3CCh
    TerrainHeightFieldInterface* height_field_3d0{nullptr}; // +3D0h (+8Ch from +344h)
    void* coast_mesh_3d4{nullptr};     // +3D4h
    void* coastline_3d8{nullptr};      // +3D8h
    float terrain_time_3dc{0.0f};      // +3DCh
    RefCountedObject* ref_3e4{nullptr};// +3E4h (dword[0F9h])
    bool has_model_3e1{false};         // +3E1h
    void* unit_part_instance_418{nullptr}; // +418h
    void* model_resource_41c{nullptr}; // +41Ch
    GeneratedModelNode* generated_model_420{nullptr}; // +420h
    std::string model_path_424{};      // +424h..+428h
    std::uint32_t shallow_water_handle_42c{0}; // +42Ch
};

struct LandscapeHost {
    virtual ~LandscapeHost() = default;
    // 007407C0 BSP_DecalList_AddDecal, __thiscall(ECX = [landscape+168h], target, a, b, c).
    virtual void add_decal_007407c0(std::uint32_t decal_list_168, void* target, std::uint32_t a,
        std::uint32_t b, std::uint32_t c) = 0;
    // 00470370 BSP_HitRecord_SetEntity
    virtual void hit_record_set_entity_00470370(HitRecordState& rec, void* entity) = 0;
    // 00BB0A90 BSP_CoastMesh_SetTime
    virtual void coast_mesh_set_time_00bb0a90(void* coast_mesh, float t) = 0;
    // 00BAEC50 BSP_Coastline_SetTime
    virtual void coastline_set_time_00baec50(void* coastline, float t) = 0;
    // 00ADA0C0 BSP_Landscape_TickTerrainNoOp
    virtual void tick_terrain_noop_00ada0c0(TerrainHeightFieldInterface* hf, std::uint32_t arg) = 0;
    // Render node vslot +38h(entity + 74h)
    virtual void update_render_node_pose_vslot38(void* render_node, const Mat4x4& pose) = 0;
    // 00414DB0 BSP_EntityPose_RefreshWorld
    virtual void refresh_world_pose_00414db0(LandscapeState& self) = 0;
    // 006FB030 BSP_ConvexHull_DebugDrawEdges
    virtual void debug_draw_hull_edges_006fb030(LandscapeConvexHull* hull, const Mat4x4& world_mat) = 0;
    // 0042E630 + 0098A500 SpatialIndex detach node
    virtual void spatial_index_detach_0098a500(void* spatial_node_1e4) = 0;
    // 004C17D0 + 00423AF0 AvoidZoneRegistry reset layers
    virtual void avoid_zone_reset_layers_00423af0() = 0;
    // 009277F0 TRIV_body_00923060
    virtual void base_teardown_009277f0(LandscapeState& self) = 0;
    // 008826F0 BSP_Landscape_ReleaseTerrain
    virtual void release_terrain_008826f0(LandscapeState& self) = 0;
    // 009287B0 BSP_GameEntity_Destruct
    virtual void destruct_game_entity_009287b0(LandscapeState& self) = 0;
    // Hooks for 00882AC0 / 00883BB0:
    virtual TerrainHeightFieldInterface* create_height_field_00add290(const std::string& file_path) = 0;
    virtual bool vfs_file_exists(const std::string& path) = 0;
    virtual void load_tdt_00adead0(
        TerrainHeightFieldInterface* hf,
        const std::string& tdt_path,
        const std::string& null_tag,
        const std::string& dds_path,
        float world_size_x,
        float world_size_z) = 0;
    virtual GeneratedModelNode* load_terrain_mmod_007188a0(
        LandscapeState& self,
        const std::string& mmod_path) = 0;
    virtual void attach_model_to_height_field_00aded10(
        TerrainHeightFieldInterface* hf,
        GeneratedModelNode* model,
        float world_size_x,
        float world_size_z) = 0;
    // The [[00E188A8]+19F0h] block of 00882AC0 (00883062..008830BE):
    // [[00E188A8]+19F0h], the ocean wave scene (null skips the block).
    virtual void* game_ocean_scene_19f0() = 0;
    // root->vtable[10h](6, 0) -> the terrain render node stored at +3CCh.
    virtual void* render_root_create_node_vslot10(GeneratedModelNode* root, std::int32_t kind,
        std::int32_t flag) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, __thiscall(node, 0) RET 4.
    virtual void node_propagate_root_registration_00b6d890(void* node, std::int32_t arg) = 0;
    // 00BA1370 BSP_OceanWaveField_AddTerrainCoverage, __thiscall(ECX = [scene+A8h], node) RET 4.
    virtual void ocean_wave_field_add_terrain_coverage_00ba1370(void* scene_19f0, void* node) = 0;
    virtual void* create_unit_part_instance_007135c0(LandscapeState& self) = 0;
    virtual void* create_hull_physics_body_00c31aa0(
        LandscapeState& self,
        LandscapeConvexHull* hull) = 0;
    virtual int get_grid_cells_x_00adac30(TerrainHeightFieldInterface* hf) = 0;
    virtual int get_grid_cells_z_00adac40(TerrainHeightFieldInterface* hf) = 0;
    virtual TerrainGridCellDesc get_grid_cell_00adea70(
        TerrainHeightFieldInterface* hf,
        int gx,
        int gz) = 0;
    virtual void* create_grid_cell_physics_body_00c31aa0(
        LandscapeState& self,
        int gx,
        int gz,
        float world_x,
        float world_z,
        const TerrainGridCellDesc& cell) = 0;
    virtual void load_foliage_default_00b75ce0(TerrainHeightFieldInterface* hf) = 0;
    // Hook for 00882420:
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor, __thiscall(ECX = node, float factor,
    // bool recursive), RET 8. Stores `factor` at node+ACh; when `recursive` repeats on each
    // child (+34h list, +3Ch sibling).
    virtual void scene_node_set_visibility_factor_00b6da70(
        GeneratedModelNode* node,
        float factor,
        bool recursive) = 0;
    // Hooks for 00883BB0:
    virtual void create_lua_self_table_00928a00(LandscapeState& self) = 0;
    virtual void reset_terrain_state_0087fe90(LandscapeState& self) = 0;
    virtual std::string get_scene_property_string(const char* key) = 0;
    virtual bool has_scene_property(const char* key) = 0;
    virtual bool has_shallow_water_property() = 0;
    virtual std::string get_shallow_water_texture() = 0;
    virtual Vec3 get_shallow_water_pos() = 0;
    virtual float get_shallow_water_size() = 0;
    virtual void release_shallow_water_decal(std::uint32_t handle) = 0;
    virtual std::uint32_t create_shallow_water_decal(
        const std::string& texture,
        Vec2 world_xz,
        Vec2 size_xz) = 0;
    virtual void set_spatial_local_bounds_0098a920(const Vec3& local_min, const Vec3& local_max) = 0;
    virtual void spatial_index_attach_0098ba10(const Mat4x4& spatial_mat) = 0;
    virtual std::uint32_t register_decal_owner_00740fe0() = 0;
};

// 0087F9A0 BSP_Landscape_OnHit
// __thiscall(ECX = landscape, param_2, param_3, param_4), RET 0Ch.
// Calls height_field->vslot_40(param_3, param_4, param_2) and 007407C0.
void landscape_on_hit_0087f9a0(
    LandscapeState& self,
    std::uint32_t param_2,
    std::uint32_t param_3,
    std::uint32_t param_4,
    LandscapeHost& host);

// 0087F9F0 BSP_Landscape_SetMatrixNoOp
// Bare RET (vslot +120h of 00CEA050).
void landscape_set_matrix_no_op_0087f9f0() noexcept;

// 0087FA20 BSP_Landscape_ForwardLocalHeight
// __thiscall(ECX = landscape, a, b), RET 8. Forwards to `[+3D0h]->vslot_24(a, b)`.
float landscape_forward_local_height_0087fa20(
    const LandscapeState& self,
    float a,
    float b);

// 0087FA50 BSP_Landscape_ForwardHeightAt
// __thiscall(ECX = landscape, x, z), RET 8. Forwards to `[+3D0h]->vslot_28(x, z)`.
float landscape_forward_height_at_0087fa50(
    const LandscapeState& self,
    float x,
    float z);

// 0087FB90 BSP_Landscape_ForwardLocalNormal
// __thiscall(ECX = landscape, out, x, z), RET 0Ch. Forwards to `[+3D0h]->vslot_34(out, x, z)`
// and returns `out`.
Vec3* landscape_forward_local_normal_0087fb90(
    const LandscapeState& self,
    Vec3* out,
    float x,
    float z);

// 0087FC30 BSP_Landscape_ForwardLocalGridToWorldXZ
// __thiscall(ECX = landscape, out, a, b), RET 0Ch. Forwards to `[+3D0h]->vslot_1c(out, a, b)`
// and returns `out`.
Vec2* landscape_forward_local_grid_to_world_xz_0087fc30(
    const LandscapeState& self,
    Vec2* out,
    float a,
    float b);

// 0087FC60 BSP_Landscape_HeightAtWorldOrDefault
// __thiscall(ECX = landscape, const float* xz), RET 4. Calls `[+3D0h]->vslot_2c(xz[0], xz[1])`
// when `[+3D0h] != nullptr`, otherwise returns `-1.0f` (`00D7A260`).
float landscape_height_at_world_or_default_0087fc60(
    const LandscapeState& self,
    const float* xz);

// 0087FE80 BSP_Landscape_VSlot198_NoOp
// Bare RET (vslot +198h of 00CEA050).
void landscape_vslot198_no_op_0087fe80() noexcept;

// 0087FF80 BSP_CollisionShape_TraceSegment_Subobject
// __thiscall(ECX = landscape + 344h, ray, filter, hit_record), RET 0Ch.
// Tests `[shape + 8Ch]->vslot_3c(ray, filter, &local_hit)`; on hit stores `local_hit`
// at `hit_record + 08h..+10h`, sets `entity = shape - 344h` via 00470370, sets
// `hit_record + 30h = 10` and `hit_record + 34h = -1`, and returns 1.
bool collision_shape_trace_segment_subobject_0087ff80(
    LandscapeState& parent_landscape,
    const void* ray,
    const void* filter,
    HitRecordState& out_record,
    LandscapeHost& host);

// 00880000 BSP_Landscape_TickTerrain
// __thiscall(ECX = landscape, float dt), RET 4. Advances `terrain_time_3dc += dt`,
// updates `coast_mesh_3d4` (00BB0A90) and `coastline_3d8` (00BAEC50), calls 00ADA0C0
// on `height_field_3d0`, and calls `terrain_render_node_3cc->vslot_38(this + 74h)`.
void landscape_tick_terrain_00880000(
    LandscapeState& self,
    float dt,
    std::uint32_t game_terrain_tick_arg,
    LandscapeHost& host);

// 00880AF0 / 00880B20 / 00880B50 / 00880B80 BSP_RefCountedPtr_Release_00880af0..b80
// __fastcall(ECX = RefCountedObject**), RET. Decrements ref_count (+04h), invokes
// vslot 0 at zero, zeroes `*slot`, and returns `slot`.
RefCountedObject** ref_counted_ptr_release_00880af0(RefCountedObject** slot);
RefCountedObject** ref_counted_ptr_release_00880b20(RefCountedObject** slot);
RefCountedObject** ref_counted_ptr_release_00880b50(RefCountedObject** slot);
RefCountedObject** ref_counted_ptr_release_00880b80(RefCountedObject** slot);

// 00880BB0 BSP_Landscape_ReleaseVertexStreams (debug-draws convex hull edges)
// __fastcall(ECX = landscape), RET. Walks `[+420h]` convex hulls (`+50h..+54h`),
// refreshes world pose (00414DB0) when `[+C8h] == 0`, copies 4x4 matrix from `+CCh`,
// adds `hull->local_offset` (`+14h..+1Ch`) to translation (`m[12..14]`), and calls
// `006FB030(hull, &mat)`.
void landscape_release_vertex_streams_00880bb0(
    LandscapeState& self,
    LandscapeHost& host);

// 00880CD0 BSP_Landscape_Teardown
// __fastcall(ECX = landscape), RET. Detaches `this + 1E4h` from SpatialIndex (0098A500),
// resets AvoidZoneRegistry layers (00423AF0), zeroes `spatial_shape_count_2dc` (+2DCh),
// and tail-jumps 009277F0.
void landscape_teardown_00880cd0(
    LandscapeState* self,
    LandscapeHost& host);

// 00881600 BSP_Landscape_BindTerrainMaterialParams
// __fastcall(ECX = model_node), RET. Recursively binds the 8 terrain material shader
// parameters (`cStaticShot_Size_OffsetX_OffsetY`, `cBasic___ShoreH_DirtH_SimpleDist_SimpleFog`,
// `cSnoise___Div_Max_UV`, `cHnoise___Div_Max`, `cGround___slope_UV_maskUV`,
// `cUW___slope_UV`, `cUWColor0`, `cUWColor1`) on every mesh element and child node.
void landscape_bind_terrain_material_params_00881600(
    GeneratedModelNode* node);

// 00882420 BSP_Landscape_ApplyOceanWaveDamp
// __fastcall(ECX = model_node), RET. Recursively checks each mesh element's effect
// basename (after the last `/`) for `"oceanwavedamp"`; on each match calls
// `00B6DA70` as `node->SetVisibilityFactor(0.0f, false)` through
// `host.scene_node_set_visibility_factor_00b6da70`. `host` is not an original argument.
void landscape_apply_ocean_wave_damp_00882420(
    GeneratedModelNode* node,
    LandscapeHost& host);

// 008828E0 BSP_Landscape_Destruct
// __fastcall(ECX = landscape), RET. Reinstalls the six Landscape vtables, calls
// 008826F0 (`BSP_Landscape_ReleaseTerrain`), releases strings/vectors/refcounted
// members, and calls 009287B0 (`BSP_GameEntity_Destruct`).
void landscape_destruct_008828e0(
    LandscapeState& self,
    LandscapeHost& host);

// 00882AC0 BSP_Landscape_LoadTerrain
// __thiscall(ECX = landscape, const NativeString* file_path), RET 4.
// Extracts basename `F` after the last `\` or `/`, constructs heightfield at `+3D0h`
// (00ADD290), checks VFS for `models/terrain/<F>.mmod`, loads either `.mmod` + physics
// hulls/grid bodies or `.tdt` heightmap (00ADEAD0), loads default foliage (00B75CE0),
// and binds terrain material parameters (00881600).
void landscape_load_terrain_00882ac0(
    LandscapeState& self,
    const std::string& file_path,
    float world_size_x,
    float world_size_z,
    LandscapeHost& host);

// 00883BB0 BSP_Landscape_InitAttachTerrain
// __fastcall(ECX = landscape), RET. Zeroes terrain pointers, creates Lua self table
// (00928A00), resets terrain state (0087FE90), reads `FilePath` and optional `ModelPath`
// from scene properties, calls 00882AC0, creates optional `ShallowWater` decal (+42Ch),
// registers collision subobject `+344h` into `+1E4h`, computes local bounds (+42.0 on
// max.y via 00CE6840), attaches to SpatialIndex (0098BA10), and registers decal owner (00740FE0).
void landscape_init_attach_terrain_00883bb0(
    LandscapeState& self,
    float world_size_x,
    float world_size_z,
    LandscapeHost& host);

// ============================================================================
// 3. MissionLuaHost, MissionLuaResultSink, MissionLuaDeferredCall,
//    MissionLuaCallQueue & LuaObject / MissionEntity Helpers (008844F0..0088B3D0)
// ============================================================================

struct LuaVariantPair {
    int key_kind{0};          // 0 = int, 1 = string
    int int_key{0};
    std::string str_key{};
    struct LuaVariantRecord* value{nullptr};
};

struct LuaVariantRecord {
    std::uint32_t vptr{0x00D0E6F4}; // +00h
    int tag{-1};                    // +04h (0=number, 1=string, 2=bool, 3=entity_id, 4=userdata, 5=table, 6=nil)
    float num_val{0.0f};            // +08h (tag 0)
    std::string str_val{};          // +08h (tag 1)
    bool bool_val{false};           // +08h (tag 2)
    std::int16_t entity_id{0};      // +08h (tag 3)
    void* userdata_val{nullptr};    // +08h (tag 4)
    std::vector<LuaVariantPair> table_entries{}; // +08h (tag 5)
};

struct MissionLuaResultSink {
    std::uint32_t vptr{0x00D0E798};       // +00h
    void* lua_machine{nullptr};           // +04h
    std::vector<LuaVariantRecord> items{}; // +08h..+10h
};

struct MissionLuaPendingResults {
    std::uint32_t vptr{0x00D0E6F0};       // +00h
    MissionLuaResultSink* sink{nullptr};  // +04h
    int neg_count{0};                     // +08h
    bool armed{false};                    // +0Ch
};

struct MissionLuaDeferredCall {
    std::string self_key{};               // +00h..+07h
    std::string func_name{};              // +08h..+0Fh
    std::vector<LuaVariantRecord> args{}; // +10h..+1Bh
    std::uint32_t stack_first{0};         // +1Ch
    std::uint32_t stack_last{0};          // +20h
};

struct MissionLuaCallQueueNode {
    MissionLuaCallQueueNode* next{nullptr}; // +00h
    MissionLuaCallQueueNode* prev{nullptr}; // +04h
    MissionLuaDeferredCall payload{};       // +08h..+2Bh
};

struct MissionLuaCallQueue {
    std::uint32_t alloc_tag{0};                 // +00h
    MissionLuaCallQueueNode* head{nullptr};     // +04h (sentinel node)
    std::uint32_t size{0};                      // +08h
};

struct MissionLuaHostState {
    std::uint32_t vptr{0x00D0E7A8};             // +00h
    void* lua_machine{nullptr};                 // +04h
    MissionLuaCallQueue deferred_calls{};       // +08h..+10h
};

struct VfsFileBlockScope {
    std::uint32_t vptr{0x00D0E6EC};             // +00h
    int saved_depth{0};                         // +04h
};

struct MissionLuaEnvironmentHost {
    virtual ~MissionLuaEnvironmentHost() = default;
    // 006B87C0 LuaMachine scalar deleting destructor (vslot 0)
    virtual void delete_lua_machine(void* machine) = 0;
    // VFS open stream (vslot +04h(path, 2)) and read all bytes
    virtual bool read_vfs_file(const std::string& path, std::vector<char>& out_bytes) = 0;
    // 006B89F0 BSP_LuaMachine_RunChunk
    virtual void run_lua_chunk_006b89f0(
        void* machine,
        const char* data,
        std::size_t size,
        const char* name,
        MissionLuaResultSink* results,
        std::string& out_error,
        int mode) = 0;
    // 00BDEF90 BSP_VfsManager_AppendLuaScriptOverrides
    virtual void append_lua_script_overrides_00bdef90(
        const std::string& path,
        std::vector<std::string>& out_variants) = 0;
    // 00BDC9B0 BSP_VFS_LeaveFileBlock
    virtual int current_vfs_block_depth() const = 0;
    virtual void leave_vfs_file_block_00bdc9b0(const std::string& empty_tag) = 0;
    // 008855B0 format single LuaVariantRecord into text
    virtual void append_variant_as_text_008855b0(
        const LuaVariantRecord& rec,
        std::string& out_text) = 0;
    // Lua stack operations for 00885DA0 / 00886E00 / 00887220 / 008874C0:
    virtual void lua_push_number_006b80e0(float val) = 0;
    virtual void lua_push_string_006b8120(const char* str) = 0;
    virtual void lua_push_boolean_006b80c0(bool val) = 0;
    virtual void lua_push_nil_006b8130() = 0;
    virtual void lua_push_entity_from_this_table(std::int16_t entity_id) = 0;
    virtual void lua_create_table_006b84b0() = 0;
    virtual void lua_push_table_pair_00885a70(const LuaVariantPair& pair) = 0;
    virtual int lua_get_top_006b7e70() = 0;
    virtual void lua_pop_006b85f0(int count) = 0;
    virtual int lua_stack_type_006b7ec0(int index) = 0;
    virtual bool lua_to_boolean_006b8000(int index) = 0;
    virtual void* lua_to_userdata_006b80a0(int index) = 0;
    virtual float lua_to_float_006b8020(int index) = 0;
    virtual const char* lua_to_string_006b8060(int index) = 0;
    virtual std::vector<LuaVariantPair> lua_read_table_entries(int index, int next_depth, int max_depth) = 0;
    virtual void lua_get_caller_info_006b86b0(int level) = 0;
    // 00887750 BSP_MissionLuaHost_CallNamed
    virtual void call_named_00887750(
        MissionLuaHostState& host,
        const char* self_key,
        const char* func_name,
        const void* args,
        std::uint32_t stack_first,
        std::uint32_t stack_last,
        MissionLuaResultSink* results,
        bool force) = 0;
};

// 008844F0 BSP_MissionLuaHost_ReleaseMachine
// __fastcall(ECX = host), RET. If `host.lua_machine != nullptr`, invokes vslot 0(1)
// and zeroes `host.lua_machine`.
void mission_lua_host_release_machine_008844f0(
    MissionLuaHostState& host,
    MissionLuaEnvironmentHost& env);

// 00885110 BSP_MissionLuaHost_RunScriptFile
// __thiscall(ECX = host, const char* path), RET 4. Opens `path` via VFS mode 2;
// when valid (`vslot_18()`), reads all `vslot_30()` bytes and executes via 006B89F0.
void mission_lua_host_run_script_file_00885110(
    MissionLuaHostState& host,
    const char* path,
    MissionLuaEnvironmentHost& env);

// 00885350 BSP_VfsFileBlockScope_Destruct
// __fastcall(ECX = scope), RET. Sets vptr 00D0E6EC; if `scope.saved_depth < vfs_depth`,
// calls 00BDC9B0(`""`).
void vfs_file_block_scope_destruct_00885350(
    VfsFileBlockScope& scope,
    MissionLuaEnvironmentHost& env);

// 00885A30 BSP_MissionLuaResultSink_AppendAllAsText
// __thiscall(ECX = sink, NativeString* out), RET 4. Walks `sink.items` (stride 14h)
// and calls 008855B0 on each.
void mission_lua_result_sink_append_all_as_text_00885a30(
    const MissionLuaResultSink& sink,
    std::string& out_text,
    MissionLuaEnvironmentHost& env);

// 00885DA0 BSP_MissionLua_PushArgumentRecord
// __thiscall(ECX = record, LuaMachine* machine), RET 4. Switches on `record.tag` (0..6).
void mission_lua_push_argument_record_00885da0(
    const LuaVariantRecord& record,
    MissionLuaEnvironmentHost& env);

// 00885FB0 BSP_MissionLuaHost_RunScriptWithVariants
// __thiscall(ECX = host, const char* path, bool run_variants), RET 8.
// Calls 00885110(path), and when `run_variants` is true queries 00BDEF90 and runs
// 00885110 on each override path (substituting `""` for null buffer).
void mission_lua_host_run_script_with_variants_00885fb0(
    MissionLuaHostState& host,
    const char* path,
    bool run_variants,
    MissionLuaEnvironmentHost& env);

// 00886E00 BSP_MissionLua_BuildVariantFromStack
// __thiscall(ECX = sink, LuaVariantRecord* out, int stack_index, int depth, int max_depth), RET 10h.
// Guarded by `depth < max_depth`; converts Lua value at `stack_index` into `*out`.
void mission_lua_build_variant_from_stack_00886e00(
    MissionLuaResultSink& sink,
    LuaVariantRecord& out,
    int stack_index,
    int depth,
    int max_depth,
    MissionLuaEnvironmentHost& env);

// 00887220 BSP_MissionLua_CollectStackResults
// __thiscall(ECX = sink, int count, int max_depth), RET 8.
// Collects top `count` stack values (or all from 1..top when `count < 1`) via 00886E00.
void mission_lua_collect_stack_results_00887220(
    MissionLuaResultSink& sink,
    int count,
    int max_depth,
    MissionLuaEnvironmentHost& env);

// 008874C0 BSP_MissionLuaPendingResults_Destruct
// __fastcall(ECX = pending), RET. Sets vptr 00D0E6F0; when `armed`, calls
// `00887220(sink, -1 - neg_count, 4)` and `006B85F0(-neg_count)`.
void mission_lua_pending_results_destruct_008874c0(
    MissionLuaPendingResults& pending,
    MissionLuaEnvironmentHost& env);

// 00887560 BSP_MissionLuaDeferredCall_CopyArguments
// __thiscall(ECX = dst_vec, const src_vec*), RET 4. Clears `dst_vec`, reserves
// `src_vec.size()`, and deep-copies each `LuaVariantRecord`.
std::vector<LuaVariantRecord>* mission_lua_deferred_call_copy_arguments_00887560(
    std::vector<LuaVariantRecord>& dst_vec,
    const std::vector<LuaVariantRecord>& src_vec);

// 008875D0 BSP_MissionLuaResultSink_Construct
// __fastcall(ECX = sink), RET. Sets vptr 00D0E798 and zeroes `+04h..+10h`.
void mission_lua_result_sink_construct_008875d0(MissionLuaResultSink& sink) noexcept;

// 008875F0 BSP_LuaScriptOverrideList_Construct
// __thiscall(ECX = sink, void* machine), RET 4. Sets vptr 00D0E798, zeroes `+04h..+10h`,
// stores `machine` at `+04h`, and returns `&sink`.
MissionLuaResultSink* lua_script_override_list_construct_008875f0(
    MissionLuaResultSink& sink,
    void* lua_machine) noexcept;

// 00887670 BSP_LuaVariantVector_Tidy_00887670
// __fastcall(ECX = sink), RET. Sets vptr 00D0E798 and clears/frees `sink.items`.
void lua_variant_vector_tidy_00887670(MissionLuaResultSink& sink) noexcept;

// 008876B0 BSP_MissionLuaDeferredCall_Destruct
// __fastcall(ECX = deferred_call), RET. Clears `args`, `func_name`, and `self_key`.
void mission_lua_deferred_call_destruct_008876b0(MissionLuaDeferredCall& call) noexcept;

// 00887B30 BSP_MissionLuaHost_CallNamedForced
// __thiscall(ECX = host, self_key, name, args, stack_first, stack_last), RET 14h.
// Forwards to `00887750(self_key, name, args, stack_first, stack_last, nullptr, true)`.
void mission_lua_host_call_named_forced_00887b30(
    MissionLuaHostState& host,
    const char* self_key,
    const char* func_name,
    const void* args,
    std::uint32_t stack_first,
    std::uint32_t stack_last,
    MissionLuaEnvironmentHost& env);

// 00887C30 BSP_MissionLuaCallQueue_InsertNode
// __thiscall(ECX = queue, next, prev, value), RET 0Ch. Allocates 2Ch-byte node and
// copy-constructs `value` at `+08h`.
MissionLuaCallQueueNode* mission_lua_call_queue_insert_node_00887c30(
    MissionLuaCallQueueNode* next_node,
    MissionLuaCallQueueNode* prev_node,
    const MissionLuaDeferredCall& value);

// 00887D90 BSP_MissionLuaCallQueue_Clear
// __fastcall(ECX = queue), RET. Reseats sentinel `head` links to itself, sets `size = 0`,
// and destroys/frees every node in the chain.
void mission_lua_call_queue_clear_00887d90(MissionLuaCallQueue& queue) noexcept;

// 00888300 BSP_MissionLuaHost_Destruct
// __fastcall(ECX = host), RET. Sets vptr 00D0E7A8, deletes `lua_machine` (+04h) if
// non-null, clears `deferred_calls` (00887D90), and frees sentinel `head`.
void mission_lua_host_destruct_00888300(
    MissionLuaHostState& host,
    MissionLuaEnvironmentHost& env);

struct LuaTableEntry {
    std::string key{};
    float num_value{0.0f};
    bool is_light_userdata{false};
    void* userdata_ptr{nullptr};
};

struct MissionEntityCandidate {
    bool flag_5c{true};        // +5Ch != 0
    bool flag_5d{false};       // +5Dh == 0
    bool flag_5e{false};       // +5Eh == 0
    bool flag_60{false};       // +60h == 0
    bool is_entity_kind_1{true}; // vslot +5Ch(1)
    const char* name{nullptr}; // vslot +10h()
};

struct MissionEntityBucketTable {
    // Indexed by `kind_index + 6` (where `kind_index` is in `-6..0x5A`).
    std::vector<MissionEntityCandidate*> buckets[103]{};
};

// 00888760 BSP_LuaObject_ReadVector3
// __fastcall(ECX = out float[3], EDX = LuaObject* table), RET (leaves 1 float arg on stack).
// Iterates table keys via 00B67080 / 00B67190, matching `"x"` (00CEB488), `"y"` (00D045F8),
// and `"z"` (00CFD718), writes `out[0..2]`, and returns `out`.
Vec3* lua_object_read_vector3_00888760(
    Vec3* out,
    const std::vector<LuaTableEntry>& table_entries) noexcept;

// 008889C0 BSP_LuaValue_IsEntityHandle
// __fastcall(ECX = LuaObject* val), RET. Checks `IsTable` (00B661B0), looks up `"Ptr"`
// (00CFAD08) via 00B67800, verifies `IsLightUserdata` (00B66160), converts via `ToUserdata`
// (00B662D0), and checks `entity->vslot_5C(1) != 0`.
bool lua_value_is_entity_handle_008889c0(
    bool is_table,
    const std::vector<LuaTableEntry>& table_entries) noexcept;

// 00888D20 BSP_LuaTable_GetPtrField
// __fastcall(ECX = LuaObject* table), RET. Looks up `"Ptr"` (00CFAD08) via 00B67800
// and returns `ToUserdata` (00B662D0).
void* lua_table_get_ptr_field_00888d20(
    const std::vector<LuaTableEntry>& table_entries) noexcept;

// 008890F0 BSP_Game_IsScriptLoadActive
// __fastcall(ECX = game), RET. Returns `*(int*)(game + 644h) > 0`.
bool game_is_script_load_active_008890f0(int script_load_depth_644) noexcept;

// 0088B1B0 BSP_MissionEntity_FindByName
// __stdcall(int len, char* data), RET 8. Walks the 14 active entity kind buckets
// (0, 9, 18, 19, 20, 21, 22, 59, 62, 63, 64, 65, 68, 71) in `game+19CCh`, filters
// by flags `+5Ch != 0 && +5Dh == 0 && +60h == 0 && +5Eh == 0`, and compares `vslot_10()`
// via `_stricmp` (or empty-string match when `data == nullptr`).
MissionEntityCandidate* mission_entity_find_by_name_0088b1b0(
    const MissionEntityBucketTable& table,
    const char* query_name) noexcept;

// 0088B3D0 BSP_NativeString_ReleaseStorage_0088b3d0
// __fastcall(ECX = NativeString*), RET. If `data (+04h) != nullptr`, returns block
// `(data, len + 1, 1)` to SizedStoragePool (00419CC0 / 00BD1510).
void native_string_release_storage_0088b3d0(
    std::uint32_t len,
    void* data,
    DamageableClassHost& pool_host);

} // namespace bsp::landscape_lua_seg55_argon
