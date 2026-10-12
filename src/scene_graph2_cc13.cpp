#include "bsp/scene_graph2_cc13.hpp"

#include "bsp/post_effects_cc13.hpp"

#include <cctype>
#include <cmath>

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

void matrix_from_transposed_3x4_00b75de0(float out[16], const float src[12]) {
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 4; ++c) out[c * 4 + r] = src[r * 4 + c];
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

}  // namespace bsp::cc13_scene2
