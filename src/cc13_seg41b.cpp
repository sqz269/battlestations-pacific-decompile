// cc13_seg41b: skipped segment 41 / 33 functions. Evidence and ABI: include/bsp/cc13_seg41b.hpp
// and docs/CC13_SEG41B.md. Not wired into the game.
#include "bsp/cc13_seg41b.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp::cc13_seg41b {
namespace {

using cc13_seg41::at;
using cc13_seg41::dword_of;
using cc13_seg41::ptr;

float bits_float(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The inlined NativeStr construction: {len 0, data 0}, 0041DD40(len, 1), memcpy(data, text,
// len + 1) when data is set.
void native_string_from_literal(std::int32_t* key, const char* text, std::uint32_t length,
                                cc13_seg41::NativeStringHost& host) {
    key[0] = 0;
    key[1] = 0;
    host.resize_0041dd40(key, length, 1);
    if (key[1] != 0)
        host.memcpy_00bf7680(ptr(static_cast<std::uint32_t>(key[1])), text, static_cast<std::size_t>(key[0]) + 1u);
}
// The inlined ~NativeStr: 00419CC0(data, len + 1, 1) then 00BD1510 on the pool.
void native_string_release(std::int32_t* key, cc13_seg41::NativeStringHost& host) {
    if (key[1] != 0)
        host.release_00419cc0_00bd1510(ptr(static_cast<std::uint32_t>(key[1])), static_cast<std::size_t>(key[0]) + 1u);
}

// The image's checked std::vector<T*> at +398h/+39Ch: 00BF6713 when begin is null or i >= size.
void check_member_index(Object convoy, std::uint32_t i, LandConvoyInitHost& host) {
    const std::uint32_t begin = at<std::uint32_t>(convoy, 0x398);
    if (begin == 0 || i >= ((at<std::uint32_t>(convoy, 0x39C) - begin) >> 2)) host.invalid_iterator_00bf6713();
}

}  // namespace

// 00742C10. Evidence: the listing 00742C10-0074305C (Ghidra's pseudocode drops every ECX).
void land_convoy_post_load_init_00742c10(Object convoy, LandConvoyInitHost& host) {
    host.net_entity_read_replication_state_0077fad0(convoy);  // 00742C2E
    at<float>(convoy, 0x3C0) = 0.0f;                          // 00742C48
    at<std::uint32_t>(convoy, 0x3C4) = 0xD01502F9u;           // 00742C50: [00CE4ADC] = -1.0e10f
    at<float>(convoy, 0x3B4) = 0.0f;                          // 00742C58
    if (at<std::uint32_t>(convoy, 0x344) != 0) {
        // 00742C70: the placement law (land_and_structures.hpp land_convoy_uses_world_pose_law_00742c70).
        Object entity = host.path_object_get_entity_007aca30(ptr(at<std::uint32_t>(convoy, 0x344)));
        at<std::uint8_t>(convoy, 0x3C8) = at<std::uint32_t>(entity, 0x3C) != at<std::uint32_t>(convoy, 0x3C) ? 1 : 0;
    }
    at<std::uint32_t>(convoy, 0x3A4) = at<std::uint32_t>(convoy, 0x3C);  // 00742C81

    std::int32_t key[2];
    LuaSlot self{};
    native_string_from_literal(key, "Members", 7, host);  // 00742C8F, 00CFF328
    host.lua_set_new_table_00b67580(host.entity_lua_self_object_00927b40(convoy, &self), key);  // 00742CCE
    host.lua_object_destruct_00b67700(self.bytes);
    native_string_release(key, host);

    LuaSlot members{};
    host.lua_get_by_name_00b67800(host.entity_lua_self_object_00927b40(convoy, &self), &members,
                                  "Members");  // 00742D2B
    host.lua_object_destruct_00b67700(self.bytes);

    // 00742D3E: indices 0 .. +354h * +358h - 1 (signed), the count re-read every pass.
    for (std::uint32_t i = 0;
         static_cast<std::int32_t>(i) < at<std::int32_t>(convoy, 0x354) * at<std::int32_t>(convoy, 0x358);) {
        check_member_index(convoy, i, host);
        if (at<std::uint32_t>(ptr(at<std::uint32_t>(convoy, 0x398)), i * 4) == 0) {
            ++i;
            host.lua_set_nil_at_index_00b664e0(members.bytes, static_cast<std::int32_t>(i));  // 00742E37
            continue;
        }
        Object parent_object = ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(convoy, 0x3A4)), 0x3D0));
        check_member_index(convoy, i, host);
        const std::uint32_t parent = host.parent_object_vfunc_40(parent_object);  // 00742DC4
        Object member = ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(convoy, 0x398)), i * 4));
        host.node_set_parent_00b6e680(ptr(at<std::uint32_t>(member, 0x4A4)), parent);  // 00742DD0
        check_member_index(convoy, i, host);
        member = ptr(at<std::uint32_t>(ptr(at<std::uint32_t>(convoy, 0x398)), i * 4));
        LuaSlot member_self{};
        Object value = host.entity_lua_self_object_00927b40(member, &member_self);  // 00742E02
        ++i;
        host.lua_set_object_at_index_00b666c0(members.bytes, static_cast<std::int32_t>(i), value);  // 00742E18
        host.lua_object_destruct_00b67700(member_self.bytes);
    }

    native_string_from_literal(key, "Class", 5, host);  // 00742E61, 00CE44C8
    LuaSlot class_self{};
    host.lua_set_new_table_00b67580(host.entity_lua_self_object_00927b40(convoy, &class_self), key);  // 00742EA2
    host.lua_object_destruct_00b67700(class_self.bytes);
    native_string_release(key, host);

    LuaSlot outer{};
    Object class_table = host.lua_get_by_name_00b67800(host.entity_lua_self_object_00927b40(convoy, &outer),
                                                       &class_self, "Class");  // 00742EF8
    host.lua_set_cstring_00b66790(class_table, "Type", "LandVehicle");          // 00742F11: 00CE4780, 00CE5858
    host.lua_object_destruct_00b67700(class_self.bytes);
    host.lua_object_destruct_00b67700(outer.bytes);

    if (at<std::uint32_t>(convoy, 0x344) != 0) {
        Object entity = host.path_object_get_entity_007aca30(ptr(at<std::uint32_t>(convoy, 0x344)));  // 00742F40
        if (entity != nullptr && host.path_entity_vfunc_5c(entity, 1)) {                             // 00742F58
            std::int32_t path_key[2];
            native_string_from_literal(path_key, "Path", 4, host);  // 00742F72, 00CEA738
            LuaSlot entity_self{};
            Object value = host.entity_lua_self_object_00927b40(entity, &entity_self);  // 00742FA1
            host.lua_set_object_00b675d0(host.entity_lua_self_object_00927b40(convoy, &outer), path_key,
                                         value);  // 00742FCA
            host.lua_object_destruct_00b67700(outer.bytes);
            host.lua_object_destruct_00b67700(entity_self.bytes);
            native_string_release(path_key, host);
        }
    }
    host.land_convoy_place_members_00742400(convoy);    // 00743010
    host.sub_object_vfunc_0c(convoy + 0x310);           // 00743024
    host.entity_vfunc_148(convoy, 0x1FF, 8);            // 00743037
    host.lua_object_destruct_00b67700(members.bytes);  // 00743045
}

// 00748CC0. Evidence: the listing 00748CC0-00748E78; the reader lives at [esp+10h] of the frame.
void land_fort_load_saved_state_00748cc0(Object fort, LandFortLoadHost& host) {
    host.unit_read_common_lua_properties_0095e5b0(fort);  // 00748CDC
    const std::uint32_t saved = at<std::uint32_t>(fort, 0xC0);
    if (saved != 0 && at<std::int32_t>(ptr(saved), 4) == 3) {
        LuaSlot lua_data{};
        LuaSlot reader{};
        host.saved_entity_lua_data_009238a0(fort, &lua_data);       // 00748D05
        host.lua_reader_construct_004425c0(&reader, &lua_data);     // 00748D0E (consumes lua_data)
        host.lua_reader_enter_00bd8e20(&reader, {0, dword_of("_landFort")});  // 00748D35, 00CFF3A0
        if (host.lua_reader_has_key_00bd5eb0(&reader, {0, dword_of("gazdi")})) {  // 00748D55, 00CFF398
            host.lua_reader_enter_00bd8e20(&reader, {0, dword_of("gazdi")});      // 00748D78
            host.reader_read_object_ref_007453c0(&reader, {0, dword_of("ID")},
                                                 &at<std::uint32_t>(fort, 0x738));  // 00748D9E, 00CE59B4
            // The image's slot [esp+8] holds a stale stack address when "section" is absent;
            // modelled as 0 (see the doc).
            std::int32_t section = 0;
            host.lua_reader_read_00bd6830(&reader, {0, dword_of("section")},
                                          {1, dword_of(&section)});  // 00748DD3, 00CFF390
            std::uint32_t element = 0;
            if (section != -1)
                element = dword_of(host.pointer_vector_at_0043f8e0(ptr(at<std::uint32_t>(fort, 0x738)) + 0x344,
                                                                   static_cast<std::uint32_t>(section)));  // 00748DEE
            at<std::uint32_t>(fort, 0x73C) = element;  // 00748DF7
            host.lua_reader_leave_00bd7a20(&reader);   // 00748DFD
        }
        Object director = ptr(at<std::uint32_t>(fort, 0x74C));
        host.reader_vfunc_4(&reader, {0, dword_of("gameUnit")});      // 00748E1C, 00CF8E4C
        host.weapon_director_visit_enables_00836510(director, &reader);  // 00748E25
        host.reader_vfunc_8(&reader);                                  // 00748E35
        host.lua_reader_leave_00bd7a20(&reader);                       // 00748E3B
        host.lua_reader_destruct_00441a20(&reader);                    // 00748E4C
    }
    host.repair_zone_slot_load_00809bc0(fort + 0x72C, fort, at<std::uint32_t>(fort, 0x30),
                                        at<std::uint32_t>(fort, 0xC0));  // 00748E63
}

// 0074D800. Evidence: the listing 0074D800-0074DA0C.
void land_vehicle_init_all_0074d800(Object vehicle, LandVehicleInitHost& host) {
    host.unit_instance_init_scene_bindings_00955420(vehicle);  // 0074D81E
    const std::uint32_t saved = at<std::uint32_t>(vehicle, 0xC0);
    if (saved != 0 && at<std::int32_t>(ptr(saved), 4) == 1 && at<std::uint32_t>(vehicle, 0x50) != 0 &&
        at<std::int32_t>(host.game_session_00e188a8(), 0x1FE4) != 2) {
        Object memory = host.operator_new_00bf681b(0x558);                              // 0074D84C
        Object gunnery = memory != nullptr ? host.gunnery_ai_construct_00864580(memory) : nullptr;  // 0074D866
        at<std::uint32_t>(vehicle, 0x6DC) = dword_of(gunnery);                        // 0074D86F
        host.gunnery_ai_vfunc_4(gunnery, vehicle + 0x310);  // 0074D88B: no null check in the image
    }
    Object klass = ptr(at<std::uint32_t>(vehicle, 0x538));
    if (at<std::uint32_t>(klass, 0x14C) == 0) return;  // 0074D89D (the NEG/SBB/TEST form of != 0)

    // 0074D8A9: identity 4x4 (1.0f is [00D7A24C]); only the translation row changes per anchor.
    float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const float* anchor = reinterpret_cast<const float*>(ptr(at<std::uint32_t>(klass, 0x140)));
    const float* end = anchor + at<std::int32_t>(klass, 0x144) * 3;
    for (; anchor != end; anchor += 3) {
        klass = ptr(at<std::uint32_t>(vehicle, 0x538));
        Object parent = ptr(at<std::uint32_t>(vehicle, 0x4A4));
        matrix[12] = anchor[0];
        matrix[13] = anchor[1];
        matrix[14] = anchor[2];
        // 0074D968: the definition [class+14Ch] passed by value with an added reference.
        RefCounted* definition = reinterpret_cast<RefCounted*>(ptr(at<std::uint32_t>(klass, 0x14C)));
        if (definition != nullptr) host.interlocked_increment(&definition->refs_04);  // 0074D984
        RefCounted* effect = nullptr;
        host.effect_create_matrix_008687c0(&effect, parent, definition, matrix, 1, 0, 0);  // 0074D990
        if (effect != nullptr) host.live_effect_append_0074d780(vehicle + 0x72C, &effect);  // 0074D9BA
        if (effect != nullptr) {
            if (host.interlocked_decrement(&effect->refs_04) == 0) host.release_slot0(effect);  // 0074D9D5
            effect = nullptr;
        }
    }
}

// 0073EBA0. Evidence: the listing and capstone over the bytes 0073EBA0-0073F1A1; every x87
// register-register form was read from its opcode bytes (docs/CC13_SEG41B.md).
bool mesh_decal_clip_triangle_to_box_0073eba0(DecalTriangle* tri, const void* matrix, float max_distance,
                                              float* out_normal, MeshDecalHost& host) {
    const Vec3& p0 = tri->position[0];
    const Vec3& p1 = tri->position[1];
    const Vec3& p2 = tri->position[2];
    const Vec3 e1{p1.x - p0.x, p1.y - p0.y, p1.z - p0.z};  // [esp+44h]
    const Vec3 e2{p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};  // [esp+2Ch]
    const Vec3 n = host.cross_004f9b30(e1, e2);            // 0073EBFE
    const float length = host.length_00419440(n);          // 0073EC07, FSTP to a float
    const float inverse = length > 0.0f ? static_cast<float>(1.0 / static_cast<double>(length)) : 0.0f;  // DE F1
    const Vec3 face{static_cast<float>(static_cast<double>(n.x) * inverse),
                    static_cast<float>(static_cast<double>(n.y) * inverse),
                    static_cast<float>(static_cast<double>(inverse) * n.z)};  // [esp+84h]
    tri->face_normal = face;                                                // 0073EC70

    Vec3 t0 = host.transform_point_004142e0(p0, matrix);  // 0073EC87
    Vec3 t1 = host.transform_point_004142e0(p1, matrix);  // 0073EC94
    Vec3 t2 = host.transform_point_004142e0(p2, matrix);  // 0073ECA1
    t0.z = 0.0f;
    t1.z = 0.0f;
    t2.z = 0.0f;
    // 0073ECAF..0073ED2D: COMISS rejections (unordered never rejects); 1.0f is [00D7A24C].
    if (0.0f > t0.x && 0.0f > t1.x && 0.0f > t2.x) return false;
    if (t0.x > 1.0f && t1.x > 1.0f && t2.x > 1.0f) return false;
    if (0.0f > t0.y && 0.0f > t1.y && 0.0f > t2.y) return false;
    if (t0.y > 1.0f && t1.y > 1.0f && t2.y > 1.0f) return false;

    // 0073ED33..0073EDBC: the 2-D edges; each z is computed as 0 - 0 (DC E0) or stored 0.
    Vec3 a{t1.x - t0.x, t1.y - t0.y, 0.0f};  // [esp+90h]
    Vec3 b{t2.x - t1.x, t2.y - t1.y, 0.0f};  // [esp+44h]
    Vec3 c{t0.x - t2.x, t0.y - t2.y, 0.0f};  // [esp+14h]
    const Vec3 up = host.normalize_00419510(host.cross_004f9b30(a, b));  // 0073EDC0, 0073EDCB
    if (0.0f >= up.z) return false;                                     // 0073EDD8: JNC

    // 0073EDDE..0073EE6C: centroid = (t0 + t1 + t2) / 3.0 (double 00D7A2B0); box centre
    // v = (0.5f, 0.5f, 0) ([00CE3800]).
    const float s_x = static_cast<float>(static_cast<double>(t1.x) + t0.x);
    const float s_y = static_cast<float>(static_cast<double>(t1.y) + t0.y);
    const float s_z = 0.0f;  // FLDZ, FADD ST0,ST1 (D8 C1)
    const float sum_x = static_cast<float>(static_cast<double>(s_x) + t2.x);
    const float sum_y = static_cast<float>(static_cast<double>(s_y) + t2.y);
    const float sum_z = static_cast<float>(0.0 + s_z);
    const Vec3 centroid{static_cast<float>(sum_x / 3.0), static_cast<float>(sum_y / 3.0),
                        static_cast<float>(sum_z / 3.0)};  // DC F9, D8 F1, FDIVR m64
    const band_0067_cc13::Vec3 box_centre{0.5f, 0.5f, 0.0f};

    // One edge: the in-plane normal m = normalize(cross(edge, up)); d = m.t (float), q = m.v
    // (float), dist = q - d; reject when dist > max_distance; count it when 0 >= dist.
    int inside = 0;
    auto edge_distance = [&](const Vec3& edge, const Vec3& t, double zero) -> float {
        const Vec3 m = host.normalize_00419510(host.cross_004f9b30(edge, up));
        const double z0 = static_cast<double>(m.z) * zero;
        const float d = static_cast<float>((static_cast<double>(m.y) * t.y + static_cast<double>(m.x) * t.x) + z0);
        const float q = static_cast<float>(z0 + (static_cast<double>(m.y) * 0.5 + static_cast<double>(m.x) * 0.5));
        return static_cast<float>(static_cast<double>(q) - d);
    };
    float dist = edge_distance(a, t0, 0.0);  // 0073EE70..0073EED4 (zero: double 00D7A258)
    if (dist > max_distance) return false;   // 0073EEE7
    if (0.0f >= dist) inside = 1;            // 0073EEF7
    dist = edge_distance(b, t1, 0.0);        // 0073EF09..0073EF61
    if (dist > max_distance) return false;
    if (0.0f >= dist) ++inside;
    dist = edge_distance(c, t2, 0.0);        // 0073EF94..0073EFEC (zero: FLDZ, kept on the stack)
    if (dist > max_distance) return false;   // 0073EFFF
    if (0.0f >= dist) ++inside;              // 0073F01C

    if (inside == 3) {  // 0073F01F: the box centre is inside all three edges
        const float negative_zero = bits_float(0x80000000u);  // [00D7A208]
        out_normal[0] = negative_zero - face.x;
        out_normal[1] = negative_zero - face.y;
        out_normal[2] = negative_zero - face.z;
    }

    // 0073F062..0073F18F: the three corner planes through t_i with normal normalize(t_i -
    // centroid); rz = 0 - centroid.z is computed once and reused.
    const float rz = static_cast<float>(0.0 - centroid.z);
    const Vec3* corners[3] = {&t0, &t1, &t2};
    for (const Vec3* t : corners) {
        const Vec3 r{t->x - centroid.x, t->y - centroid.y, rz};
        const Vec3 m = host.normalize_00419510(r);
        band_0067_cc13::Plane plane;
        band_0067_cc13::plane_from_normal_and_point_0073e780(plane, *t, m);
        if (band_0067_cc13::plane_signed_distance_0073e7c0(plane, box_centre) > max_distance) return false;
    }
    return true;
}

// 0073F1B0. Evidence: capstone over the bytes 0073F1B0-0073F6D5, x87 register forms checked
// against the opcode bytes (docs/CC13_SEG41B.md). Every dot product is (x + y) + z.
bool triangle_sphere_contact_0073f1b0(const Vec3* p0, const Vec3* p1, const Vec3* p2, const Vec3* centre,
                                      float radius, float* out_normal, MeshDecalHost& host) {
    const Vec3 e01{p1->x - p0->x, p1->y - p0->y, p1->z - p0->z};  // [esp+3Ch] (DE EA)
    const Vec3 e12{p2->x - p1->x, p2->y - p1->y, p2->z - p1->z};  // [esp+48h] (D8 E6/E4/E2)
    const Vec3 e20{p0->x - p2->x, p0->y - p2->y, p0->z - p2->z};  // [esp+54h]
    const float s_x = static_cast<float>(static_cast<double>(p1->x) + p0->x);  // DE C5
    const float s_y = static_cast<float>(static_cast<double>(p1->y) + p0->y);
    const float s_z = static_cast<float>(static_cast<double>(p1->z) + p0->z);
    const float sum_x = static_cast<float>(static_cast<double>(p2->x) + s_x);
    const float sum_y = static_cast<float>(static_cast<double>(p2->y) + s_y);
    const float sum_z = static_cast<float>(static_cast<double>(p2->z) + s_z);
    const Vec3 centroid{static_cast<float>(sum_x / 3.0), static_cast<float>(sum_y / 3.0),
                        static_cast<float>(sum_z / 3.0)};  // double 00D7A2B0; DC F9, D8 F1, FDIVR m32

    const Vec3 n = host.cross_004f9b30(e01, e12);    // 0073F2E2
    const float length = host.length_00419440(n);    // 0073F2EB
    const float inverse = length > 0.0f ? static_cast<float>(1.0 / static_cast<double>(length)) : 0.0f;
    const Vec3 face{static_cast<float>(static_cast<double>(n.x) * inverse),
                    static_cast<float>(static_cast<double>(n.y) * inverse),
                    static_cast<float>(static_cast<double>(inverse) * n.z)};  // [esp+24h]

    auto dot_float = [](const Vec3& a, const Vec3& b) {
        return static_cast<float>((static_cast<double>(a.x) * b.x + static_cast<double>(a.y) * b.y) +
                                  static_cast<double>(a.z) * b.z);
    };
    // 0073F33B..0073F3EF: |face plane distance of the centre| > radius rejects. The FLDZ left
    // on the x87 stack by the length test is the zero of the sign test; dist <= 0 or unordered
    // takes -0.0f - dist ([00D7A208]).
    float dist = static_cast<float>(static_cast<double>(dot_float(face, *centre)) - dot_float(*p0, face));
    const float magnitude = dist > 0.0f ? dist : bits_float(0x80000000u) - dist;
    if (magnitude > radius) return false;

    // 0073F3FD..0073F594: the three edge planes, normal normalize(cross(edge, face)) through the
    // edge's first vertex; one-sided (no absolute value).
    const Vec3* edge_points[3] = {p0, p1, p2};
    const Vec3* edges[3] = {&e01, &e12, &e20};
    for (int k = 0; k < 3; ++k) {
        const Vec3 m = host.normalize_00419510(host.cross_004f9b30(*edges[k], face));
        dist = static_cast<float>(static_cast<double>(dot_float(m, *centre)) - dot_float(*edge_points[k], m));
        if (dist > radius) return false;
    }
    out_normal[0] = face.x;  // 0073F5A7
    out_normal[1] = face.y;
    out_normal[2] = face.z;

    // 0073F5C1..0073F6C6: the three corner planes through p_i with normal normalize(p_i - centroid).
    for (const Vec3* p : edge_points) {
        const Vec3 r{p->x - centroid.x, p->y - centroid.y, p->z - centroid.z};
        const Vec3 m = host.normalize_00419510(r);
        band_0067_cc13::Plane plane;
        band_0067_cc13::plane_from_normal_and_point_0073e780(plane, *p, m);
        if (band_0067_cc13::plane_signed_distance_0073e7c0(plane, *centre) > radius) return false;
    }
    return true;
}

// 0073F6E0. Evidence: the listing 0073F6E0-0073F9AC.
void mesh_decal_collect_tree_triangles_0073f6e0(void* ctx, Object node, Object query, MeshDecalHost& host) {
    const bool leaf = at<std::uint32_t>(node, 0x1C) == 0 && at<std::uint32_t>(node, 0x20) == 0;  // BL
    // Per axis the child is visited unless q.min (+68h) > child.max and !(child.min > q.max
    // (+74h)); FCOMIP/JBE, so an unordered compare never skips.
    auto overlaps = [&](Object child) {
        for (std::uint32_t k = 0; k < 3; ++k) {
            const float q_min = at<float>(query, 0x68 + 4 * k);
            const float q_max = at<float>(query, 0x74 + 4 * k);
            if (q_min > at<float>(child, 0x0C + 4 * k) && !(at<float>(child, 4 * k) > q_max)) return false;
        }
        return true;
    };
    for (std::uint32_t link : {0x1Cu, 0x20u}) {
        Object child = ptr(at<std::uint32_t>(node, link));
        if (child != nullptr && overlaps(child)) mesh_decal_collect_tree_triangles_0073f6e0(ctx, child, query, host);
    }
    if (!leaf) return;
    for (std::uint32_t i = at<std::uint16_t>(node, 0x18); i < static_cast<std::uint32_t>(at<std::uint16_t>(node, 0x18)) + at<std::uint16_t>(node, 0x1A);
         ++i) {
        Object mesh = ptr(at<std::uint32_t>(query, 0));
        const std::uint32_t triangle = at<std::uint16_t>(ptr(at<std::uint32_t>(mesh, 0x18)), i * 2);
        Object indices = ptr(at<std::uint32_t>(query, 0x64)) + triangle * 6;
        Object stream = ptr(at<std::uint32_t>(query, 0x60));
        const std::uint32_t i1 = at<std::uint16_t>(indices, 2);
        const std::uint32_t i2 = at<std::uint16_t>(indices, 4);
        const Vec3 v0 = host.vertex_stream_read_position_004768d0(stream, at<std::uint16_t>(indices, 0));  // 0073F800
        const Vec3 v1 = host.vertex_stream_read_position_004768d0(ptr(at<std::uint32_t>(query, 0x60)), i1);  // 0073F80E
        const Vec3 v2 = host.vertex_stream_read_position_004768d0(ptr(at<std::uint32_t>(query, 0x60)), i2);  // 0073F81C
        if (at<std::uint32_t>(query, 8) >= 0x200) continue;  // 0073F829: the record array holds 200h
        auto* records = reinterpret_cast<DecalTriangle*>(ptr(at<std::uint32_t>(query, 4)));
        DecalTriangle* record = &records[at<std::uint32_t>(query, 8)];
        record->position[0] = v0;
        record->position[1] = v1;
        record->position[2] = v2;
        (void)ctx;  // 0073F8CB: ECX = ctx for 0073EBA0, which never reads it
        if (!mesh_decal_clip_triangle_to_box_0073eba0(record, query + 0x0C, at<float>(query, 0x4C),
                                                      reinterpret_cast<float*>(ptr(at<std::uint32_t>(query, 0x5C))),
                                                      host))
            continue;
        for (std::uint32_t k = 0; k < 3; ++k) {
            const Vec3 normal = host.vertex_stream_read_normal_0070fdb0(
                ptr(at<std::uint32_t>(query, 0x60)),
                at<std::uint16_t>(ptr(at<std::uint32_t>(query, 0x64)) + triangle * 6, 2 * k));  // 0073F8EC..
            records = reinterpret_cast<DecalTriangle*>(ptr(at<std::uint32_t>(query, 4)));
            records[at<std::uint32_t>(query, 8)].normal[k] = normal;  // FLD/FSTP float copies
        }
        ++at<std::uint32_t>(query, 8);  // 0073F97C
    }
}

// 006F3CC0. Evidence: capstone over the bytes 006F3CC0-006F3FBB; x87 forms from the opcode
// bytes (DE CA, DE C2, DC C8 at 006F3D62..006F3D6C; DC C9 at 006F3EE2, D8 C9 at 006F3EFE).
void command_building_debug_draw_id_006f3cc0(Object building, DebugIdColourStatics& statics,
                                             CommandBuildingDebugIdHost& host) {
    if (at<std::uint8_t>(host.debug_feature_owner_0051f460(), 0x2C) == 0) return;  // 006F3CCE
    Object camera = ptr(at<std::uint32_t>(host.game_session_00e188a8(), 0x19FC));
    if ((at<std::uint8_t>(camera, 0x5C) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(camera);  // 006F3CEB
    const float camera_x = at<float>(camera, 0x120);
    const float camera_y = at<float>(camera, 0x124);
    const float camera_z = at<float>(camera, 0x128);
    if (at<std::uint8_t>(building, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(building);  // 006F3D25
    const float dx = at<float>(building, 0xFC) - camera_x;
    const float dy = at<float>(building, 0x100) - camera_y;
    const float dz = at<float>(building, 0x104) - camera_z;
    const float distance_sq = static_cast<float>((static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) +
                                                 static_cast<double>(dz) * dz);
    if (!(bits_float(0x4B3AEB90u) > distance_sq)) return;  // 006F3D82: [00CFAE80] = 3500^2

    for (std::uint32_t it = at<std::uint32_t>(building, 0x794);
         it != at<std::uint32_t>(building, 0x794) + at<std::uint32_t>(building, 0x798) * 4; it += 4) {
        Object member = ptr(at<std::uint32_t>(ptr(it), 0));
        if (at<std::uint8_t>(member, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(member);  // 006F3DB2
        // [esp+14h]: the member's position with y replaced by [00CE3958] = 2.0f.
        const Vec3 position{at<float>(member, 0xFC), bits_float(0x40000000u), at<float>(member, 0x104)};
        const Vec3 forward = host.entity_flat_forward_006ac260(member);  // 006F3DF6
        if ((statics.sphere_guard_00e19a5c & 1) == 0) {
            statics.sphere_guard_00e19a5c |= 1;
            statics.sphere_rgba_00e19a4c[1] = 0.5f;  // [00CE3800]
            statics.sphere_rgba_00e19a4c[0] = 1.0f;  // [00D7A24C]
            statics.sphere_rgba_00e19a4c[2] = 0.0f;
            statics.sphere_rgba_00e19a4c[3] = 1.0f;
        }
        auto init_arrow_colour = [&] {
            if ((statics.arrow_guard_00e19a48 & 1) != 0) return;
            statics.arrow_guard_00e19a48 |= 1;
            statics.arrow_rgba_00e19a38[0] = 1.0f;
            statics.arrow_rgba_00e19a38[1] = 1.0f;
            statics.arrow_rgba_00e19a38[2] = 0.0f;
            statics.arrow_rgba_00e19a38[3] = 1.0f;
        };
        init_arrow_colour();  // 006F3E42
        host.debug_wire_sphere_00860be0(&position, statics.arrow_rgba_00e19a38, 5.0f /* [00CE3850] */,
                                        statics.sphere_rgba_00e19a4c, 1);  // 006F3E8D
        init_arrow_colour();  // 006F3E92
        // 006F3ECC: arrow = forward * 50.0 (double 00CE3938), each product stored as a float.
        const Vec3 scaled{static_cast<float>(static_cast<double>(forward.x) * 50.0),
                          static_cast<float>(static_cast<double>(forward.y) * 50.0),
                          static_cast<float>(50.0 * static_cast<double>(forward.z))};
        const Vec3 tip{scaled.x + position.x, scaled.y + position.y, scaled.z + position.z};
        const float rgba[4] = {statics.arrow_rgba_00e19a38[0], statics.arrow_rgba_00e19a38[1],
                               statics.arrow_rgba_00e19a38[2], statics.arrow_rgba_00e19a38[3]};
        host.debug_arrow_0085fff0(&position, &tip, rgba);  // 006F3F44
    }
    if (at<std::uint8_t>(building, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(building);  // 006F3F6E
    float screen[3];
    if (host.camera_project_world_to_screen_0043a660(&at<float>(building, 0xFC), screen, 2, 1) != 7) return;  // 006F3F7D
    host.diagnostic_printf_004c7070(host.game_session_00e188a8(), screen[0], screen[1], "ID: %d",
                                    at<std::uint16_t>(building, 0x174));  // 006F3FAC, 00CFAE78
}

}  // namespace bsp::cc13_seg41b
