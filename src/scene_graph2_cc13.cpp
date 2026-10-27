#include "bsp/scene_graph2_cc13.hpp"

#include "bsp/post_effects_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cctype>
#include <cmath>
#include <stdexcept>

// Segments 88/91, second scene-graph pass (cc13-scene2). Evidence and ABIs:
// docs/CC13_SCENE_2.md. x87 expressions are evaluated in double and rounded to float
// wherever the native code stores a float.

namespace bsp::cc13_scene2 {
namespace {

float f(double v) noexcept { return static_cast<float>(v); }

bool contains(const std::uint32_t* list, std::size_t n, std::uint32_t address) noexcept {
    for (std::size_t i = 0; i < n; ++i)
        if (list[i] == address) return true;
    return false;
}

// Byte-identical (after relocation) pointer-array bodies, grouped by shape.
constexpr std::uint32_t kAllocate[] = {0x00B6DEA0, 0x00B6FFF0, 0x00B72900, 0x00B74570, 0x00B7ADE0, 0x00B7AE80};
constexpr std::uint32_t kGrow[] = {0x00B6DEF0, 0x00B70040, 0x00B72950, 0x00B745C0, 0x00B7AE30,
    0x00B7AED0, 0x00B8B830, 0x00B8E930, 0x00B8FCF0, 0x00B8FD90};
constexpr std::uint32_t kPushBack[] = {0x00B6E580, 0x00B71260, 0x00B72E80, 0x00B747D0, 0x00B7B410,
    0x00B7B510, 0x00B8C430, 0x00B8EDC0, 0x00B90280, 0x00B90380};
constexpr std::uint32_t kReserve[] = {0x00B6E610, 0x00B712F0, 0x00B72F10, 0x00B74860, 0x00B7B4A0, 0x00B7B5A0};
constexpr std::uint32_t kPushViaReserveDouble[] = {0x00B6E940, 0x00B775B0, 0x00B7BC00, 0x00B7BE90,
    0x00B8A380, 0x00B8A430};

std::size_t count_of(const std::uint32_t (&)[6]) { return 6; }
std::size_t count_of(const std::uint32_t (&)[10]) { return 10; }

}  // namespace

// ---------------------------------------------------------------------------
// Arrays.

bool is_pointer_array_routine(std::uint32_t address) noexcept {
    return contains(kAllocate, 6, address) || contains(kGrow, 10, address) || contains(kPushBack, 10, address)
        || contains(kReserve, 6, address) || contains(kPushViaReserveDouble, 6, address)
        || address == 0x00B73800 || address == 0x00B7D9D0;
}

void allocate_pointer_array(std::uint32_t address, PointerArray& a, std::int32_t n) {
    (void)count_of(kAllocate);
    if (contains(kAllocate, 6, address)) array_reallocate_exact(a, n);
}

void grow_pointer_array(std::uint32_t address, PointerArray& a) {
    if (contains(kGrow, 10, address)) array_grow_double_plus_two(a);
}

void push_back_pointer_array(std::uint32_t address, PointerArray& a, void* value) {
    if (contains(kPushBack, 10, address)) array_push_back_grow(a, value);
}

void reserve_pointer_array(std::uint32_t address, PointerArray& a, std::uint32_t n) {
    if (contains(kReserve, 6, address)) array_reserve_unsigned(a, n);
}

void push_back_pointer_via_reserve(std::uint32_t address, PointerArray& a, void* value,
    const std::function<void(PointerArray&, std::int32_t)>& reserve) {
    CapacityRule rule = nullptr;
    if (contains(kPushViaReserveDouble, 6, address)) rule = capacity_double_min_one;
    else if (address == 0x00B73800) rule = capacity_double_min_four;
    else if (address == 0x00B7D9D0) rule = capacity_plus_four_min_eight;
    if (rule) array_push_back_via_reserve(a, value, rule, reserve);
}

void push_back_animation_key_00b77490(NativeArray<AnimKeyRecord>& a, const AnimKeyRecord& key,
    const std::function<void(NativeArray<AnimKeyRecord>&, std::int32_t)>& reserve) {
    array_push_back_via_reserve(a, key, capacity_double_min_one, reserve);
}

void push_back_pair_00b775f0(NativeArray<PairRecord>& a, const PairRecord& value,
    const std::function<void(NativeArray<PairRecord>&, std::int32_t)>& reserve) {
    array_push_back_via_reserve(a, value, capacity_double_min_one, reserve);
}

void reserve_dump_entries_00b7d6a0(NativeArray<DumpRecord>& a, std::int32_t n) {
    array_reserve_min_one(a, n);
}

void resize_dump_entries_00b7d820(NativeArray<DumpRecord>& a, std::int32_t n) {
    // The shrink loop at 00B7D884 only decrements count down to n; the final store makes it n.
    array_resize_zero_fill<DumpRecord>(a, n, reserve_dump_entries_00b7d6a0);
}

void push_back_dump_entry_00b7da10(NativeArray<DumpRecord>& a, const DumpRecord& value) {
    array_push_back_via_reserve<DumpRecord>(a, value, capacity_double_min_one, reserve_dump_entries_00b7d6a0);
}

// ---------------------------------------------------------------------------
// Reference counting.

RefCounted** release_ref_slot(RefCounted** slot) {
    if (RefCounted* p = *slot) {
        if (--p->refcount == 0) p->destroy_slot0();
        *slot = nullptr;
    }
    return slot;
}

void assign_retained_ref_00b74ed0(RefCounted*& slot_174, RefCounted* value) {
    RefCounted* old = slot_174;
    if (old == value) return;
    slot_174 = value;
    if (value) ++value->refcount;
    if (old && --old->refcount == 0) old->destroy_slot0();
}

void assign_target_ref_00b77ed0(RefCounted*& slot_438, RefCounted* value) {
    if (RefCounted* old = slot_438) {
        if (--old->refcount == 0) old->destroy_slot0();
        slot_438 = nullptr;
    }
    slot_438 = value;
    if (value) ++value->refcount;  // the NEG/AND mask at 00B77F0B is non-zero exactly when value != 0
}

void delete_owned_slot(OwnedVirtual** slot) {
    if (OwnedVirtual* p = *slot) {
        p->scalar_delete_slot1(1);
        *slot = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Scene types.

void initialize_derived_type(bool& guard, SceneTypeDescriptor& target, const char* name,
    const std::function<void()>& initialize_parent, const SceneTypeDescriptor& parent,
    SceneTypeCounter& counter) {
    if (guard) return;
    guard = true;
    target.name = name;
    initialize_parent();
    target.ids.assign(1, 0);
    target.ids.insert(target.ids.end(), parent.ids.begin(), parent.ids.end());
    target.ids[0] = counter.next_id++;
}

namespace {

void initialize_animator_child(AnimatorTypeState& s, bool& guard, SceneTypeDescriptor& target, const char* name,
    const std::function<void()>& initialize_root, const SceneTypeDescriptor& root, SceneTypeCounter& counter) {
    if (guard) return;
    guard = true;
    target.name = name;
    if (!s.animator_guard_010900c5) {
        s.animator_guard_010900c5 = true;
        s.animator_010900e0.name = "c3dNodeAnimator";
        initialize_root();
        const std::uint32_t root_id = root.ids.empty() ? 0 : root.ids[0];  // [0109DB84]
        s.animator_010900e0.ids = {counter.next_id++, root_id};
    }
    target.ids = {0, s.animator_010900e0.ids[0], s.animator_010900e0.ids[1]};
    target.ids[0] = counter.next_id++;
}

}  // namespace

void initialize_camera_animator_type_00b78170(AnimatorTypeState& s, SceneTypeDescriptor& target,
    const std::function<void()>& initialize_root, const SceneTypeDescriptor& root, SceneTypeCounter& counter) {
    initialize_animator_child(s, s.camera_animator_guard_010900c6, target, "cCameraAnimator", initialize_root, root,
        counter);
}

void initialize_optimized_animator_type_00b781f0(AnimatorTypeState& s, SceneTypeDescriptor& target,
    const std::function<void()>& initialize_root, const SceneTypeDescriptor& root, SceneTypeCounter& counter) {
    initialize_animator_child(s, s.optimized_guard_010900c7, target, "cOptimized3dNodeAnimator", initialize_root,
        root, counter);
}

bool scene_type_has_id(const SceneTypeDescriptor& d, std::uint32_t token) noexcept {
    for (std::uint32_t id : d.ids)
        if (id == token) return true;
    return false;
}

namespace {

constexpr SceneTypeSlot kSceneTypeSlots[] = {
    {0x00B6D790, 0x0108FF90, 2, SceneTypeSlotKind::id, "c3dNode"},
    {0x00B6D7A0, 0x0108FF90, 2, SceneTypeSlotKind::name, "c3dNode"},
    {0x00B6FB70, 0x0108FFA0, 3, SceneTypeSlotKind::name, "cCamera"},
    {0x00B72630, 0x0108FFEC, 2, SceneTypeSlotKind::id, "mesh object (name not recovered)"},
    {0x00B72640, 0x0108FFEC, 2, SceneTypeSlotKind::name, "mesh object (name not recovered)"},
    {0x00B73E30, 0x0108FFEC, 2, SceneTypeSlotKind::is_type, "mesh object (name not recovered)"},
    {0x00B74370, 0x01090044, 3, SceneTypeSlotKind::id, "cDummy"},
    {0x00B74380, 0x01090044, 3, SceneTypeSlotKind::name, "cDummy"},
    {0x00B75D00, 0x010900C8, 3, SceneTypeSlotKind::id, "animation channel group (name not recovered)"},
    {0x00B75D10, 0x010900C8, 3, SceneTypeSlotKind::name, "animation channel group (name not recovered)"},
    {0x00B78600, 0x010900C8, 3, SceneTypeSlotKind::is_type, "animation channel group (name not recovered)"},
    {0x00B75D60, 0x010900E0, 2, SceneTypeSlotKind::id, "c3dNodeAnimator"},
    {0x00B75D70, 0x010900E0, 2, SceneTypeSlotKind::name, "c3dNodeAnimator"},
    {0x00B780A0, 0x010900E0, 2, SceneTypeSlotKind::is_type, "c3dNodeAnimator"},
    {0x00B75E70, 0x010900FC, 3, SceneTypeSlotKind::id, "cOptimized3dNodeAnimator"},
    {0x00B75E80, 0x010900FC, 3, SceneTypeSlotKind::name, "cOptimized3dNodeAnimator"},
    {0x00B7AA00, 0x01090110, 2, SceneTypeSlotKind::id, "cAmbientLight"},
    {0x00B7AA10, 0x01090110, 2, SceneTypeSlotKind::name, "cAmbientLight"},
    {0x00B7C420, 0x01090110, 2, SceneTypeSlotKind::is_type, "cAmbientLight"},
    {0x00B7AA90, 0x0109018C, 3, SceneTypeSlotKind::id, "cLight"},
    {0x00B7AAA0, 0x0109018C, 3, SceneTypeSlotKind::name, "cLight"},
    {0x00B7AB00, 0x0109019C, 4, SceneTypeSlotKind::id, "cDirectionalLight"},
    {0x00B7AB10, 0x0109019C, 4, SceneTypeSlotKind::name, "cDirectionalLight"},
    {0x00B7AB40, 0x010901B0, 4, SceneTypeSlotKind::id, "cPointLight"},
    {0x00B7AB50, 0x010901B0, 4, SceneTypeSlotKind::name, "cPointLight"},
    {0x00B91250, 0x010903C8, 4, SceneTypeSlotKind::is_type, "vtable 00D63590 class (provisional)"},
    {0x00B91620, 0x01090344, 4, SceneTypeSlotKind::is_type, "skin model (00B8F920 returns its id)"},
    {0x00B91760, 0x010903E0, 3, SceneTypeSlotKind::is_type, "quadtree node (00B916F0 returns its id)"},
    {0x00B92E90, 0x0109042C, 3, SceneTypeSlotKind::is_type, "compact track item (00B922C0 returns its id)"},
};

}  // namespace

const SceneTypeSlot* find_scene_type_slot(std::uint32_t address) noexcept {
    for (const SceneTypeSlot& s : kSceneTypeSlots)
        if (s.address == address) return &s;
    return nullptr;
}

const SceneTypeSlot* scene_type_slots(std::size_t& count) noexcept {
    count = sizeof(kSceneTypeSlots) / sizeof(kSceneTypeSlots[0]);
    return kSceneTypeSlots;
}

// ---------------------------------------------------------------------------
// Nodes.

void detach_all_children_00b6d820(SceneNode& node) {
    for (SceneNode* c = node.first_child; c; c = c->next_sibling) c->parent = nullptr;
    node.first_child = nullptr;
    node.child_count = 0;
}

void copy_vector_08_00b6dc00(const SceneNode& node, float out[4]) {
    for (int i = 0; i < 4; ++i) out[i] = node.vector_08[i];
}

void set_word_50_00b6dca0(SceneNode& node, std::uint32_t value) { node.word_50 = value; }

void apply_animation_by_name_00b6e110(SceneNode& node, const std::string& name, float time, SceneNodeServices& s) {
    const std::int32_t index = s.find_animation_index_00b78670(node, name);
    if (index != -1) s.node_apply_animation_slot_24(node, index, time, 1);
}

void refresh_world_recursive_00b6e380(SceneNode& node, SceneNodeServices& s) {
    if (!(node.flags_5c & kNodeWorldValid)) s.refresh_world_00b6db70(node);
    s.node_slot_48(node);
    for (SceneNode* c = node.first_child; c; c = c->next_sibling) refresh_world_recursive_00b6e380(*c, s);
}

bool ensure_inverse_world_00b6fc70(SceneNode& node, SceneNodeServices& s) {
    if (node.flags_5c & kNodeInverseValid) return false;
    if (!(node.flags_5c & kNodeWorldValid)) s.refresh_world_00b6db70(node);
    s.store_inverse_world_00b63b30(node);
    node.flags_5c |= kNodeInverseValid;
    return true;
}

void refresh_leaf_worlds_00b72080(SceneNode& node, SceneNodeServices& s) {
    if (node.child_count == 0) {
        if (!(node.flags_5c & kNodeWorldValid)) s.refresh_world_00b6db70(node);
        return;
    }
    for (SceneNode* c = node.first_child; c; c = c->next_sibling) refresh_leaf_worlds_00b72080(*c, s);
}

bool native_name_equal(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    return true;
}

namespace {

// The 00B72300 exclusion test compares with _stricmp only (no length pre-check), which
// gives the same answer as native_name_equal for NUL-free names.
bool stricmp_equal(const std::string& a, const std::string& b) noexcept { return native_name_equal(a, b); }

}  // namespace

SceneNode* find_node_by_name_00b72270(const SceneRoots& scene, const std::string& name, bool recursive,
    SceneNodeServices& s) {
    for (SceneNode* n = scene.first; n; n = n->next_sibling)
        if (native_name_equal(n->name, name)) return n;
    if (!recursive) return nullptr;
    for (SceneNode* n = scene.first; n; n = n->next_sibling)
        if (SceneNode* hit = s.find_child_by_name_00b6e750(*n, name, true)) return hit;
    return nullptr;
}

SceneNode* find_node_by_name_excluding_00b72300(const SceneRoots& scene, const std::string& name,
    const std::string& excluded, SceneNodeServices& s) {
    for (SceneNode* n = scene.first; n; n = n->next_sibling)
        if (native_name_equal(n->name, name)) return n;
    for (SceneNode* n = scene.first; n; n = n->next_sibling) {
        if (stricmp_equal(n->name, excluded)) continue;
        if (SceneNode* hit = s.find_child_by_name_00b6e750(*n, name, true)) return hit;
    }
    return nullptr;
}

void build_quadtree_if_type_00b72000(SceneNode& node, QuadTreeBuildServices& s) {
    if (s.node_is_type_slot_0c(node, s.quadtree_type_00b916f0())) s.build_quadtree_00b91d70(node);
}

void build_quadtrees_for_roots_00b72030(const SceneRoots& scene, QuadTreeBuildServices& s) {
    for (SceneNode* n = scene.first; n; n = n->next_sibling) build_quadtree_if_type_00b72000(*n, s);
}

// ---------------------------------------------------------------------------
// Bounds.

namespace {

// Shared vertex sweep of 00B732D0 / 00B734D0 (identical loops; only the cache differs).
// The x87 compare is FCOMIP + JBE: an unordered pair leaves the bound unchanged.
void sweep_positions(const std::vector<VertexStreamView>& streams, float mn[3], float mx[3]) {
    for (int k = 0; k < 3; ++k) {
        mn[k] = kBoundsInitialMin;
        mx[k] = kBoundsInitialMax;
    }
    for (const VertexStreamView& v : streams) {
        if (v.position_type != 2) continue;
        const std::uint8_t* p = v.vertices + v.offset;
        for (std::int32_t i = 0; i < v.vertex_count; ++i, p += v.stride) {
            float xyz[3];
            std::memcpy(xyz, p, sizeof xyz);
            for (int k = 0; k < 3; ++k) {
                if (mn[k] > xyz[k]) mn[k] = xyz[k];
                if (xyz[k] > mx[k]) mx[k] = xyz[k];
            }
        }
    }
}

}  // namespace

bool compute_cached_aabb_00b732d0(bool& valid_94, Aabb& out_98, const std::vector<VertexStreamView>& streams) {
    if (valid_94) return false;
    sweep_positions(streams, out_98.min, out_98.max);
    valid_94 = true;
    return true;
}

bool compute_cached_sphere_00b734d0(bool& valid_80, Sphere& out_84, const std::vector<VertexStreamView>& streams) {
    if (valid_80) return false;
    float mn[3], mx[3];
    sweep_positions(streams, mn, mx);
    float h[3];
    for (int k = 0; k < 3; ++k) {
        const float d = f(static_cast<double>(mx[k]) - mn[k]);  // 00B73670 FSUBP, FSTP float
        h[k] = f(0.5 * d);                                      // D7A280 0.5 (DC C9 at 00B736A6)
        out_84.centre[k] = f(static_cast<double>(mn[k]) + h[k]);
    }
    const float hy2 = f(static_cast<double>(h[1]) * h[1]);
    const float hx2 = f(static_cast<double>(h[0]) * h[0]);
    const double xy = static_cast<double>(hy2) + hx2;           // 00B73706, kept on the stack
    const float hz2 = f(static_cast<double>(h[2]) * h[2]);
    const float sum = f(xy + hz2);
    out_84.radius = f(std::sqrt(static_cast<double>(sum)));     // 00BF7030
    valid_80 = true;
    return true;
}

void reset_sphere_cache_00b72b50(bool& valid_80, Sphere& sphere_84) {
    valid_80 = false;
    sphere_84 = Sphere{{0, 0, 0}, 0};
}

std::uint32_t tree_group_slot_offset_00b732b0(std::int32_t index) noexcept {
    return static_cast<std::uint32_t>((index + 1) * 0x10);
}

void zero_vector4_00b74410(float v[4]) {
    for (int i = 0; i < 4; ++i) v[i] = 0;
}

// ---------------------------------------------------------------------------
// Optimized animator.

std::int32_t baked_frame_index_00b92500(const BakedTrack& track, std::int32_t channel, float time) {
    const BakedChannel& ch = track.channels.at(static_cast<std::size_t>(channel));
    const float u0 = f(static_cast<double>(time) / ch.duration);
    const float u = f(std::fmod(static_cast<double>(u0), 1.0));
    const std::int32_t frames = ch.word_count / track.nodes_per_frame - 1;  // CDQ/IDIV
    const std::int32_t frame = static_cast<std::int32_t>(static_cast<double>(u) * frames);  // FIMUL, 00BF7420
    return frame * track.nodes_per_frame;
}

// Each element moves by FLD / FSTP DWORD, which quiets a signalling NaN (cc13-fixture6).
void matrix_from_transposed_3x4_00b75de0(float out[16], const float src[12]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 4; ++c) out[c * 4 + r] = cc13x87::x87_fld_fstp(src[r * 4 + c]);
    out[3] = 0;
    out[7] = 0;
    out[11] = 0;
    out[15] = 1.0f;  // [00D7A24C]
}

bool optimized_evaluate_pose_00b75e90(bool enabled, const BakedTrack* track, std::int32_t channel, float time,
    float out[16]) {
    if (!enabled || !track) return false;
    const std::int32_t index = baked_frame_index_00b92500(*track, channel, time);
    const BakedChannel& ch = track->channels.at(static_cast<std::size_t>(channel));
    matrix_from_transposed_3x4_00b75de0(out, ch.frames + static_cast<std::ptrdiff_t>(index) * 12);
    return true;
}

bool optimized_evaluate_channel_00b75ed0() noexcept { return false; }

std::int32_t find_baked_channel_00b92560(const std::vector<std::string>& names, const std::string& name) {
    for (std::size_t i = 0; i < names.size(); ++i)
        if (native_name_equal(names[i], name)) return static_cast<std::int32_t>(i);
    return -1;
}

float registry_float_00b77480(const std::vector<float>& values_14, std::int32_t index) {
    return values_14.at(static_cast<std::size_t>(index));
}

// ---------------------------------------------------------------------------
// Camera.

void camera_get_190_00b6fe60(const CameraView& c, std::uint32_t& out) { out = c.word_190; }
void camera_set_194_00b6fe70(CameraView& c, std::uint32_t value) { c.word_194 = value; }

void camera_set_eye_and_target_00b70200(CameraView& c, const float eye[3], const float target[3],
    const float up[3], CameraServices& s) {
    c.flags_2f0 &= 0xFFFFFE4Bu;
    s.camera_slot_30_set_position(c, eye);
    for (int k = 0; k < 3; ++k) c.target_1a0[k] = target[k];
    float d[3];
    for (int k = 0; k < 3; ++k) d[k] = f(static_cast<double>(c.target_1a0[k]) - eye[k]);
    const float length = s.vector_length_00419440(d);
    const float inv = length > 0.0f ? f(1.0 / length) : 0.0f;  // FCOMI + JBE: NaN takes the zero arm
    for (int k = 0; k < 3; ++k) c.direction_1ac[k] = f(static_cast<double>(d[k]) * inv);
    s.look_at_and_apply_slot_34(c, eye, c.target_1a0, up);
}

void camera_set_local_position_00b713d0(CameraView& c, const float position[3], CameraServices& s) {
    c.flags_2f0 &= 0xFFFFFE4Bu;
    s.set_local_position_00b6dab0(c, position);
    s.refresh_direction_00b70660(c);
}

std::uint32_t camera_classify_point_00b714d0(CameraView& c, const float p[3], CameraServices& s) {
    if (!(c.flags_2f0 & 4)) {
        c.flags_2f0 |= 4;
        s.rebuild_frustum_planes(c);
    }
    return s.classify_point_00b65140(c, p);
}

std::uint32_t camera_classify_aabb_00b71590(CameraView& c, const float box[6], CameraServices& s) {
    if (!(c.flags_2f0 & 4)) {
        c.flags_2f0 |= 4;
        s.rebuild_frustum_planes(c);
    }
    return s.classify_aabb_00b65710(c, box);
}

void camera_apply_oblique_clip_00b70760(CameraView& c, float projection[16], CameraServices& s) {
    float plane[4];
    for (int k = 0; k < 4; ++k) plane[k] = c.clip_plane_2e0[k];
    s.ensure_view_matrix(c);
    s.transform_plane_00b65c30(c, plane);
    auto sign = [](float v) -> std::int32_t { return v < 0.0f ? -1 : (v > 0.0f ? 1 : 0); };
    float* P = projection;
    const float qx = f(static_cast<double>(sign(plane[0])) / P[0]);   // FILD, FDIV [EAX]
    const float qy = f(static_cast<double>(sign(plane[1])) / P[5]);   // FILD, FDIV [EAX+14h]
    const float qw = f((1.0 - P[10]) / P[14]);
    // 00B7084C..00B70880: ((qx*px + qy*py) + pz) + qw*pw on the x87 stack, stored as float.
    const double dot = ((static_cast<double>(qx) * plane[0] + static_cast<double>(qy) * plane[1]) + plane[2])
        + static_cast<double>(qw) * plane[3];
    const float dot_f = f(dot);
    const float k = f(1.0 / dot_f);
    P[2] = f(static_cast<double>(k) * plane[0]);
    P[6] = f(static_cast<double>(k) * plane[1]);
    P[10] = f(static_cast<double>(k) * plane[2]);
    P[14] = f(static_cast<double>(k) * plane[3]);
}

void camera_projection_scale_00b70550(const float projection[16], float out_x[4], float out_y[4]) {
    out_x[0] = projection[0];
    out_x[1] = projection[4];
    out_x[2] = 0;
    out_x[3] = 0;
    out_y[0] = projection[1];
    out_y[1] = projection[5];
    out_y[2] = 0;
    out_y[3] = 0;
}

void camera_projection_scale_words_00b705d0(const float projection[16], float* out[8]) {
    float x[4], y[4];
    camera_projection_scale_00b70550(projection, x, y);
    for (int i = 0; i < 4; ++i) {
        *out[i] = x[i];
        *out[4 + i] = y[i];
    }
}

void camera_copy_construct_00b71d10(void* self, CameraCopyFields& dst, const void* source,
    const std::string& source_name, const CameraCopyFields& src, CameraCopyServices& s) {
    s.construct_node_00b6f5a0(self, source_name);
    s.set_vtable(self, 0x00D62CF0);
    for (float& v : dst.zero_1b8) v = 0;
    s.construct_plane_set_00b659d0(self);
    dst.target_438 = nullptr;
    if (!s.source_world_valid(source)) s.refresh_world_00b6db70(source);
    dst.flags_2f0 &= 0xFFFFFE4Bu;
    s.set_world_matrix_00b6e870(self, source);
    s.refresh_direction_00b70660(self);
    s.copy_projection_00b6fcf0(self, source);
    dst.flags_2f0 = (dst.flags_2f0 & 0xFFFFFF4Bu) | 8u;
    s.assign_name_00b6f960(self, source_name);
    dst.word_180 = 0;
    dst.word_184 = 0;
    for (int i = 0; i < 6; ++i) dst.projection_params_1c4[i] = src.projection_params_1c4[i];
    dst.flags_2f0 = src.flags_2f0 & 0xFFFFFE41u;
    dst.word_198 = src.word_198;
    dst.word_188 = src.word_188;
    dst.word_190 = src.word_190;
    dst.float_18c = src.float_18c;
    dst.word_194 = src.word_194;
    dst.float_1dc = src.float_1dc;
    dst.byte_17c = src.byte_17c;
    for (int i = 0; i < 6; ++i) dst.tail_440[i] = src.tail_440[i];
}

// ---------------------------------------------------------------------------
// Clones and deleting destructors.

void* clone_node_00b6f8f0(const void* source, const std::string& name, std::uint32_t flags, void* parent,
    NodeCloneServices& s) {
    void* slot = s.allocate_node_slot_00b6eb00();
    void* clone = slot ? s.construct_node_00b6f5a0(slot, name) : nullptr;
    s.copy_clone_base_state_00b6f150(source, clone, flags, parent);
    return clone;
}

void* clone_draw_section_00b725a0(const void* source, const std::function<void*()>& allocate,
    const std::function<void*(void*, const void*)>& copy_construct) {
    void* slot = allocate();
    return slot ? copy_construct(slot, source) : nullptr;
}

void* light_scalar_deleting_destructor_00b7c800(void* self, std::uint32_t flags,
    const std::function<void(void*)>& destroy, const std::function<void(void*)>& return_slot) {
    destroy(self);
    if (flags & 1) return_slot(self);
    return self;
}

namespace {

constexpr SingletonBaseDelete kSingletonBaseDeletes[] = {
    {0x00B7D410, 0x0109033C, "group-params parser"},
    {0x00B7D450, 0x0109047C, "mesh parser"},
    {0x00B7D490, 0x01090480, "skinned-mesh parser"},
    {0x00B7D4D0, 0x01090484, "matrix-indexed-mesh parser"},
    {0x00B7D510, 0x010902A0, "camera parser"},
    {0x00B7D550, 0x0109043C, "skinned-mesh-animation parser"},
    {0x00B7D9A0, 0x010901C4, "resource manager"},
};

constexpr IteratorWrapper kIteratorWrappers[] = {
    {0x00B776D0, 0x00B76AE0, true}, {0x00B77A80, 0x00B76AE0, true},
    {0x00B776F0, 0x00B76BB0, false}, {0x00B77DE0, 0x00B76BB0, false},
    {0x00B7D8B0, 0x00B7CE80, true}, {0x00B7DED0, 0x00B7CE80, true},
    {0x00B7D8C0, 0x00B7CDF0, false}, {0x00B7DEE0, 0x00B7CDF0, false},
    {0x00B7D900, 0x00B7CF80, true}, {0x00B7DF20, 0x00B7CF80, true},
    {0x00B7D910, 0x00B7CEF0, false}, {0x00B7DF30, 0x00B7CEF0, false},
};

constexpr TypeInitThunk kTypeInitThunks[] = {
    {0x00B78510, 0x00B78170}, {0x00B78520, 0x00B781F0}, {0x00B7C270, 0x00B7C0C0}, {0x00B7C280, 0x00B7C110},
    {0x00B91570, 0x00B90FB0}, {0x00B91580, 0x00B911D0}, {0x00B93D10, 0x00B93C20}, {0x00B93D20, 0x00B93C70},
};

}  // namespace

const SingletonBaseDelete* find_singleton_base_delete(std::uint32_t address) noexcept {
    for (const SingletonBaseDelete& d : kSingletonBaseDeletes)
        if (d.address == address) return &d;
    return nullptr;
}

void* singleton_base_scalar_delete(std::uint32_t address, void* self, std::uint32_t flags, void*& instance_global,
    std::uint32_t& vtable_word, const std::function<void(void*)>& free_block) {
    if (!find_singleton_base_delete(address)) return nullptr;
    instance_global = nullptr;
    vtable_word = 0x00CE3818;
    if (flags & 1) free_block(self);
    return self;
}

std::int32_t compare_dump_size_descending_00b7c920(const DumpRecord& a, const DumpRecord& b) noexcept {
    return static_cast<std::int32_t>(b.words[1] - a.words[1]);
}

const IteratorWrapper* find_iterator_wrapper(std::uint32_t address) noexcept {
    for (const IteratorWrapper& w : kIteratorWrappers)
        if (w.address == address) return &w;
    return nullptr;
}

const TypeInitThunk* find_type_init_thunk(std::uint32_t address) noexcept {
    for (const TypeInitThunk& t : kTypeInitThunks)
        if (t.address == address) return &t;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Lua script cache.

void reset_global_scripts_unless_loaded_00b6d3c0(const std::vector<std::string>& names,
    std::vector<std::string>& global_names_0108ff30, void* global_records_0108ff40, LuaScriptCacheServices& s) {
    if (s.names_equal_00b6c990(names, global_names_0108ff30)) return;
    s.clear_globals_00b6cf90();
    if (&global_names_0108ff30 != &names) global_names_0108ff30 = names;  // 00506C80 guards self-assignment
    s.build_compressed_00b6d1b0(global_names_0108ff30, global_records_0108ff40);
}

void reload_script_cache_00b6d400(void* cache, std::vector<std::string>& cache_names, void* cache_records_10,
    const std::vector<std::string>& names, LuaScriptCacheServices& s) {
    if (s.names_equal_00b6c990(names, cache_names)) return;
    s.clear_cache_00b6d0a0(cache);
    if (&cache_names != &names) cache_names = names;
    s.build_compressed_00b6d1b0(names, cache_records_10);
}

// ---------------------------------------------------------------------------
// Strings and trees.

void copy_string_entry(NativeStringEntry& dst, const NativeStringEntry& src) {
    if (&dst != &src) dst.text.assign(src.text);
    dst.word_8 = src.word_8;
}

void construct_string_pair(NativeStringEntry& dst, const std::string& key, std::uint32_t value) {
    if (&dst.text != &key) dst.text.assign(key);
    dst.word_8 = value;
}

void release_string_storage(std::uint32_t length, const void* chars,
    const std::function<void(const void*, std::uint32_t)>& return_block) {
    if (chars) return_block(chars, length + 1);
}

void operator_delete_thunk(void* p, const std::function<void(void*)>& free_block) { free_block(p); }

void* allocate_tree_nodes_1c(std::uint32_t count, const std::function<void*(std::size_t)>& operator_new) {
    if (count != 0 && 0xFFFFFFFFu / count < 0x1Cu) throw std::bad_alloc();
    return operator_new(static_cast<std::size_t>(count) * 0x1C);
}

// ---------------------------------------------------------------------------
// Flare.

const float kFlareCorners[4][2] = {{-1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, -1.0f}, {-1.0f, -1.0f}};

namespace {

constexpr std::uint32_t kFlareWhiteNoAlpha = 0x00FFFFFFu;

std::uint32_t scaled_colour(const std::uint8_t rgba[4], float scale) {
    std::uint8_t out[4];
    bsp::cc13fx::scale_color32(rgba, out, scale);  // 00B8C520
    std::uint32_t v;
    std::memcpy(&v, out, 4);
    return v;
}

// The rotated corner shared by the three quad writers (00B8C6D0.., 00B8C960.., 00B8CD50..).
void rotated_corner(const FlareElement& e, int k, float& rx, float& ry) {
    const float cx = f(static_cast<double>(kFlareCorners[k][0]) * e.size[0]);
    const float cy = f(static_cast<double>(kFlareCorners[k][1]) * e.size[1]);
    const float c = f(std::cos(static_cast<double>(e.angle)));
    const float s = f(std::sin(static_cast<double>(e.angle)));
    rx = f(static_cast<double>(c) * cx - static_cast<double>(s) * cy);
    ry = f(static_cast<double>(c) * cy + static_cast<double>(s) * cx);
}

void write_rect_uv(FlareVertex quad[4], const FlareAtlasRect& r) {
    quad[0].u = r.u0; quad[0].v = r.v0;
    quad[1].u = r.u1; quad[1].v = r.v0;
    quad[2].u = r.u1; quad[2].v = r.v1;
    quad[3].u = r.u0; quad[3].v = r.v1;
}

void project_corner(FlareView& flare, const FlareElement& e, int k, FlareVertex& out, FlareServices& s) {
    FlareCameraView& cam = *flare.camera_1bc;
    float rx, ry;
    rotated_corner(e, k, rx, ry);
    float p[4];
    for (int i = 0; i < 3; ++i) {
        const float w = f(static_cast<double>(f(static_cast<double>(rx) * cam.right_f0[i]))
            + f(static_cast<double>(ry) * cam.up_100[i]));
        p[i] = f(static_cast<double>(flare.anchor_198[i]) + w);
    }
    p[3] = 1.0f;
    float clip[4];
    s.transform_view_projection(cam, p, clip);
    out.x = f(static_cast<double>(clip[0]) / clip[3]);
    out.y = f(static_cast<double>(clip[1]) / clip[3]);
    out.z = 0.9999f;  // [00D63418]
    out.w = 1.0f;
}

}  // namespace

void flare_write_screen_quad_00b8c6b0(const FlareView& flare, FlareVertex quad[4], const FlareElement& e) {
    for (int k = 0; k < 4; ++k) {
        float rx, ry;
        rotated_corner(e, k, rx, ry);
        quad[k].x = f(static_cast<double>(flare.ndc_1a4[0]) * e.scale + rx);
        quad[k].y = f(static_cast<double>(ry) * flare.camera_1bc->aspect_1c8
            + static_cast<double>(flare.ndc_1a4[1]) * e.scale);
        quad[k].z = 1.6e-5f;  // [00D63414]
        quad[k].colour = scaled_colour(e.rgba, flare.visibility_ac);
    }
    write_rect_uv(quad, *e.rects[0]);
}

void flare_write_projected_quad_00b8c8a0(FlareView& flare, FlareVertex quad[4], const FlareElement& e,
    FlareServices& s) {
    // The prologue checks the camera world twice, before reading row +F0h and row +100h.
    for (int i = 0; i < 2; ++i)
        if (!s.world_valid(flare.camera_1bc)) s.refresh_world_00b6db70(flare.camera_1bc);
    for (int k = 0; k < 4; ++k) {
        project_corner(flare, e, k, quad[k], s);
        quad[k].colour = scaled_colour(e.rgba, flare.visibility_ac);
        quad[k].colour2 = 0;
    }
    write_rect_uv(quad, *e.rects[0]);
}

void flare_write_projected_quad2_00b8cc90(FlareView& flare, FlareVertex quad[4], const FlareElement& e,
    FlareServices& s) {
    // The prologue checks the camera world twice, before reading row +F0h and row +100h.
    for (int i = 0; i < 2; ++i)
        if (!s.world_valid(flare.camera_1bc)) s.refresh_world_00b6db70(flare.camera_1bc);
    for (int k = 0; k < 4; ++k) {
        project_corner(flare, e, k, quad[k], s);
        quad[k].colour = scaled_colour(e.rgba, f(static_cast<double>(flare.visibility_ac) * 3.0));  // [00D7A2B0]
        quad[k].colour2 = scaled_colour(e.rgba2, flare.colour2_scale_184);
    }
    write_rect_uv(quad, *e.rects[0]);
    const float uv2[4][2] = {{-1.0f, -1.0f}, {-1.0f, 2.0f}, {2.0f, 2.0f}, {2.0f, -1.0f}};  // D7A260 / CE3958
    for (int k = 0; k < 4; ++k) {
        quad[k].u2 = uv2[k][0];
        quad[k].v2 = uv2[k][1];
    }
}

void flare_write_ring_00b8bb50(FlareVertex* vertices, std::uint16_t* indices, const float outer[2],
    const float inner[2], std::uint32_t n) {
    const float step = f(6.2831854820251465 / static_cast<double>(n));  // [CE3828], n as unsigned
    float angle = 0.0f;
    FlareVertex* v = vertices;
    for (std::uint32_t i = 0; i < n; ++i) {
        const float s = f(std::sin(static_cast<double>(angle)));
        const float c = f(std::cos(static_cast<double>(angle)));
        v->x = f(static_cast<double>(inner[0]) * c);
        v->y = f(static_cast<double>(inner[1]) * s);
        v->z = 0.0f;
        v->w = 1.0f;
        v->colour = kFlareWhiteNoAlpha;
        v->u = 0.5f;
        v->v = 0.5f;
        ++v;
        v->x = f(static_cast<double>(outer[0]) * c);
        v->y = f(static_cast<double>(outer[1]) * s);
        v->z = 0.0f;
        v->w = 1.0f;
        v->colour = kFlareWhiteNoAlpha;
        v->u = 0.0f;
        v->v = 0.0f;
        ++v;
        angle = f(static_cast<double>(step) + angle);
    }
    const std::uint32_t m = 2 * n;
    for (std::uint32_t i = 0; i < n; ++i) {
        const auto a = static_cast<std::uint16_t>(2 * i);
        const auto b = static_cast<std::uint16_t>((2 * i + 3) % m);
        *indices++ = a;
        *indices++ = static_cast<std::uint16_t>(2 * i + 1);
        *indices++ = b;
        *indices++ = a;
        *indices++ = b;
        *indices++ = static_cast<std::uint16_t>((2 * i + 2) % m);
    }
}

void flare_update_billboard_00b8bdd0(FlareView& flare, FlareServices& s) {
    if (!s.world_valid(&flare)) s.refresh_world_00b6db70(&flare);
    const float p[3] = {flare.world_120[0], flare.world_120[1], flare.world_120[2]};
    const float length = s.vector_length_00419440(p);
    const float inv = length > 0.0f ? f(1.0 / length) : 0.0f;
    const float n[3] = {f(static_cast<double>(p[0]) * inv), f(static_cast<double>(p[1]) * inv),
        f(static_cast<double>(inv) * p[2])};
    FlareVertex* v = s.lock_stream0(flare, 1);
    const float dot = f((static_cast<double>(n[0]) * 0.0 + n[1]) + 0.0 * n[2]);
    const float a = dot > 0.0f ? dot : f(-0.0 - static_cast<double>(dot));
    float up[3] = {0.0f, 1.0f, 0.0f};
    if (0.01f > a) {  // [00D7A238]
        up[1] = 0.0f;
        up[2] = -1.0f;  // [00D7A260]
    }
    float right[3];
    s.cross_004f9b30(right, n, up);
    FlareCameraView& cam = *flare.camera_1bc;
    if (!s.world_valid(&cam)) s.refresh_world_00b6db70(&cam);
    float d[3];
    for (int i = 0; i < 3; ++i) d[i] = f(static_cast<double>(n[i]) * 4500.0);  // [00D63408]
    // Centre: the rim formula with both radius terms 0; up.x is the folded constant 0.
    for (int i = 0; i < 3; ++i) {
        const float r = f(static_cast<double>(right[i]) * 0.0);
        const float u = f((i == 0 ? 0.0 : static_cast<double>(up[i])) * 0.0);
        const float q = f(static_cast<double>(f(static_cast<double>(r) + u)) + d[i]);
        (&v[0].x)[i] = f(static_cast<double>(q) + cam.position_120[i]);
    }
    v[0].w = 1.0f;
    v[0].colour = kFlareWhiteNoAlpha;
    float angle = 0.0f;
    for (int k = 1; k <= 8; ++k) {
        const float sn = f(std::sin(static_cast<double>(angle)));
        const float cs = f(std::cos(static_cast<double>(angle)));
        if (!s.world_valid(&cam)) s.refresh_world_00b6db70(&cam);
        const float C = f(static_cast<double>(cs) * 300.0);  // [00CE3CA8]
        const float S = f(300.0 * sn);
        for (int i = 0; i < 3; ++i) {
            const float r = f(static_cast<double>(right[i]) * C);
            const float u = f((i == 0 ? 0.0 : static_cast<double>(up[i])) * S);  // [00D7A258] = 0 for x
            const float q = f(static_cast<double>(f(static_cast<double>(r) + u)) + d[i]);
            (&v[k].x)[i] = f(static_cast<double>(q) + cam.position_120[i]);
        }
        v[k].w = 1.0f;
        v[k].colour = kFlareWhiteNoAlpha;
        v[k].u = 0.5f;  // [00CE3800]
        v[k].v = 0.5f;
        angle = f(static_cast<double>(angle) + 0.7853981852531433);  // [00CEDCD0]
    }
    s.unlock_stream0(flare, 1);
}

void flare_update_00b8d150(FlareView& flare, FlareCameraView& camera, float dt, FlareServices& s) {
    flare.timer_1c4 = f(static_cast<double>(dt) + flare.timer_1c4);
    if (camera.word_198 != 0) return;
    flare.camera_1bc = &camera;
    if (!s.world_valid(&camera)) s.refresh_world_00b6db70(&camera);
    if (!s.world_valid(&flare)) s.refresh_world_00b6db70(&flare);
    for (int i = 0; i < 3; ++i)
        flare.anchor_198[i] = f(static_cast<double>(flare.world_120[i]) + camera.position_120[i]);
    const float in[4] = {flare.anchor_198[0], flare.anchor_198[1], flare.anchor_198[2], 1.0f};
    s.transform_view_projection(camera, in, flare.clip_1ac);
    flare.ndc_1a4[0] = f(static_cast<double>(flare.clip_1ac[0]) / flare.clip_1ac[3]);
    flare.ndc_1a4[1] = f(static_cast<double>(flare.clip_1ac[1]) / flare.clip_1ac[3]);
    if (!s.world_valid(&camera)) s.refresh_world_00b6db70(&camera);
    float n[3];
    s.normalize_00419510(n, flare.anchor_198);
    flare.facing_1dc = f((static_cast<double>(n[1]) * camera.forward_110[1]
        + static_cast<double>(n[0]) * camera.forward_110[0]) + static_cast<double>(n[2]) * camera.forward_110[2]);
    const float edge = s.edge_fade_00b8b3b0(flare);
    const float fade = f(static_cast<double>(s.centre_weight_00b8b350(flare)) * edge);
    flare_update_billboard_00b8bdd0(flare, s);
    float excess = 0.0f;  // the slot [ESP+10h] that the element loop truncates
    bool counted = false;
    if (s.has_queries(flare)) {
        const std::uint32_t visible = s.visible_pixels(flare);
        const std::uint32_t total = s.total_pixels(flare);
        if (total != 0) {
            const float ratio = f(static_cast<double>(visible) / static_cast<double>(total));
            s.set_visibility_00b6da70(flare, f((static_cast<double>(ratio) * fade) * 0.699999988079071), false);
            float q = f(static_cast<double>(ratio) - 0.5);
            if (0.0f > q) q = 0.0f;
            flare.colour2_scale_184 = f((static_cast<double>(fade) * (static_cast<double>(q) * fade)) * 2.0);
            excess = q;
            counted = true;
        }
    }
    if (!counted) {
        s.set_visibility_00b6da70(flare, 0.0f, false);
        flare.colour2_scale_184 = 0.0f;
    }
    FlareVertex* v = s.lock_stream0(flare, 0);
    for (FlareElement* e : flare.elements) {
        if (e->place) s.place_element_00b8b8c0(flare, dt, *e);
        if (e->truncate_20) e->word_20 = static_cast<std::int32_t>(static_cast<std::int64_t>(excess));  // FISTP, RC=11b
        if (e->type == 0) flare_write_projected_quad_00b8c8a0(flare, v, *e, s);
        else if (e->type == 2) flare_write_projected_quad2_00b8cc90(flare, v, *e, s);
        else flare_write_screen_quad_00b8c6b0(flare, v, *e);
        v += 4;
    }
    s.unlock_stream0(flare, 0);
    s.set_visibility_00b6da70(flare, 1.0f, false);
}

void flare_construct_00b8d9c0(void* self, FlareStorage& f, void* parent, float duration, bool flag,
    FlareLifetimeServices& s) {
    s.construct_generated_model_00b75030(self, "flare");  // temporary NativeString, returned to the pool after
    f.vtable = 0x00D63420;
    f.elements.clear();
    f.ref_1cc = nullptr;
    s.set_parent_00b6e680(self, parent);
    f.timer_1c4 = 0.0f;
    f.flag_1c8 = flag;
    f.query_total_1d4 = nullptr;
    f.query_visible_1d0 = nullptr;
    f.duration_1c0 = duration;
}

void flare_destruct_00b8d8c0(void* self, FlareStorage& f, FlareLifetimeServices& s) {
    f.vtable = 0x00D63420;
    for (void** slot : {&f.ref_1cc, &f.query_visible_1d0, &f.query_total_1d4}) {
        if (*slot) {
            s.release_ref(*slot);
            *slot = nullptr;
        }
    }
    f.elements.clear();  // 0086AA10(0), then _free of the data pointer
    f.elements.shrink_to_fit();
    s.destroy_model_00b750c0(self);
}

void* flare_scalar_delete_00b8d9a0(void* self, FlareStorage& f, std::uint32_t flags, FlareLifetimeServices& s) {
    flare_destruct_00b8d8c0(self, f, s);
    if (flags & 1) s.pool_free_00b8c3a0(self);
    return self;
}

void* flare_operator_new_00b8d840(FlareLifetimeServices& s) { return s.pool_allocate_00b8d700(); }
void flare_operator_delete_00b8c6a0(void* p, FlareLifetimeServices& s) { s.pool_free_00b8c3a0(p); }

void assign_element_pointers_00b8d850(PointerArray& dst, const PointerArray& src,
    const std::function<void(PointerArray&, std::int32_t)>& resize,
    const std::function<void(PointerArray&, std::int32_t)>& reserve) {
    resize(dst, 0);
    reserve(dst, src.count);
    for (std::int32_t i = 0; i < src.count; ++i)
        array_push_back_via_reserve(dst, src.data[i], capacity_double_min_one, reserve);
}

void flare_initialize_occlusion_00b8dac0(void* flare, FlareStorage& f, FlareOcclusionServices& s) {
    void* material = s.create_material_00535320("flareocclusion.mshd");
    void* mesh = s.create_mesh_object();
    void* decl = s.create_declaration("simplecolor2.mvfm");
    void* vertices = s.create_vertex_buffer(9, 1, decl);
    s.release_ref(decl);
    s.set_vertex_stream_00b73bb0(mesh, 0, vertices);
    s.release_ref(vertices);
    void* indices = s.create_index_buffer(0x18, 1, 0x65);
    s.set_index_stream_00b73b70(mesh, indices);
    s.release_ref(indices);
    void* section = s.create_draw_section_00533fa0();
    s.set_section_fields(section, 4, 0, 9, 0, 8);
    s.set_section_material_00b864c0(section, material);
    s.rebuild_section_layout_00b865a0(section, mesh);
    s.append_section_00b73c60(mesh, section);
    s.release_ref(section);
    const float radii[2] = {1.0f, 1.0f};  // [00D7A24C]
    s.write_disc_fan_00b8b980(flare, mesh, radii, 8);
    s.set_model_geometry_00b75170(flare, 1, mesh, -1.0f, -1.0f);  // [00D7A260]
    s.release_ref(mesh);
    s.release_ref(material);
    f.query_visible_1d0 = s.create_query();
    f.query_total_1d4 = s.create_query();
}

// ---------------------------------------------------------------------------
// Mit pools.

void mit_pool_construct(MitPoolOwner& pool, MitAllocatorListElem*& list_head_00e188b4, std::uint32_t vtable) {
    pool.vtable = 0x00D7A0C0;
    pool.link.prev = nullptr;
    pool.link.next = list_head_00e188b4;
    if (list_head_00e188b4) list_head_00e188b4->prev = &pool.link;
    list_head_00e188b4 = &pool.link;
    pool.vtable = vtable;
    pool.recursion_24 = 0;  // after InitializeCriticalSection(+0Ch)
    pool.blocks = NativeArray<MitBlockModel*>{};
    pool.current_34 = -1;
    if (static_cast<std::uint32_t>(pool.blocks.capacity) < 32) array_reallocate_exact(pool.blocks, 32);
}

void mit_pool_destruct(MitPoolOwner& pool, MitAllocatorListElem*& list_head_00e188b4) {
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(pool.blocks.count); ++i) delete pool.blocks.data[i];
    if (pool.blocks.data) ::operator delete(pool.blocks.data);
    while (pool.recursion_24 > 0) --pool.recursion_24;  // LeaveCriticalSection per level, then Delete
    pool.vtable = 0x00D7A0C0;
    if (!pool.link.prev) list_head_00e188b4 = pool.link.next;
    else pool.link.prev->next = pool.link.next;
    if (pool.link.next) pool.link.next->prev = pool.link.prev;
}

void mit_pool_init_block(MitBlockModel& block, std::uint32_t index) {
    block.free_count = 32;
    block.slot_block_index.assign(32, index);
    for (int i = 0; i < 32; ++i) block.stack[i] = static_cast<std::uint16_t>(31 - i);
}

namespace {

constexpr MitPoolThunk kMitPoolThunks[] = {
    {0x00B90470, 0x01090358, 0x00B90020},  // skin model operator delete
    {0x00B90480, 0x01090390, 0x00B90150},  // bone model operator delete
    {0x00B91BF0, 0x010903F0, 0x00B90020},  // quadtree node operator delete
    {0x00B8C6A0, 0x010902A8, 0x00B8C3A0},  // flare operator delete
    {0x00B8D840, 0x010902A8, 0x00B8D700},  // flare operator new (JMP)
};

constexpr ResourceItemConstructor kResourceItemConstructors[] = {
    {0x00B8A780, 0x00D6328C, 2, 3},
    {0x00B8A850, 0x00D632B8, 2, 2},
    {0x00B8AA10, 0x00D632DC, 4, 2},
    {0x00B92E70, 0x00D63700, 3, 3},
};

constexpr ModelWordsConstructor kModelWordsConstructors[] = {
    {0x00B91220, 0x00D63590, 3},  // bone model: +184h..+18Ch
    {0x00B915E0, 0x00D635F0, 4},  // skin model: +184h..+190h
};

}  // namespace

const MitPoolThunk* find_mit_pool_thunk(std::uint32_t address) noexcept {
    for (const MitPoolThunk& t : kMitPoolThunks)
        if (t.address == address) return &t;
    return nullptr;
}

// ---------------------------------------------------------------------------
// Resource items, tracks and group parameters.

const ResourceItemConstructor* find_resource_item_constructor(std::uint32_t address) noexcept {
    for (const ResourceItemConstructor& c : kResourceItemConstructors)
        if (c.address == address) return &c;
    return nullptr;
}

void construct_resource_item(std::uint32_t address, std::uint32_t words[8],
    const std::function<void()>& construct_reference_base) {
    const ResourceItemConstructor* c = find_resource_item_constructor(address);
    if (!c) return;
    construct_reference_base();
    words[0] = c->vtable;
    for (int i = 0; i < c->zero_count; ++i) words[c->zero_first + i] = 0;
}

void construct_mesh_resource(std::uint32_t address, MeshResourceWords& out, RefCounted* mesh, std::uint32_t word,
    const std::function<void()>& construct_reference_base) {
    construct_reference_base();
    out.mesh_08 = mesh;
    out.vtable = 0x00D63738;
    out.word_0c = word;
    ++mesh->refcount;
    if (address == 0x00B93980) out.vtable = 0x00D6375C;
    else if (address == 0x00B93A00) out.vtable = 0x00D63780;
}

void truncate_float_to_field_20_00b8b940(std::int32_t& field_20, float value) {
    field_20 = static_cast<std::int32_t>(static_cast<std::int64_t>(value));
}

void copy_track_range_00b8e730(const float track_08[4], float out[4]) {
    for (int i = 0; i < 4; ++i) out[i] = track_08[i];
}

void copy_track_key_block_00b8e750(const float track_18[6], float out[6]) {
    for (int i = 0; i < 6; ++i) out[i] = track_18[i];
}

void group_params_apply_00b8eb20(float params_08, float& node_184, bool node_is_group) {
    if (node_is_group) node_184 = params_08;
}

// ---------------------------------------------------------------------------
// Groups.

float lod_fade_from_projected_sphere(float projected_z, float projected_w, float camera_4c) {
    const float d = projected_z > 0.01f ? projected_z : 0.01f;  // COMISS + JBE: NaN takes 0.01
    const float q = std::fabs(f(static_cast<double>(projected_w) / d));
    float k = f(1.0 - static_cast<double>(camera_4c) * q);
    if (0.0f > k) k = 0.0f;
    else if (k > 1.0f) k = 1.0f;
    return k;
}

void group_render_00b8ef10(void* group, float visibility_ac, float threshold_184, std::uint8_t byte_138,
    void* context, void* cull_camera, void* view_camera, float alpha, std::uint32_t flags,
    const std::vector<GroupChildView>& children, GroupRenderServices& s) {
    if (!(visibility_ac > 0.0f)) return;
    std::uint32_t result = flags;
    if ((flags & 0x555u) != 0x555u && (byte_138 & 3)) result = s.classify_sphere_00b71530(cull_camera, group);
    if (result == 0xAAAu) return;
    float z = 0, w = 0;
    s.project_sphere(view_camera, group, z, w);
    const float k = lod_fade_from_projected_sphere(z, w, s.view_camera_lod_4c(view_camera));
    if (!(threshold_184 > k)) return;
    const std::uint32_t mask = s.cull_camera_mask_19c(cull_camera);
    for (const GroupChildView& c : children)
        if (mask & c.mask_48)
            s.render_child_slot_20(c.object, context, k, f(static_cast<double>(visibility_ac) * alpha), flags);
}

void group_detach_nodes_00b8f090(SceneNode& group, std::vector<void**>& attached_a0,
    const std::function<void(SceneNode&)>& detach_children) {
    detach_children(group);
    while (!attached_a0.empty()) {
        *attached_a0.back() = nullptr;
        attached_a0.pop_back();
    }
}

void group_move_typed_children_last_00b8f6f0(GroupSibling& group) {
    GroupSibling* moved = nullptr;
    for (GroupSibling* p = group.first; p;) {
        GroupSibling* next = p->next;
        if (p->movable) {
            if (!p->prev) {
                if (p->parent) p->parent->first = p->next;
            } else {
                p->prev->next = p->next;
            }
            if (p->next) p->next->prev = p->prev;
            p->next = moved;
            p->prev = nullptr;
            if (moved) moved->prev = p;
            moved = p;
        }
        p = next;
    }
    if (!moved) return;
    GroupSibling* last = group.first;
    if (!last) throw std::logic_error("00B8F6F0 reads [first child + 3Ch] with no child left");
    while (last->next) last = last->next;
    last->next = moved;
    moved->prev = last;
}

void* group_clone_00b8f6f0(void* source, GroupSibling& children, const std::string& name, std::uint32_t flags,
    void* parent, GroupCloneServices& s) {
    group_move_typed_children_last_00b8f6f0(children);
    void* slot = s.allocate_group_00b8f310();
    void* clone = slot ? s.construct_group_00b8f5e0(slot, name) : nullptr;
    s.copy_clone_base_state_00b6f150(source, clone, flags, parent);
    s.copy_bytes_174_175(source, clone);
    return clone;
}

// ---------------------------------------------------------------------------
// Skin and bone models.

void* skin_model_clone_00b8f960(const std::string& name, const std::function<void*()>& allocate_node,
    const std::function<void*(void*, const std::string&)>& construct_node) {
    void* slot = allocate_node();
    return slot ? construct_node(slot, name) : nullptr;
}

void skin_model_update_animator_00b8fde0(void* model, const float*& frame_190, std::int32_t channel, float time,
    std::int32_t flag, SkinModelAnimServices& s) {
    void* animator = s.animator_130(model);
    if (animator && s.animator_is_optimized(animator)) frame_190 = s.baked_frame_00b92500(animator, channel, time);
    else frame_190 = nullptr;
    s.apply_animation_00b6e150(model, channel, time, flag);
}

void animated_transform_construct_00b8fe50(AnimatedTransform& t) {
    for (std::uint32_t& w : t.words) w = 0;
    for (int i = 0; i < 16; ++i) t.matrix_20[i] = (i % 5 == 0) ? 1.0f : 0.0f;  // 004134F0 copy of identity
}

float skin_model_render_fade_00b90490(bool has_a0, float fade, float projected_z, float projected_w, float camera_4c) {
    return has_a0 ? lod_fade_from_projected_sphere(projected_z, projected_w, camera_4c) : fade;
}

const ModelWordsConstructor* find_model_words_constructor(std::uint32_t address) noexcept {
    for (const ModelWordsConstructor& c : kModelWordsConstructors)
        if (c.address == address) return &c;
    return nullptr;
}

void release_node_holder_00b917b0(void*& holder, const std::function<void(void*)>& unlink_and_release) {
    if (holder) {
        unlink_and_release(holder);
        holder = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Quadtree.

std::int32_t quadtree_clip_range_00b917d0(const QuadAabb& box, float a, float b, float c, float d) {
    // FCOMIP + JBE selections: (hi > lo) ? lo : hi and (lo > hi) ? lo : hi keep the second
    // operand on an unordered compare, as the native branches do.
    auto smaller = [](float lo, float hi) { return hi > lo ? lo : hi; };
    auto larger = [](float lo, float hi) { return lo > hi ? lo : hi; };
    float nx, px, nz, pz;
    if (a >= 0.0f) {  // COMISS + JB: NaN takes the negative arm
        nx = smaller(box.min[0], box.max[0]);
        px = larger(box.min[0], box.max[0]);
    } else {
        nx = box.min[0] > box.max[0] ? box.min[0] : box.max[0];
        px = box.max[0] > box.min[0] ? box.min[0] : box.max[0];
    }
    const float ny = b >= 0.0f ? box.min[1] : box.max[1];
    const float py = b >= 0.0f ? box.max[1] : box.min[1];
    if (c >= 0.0f) {
        nz = smaller(box.min[2], box.max[2]);
        pz = larger(box.min[2], box.max[2]);
    } else {
        nz = box.min[2] > box.max[2] ? box.min[2] : box.max[2];
        pz = box.max[2] > box.min[2] ? box.min[2] : box.max[2];
    }
    const float far_sum = f((static_cast<double>(b) * py + static_cast<double>(a) * px) + static_cast<double>(c) * pz);
    const float far_dist = f(static_cast<double>(far_sum) - d);
    if (0.0f > far_dist) return 0;
    const float near_sum = f((static_cast<double>(a) * nx + static_cast<double>(b) * ny) + static_cast<double>(c) * nz);
    const float near_dist = f(static_cast<double>(near_sum) - d);
    return near_dist > 0.0f ? 1 : 10;
}

void quadtree_render_00b91a50(void* node, const QuadAabb& box, void* context, float a, float b, std::uint32_t flags,
    QuadRenderServices& s) {
    if (!(s.node_mask_48(node) & s.cull_camera_mask_19c(context))) return;
    std::uint32_t out_flags = flags;
    if ((flags & 0x555u) != 0x555u) {
        std::uint32_t n = s.plane_count(context);
        if (n > 6) n = 6;
        std::int32_t sum = 0;
        if (n > 0) {
            for (std::int32_t i = static_cast<std::int32_t>(n) - 1; i >= 0; --i) {
                float p[4];
                s.plane(context, static_cast<std::uint32_t>(i), p);
                const std::int32_t r = quadtree_clip_range_00b917d0(box, p[0], p[1], p[2], p[3]);
                if (r == 0) return;
                sum += r;
            }
            if (sum == 6) out_flags = flags | 0x555u;
        }
    }
    s.render_children_00b6d990(node, context, a, b, out_flags);
}

void quadtree_include_child_bounds_00b91b20(void* node, QuadAabb& box, QuadBoundsServices& s) {
    if (s.is_generated_model(node)) {
        const float v = s.visibility_ac(node);
        if (!(v == 0.0f)) {  // UCOMISS/LAHF: NaN counts as non-zero
            QuadAabb w{};
            s.world_box(node, w);
            if (-100.0 > static_cast<double>(w.min[1])) w.min[1] = -100.0f;
            s.include_point_00427d10(box, w.min);
            s.include_point_00427d10(box, w.max);
        }
    }
    for (void* c : s.children(node))
        if (s.tagged_bit30(c)) quadtree_include_child_bounds_00b91b20(c, box, s);
}

void quadtree_build_00b91d70(void* node, QuadBuildServices& s) {
    QuadAabb& own = s.bounds(node);
    if (s.level_178(node) == 0) {
        for (int i = 0; i < 3; ++i) {
            own.min[i] = 1e10f;   // [00CE4970]
            own.max[i] = -1e10f;  // [00CE4ADC]
        }
        std::uint32_t tagged = 0;
        for (void* c : s.children(node))
            if (s.tagged_bit30(c)) {
                s.include_child_bounds_00b91b20(c, own);
                ++tagged;
            }
        s.set_leaf_174(node, false);
        if (tagged < 10) return;
    } else if (s.child_count_38(node) < 5 || s.level_178(node) > 6) {
        s.set_leaf_174(node, true);
        return;
    }
    s.set_leaf_174(node, false);
    void* quads[2];
    for (int i = 0; i < 2; ++i) quads[i] = s.create_quad_child(node, s.level_178(node) + 1);
    float sum_x = 0.0f, sum_z = 0.0f;
    std::uint32_t count = 0;
    for (void* c : s.children(node)) {
        if (!s.tagged_bit30(c)) continue;
        float p[3];
        s.world_position(c, p);
        ++count;
        sum_x = f(static_cast<double>(p[0]) + sum_x);
        sum_z = f(static_cast<double>(p[2]) + sum_z);
    }
    const float n = f(static_cast<double>(count));  // FILD + 2^32 when negative
    const float mean_x = f(static_cast<double>(sum_x) / n);
    const float mean_z = f(static_cast<double>(sum_z) / n);
    const double extent_x = static_cast<double>(own.max[0]) - own.min[0];
    const double extent_z = static_cast<double>(own.max[2]) - own.min[2];
    float nx, nz, mean;
    if (extent_x < extent_z) {
        nx = 0.0f;
        nz = 1.0f;
        mean = mean_z;
    } else {
        nx = 1.0f;
        nz = 0.0f;
        mean = mean_x;
    }
    for (void* q : quads) {
        QuadAabb& b = s.bounds(q);
        for (int i = 0; i < 3; ++i) {
            b.min[i] = 1e10f;
            b.max[i] = -1e10f;
        }
    }
    std::vector<void*> tagged;  // gathered first: re-parenting edits the sibling chain
    for (void* c : s.children(node))
        if (s.tagged_bit30(c)) tagged.push_back(c);
    for (void* c : tagged) {
        if (!s.tagged_bit30(c)) continue;
        float p[3];
        s.world_position(c, p);
        const float dot = f((static_cast<double>(p[0]) * nx + static_cast<double>(p[1]) * 0.0) + static_cast<double>(p[2]) * nz);
        const float dist = f(static_cast<double>(dot) - mean);
        void* target = dist >= 0.0f ? quads[0] : quads[1];  // FCOMIP + JAE: NaN goes to the second
        s.include_child_bounds_00b91b20(c, s.bounds(target));
        s.set_parent_00b6e680(c, target);
    }
    for (void*& q : quads) {
        if (s.child_count_38(q) == 0) {
            if (q) {
                s.unlink_and_release_00b6dfa0(q);
                q = nullptr;
            }
        } else {
            s.set_parent_00b6e680(q, node);
            quadtree_build_00b91d70(q, s);
        }
    }
}

// ---------------------------------------------------------------------------
// Animation values, model creation, readers and small containers.

void push_back_animation_value_00b92650(NativeArray<AnimValue>& a, const AnimValue& v,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve) {
    array_push_back_via_reserve(a, v, capacity_double_min_one, reserve);
}

void clear_animation_values_00b926a0(NativeArray<AnimValue>& a,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve) {
    if (a.capacity < 0) reserve(a, 0);
    a.count = 0;
}

void destroy_animation_values_00b92790(NativeArray<AnimValue>& a,
    const std::function<void(NativeArray<AnimValue>&, std::int32_t)>& reserve) {
    clear_animation_values_00b926a0(a, reserve);
    ::operator delete(a.data);
}

void* create_model_from_resource(const std::function<void*()>& allocate,
    const std::function<void*(void*)>& construct) {
    void* slot = allocate();
    return slot ? construct(slot) : nullptr;
}

void read_element_params_00b934d0(std::int32_t& element_index_08, StructuredReader& reader, bool with_extra,
    const std::function<void(std::int32_t)>& lookup_element) {
    lookup_element(element_index_08);
    const std::uint32_t n = reader.read_dword_00be99f0();
    std::uint32_t min_start = 0xFFFFFFFFu, max_end = 0;  // EBP/EBX; never stored
    for (std::uint32_t i = 0; i < n; ++i) {
        const std::uint32_t start = reader.read_dword_00be99f0();
        const std::uint32_t length = reader.read_dword_00be99f0();
        if (start < min_start) min_start = start;
        if (start + length > max_end) max_end = start + length;
        if (with_extra) {
            (void)reader.read_dword_00be99f0();
            (void)reader.read_float_00be99d0();
        }
    }
    (void)min_start;
    (void)max_end;
    ++element_index_08;
}

void mesh_resource_visit_00b94020(void* self, std::uint32_t word_0c, void* a1, void* a2, void* node, std::int32_t lod,
    MeshResourceVisitServices& s) {
    s.base_dispatch_00b86820(self, a1, a2, node, lod);
    if (node && s.is_generated_model(node)) {
        void* geometry = s.create_geometry(self, lod);
        s.set_geometry_00b75170(node, word_0c, geometry, -1.0f, -1.0f);
        s.release_ref(geometry);
        return;
    }
    void* child = s.create_child_model(self, "Object");
    s.set_parent_00b6e680(child, node);
    void* geometry = s.create_geometry(self, lod);
    s.set_geometry_00b75170(child, word_0c, geometry, -1.0f, -1.0f);
    s.release_ref(geometry);
    s.release_ref(child);
}

void* allocate_8byte_elements_00b94bd0(std::uint32_t count, const std::function<void*(std::size_t)>& operator_new) {
    if (count != 0 && 0xFFFFFFFFu / count < 8u) throw std::bad_alloc();
    return operator_new(static_cast<std::size_t>(count) * 8);
}

void fill_pairs_00b96b60(PairRecord* first, std::int32_t n, const PairRecord& value) {
    for (; n != 0; --n, ++first)
        if (first) *first = value;
}

void free_vector_at_4(void*& data_4, void*& end_8, void*& cap_c, const std::function<void(void*)>& free_block) {
    if (data_4) free_block(data_4);
    data_4 = nullptr;
    end_8 = nullptr;
    cap_c = nullptr;
}

void free_pod_data(void* data, const std::function<void(void*)>& free_block) {
    if (data) free_block(data);
}

// ---------------------------------------------------------------------------
// Ocean helpers.

namespace {

// CVTTSS2SI: out-of-range and NaN give 80000000h.
std::int32_t cvttss2si(float v) noexcept {
    if (!(v > -2147483904.0f && v < 2147483648.0f)) return INT32_MIN;
    return static_cast<std::int32_t>(v);
}

// FISTP under the default round-to-nearest-even control word.
std::int32_t fistp_nearest(float v) noexcept {
    if (!(v > -2147483904.0f && v < 2147483648.0f)) return INT32_MIN;
    return static_cast<std::int32_t>(std::nearbyint(static_cast<double>(v)));
}

float pair_part(const std::vector<float>& pairs, std::uint32_t cell, int part) {
    return pairs.at(static_cast<std::size_t>(cell) * 2 + static_cast<std::size_t>(part));
}

}  // namespace

void ocean_fft32(float* cells, int stride_complex) {
    using cc13x87::x87_add24; using cc13x87::x87_sub24; using cc13x87::x87_mul24;
    const auto add24 = [](double a, double b) { return x87_add24(a, b); };
    const auto sub24 = [](double a, double b) { return x87_sub24(a, b); };
    const auto mul24 = [](double a, double b) { return x87_mul24(a, b); };
    auto re = [&](int i) -> float& { return cells[2 * i * stride_complex]; };
    auto im = [&](int i) -> float& { return cells[2 * i * stride_complex + 1]; };
    int j = 0;
    for (int i = 0; i < 31; ++i) {
        if (i < j) {
            std::swap(re(i), re(j));
            std::swap(im(i), im(j));
        }
        int k = 16;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }
    float c1 = -1.0f, c2 = 0.0f;  // [00D7A260]
    int l1 = 1;
    for (int stage = 0; stage < 5; ++stage) {
        const int l2 = l1 * 2;
        float u1 = 1.0f, u2 = 0.0f;  // [00D638E4]
        for (int jj = 0; jj < l1; ++jj) {
            for (int i = jj; i < 32; i += l2) {
                const int i1 = i + l1;
                const float x1 = re(i1), y1 = im(i1);
                // cc13-fixture4: main-thread precision (24-bit), each product and sum
                // rounded on the stack before the FSTP float (00B94FAC..00B94FC8).
                const float t1 = f(sub24(mul24(x1, u1), mul24(y1, u2)));
                const float t2 = f(add24(mul24(y1, u1), mul24(x1, u2)));
                re(i1) = f(sub24(re(i), t1));
                im(i1) = f(sub24(im(i), t2));
                re(i) = f(add24(t1, re(i)));
                im(i) = f(add24(t2, im(i)));
            }
            const float z = f(sub24(mul24(u1, c1), mul24(u2, c2)));
            u2 = f(add24(mul24(u2, c1), mul24(u1, c2)));
            u1 = z;
        }
        const float c2n = f(std::sqrt(static_cast<double>(f((1.0 - c1) * 0.5))));
        c1 = f(std::sqrt(static_cast<double>(f((static_cast<double>(c1) + 1.0) * 0.5))));
        c2 = c2n;
        l1 = l2;
    }
}

float ocean_sample_displacement(const std::vector<float>& pairs_3c, float u, float v, int part) {
    const float fx = f(static_cast<double>(u) * 64.0);
    const float fy = f(64.0 * static_cast<double>(v));
    const std::int32_t ix = cvttss2si(fx), iy = cvttss2si(fy);
    const std::uint32_t cx = static_cast<std::uint32_t>(ix) & 63u, nx = static_cast<std::uint32_t>(ix + 1) & 63u;
    const std::uint32_t cy = static_cast<std::uint32_t>(iy) & 63u, ny = static_cast<std::uint32_t>(iy + 1) & 63u;
    const float a = pair_part(pairs_3c, cy * 64 + cx, part), b = pair_part(pairs_3c, cy * 64 + nx, part);
    const float c = pair_part(pairs_3c, ny * 64 + cx, part), d = pair_part(pairs_3c, ny * 64 + nx, part);
    const double tx = static_cast<double>(fx) - ix, ty = static_cast<double>(fy) - iy;  // FISUB, unrounded
    const float top = f(static_cast<double>(a) + (static_cast<double>(b) - a) * tx);
    const float bottom = f(static_cast<double>(c) + (static_cast<double>(d) - c) * tx);
    return f(static_cast<double>(top) + (static_cast<double>(bottom) - top) * ty);
}

float ocean_sample_rounded(const std::vector<float>& pairs, float u, float v, int part) {
    const float fu = f(static_cast<double>(u) + 0.0);
    const float fv = f(0.0 + static_cast<double>(v));
    const std::int32_t iu = fistp_nearest(fu), iv = fistp_nearest(fv);
    float tu = f(static_cast<double>(fu) - iu);
    if (!(tu > 0.0f)) tu = f(-0.0 - static_cast<double>(tu));  // [00D7A208]
    float tv = f(static_cast<double>(fv) - iv);
    if (!(tv > 0.0f)) tv = f(-0.0 - static_cast<double>(tv));
    const std::uint32_t cu = static_cast<std::uint32_t>(iu) & 63u;
    const std::uint32_t nu = static_cast<std::uint32_t>(iu + (iu > 0 ? 1 : -1)) & 63u;
    const std::uint32_t cv = static_cast<std::uint32_t>(iv) & 63u;
    const std::uint32_t nv = static_cast<std::uint32_t>(iv + (iv > 0 ? 1 : -1)) & 63u;
    const float a = pair_part(pairs, cv * 64 + cu, part), b = pair_part(pairs, cv * 64 + nu, part);
    const float c = pair_part(pairs, nv * 64 + cu, part), d = pair_part(pairs, nv * 64 + nu, part);
    const float top = f(static_cast<double>(a) + (static_cast<double>(b) - a) * tu);
    const float bottom = f(static_cast<double>(c) + (static_cast<double>(d) - c) * tu);
    return f(static_cast<double>(top) + (static_cast<double>(bottom) - top) * tv);
}

void ocean_step_slot_00b96c80(void* spectrum, float t, OceanStepServices& s) {
    s.slot_14(spectrum);
    s.advance_00b95d30(spectrum, t);
    s.member_08_slot_0(spectrum);
    s.slot_10(spectrum);
}

std::vector<std::uint32_t> ocean_destruct_order_00b96da0() { return {0x60, 0x50, 0x40, 0x30, 0x20, 0x10}; }

std::vector<PairRecord> construct_pair_vector_00b96f80(std::uint32_t n, const PairRecord& value) {
    if (n > 0x1FFFFFFFu) throw std::length_error("vector<T> too long");  // 00B96EA0
    return std::vector<PairRecord>(n, value);
}

std::vector<float> construct_float_vector_00b97030(std::uint32_t n, float value) {
    if (n > 0x3FFFFFFFu) throw std::length_error("vector<T> too long");  // 0041FCF0
    return std::vector<float>(n, value);
}

std::vector<PairRecord> construct_zero_pair_vector_00b970a0(std::uint32_t n) {
    return construct_pair_vector_00b96f80(n, PairRecord{0, 0});
}

// ---------------------------------------------------------------------------
// Grid index builder containers.

void strips_reserve_00b974b0(OwnedArray<GridStrip14>& a, std::int32_t n) { owned_reserve_min_one(a, n); }
void strips_resize_00b97560(OwnedArray<GridStrip14>& a, std::int32_t n) { owned_resize(a, n); }

void strips_push_back_00b97620(OwnedArray<GridStrip14>& a, const GridStrip14& v) {
    const std::int32_t count = static_cast<std::int32_t>(a.items.size());
    if (count == a.capacity) strips_reserve_00b974b0(a, capacity_double_min_one(a.capacity));
    a.items.push_back(v);
}

void strips_assign_00b976a0(OwnedArray<GridStrip14>& dst, const OwnedArray<GridStrip14>& src) {
    strips_resize_00b97560(dst, 0);
    strips_reserve_00b974b0(dst, static_cast<std::int32_t>(src.items.size()));
    for (const GridStrip14& s : src.items) strips_push_back_00b97620(dst, s);
}

void strips_tidy_00b97840(OwnedArray<GridStrip14>& a) {
    strips_resize_00b97560(a, 0);
    a.items.shrink_to_fit();  // _free of the data pointer; the capacity word is left as it was
}

void patch_copy(GridPatch20& dst, const GridPatch20& src) {
    for (int i = 0; i < 5; ++i) dst.words[i] = src.words[i];
    dst.strips = OwnedArray<GridStrip14>{};
    strips_assign_00b976a0(dst.strips, src.strips);
}

void patches_reserve_00b985d0(OwnedArray<GridPatch20>& a, std::int32_t n) {
    if (n < 1) n = 1;
    if (a.capacity >= n) return;
    std::vector<GridPatch20> fresh(a.items.size());
    for (std::size_t i = 0; i < a.items.size(); ++i) patch_copy(fresh[i], a.items[i]);
    for (GridPatch20& p : a.items) strips_tidy_00b97840(p.strips);
    a.items = std::move(fresh);
    a.items.reserve(static_cast<std::size_t>(n));
    a.capacity = n;
}

void patches_resize_00b986f0(OwnedArray<GridPatch20>& a, std::int32_t n) {
    if (a.capacity < n) patches_reserve_00b985d0(a, n);
    while (static_cast<std::int32_t>(a.items.size()) < n) a.items.emplace_back();
    while (static_cast<std::int32_t>(a.items.size()) > n) {
        strips_tidy_00b97840(a.items.back().strips);
        a.items.pop_back();
    }
}

void patches_push_back_00b98770(OwnedArray<GridPatch20>& a, const GridPatch20& v) {
    if (static_cast<std::int32_t>(a.items.size()) == a.capacity)
        patches_reserve_00b985d0(a, capacity_double_min_one(a.capacity));
    a.items.emplace_back();
    patch_copy(a.items.back(), v);
}

void patches_assign_00b98830(OwnedArray<GridPatch20>& dst, const OwnedArray<GridPatch20>& src) {
    patches_resize_00b986f0(dst, 0);
    patches_reserve_00b985d0(dst, static_cast<std::int32_t>(src.items.size()));
    for (const GridPatch20& p : src.items) patches_push_back_00b98770(dst, p);
}

void patches_tidy_00b98880(OwnedArray<GridPatch20>& a) {
    patches_resize_00b986f0(a, 0);
    a.items.shrink_to_fit();
}

void rows_reserve_00b988f0(OwnedArray<GridRow0C>& a, std::int32_t n) {
    if (n < 1) n = 1;
    if (a.capacity >= n) return;
    std::vector<GridRow0C> fresh(a.items.size());
    for (std::size_t i = 0; i < a.items.size(); ++i) patches_assign_00b98830(fresh[i], a.items[i]);
    for (GridRow0C& r : a.items) patches_tidy_00b98880(r);
    a.items = std::move(fresh);
    a.capacity = n;
}

void rows_resize_00b989f0(OwnedArray<GridRow0C>& a, std::int32_t n) {
    if (a.capacity < n) rows_reserve_00b988f0(a, n);
    while (static_cast<std::int32_t>(a.items.size()) < n) a.items.emplace_back();
    while (static_cast<std::int32_t>(a.items.size()) > n) {
        patches_tidy_00b98880(a.items.back());
        a.items.pop_back();
    }
}

void rows_tidy_00b98a90(OwnedArray<GridRow0C>& a) {
    rows_resize_00b989f0(a, 0);
    a.items.shrink_to_fit();
}

std::vector<std::uint32_t> grid_patch_masks_00b99790(std::uint32_t size, std::uint32_t max_size) {
    (void)max_size;
    if (size >= 0x20) return {0};
    return {0, 1, 2, 4, 8, 3, 6, 0xC, 9, 7, 0xE, 0xD, 0xB, 0xF};
}

void grid_builder_build_all_00b99790(GridBuilder& b, GridBuilderServices& s) {
    for (std::uint32_t size = 1; b.max_size_04 != 0 && size <= b.max_size_04; size *= 2)
        for (std::uint32_t mask : grid_patch_masks_00b99790(size, b.max_size_04)) s.build_patch_00b98be0(size, mask);
    const auto n = static_cast<std::int32_t>(b.words_08.size());
    b.index_buffer_20 = s.create_index_buffer(n);
    std::uint16_t* out = s.lock(b.index_buffer_20, n);
    for (std::int32_t i = 0; i < n; ++i) out[i] = b.words_08[static_cast<std::size_t>(i)];
    s.unlock(b.index_buffer_20);
    b.words_08.clear();  // 00529980(0)
}

namespace {

struct GridWriter {
    GridBuilder& b;
    std::uint32_t n;
    void push(std::uint32_t y, std::uint32_t x) {
        b.words_08.push_back(static_cast<std::uint16_t>((n + 1) * y + x));
    }
    void push_centre() {
        const std::uint32_t h = n >> 1;
        push(h, h);
    }
};

}  // namespace

void grid_write_full_patch_00b978b0(GridBuilder& b, std::uint32_t cells) {
    GridWriter w{b, b.max_size_04};
    const std::uint32_t st = w.n / cells;
    for (std::uint32_t r = 0, y0 = 0; r < cells; ++r, y0 += st) {
        const std::uint32_t y1 = y0 + st;
        for (std::uint32_t c = 0, x0 = 0; c < cells; ++c, x0 += st) {
            const std::uint32_t x1 = x0 + st;
            if ((c & 1) == (r & 1)) {
                w.push(y1, x0); w.push(y1, x1); w.push(y0, x0); w.push(y0, x1); w.push(y0, x0); w.push(y1, x1);
            } else {
                w.push(y1, x0); w.push(y0, x1); w.push(y0, x0); w.push(y1, x0); w.push(y1, x1); w.push(y0, x1);
            }
        }
    }
}

void grid_write_interior_00b97c80(GridBuilder& b, std::uint32_t cells) {
    GridWriter w{b, b.max_size_04};
    const std::uint32_t st = w.n / cells;
    const std::uint32_t last = cells - 1;
    for (std::uint32_t r = 1, y0 = st; r < last; ++r, y0 += st) {
        const std::uint32_t y1 = y0 + st;
        for (std::uint32_t c = 1, x0 = st; c < last; ++c, x0 += st) {
            const std::uint32_t x1 = x0 + st;
            if ((c & 1) == (r & 1)) {
                w.push(y1, x1); w.push(y0, x0); w.push(y1, x0); w.push(y0, x1); w.push(y0, x0); w.push(y1, x1);
            } else {
                w.push(y0, x1); w.push(y0, x0); w.push(y1, x0); w.push(y1, x1); w.push(y0, x1); w.push(y1, x0);
            }
        }
    }
}

void grid_write_edge_00b980a0(GridBuilder& b, std::uint32_t edge, std::uint32_t cells, std::uint32_t edge_cells) {
    GridWriter w{b, b.max_size_04};
    const std::uint32_t n = w.n;
    std::uint32_t st = n / cells;
    const std::uint32_t cs = n / edge_cells;
    // A and B as (x, y); `sel` picks the running coordinate; `end` is where A stops.
    std::uint32_t a[2], bb[2], da[2], db[2], end, sel;
    switch (edge) {
    case 1: end = n; sel = 0; a[0] = 0; a[1] = 0; bb[0] = st; bb[1] = st; da[0] = cs; da[1] = 0; db[0] = st; db[1] = 0; break;
    case 2: end = n; sel = 1; a[0] = n; a[1] = 0; bb[0] = n - st; bb[1] = st; da[0] = 0; da[1] = cs; db[0] = 0; db[1] = st; break;
    case 4:
        end = 0; sel = 0; a[0] = n; a[1] = n; bb[0] = n - st; bb[1] = n - st; st = 0u - st;
        da[0] = 0u - cs; da[1] = 0; db[0] = st; db[1] = 0; break;
    case 8:
        end = 0; sel = 1; a[0] = 0; a[1] = n; bb[0] = st; bb[1] = n - st; st = 0u - st;
        da[0] = 0; da[1] = 0u - cs; db[0] = 0; db[1] = st; break;
    default: return;  // the native locals are left uninitialised
    }
    if (cells == 1) {
        for (std::uint32_t k = 0; k < edge_cells; ++k) {
            const std::uint32_t nx = a[0] + da[0], ny = a[1] + da[1];
            w.push(ny, nx);
            w.push(a[1], a[0]);
            w.push_centre();
            a[0] = nx;
            a[1] = ny;
        }
        return;
    }
    while (a[sel] != end) {
        const std::uint32_t nx = a[0] + da[0], ny = a[1] + da[1];
        w.push(ny, nx);
        w.push(a[1], a[0]);
        w.push(bb[1], bb[0]);
        a[0] = nx;
        a[1] = ny;
        if (bb[sel] == a[sel] && bb[sel] != end - st) {
            const std::uint32_t bx = db[0] + bb[0], by = bb[1] + db[1];
            w.push(by, bx);
            w.push(a[1], a[0]);
            w.push(bb[1], bb[0]);
            bb[0] = bx;
            bb[1] = by;
        }
    }
}

void grid_build_patch_00b98be0(GridBuilder& b, std::uint32_t cells, std::uint32_t mask) {
    GridWriter w{b, b.max_size_04};
    const std::uint32_t n = w.n;
    const std::uint32_t e1 = (mask & 1) ? cells * 2 : cells;
    const std::uint32_t e2 = (mask & 2) ? cells * 2 : cells;
    const std::uint32_t e4 = (mask & 4) ? cells * 2 : cells;
    const std::uint32_t e8 = (mask & 8) ? cells * 2 : cells;
    const auto first = static_cast<std::uint32_t>(b.words_08.size());
    const std::uint32_t side = n + 1;
    if (mask == 0) {
        grid_write_full_patch_00b978b0(b, cells);
    } else {
        grid_write_interior_00b97c80(b, cells);
        grid_write_edge_00b980a0(b, 1, cells, e1);
        const std::uint32_t st = n / cells;
        // Bottom edge (y = N), x running from N down to 0.
        {
            const std::uint32_t cs = n / e4;
            std::uint32_t x = n;
            if (cells == 1) {
                for (std::uint32_t k = 0; k < e4; ++k) {
                    const std::uint32_t xn = x - cs;
                    w.push(n, xn); w.push(n, x); w.push_centre();
                    x = xn;
                }
            } else {
                const std::uint32_t inner = n - st;
                std::uint32_t cursor = n - st;
                while (x != 0) {
                    const std::uint32_t xn = x - cs;
                    w.push(n, xn); w.push(n, x); w.push(inner, cursor);
                    if (cursor == xn && cursor != st) {
                        const std::uint32_t old = cursor;
                        cursor -= st;
                        w.push(inner, cursor); w.push(n, xn); w.push(inner, old);
                    }
                    x = xn;
                }
            }
        }
        // Left edge (x = 0), y running from N down to 0.
        {
            const std::uint32_t cs = n / e8;
            std::uint32_t y = n;
            if (cells == 1) {
                for (std::uint32_t k = 0; k < e8; ++k) {
                    const std::uint32_t yn = y - cs;
                    w.push(yn, 0); w.push(y, 0); w.push_centre();
                    y = yn;
                }
            } else {
                std::uint32_t cursor = n - st;
                while (y != 0) {
                    const std::uint32_t yn = y - cs;
                    w.push(yn, 0); w.push(y, 0); w.push(cursor, st);
                    if (cursor == yn && cursor != st) {
                        w.push(cursor - st, st); w.push(yn, 0); w.push(cursor, st);
                        cursor -= st;
                    }
                    y = yn;
                }
            }
        }
        // Right edge (x = N), y running from 0 up to N.
        {
            const std::uint32_t cs = n / e2;
            const std::uint32_t inner = n - st;
            std::uint32_t y = 0;
            if (cells == 1) {
                for (std::uint32_t k = 0; k < e2; ++k) {
                    const std::uint32_t yn = cs + y;
                    w.push(yn, n); w.push(y, n); w.push_centre();
                    y = yn;
                }
            } else {
                std::uint32_t cursor = st;
                while (y != n) {
                    const std::uint32_t yn = cs + y;
                    w.push(yn, n); w.push(y, n); w.push(cursor, inner);
                    if (cursor == yn && cursor != n - st) {
                        w.push(st + cursor, inner); w.push(yn, n); w.push(cursor, inner);
                        cursor += st;
                    }
                    y = yn;
                }
            }
        }
    }
    GridPatch20 patch;
    patch.words[0] = cells;
    patch.words[1] = e1;
    patch.words[2] = e4;
    patch.words[3] = e8;
    patch.words[4] = e2;
    strips_reserve_00b974b0(patch.strips, 1);
    GridStrip14 strip;
    strip.words[0] = 4;
    strip.words[1] = 0;
    strip.words[2] = side * side;
    strip.words[3] = first;
    strip.words[4] = (static_cast<std::uint32_t>(b.words_08.size()) - first) / 3;
    patch.strips.items.push_back(strip);
    if (static_cast<std::int32_t>(b.rows_14.items.size()) <= static_cast<std::int32_t>(cells))
        rows_resize_00b989f0(b.rows_14, static_cast<std::int32_t>(cells) + 1);
    patches_push_back_00b98770(b.rows_14.items[cells], patch);
}

void grid_builder_construct_00b998c0(GridBuilder& b, GridBuilderServices& s) {
    b.vtable = 0x00D6391C;
    b.words_08.clear();
    b.rows_14 = OwnedArray<GridRow0C>{};
    b.index_buffer_20 = nullptr;
    b.ref_24 = nullptr;
    b.max_size_04 = 0x20;
    grid_builder_build_all_00b99790(b, s);
}

void grid_builder_destruct_00b98b10(GridBuilder& b, void*& instance_01090490, GridBuilderServices& s) {
    b.vtable = 0x00D6391C;
    for (void** slot : {&b.index_buffer_20, &b.ref_24})
        if (*slot) {
            s.release_ref(*slot);
            *slot = nullptr;
        }
    rows_tidy_00b98a90(b.rows_14);
    b.words_08.clear();
    b.words_08.shrink_to_fit();
    instance_01090490 = nullptr;
    b.vtable = 0x00CE3818;
}

void grid_instance_clear_00b97480(void*& instance_01090490, std::uint32_t& vtable) {
    instance_01090490 = nullptr;
    vtable = 0x00CE3818;
}

// ---------------------------------------------------------------------------
// Remaining small bodies.

void camera_submit_clear_00b700a0(const CameraClearView& c,
    const std::function<void(std::uint32_t, std::uint32_t, std::uint32_t, const std::uint32_t*, float, std::uint32_t)>& renderer_slot_08) {
    renderer_slot_08(0, 0, c.word_188, &c.colour_190, c.depth_18c, c.word_194);
}

void node_apply_animation_thunk_00b8e690(void* node, std::int32_t channel, float time, std::int32_t flag,
    const std::function<void(void*, std::int32_t, float, std::int32_t)>& apply_00b6e150) {
    apply_00b6e150(node, channel, time, flag);
}

void* quadtree_clone_00b91c40(const void* source, const std::string& name, std::uint32_t flags, void* parent,
    const std::function<void*()>& pool_allocate, const std::function<void*(void*, const std::string&)>& construct_node,
    const std::function<void(void*)>& init_quadtree_words, NodeCloneServices& s) {
    void* clone = pool_allocate();  // pool 010903F0
    if (clone) {
        clone = construct_node(clone, name);
        init_quadtree_words(clone);  // vtable 00D63650, +178h = 0
    }
    s.copy_clone_base_state_00b6f150(source, clone, flags, parent);
    return clone;
}

void* quadtree_scalar_delete_00b91c00(void* self, std::uint32_t flags, const std::function<void(void*)>& set_vtable,
    const std::function<void(void*)>& destroy_node, const std::function<void(void*)>& pool_free) {
    set_vtable(self);    // 00D63650
    destroy_node(self);  // 00B6F440
    if (flags & 1) pool_free(self);
    return self;
}

void model_destruct(void* self, bool set_skin_vtable, const std::function<void(void*)>& set_vtable,
    const std::function<void(void*)>& resize_array_to_zero_and_free, const std::function<void(void*)>& destroy_model) {
    if (set_skin_vtable) set_vtable(self);  // 00B91650 stores 00D635F0; 00B91280 stores none
    resize_array_to_zero_and_free(self);    // +184h array
    destroy_model(self);                    // 00B750C0
}

void* model_scalar_delete(void* self, std::uint32_t flags, const std::function<void(void*)>& destruct,
    const std::function<void(void*)>& pool_free) {
    destruct(self);
    if (flags & 1) pool_free(self);
    return self;
}

void* bone_model_clone_00b912e0(const void* source, const std::string& name, const std::vector<std::string>& bone_names,
    float word_178, float word_17c, std::uint32_t flags, void* parent, BoneCloneServices& s) {
    void* slot = s.allocate_00b90ac0();
    void* clone = slot ? s.construct_bone_model(slot, name) : nullptr;
    s.copy_clone_base_state_00b6f150(source, clone, flags, parent);
    s.resize_bones_00b90630(clone, static_cast<std::int32_t>(bone_names.size()));
    for (std::size_t i = 0; i < bone_names.size(); ++i)
        s.set_bone(clone, static_cast<std::int32_t>(i), s.find_child_00b6e750(parent, bone_names[i]));
    if (void* geometry = s.source_mesh_geometry(source, flags)) {
        s.set_geometry_00b75170(clone, geometry, word_178, word_17c);
        s.release_ref(geometry);
    }
    s.assign_retained_174(clone, source);
    s.copy_vector_08(clone, source);
    return clone;
}

void catch_all_free_and_rethrow(void* buffer, const std::function<void(void*)>& free_block) {
    free_block(buffer);
    throw;  // __CxxThrowException(0, 0)
}

// Explicit instantiations: compile every header template at least once.
template NativeTreeNode<std::uint32_t>* tree_minimum<std::uint32_t>(NativeTreeNode<std::uint32_t>*);
template NativeTreeNode<std::uint32_t>* tree_maximum<std::uint32_t>(NativeTreeNode<std::uint32_t>*);
template NativeTree<std::uint32_t>& tree_initialize_empty<std::uint32_t>(NativeTree<std::uint32_t>&,
    const std::function<NativeTreeNode<std::uint32_t>*()>&);
template void tree_clear<std::uint32_t>(NativeTree<std::uint32_t>&,
    const std::function<void(NativeTreeNode<std::uint32_t>*)>&);
template void tree_destroy<std::uint32_t>(NativeTree<std::uint32_t>&,
    const std::function<void(NativeTree<std::uint32_t>&, NativeTreeNode<std::uint32_t>*, NativeTreeNode<std::uint32_t>*)>&,
    const std::function<void(void*)>&);
template NativeTreeNode<std::uint32_t>* tree_find_uint_key<std::uint32_t>(const NativeTree<std::uint32_t>&, std::uint32_t,
    const std::function<std::uint32_t(const std::uint32_t&)>&);
template void owned_reserve_min_one<std::vector<std::string>>(OwnedArray<std::vector<std::string>>&, std::int32_t);
template void owned_resize<std::vector<std::string>>(OwnedArray<std::vector<std::string>>&, std::int32_t);
template void array_destroy<void*>(PointerArray&, const std::function<void(PointerArray&, std::int32_t)>&);
template std::uint32_t& step_iterator<std::uint32_t>(std::uint32_t&, const std::function<void(std::uint32_t&)>&);
template SceneNode* checked_cast<SceneNode>(SceneNode*, std::uint32_t, const std::function<bool(SceneNode&, std::uint32_t)>&);

}  // namespace bsp::cc13_scene2

namespace bsp::cc13_scene2 {
namespace {
template<class T> T& nc_field(void* p, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(p) + offset);
}
float nc_x87_add(float left, float right) {
    float result;
    __asm {
        fld right
        fadd left
        fstp result
    }
    return result;
}
float nc_x87_subtract(float left, float right) {
    float result;
    __asm {
        fld left
        fsub right
        fstp result
    }
    return result;
}
float nc_x87_scale(float left, float scalar) {
    float result;
    __asm {
        fld left
        fmul scalar
        fstp result
    }
    return result;
}
float nc_x87_copy(float value) {
    float result;
    __asm {
        fld value
        fstp result
    }
    return result;
}
} // namespace

float* matrix_scale_00b6d440(const float* left, float* output, float scale) {
    for (int i = 0; i != 16; ++i) output[i] = nc_x87_scale(left[i], scale);
    return output;
}
float* matrix_add_00b6d630(const float* left, float* output, const float* right) {
    for (int i = 0; i != 16; ++i) output[i] = nc_x87_add(left[i], right[i]);
    return output;
}
float* matrix_subtract_00b6d6d0(const float* left, float* output, const float* right) {
    for (int i = 0; i != 16; ++i) output[i] = nc_x87_subtract(left[i], right[i]);
    return output;
}
float* vector4_add_00b50020(const float* left, float* output, const float* right) {
    for (int i = 0; i != 4; ++i) output[i] = nc_x87_add(left[i], right[i]);
    return output;
}
float* matrix_blend_00b6de40(float* output, const float* base,
    const float* target, const float* weight) {
    // The listing spills all three intermediate matrices, rather than keeping
    // base + (target-base)*weight in extended precision.
    const float amount = *weight;
    float difference[16], scaled[16], result[16];
    matrix_subtract_00b6d6d0(target, difference, base);
    matrix_scale_00b6d440(difference, scaled, amount);
    matrix_add_00b6d630(base, result, scaled);
    for (int i = 0; i != 16; ++i) output[i] = result[i];
    return output;
}
void invalidate_world_subtree_00b6da00(void* node) {
    const auto flags = nc_field<std::uint32_t>(node, 0x5c);
    if (!(flags & 2u)) return;
    nc_field<std::uint32_t>(node, 0x138) &= 0xffffffcfu;
    nc_field<std::uint32_t>(node, 0x5c) = flags & 0xfffffff5u;
    for (void* child = nc_field<void*>(node, 0x34); child;
         child = nc_field<void*>(child, 0x3c))
        invalidate_world_subtree_00b6da00(child);
}
void blend_animation_subtree_00b6e240(void* node, std::uint32_t animation,
    float time, float weight, std::uint32_t recurse, NodeBlendCalls& calls,
    BlendScratch scratch) {
    float* sampled = scratch(node);
    void* animator = nc_field<void*>(node, 0x130);
    if (animator && calls.sample_slot18(animator, sampled, animation, time)) {
        // 00B6E281..2F5 writes identity, then copies it TO sampled (+4Ch).
        for (int i = 0; i != 16; ++i) sampled[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
    float result[16];
    matrix_blend_00b6de40(result, &nc_field<float>(node, 0xb0), sampled, &weight);
    calls.set_matrix_slot38(node, result);
    if (nc_field<std::uint8_t>(node, 0x134) && (recurse & 0xffu))
        for (void* child = nc_field<void*>(node, 0x34); child;
             child = nc_field<void*>(child, 0x3c))
            blend_animation_subtree_00b6e240(child, animation, time, weight, recurse, calls, scratch);
}
void camera_set_position_direction_00b70320(void* camera, const float* position,
    const float* direction, CameraAimCalls& calls) {
    nc_field<std::uint32_t>(camera, 0x2f0) &= 0xfffffe4bu;
    calls.set_position_slot2c(camera, position);
    for (int i = 0; i != 3; ++i)
        nc_field<float>(camera, 0x1ac + i * 4) = nc_x87_copy(direction[i]);
    float target[3];
    for (int i = 0; i != 3; ++i)
        target[i] = nc_x87_add(position[i], nc_field<float>(camera, 0x1ac + i * 4));
    for (int i = 0; i != 3; ++i)
        nc_field<float>(camera, 0x1a0 + i * 4) = nc_x87_copy(target[i]);
    const float up[3]{0.0f, 1.0f, 0.0f};
    float look_storage[16], look[16], inverse_storage[16], inverse[16];
    const float* built = calls.build_look_at_00b63f10(look_storage,
        &nc_field<float>(camera, 0x1a0), position, up);
    for (int i = 0; i != 16; ++i) look[i] = nc_x87_copy(built[i]);
    const float* inverted = calls.invert_00b63b30(inverse_storage, look);
    for (int i = 0; i != 16; ++i) inverse[i] = nc_x87_copy(inverted[i]);
    calls.set_matrix_slot34(camera, inverse);
}
void collect_typed_subtree_00b85430(void* array, void* node, std::uint32_t type,
    CollectNodeCalls& calls) {
    if (calls.is_type_slot0c(node, type)) {
        if (nc_field<std::int32_t>(array, 4) == nc_field<std::int32_t>(array, 8)) {
            const auto doubled = nc_field<std::uint32_t>(array, 8) * 2u;
            calls.reserve_0059e5e0(array, static_cast<std::int32_t>(doubled) > 1
                ? static_cast<std::int32_t>(doubled) : 1);
        }
        const auto slot = reinterpret_cast<std::uintptr_t>(nc_field<void*>(array, 0))
            + nc_field<std::uint32_t>(array, 4) * 4u;
        if (slot) *reinterpret_cast<void**>(slot) = node;
        ++nc_field<std::uint32_t>(array, 4);
    }
    for (void* child = nc_field<void*>(node, 0x34); child;
         child = nc_field<void*>(child, 0x3c))
        collect_typed_subtree_00b85430(array, child, type, calls);
}
std::uint32_t mesh_resource_owner_00b930a0() noexcept { return 0x00e130a8u; }
std::uint32_t skin_mesh_resource_owner_00b931a0() noexcept { return 0x00e130acu; }
std::uint32_t bone_mesh_resource_owner_00b932a0() noexcept { return 0x00e130b0u; }
void* bone_mesh_resource_create_00b93250(BoneResourceFactory create_00b931f0) {
    return create_00b931f0(0x00e130b0u);
}
void* object_field8c_00ba70b0(void* object) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(object) + 0x8cu);
}
} // namespace bsp::cc13_scene2
