// Leftovers after the cc13 band sweeps (cc13-leftovers1): the MGeomMesh triangle AABB tree build
// and the unreferenced node counter 00721C60.
// Reconstructed and build-tested only: not ABI-compatible, not fixture- or game-validated.
// See docs/CC13_LEFTOVERS_1.md.

#include "bsp/leftovers_cc13.hpp"

#include <climits>
#include <cmath>
#include <cstring>
#include <limits>

namespace bsp::leftovers_cc13 {
namespace {

// 00D7A248 / 00D7A244: the empty-box seeds.
constexpr float kBoxMinSeed = std::numeric_limits<float>::max();
constexpr float kBoxMaxSeed = -std::numeric_limits<float>::max();

// 00724462 CMP EDI,14h / JLE: a range of at most 20 triangles is a leaf.
constexpr int kLeafTriangles = 20;

std::uint32_t float_bits(float v) noexcept {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof bits);
    return bits;
}

// CVTTSS2SI: truncation, and the integer indefinite 80000000h for NaN or out of range.
std::int32_t cvttss2si(float v) noexcept {
    if (!(v > -2147483904.0f && v < 2147483648.0f)) {
        return INT_MIN;
    }
    return static_cast<std::int32_t>(v);
}

// Per-vertex update order of 007240E0 and 00725380: min x, y, z, then max x, y, z. The native
// compares in 007240E0 are x87 FCOMI / JBE (00724248), so a NaN coordinate updates nothing there;
// 00725380's compare instructions were not read beyond the pseudocode's strict less-than.
void grow_box(Vec3& lo, Vec3& hi, const Vec3& p) noexcept {
    if (p.x < lo.x) lo.x = p.x;
    if (p.y < lo.y) lo.y = p.y;
    if (p.z < lo.z) lo.z = p.z;
    if (hi.x < p.x) hi.x = p.x;
    if (hi.y < p.y) hi.y = p.y;
    if (hi.z < p.z) hi.z = p.z;
}

// The three checked vertex reads of one triangle: the index is MOVSX, compared unsigned
// (CMP / JB) against the vertex count, so a negative index fails like a large one.
bool triangle_vertices(const GeomMesh& mesh, std::uint32_t triangle, const Vec3* (&out)[3]) noexcept {
    if (triangle >= mesh.triangles.size()) {
        return false;
    }
    for (std::size_t k = 0; k < 3; ++k) {
        const auto index = static_cast<std::uint32_t>(static_cast<std::int32_t>(mesh.triangles[triangle][k]));
        if (index >= mesh.positions.size()) {
            return false;
        }
        out[k] = &mesh.positions[index];
    }
    return true;
}

} // namespace

std::uint32_t float_radix_key_00724ce0(float v) noexcept {
    const std::uint32_t bits = float_bits(v);
    const std::uint32_t mask = (~(bits >> 31) + 1u) | 0x80000000u;
    return mask ^ bits;
}

int count_tree_nodes_00721c60(int count, int leaf) noexcept {
    // 00721C63 CMP ECX,EDI / JG: count <= leaf is one node.
    if (count <= leaf) {
        return 1;
    }
    const int half = count >> 1; // 00721C72 SAR ESI,1
    const int first = count_tree_nodes_00721c60(count - half, leaf);
    const int second = count_tree_nodes_00721c60(half, leaf);
    return first + second + 1; // 00721C87 LEA EAX,[EBX+EAX+1]
}

GeomMeshBuild build_tree_node_007240e0(const GeomMesh& mesh, GeomMeshPart& part, std::vector<Record16>& records,
                                       int node, int start, int count) {
    const std::size_t n = records.size() / 2;
    for (;;) {
        Vec3 lo{kBoxMinSeed, kBoxMinSeed, kBoxMinSeed};
        Vec3 hi{kBoxMaxSeed, kBoxMaxSeed, kBoxMaxSeed};
        for (int i = 0; i < count; ++i) {
            // 0072414D: the triangle number is the whole dword at record+0Ch.
            const std::uint32_t triangle = records[static_cast<std::size_t>(start + i)].words[3];
            const Vec3* v[3]{};
            if (!triangle_vertices(mesh, triangle, v)) {
                return GeomMeshBuild::invalid_parameter;
            }
            for (const Vec3* p : v) {
                grow_box(lo, hi, *p);
            }
        }
        // The extents are formed before the node is written (00724406..0072445E, FSUB then FSTP
        // to float; one subtraction of two floats, so the double model rounds identically).
        const float dx = static_cast<float>(static_cast<double>(hi.x) - lo.x);
        const float dy = static_cast<float>(static_cast<double>(hi.y) - lo.y);
        const float dz = static_cast<float>(static_cast<double>(hi.z) - lo.z);
        // The image indexes the operator_new arrays without a bound; the capacity n*4/20 + 1
        // covers every node index for n < 70000 (local/lo1_cap.py), so .at() never throws there.
        part.node_min.at(static_cast<std::size_t>(node)) = lo;
        part.node_max.at(static_cast<std::size_t>(node)) = hi;
        if (count <= kLeafTriangles) {
            return GeomMeshBuild::ok;
        }
        // 00724481..007244B8: each later extent is compared (FILD / FCOMI, exactly) with the
        // TRUNCATED best extent so far. Only the y test updates the integer; z is compared with
        // trunc(dx) or trunc(dy). An extent below 1.0 truncates to 0, so a box 0.6 wide in x and
        // 0.4 in y splits on y.
        int axis = 0;
        std::int32_t best = cvttss2si(dx);
        if (static_cast<double>(dy) > static_cast<double>(best)) {
            best = cvttss2si(dy);
            axis = 1;
        }
        if (static_cast<double>(dz) > static_cast<double>(best)) {
            axis = 2;
        }
        // 007230F0 picks 00722210 / 00722400 / 007225F0 for key column 0 / 1 / 2; the scratch
        // pointer [00E19BDC] is the fixed second half of the buffer, not offset by start.
        band_0067_cc13::radix_sort_records16_00722210(records.data() + start, count, records.data() + n, axis);
        const int half = count / 2; // CDQ, SUB EAX,EDX, SAR 1
        const GeomMeshBuild left = build_tree_node_007240e0(mesh, part, records, node * 2 + 1, start, half);
        if (left != GeomMeshBuild::ok) {
            return left;
        }
        // 007244E6..007244F8: the right child reuses the frame (JMP 007240E9).
        count -= half;
        start += half;
        node = node * 2 + 2;
    }
}

GeomMeshBuild build_part_tree_00724ce0(const GeomMesh& mesh, GeomMeshPart& part) {
    const int n = static_cast<int>(part.triangle_list.size());
    part.node_capacity = static_cast<std::uint32_t>(n) * 4u / 20u + 1u;
    part.node_min.assign(part.node_capacity, Vec3{kBoxMinSeed, kBoxMinSeed, kBoxMinSeed});
    part.node_max.assign(part.node_capacity, Vec3{kBoxMaxSeed, kBoxMaxSeed, kBoxMaxSeed});
    // operator_new(n * 2 * 10h): n records, then the n-record scratch area.
    std::vector<Record16> records(static_cast<std::size_t>(n) * 2);
    for (int i = 0; i < n; ++i) {
        const std::uint32_t triangle = part.triangle_list[static_cast<std::size_t>(i)]; // MOVZX
        const Vec3* v[3]{};
        if (!triangle_vertices(mesh, triangle, v)) {
            return GeomMeshBuild::invalid_parameter;
        }
        // 00724EDC..00724F14: (v0 + v1) then + v2, each sum stored as a float.
        const float sx = static_cast<float>(static_cast<float>(static_cast<double>(v[0]->x) + v[1]->x) +
                                            static_cast<double>(v[2]->x));
        const float sy = static_cast<float>(static_cast<float>(static_cast<double>(v[0]->y) + v[1]->y) +
                                            static_cast<double>(v[2]->y));
        const float sz = static_cast<float>(static_cast<float>(static_cast<double>(v[0]->z) + v[1]->z) +
                                            static_cast<double>(v[2]->z));
        Record16& r = records[static_cast<std::size_t>(i)];
        r.words = {float_radix_key_00724ce0(sx), float_radix_key_00724ce0(sy), float_radix_key_00724ce0(sz),
                   triangle};
    }
    const GeomMeshBuild built = build_tree_node_007240e0(mesh, part, records, 0, 0, n);
    if (built != GeomMeshBuild::ok) {
        return built;
    }
    // Write-back of the low word at record+0Ch, in sorted order.
    for (int i = 0; i < n; ++i) {
        part.triangle_list[static_cast<std::size_t>(i)] =
            static_cast<std::uint16_t>(records[static_cast<std::size_t>(i)].words[3] & 0xFFFFu);
    }
    return GeomMeshBuild::ok;
}

GeomMeshBuild build_mesh_trees_00725380(GeomMesh& mesh) {
    mesh.bounds_min = {kBoxMinSeed, kBoxMinSeed, kBoxMinSeed};
    mesh.bounds_max = {kBoxMaxSeed, kBoxMaxSeed, kBoxMaxSeed};
    for (const Vec3& p : mesh.positions) {
        grow_box(mesh.bounds_min, mesh.bounds_max, p);
    }
    for (GeomMeshPart& part : mesh.parts) {
        const GeomMeshBuild built = build_part_tree_00724ce0(mesh, part);
        if (built != GeomMeshBuild::ok) {
            return built;
        }
    }
    return GeomMeshBuild::ok;
}

// --- Landscape patch index lists ---------------------------------------------------------------
// The builders follow the pseudocode's call sequence literally (integer-only code; the
// 00880E50 listing was read against it at 00880E96..00880FAE). Each pushed value is the low 16
// bits of the int expression (MOVZX before the push).

namespace {

constexpr int kPatchRow = 0x21;     // 33 vertices per row
constexpr int kPatchSpan = 0x20;    // 32 cells
constexpr int kPatchLastRow = 0x420; // 32 * 33

void push(PatchIndexList& list, int value) {
    push_patch_index_00880a60(list, static_cast<std::uint16_t>(value & 0xFFFF));
}

} // namespace

void reserve_patch_index_list_004e7690(PatchIndexList& list, std::int32_t new_capacity) {
    const std::int32_t capacity = list.allocated ? list.capacity : 0;
    if (capacity < new_capacity) {
        list.indices.reserve(static_cast<std::size_t>(new_capacity));
        list.allocated = true;
    }
    list.capacity = new_capacity;
}

void push_patch_index_00880a60(PatchIndexList& list, std::uint16_t value) {
    const std::int32_t size = list.allocated ? static_cast<std::int32_t>(list.indices.size()) : 0;
    const std::int32_t capacity = list.allocated ? list.capacity : 0;
    if (size == capacity) {
        std::int32_t grown = capacity + capacity / 4;
        if (grown < 0x21) {
            grown = 0x20;
        }
        reserve_patch_index_list_004e7690(list, grown);
    }
    list.indices.push_back(value);
}

void emit_patch_interior_00880d10(PatchIndexList& list, int res) {
    const int s = kPatchSpan / res;
    if (res - 1 <= 1) {
        return;
    }
    int row = s;
    for (int r = res - 2; r != 0; --r) {
        const int top = row * kPatchRow;
        const int bottom = (row + s) * kPatchRow;
        int col = s;
        for (int c = res - 2; c != 0; --c) {
            const int next = col + s;
            push(list, top + col);
            push(list, top + next);
            push(list, bottom + col);
            push(list, bottom + col);
            push(list, top + next);
            push(list, bottom + next);
            col = next;
        }
        row += s;
    }
}

void emit_patch_edge_row0_00880ff0(PatchIndexList& list, int res, bool stitch) {
    const int s = kPatchSpan / res;
    if (res <= 0) {
        return;
    }
    const int row1 = s * kPatchRow;
    int col = s;
    int col2 = s * 2;
    int k = 0;
    do {
        push(list, col + row1);
        push(list, s * k);
        if (!stitch) {
            push(list, col);
            push(list, col + row1);
            push(list, col);
        }
        push(list, col2);
        if (k < res - 2) {
            push(list, col + row1);
            push(list, col2);
            push(list, col2 + row1);
            push(list, col2 + row1);
            push(list, col2);
            push(list, col2 + s * 0x22);
        }
        col += s * 2;
        k += 2;
        col2 += s * 2;
    } while (k < res);
}

void emit_patch_edge_col32_00881180(PatchIndexList& list, int res, bool stitch) {
    const int s = kPatchSpan / res;
    const int inner = kPatchSpan - s;
    if (res <= 0) {
        return;
    }
    int row = s;
    int row2 = s * 2;
    int k = 0;
    do {
        int a = row * kPatchRow + inner;
        push(list, a);
        push(list, s * k * kPatchRow + kPatchSpan);
        if (!stitch) {
            const int mid = row * kPatchRow + kPatchSpan;
            push(list, mid);
            push(list, a);
            push(list, mid);
        }
        const int b = row2 * kPatchRow + kPatchSpan;
        push(list, b);
        if (k < res - 2) {
            push(list, a);
            push(list, b);
            a = row2 * kPatchRow + inner;
            push(list, a);
            push(list, a);
            push(list, b);
            push(list, (s + row2) * kPatchRow + inner);
        }
        row += s * 2;
        row2 += s * 2;
        k += 2;
    } while (k < res);
}

void emit_patch_edge_row32_00881350(PatchIndexList& list, int res, bool stitch) {
    const int s = kPatchSpan / res;
    if (res <= 0) {
        return;
    }
    const int inner = (kPatchSpan - s) * kPatchRow;
    int col = s;
    int col2 = s * 2;
    int k = 0;
    do {
        push(list, s * k + kPatchLastRow);
        const int a = inner + col;
        push(list, a);
        if (!stitch) {
            push(list, col + kPatchLastRow);
            push(list, col + kPatchLastRow);
            push(list, a);
        }
        push(list, col2 + kPatchLastRow);
        if (k < res - 2) {
            push(list, col2 + kPatchLastRow);
            push(list, a);
            push(list, inner + col2);
            push(list, col2 + kPatchLastRow);
            push(list, inner + col2);
            push(list, inner + col2 + s);
        }
        col += s * 2;
        k += 2;
        col2 += s * 2;
    } while (k < res);
}

void emit_patch_edge_col0_00880e50(PatchIndexList& list, int res, bool stitch) {
    const int s = kPatchSpan / res;
    if (res <= 0) {
        return;
    }
    int row = s;
    int row2 = s * 2;
    int k = 0;
    do {
        push(list, s * k * kPatchRow);
        const int mid = row * kPatchRow;
        const int a = mid + s;
        push(list, a);
        if (!stitch) {
            push(list, mid);
            push(list, mid);
            push(list, a);
        }
        const int b = row2 * kPatchRow;
        push(list, b);
        if (k < res - 2) {
            push(list, b);
            push(list, a);
            push(list, b + s);
            push(list, b);
            push(list, b + s);
            push(list, row2 * kPatchRow + s * 0x22);
        }
        row += s * 2;
        row2 += s * 2;
        k += 2;
    } while (k < res);
}

void emit_patch_lod_00881510(PatchIndexList& list, int res, std::uint32_t flags) {
    if (res == 1) {
        for (const int v : {0, kPatchSpan, kPatchLastRow, kPatchLastRow, kPatchSpan, kPatchLastRow + kPatchSpan}) {
            push(list, v);
        }
        return;
    }
    emit_patch_interior_00880d10(list, res);
    emit_patch_edge_row0_00880ff0(list, res, (flags & 1u) != 0);
    emit_patch_edge_col32_00881180(list, res, (flags & 2u) == 2u);
    emit_patch_edge_row32_00881350(list, res, (flags & 4u) == 4u);
    emit_patch_edge_col0_00880e50(list, res, (flags & 8u) == 8u);
}

void reset_terrain_state_00881a70(LandscapeTerrainState& state) {
    state.dword_40c = 0;
    state.dword_3e8 = 0;
    state.byte_3ec = 0;
    state.byte_3ed = 0;
    state.byte_3e0 = 0;
    state.float_410 = 300.0f; // 00CE3AE8
    state.float_3f0 = 9.375f; // 00D0E658
    for (int res = 2; res < 0x21; res *= 2) {
        for (const std::uint8_t flags : kPatchStitchOrder_00881a70) {
            emit_patch_lod_00881510(state.list_3f4, res, flags);
        }
    }
}

} // namespace bsp::leftovers_cc13

// October 18 tB bounded storage tail; see docs/OCT18_TAIL_B.md.
#include "bsp/oct18_tail_b.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp::oct18_tail_b {
namespace {
static_assert(sizeof(void*) == 4, "Original storage uses Win32 pointers");
template<class T> T get(const void* p, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
template<class T> void put(void* p, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
void* at(void* p, std::size_t offset) {
    return static_cast<std::byte*>(p) + offset;
}
[[noreturn]] void invalid(Host& host) {
    host.invalid_iterator_00bf6713();
    throw std::logic_error("non-returning CRT provider returned");
}
}

// 007810F0 / 0079A380 / 0079D840: __thiscall, RET 4, EAX 0/1;
// vslot 5Ch. These expose the already catalogued class-test leaves over +C4h.
bool class60_is_kind_007810f0(const void* object, int query) {
    return query == 0x60 || query == 2 || query == 1 || query == 0 ||
           query == get<int>(object, 0xc4);
}
bool class54_is_kind_0079a380(const void* object, int query) {
    return query == 0x54 || query == 0x4c || query == 0 || query == get<int>(object, 0xc4);
}
bool class55_is_kind_0079d840(const void* object, int query) {
    return query == 0x55 || query == 0x4c || query == 0 || query == get<int>(object, 0xc4);
}
// 007B8A90: ECX=plane, RET; original two-instruction business body.
void plane_clear_turbo_007b8a90(void* plane) { put<std::uint8_t>(plane, 0x9f9, 0); }

// 008364B0 / 00836790: vslot 48h, unused ECX, stack category, RET 4.
bool director_allows_category_008364b0(int category) noexcept {
    return category == 2 || category == 0;
}
bool ship_director_allows_category_00836790(int category) noexcept {
    return category == 3 || category == 2 || category == 0;
}

// 00836EB0: __thiscall(class, target), RET 4, ST0 float. Assembly shows
// MOVSS/FLD only, with no x87 arithmetic or excess precision to reproduce.
float gun_class_value_for_target_00836eb0(const void* gun_class, void* target, Host& host) {
    return get<float>(gun_class, host.is_kind_vslot_5c(target, 8) ? 0x764 : 0x768);
}
// 00837DE0: ECX=settings, RET, EAX=selected interior pointer. The actual
// [00E188A8] global is supplied by the caller, without substituting host state.
void* ship_tuning_block_00837de0(void* settings, const void* game) {
    return at(settings, get<int>(game, 0x1fe4) != 0 ? 0xf0 : 0x80);
}
// 0084EB60: cdecl/no args, tail delegation to the Lua machine.
void reset_stationary_class_table_0084eb60(Host& host) {
    host.run_lua_006b8ad0("StationaryClass=nil\n", 0, 0, 2);
}

// 00797DA0: __thiscall(track,float), RET 4. Despite its old name it does
// not store time: rewinding before an active cue's +F0h clears +F8h and
// resets its optional +E4h object. FCOMIP/JBE skips unordered comparisons.
void movie_camera_track_set_time_00797da0(void* track, float time, Host& host) {
    void* node = get<void*>(get<void*>(track, 0x6c), 0);
    while (node != get<void*>(track, 0x6c)) {
        void* cue = get<void*>(node, 8);
        if (get<int>(cue, 0xf8) != 0 && time < get<float>(cue, 0xf0)) {
            put<int>(cue, 0xf8, 0);
            void* callback = get<void*>(cue, 0xe4);
            if (callback != nullptr) host.movie_reset_vslot_2c(callback);
        }
        // Re-read the sentinel after the callback, like the native iterator.
        if (node == get<void*>(track, 0x6c)) invalid(host);
        node = get<void*>(node, 0);
    }
}
// 00836680: ECX=destination, stack source, RET 4. EDI preserves destination
// across the base call. Force byte is captured before the source target query.
void director_copy_state_00836680(void* destination, void* source, Host& host) {
    host.copy_controller_00720450(destination, source);
    if (host.category_vslot_48(source, 2)) {
        const auto force = get<std::uint8_t>(source, 0x23c);
        if (force != 0) {
            void* target = host.fire_target_vslot_2c(source);
            host.store_fire_target_00836240(destination, target, force);
        }
    }
}
// 008367F0: __thiscall(director,visitor), RET 4. These are addresses of
// mutable fields, not copied values; tag 3 is a byte and tag 2 a float.
void director_visit_state_008367f0(void* director, void* visitor, Host& host) {
    host.visit_enables_00836510(director, visitor);
    struct Field { const char* name; std::size_t offset; int type; };
    constexpr Field fields[] = {
        {"torpedoAvoidance", 0x240, 3}, {"shipCollisionAvoidance", 0x241, 3},
        {"landCollisionAvoidance", 0x242, 3}, {"cruiseIsHeading", 0x243, 3},
        {"cruiseSteerOrHeading", 0x244, 2}, {"cruiseThrust", 0x248, 2}};
    for (const auto& field : fields)
        host.visit_reference_vslot_10(visitor, field.name, field.type, at(director, field.offset));
}
// 0084EFE0: ECX=stationary, RET, nullable part+160h query. The part link
// has no recovered descriptive type; this function claims only its offsets.
void* stationary_part_scene_0084efe0(const void* stationary) {
    void* part = get<void*>(stationary, 0x19c);
    return part != nullptr ? get<void*>(part, 0x160) : nullptr;
}
// 00784CD0: ECX=transport, RET, bool in AL. Deliberately clear the FIRST
// peer's flag when the count is one, even if a later peer supplied that flag.
bool consume_single_peer_flag_00784cd0(void* transport, Host& host) {
    std::uint32_t count = 0;
    for (void* node = get<void*>(get<void*>(transport, 0xc), 0);
         node != get<void*>(transport, 0xc); node = get<void*>(node, 0)) {
        void* peer = get<void*>(node, 8);
        if (peer != nullptr) {
            void* record = get<void*>(peer, 0xd50);
            if (record != nullptr && get<std::uint8_t>(record, 0x1a) != 0) ++count;
        }
    }
    if (count != 1) return false;
    void* first = get<void*>(get<void*>(transport, 0xc), 0);
    if (first == get<void*>(transport, 0xc)) invalid(host);
    put<std::uint8_t>(get<void*>(get<void*>(first, 8), 0xd50), 0x1a, 0);
    return true;
}
// 00968470: __thiscall(subscription,parameter-vector), RET 4, EAX 0/1.
// Keep lazy bounds checks: a rejecting earlier filter does not inspect later
// parameters. Re-read the vector and filter pointers after each virtual call.
bool recon_subscription_matches_00968470(const void* subscription,
                                         const void* parameters, Host& host) {
    for (std::uint32_t i = 0; i != 4; ++i) {
        const auto begin = get<std::uintptr_t>(parameters, 4);
        const auto end = get<std::uintptr_t>(parameters, 8);
        if (begin == 0 || ((end - begin) >> 2) < i + 1) invalid(host);
        const auto value = get<std::uint32_t>(reinterpret_cast<void*>(begin), i * 4);
        void* filter = get<void*>(subscription, 0xc + i * 0x10);
        if (!host.filter_vslot_00(filter, value)) return false;
    }
    return true;
}
} // namespace bsp::oct18_tail_b

#include <initializer_list>
namespace bsp::oct18_tail_b {
namespace {
void enter_lock(void* lock, Host& host) {
    host.enter_critical_section(lock);
    put<std::uint32_t>(lock, 0x18, get<std::uint32_t>(lock, 0x18) + 1);
}
void leave_lock(void* lock, Host& host) {
    put<std::uint32_t>(lock, 0x18, get<std::uint32_t>(lock, 0x18) - 1);
    host.leave_critical_section(lock);
}
std::uint32_t vector_count(const void* object, std::size_t first_offset) {
    const auto first = get<std::uintptr_t>(object, first_offset);
    return first == 0 ? 0 : (get<std::uintptr_t>(object, first_offset + 4) - first) / 4;
}
void clear_pointer_list(void* object, std::size_t head_offset, bool free_head, Host& host) {
    void* head = get<void*>(object, head_offset);
    void* node = get<void*>(head, 0);
    put<void*>(head, 0, head);
    put<void*>(head, 4, head);
    put<std::uint32_t>(object, head_offset + 4, 0);
    while (node != get<void*>(object, head_offset)) {
        void* next = get<void*>(node, 0);
        host.free_00bf65ac(node);
        node = next;
    }
    if (free_head) {
        host.free_00bf65ac(get<void*>(object, head_offset));
        put<void*>(object, head_offset, nullptr);
    }
}
}

// 0077C710, __thiscall(entity,message), RET 4. Signed stale-tick comparison
// is not a modular uint16 subtraction: +0Ch is a signed 32-bit message tick.
void deliver_entity_sync_0077c710(void* entity, void* message, Host& host) {
    if (get<std::uint8_t>(entity, 0x5e) != 0) return;
    for (void* node = get<void*>(get<void*>(entity, 0x2a8), 0);
         node != get<void*>(entity, 0x2a8); node = get<void*>(node, 0)) {
        if (get<void*>(node, 8) != get<void*>(message, 0x14)) continue;
        if (reinterpret_cast<std::uintptr_t>(node) == 0xfffffff8u) return;
        const auto tick = get<std::int32_t>(message, 0xc);
        const auto previous = get<std::uint16_t>(node, 0x10);
        const auto delta = static_cast<std::int32_t>(static_cast<std::uint32_t>(previous) -
                                                    static_cast<std::uint32_t>(tick));
        if (tick < previous && delta <= 0x7530) return;
        put<std::uint16_t>(node, 0x10, static_cast<std::uint16_t>(tick));
        host.deliver_entity_vslot_18c(entity, message);
        return;
    }
}
// 0077DA20, __thiscall(entity,peer-record), RET 4. First remove holder from
// +2A4h through its library erase, then unlink/free the matching +298h node.
void remove_peer_holder_0077da20(void* entity, void* record, Host& host) {
    for (void* node = get<void*>(get<void*>(entity, 0x2a8), 0);
         node != get<void*>(entity, 0x2a8); node = get<void*>(node, 0)) {
        if (get<void*>(node, 8) == record) {
            host.erase_peer_holder_0077d980(at(entity, 0x2a4), node);
            break;
        }
    }
    for (void* node = get<void*>(get<void*>(entity, 0x29c), 0);
         node != get<void*>(entity, 0x29c); node = get<void*>(node, 0)) {
        if (get<void*>(node, 8) != record) continue;
        put<void*>(get<void*>(node, 4), 0, get<void*>(node, 0));
        put<void*>(get<void*>(node, 0), 4, get<void*>(node, 4));
        host.free_00bf65ac(node);
        put<std::uint32_t>(entity, 0x2a0, get<std::uint32_t>(entity, 0x2a0) - 1);
        return;
    }
}
// 007844E0, ECX=network, RET. Normal destructor path, including the four
// list heads and tracked recursive-lock depths; compiler EH is not reproduced.
void network_destruct_007844e0(void* network, Host& host) {
    put<std::uint32_t>(network, 0, 0x00d04274);
    for (const std::size_t offset : {std::size_t{0xc}, std::size_t{0x3c}}) {
        for (void* node = get<void*>(get<void*>(network, offset), 0);
             node != get<void*>(network, offset); node = get<void*>(node, 0)) {
            void* value = get<void*>(node, 8);
            if (value != nullptr) host.scalar_delete_vslot_00(value, 1);
            if (node == get<void*>(network, offset)) invalid(host);
        }
        clear_pointer_list(network, offset, false, host);
    }
    for (const std::size_t offset : {std::size_t{0x44}, std::size_t{0x24}, std::size_t{0x34}}) {
        void* lock = get<void*>(network, offset);
        if (lock == nullptr) continue;
        while (get<std::int32_t>(lock, 0x18) > 0) leave_lock(lock, host);
        host.delete_critical_section(lock);
        host.free_00bf65ac(lock);
    }
    for (const std::size_t offset : {std::size_t{0x3c}, std::size_t{0x2c},
                                     std::size_t{0x1c}, std::size_t{0xc}})
        clear_pointer_list(network, offset, true, host);
}
// 00785170, ECX=client, RET. Deletion callbacks may change the stored end;
// the iteration and lock release therefore read it again at every native site.
void clear_host_lists_00785170(void* client, Host& host) {
    enter_lock(get<void*>(client, 0xc0), host);
    for (const std::size_t first : {std::size_t{0xa4}, std::size_t{0xb4}}) {
        auto cursor = get<std::uintptr_t>(client, first);
        if (get<std::uintptr_t>(client, first + 4) < cursor) invalid(host);
        while (cursor != get<std::uintptr_t>(client, first + 4)) {
            if (get<std::uintptr_t>(client, first + 4) < get<std::uintptr_t>(client, first) ||
                get<std::uintptr_t>(client, first + 4) <= cursor) invalid(host);
            void* value = get<void*>(reinterpret_cast<void*>(cursor), 0);
            if (value != nullptr) host.scalar_delete_vslot_00(value, 1);
            if (get<std::uintptr_t>(client, first + 4) <= cursor) invalid(host);
            cursor += 4;
        }
        if (get<std::uintptr_t>(client, first + 4) < get<std::uintptr_t>(client, first)) invalid(host);
        put<void*>(client, first + 4, get<void*>(client, first));
    }
    leave_lock(get<void*>(client, 0xc0), host);
}
// 007857F0, __thiscall(network,peer), RET 4. 00773600 is a checked size
// increment, not an unconditional throw; preserve allocation/grow/link order.
void queue_peer_removal_007857f0(void* network, void* peer, std::uint32_t timer_bits, Host& host) {
    void* node = get<void*>(get<void*>(network, 0xc), 0);
    while (node != get<void*>(network, 0xc) && get<void*>(node, 8) != peer)
        node = get<void*>(node, 0);
    if (node == get<void*>(network, 0xc)) return;
    enter_lock(get<void*>(network, 0x24), host);
    node = get<void*>(get<void*>(network, 0x1c), 0);
    while (node != get<void*>(network, 0x1c) && get<void*>(node, 8) != peer)
        node = get<void*>(node, 0);
    if (node == get<void*>(network, 0x1c)) {
        void* head = get<void*>(network, 0x1c);
        void* inserted = host.buy_peer_node_0076ea10(at(network, 0x18), head, get<void*>(head, 4), peer);
        host.grow_list_00773600(at(network, 0x18), 1);
        put<void*>(head, 4, inserted);
        put<void*>(get<void*>(inserted, 4), 0, inserted);
        put<std::uint32_t>(network, 0x48, timer_bits); // actual [00CE38C8]
    }
    leave_lock(get<void*>(network, 0x24), host);
}
// 00785CC0, ECX=client, RET. Pending vector drains front-first. Replace
// before deleting the old host; retain current pending cursor across callbacks.
void merge_pending_hosts_00785cc0(void* client, Host& host) {
    enter_lock(get<void*>(client, 0xc0), host);
    void* pending = get<void*>(client, 0xb4);
    while (pending != get<void*>(client, 0xb8)) {
        if (get<std::uintptr_t>(client, 0xb8) < get<std::uintptr_t>(client, 0xb4)) invalid(host);
        std::uint32_t index = 0;
        for (; index < vector_count(client, 0xa4); ++index) {
            if (host.host_address_equal_vslot_34(client,
                    get<void*>(get<void*>(client, 0xa4), index * 4), get<void*>(pending, 0))) break;
        }
        if (index == vector_count(client, 0xa4)) {
            if (get<void*>(client, 0xa4) == nullptr ||
                vector_count(client, 0xa4) >=
                    (get<std::uintptr_t>(client, 0xac) - get<std::uintptr_t>(client, 0xa4)) / 4) {
                host.insert_host_vector_00785700(at(client, 0xa0), get<void*>(client, 0xa8),
                                                static_cast<void* const*>(pending));
            } else {
                void* end = get<void*>(client, 0xa8);
                put<void*>(end, 0, get<void*>(pending, 0));
                put<void*>(client, 0xa8, at(end, 4));
            }
        } else {
            void* slot = at(get<void*>(client, 0xa4), index * 4);
            void* old = get<void*>(slot, 0);
            put<void*>(slot, 0, get<void*>(pending, 0));
            if (old != nullptr) host.scalar_delete_vslot_00(old, 1);
        }
        const auto remaining = static_cast<std::int32_t>(get<std::uintptr_t>(client, 0xb8) -
                               reinterpret_cast<std::uintptr_t>(pending) - 4);
        if (remaining > 0) std::memmove(pending, at(pending, 4), static_cast<std::size_t>(remaining));
        put<std::uintptr_t>(client, 0xb8, get<std::uintptr_t>(client, 0xb8) - 4);
    }
    leave_lock(get<void*>(client, 0xc0), host);
}
// 008363E0, __thiscall(director,endpoint,byte), RET 8, EAX receiver.
// When a class test succeeds, preserve the base's permission bytes verbatim.
void* construct_director_base_008363e0(void* director, void* endpoint, std::uint8_t flag, Host& host) {
    host.construct_controller_00720180(director, endpoint);
    put<std::uint32_t>(director, 0, 0x00d09ec0);
    put<std::uint32_t>(director, 0x1c, 0x00d09ea8);
    for (const std::size_t offset : {std::size_t{0x228}, std::size_t{0x22c},
                                     std::size_t{0x230}, std::size_t{0x238}})
        put<std::uint32_t>(director, offset, 0);
    put<std::uint8_t>(director, 0x234, 1);
    put<std::uint32_t>(director, 0x224, 0x00cf938c);
    put<std::uint8_t>(director, 0x23d, flag);
    put<std::uint8_t>(director, 0x23c, 0);
    if (!host.is_kind_vslot_5c(get<void*>(director, 0x34), 0xb) &&
        !host.is_kind_vslot_5c(get<void*>(director, 0x34), 9)) {
        put<std::uint8_t>(director, 0x3c, 1);
        put<std::uint8_t>(director, 0x3d, 1);
    }
    return director;
}
// 008360C0, __thiscall(director,visitor), RET 4. 0071BDD0 is a proven
// no-op, so the nested _gameUnit scope has no additional business operation.
void director_visit_properties_008360c0(void* director, void* visitor, Host& host) {
    host.visitor_open_vslot_04(visitor, "_gameUnit");
    host.visitor_close_vslot_08(visitor);
    constexpr const char* names[] = {"artilleryEnabled", "aaEnabled", "torpedoEnabled", "depthChargeEnabled"};
    for (unsigned i = 0; i != 4; ++i)
        host.visitor_value_vslot_0c(visitor, names[i], 3, get<std::uint8_t>(director, 0x220 + i));
    host.visitor_value_vslot_0c(visitor, "fireTargetID", 4, get<std::uint32_t>(director, 0x238));
    host.visitor_value_vslot_0c(visitor, "fireTargetIsPrimary", 3, get<std::uint8_t>(director, 0x23c));
}
// 008362A0, __thiscall(director,visitor), RET 4. Float values travel as
// their 32-bit boxed payload, without arithmetic conversion.
void director_dump_properties_008362a0(void* director, void* visitor, Host& host) {
    director_visit_properties_008360c0(director, visitor, host);
    constexpr const char* names[] = {"torpedoAvoidance", "shipCollisionAvoidance", "landCollisionAvoidance", "cruiseIsHeading"};
    for (unsigned i = 0; i != 4; ++i)
        host.visitor_value_vslot_0c(visitor, names[i], 3, get<std::uint8_t>(director, 0x240 + i));
    host.visitor_value_vslot_0c(visitor, "cruiseSteerOrHeading", 2, get<std::uint32_t>(director, 0x244));
    host.visitor_value_vslot_0c(visitor, "cruiseThrust", 2, get<std::uint32_t>(director, 0x248));
}
// 0084D810, __thiscall(block,squadron), RET 4. Full storage/permission and
// allocation policy; dependency constructors retain their own lifetime contract.
void* construct_squadron_block_0084d810(void* block, void* squadron, Host& host) {
    host.construct_controller_00720180(block, squadron == nullptr ? nullptr : at(squadron, 0x310));
    put<std::uint32_t>(block, 0, 0x00d0bd98);
    put<std::uint32_t>(block, 0x1c, 0x00d0bd7c);
    put<void*>(block, 0x224, squadron);
    put<std::uint8_t>(block, 0x228, 0);
    const bool fighter = host.class_is_kind_vslot_18(get<void*>(squadron, 0x35c), 0x13);
    const auto behaviour = get<std::int32_t>(get<void*>(block, 0x224), 0x364);
    if (behaviour != 0 && !fighter) {
        put<std::uint8_t>(block, 0x3c, 0);
        put<std::uint8_t>(block, 0x3d, 1);
    } else {
        put<std::uint8_t>(block, 0x3d, behaviour == 0 ? 1 : 0);
        put<std::uint8_t>(block, 0x3c, 1);
    }
    void* memory = host.allocate_00bf681b(0x40);
    put<void*>(block, 0x38, memory == nullptr ? nullptr :
        host.construct_auto_target_009f7590(memory, block, squadron));
    return block;
}
// 0084EC10, ECX=message, RET. Globals [00F87560,00F87564) are supplied
// by address so callbacks can retain their genuine registry identity.
void stationary_apply_state_0084ec10(void* message, void* const* first_global,
                                    void* const* end_global, Host& host) {
    void* first = *first_global;
    const auto count = first == nullptr ? 0u :
        (reinterpret_cast<std::uintptr_t>(*end_global) - reinterpret_cast<std::uintptr_t>(first)) / 4;
    const auto index = get<std::uint32_t>(message, 0x18);
    if (index >= count) return;
    void* object = get<void*>(first, index * 4);
    if (get<std::uint8_t>(message, 0x1c) != 0) put<std::uint8_t>(object, 0x1a4, 1);
    if (get<std::uint8_t>(object, 0x5d) != 0) host.stationary_dead_vslot_7c(object);
    else host.stationary_live_vslot_70(object, 1);
}
// 008826F0, ECX=landscape, RET. Re-read ocean and object slots after each
// call. Reference release provider owns the original atomic decrement/zero test.
void landscape_release_terrain_008826f0(void* landscape, const void* game, Host& host) {
    for (const std::size_t offset : {std::size_t{0x368}, std::size_t{0x378}}) {
        for (unsigned index = 0; index < vector_count(landscape, offset); ++index)
            host.release_body_00c34f70(get<void*>(game, 0x18),
                get<void*>(get<void*>(landscape, offset), index * 4));
        host.resize_body_vector_00882620(at(landscape, offset - 4), 0, 0);
    }
    if (get<void*>(landscape, 0x3cc) != nullptr) {
        void* ocean = get<void*>(game, 0x19f0);
        if (ocean != nullptr) host.remove_floating_object_00b9fe00(get<void*>(ocean, 0xa8), get<void*>(landscape, 0x3cc));
        if (get<void*>(landscape, 0x3cc) != nullptr) {
            host.unlink_release_node_00b6dfa0(get<void*>(landscape, 0x3cc));
            put<void*>(landscape, 0x3cc, nullptr);
        }
    }
    if (get<void*>(landscape, 0x42c) != nullptr) {
        void* ocean = get<void*>(game, 0x19f0);
        if (ocean != nullptr) host.remove_wave_object_vslot_1c(get<void*>(ocean, 0xa8), get<void*>(landscape, 0x42c));
        put<void*>(landscape, 0x42c, nullptr);
    }
    if (get<void*>(landscape, 0x418) != nullptr) {
        host.terrain_delete_vslot_04(get<void*>(landscape, 0x418), 1);
        put<void*>(landscape, 0x418, nullptr);
    }
    for (const std::size_t offset : {std::size_t{0x41c}, std::size_t{0x3d0},
                                     std::size_t{0x3e8}, std::size_t{0x3d4}, std::size_t{0x3d8}}) {
        void* value = get<void*>(landscape, offset);
        if (value != nullptr) {
            host.release_reference(value);
            put<void*>(landscape, offset, nullptr);
        }
    }
}
// 00864CA0, __thiscall(ai,target,gun), RET 8. Listing recovers EDI=record
// and the map value at node+10h, missing in the old pseudocode.
void add_gun_to_target_00864ca0(void* ai, void* target, void* gun, Host& host) {
    void* tree = at(ai, 0xb0);
    const auto found = host.find_target_00863b10(tree, target);
    if (found.owner == nullptr || found.owner != tree) invalid(host);
    void* record;
    if (found.node == get<void*>(tree, 4)) {
        void* memory = host.allocate_00bf681b(0x8c);
        record = memory == nullptr ? nullptr :
            host.construct_target_00864880(memory, get<void*>(ai, 0x50), target);
        host.insert_target_00864160(tree, target, record);
    } else record = get<void*>(found.node, 0x10);
    host.register_gun_00862660(record, gun);
}
// 0081BFE0, ECX=SHIP_CREATE message, RET, EAX=created entity. The listing
// recovers class id at +1Eh and DL=1, absent from the old decompile. Copy
// exactly the resized byte length; the original does not request a terminator.
void* instantiate_ship_message_0081bfe0(void* message, Host& host) {
    void* descriptor = host.vehicle_class_00964790(get<std::uint16_t>(message, 0x1e), true);
    void* entity = host.instantiate_class_vslot_28(descriptor, get<std::uint16_t>(message, 0x1c));
    const char* name = get<const char*>(message, 0xc0);
    const auto length = name == nullptr ? 0u : static_cast<std::uint32_t>(std::strlen(name));
    host.native_string_resize_0041dd40(at(entity, 0x154), length, false);
    void* destination = get<void*>(entity, 0x158);
    const auto copied = get<std::uint32_t>(entity, 0x154);
    if (destination != nullptr && copied != 0) std::memcpy(destination, name, copied);
    return entity;
}
// 0091D620, ECX=receiver, RET, EAX=receiver. ESI preserves the original
// receiver across the required 0091CE90 constructor; +284h meaning is provisional.
void* construct_score_record_tail_0091d620(void* receiver, Host& host) {
    host.construct_score_record_0091ce90(receiver);
    put<std::uint32_t>(receiver, 0x284, 0);
    return receiver;
}
// 007701F0, __thiscall(session,index), RET 4. The selected record pointer
// is captured before 00783F20; the game global is loaded only after all copies.
// Globals flag/empty bytes are actual 00F1AF2C/00CE3A0C storage, supplied by caller.
void select_listed_server_007701f0(void* session, std::uint32_t index,
                                  std::uint32_t& scene_flag, const void* empty_bytes, Host& host) {
    void* client = get<void*>(session, 0x18c);
    if (get<void*>(client, 0xa4) == nullptr || index >= vector_count(client, 0xa4)) invalid(host);
    void* record = get<void*>(get<void*>(get<void*>(client, 0xa4), index * 4), 0x20);
    host.select_host_00783f20(get<void*>(session, 0x18c), index);
    unsigned offset = 0;
    std::uint8_t ch;
    do {
        ch = get<std::uint8_t>(record, offset);
        put<std::uint8_t>(session, 0x100 + offset, ch);
        ++offset;
    } while (ch != 0);
    for (const std::size_t field : {std::size_t{0x25}, std::size_t{0x27},
                                     std::size_t{0x29}, std::size_t{0x69}})
        put<std::uint8_t>(session, 0x100 + field, get<std::uint8_t>(record, field));
    put<std::uint32_t>(session, 0x170, get<std::uint32_t>(record, 0x70));
    put<std::uint32_t>(session, 0x174, get<std::uint32_t>(record, 0x74));
    void* game = host.game_global_00e188a8();
    scene_flag = 0;
    host.scene_records_destroy_all_004bf930(at(game, 0x5f0));
    put<std::uint32_t>(game, 0x5fc, 0);
    host.native_string_resize_0041dd40(at(game, 0x600), 0, false);
    void* destination = get<void*>(game, 0x604);
    const auto length = get<std::uint32_t>(game, 0x600);
    if (destination != nullptr && length != 0) std::memcpy(destination, empty_bytes, length);
}
// The following are game entry aliases, not library or compiler-EH exclusions.
// 007B8A80: MOV DL,1 / JMP00964790; ECX type id, EAX descriptor.
void* vehicle_class_get_or_create_007b8a80(std::uint32_t type_id, Host& host) {
    return host.vehicle_class_00964790(type_id, true);
}
// 00835610: JMP0071C1E0, unchanged ECX director and stack message, RET 4
// supplied by target. Contract also documented in gameplay_seg38_cc13.hpp.
void director_apply_command_thunk_00835610(void* director, void* message, Host& host) {
    host.apply_director_command_0071c1e0(director, message);
}
// 008FE790: JMP006DF170, unchanged bot/target; target source declaration
// is gun_bot_set_fire_target_006df170 in cc13_seg33a.hpp.
void gunbot_set_target_thunk_008fe790(void* bot, void* target, Host& host) {
    host.set_gunbot_target_006df170(bot, target);
}
// 00852200: JMP00828F20, ECX descriptor, no stack argument, AL boolean.
// This binds the existing arithmetic body; it does not duplicate its x87 math.
bool ship_derive_turn_fields_thunk_00852200(void* descriptor, Host& host) {
    return host.derive_ship_turn_fields_00828f20(descriptor);
}
} // namespace bsp::oct18_tail_b

#include "bsp/entity_mission_seg60_argon.hpp"
namespace bsp::oct18_tail_b {
// 0093A3F0, ECX=task, RET, ST0 float. Last matching record wins; compare
// through the canonical CRT binding. Listing is MOVSS/FLD, with no x87 math.
float engine_jam_seconds_0093a3f0(const void* task, Host& host) {
    float seconds = 0.0f;
    for (std::uint32_t index = 0;; ++index) {
        void* begin = get<void*>(task, 0x18);
        if (begin == nullptr || index >=
            (get<std::uintptr_t>(task, 0x1c) - reinterpret_cast<std::uintptr_t>(begin)) / 16) break;
        void* record = at(begin, index * 16);
        const char* name = get<const char*>(record, 8);
        if (name != nullptr && host.compare_case_insensitive_00bf7fbf(name, "EngineJam") == 0)
            seconds = get<float>(record, 0xc);
    }
    return seconds;
}
// 0095C550, ECX=class id, RET. Supplied actual vector is 00F8A09C, not
// an independent census. Keep the duplicate scan and capacity fast append.
void append_vehicle_census_0095c550(void* vector, std::int32_t class_id, Host& host) {
    auto cursor = get<std::uintptr_t>(vector, 4);
    const auto end = get<std::uintptr_t>(vector, 8);
    if (end < cursor) invalid(host);
    while (cursor != end) {
        if (get<std::int32_t>(reinterpret_cast<void*>(cursor), 0) == class_id) return;
        cursor += 4;
    }
    const auto begin = get<std::uintptr_t>(vector, 4);
    if (begin != 0 && (end - begin) / 4 < (get<std::uintptr_t>(vector, 0xc) - begin) / 4) {
        put<std::int32_t>(reinterpret_cast<void*>(end), 0, class_id);
        put<std::uintptr_t>(vector, 8, end + 4);
    } else host.insert_census_00441f30(vector, reinterpret_cast<void*>(end), &class_id);
}
// 009277E0: five-byte JMP00923840, unchanged ECX. Bind the existing
// complete raw-storage implementation rather than copy its flag writes.
void inherit_race_activate_thunk_009277e0(void* entity) {
    entity_mission_seg60_argon::entity_inherit_race_and_activate_00923840(entity);
}
// 00922DC0: MOV DL,1 / JMP00922C80, AL result. The existing source's
// actual allow_far argument is forced to one, exactly as the native thunk.
std::uint8_t surface_target_thunk_00922dc0(void* entity,
                                          entity_mission_seg60_argon::Seg60Host& host) {
    return entity_mission_seg60_argon::entity_is_surface_target_00922c80(host, entity, 1);
}
// 008429C0: JMP00960230, unchanged ECX descriptor / stack LuaObject row.
// The target's original contract is __thiscall(descriptor,row), RET 4.
void read_vehicle_fields_thunk_008429c0(void* descriptor, void* lua_row, Host& host) {
    host.read_vehicle_fields_00960230(descriptor, lua_row);
}
// 00968AF0 and 00968470 have identical raw filter/parameter layouts and
// ordered short-circuit checks. Reuse the complete checked storage body.
bool event_subscription_matches_00968af0(const void* subscription, const void* parameters, Host& host) {
    return recon_subscription_matches_00968470(subscription, parameters, host);
}
// 008EE670: ECX=nullable text, RET, EAX=00F88C50 scratch. The source API
// supplies that actual scratch explicitly; forward aliasing/read/write order
// is retained. Preconditions: terminated input, no lone trailing backslash,
// and sufficient scratch storage, matching the native valid domain.
char* decode_scene_fragment_008ee670(const char* input, char* scratch) {
    char* output = scratch;
    if (input != nullptr) {
        while (*input != '\0') {
            if (*input == '\\') {
                const char escaped = input[1];
                input += 2;
                if (escaped == '\\') *output++ = '\\';
                else if (escaped == 'n') *output++ = '\n';
                else if (escaped == 'q') *output++ = '"';
            } else *output++ = *input++;
        }
    }
    *output = '\0';
    return scratch;
}
} // namespace bsp::oct18_tail_b
