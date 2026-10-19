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

// Flare lifetime. Constructor 00B8D9C0 (thiscall(this, parent, float duration, byte flag),
// RET 0Ch): generated-model constructor (00B75030) with the name "flare", vtable 00D63420,
// element array cleared, +1CCh = 0, parent set (00B6E680), +1C4h = 0, byte +1C8h = flag,
// +1D4h = +1D0h = 0, +1C0h = duration.
struct FlareLifetimeServices {
    virtual void construct_generated_model_00b75030(void* self, const std::string& name) = 0;
    virtual void set_parent_00b6e680(void* self, void* parent) = 0;
    virtual void release_ref(void* object) = 0;           // InterlockedDecrement +04h, slot 0 at zero
    virtual void destroy_model_00b750c0(void* self) = 0;
    virtual void* pool_allocate_00b8d700() = 0;           // pool 010902A8
    virtual void pool_free_00b8c3a0(void* p) = 0;
protected:
    ~FlareLifetimeServices() = default;
};
struct FlareStorage {
    std::uint32_t vtable = 0;
    std::vector<FlareElement*> elements;   // +18Ch
    void* ref_1cc = nullptr;
    void* query_visible_1d0 = nullptr;
    void* query_total_1d4 = nullptr;
    float duration_1c0 = 0;
    float timer_1c4 = 0;
    bool flag_1c8 = false;
};
void flare_construct_00b8d9c0(void* self, FlareStorage& f, void* parent, float duration, bool flag,
    FlareLifetimeServices& s);
// 00B8D8C0: vtable 00D63420, release +1CCh, +1D0h, +1D4h (each cleared), element array
// resized to 0 (0086AA10) and freed, then the model destructor (00B750C0).
void flare_destruct_00b8d8c0(void* self, FlareStorage& f, FlareLifetimeServices& s);
// 00B8D9A0 scalar delete: destruct, then return the slot to pool 010902A8 when flags & 1.
void* flare_scalar_delete_00b8d9a0(void* self, FlareStorage& f, std::uint32_t flags, FlareLifetimeServices& s);
// 00B8D840 operator new (JMP to the pool allocate with ECX = 010902A8) and 00B8C6A0
// operator delete (pool free).
void* flare_operator_new_00b8d840(FlareLifetimeServices& s);
void flare_operator_delete_00b8c6a0(void* p, FlareLifetimeServices& s);
// 00B8D850 (thiscall(dst, const src&), returns dst): resize dst to 0 (0086AA10), reserve
// src.count (0086A280), then append every src pointer with the double/min-1 reserve rule.
void assign_element_pointers_00b8d850(PointerArray& dst, const PointerArray& src,
    const std::function<void(PointerArray&, std::int32_t)>& resize,
    const std::function<void(PointerArray&, std::int32_t)>& reserve);

// 00B8DAC0 (thiscall(flare)): occlusion geometry. Material from "flareocclusion.mshd"
// (00535320); mesh object (00B73B60 + 00B73D70, null when the slot is null); vertex
// declaration "simplecolor2.mvfm" (device vtable +38h); vertex buffer (+5Ch)(9, 1, decl),
// set as stream 0; index buffer (+60h)(24, 1, 65h); draw section (00533FA0) with +08h = 4,
// +0Ch = 0, +10h = 9, +14h = 0, +18h = 8, material, layout and append; disc fan 00B8B980 with
// radii (1, 1) and 8 rim vertices; model geometry (00B75170)(1, mesh, -1, -1); then
// +1D0h and +1D4h = device vtable +24h() (occlusion queries). Temporaries are released in
// native order.
struct FlareOcclusionServices {
    virtual void* create_material_00535320(const std::string& effect) = 0;
    virtual void* create_mesh_object() = 0;                             // 00B73B60 + 00B73D70
    virtual void* create_declaration(const std::string& format) = 0;    // device +38h
    virtual void* create_vertex_buffer(std::int32_t count, std::int32_t usage, void* decl) = 0;  // +5Ch
    virtual void* create_index_buffer(std::int32_t count, std::int32_t usage, std::int32_t format) = 0;  // +60h
    virtual void set_vertex_stream_00b73bb0(void* mesh, std::int32_t index, void* stream) = 0;
    virtual void set_index_stream_00b73b70(void* mesh, void* indices) = 0;
    virtual void* create_draw_section_00533fa0() = 0;
    virtual void set_section_fields(void* section, std::int32_t primitive, std::int32_t first_vertex,
        std::int32_t vertex_count, std::int32_t first_index, std::int32_t primitive_count) = 0;
    virtual void set_section_material_00b864c0(void* section, void* material) = 0;
    virtual void rebuild_section_layout_00b865a0(void* section, void* mesh) = 0;
    virtual void append_section_00b73c60(void* mesh, void* section) = 0;
    virtual void write_disc_fan_00b8b980(void* flare, void* mesh, const float radii[2], std::uint32_t rim) = 0;
    virtual void set_model_geometry_00b75170(void* flare, std::uint32_t word, void* mesh, float a, float b) = 0;
    virtual void* create_query() = 0;                                   // device +24h
    virtual void release_ref(void* object) = 0;
protected:
    ~FlareOcclusionServices() = default;
};
void flare_initialize_occlusion_00b8dac0(void* flare, FlareStorage& f, FlareOcclusionServices& s);

// ---------------------------------------------------------------------------
// Mit::allocator block pools (owner layout and block format: bsp/post_effects_cc13.hpp
// MitBlockPool, which models the allocate/free/trim slot order). This adds the owner
// lifetime and the block initializer.
struct MitAllocatorListElem {
    MitAllocatorListElem* prev = nullptr;  // +04h
    MitAllocatorListElem* next = nullptr;  // +08h
};
struct MitBlockModel {
    std::vector<std::uint32_t> slot_block_index;  // last dword of each slot
    std::uint16_t stack[32]{};
    std::uint16_t free_count = 0;
};
struct MitPoolOwner {
    MitAllocatorListElem link;
    std::uint32_t vtable = 0;
    std::int32_t recursion_24 = 0;
    NativeArray<MitBlockModel*> blocks;    // +28h
    std::int32_t current_34 = -1;
};
// Instantiations: payload 1E0h (slot 1E4h, flare, vtable D63410), 194h (slot 198h; pools
// 010903F0 quadtree node and 01090358 skin model, vtable D6355C) and 190h (slot 194h; pool
// 01090390 bone model, vtable D63560). The base vtable is Mit::allocator_list_elem 00D7A0C0. Constructors 00B8D580, 00B90680, 00B90940: link at the head of the
// allocator list [00E188B4], set the pool vtable, initialise the critical section, clear the
// recursion count and block vector, current = -1, then reserve 32 block pointers.
void mit_pool_construct(MitPoolOwner& pool, MitAllocatorListElem*& list_head_00e188b4, std::uint32_t vtable);
// Destructors 00B8C2E0, 00B8FF60, 00B90090: free every block and the vector, leave the
// critical section once per outstanding recursion, delete it, restore the base vtable
// 00D7A0C0 and unlink.
void mit_pool_destruct(MitPoolOwner& pool, MitAllocatorListElem*& list_head_00e188b4);
// Block initializers 00B8B6D0, 00B8FA80, 00B8FB50: free count 32, stack[i] = 31 - i, every
// slot's block index = `index`.
void mit_pool_init_block(MitBlockModel& block, std::uint32_t index);
// Free thunks 00B90470, 00B90480, 00B91BF0 (operator delete for skin model, bone model and
// quadtree node): the pool free with ECX = the pool global.
struct MitPoolThunk { std::uint32_t address; std::uint32_t pool; std::uint32_t target; };
const MitPoolThunk* find_mit_pool_thunk(std::uint32_t address) noexcept;

// ---------------------------------------------------------------------------
// Resource items. Constructors (fastcall(this, EDX name)): the reference base (00B868B0),
// then the vtable and zeroed words.
struct ResourceItemConstructor {
    std::uint32_t address;
    std::uint32_t vtable;
    std::uint8_t zero_first;   // first zeroed dword index
    std::uint8_t zero_count;
};
const ResourceItemConstructor* find_resource_item_constructor(std::uint32_t address) noexcept;
void construct_resource_item(std::uint32_t address, std::uint32_t words[8],
    const std::function<void()>& construct_reference_base);
// 00B93680 / 00B93980 / 00B93A00 (fastcall(this, EDX name, mesh, word)): reference base,
// +08h = mesh, vtable 00D63738, +0Ch = word, mesh retained (+04h), then the derived vtable
// (00D6375C, 00D63780) for the last two.
struct MeshResourceWords { std::uint32_t vtable = 0; RefCounted* mesh_08 = nullptr; std::uint32_t word_0c = 0; };
void construct_mesh_resource(std::uint32_t address, MeshResourceWords& out, RefCounted* mesh, std::uint32_t word,
    const std::function<void()>& construct_reference_base);
// 00B8B940 (stdcall(object, float), RET 8): object +20h = low dword of the value truncated
// toward zero (FISTP qword with RC = 11b). Renamed from RoundFloatToField20: it truncates.
void truncate_float_to_field_20_00b8b940(std::int32_t& field_20, float value);
// 00B8E730 / 00B8E750: copy track words +08h..+14h / +18h..+2Ch (FLD/FSTP float copies).
void copy_track_range_00b8e730(const float track_08[4], float out[4]);
void copy_track_key_block_00b8e750(const float track_18[6], float out[6]);
// 00B8E690 (node vtable thunk): 00B6E150(this, channel, time, flag) with the time reloaded
// as a float.
// 00B8EB20 (cGroupParamsResource apply, thiscall(params, a, b, node), RET 0Ch): when the
// node's IsType accepts [0109032C], node +184h = params +08h.
void group_params_apply_00b8eb20(float params_08, float& node_184, bool node_is_group);

// ---------------------------------------------------------------------------
// cGroup (vtable slots at 00D63508 clone 00B8F6F0, 00D63518 render 00B8EF10, 00D63520 detach 00B8F090).
// LOD fade of a node's bounding sphere seen from the view camera: the sphere is transformed
// by the view-projection (00B70490, 007C1180), d = max(sphere.z, 0.01) (NaN gives 0.01),
// k = f(1 - f(camera +4Ch * |f(sphere.w / d)|)), clamped to [0, 1] (NaN passes through).
float lod_fade_from_projected_sphere(float projected_z, float projected_w, float camera_4c);
struct GroupChildView {
    std::uint32_t mask_48 = 0;
    void* object = nullptr;
};
struct GroupRenderServices {
    virtual std::uint32_t classify_sphere_00b71530(void* cull_camera, void* node) = 0;  // node vtable +48h sphere
    virtual void project_sphere(void* view_camera, void* node, float& z, float& w) = 0; // 00B70490 + 007C1180
    virtual float view_camera_lod_4c(void* view_camera) = 0;
    virtual std::uint32_t cull_camera_mask_19c(void* cull_camera) = 0;
    virtual void render_child_slot_20(void* child, void* context, float fade, float alpha, std::uint32_t flags) = 0;
protected:
    ~GroupRenderServices() = default;
};
// 00B8EF10 (thiscall(group, context, unused, float alpha, flags), RET 10h). Context +08h
// is the cull camera, +0Ch the view camera. Nothing happens unless +ACh > 0. Unless all
// clip bits are set (flags & 555h) and when byte +138h & 3, the sphere is classified;
// AAAh (outside) stops. Otherwise k = lod fade; when +184h > k every child whose +48h mask
// meets the cull camera +19Ch is rendered with (context, k, f(+ACh * alpha), flags).
void group_render_00b8ef10(void* group, float visibility_ac, float threshold_184, std::uint8_t byte_138,
    void* context, void* cull_camera, void* view_camera, float alpha, std::uint32_t flags,
    const std::vector<GroupChildView>& children, GroupRenderServices& s);
// 00B8F090: detach children (00B6D820), then pop the attached-node array (+178h data,
// +17Ch count) from the end, clearing each node's +A0h back link.
void group_detach_nodes_00b8f090(SceneNode& group, std::vector<void**>& attached_a0,
    const std::function<void(SceneNode&)>& detach_children);
// 00B8F6F0 step 1: children whose IsType accepts [010903C8] (00B8F9C0) are unlinked and
// re-appended after the last remaining child, in REVERSE order of their appearance. When
// every child moves, the native code dereferences a null first child (here:
// std::logic_error). Sibling links: +3Ch next, +40h previous, parent +34h first child.
struct GroupSibling {
    GroupSibling* parent = nullptr;  // +30h
    GroupSibling* first = nullptr;   // +34h
    GroupSibling* next = nullptr;    // +3Ch
    GroupSibling* prev = nullptr;    // +40h
    bool movable = false;            // IsType([010903C8])
};
void group_move_typed_children_last_00b8f6f0(GroupSibling& group);
// 00B8F6F0 step 2: allocate (00B8F310), construct with the name (00B8F5E0), copy the base
// clone state (00B6F150), copy bytes +174h and +175h. Returns the clone (null slot passes
// null to the copy, as in 00B6F8F0).
struct GroupCloneServices {
    virtual void* allocate_group_00b8f310() = 0;
    virtual void* construct_group_00b8f5e0(void* slot, const std::string& name) = 0;
    virtual void copy_clone_base_state_00b6f150(const void* source, void* clone, std::uint32_t flags, void* parent) = 0;
    virtual void copy_bytes_174_175(const void* source, void* clone) = 0;
protected:
    ~GroupCloneServices() = default;
};
void* group_clone_00b8f6f0(void* source, GroupSibling& children, const std::string& name, std::uint32_t flags,
    void* parent, GroupCloneServices& s);

// ---------------------------------------------------------------------------
// Skin and bone models.
// 00B8F960 (skin model vtable 00D635F0 +10h Clone, thiscall(this, flags, parent), RET 8):
// allocates a raw node slot (00B6ED70, ECX = 174h) and constructs a PLAIN c3dNode with this
// node's name; no base state is copied and both arguments are ignored. Null when the slot
// is null.
void* skin_model_clone_00b8f960(const std::string& name, const std::function<void*()>& allocate_node,
    const std::function<void*(void*, const std::string&)>& construct_node);
// 00B8FDE0 (thiscall(model, channel, float time, flag), RET 0Ch): when the animator (+130h)
// accepts the optimized-animator type (00B75E50), +190h = the baked frame address
// (00B92500 on animator +34h), else 0; then 00B6E150(model, channel, time, flag).
struct SkinModelAnimServices {
    virtual void* animator_130(void* model) = 0;
    virtual bool animator_is_optimized(void* animator) = 0;
    virtual const float* baked_frame_00b92500(void* animator, std::int32_t channel, float time) = 0;
    virtual void apply_animation_00b6e150(void* model, std::int32_t channel, float time, std::int32_t flag) = 0;
protected:
    ~SkinModelAnimServices() = default;
};
void skin_model_update_animator_00b8fde0(void* model, const float*& frame_190, std::int32_t channel, float time,
    std::int32_t flag, SkinModelAnimServices& s);
// 00B8FE50: +00h..+18h = 0 (seven dwords), then the identity matrix copied to +20h.
struct AnimatedTransform { std::uint32_t words[7]{}; float matrix_20[16]{}; };
void animated_transform_construct_00b8fe50(AnimatedTransform& t);
// 00B90490 (thiscall(model, context, float fade, float alpha, flags), RET 10h): when
// +A0h (00B6D810) is set, fade is replaced by the LOD fade of the view camera (context
// +0Ch), as in 00B8EF10; then the generated-model render 00B748E0(context, fade, alpha, flags).
float skin_model_render_fade_00b90490(bool has_a0, float fade, float projected_z, float projected_w, float camera_4c);
// Constructors 00B91220 (bone model, vtable 00D63590, +184h..+18Ch = 0) and 00B915E0
// (skin model, vtable 00D635F0, +184h..+190h = 0) after the generated-model constructor.
// Destructors 00B91280 (bone; resize bone pointers 00B90630 to 0, free +184h, 00B750C0) and
// 00B91650 (skin; vtable 00D635F0, resize bindings 00B91460 to 0, free +184h, 00B750C0;
// renamed from ConstructWithBindings: it is the destructor). Scalar deletes 00B915A0 (pool
// 01090390) and 00B916C0 (pool 01090358).
struct ModelWordsConstructor { std::uint32_t address; std::uint32_t vtable; std::uint8_t zero_count; };
const ModelWordsConstructor* find_model_words_constructor(std::uint32_t address) noexcept;
// 00B917B0: when the holder is set, unlink and release the node (00B6DFA0) and clear it.
void release_node_holder_00b917b0(void*& holder, const std::function<void(void*)>& unlink_and_release);

// ---------------------------------------------------------------------------
// Quadtree node (vtable 00D63650; bounds +17Ch min / +188h max, level +178h, leaf byte +174h).
struct QuadAabb { float min[3]; float max[3]; };
// 00B917D0 (thiscall(node, a, b, c, d), RET 10h): plane test of the box. For x and z the
// near/far corners are the smaller/larger of the two stored values (chosen by the sign of
// the coefficient; a NaN coefficient takes the negative arm); for y the stored min/max are
// used as they are. far = f(f((b*py + a*px) + c*pz) - d): < 0 gives 0 (outside); near =
// f(f((a*nx + b*ny) + c*nz) - d): > 0 gives 1 (inside), else 10 (straddling).
std::int32_t quadtree_clip_range_00b917d0(const QuadAabb& box, float a, float b, float c, float d);
// 00B91A50 (thiscall(node, context, a, b, flags), RET 10h): when node +48h meets the cull
// camera +19Ch: unless flags & 555h is complete, test min(planes, 6) frustum planes from the
// last down (00B70710, 00B65080, 00B6FDC0); any 0 stops; a sum of 6 adds 555h to the flags;
// then render the children (00B6D990).
struct QuadRenderServices {
    virtual std::uint32_t node_mask_48(void* node) = 0;
    virtual std::uint32_t cull_camera_mask_19c(void* context) = 0;
    virtual std::uint32_t plane_count(void* context) = 0;
    virtual void plane(void* context, std::uint32_t index, float out[4]) = 0;
    virtual void render_children_00b6d990(void* node, void* context, float a, float b, std::uint32_t flags) = 0;
protected:
    ~QuadRenderServices() = default;
};
void quadtree_render_00b91a50(void* node, const QuadAabb& box, void* context, float a, float b, std::uint32_t flags,
    QuadRenderServices& s);
// 00B91B20 (thiscall(builder, node, QuadAabb* box), RET 8): when the node is a generated
// model (00B74310) with +ACh != 0 (NaN passes), its world box (vtable +4Ch local bounds,
// 0092E250 transform by +F0h) has min.y raised to -100 when below (-100 double D636A8,
// float CE65D8) and both corners are added (00427D10); then recurse into children with
// +48h bit 30.
struct QuadBoundsServices {
    virtual bool is_generated_model(void* node) = 0;
    virtual float visibility_ac(void* node) = 0;
    virtual void world_box(void* node, QuadAabb& out) = 0;   // refresh + vtable +4Ch + 0092E250
    virtual void include_point_00427d10(QuadAabb& box, const float p[3]) = 0;
    virtual std::vector<void*> children(void* node) = 0;
    virtual bool tagged_bit30(void* node) = 0;
protected:
    ~QuadBoundsServices() = default;
};
void quadtree_include_child_bounds_00b91b20(void* node, QuadAabb& box, QuadBoundsServices& s);
// 00B91C40 (Clone, thiscall(source, flags, parent), RET 8): allocate from pool 010903F0,
// construct with the source name, vtable 00D63650, +178h = 0, then copy the base state.
// 00B91C00 scalar delete: vtable 00D63650, node destroy (00B6F440), pool free when flags & 1.
// 00B91D70 build: see the doc (binary split of the tagged children along the longer of the
// x/z extents at the mean world position; depth limit 6, fewer than 5 children make a leaf).
struct QuadBuildServices {
    virtual std::uint32_t level_178(void* node) = 0;
    virtual std::uint32_t child_count_38(void* node) = 0;
    virtual QuadAabb& bounds(void* node) = 0;
    virtual void set_leaf_174(void* node, bool leaf) = 0;
    virtual std::vector<void*> children(void* node) = 0;   // sibling order
    virtual bool tagged_bit30(void* node) = 0;
    virtual void world_position(void* node, float out[3]) = 0;  // refresh + +120h
    virtual void* create_quad_child(void* parent, std::uint32_t level) = 0;  // pool, "QuadTreeNode", 00B6D890
    virtual void include_child_bounds_00b91b20(void* child, QuadAabb& box) = 0;
    virtual void set_parent_00b6e680(void* node, void* parent) = 0;
    virtual void unlink_and_release_00b6dfa0(void* node) = 0;
protected:
    ~QuadBuildServices() = default;
};
void quadtree_build_00b91d70(void* node, QuadBuildServices& s);

// ---------------------------------------------------------------------------
// Animation value arrays (30h-byte elements) and mesh resources.
struct AnimValue { std::uint32_t words[12]; };
// 00B92650: append with the double/min-1 reserve rule (00B92440).
void push_back_animation_value_00b92650(NativeArray<AnimValue>& a, const AnimValue& v,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve);
// 00B926A0: when capacity < 0, reserve(0); count = 0. 00B92790 additionally frees the data.
void clear_animation_values_00b926a0(NativeArray<AnimValue>& a,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve);
void destroy_animation_values_00b92790(NativeArray<AnimValue>& a,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve);
// 00B93020 / 00B930F0 / 00B931F0: allocate a model slot (00B74EB0, pool 01090358 via
// 00B90C10, pool 01090390 via 00B90C20) and construct (00B75030, 00B915E0, 00B91220) with
// the name; null when the slot is null. Thunks 00B93080 / 00B93150 jump to the first two.
void* create_model_from_resource(const std::function<void*()>& allocate,
    const std::function<void*(void*)>& construct);
// 00B934D0 (stdcall(state, reader, byte with_extra), RET 0Ch): looks up element state+08h
// (00B732C0, result unused), reads a count, then per entry two dwords (start, length) and,
// with the flag, a dword and a float; the min start and max end are computed in registers
// and discarded. state+08h increments.
struct StructuredReader {
    virtual std::uint32_t read_dword_00be99f0() = 0;
    virtual float read_float_00be99d0() = 0;
protected:
    ~StructuredReader() = default;
};
void read_element_params_00b934d0(std::int32_t& element_index_08, StructuredReader& reader, bool with_extra,
    const std::function<void(std::int32_t)>& lookup_element);
// 00B94020 (cRenderMeshResource visit, thiscall(this, a1, a2, node, lod), RET 10h): base
// dispatch (00B86820); a generated-model node gets geometry = mesh(+08h)->vtable +10h(lod, 0)
// through 00B75170(+0Ch word, geometry, -1, -1); any other node gets a new child model named
// "Object" (this vtable +20h) parented to it (00B6E680) with that geometry. References are
// released in native order.
struct MeshResourceVisitServices {
    virtual void base_dispatch_00b86820(void* self, void* a1, void* a2, void* node, std::int32_t lod) = 0;
    virtual bool is_generated_model(void* node) = 0;
    virtual void* create_geometry(void* self, std::int32_t lod) = 0;        // mesh vtable +10h(lod, 0)
    virtual void set_geometry_00b75170(void* model, std::uint32_t word, void* geometry, float a, float b) = 0;
    virtual void* create_child_model(void* self, const std::string& name) = 0;  // this vtable +20h
    virtual void set_parent_00b6e680(void* node, void* parent) = 0;
    virtual void release_ref(void* object) = 0;
protected:
    ~MeshResourceVisitServices() = default;
};
void mesh_resource_visit_00b94020(void* self, std::uint32_t word_0c, void* a1, void* a2, void* node, std::int32_t lod,
    MeshResourceVisitServices& s);
// 00B94BD0: allocator<8-byte>::allocate (count 0 allocates 0; overflow throws bad_alloc).
void* allocate_8byte_elements_00b94bd0(std::uint32_t count, const std::function<void*(std::size_t)>& operator_new);
// 00B96B60: fill n 8-byte pairs with a value (placement copies skip a null destination).
void fill_pairs_00b96b60(PairRecord* first, std::int32_t n, const PairRecord& value);
// 00B96D40 / 00B96D70: free the buffer at +04h (when non-null) and clear +04h..+0Ch.
void free_vector_at_4(void*& data_4, void*& end_8, void*& cap_c, const std::function<void(void*)>& free_block);
// PodVector FreeData 00B8B7B0, 00B8FC30, 00B8FC70: free +00h when non-null.
void free_pod_data(void* data, const std::function<void(void*)>& free_block);

// ---------------------------------------------------------------------------
// Ocean spectrum helpers not covered by src/ocean_spectrum.cpp (cc9). The 64x64 cell
// vectors hold interleaved (re, im) float pairs.
// 00B94E90 / 00B950B0 (fastcall(float* cells), both unreferenced): an in-place 32-point
// radix-2 FFT over interleaved complex values, along a row (stride 1) or a column (stride
// 32) of a 32x32 grid. Bit reversal, then five stages; twiddles start at (1, 0) and step by
// (c1, c2) with c1 = -1, c2 = 0 initially and, after each stage, c2 = f(sqrt(f((1 - c1) *
// 0.5))), c1 = f(sqrt(f((c1 + 1) * 0.5))) (no sign flip, no final scaling).
void ocean_fft32(float* cells, int stride_complex);
// 00B962E0 / 00B96450 (thiscall(spectrum, u, v), RET 8; HorizontalDisplacement callers):
// fx = f(u*64), fy = f(64*v) [D638F0 double], i = CVTTSS2SI, cells (i & 63) and
// ((i + 1) & 63); bilinear in lerp form with the fractions fx - ix / fy - iy kept
// unrounded; the re (A) or im (B) part of the +3Ch vector.
float ocean_sample_displacement(const std::vector<float>& pairs_3c, float u, float v, int part);
// 00B965C0 / 00B96790 / 00B96960 (unreferenced): FISTP-rounded cells (default rounding),
// fractions |f(f(u+0) - i)| (a zero fraction becomes -0), neighbour i + 1 when i > 0 else
// i - 1, lerp form. A/B read the +3Ch pairs (re/im), C the +1Ch vector re.
float ocean_sample_rounded(const std::vector<float>& pairs, float u, float v, int part);
// 00B96C80 (vtable 00D63900 slot, thiscall(spectrum, float t), RET 4): vtable +14h, advance
// (00B95D30), the +08h member's vtable slot 0, vtable +10h.
struct OceanStepServices {
    virtual void slot_14(void* spectrum) = 0;
    virtual void advance_00b95d30(void* spectrum, float t) = 0;
    virtual void member_08_slot_0(void* spectrum) = 0;
    virtual void slot_10(void* spectrum) = 0;
protected:
    ~OceanStepServices() = default;
};
void ocean_step_slot_00b96c80(void* spectrum, float t, OceanStepServices& s);
// 00B96DA0: vtable 00D638F8, then the six vectors freed and cleared in the order +5Ch,
// +4Ch, +3Ch, +2Ch, +1Ch, +0Ch (begin words +60h .. +10h), then the ref-counted base.
std::vector<std::uint32_t> ocean_destruct_order_00b96da0();
// 00B96F80 vector<8-byte>(n, value) (length error above 1FFFFFFFh), 00B97030
// vector<float>(n, value) (above 3FFFFFFFh), 00B970A0 vector<8-byte>(n, {0, 0}).
std::vector<PairRecord> construct_pair_vector_00b96f80(std::uint32_t n, const PairRecord& value);
std::vector<float> construct_float_vector_00b97030(std::uint32_t n, float value);
std::vector<PairRecord> construct_zero_pair_vector_00b970a0(std::uint32_t n);

// ---------------------------------------------------------------------------
// Grid index builder containers (singleton [01090490], vtable 00D6391C).
struct GridStrip14 { std::uint32_t words[5]{}; };
struct GridPatch20 {
    std::uint32_t words[5]{};
    OwnedArray<GridStrip14> strips;   // +14h array
};
using GridRow0C = OwnedArray<GridPatch20>;
// Vector14 (POD rows): reserve 00B974B0 (min 1, signed, capacity last), resize 00B97560
// (zero fill), push 00B97620 (double/min 1), assign 00B976A0 (resize 0, reserve n, push
// each), tidy 00B97840 / 00B97890 (resize 0, free).
void strips_reserve_00b974b0(OwnedArray<GridStrip14>& a, std::int32_t n);
void strips_resize_00b97560(OwnedArray<GridStrip14>& a, std::int32_t n);
void strips_push_back_00b97620(OwnedArray<GridStrip14>& a, const GridStrip14& v);
void strips_assign_00b976a0(OwnedArray<GridStrip14>& dst, const OwnedArray<GridStrip14>& src);
void strips_tidy_00b97840(OwnedArray<GridStrip14>& a);
// Vector20: an element copy copies five words, clears the array and assigns it (so the
// copy's capacity is max(1, count)). reserve 00B985D0, resize 00B986F0, push 00B98770,
// assign 00B98830, tidy 00B98880.
void patch_copy(GridPatch20& dst, const GridPatch20& src);
void patches_reserve_00b985d0(OwnedArray<GridPatch20>& a, std::int32_t n);
void patches_resize_00b986f0(OwnedArray<GridPatch20>& a, std::int32_t n);
void patches_push_back_00b98770(OwnedArray<GridPatch20>& a, const GridPatch20& v);
void patches_assign_00b98830(OwnedArray<GridPatch20>& dst, const OwnedArray<GridPatch20>& src);
void patches_tidy_00b98880(OwnedArray<GridPatch20>& a);
// Vector0C (rows of patches): reserve 00B988F0, resize 00B989F0, tidy 00B98A90 / 00B98AF0.
void rows_reserve_00b988f0(OwnedArray<GridRow0C>& a, std::int32_t n);
void rows_resize_00b989f0(OwnedArray<GridRow0C>& a, std::int32_t n);
void rows_tidy_00b98a90(OwnedArray<GridRow0C>& a);
// Builder lifetime. Constructor 00B998C0: vtable 00D6391C, +08h..+24h = 0, +04h = 20h, then
// 00B99790. 00B99790: BuildPatch(s, 0) for s = 1, 2, 4, .. <= +04h, and for s < 20h also
// the masks 1, 2, 4, 8, 3, 6, Ch, 9, 7, Eh, Dh, Bh, Fh; then an index buffer of +0Ch words
// (device +60h(n, 1, 65h)) at +20h is filled from the word array and the array is emptied.
// Destructor 00B98B10: vtable 00D6391C, release +20h and +24h, rows resized to 0 and freed,
// word array resized to 0 and freed, [01090490] = 0, singleton-base vtable 00CE3818.
// 00B97480: [01090490] = 0 and the singleton-base vtable.
std::vector<std::uint32_t> grid_patch_masks_00b99790(std::uint32_t size, std::uint32_t max_size);
struct GridBuilderServices {
    virtual void build_patch_00b98be0(std::uint32_t size, std::uint32_t mask) = 0;  // not reconstructed
    virtual void* create_index_buffer(std::int32_t count) = 0;                    // device +60h(n, 1, 65h)
    virtual std::uint16_t* lock(void* buffer, std::int32_t count) = 0;            // +0Ch(n, 0, 0)
    virtual void unlock(void* buffer) = 0;                                        // +10h
    virtual void release_ref(void* object) = 0;
protected:
    ~GridBuilderServices() = default;
};
struct GridBuilder {
    std::uint32_t vtable = 0;
    std::uint32_t max_size_04 = 0;
    std::vector<std::uint16_t> words_08;  // +08h data, +0Ch count, +10h capacity
    OwnedArray<GridRow0C> rows_14;
    void* index_buffer_20 = nullptr;
    void* ref_24 = nullptr;
};
void grid_builder_build_all_00b99790(GridBuilder& b, GridBuilderServices& s);
// Index writers over the word array (+08h; growth double/min 1 through 0059E5E0-style
// reserve, not modelled: the host vector only keeps the contents). Every index is the
// 16-bit value (N + 1) * y + x with N = builder +04h; step = N / cells.
// 00B978B0 (thiscall(builder, cells), RET 4): the full patch: per cell (row r, column c)
// six indices; when the parities of c and r agree (y1,x0) (y1,x1) (y0,x0) (y0,x1) (y0,x0)
// (y1,x1), else (y1,x0) (y0,x1) (y0,x0) (y1,x0) (y1,x1) (y0,x1).
void grid_write_full_patch_00b978b0(GridBuilder& b, std::uint32_t cells);
// 00B97C80 (thiscall(builder, cells), RET 4): the interior cells 1 .. cells-2 in both
// directions; parities agree: (y1,x1) (y0,x0) (y1,x0) (y0,x1) (y0,x0) (y1,x1), else (y0,x1)
// (y0,x0) (y1,x0) (y1,x1) (y0,x1) (y1,x0).
void grid_write_interior_00b97c80(GridBuilder& b, std::uint32_t cells);
// 00B980A0 (thiscall(builder, edge, cells, edge_cells), RET 0Ch): one stitched edge (1 top,
// 2 right, 4 bottom, 8 left). A runs along the outer edge in coarse steps N / edge_cells,
// B along the inner row in fine steps N / cells; each coarse step emits (A', A, B) and,
// when B has been reached and is not the last inner point, (B', A', B) with B advanced.
// cells == 1 fans every outer step to the centre (N/2, N/2).
void grid_write_edge_00b980a0(GridBuilder& b, std::uint32_t edge, std::uint32_t cells, std::uint32_t edge_cells);
// 00B98BE0 (thiscall(builder, cells, mask), RET 8): mask 0 writes the full patch; otherwise
// the interior, the top edge through 00B980A0, then the bottom (4), left (8) and right (2)
// edges inline (cells doubled on an edge whose mask bit is set). The patch record {cells,
// edge cells for bits 1, 4, 8, 2; strips = [{4, 0, (N+1)^2, first index, triangles}]} is
// appended to row `cells` of +14h (rows grown to cells + 1 first).
void grid_build_patch_00b98be0(GridBuilder& b, std::uint32_t cells, std::uint32_t mask);
void grid_builder_construct_00b998c0(GridBuilder& b, GridBuilderServices& s);
void grid_builder_destruct_00b98b10(GridBuilder& b, void*& instance_01090490, GridBuilderServices& s);
// 00B97480: [01090490] = 0 and the singleton-base vtable 00CE3818.
void grid_instance_clear_00b97480(void*& instance_01090490, std::uint32_t& vtable);

// ---------------------------------------------------------------------------
// Remaining small bodies.
// 00B700A0 (fastcall(camera)): renderer (00F8D394) vtable +08h(0, 0, +188h, &+190h, +18Ch, +194h).
struct CameraClearView { std::uint32_t word_188 = 0; std::uint32_t colour_190 = 0; float depth_18c = 0; std::uint32_t word_194 = 0; };
void camera_submit_clear_00b700a0(const CameraClearView& c,
    const std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, const std::uint32_t*, float, std::uint32_t)>& renderer_slot_08);
// 00B8E690: forwards (channel, time, flag) to 00B6E150 with the same ECX.
void node_apply_animation_thunk_00b8e690(void* node, std::int32_t channel, float time, std::int32_t flag,
    const std::function<void(void*, std::int32_t, float, std::int32_t)>& apply_00b6e150);
// 00B91C40 quadtree clone and 00B91C00 scalar delete.
void* quadtree_clone_00b91c40(const void* source, const std::string& name, std::uint32_t flags, void* parent,
    const std::function<void*()>& pool_allocate, const std::function<void*(void*, const std::string&)>& construct_node,
    const std::function<void(void*)>& init_quadtree_words, NodeCloneServices& s);
void* quadtree_scalar_delete_00b91c00(void* self, std::uint32_t flags, const std::function<void(void*)>& set_vtable,
    const std::function<void(void*)>& destroy_node, const std::function<void(void*)>& pool_free);
// 00B91280 bone model and 00B91650 skin model destructors; 00B915A0 / 00B916C0 scalar deletes.
void model_destruct(void* self, bool set_skin_vtable, const std::function<void(void*)>& set_vtable,
    const std::function<void(void*)>& resize_array_to_zero_and_free, const std::function<void(void*)>& destroy_model);
void* model_scalar_delete(void* self, std::uint32_t flags, const std::function<void(void*)>& destruct,
    const std::function<void(void*)>& pool_free);
// 00B912E0 (bone model Clone, thiscall(source, flags, parent), RET 8): pool 01090390, generated
// model constructed with the source name, vtable 00D63590, +184h..+18Ch = 0; base state copied;
// the bone array resized to the source count and each bone looked up by name under `parent`
// (00B6E750(parent, name, 1)); when the source has a mesh (+180h) its geometry
// (vtable +10h(flags, 0)) is set with the source +178h/+17Ch (00B75170(0, ...)); +174h assigned
// through 00B74ED0; +08h..+14h copied.
struct BoneCloneServices {
    virtual void* allocate_00b90ac0() = 0;
    virtual void* construct_bone_model(void* slot, const std::string& name) = 0;  // 00B75030 + vtable + zeros
    virtual void copy_clone_base_state_00b6f150(const void* source, void* clone, std::uint32_t flags, void* parent) = 0;
    virtual void resize_bones_00b90630(void* clone, std::int32_t count) = 0;
    virtual void set_bone(void* clone, std::int32_t index, void* bone) = 0;
    virtual void* find_child_00b6e750(void* parent, const std::string& name) = 0;
    virtual void* source_mesh_geometry(const void* source, std::uint32_t flags) = 0;  // null when +180h is null
    virtual void set_geometry_00b75170(void* clone, void* geometry, float a, float b) = 0;
    virtual void release_ref(void* object) = 0;
    virtual void assign_retained_174(void* clone, const void* source) = 0;
    virtual void copy_vector_08(void* clone, const void* source) = 0;
protected:
    ~BoneCloneServices() = default;
};
void* bone_model_clone_00b912e0(const void* source, const std::string& name, const std::vector<std::string>& bone_names,
    float word_178, float word_17c, std::uint32_t flags, void* parent, BoneCloneServices& s);
// Catch_All funclets 00B7F682 / 00B7F712: free the buffer at [EBP-14h] and rethrow.
void catch_all_free_and_rethrow(void* buffer, const std::function<void(void*)>& free_block);

// oct18b-nC additions. New C++ interfaces; original register/stack ABIs and
// evidence are in reports/oct18b_nc_high.json. Names remain hypotheses.
// 00B6D440/630/6D0: ECX=left, stack output/right-or-scalar, EAX=output, RET 8.
float* matrix_scale_00b6d440(const float* left, float* output, float scale);
float* matrix_add_00b6d630(const float* left, float* output, const float* right);
float* matrix_subtract_00b6d6d0(const float* left, float* output, const float* right);
// 00B50020: same ABI as matrix_add, four components only.
float* vector4_add_00b50020(const float* left, float* output, const float* right);
// 00B6DE40: ECX=output, EDX=base; stack target, pointer-to-weight; RET 8.
float* matrix_blend_00b6de40(float* output, const float* base,
    const float* target, const float* weight);
// Native Win32 object layout. ECX=node, RET. Conditional recursive invalidation.
void invalidate_world_subtree_00b6da00(void* node);
struct NodeBlendCalls {
    virtual bool sample_slot18(void* animator, float* output, std::uint32_t animation, float time) = 0;
    virtual void set_matrix_slot38(void* node, const float* matrix) = 0;
protected:
    ~NodeBlendCalls() = default;
};
// 00B6E240: ECX=node; stack animation,time,weight,recurse; RET 10h.
// The image overwrites sample storage with identity when sampling returns
// AL!=0. For AL==0 it retains callback writes; without an animator it does not
// write the region. The semantic meaning of that byte is unproven. The caller
// supplies the entry contents of that native scratch region; no fallback is
// invented for the indeterminate native failure path. Each recursion obtains
// its own scratch through the required callback.
using BlendScratch = float* (*)(void* node);
void blend_animation_subtree_00b6e240(void* node, std::uint32_t animation,
    float time, float weight, std::uint32_t recurse, NodeBlendCalls& calls,
    BlendScratch scratch);
struct CameraAimCalls {
    virtual void set_position_slot2c(void* camera, const float* position) = 0;
    virtual const float* build_look_at_00b63f10(float* output, const float* target,
        const float* position, const float* up) = 0;
    virtual const float* invert_00b63b30(float* output, const float* matrix) = 0;
    virtual void set_matrix_slot34(void* camera, const float* matrix) = 0;
protected:
    ~CameraAimCalls() = default;
};
// 00B70320: ECX=camera; stack position,direction; RET 8 (Ghidra missed arg 2).
void camera_set_position_direction_00b70320(void* camera, const float* position,
    const float* direction, CameraAimCalls& calls);
struct CollectNodeCalls {
    virtual bool is_type_slot0c(void* node, std::uint32_t type) = 0;
    virtual void reserve_0059e5e0(void* array, std::int32_t capacity) = 0;
protected:
    ~CollectNodeCalls() = default;
};
// 00B85430: ECX=raw pointer-array, EDX=node; stack type, RET 4.
void collect_typed_subtree_00b85430(void* array, void* node, std::uint32_t type,
    CollectNodeCalls& calls);
// Descriptor-owner getters: MOV EAX,imm; RET. Values are image addresses,
// not dereferenceable pointers in this reconstruction's process.
std::uint32_t mesh_resource_owner_00b930a0() noexcept;
std::uint32_t skin_mesh_resource_owner_00b931a0() noexcept;
std::uint32_t bone_mesh_resource_owner_00b932a0() noexcept;
using BoneResourceFactory = void* (*)(std::uint32_t owner_image_address);
void* bone_mesh_resource_create_00b93250(BoneResourceFactory create_00b931f0);
// 00BA70B0: ECX=object, LEA EAX,[ECX+8Ch], RET; class is provisional.
void* object_field8c_00ba70b0(void* object) noexcept;

}  // namespace bsp::cc13_scene2
