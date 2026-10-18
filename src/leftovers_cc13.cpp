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
