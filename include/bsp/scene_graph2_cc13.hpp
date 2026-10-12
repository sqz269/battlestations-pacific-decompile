#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <new>
#include <string>
#include <vector>

// Partition segments 88 (00B6D3C0..00B80ED0) and 91 (00B8A2B0..00B999A0): the second
// scene-graph pass (packet cc13-scene2). Evidence, original ABIs and uncertainty:
// docs/CC13_SCENE_2.md. Names are hypotheses, not recovered symbols.
//
// These are new C++ interfaces over host data, not drop-in binary replacements. Image
// routines that these bodies call are reached through the abstract service interfaces
// below, each virtual named by the address it stands for; nothing is stubbed. x87
// arithmetic is evaluated in double and rounded to float wherever the native code
// stores a float, which approximates the extended-precision intermediates.

namespace bsp::cc13_scene2 {

// ---------------------------------------------------------------------------
// Engine arrays. Every array here has the header {+00 data, +04 count, +08 capacity}
// (signed 32-bit count and capacity). The native bodies allocate with operator new
// (00BF55BE) and release with _free (00BF6989); the host model pairs ::operator new
// with ::operator delete. Element copies are bitwise (the image copies dwords).
template <class T>
struct NativeArray {
    T* data = nullptr;
    std::int32_t count = 0;
    std::int32_t capacity = 0;
};

// The "Allocate" shape (00B6DEA0 and five identical bodies): capacity = n is stored
// FIRST, then a buffer of n elements is allocated, the live elements are copied, the old
// buffer is freed and the new one published. No guard against n < count.
template <class T>
void array_reallocate_exact(NativeArray<T>& a, std::int32_t n) {
    a.capacity = n;
    T* fresh = static_cast<T*>(::operator new(static_cast<std::size_t>(n) * sizeof(T)));
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(a.count); ++i)
        std::memcpy(&fresh[i], &a.data[i], sizeof(T));
    if (a.data) ::operator delete(a.data);
    a.data = fresh;
}

// The "Grow" shape (00B6DEF0 and nine identical bodies): capacity*2 + 2, then as above.
template <class T>
void array_grow_double_plus_two(NativeArray<T>& a) {
    array_reallocate_exact(a, a.capacity * 2 + 2);
}

// The "PushBack" shape (00B6E580 and nine identical bodies): when count == capacity the
// grow above is inlined; then the element is copied to data[count] and count increments.
template <class T>
void array_push_back_grow(NativeArray<T>& a, const T& value) {
    if (a.count == a.capacity) array_grow_double_plus_two(a);
    std::memcpy(&a.data[a.count], &value, sizeof(T));
    ++a.count;
}

// The "Reserve" shape (00B6E610 and five identical bodies): grows only when the
// capacity, compared UNSIGNED, is below n.
template <class T>
void array_reserve_unsigned(NativeArray<T>& a, std::uint32_t n) {
    if (static_cast<std::uint32_t>(a.capacity) < n) array_reallocate_exact(a, static_cast<std::int32_t>(n));
}

// The "PushBack through a separate reserve" shape. `rule` computes the requested
// capacity from the current one; `reserve` is the image reserve routine named per entry.
using CapacityRule = std::int32_t (*)(std::int32_t capacity);
inline std::int32_t capacity_double_min_one(std::int32_t c) { const std::int32_t n = c * 2; return n < 2 ? 1 : n; }
inline std::int32_t capacity_double_min_four(std::int32_t c) { const std::int32_t n = c * 2; return n < 5 ? 4 : n; }
inline std::int32_t capacity_plus_four_min_eight(std::int32_t c) { const std::int32_t n = c + 4; return n < 9 ? 8 : n; }
template <class T>
void array_push_back_via_reserve(NativeArray<T>& a, const T& value, CapacityRule rule,
    const std::function<void(NativeArray<T>&, std::int32_t)>& reserve) {
    if (a.count == a.capacity) reserve(a, rule(a.capacity));
    std::memcpy(&a.data[a.count], &value, sizeof(T));
    ++a.count;
}

// 00B7D6A0 shape: n clamped to at least 1; grows when capacity < n (signed); the old
// buffer is freed unconditionally and capacity is stored LAST.
template <class T>
void array_reserve_min_one(NativeArray<T>& a, std::int32_t n) {
    if (n < 1) n = 1;
    if (a.capacity >= n) return;
    T* fresh = static_cast<T*>(::operator new(static_cast<std::size_t>(n) * sizeof(T)));
    for (std::int32_t i = 0; i < a.count; ++i) std::memcpy(&fresh[i], &a.data[i], sizeof(T));
    ::operator delete(a.data);
    a.data = fresh;
    a.capacity = n;
}

// 00B7D820 shape: reserve(n) when capacity < n, zero-fill new elements, set count = n.
template <class T>
void array_resize_zero_fill(NativeArray<T>& a, std::int32_t n,
    const std::function<void(NativeArray<T>&, std::int32_t)>& reserve) {
    if (a.capacity < n) reserve(a, n);
    for (std::int32_t i = a.count; i < n; ++i) std::memset(&a.data[i], 0, sizeof(T));
    a.count = n;
}

// The "Tidy"/"Destroy" shape (00B77DF0 and twelve identical bodies): the element-typed
// resize routine is called with 0, then the data pointer is freed (the pointer and
// capacity words are left as they were).
template <class T>
void array_destroy(NativeArray<T>& a, const std::function<void(NativeArray<T>&, std::int32_t)>& resize) {
    resize(a, 0);
    ::operator delete(a.data);
}

// Per-address entry points. Pointer arrays (4-byte cells):
using PointerArray = NativeArray<void*>;
void allocate_pointer_array(std::uint32_t address, PointerArray& a, std::int32_t n);
void grow_pointer_array(std::uint32_t address, PointerArray& a);
void push_back_pointer_array(std::uint32_t address, PointerArray& a, void* value);
void reserve_pointer_array(std::uint32_t address, PointerArray& a, std::uint32_t n);
// Returns false when `address` is not one of the identical bodies listed in the source.
bool is_pointer_array_routine(std::uint32_t address) noexcept;

// 28h-byte animation key record (layout: bsp/scene_graph_cc13.hpp AnimKey).
struct AnimKeyRecord { std::uint32_t words[10]; };
struct PairRecord { std::uint32_t first, second; };
struct DumpRecord { std::uint32_t words[3]; };
// 00B77490: append to the key array (+0Ch keys of a track) through 00B8A5B0-style reserve
// (image BSP_AnimationKeys_Reserve, passed as `reserve`), capacity rule double/min 1.
void push_back_animation_key_00b77490(NativeArray<AnimKeyRecord>& a, const AnimKeyRecord& key,
    const std::function<void(NativeArray<AnimKeyRecord>&, std::int32_t)>& reserve);
// 00B775F0: the same for 8-byte pairs (image BSP_PostprocessPairs_Reserve).
void push_back_pair_00b775f0(NativeArray<PairRecord>& a, const PairRecord& value,
    const std::function<void(NativeArray<PairRecord>&, std::int32_t)>& reserve);
// 00B7D6A0 / 00B7D820 / 00B7DA10: the resource-dump entry array (12-byte rows).
void reserve_dump_entries_00b7d6a0(NativeArray<DumpRecord>& a, std::int32_t n);
void resize_dump_entries_00b7d820(NativeArray<DumpRecord>& a, std::int32_t n);
void push_back_dump_entry_00b7da10(NativeArray<DumpRecord>& a, const DumpRecord& value);
// Pointer-array push-backs that call a separately named reserve (rule per address):
// 00B6E940, 00B775B0, 00B7BC00, 00B7BE90, 00B8A380, 00B8A430 (double, min 1),
// 00B73800 (double, min 4), 00B7D9D0 (+4, min 8).
void push_back_pointer_via_reserve(std::uint32_t address, PointerArray& a, void* value,
    const std::function<void(PointerArray&, std::int32_t)>& reserve);

// ---------------------------------------------------------------------------
// Reference counting. Ref-counted objects keep the count at +04h; reaching zero calls
// vtable slot 0 (the deleting destructor) with ECX = object and no stack argument.
struct RefCounted {
    virtual void destroy_slot0() = 0;   // vtable +00h
    std::int32_t refcount = 0;          // +04h (InterlockedIncrement/Decrement)
protected:
    ~RefCounted() = default;
};
// 00B6ECC0 (and 00B723C0, 00B74E40, 00B77800, 00B7BCC0): release *slot when non-null
// (slot 0 on reaching zero) and clear it. Returns the slot address (EAX = ECX).
RefCounted** release_ref_slot(RefCounted** slot);
// 00B74ED0: node +174h retained reference assignment. No-op when unchanged; otherwise
// store, retain the new one, then release the old one.
void assign_retained_ref_00b74ed0(RefCounted*& slot_174, RefCounted* value);
// 00B77ED0: camera +438h target assignment: release the old one (clearing the slot),
// store the new one, then retain it when non-null. Unlike 00B74ED0 there is no equality
// test, so assigning the current target releases it before retaining it again.
void assign_target_ref_00b77ed0(RefCounted*& slot_438, RefCounted* value);

// Owned virtual objects: 00B763D0 and 00B8A2B0 call vtable +04h with 1 (scalar
// deleting destructor) on *slot when non-null and clear it.
struct OwnedVirtual {
    virtual void scalar_delete_slot1(std::uint32_t flags) = 0;  // vtable +04h
protected:
    ~OwnedVirtual() = default;
};
void delete_owned_slot(OwnedVirtual** slot);

// ---------------------------------------------------------------------------
// Scene type descriptors. Each class has a static descriptor: its own id, the ids of
// every ancestor (copied from the parent descriptor), then the class-name word. Ids come
// from one shared counter (the singleton at [0109DB7C], next id at +04h).
struct SceneTypeDescriptor {
    std::vector<std::uint32_t> ids;  // own id first
    std::string name;
};
struct SceneTypeCounter { std::uint32_t next_id = 0; };

// The guarded initializer body shared by 00B7BFD0, 00B7C0C0, 00B7C110, 00B90FB0,
// 00B911D0, 00B93C20 and 00B93C70: when the process guard is clear, set it, store the
// name, run the parent initializer, copy the parent's ids behind slot 0, then take the
// next id. `initialize_parent` stands for the image call (00B6F110, 00B7BFD0, 00B90E70,
// 00B93BD0 per entry).
void initialize_derived_type(bool& guard, SceneTypeDescriptor& target, const char* name,
    const std::function<void()>& initialize_parent, const SceneTypeDescriptor& parent,
    SceneTypeCounter& counter);
// 00B78170 / 00B781F0: the animator initializers inline their parent (c3dNodeAnimator,
// guard 010900C5, descriptor 010900E0) behind its own guard; the parent's parent is cRoot
// (00BEA780 initializes it; its id word is [0109DB84]).
struct AnimatorTypeState {
    bool animator_guard_010900c5 = false;
    SceneTypeDescriptor animator_010900e0;   // {own, root}, "c3dNodeAnimator"
    bool camera_animator_guard_010900c6 = false;
    bool optimized_guard_010900c7 = false;
};
void initialize_camera_animator_type_00b78170(AnimatorTypeState& s, SceneTypeDescriptor& target,
    const std::function<void()>& initialize_root, const SceneTypeDescriptor& root, SceneTypeCounter& counter);
void initialize_optimized_animator_type_00b781f0(AnimatorTypeState& s, SceneTypeDescriptor& target,
    const std::function<void()>& initialize_root, const SceneTypeDescriptor& root, SceneTypeCounter& counter);

// IsType slot (+0Ch): scan the descriptor's id words. The native returns AL = 1 on a hit
// and otherwise the address of an unrelated global whose low byte is 0 (AL = 0).
bool scene_type_has_id(const SceneTypeDescriptor& d, std::uint32_t token) noexcept;

// Address table of the type-slot leaves. Kind `id` returns descriptor word 0 (vtable +08h,
// "GetRuntimeTypeId"); kind `name` returns the class-name word (vtable +14h). The +14h
// slots were named GetTypeDescriptor by cc13-files1; the descriptor layout shows they
// return the name word (see the doc). Kind `is_type` is the +0Ch scan.
enum class SceneTypeSlotKind { id, name, is_type };
struct SceneTypeSlot {
    std::uint32_t address;
    std::uint32_t descriptor;    // address of descriptor word 0
    std::uint32_t id_count;      // number of id words scanned by IsType
    SceneTypeSlotKind kind;
    const char* class_name;      // from the initializer strings; provisional where noted
};
const SceneTypeSlot* find_scene_type_slot(std::uint32_t address) noexcept;
const SceneTypeSlot* scene_type_slots(std::size_t& count) noexcept;

// Cast helpers 00B763F0 / 00B93600: return `object` when it is non-null and its IsType
// slot accepts the current type token, else null. `is_type` is the object's vtable +0Ch.
template <class T>
T* checked_cast(T* object, std::uint32_t token, const std::function<bool(T&, std::uint32_t)>& is_type) {
    if (object && is_type(*object, token)) return object;
    return nullptr;
}

// ---------------------------------------------------------------------------
// c3dNode fields used by this pass (host view).
struct SceneNode {
    float vector_08[4]{};          // +08h..+14h
    SceneNode* parent = nullptr;   // +30h
    SceneNode* first_child = nullptr;  // +34h
    std::int32_t child_count = 0;  // +38h (00B6D980 returns it)
    SceneNode* next_sibling = nullptr; // +3Ch
    std::uint32_t word_50 = 0;     // +50h
    std::string name;              // +54h NativeString {length, chars}
    std::uint32_t flags_5c = 0;    // +5Ch: 2 world valid, 8 inverse world valid
};
constexpr std::uint32_t kNodeWorldValid = 0x2;
constexpr std::uint32_t kNodeInverseValid = 0x8;

// Image routines and virtual slots reached from the node, camera and scene bodies.
struct SceneNodeServices {
    virtual void refresh_world_00b6db70(SceneNode& node) = 0;          // BSP_Transform_RefreshWorldMatrix
    virtual void node_slot_48(SceneNode& node) = 0;                    // node vtable +48h
    virtual void node_apply_animation_slot_24(SceneNode& node, std::int32_t index, float time,
        std::int32_t flag) = 0;                                        // node vtable +24h
    // 00B78670: node+130h animator -> +2Ch registry; map lookup of the name, value +14h
    // of the tree node, or -1.
    virtual std::int32_t find_animation_index_00b78670(SceneNode& node, const std::string& name) = 0;
    // 00B63B30 then 004134F0: invert the world matrix (+F0h) into +60h.
    virtual void store_inverse_world_00b63b30(SceneNode& node) = 0;
    // 00B6E750 (reconstructed in scene_graph_cc13.cpp): child search under `node`.
    virtual SceneNode* find_child_by_name_00b6e750(SceneNode& node, const std::string& name, bool recursive) = 0;
protected:
    ~SceneNodeServices() = default;
};

// 00B6D820: clear every child's parent along the sibling chain, then first child and count.
void detach_all_children_00b6d820(SceneNode& node);
// 00B6DC00: copy +08h..+14h out.
void copy_vector_08_00b6dc00(const SceneNode& node, float out[4]);
// 00B6DCA0: store +50h.
void set_word_50_00b6dca0(SceneNode& node, std::uint32_t value);
// 00B6E110: look the name up in the animator registry; when found call slot +24h(index, time, 1).
void apply_animation_by_name_00b6e110(SceneNode& node, const std::string& name, float time, SceneNodeServices& s);
// 00B6E380: refresh the world matrix when not valid, call slot +48h, then recurse into the
// children in sibling order.
void refresh_world_recursive_00b6e380(SceneNode& node, SceneNodeServices& s);
// 00B6FC70: rebuild the inverse world (+60h) when bit 8 is clear (refreshing the world
// first when bit 2 is clear), set bit 8. Returns true when it was rebuilt (the native
// returns the address of +60h either way).
bool ensure_inverse_world_00b6fc70(SceneNode& node, SceneNodeServices& s);
// 00B72080: leaves refresh their world matrix when bit 2 is clear; nodes with children
// recurse instead (renamed from UpdateWorldIfRoot: the test is 00B6D980 = child count).
void refresh_leaf_worlds_00b72080(SceneNode& node, SceneNodeServices& s);

// Scene roots: +0Ch first root node, linked through +3Ch.
struct SceneRoots { SceneNode* first = nullptr; };
// NativeString equality used by the scene searches: equal length and _stricmp == 0.
bool native_name_equal(const std::string& a, const std::string& b) noexcept;
// 00B72270: roots whose name matches; then, when `recursive`, 00B6E750(root, name, 1) per root.
SceneNode* find_node_by_name_00b72270(const SceneRoots& scene, const std::string& name, bool recursive,
    SceneNodeServices& s);
// 00B72300: roots whose name matches; then 00B6E750(root, name, 1) for every root whose
// name differs from `excluded` (case-insensitive; the length is not compared first here).
SceneNode* find_node_by_name_excluding_00b72300(const SceneRoots& scene, const std::string& name,
    const std::string& excluded, SceneNodeServices& s);
// 00B72000 / 00B72030: for one node / every root, build the quadtree (00B91D70) when the
// node's IsType accepts the quadtree token (00B916F0 returns [010903E0]).
struct QuadTreeBuildServices {
    virtual std::uint32_t quadtree_type_00b916f0() = 0;
    virtual bool node_is_type_slot_0c(SceneNode& node, std::uint32_t token) = 0;
    virtual void build_quadtree_00b91d70(SceneNode& node) = 0;
protected:
    ~QuadTreeBuildServices() = default;
};
void build_quadtree_if_type_00b72000(SceneNode& node, QuadTreeBuildServices& s);
void build_quadtrees_for_roots_00b72030(const SceneRoots& scene, QuadTreeBuildServices& s);

// ---------------------------------------------------------------------------
// Vertex bounds over the sub-meshes of a mesh object (array +64h, count +7Ch). Each
// sub-mesh reports its declaration (vtable +24h), locks (vtable +10h(0,0,0)), reports its
// vertex count (vtable +20h, re-read every iteration) and unlocks (vtable +14h). Only
// sub-meshes whose usage (0,0) type (00B47C20) is 2 contribute; positions are read at
// the usage offset (00B47C40) with the declaration stride +CCh.
struct VertexStreamView {
    std::int32_t position_type = 0;  // 00B47C20(decl, 0, 0)
    std::int32_t stride = 0;         // decl +CCh
    std::int32_t offset = 0;         // 00B47C40(decl, 0, 0)
    const std::uint8_t* vertices = nullptr;  // lock result
    std::int32_t vertex_count = 0;
};
constexpr float kBoundsInitialMin = 1e11f;    // [00D21528] 51BA43B7
constexpr float kBoundsInitialMax = -1e11f;   // [00D62D54] D1BA43B7
struct Aabb { float min[3]; float max[3]; };
// 00B732D0: cached AABB: byte +94h, min +98h, max +A4h. A NaN coordinate never replaces
// a bound. Returns false (no change) when the cache byte is already set.
bool compute_cached_aabb_00b732d0(bool& valid_94, Aabb& out_98, const std::vector<VertexStreamView>& streams);
// 00B734D0: cached bounding sphere: byte +80h, centre +84h, radius +90h. The centre is
// min + (max - min)*0.5 and the radius sqrt(hx^2 + hy^2 + hz^2) of the half extents
// (00BF7030 is the CRT sqrt).
struct Sphere { float centre[3]; float radius; };
bool compute_cached_sphere_00b734d0(bool& valid_80, Sphere& out_84, const std::vector<VertexStreamView>& streams);
// 00B72B50: clear the sphere cache (+80h byte, +84h..+90h).
void reset_sphere_cache_00b72b50(bool& valid_80, Sphere& sphere_84);
// 00B732B0: tree-group slot address = this + (index + 1) * 10h (as a byte offset).
std::uint32_t tree_group_slot_offset_00b732b0(std::int32_t index) noexcept;
// 00B74410: zero four dwords.
void zero_vector4_00b74410(float v[4]);

// ---------------------------------------------------------------------------
// Optimized animator (vtable 00D62EB0): +30h enabled byte, +34h baked track.
// Baked track: +08h nodes per frame, +0Ch channel records (18h bytes: +00 duration
// float, +0Ch frame data, +10h dword count).
struct BakedChannel {
    float duration = 0;            // +00h
    const float* frames = nullptr; // +0Ch: frames * nodes 30h-byte 3x4 matrices
    std::int32_t word_count = 0;   // +10h
};
struct BakedTrack {
    std::int32_t nodes_per_frame = 0;  // +08h
    std::vector<BakedChannel> channels;
};
// 00B92500: matrix index (in 30h records) of `channel` at `time`:
// u = fmod(time / duration, 1) (both rounded to float; 00BF857A is the CRT fmod), frames =
// word_count / nodes - 1 (integer division), index = trunc(u * frames) * nodes.
std::int32_t baked_frame_index_00b92500(const BakedTrack& track, std::int32_t channel, float time);
// 00B75DE0: 4x4 row-major output from a 3x4 source stored column-major: out[r][c] =
// src[c*4 + r] for the 3x3 part, row 3 = src[3], src[7], src[11], column 3 = 0,0,0,1.
void matrix_from_transposed_3x4_00b75de0(float out[16], const float src[12]);
// 00B75E90 (vtable +18h): when enabled and a track is bound, compose the frame matrix of
// `channel` at `time` into `out` and return true; else false.
bool optimized_evaluate_pose_00b75e90(bool enabled, const BakedTrack* track, std::int32_t channel, float time,
    float out[16]);
// 00B75ED0 (vtable +1Ch): always false (RET 10h).
bool optimized_evaluate_channel_00b75ed0() noexcept;
// 00B92560: index of the channel whose name (+04h length, +08h chars) equals `name`
// (equal length and _stricmp), else -1.
std::int32_t find_baked_channel_00b92560(const std::vector<std::string>& names, const std::string& name);
// 00B77480: registry float: (+2Ch)->(+14h)[index].
float registry_float_00b77480(const std::vector<float>& values_14, std::int32_t index);

// ---------------------------------------------------------------------------
// Camera (vtable 00D62CF0) fields used here.
struct CameraView {
    float target_1a0[3]{};         // +1A0h look-at point
    float direction_1ac[3]{};      // +1ACh normalised (target - eye)
    float fov_1c4 = 0;             // +1C4h
    float clip_plane_2e0[4]{};     // +2E0h (world-space oblique clip plane)
    std::uint32_t flags_2f0 = 0;   // +2F0h: bit 4 frustum planes valid
    std::uint32_t word_190 = 0;    // +190h
    std::uint32_t word_194 = 0;    // +194h
};
// 00B6FE60 / 00B6FE70.
void camera_get_190_00b6fe60(const CameraView& c, std::uint32_t& out);
void camera_set_194_00b6fe70(CameraView& c, std::uint32_t value);

struct CameraServices {
    virtual void camera_slot_30_set_position(CameraView& c, const float eye[3]) = 0;  // vtable +30h
    virtual float vector_length_00419440(const float v[3]) = 0;                        // BSP_Vector3f_Length
    // 00B63F10 BSP_Matrix_BuildLookAt(ECX local matrix, EDX eye, target, up) into a stack
    // matrix, then 00B63B30 inverts it and vtable +34h receives the inverse.
    virtual void look_at_and_apply_slot_34(CameraView& c, const float eye[3], const float target[3],
        const float up[3]) = 0;
    virtual void set_local_position_00b6dab0(CameraView& c, const float position[3]) = 0;
    virtual void refresh_direction_00b70660(CameraView& c) = 0;
    // Frustum rebuild: 00B70490 view-projection, 00B653F0 extract/normalise into a stack
    // array, 00B658E0 assign with flags 7 into the plane set at +2F4h.
    virtual void rebuild_frustum_planes(CameraView& c) = 0;
    virtual std::uint32_t classify_point_00b65140(CameraView& c, const float p[3]) = 0;
    virtual std::uint32_t classify_aabb_00b65710(CameraView& c, const float box[6]) = 0;
    // As 00B6FC70 on the camera: world refresh (00B6DB70) when bit 2 is clear, then
    // 00B63B30/004134F0 inverse into +60h and bit 8, when bit 8 is clear.
    virtual void ensure_view_matrix(CameraView& c) = 0;
    // 00B65C30: transform a plane by the view matrix (+60h) in place.
    virtual void transform_plane_00b65c30(CameraView& c, float plane[4]) = 0;
protected:
    ~CameraServices() = default;
};
// 00B70200 (thiscall, RET 0Ch): clear flag bits 1B4h of +2F0h, call slot +30h(eye), copy
// `target` to +1A0h, store normalize(target - eye) at +1ACh (each difference, the length
// and its reciprocal rounded to float; a length <= 0 gives a zero vector), then the
// look-at matrix (eye, target, up) is inverted and passed to slot +34h.
void camera_set_eye_and_target_00b70200(CameraView& c, const float eye[3], const float target[3],
    const float up[3], CameraServices& s);
// 00B713D0: clear flag bits 1B4h, set the local position (00B6DAB0), refresh (00B70660).
void camera_set_local_position_00b713d0(CameraView& c, const float position[3], CameraServices& s);
// 00B714D0 / 00B71590: rebuild the frustum planes once (bit 4), then classify. The
// result is the EAX of the classifier (Ghidra shows void).
std::uint32_t camera_classify_point_00b714d0(CameraView& c, const float p[3], CameraServices& s);
std::uint32_t camera_classify_aabb_00b71590(CameraView& c, const float box[6], CameraServices& s);
// 00B70760: oblique near-plane projection. The +2E0h plane is copied, transformed into view
// space (00B65C30 with +60h, after ensuring the inverse world), then with
// q = (sgn(c.x)/P[0], sgn(c.y)/P[5], 1, (1 - P[10])/P[14]) and k = 1/dot(c, q) the third
// projection column becomes k*c: P[2], P[6], P[10], P[14]. sgn(0) = 0; NaN gives 0.
void camera_apply_oblique_clip_00b70760(CameraView& c, float projection[16], CameraServices& s);
// 00B70550: projection scale pair: out_x = {P[0], P[4], 0, 0}, out_y = {P[1], P[5], 0, 0}
// of the projection matrix (00B6FCF0). The tangent of f(fov*0.5) (00412E20) is computed and
// discarded (FSTP ST0 at 00B7057D).
void camera_projection_scale_00b70550(const float projection[16], float out_x[4], float out_y[4]);
// 00B705D0 (RET 20h): calls 00B70550 into two stack vectors and copies the eight words to
// eight separate out pointers.
void camera_projection_scale_words_00b705d0(const float projection[16], float* out[8]);

// Camera copy constructor 00B71D10 (thiscall(this, const cCamera& source), RET 4).
struct CameraCopyFields {
    std::uint8_t byte_17c = 0;
    std::uint32_t word_180 = 0, word_184 = 0, word_188 = 0;
    float float_18c = 0;
    std::uint32_t word_190 = 0, word_194 = 0, word_198 = 0;
    float zero_1b8[3]{};
    float projection_params_1c4[6]{};   // +1C4h..+1D8h (fov first)
    float float_1dc = 0;
    std::uint32_t flags_2f0 = 0;
    void* target_438 = nullptr;
    float tail_440[6]{};
};
struct CameraCopyServices {
    virtual void construct_node_00b6f5a0(void* self, const std::string& name) = 0;  // name via 00B6D800
    virtual void set_vtable(void* self, std::uint32_t vtable) = 0;                   // 00D62CF0
    virtual void construct_plane_set_00b659d0(void* self) = 0;                       // +2F4h
    virtual bool source_world_valid(const void* source) = 0;                         // +5Ch bit 2
    virtual void refresh_world_00b6db70(const void* source) = 0;
    virtual void set_world_matrix_00b6e870(void* self, const void* source) = 0;      // source +F0h
    virtual void refresh_direction_00b70660(void* self) = 0;
    // 00B6FCF0 on the source, then 004134F0 into +1E0h and +2A0h.
    virtual void copy_projection_00b6fcf0(void* self, const void* source) = 0;
    virtual void assign_name_00b6f960(void* self, const std::string& name) = 0;
protected:
    ~CameraCopyServices() = default;
};
// Steps in native order: node construct with the source name; vtable; +1B8h..+1C0h = 0;
// plane set; +438h = 0; refresh the source world when needed; flags &= FFFFFE4Bh; world
// from the source; direction refresh; projection copies; flags = (flags & FFFFFF4Bh) | 8;
// name assigned again; +180h/+184h = 0; +1C4h..+1D8h copied; flags = source flags &
// FFFFFE41h; then +198h, +188h, +190h, +18Ch, +194h, +1DCh, byte +17Ch, +440h..+454h.
void camera_copy_construct_00b71d10(void* self, CameraCopyFields& dst, const void* source,
    const std::string& source_name, const CameraCopyFields& src, CameraCopyServices& s);

// ---------------------------------------------------------------------------
// Pool-backed clones and scalar deleting destructors.
struct NodeCloneServices {
    virtual void* allocate_node_slot_00b6eb00() = 0;                       // pool 0108FF58
    virtual void* construct_node_00b6f5a0(void* slot, const std::string& name) = 0;
    virtual void copy_clone_base_state_00b6f150(const void* source, void* clone, std::uint32_t flags,
        void* parent) = 0;
protected:
    ~NodeCloneServices() = default;
};
// 00B6F8F0 (node vtable Clone, thiscall(source, flags, parent), RET 8): allocate, construct
// with the source name (+54h) when the slot is non-null, then copy the base state into the
// (possibly null) result; returns it.
void* clone_node_00b6f8f0(const void* source, const std::string& name, std::uint32_t flags, void* parent,
    NodeCloneServices& s);
// 00B725A0: allocate a draw-section slot (00B85EE0) and copy-construct (00B85EF0) from the
// source; null when the pool returns null.
void* clone_draw_section_00b725a0(const void* source, const std::function<void*()>& allocate,
    const std::function<void*(void*, const void*)>& copy_construct);
// 00B7C800 (cLight vtable +00h, thiscall(this, flags), RET 4): destroy (00B7C5B0), then
// return the slot to the node pool (00B6E670) when flags & 1; returns this.
void* light_scalar_deleting_destructor_00b7c800(void* self, std::uint32_t flags,
    const std::function<void(void*)>& destroy, const std::function<void(void*)>& return_slot);
// Parser and manager singleton bases (00B7D410, 00B7D450, 00B7D490, 00B7D4D0, 00B7D510,
// 00B7D550, 00B7D9A0): clear the instance global, restore the singleton-base vtable
// 00CE3818, free (00BF65AC) when flags & 1; return this.
struct SingletonBaseDelete {
    std::uint32_t address;
    std::uint32_t instance_global;
    const char* class_name;
};
const SingletonBaseDelete* find_singleton_base_delete(std::uint32_t address) noexcept;
void* singleton_base_scalar_delete(std::uint32_t address, void* self, std::uint32_t flags, void*& instance_global,
    std::uint32_t& vtable_word, const std::function<void(void*)>& free_block);

// 00B7C920: qsort comparator, size (+04h) descending: b.size - a.size (wrapping int).
std::int32_t compare_dump_size_descending_00b7c920(const DumpRecord& a, const DumpRecord& b) noexcept;

// ---------------------------------------------------------------------------
// Lua script cache. The global cache is the name vector 0108FF30 with its record vector
// 0108FF40; an instance cache keeps names at +00h and records at +10h.
struct LuaScriptCacheServices {
    // 00B6C990: ECX names, EDX cache names; true when equal.
    virtual bool names_equal_00b6c990(const std::vector<std::string>& names,
        const std::vector<std::string>& cache) = 0;
    virtual void clear_globals_00b6cf90() = 0;
    virtual void clear_cache_00b6d0a0(void* cache) = 0;
    // 00B6D1B0: ECX names, EDX records.
    virtual void build_compressed_00b6d1b0(const std::vector<std::string>& names, void* records) = 0;
protected:
    ~LuaScriptCacheServices() = default;
};
// 00B6D3C0 (ECX names): unless equal to the global names: clear the globals, assign
// (00506C80), and tail-jump to the build over the GLOBAL names and records.
void reset_global_scripts_unless_loaded_00b6d3c0(const std::vector<std::string>& names,
    std::vector<std::string>& global_names_0108ff30, void* global_records_0108ff40, LuaScriptCacheServices& s);
// 00B6D400 (thiscall(cache, names), RET 4): the instance twin; the build reads the
// ARGUMENT names (ECX = EDI) and the cache's records +10h.
void reload_script_cache_00b6d400(void* cache, std::vector<std::string>& cache_names, void* cache_records_10,
    const std::vector<std::string>& names, LuaScriptCacheServices& s);

// ---------------------------------------------------------------------------
// NativeString {+00 length, +04 chars} entries with a trailing word (12 bytes).
struct NativeStringEntry {
    std::string text;
    std::uint32_t word_8 = 0;
};
// 00B78E80 (and 00B78ED0, 00B7EEA0, 00B7EEF0, 00B7EF40, 00B7EF90): copy constructor. The
// destination starts {0, null}; unless it aliases the source the text is resized
// (0041DD40) and copied (_memcpy, skipped for length 0); the trailing word is copied.
void copy_string_entry(NativeStringEntry& dst, const NativeStringEntry& src);
// 00B78D20 (and 00B7F060, 00B7F0B0): pair constructor (key, value): as above, but the
// trailing word comes from the separate value argument.
void construct_string_pair(NativeStringEntry& dst, const std::string& key, std::uint32_t value);
// 00B78630 / 00B78650 (unwind helpers): when the chars pointer is non-null, return
// length + 1 bytes to the sized storage pool (00419CC0 singleton, 00BD1510 return).
void release_string_storage(std::uint32_t length, const void* chars,
    const std::function<void(const void*, std::uint32_t)>& return_block);
// 00B76230, 00B7CB00, 00B7CC50: operator delete thunks (00BF65AC).
void operator_delete_thunk(void* p, const std::function<void(void*)>& free_block);

// ---------------------------------------------------------------------------
// MSVC (Dinkumware) red-black trees: header {+00 comparator/allocator, +04 head, +08 size};
// nodes {+00 left, +04 parent, +08 right, value, colour byte, nil byte}. The head's parent
// is the root, its left the minimum and its right the maximum; an empty tree points all
// three at the head.
template <class V>
struct NativeTreeNode {
    NativeTreeNode* left = nullptr;
    NativeTreeNode* parent = nullptr;
    NativeTreeNode* right = nullptr;
    V value{};
    std::uint8_t colour = 0;
    std::uint8_t is_nil = 0;
};
template <class V>
struct NativeTree {
    NativeTreeNode<V>* head = nullptr;
    std::uint32_t size = 0;
};
// 00B762B0 _Min: follow left links until the next one is nil.
template <class V>
NativeTreeNode<V>* tree_minimum(NativeTreeNode<V>* node) {
    while (!node->left->is_nil) node = node->left;
    return node;
}
// 00B76360 _Max: the same over right links.
template <class V>
NativeTreeNode<V>* tree_maximum(NativeTreeNode<V>* node) {
    while (!node->right->is_nil) node = node->right;
    return node;
}
// Empty-tree initializer (00B77E10, 00B783B0, 00B79A50, 00B7E6C0, 00B7E700, 00B7E820,
// 00B7E850): allocate the head (00B77710, 00B7E000 or 00B7E050), mark it nil, point
// parent, left and right at itself, size 0. The Construct forms return the tree.
template <class V>
NativeTree<V>& tree_initialize_empty(NativeTree<V>& t, const std::function<NativeTreeNode<V>*()>& allocate_head) {
    NativeTreeNode<V>* h = allocate_head();
    t.head = h;
    h->is_nil = 1;
    h->parent = h;
    h->left = h;
    h->right = h;
    t.size = 0;
    return t;
}
// Clear (00B79710, 00B80170, 00B804D0): erase the subtree under the root (00B79590,
// 00B7F730, 00B7FF20), then reset head links and size (root first, then size, left, right).
template <class V>
void tree_clear(NativeTree<V>& t, const std::function<void(NativeTreeNode<V>*)>& erase_subtree) {
    erase_subtree(t.head->parent);
    t.head->parent = t.head;
    t.size = 0;
    t.head->left = t.head;
    t.head->right = t.head;
}
// Destroy (00B79990, 00B799D0, 00B79A10, 00B80BC0, 00B80C00, 00B80D90, 00B80DD0): erase
// [begin, end) (00B798C0, 00B80500, 00B805D0), free the head (00BF65AC), head = 0, size = 0.
template <class V>
void tree_destroy(NativeTree<V>& t,
    const std::function<void(NativeTree<V>&, NativeTreeNode<V>*, NativeTreeNode<V>*)>& erase_range,
    const std::function<void(void*)>& free_block) {
    erase_range(t, t.head->left, t.head);
    free_block(t.head);
    t.head = nullptr;
    t.size = 0;
}
// Node allocators 00B76450, 00B7D050, 00B7D0C0 (1Ch-byte nodes): count 0 allocates 0
// bytes; a count above FFFFFFFFh / 1Ch throws std::bad_alloc; else operator new(n * 1Ch).
void* allocate_tree_nodes_1c(std::uint32_t count, const std::function<void*(std::size_t)>& operator_new);
// 00B783E0: map<uint32,...>::find over nodes with the key at +0Ch (nil byte +21h): lower
// bound, then end when it is the head or key < node key. Returns {tree, node}.
template <class V>
NativeTreeNode<V>* tree_find_uint_key(const NativeTree<V>& t, std::uint32_t key,
    const std::function<std::uint32_t(const V&)>& key_of) {
    NativeTreeNode<V>* best = t.head;
    for (NativeTreeNode<V>* n = t.head->parent; !n->is_nil;) {
        if (key_of(n->value) < key) {
            n = n->right;
        } else {
            best = n;
            n = n->left;
        }
    }
    if (best == t.head || key < key_of(best->value)) return t.head;
    return best;
}
// Iterator pre-increment/decrement wrappers (00B776D0, 00B776F0, 00B77A80, 00B77DE0,
// 00B7D8B0, 00B7D8C0, 00B7D900, 00B7D910, 00B7DED0, 00B7DEE0, 00B7DF20, 00B7DF30): call
// the step routine on the iterator and return it (EAX = ECX).
struct IteratorWrapper { std::uint32_t address; std::uint32_t step; bool increment; };
const IteratorWrapper* find_iterator_wrapper(std::uint32_t address) noexcept;
template <class It>
It& step_iterator(It& it, const std::function<void(It&)>& step) {
    step(it);
    return it;
}

// Type-info initializer thunks (00B78510, 00B78520, 00B7C270, 00B7C280, 00B91570,
// 00B91580, 00B93D10, 00B93D20): run the initializer on ECX and return ECX. None has a
// static reference (rel32 and absolute scans), so they are dead copies of the inlined
// static initializers.
struct TypeInitThunk { std::uint32_t address; std::uint32_t initializer; };
const TypeInitThunk* find_type_init_thunk(std::uint32_t address) noexcept;

// ---------------------------------------------------------------------------
// Element arrays whose element type is non-trivial (host model: std::vector plus the
// native capacity word).
template <class T>
struct OwnedArray {
    std::vector<T> items;        // count = items.size()
    std::int32_t capacity = 0;
};
// 00B80C70 (and 00B988F0): reserve: n clamped to >= 1; when capacity < n the rows are
// copied into a new buffer, the old rows destroyed (resize 0 + free) and capacity = n.
template <class T>
void owned_reserve_min_one(OwnedArray<T>& a, std::int32_t n) {
    if (n < 1) n = 1;
    if (a.capacity < n) {
        a.items.reserve(static_cast<std::size_t>(n));
        a.capacity = n;
    }
}
// 00B80E10 (and 00B989F0): resize: reserve(n) when capacity < n, value-initialise new rows,
// destroy rows from the end down to n.
template <class T>
void owned_resize(OwnedArray<T>& a, std::int32_t n) {
    if (a.capacity < n) owned_reserve_min_one(a, n);
    a.items.resize(static_cast<std::size_t>(n < 0 ? 0 : n));
}

// ---------------------------------------------------------------------------
// Flare (cFlare-like generated model, vtable 00D63420, pool 1E4h slots at 010902A8).
// Vertex format of the flare streams (28h bytes, declaration "simplecolor2.mvfm").
struct FlareVertex {
    float x = 0, y = 0, z = 0, w = 0;  // +00..+0Ch
    std::uint32_t colour = 0;          // +10h
    std::uint32_t colour2 = 0;         // +14h
    float u = 0, v = 0;                // +18h, +1Ch
    float u2 = 0, v2 = 0;              // +20h, +24h
};
static_assert(sizeof(FlareVertex) == 0x28, "flare vertex stride");

// Atlas item fields used for texture coordinates.
struct FlareAtlasRect { float u0 = 0, v0 = 0, u1 = 0, v1 = 0; };  // item +14h..+20h

// Flare element record (pointer array at flare +18Ch, count +190h).
struct FlareElement {
    std::int32_t type = 0;         // +00: 0 projected quad, 2 projected two-colour quad, else screen quad
    bool place = false;            // +04: run 00B8B8C0 before writing
    bool truncate_20 = false;      // +05: store trunc(occlusion excess) at +20h
    float origin[2]{};             // +08h/+0Ch
    float angle = 0;               // +10h
    float scale = 0;               // +14h
    std::uint8_t rgba[4]{};        // +18h
    std::uint8_t rgba2[4]{};       // +1Ch
    std::int32_t word_20 = 0;      // +20h
    float size[2]{};               // +34h/+38h
    float dir[2]{};                // +3Ch/+40h
    float t = 0;                   // +44h
    std::string names[2];          // +48h/+50h atlas item names
    const FlareAtlasRect* rects[2]{};  // +58h/+5Ch
};

// Camera node fields read by the flare (world matrix rows at +F0h).
struct FlareCameraView {
    float right_f0[3]{};     // +F0h
    float up_100[3]{};       // +100h
    float forward_110[3]{};  // +110h
    float position_120[3]{}; // +120h
    float aspect_1c8 = 0;    // +1C8h
    std::uint32_t word_198 = 0;  // +198h: non-zero suspends flare updates
};

struct FlareView {
    float world_120[3]{};          // flare node world translation (a direction for sun flares)
    float visibility_ac = 0;       // +ACh, written through 00B6DA70
    float colour2_scale_184 = 0;   // +184h
    float anchor_198[3]{};         // +198h
    float ndc_1a4[2]{};            // +1A4h/+1A8h
    float clip_1ac[4]{};           // +1ACh..+1B8h
    FlareCameraView* camera_1bc = nullptr;
    float duration_1c0 = 0;        // +1C0h (constructor argument 2)
    float timer_1c4 = 0;           // +1C4h (dt accumulator)
    bool limit_flag_1c8 = false;   // +1C8h (constructor argument 3)
    float facing_1dc = 0;          // +1DCh
    std::vector<FlareElement*> elements;  // +18Ch/+190h/+194h
};

struct FlareServices {
    virtual void refresh_world_00b6db70(void* node) = 0;   // when +5Ch bit 2 is clear
    virtual bool world_valid(void* node) = 0;
    // 00B70490 view-projection of the camera, then 00B62D10 row-vector transform.
    virtual void transform_view_projection(FlareCameraView& camera, const float in[4], float out[4]) = 0;
    virtual float vector_length_00419440(const float v[3]) = 0;
    virtual void normalize_00419510(float out[3], const float v[3]) = 0;
    virtual void cross_004f9b30(float out[3], const float a[3], const float b[3]) = 0;
    virtual void set_visibility_00b6da70(FlareView& flare, float value, bool recurse) = 0;
    // Occlusion queries +1D0h / +1D4h: vtable +14h returns the pixel count.
    virtual bool has_queries(const FlareView& flare) = 0;
    virtual std::uint32_t visible_pixels(FlareView& flare) = 0;  // +1D0h
    virtual std::uint32_t total_pixels(FlareView& flare) = 0;    // +1D4h
    // 00B74640(model, n) geometry, 00B73260(geometry, 0) stream, vtable +10h lock / +14h unlock.
    virtual FlareVertex* lock_stream0(FlareView& flare, std::int32_t geometry_index) = 0;
    virtual void unlock_stream0(FlareView& flare, std::int32_t geometry_index) = 0;
    // 00B8B350 / 00B8B3B0: reconstructed in post_effects_cc13.cpp (bsp::cc13fx).
    virtual float centre_weight_00b8b350(const FlareView& flare) = 0;
    virtual float edge_fade_00b8b3b0(const FlareView& flare) = 0;
    // 00B8B8C0 (dt is passed but unused by the callee).
    virtual void place_element_00b8b8c0(FlareView& flare, float dt, FlareElement& e) = 0;
protected:
    ~FlareServices() = default;
};

// Corner table [00E13088]: (-1, 1), (1, 1), (1, -1), (-1, -1).
extern const float kFlareCorners[4][2];
// 00B8C6B0 (thiscall(flare, FlareVertex* quad, element), RET 8): screen quad. Per corner
// k: c = f(corner*size), rotated by the element angle with f(cos)/f(sin); x = f(ndc.x *
// scale + rx), y = f(ry * camera aspect + ndc.y * scale), z = 1.6e-5 [00D63414], colour =
// 00B8C520(rgba, visibility). +0Ch and +14h are not written. UV from the first rect:
// (u0,v0), (u1,v0), (u1,v1), (u0,v1).
void flare_write_screen_quad_00b8c6b0(const FlareView& flare, FlareVertex quad[4], const FlareElement& e);
// 00B8C8A0 (same ABI): world quad: the rotated corner spans the camera right/up rows,
// added to the anchor, projected (w = 1) and divided by w; z = 0.9999 [00D63418], w word
// 1.0, colour from visibility, +14h = 0, UV as above.
void flare_write_projected_quad_00b8c8a0(FlareView& flare, FlareVertex quad[4], const FlareElement& e,
    FlareServices& s);
// 00B8CC90 (same ABI): as 00B8C8A0 with colour = 00B8C520(rgba, f(visibility * 3.0)),
// colour2 = 00B8C520(rgba2, +184h) and the second UV set (-1,-1), (-1,2), (2,2), (2,-1).
void flare_write_projected_quad2_00b8cc90(FlareView& flare, FlareVertex quad[4], const FlareElement& e,
    FlareServices& s);
// 00B8BB50 (stdcall(model, const float outer[2], const float inner[2], uint32 n), RET 10h;
// unreferenced): n segments of a ring, two vertices each (inner first: uv (0.5, 0.5);
// outer: uv (0, 0)), colour 00FFFFFFh, z 0, w 1; angle step f(2*pi / n) [CE3828] with n
// unsigned; indices (2i, 2i+1, (2i+3) % 2n), (2i, (2i+3) % 2n, (2i+2) % 2n).
void flare_write_ring_00b8bb50(FlareVertex* vertices, std::uint16_t* indices, const float outer[2],
    const float inner[2], std::uint32_t n);
// 00B8BDD0 (thiscall(flare)): the sun disc of geometry 1. dir = normalize(flare world
// position) (0 when the length is <= 0); up = (0,1,0) unless |dir.y| < 0.01, then (0,0,-1)
// (the test is as written: a nearly HORIZONTAL direction switches the up vector); right =
// cross(dir, up); centre = camera + dir*4500 [D63408]; eight rim vertices at radius 300
// [CE3CA8] in the right/up plane, angle step f(pi/4) [CEDCD0]. w = 1, colour 00FFFFFFh,
// rim uv (0.5, 0.5); the centre's uv is not written.
void flare_update_billboard_00b8bdd0(FlareView& flare, FlareServices& s);
// 00B8D150 (thiscall(flare, camera, float dt), RET 8): see the doc for the full order.
void flare_update_00b8d150(FlareView& flare, FlareCameraView& camera, float dt, FlareServices& s);

}  // namespace bsp::cc13_scene2
