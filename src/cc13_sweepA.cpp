// Packet cc13-sweepA: segments 0..14 leftovers (00401010..00527C80). See
// include/bsp/cc13_sweepA.hpp and docs/CC13_SWEEPA.md. Each body carries its
// address, the original ABI and every simplification next to it.

#include "bsp/cc13_sweepA.hpp"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <initializer_list>

namespace bsp::cc13_sweepA {

// ============================ segment 0 ====================================

// 00412D90, __stdcall(float), RET 4, result in ST0: clears the sign bit of the
// argument's dword (AND 7FFFFFFFh) and loads it. Unreferenced in the image.
float abs_float_00412d90(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    bits &= 0x7fffffffu;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// 00412E80, __thiscall(ECX = {void** data +0, int count +4}), RET: swaps the
// dword slots from both ends towards the middle (signed compare, the data
// pointer reloaded for each store). Unreferenced in the image.
void pointer_array_reverse_00412e80(Raw array) {
    std::int32_t high = array.at<std::int32_t>(4) - 1;
    std::int32_t low = 0;
    if (high <= 0) return;
    do {
        auto** data = static_cast<void**>(array.ptr(0));
        void* upper = data[high];
        void* lower = data[low];
        data[low] = upper;
        static_cast<void**>(array.ptr(0))[high] = lower;
        ++low;
        --high;
    } while (high > low);
}

// 00412F60, __fastcall(ECX = int* value, EDX = int* low, stack int* high), RET 4:
// returns *low when *value < *low, else *high when *value > *high, else *value
// (signed). Unreferenced in the image.
std::int32_t int_clamp_00412f60(const std::int32_t* value, const std::int32_t* low,
                                const std::int32_t* high) {
    const std::int32_t v = *value;
    if (v < *low) return *low;
    if (v > *high) return *high;
    return v;
}

// 00412F80, __cdecl(char* dst, const char* format, ...): _vsprintf_s (00BF7289)
// with count 64h; EAX passes its result through. Caller 00423640.
int format_string_100_00412f80(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vsprintf_s(dst, 0x64, format, args);
    va_end(args);
    return result;
}

// 00412FA0, __fastcall(ECX = int* a, EDX = int* b): EAX = *a when *a < *b, else
// *b (signed). Unreferenced in the image.
std::int32_t int_min_00412fa0(const std::int32_t* a, const std::int32_t* b) {
    const std::int32_t va = *a;
    const std::int32_t vb = *b;
    return va < vb ? va : vb;
}

// ============================ segment 2 ====================================

// 0042AA00, __thiscall(ECX = camera, float rho, float theta, char snap), RET 0Ch.
// Target angles +384h/+398h always; snap also sets the current angles
// +38Ch/+3A0h and both blend rates +394h/+3A8h to 1.0f (00D7A24C), otherwise
// the rates are 5.0f (00CE3850). The UCOMISS/LAHF pair at 0042AA4E sets flags
// nobody reads and has no effect.
void camera_set_angles_0042aa00(Raw camera, float rho, float theta, char snap) {
    camera.at<float>(0x384) = rho;
    camera.at<float>(0x398) = theta;
    if (snap != 0) {
        camera.at<float>(0x38c) = rho;
        camera.at<float>(0x3a0) = theta;
        camera.at<float>(0x394) = 1.0f;
        camera.at<float>(0x3a8) = 1.0f;
        return;
    }
    camera.at<float>(0x394) = 5.0f;
    camera.at<float>(0x3a8) = 5.0f;
}

// 0042C450, __thiscall(ECX = mover, float low), RET 4. +1C0h is raised to
// `low` when low > it (FCOMIP, JBE skips on unordered), otherwise lowered to
// the ceiling +1C4h when it exceeds that; NaN keeps the current value.
void camera_clamp_zoom_0042c450(Raw mover, float low) {
    const float current = mover.at<float>(0x1c0);
    if (low > current) {
        mover.at<float>(0x1c0) = low;
        return;
    }
    const float high = mover.at<float>(0x1c4);
    mover.at<float>(0x1c0) = current > high ? high : current;
}

// 0042C4C0, __thiscall(ECX = mover, six floats), RET 18h: stores them to
// +1D0h..+1E4h in argument order.
void camera_mover_set_shake_0042c4c0(Raw mover, float a0, float a1, float a2, float f0, float f1,
                                     float f2) {
    mover.at<float>(0x1d0) = a0;
    mover.at<float>(0x1d4) = a1;
    mover.at<float>(0x1d8) = a2;
    mover.at<float>(0x1dc) = f0;
    mover.at<float>(0x1e0) = f1;
    mover.at<float>(0x1e4) = f2;
}

// 0042CD00, __thiscall(ECX = camera, float* out), RET 4, EAX = out. With
// unit = [[camera+388h]+538h]: out = {0.0f, unit+16Ch + camera+3B8h,
// -0.0f (00D7A208) - unit+168h}. The middle sum is an x87 FADD rounded once to
// float by FSTP; the C++ float add rounds the exact sum once, so they differ
// only where the extended sum was itself inexact (exponents over 40 apart).
float* camera_eye_offset_0042cd00(Raw camera, float* out) {
    const Raw unit{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(camera.ptr(0x388))}.ptr(0x538))};
    out[0] = 0.0f;
    out[1] = unit.at<float>(0x16c) + camera.at<float>(0x3b8);
    out[2] = -0.0f - unit.at<float>(0x168);
    return out;
}

// 0042E3D0, __thiscall(ECX = mover, const float* point), RET 4, AL. Calls
// 0042E180, then for the six planes {a, b, c, d} at +1F8h (stride 10h) returns
// 0 as soon as a*x + b*y + c*z (x87, D8-form FMULs checked from the bytes,
// rounded to float by FSTP at 0042E426) is above d (FCOMIP/JA: NaN passes),
// else 1. The extended-precision sum is modelled in double: identical except
// when the double-rounded sum differs from the extended one in the last float bit.
// Caller 004B48BF (inside 004B4850), which Ghidra's call graph misses.
bool camera_mover_frustum_contains_point_0042e3d0(Raw mover, const float* point, SweepAHost& host) {
    host.camera_mover_build_frustum_planes_0042e180(mover);
    const double x = point[0];
    const double y = point[1];
    const double z = point[2];
    for (int plane = 0; plane < 6; ++plane) {
        const float* row = &mover.at<float>(0x1f8 + 0x10 * plane);
        const float dot = static_cast<float>(row[0] * x + row[1] * y + row[2] * z);
        if (dot > row[3]) return false;
    }
    return true;
}

// 004313D0, __thiscall(ECX = entity, const NativeString* name, unused), RET 8;
// vtable slot +0Ch of several entity classes (00CE3B34 and five more). When the
// name is not the entity's own string at +15Ch: 0041DD40(string, name.length,
// 1), then, for a non-zero length, memcpy(string.data, name.data, string.length).
void entity_set_name_004313d0(Raw entity, Raw name, std::uint32_t /*unused*/, SweepAHost& host) {
    const Raw own{entity.field(0x15c)};
    if (own.p == name.p) return;
    host.native_string_resize_0041dd40(own, name.at<std::uint32_t>(0), 1);
    if (name.at<std::uint32_t>(0) != 0) {
        std::memcpy(own.ptr(4), name.ptr(4), own.at<std::uint32_t>(0));
    }
}

// 00432380, __thiscall(ECX = record), EAX = record. Zeroes +4,+8,+0Ch, +14h..+1Ch,
// +24h..+2Ch and +34h..+3Ch (the x, y, z of four 10h-byte vectors); +0, +10h,
// +20h, +30h are left alone. The SEH frame registers no unwind action. Not
// referenced: the one dword 00432380 in the image (.text 00A81689) is the
// immediate 4400000 of sound_system_owner's frequency limit.
Raw four_vector_record_construct_00432380(Raw record) {
    for (std::size_t base = 0; base < 0x40; base += 0x10) {
        record.at<std::uint32_t>(base + 4) = 0;
        record.at<std::uint32_t>(base + 8) = 0;
        record.at<std::uint32_t>(base + 0xc) = 0;
    }
    return record;
}

// 00435410, __thiscall(ECX = mover, float dt), RET 4; vtable slot (00CE3C04):
// forwards to 004329D0(mover, dt).
void camera_mover_step_base_00435410(Raw mover, float dt, SweepAHost& host) {
    host.camera_mover_apply_shake_004329d0(mover, dt);
}

// 00435960, __thiscall(ECX = {3-byte rows* +0, int count +4, int capacity +8},
// int capacity), RET 4. Capacity is at least 1; a larger one allocates
// capacity*3 bytes (operator new 00BF55BE), copies each row (word + byte) to a
// non-null destination, frees the old block (_free 00BF6989, also when null)
// and stores data and capacity.
void party_rows_reserve_00435960(Raw rows, std::int32_t capacity, SweepAHost& host) {
    if (capacity < 1) capacity = 1;
    if (rows.at<std::int32_t>(8) >= capacity) return;
    auto* fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity * 3)));
    std::uint8_t* dst = fresh;
    for (std::int32_t i = 0; i < rows.at<std::int32_t>(4); ++i, dst += 3) {
        if (dst == nullptr) continue;
        const std::uint8_t* src = static_cast<std::uint8_t*>(rows.ptr(0)) + 3 * i;
        std::memcpy(dst, src, 2);
        dst[2] = src[2];
    }
    host.free_00bf6989(rows.ptr(0));
    rows.ptr(0) = fresh;
    rows.at<std::int32_t>(8) = capacity;
}

// 004359E0, __thiscall(ECX = rows, int count), RET 4. Reserves through
// 00435960 when count exceeds the capacity, zero-constructs rows count..new-1
// (a null row address is skipped), then the count is stepped down one at a
// time while above the new count and finally set to it; rows have no destructor.
void party_rows_resize_004359e0(Raw rows, std::int32_t count, SweepAHost& host) {
    if (count > rows.at<std::int32_t>(8)) party_rows_reserve_00435960(rows, count, host);
    const std::int32_t old = rows.at<std::int32_t>(4);
    for (std::int32_t i = old; i < count; ++i) {
        std::uint8_t* row = static_cast<std::uint8_t*>(rows.ptr(0)) + 3 * i;
        if (row == nullptr) continue;
        row[0] = 0;
        row[1] = 0;
        row[2] = 0;
    }
    rows.at<std::int32_t>(4) = count;
}

// 00435F40, __thiscall(ECX = owner), RET; reached by the tail JMP at 00436487
// in 00436460. When 00A3E540(ECX = [00F8ABE8]) holds and [[0109CECC]+8] != 1:
// globals = 00B67980([00E188A8]+1A0Ch); table = 00B67800(globals, "BSP_Chk_Save"
// 00CE4124); globals destroyed; a text writer {vptr 00CE4104, 0} opens section
// 00BD8C80({0, "BSP_Chk_Save"}); when the table is bound, key and value are
// nil-constructed and every pair from 00B67080/00B67190 is written by
// 00435D30(owner, writer, key, value) until 00B66420 reports the end; then
// value and key are destroyed, 00BD6CC0 closes the section, 006ADB50 runs with
// ECX = 00BD53C0, the writer's vptr becomes the base 00CE3784 and the table is
// destroyed. The unwind actions for a throwing callee are not modelled.
void lua_global_table_write_archive_00435f40(Raw owner, SweepAHost& host) {
    if (!host.storage_gate_00a3e540(host.global_pointer(0x00f8abe8))) return;
    const Raw manager{static_cast<std::uint8_t*>(host.global_pointer(0x0109cecc))};
    if (manager.at<std::int32_t>(8) == 1) return;

    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    static constexpr char kName[] = "BSP_Chk_Save";  // 00CE4124
    LuaObjectStorage globals_out;
    LuaObjectStorage table;
    LuaObjectStorage* globals = host.lua_state_owner_get_globals_00b67980(game.field(0x1a0c), &globals_out);
    host.lua_object_get_by_name_00b67800(globals, &table, kName);
    host.lua_object_destruct_00b67700(&globals_out);

    alignas(4) std::uint8_t writer_bytes[8];
    const Raw writer{writer_bytes};
    writer.at<std::uint32_t>(0) = 0x00ce4104;
    writer.at<std::uint32_t>(4) = 0;
    host.archive_text_writer_begin_section_00bd8c80(writer, 0, kName);
    if (host.lua_object_is_bound_00b65f70(&table)) {
        LuaObjectStorage key;
        LuaObjectStorage value;
        host.lua_object_construct_00b65f50(&key);
        host.lua_object_construct_00b65f50(&value);
        host.lua_object_iterate_first_00b67080(&table, &key, &value);
        while (!host.lua_object_is_unbound_00b66420(&table, &key)) {
            host.lua_slot_write_archive_00435d30(owner, writer, &key, &value);
            host.lua_object_iterate_next_00b67190(&table, &key, &value);
        }
        host.lua_object_destruct_00b67700(&value);
        host.lua_object_destruct_00b67700(&key);
    }
    host.archive_text_writer_end_section_00bd6cc0(writer);
    host.storage_operation_run_continuation_006adb50(0x00bd53c0);
    writer.at<std::uint32_t>(0) = 0x00ce3784;
    host.lua_object_destruct_00b67700(&table);
}

// 00438A00, __thiscall(ECX = profile manager, const LuaObject* source), RET 4.
// EDI = [0109CECC] is read first; 00B67690 assigns the source to +4, the
// pending byte +18h becomes 1; when 00A3E540([00F8ABE8]) holds, 00BD3E10(EDI,
// [00E188A8]+684h, 2) requests the write and 006ADB50 runs with the
// continuation 004389F0; otherwise 00438020 saves at once on the 00425C20
// singleton.
void profile_manager_request_save_checkpoint_00438a00(Raw manager, const void* source,
                                                     SweepAHost& host) {
    void* const storage = host.global_pointer(0x0109cecc);
    host.lua_object_assign_00b67690(manager.field(4), source);
    manager.at<std::uint8_t>(0x18) = 1;
    if (host.storage_gate_00a3e540(host.global_pointer(0x00f8abe8))) {
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
        host.storage_request_single_write_00bd3e10(storage, game.field(0x684), 2);
        host.storage_operation_run_continuation_006adb50(0x004389f0);
        return;
    }
    host.profile_manager_save_checkpoint_00438020(host.profile_manager_get_singleton_00425c20());
}

// 00438F90, __thiscall(ECX = key, const char* text), RET 4, EAX = key:
// key+0 = operator new(strlen(text) + 1) (00BF55BE) holding a copy of the text
// (copied up to and including the terminator, no null check of the block),
// key+4 byte = 1 (owned). Unreferenced in the image.
Raw cstring_key_construct_owned_00438f90(Raw key, const char* text, SweepAHost& host) {
    const std::size_t length = std::strlen(text);
    auto* block = static_cast<char*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(length + 1)));
    key.ptr(0) = block;
    std::memcpy(block, text, length + 1);
    key.at<std::uint8_t>(4) = 1;
    return key;
}

// 00439820, __fastcall(ECX = float3* out, EDX = const float3* point, matrix,
// float* inverse_w), RET 8, EAX = out. (x, y, z, 1.0f) is transformed by
// 00B62D10; 1/w is an x87 FDIVRP rounded to float in the matrix argument's
// slot (0043989C), stored through inverse_w when that is non-null, and the
// three outputs are float products with it (D8-form FMUL; the scan reports no
// DC form here). Earlier notes called the last argument a flag; it is a pointer.
// The quotient is modelled as double then float: identical except when
// double rounding differs from the extended-then-float rounding.
float* vector3_transform_project_point_00439820(float* out, const float* point, const void* matrix,
                                               float* inverse_w, SweepAHost& host) {
    const float source[4] = {point[0], point[1], point[2], 1.0f};
    float clip[4];
    const float* r = host.vector4_transform_00b62d10(source, clip, matrix);
    const float x = r[0];
    const float y = r[1];
    const float z = r[2];
    const float inv = static_cast<float>(1.0 / static_cast<double>(r[3]));
    if (inverse_w != nullptr) *inverse_w = inv;
    out[0] = x * inv;
    out[1] = y * inv;
    out[2] = inv * z;
    return out;
}

// 00439A00, __thiscall(ECX = float2* out), EAX = out. angle =
// 00BD2F10(stream 1, 0.0f, 6.2831855f (00CE3D9C)) rounded to float; out =
// (cos angle, sin angle) by FCOS/FSIN, each rounded to float. Unreferenced in
// the image. Modelled with the double cos/sin, which can differ from the x87
// extended results in the last float bit.
float* vector2_set_random_unit_direction_00439a00(float* out, SweepAHost& host) {
    const float angle = host.random_uniform_00bd2f10(1, 0.0f, 6.2831855f);
    out[0] = static_cast<float>(std::cos(static_cast<double>(angle)));
    out[1] = static_cast<float>(std::sin(static_cast<double>(angle)));
    return out;
}

// 00439AC0, __cdecl(format, ...), EAX = 00E177E8: _vsprintf_s (00BF7289) into
// the 400h-byte static buffer, whose address is returned whatever the result.
char* string_format_static_00439ac0(SweepAHost& host, const char* format, ...) {
    char* const buffer = host.format_buffer_00e177e8();
    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, 0x400, format, args);
    va_end(args);
    return buffer;
}

namespace {

// The shared tail of 00439F50's two transforms: (x, y, z, 1.0f) through
// 00B62D10 with the inverse view-projection, then x, y, z times the float
// 1/w (FLD1/FDIVRP rounded by FSTP), as in 00439820.
void unproject_one(void* camera, float x, float y, float z, float* out, SweepAHost& host) {
    const void* matrix = host.camera_get_inverse_view_projection_00b70510(camera);
    const float source[4] = {x, y, z, 1.0f};
    float clip[4];
    const float* r = host.vector4_transform_00b62d10(source, clip, matrix);
    const float px = r[0];
    const float py = r[1];
    const float pz = r[2];
    const float inv = static_cast<float>(1.0 / static_cast<double>(r[3]));
    out[0] = px * inv;
    out[1] = py * inv;
    out[2] = inv * pz;
}

}  // namespace

// 00439F50, __fastcall(ECX = camera, EDX = float3* out_near, float sx, float sy,
// float3* out_far), RET 0Ch, AL. 00B6FDE0(camera) null -> 0. Otherwise with the
// viewport's int width +10h and height +14h: nx = (sx - W*0.5) / (W*0.5) and
// ny = (H*0.5 - sy) / (H*0.5) (0.5 is the double 00D7A280; the DC-form FMUL at
// 00439F77 keeps 0.5 in ST0 for the FIMUL), each rounded to float. Far =
// unproject(nx, ny, 1.0f) is written through out_far first, then near =
// unproject(nx, ny, 0.01f (00D7A238)) through out_near; 00B70510 is called
// before each transform; returns 1. The extended quotients are modelled in
// double then float (double rounding can differ in the last float bit).
bool camera_unproject_screen_ray_00439f50(void* camera, float* out_near, float screen_x,
                                          float screen_y, float* out_far, SweepAHost& host) {
    const Raw viewport{static_cast<std::uint8_t*>(host.camera_get_viewport_00b6fde0(camera))};
    if (viewport.p == nullptr) return false;
    const double half_width = static_cast<double>(viewport.at<std::int32_t>(0x10)) * 0.5;
    const float nx = static_cast<float>((static_cast<double>(screen_x) - half_width) / half_width);
    const double half_height = 0.5 * static_cast<double>(viewport.at<std::int32_t>(0x14));
    const float ny = static_cast<float>((half_height - static_cast<double>(screen_y)) / half_height);
    float far_point[3];
    unproject_one(camera, nx, ny, 1.0f, far_point, host);
    out_far[0] = far_point[0];
    out_far[1] = far_point[1];
    out_far[2] = far_point[2];
    float near_point[3];
    unproject_one(camera, nx, ny, 0.01f, near_point, host);
    out_near[0] = near_point[0];
    out_near[1] = near_point[1];
    out_near[2] = near_point[2];
    return true;
}

// 0043A290, __fastcall(ECX = camera, EDX = float3* out, float sx, float sy,
// float plane_y, float3* ray), RET 10h, AL; caller 006438E8 (HUD markers).
// 00439F50(camera, &near, sx, sy, ray) false -> 0. ray becomes the float
// direction far - near (each component an x87 FSUBP rounded by FSTP). dy == 0
// (FUCOMIP, JP: unordered continues) -> 0. t = (plane_y - near.y) / dy rounded
// to float; 0 > t -> 0 (FCOMIP/JBE: NaN continues). out = near + (d * t),
// each product and sum rounded to float; returns 1. The stack walk was done
// on the bytes (DE-form FSUBP/FMULP/FADDP targets from x87_dc_form_scan).
bool camera_pick_world_point_on_horizontal_plane_0043a290(void* camera, float* out, float screen_x,
                                                         float screen_y, float plane_y,
                                                         float* ray, SweepAHost& host) {
    float near_point[3];
    if (!camera_unproject_screen_ray_00439f50(camera, near_point, screen_x, screen_y, ray, host)) {
        return false;
    }
    const float dx = ray[0] - near_point[0];
    ray[0] = dx;
    const float dy = ray[1] - near_point[1];
    ray[1] = dy;
    const float dz = ray[2] - near_point[2];
    ray[2] = dz;
    if (dy == 0.0f) return false;
    const float t = static_cast<float>((static_cast<double>(plane_y) - near_point[1]) / dy);
    if (0.0f > t) return false;
    const float ox = dx * t;
    const float oy = dy * t;
    const float oz = t * dz;
    out[0] = near_point[0] + ox;
    out[1] = near_point[1] + oy;
    out[2] = near_point[2] + oz;
    return true;
}

// 0043A870, __fastcall(ECX = const float3* world, EDX = float2* out, int mode,
// char clamp_depth), RET 8: 0043A660 into a float3 temporary, then x and y are
// copied out; EAX is 0043A660's result passed through (callers compare it to 7).
std::int32_t camera_project_world_to_screen_xy_0043a870(const float* world, float* out_xy,
                                                       std::int32_t mode, char clamp_depth,
                                                       SweepAHost& host) {
    float screen[3];
    const std::int32_t code = host.camera_project_world_to_screen_0043a660(world, screen, mode, clamp_depth);
    out_xy[0] = screen[0];
    out_xy[1] = screen[1];
    return code;
}

// 0043B9C0, __fastcall(ECX = const float3* point), AL; caller 0049BB80 in
// 0049B1F0, the splash path (docs/CC13_LEFT8C.md). A stack hit record (54h bytes) is reset by
// 00470470; water = 0078CF20([[00E188A8]+19F0h], x, z) plus the double
// 00D7A3A0 (0.1f widened), rounded to float. 0098ADD0 on the 0042E630 index
// queries from (x, water + 0.1, z) to (x, 100.0f (00CE3D08), z) with no
// excluded entity, the record, and filter 0; a hit whose entity (record+0)
// answers vtable +5Ch(6) true gives 1. 004704B0 destroys the record. The
// extended FADD is modelled in double then float. The unwind action is not
// modelled.
bool surface_probe_kind6_above_water_0043b9c0(const float* point, SweepAHost& host) {
    alignas(4) std::uint8_t record_bytes[0x54];
    const Raw record{record_bytes};
    host.hit_record_reset_00470470(record);
    const float x = point[0];
    const float y = point[1];
    const float z = point[2];
    static_cast<void>(y);
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    const float water = host.game_world_sample_water_height_0078cf20(game.ptr(0x19f0), x, z);
    const float to[3] = {x, 100.0f, z};
    const float from[3] = {x, static_cast<float>(static_cast<double>(water) + 0.10000000149011612), z};
    bool result = false;
    void* const index = host.spatial_index_get_singleton_0042e630();
    if (host.spatial_index_query_segment_0098add0(index, from, to, nullptr, record, 0)) {
        if (host.vcall_entity_is_kind_5c(record.ptr(0), 6)) result = true;
    }
    host.hit_record_destroy_004704b0(record);
    return result;
}

namespace {

// The pooled-string release the 0043xxxx helpers inline: when data (+4) is
// set, 00BD1510(00419CC0(), data, length + 1, 1).
void release_pooled_data(Raw string, SweepAHost& host) {
    void* const data = string.ptr(4);
    if (data == nullptr) return;
    const std::uint32_t size = string.at<std::uint32_t>(0) + 1;
    host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                  size, 1);
}

constexpr std::uint32_t kNodePool = 0x00e175b0;  // ECX of every 0043B0A0 call here

}  // namespace

// 0043BFC0, __thiscall(ECX = pooled string {length +0, data +4}, flags), RET 4,
// EAX = string: releases the data, and with flags bit 0 returns the header to
// the 00E175B0 node pool (0043B0A0). Unreferenced in the image.
Raw pooled_string_scalar_delete_0043bfc0(Raw string, std::uint32_t flags, SweepAHost& host) {
    release_pooled_data(string, host);
    if ((flags & 1) != 0) host.node_pool_return_0043b0a0(kNodePool, string.p);
    return string;
}

// 0043CFA0, __thiscall(ECX = PooledString** slot): a non-null string has its
// data released and its header returned to the 00E175B0 pool, then the slot is
// cleared (only in that case). Unreferenced in the image.
void pooled_string_ptr_delete_reset_0043cfa0(void** slot, SweepAHost& host) {
    const Raw string{static_cast<std::uint8_t*>(*slot)};
    if (string.p == nullptr) return;
    release_pooled_data(string, host);
    host.node_pool_return_0043b0a0(kNodePool, string.p);
    *slot = nullptr;
}

// 0043D2D0, __thiscall(ECX = map, unused; node), RET 4. Node: +0/+4 the pooled
// key, +8 owned value, +0Ch next. A non-null value is deleted through vtable
// slot 0 with flags 1 and +8 cleared; this happens before the image tests the
// node for null. A non-null node then has its key data released and returns to
// the 00E175B0 pool.
void property_map_node_destroy_0043d2d0(Raw /*map*/, Raw node, SweepAHost& host) {
    void* const value = node.ptr(8);
    if (value != nullptr) {
        host.vcall_scalar_deleting_destructor_slot0(value, 1);
        node.ptr(8) = nullptr;
    }
    if (node.p == nullptr) return;
    release_pooled_data(node, host);
    host.node_pool_return_0043b0a0(kNodePool, node.p);
}

// 0043D4D0, __thiscall(ECX = map {+4 count, +8 bucket heads}, const PooledString*
// key), RET 4, AL. 0043B8B0 finds the node and the bucket (written into the key
// argument's stack slot). Not found -> 0. The node is unlinked from the bucket
// head or from its predecessor (walked by +0Ch, no end check, as in the image),
// destroyed by 0043D2D0 and the count decremented; -> 1.
bool property_map_erase_0043d4d0(Raw map, const void* key, SweepAHost& host) {
    std::uint32_t bucket = 0;
    const Raw node{static_cast<std::uint8_t*>(host.property_map_find_0043b8b0(map, key, &bucket))};
    if (node.p == nullptr) return false;
    void*& head = map.ptr(8 + 4 * bucket);
    if (head == node.p) {
        head = node.ptr(0xc);
    } else {
        Raw previous{static_cast<std::uint8_t*>(head)};
        while (previous.ptr(0xc) != node.p) previous = Raw{static_cast<std::uint8_t*>(previous.ptr(0xc))};
        previous.ptr(0xc) = node.ptr(0xc);
    }
    property_map_node_destroy_0043d2d0(map, node, host);
    map.at<std::int32_t>(4) -= 1;
    return true;
}

// 0043D870, __thiscall(ECX = owner, const char* name), RET 4, AL: a stack
// pooled string is built by 0041E870, 0043D4D0(owner+4, &key) erases, the key's
// data is released (no header return: it lives on the stack) and the erase
// result is returned. The unwind action is not modelled.
bool property_map_erase_by_name_0043d870(Raw owner, const char* name, SweepAHost& host) {
    alignas(4) std::uint8_t key_bytes[8];
    const Raw key{key_bytes};
    host.native_string_construct_0041e870(key, name);
    const bool erased = property_map_erase_0043d4d0(Raw{owner.field(4)}, key.p, host);
    release_pooled_data(key, host);
    return erased;
}

namespace {

// The body shared by 0043DF40/0043DF90/0043DFF0/0043E060: vector +4/+8/+0Ch
// zeroed, each argument pushed by 0043DCA0 from the LAST to the first (the
// image reads the highest stack slot first), then the byte +18h = 0.
Raw observer_set_construct(Raw set, const std::uint32_t* args, int count, SweepAHost& host) {
    set.at<std::uint32_t>(4) = 0;
    set.at<std::uint32_t>(8) = 0;
    set.at<std::uint32_t>(0xc) = 0;
    for (int i = count - 1; i >= 0; --i) host.vector_push_back_0043dca0(set, &args[i]);
    set.at<std::uint8_t>(0x18) = 0;
    return set;
}

}  // namespace

// 0043DF40 / 0043DF90 / 0043DFF0 / 0043E060, __thiscall(ECX = set, one to four
// dwords), RET 4/8/0Ch/10h, EAX = set. The pushes take the address of the
// caller's argument slot. 0043DF40 and 0043E060 are unreferenced in the image.
// The unwind actions for a throwing push are not modelled.
Raw observer_set_construct_0043df40(Raw set, std::uint32_t a, SweepAHost& host) {
    const std::uint32_t args[1] = {a};
    return observer_set_construct(set, args, 1, host);
}
Raw observer_set_construct_0043df90(Raw set, std::uint32_t a, std::uint32_t b, SweepAHost& host) {
    const std::uint32_t args[2] = {a, b};
    return observer_set_construct(set, args, 2, host);
}
Raw observer_set_construct_0043dff0(Raw set, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    SweepAHost& host) {
    const std::uint32_t args[3] = {a, b, c};
    return observer_set_construct(set, args, 3, host);
}
Raw observer_set_construct_0043e060(Raw set, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                                    std::uint32_t d, SweepAHost& host) {
    const std::uint32_t args[4] = {a, b, c, d};
    return observer_set_construct(set, args, 4, host);
}

// ============================ segment 3 ====================================

// 0043FF40, __thiscall(ECX = gun, unused), RET 4; vtable slot (00CFDD34) and the
// call at 0072B33F in 0072B2D0. With p = [gun+360h]: p non-null, obj =
// [[p+160h]+0Ch], q = [obj+130h] non-null and [[q+2Ch]+10h] != 0: the float
// clock at 00F876A4 is widened to double, length = 00B6DC90(obj, 0), and
// vtable +24h of obj gets (0, (float)fmod(clock, length), 1) (_CIfmod 00BF857A
// with the clock in ST1, the length in ST0). The double fmod is exact like the
// x87 one; the length is taken as the float the host returns.
void gun_animation_sync_to_clock_0043ff40(Raw gun, std::uint32_t /*unused*/, SweepAHost& host) {
    const Raw p{static_cast<std::uint8_t*>(gun.ptr(0x360))};
    if (p.p == nullptr) return;
    const Raw obj{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(p.ptr(0x160))}.ptr(0xc))};
    const Raw q{static_cast<std::uint8_t*>(obj.ptr(0x130))};
    if (q.p == nullptr) return;
    if (Raw{static_cast<std::uint8_t*>(q.ptr(0x2c))}.at<std::uint32_t>(0x10) == 0) return;
    const double clock = *static_cast<const float*>(host.global_address(0x00f876a4));
    const float length = host.animation_length_00b6dc90(obj.p, 0);
    const float phase = static_cast<float>(std::fmod(clock, static_cast<double>(length)));
    host.vcall_animation_set_time_24(obj.p, 0, phase, 1);
}

// 004407A0, __thiscall(ECX = class), ST0 = float +4Ch; held by 26 vtable slots
// (00CE44A4 .. 00D1AA7C), so the name is only a guess for one of its users.
float vehicle_class_underwater_armour_default_004407a0(Raw vehicle_class) {
    return vehicle_class.at<float>(0x4c);
}

// 00440CA0 / 004411F0 (identical bodies), __thiscall(ECX = slot array):
// 00440180(array, 0), then _free(data). Unreferenced in the image.
void device_class_slots_destroy_00440ca0(Raw slots, SweepAHost& host) {
    host.device_class_registry_resize_slots_00440180(slots, 0);
    host.free_00bf6989(slots.ptr(0));
}
void device_class_slots_destroy_004411f0(Raw slots, SweepAHost& host) {
    host.device_class_registry_resize_slots_00440180(slots, 0);
    host.free_00bf6989(slots.ptr(0));
}

// 00442170, no arguments: 00440180(00441780() + 4, 0). Caller 004D2B2A in
// 004D2B10 (world container teardown).
void device_class_registry_clear_00442170(SweepAHost& host) {
    auto* registry = static_cast<std::uint8_t*>(host.device_class_registry_get_singleton_00441780());
    host.device_class_registry_resize_slots_00440180(Raw{registry + 4}, 0);
}

// 004427A0, __thiscall(ECX = 48h-byte gun record), EAX = record. vptr 00CE4530;
// +0Ch..+18h, +34h, +38h zeroed; the std::map at +3Ch gets a head node from
// 00440380 marked isnil (+11h = 1) with its three links on itself, size +44h = 0.
// +4, +8, +1Ch..+30h are not written. The unwind actions are not modelled.
Raw gun_record_construct_004427a0(Raw record, SweepAHost& host) {
    record.at<std::uint32_t>(0) = 0x00ce4530;
    record.at<std::uint32_t>(0xc) = 0;
    record.at<std::uint32_t>(0x10) = 0;
    record.at<std::uint32_t>(0x14) = 0;
    record.at<std::uint32_t>(0x18) = 0;
    record.at<std::uint32_t>(0x34) = 0;
    record.at<std::uint32_t>(0x38) = 0;
    const Raw map{record.field(0x3c)};
    map.ptr(4) = host.map_buy_head_node_00440380(map);
    const Raw head{static_cast<std::uint8_t*>(map.ptr(4))};
    head.at<std::uint8_t>(0x11) = 1;
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(4) = map.ptr(4);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(0) = map.ptr(4);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(8) = map.ptr(4);
    map.at<std::uint32_t>(8) = 0;
    return record;
}

// 00442840, __thiscall(ECX = destination 48h-byte gun record, const record*
// source), RET 4, EAX = destination; only caller 00442940. vptr 00CE4530;
// floats +4, +8, +1Ch, +28h..+30h and dwords +20h, +24h are copied; the counted
// pointers +0Ch, +10h, +14h, +18h, +34h, +38h are cleared, then take the
// source's pointer with InterlockedIncrement (00CE221C) of its +4 when it is
// non-null; the std::map +3Ch is copy-constructed by 00442310. The unwind
// actions are not modelled.
Raw gun_record_copy_construct_00442840(Raw record, Raw source, SweepAHost& host) {
    record.at<std::uint32_t>(0) = 0x00ce4530;
    record.at<float>(4) = source.at<float>(4);
    record.at<float>(8) = source.at<float>(8);
    const auto copy_counted = [&](std::size_t off) {
        record.ptr(off) = nullptr;
        void* const value = source.ptr(off);
        if (value != nullptr) {
            record.ptr(off) = value;
            host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(value) + 4));
        }
    };
    copy_counted(0xc);
    copy_counted(0x10);
    copy_counted(0x14);
    copy_counted(0x18);
    record.at<float>(0x1c) = source.at<float>(0x1c);
    record.at<std::uint32_t>(0x20) = source.at<std::uint32_t>(0x20);
    record.at<std::uint32_t>(0x24) = source.at<std::uint32_t>(0x24);
    record.at<float>(0x28) = source.at<float>(0x28);
    record.at<float>(0x2c) = source.at<float>(0x2c);
    record.at<float>(0x30) = source.at<float>(0x30);
    copy_counted(0x34);
    copy_counted(0x38);
    host.gun_record_map_copy_construct_00442310(Raw{record.field(0x3c)}, Raw{source.field(0x3c)});
    return record;
}

// 00442940, __thiscall(ECX = {48h records* +0, count +4, capacity +8}, int
// capacity), RET 4. Capacity at least 1; a larger one allocates capacity*48h
// (00BF55BE), copy-constructs each record into a non-null slot with 00442840,
// destroys every old record through vtable slot 0 (flags 0), frees the old
// block (_free 00BF6989) and stores data and capacity. Only caller 00442A30.
void gun_records_reserve_00442940(Raw records, std::int32_t capacity, SweepAHost& host) {
    if (capacity < 1) capacity = 1;
    if (records.at<std::int32_t>(8) >= capacity) return;
    auto* fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity * 0x48)));
    for (std::int32_t i = 0; i < records.at<std::int32_t>(4); ++i) {
        std::uint8_t* const dst = fresh + 0x48 * i;
        if (dst == nullptr) continue;
        gun_record_copy_construct_00442840(Raw{dst}, Raw{static_cast<std::uint8_t*>(records.ptr(0)) + 0x48 * i}, host);
    }
    for (std::int32_t i = 0; i < records.at<std::int32_t>(4); ++i) {
        host.vcall_scalar_deleting_destructor_slot0(static_cast<std::uint8_t*>(records.ptr(0)) + 0x48 * i, 0);
    }
    host.free_00bf6989(records.ptr(0));
    records.ptr(0) = fresh;
    records.at<std::int32_t>(8) = capacity;
}

// 00442A30, __thiscall(ECX = records, int count), RET 4. Reserves through
// 00442940 when count exceeds the capacity, constructs records count..new-1 in
// non-null slots with 004427A0, then while the stored count is above the new
// one decrements it and destroys that record through vtable slot 0 (flags 0);
// finally stores the count. Callers 00442B62, 00442B75, 00732C9A, 00732FF9.
void gun_records_resize_00442a30(Raw records, std::int32_t count, SweepAHost& host) {
    if (count > records.at<std::int32_t>(8)) gun_records_reserve_00442940(records, count, host);
    for (std::int32_t i = records.at<std::int32_t>(4); i < count; ++i) {
        std::uint8_t* const slot = static_cast<std::uint8_t*>(records.ptr(0)) + 0x48 * i;
        if (slot != nullptr) gun_record_construct_004427a0(Raw{slot}, host);
    }
    while (count < records.at<std::int32_t>(4)) {
        records.at<std::int32_t>(4) -= 1;
        host.vcall_scalar_deleting_destructor_slot0(
            static_cast<std::uint8_t*>(records.ptr(0)) + 0x48 * records.at<std::int32_t>(4), 0);
    }
    records.at<std::int32_t>(4) = count;
}

// 00442AE0, __thiscall(ECX = device, float amount), RET 4; vtable slot
// (00CFDE04) and the call at 00728091 in 00728000. When [[00E188A8]+1FE4h] == 0,
// the unit [device+3Ch] answers vtable +5Ch(5) and 00927F30(unit, 0): amount
// *= *0043F880(config+4Ch, [game+1FE4h] != 0 ? 2 : [game+6ACh]) with config =
// 00432650() (the second +1FE4h read is redundant but kept). Then 0087D730
// (device, amount). The product is rounded to float in the argument slot.
void device_apply_scaled_damage_00442ae0(Raw device, float amount, SweepAHost& host) {
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    if (game.at<std::uint32_t>(0x1fe4) == 0) {
        void* const unit = device.ptr(0x3c);
        if (host.vcall_entity_is_kind_5c(unit, 5) && host.unit_is_local_player_role_00927f30(unit, 0)) {
            const Raw game_now{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
            const std::int32_t index = game_now.at<std::uint32_t>(0x1fe4) != 0
                                           ? 2
                                           : game_now.at<std::int32_t>(0x6ac);
            auto* config = static_cast<std::uint8_t*>(host.global_config_get_singleton_00432650());
            amount = *host.config_vector_at_0043f880(Raw{config + 0x4c}, index) * amount;
        }
    }
    host.unit_scale_damage_for_player_unit_0087d730(device, amount);
}

// 00442B70, __thiscall(ECX = records): 00442A30(records, 0), then _free(data).
// Reached by the unwind funclets 00C5FB28 and 00C85BC8 (JMP).
void gun_records_destroy_00442b70(Raw records, SweepAHost& host) {
    gun_records_resize_00442a30(records, 0, host);
    host.free_00bf6989(records.ptr(0));
}

// 00442C00..00443010: the class-id IsKindOf predicates, __thiscall(ECX = class,
// unused; int id), RET 4, EAX = 1 when id is the class's own id or an
// ancestor's, else 0. Each is one vtable slot (00CE454C, 00CE4578, 00CE45B4,
// 00CE45F8, 00CE462C, 00CE4660, 00CE4694, 00CE46D8) and has no direct caller.
// The ids are compared in the image's order; the order has no effect.
bool gun_class_base20_is_kind_of_00442c00(std::int32_t id) {
    return id == 0x20 || id == 0x1e || id == 0x4;
}
bool bomb_platform_class_is_kind_of_00442c70(std::int32_t id) {
    return id == 0x25 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool multiple_bomb_platform_class_is_kind_of_00442d00(std::int32_t id) {
    return id == 0x26 || id == 0x25 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool rfs_gun_class_is_kind_of_00442db0(std::int32_t id) {
    return id == 0x21 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool rt_gun_class_is_kind_of_00442e50(std::int32_t id) {
    return id == 0x23 || id == 0x22 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool st_gun_class_is_kind_of_00442ee0(std::int32_t id) {
    return id == 0x24 || id == 0x22 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool depth_charge_launcher_class_is_kind_of_00442fa0(std::int32_t id) {
    return id == 0x27 || id == 0x24 || id == 0x22 || id == 0x20 || id == 0x1e || id == 0x4;
}
bool catapult_class_is_kind_of_00443010(std::int32_t id) {
    return id == 0x28 || id == 0x20 || id == 0x1e || id == 0x4;
}

// 00440E10, __thiscall(ECX = entity); vtable slot (00CFDCFC) and one call.
// 0087BF80 first. A stack NativeString key is zeroed, sized 5 by 0041DD40(5, 1)
// and, when its data is set, memcpy'd from "Class" (00CE44C8) for length + 1
// bytes. globals (00B67980 on [00E188A8]+1A0Ch) -> "DeviceClass" (00CE44BC,
// 00B67800) -> index [[entity+354h]+6Ch] (00B67720) = value; self = 00927B40
// (entity); 00B675D0(self, &key, value) sets self.Class; the four LuaObjects
// are destroyed in reverse (self, value, DeviceClass, globals) and the key's
// data released. The unwind actions are not modelled.
void device_entity_set_lua_class_00440e10(Raw entity, SweepAHost& host) {
    host.unit_tickable_entity_init_slot_a4_0087bf80(entity);
    alignas(4) std::uint8_t key_bytes[8] = {};
    const Raw key{key_bytes};
    host.native_string_resize_0041dd40(key, 5, 1);
    if (key.ptr(4) != nullptr) std::memcpy(key.ptr(4), "Class", key.at<std::uint32_t>(0) + 1);
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    LuaObjectStorage globals_out;
    LuaObjectStorage* const globals = host.lua_state_owner_get_globals_00b67980(game.field(0x1a0c), &globals_out);
    LuaObjectStorage classes;
    host.lua_object_get_by_name_00b67800(globals, &classes, "DeviceClass");
    LuaObjectStorage value_out;
    const Raw device_class{static_cast<std::uint8_t*>(entity.ptr(0x354))};
    LuaObjectStorage* const value =
        host.lua_object_get_by_index_00b67720(&classes, &value_out, device_class.at<std::int32_t>(0x6c));
    LuaObjectStorage self_out;
    LuaObjectStorage* const self = host.mission_entity_lua_self_00927b40(entity, &self_out);
    host.lua_object_set_object_00b675d0(self, key, value);
    host.lua_object_destruct_00b67700(&self_out);
    host.lua_object_destruct_00b67700(&value_out);
    host.lua_object_destruct_00b67700(&classes);
    host.lua_object_destruct_00b67700(&globals_out);
    release_pooled_data(key, host);
}

// 004426C0, __thiscall(ECX = entity); vtable slot (00CFDCF4) and one call.
// 0077E830 first; then, when the holder [entity+C0h] is non-null with kind
// ([holder+4]) 3: 009238A0 fills a by-value LuaObject that 004425C0 consumes
// to build a stack reader; 00BD8E20 enters "_device" (00CE446C), 00BD6830
// reads "platform" (00CE4460) into entity+38Ch as field kind 1, 00BD7A20
// leaves and 00441A20 destroys the reader. The unwind action is not modelled.
void device_entity_place_in_world_lua_004426c0(Raw entity, SweepAHost& host) {
    host.mission_entity_attach_lua_self_0077e830(entity);
    const Raw holder{static_cast<std::uint8_t*>(entity.ptr(0xc0))};
    if (holder.p == nullptr || holder.at<std::int32_t>(4) != 3) return;
    LuaObjectStorage saved;
    host.sentity_get_saved_lua_data_009238a0(entity, &saved);
    alignas(4) std::uint8_t reader_bytes[0x14];
    const Raw reader{reader_bytes};
    host.lua_reader_construct_004425c0(reader, &saved);
    host.lua_reader_enter_key_00bd8e20(reader, 0, "_device");
    host.lua_reader_read_field_00bd6830(reader, 0, "platform", 1, entity.field(0x38c));
    host.lua_reader_leave_key_00bd7a20(reader);
    host.lua_reader_destroy_00441a20(reader);
}

// 00443490, __fastcall(ECX = class id, DL = retain flag), EAX = class; about
// fifty callers. ECX and DL pass unchanged to 00443090; a non-null class whose
// byte +44h is 0 gets vtable +10h(0) then vtable +14h(); the class pointer is
// returned either way.
void* device_class_resolve_and_activate_00443490(std::int32_t id, std::uint8_t flag, SweepAHost& host) {
    void* const cls = host.device_class_resolve_from_lua_00443090(id, flag);
    if (cls == nullptr) return cls;
    if (static_cast<std::uint8_t*>(cls)[0x44] != 0) return cls;
    host.vcall_device_class_activate_10(cls, 0);
    host.vcall_device_class_finish_14(cls);
    return cls;
}

// ============================ segment 4 ====================================

// 0044A3C0, __thiscall(ECX = field, archive), RET 4: vtable +10h of the archive
// with ({0, "picture" 00CE4994}, {1, field}) by value. Unreferenced in the image.
void archive_serialize_picture_field_0044a3c0(void* field, void* archive, SweepAHost& host) {
    host.vcall_archive_serialize_field_10(archive, 0, "picture", 1, field);
}

// 0044E670, __thiscall(ECX = reader, const reader* source), RET 4, EAX =
// reader: vptr 00CE44FC, then the LuaObject vector at +4 copy-constructed from
// source+4 by 0044DA70. Unreferenced in the image (00451A90 inlines the same).
Raw lua_reader_copy_construct_0044e670(Raw reader, Raw source, SweepAHost& host) {
    reader.at<std::uint32_t>(0) = 0x00ce44fc;
    host.lua_object_vector_copy_0044da70(Raw{reader.field(4)}, Raw{source.field(4)});
    return reader;
}

namespace {

// The checked-iterator test MSVC inlines after a find: a null or foreign
// container reports through 00BF6713.
void check_iterator(const void* container, const void* expected, SweepAHost& host) {
    if (container == nullptr || container != expected) host.invalid_parameter_noinfo_00bf6713();
}

// The stack reader destructor 00451A90 inlines: vptr 00CE44FC, 00441210 on
// +4, then the base vptr 00CE374C.
void destroy_inline_reader(Raw reader, SweepAHost& host) {
    reader.at<std::uint32_t>(0) = 0x00ce44fc;
    host.lua_object_vector_tidy_00441210(Raw{reader.field(4)});
    reader.at<std::uint32_t>(0) = 0x00ce374c;
}

}  // namespace

// 004514A0, __thiscall(ECX = panel owner, PooledString name by value), RET 8;
// caller 008B0A5F (KillDialog). The map at +1Ch (head [owner+20h] read first)
// is searched by 0044BA00 with a checked-iterator test; at the end iterator
// only the name is released. Otherwise 0044BA00 runs again and 00451020 erases
// that iterator; when 00435C40 finds the name equal (case-insensitive) to the
// current dialog string +28h, that string is assigned "" (00CE3A0C, 0041E350),
// 005B68D0 stops [[00E198C4]+A4h] and +34h = 0. The by-value name's data is
// released last (callee-destroyed argument).
void panel_owner_kill_dialog_004514a0(Raw owner, Raw name_by_value, SweepAHost& host) {
    void* const head = owner.ptr(0x20);
    const Raw map{owner.field(0x1c)};
    void* found[2];
    const auto* it = static_cast<void* const*>(host.panel_map_find_0044ba00(map, found, name_by_value.p));
    check_iterator(it[0], map.p, host);
    if (it[1] != head) {
        const auto* again = static_cast<void* const*>(host.panel_map_find_0044ba00(map, found, name_by_value.p));
        void* const container = again[0];
        void* const node = again[1];
        void* erased[2];
        host.panel_map_erase_iterator_00451020(map, erased, container, node);
        const Raw current{owner.field(0x28)};
        if (host.native_string_equals_insensitive_00435c40(name_by_value, current)) {
            host.native_string_assign_cstring_0041e350(current, "");
            const Raw interface_root{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
            host.panel_owner_stop_voice_005b68d0(interface_root.ptr(0xa4));
            owner.at<std::uint32_t>(0x34) = 0;
        }
    }
    release_pooled_data(name_by_value, host);
}

// 004518B0, __thiscall(ECX = panel owner); caller 0045F6A3 in 0045F600. The
// sequence map at +1Ch is cleared inline (004512E0 on the root [[+20h]+4], the
// head's three links on itself, size +24h = 0); the string +28h is resized to
// 0 (0041DD40(0, 0)) and, when its data is set, memcpy'd from "" (00CE3A0C)
// for its (zero) length; 005B68D0 on [[00E198C4]+A4h]; +34h = 0.
void panel_owner_reset_004518b0(Raw owner, SweepAHost& host) {
    const Raw map{owner.field(0x1c)};
    host.panel_map_destroy_subtree_004512e0(map, Raw{static_cast<std::uint8_t*>(owner.ptr(0x20))}.ptr(4));
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(4) = map.ptr(4);
    map.at<std::uint32_t>(8) = 0;
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(0) = map.ptr(4);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(8) = map.ptr(4);
    const Raw text{owner.field(0x28)};
    host.native_string_resize_0041dd40(text, 0, 0);
    if (text.ptr(4) != nullptr) std::memcpy(text.ptr(4), "", text.at<std::uint32_t>(0));
    const Raw interface_root{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    host.panel_owner_stop_voice_005b68d0(interface_root.ptr(0xa4));
    owner.at<std::uint32_t>(0x34) = 0;
}

// 00451A90, __thiscall(ECX = panel owner, PooledString name by value, LuaObject
// table by value), RET 1Ch; callers 008B0688 (StartDialog) and 00451F7B in
// 00451E50 (the session message). The
// table is copied (00B66FA0) into a by-value argument that 004425C0 consumes to
// build reader A; reader B is an inline copy of A (vptr 00CE44FC, 0044DA70 on
// +4); A is destroyed inline; 00451920(map +1Ch, &name, &B) finds or inserts
// the sequence and 004507D0 reads its properties; B is destroyed inline; the
// name's data is released and the by-value table destroyed (00B67700). The
// unwind actions are not modelled.
void panel_owner_start_dialog_from_lua_00451a90(Raw owner, Raw name_by_value,
                                                LuaObjectStorage* table_by_value, SweepAHost& host) {
    LuaObjectStorage copy;
    host.lua_object_copy_construct_00b66fa0(&copy, table_by_value);
    alignas(4) std::uint8_t reader_a_bytes[0x14];
    const Raw reader_a{reader_a_bytes};
    host.lua_reader_construct_004425c0(reader_a, &copy);
    alignas(4) std::uint8_t reader_b_bytes[0x14];
    const Raw reader_b{reader_b_bytes};
    reader_b.at<std::uint32_t>(0) = 0x00ce44fc;
    host.lua_object_vector_copy_0044da70(Raw{reader_b.field(4)}, Raw{reader_a.field(4)});
    destroy_inline_reader(reader_a, host);
    void* const sequence = host.panel_map_lookup_or_insert_00451920(Raw{owner.field(0x1c)}, name_by_value.p, reader_b);
    host.panel_sequence_read_properties_004507d0(sequence);
    destroy_inline_reader(reader_b, host);
    release_pooled_data(name_by_value, host);
    host.lua_object_destruct_00b67700(table_by_value);
}

// 00451E50, __thiscall(ECX = panel owner, message), RET 4; handler E1h in the
// session message table (multiplayer_session_cc13.cpp). A borrowed Lua state
// owner (4C8h-byte frame object) is built by 00B66C00 on [[[[00E188A8]+1A08h]
// +4]+4], runs the message's script with 00B65EC0(data [msg+28h] or the image
// byte 00E17BFB when null, length [msg+24h]); globals (00B67980) -> "Dialogues"
// (00CE4BB4, 00B67800) -> field named by the NativeString msg+18h (00B68100);
// both temporaries are destroyed; the field is copied (00B66FA0) into a by-value
// LuaObject, msg+18h is copied into a by-value NativeString (zeroed, 0041DD40
// (length, 1), memcpy of length bytes when non-zero), and 00451A90 runs with
// both; the field is destroyed (00B67700) and the owner closed (00B669A0).
// The unwind actions are not modelled.
void session_message_start_dialog_apply_00451e50(Raw owner, Raw message, SweepAHost& host) {
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    const Raw runner{static_cast<std::uint8_t*>(game.ptr(0x1a08))};
    void* const lua_state = Raw{static_cast<std::uint8_t*>(runner.ptr(4))}.ptr(4);
    alignas(4) std::uint8_t state_owner_bytes[0x4c8];
    const Raw state_owner{state_owner_bytes};
    host.lua_state_owner_construct_borrowed_00b66c00(state_owner, lua_state);
    const char* buffer = static_cast<const char*>(message.ptr(0x28));
    const std::uint32_t length = message.at<std::uint32_t>(0x24);
    if (buffer == nullptr) buffer = static_cast<const char*>(host.global_address(0x00e17bfb));
    host.lua_state_owner_do_buffer_00b65ec0(state_owner, buffer, length);
    LuaObjectStorage globals;
    LuaObjectStorage* const table = host.lua_state_owner_get_globals_00b67980(state_owner.p, &globals);
    LuaObjectStorage dialogues;
    // 00B67800 returns its out pointer in EAX, which the image uses as ECX next.
    host.lua_object_get_by_name_00b67800(table, &dialogues, "Dialogues");
    const Raw name{message.field(0x18)};
    LuaObjectStorage field;
    host.lua_object_get_by_native_string_00b68100(&dialogues, &field, name);
    host.lua_object_destruct_00b67700(&dialogues);
    host.lua_object_destruct_00b67700(&globals);

    LuaObjectStorage table_arg;
    host.lua_object_copy_construct_00b66fa0(&table_arg, &field);
    alignas(4) std::uint8_t name_bytes[8];
    const Raw name_arg{name_bytes};
    name_arg.at<std::uint32_t>(0) = 0;
    name_arg.at<std::uint32_t>(4) = 0;
    host.native_string_resize_0041dd40(name_arg, name.at<std::uint32_t>(0), 1);
    if (name.at<std::uint32_t>(0) != 0) std::memcpy(name_arg.ptr(4), name.ptr(4), name_arg.at<std::uint32_t>(0));
    panel_owner_start_dialog_from_lua_00451a90(owner, name_arg, &table_arg, host);
    host.lua_object_destruct_00b67700(&field);
    host.lua_state_owner_close_00b669a0(state_owner);
}

// ============================ segment 5 ====================================

// 004567B0 / 00456D10 (identical bodies), __thiscall(ECX = record): the
// NativeString at +4 (length +4, data +8) returns its data to the sized pool
// when set; nothing is cleared. 004567B0 has 5 calls, 00456D10 19.
void native_string_at4_destroy_storage_004567b0(Raw record, SweepAHost& host) {
    release_pooled_data(Raw{record.field(4)}, host);
}
void native_string_at4_destroy_storage_00456d10(Raw record, SweepAHost& host) {
    release_pooled_data(Raw{record.field(4)}, host);
}

// 0045CFC0, __fastcall(ECX = record); 75 calls. The record's destructor: the
// NativeStrings {length, data} at +4Ch, +40h, +38h, +2Ch, +20h, +14h, +8, +0
// return their data to the sized pool in that (reverse member) order; nothing
// is cleared. The SEH frame only tracks the member states.
void scene_record_eight_strings_destroy_0045cfc0(Raw record, SweepAHost& host) {
    static constexpr std::size_t kStrings[8] = {0x4c, 0x40, 0x38, 0x2c, 0x20, 0x14, 0x8, 0x0};
    for (const std::size_t offset : kStrings) release_pooled_data(Raw{record.field(offset)}, host);
}

// 0045C440, ECX = tokenizer passed through untouched to 008D8A70, stack
// (const char* text), RET 4, EAX = 1 when 00438E10(peeked token, text) is 0,
// else 0 (NEG/SBB/ADD). 13 calls, the scene property parser among them.
bool scene_token_equals_0045c440(void* tokenizer, const char* text, SweepAHost& host) {
    const char* const token = host.scene_tokenizer_peek_token_008d8a70(tokenizer);
    return host.cstring_compare_insensitive_00438e10(token, text) == 0;
}

// 004643A0, __thiscall(ECX = holder, entity), RET 4; vtable slot (00CE5500).
// director = entity vtable +114h(); a non-null director gets 00835860(director,
// [holder+14h], 1).
void fire_target_holder_apply_004643a0(Raw holder, void* entity, SweepAHost& host) {
    void* const director = host.vcall_entity_weapon_director_114(entity);
    if (director == nullptr) return;
    host.weapon_director_set_fire_target_00835860(director, holder.ptr(0x14), 1);
}

// 004654D0, __thiscall(ECX = command, entity), RET 4; vtable slot (00CE5530).
// A non-null entity answering vtable +5Ch(8) has the command string +2Ch
// compared case-insensitively (00425850) with "Surface" (00CE5490),
// "Periscope" (00CE5418), "Underwater" (00CE5484), "Maxdepth" (00CE5478) in
// that order; the first match calls 008528B0(entity, 0..3). No match: nothing.
void entity_command_set_submarine_depth_004654d0(Raw command, void* entity, SweepAHost& host) {
    if (entity == nullptr) return;
    if (!host.vcall_entity_is_kind_5c(entity, 8)) return;
    const Raw text{command.field(0x2c)};
    static constexpr const char* kLevels[4] = {"Surface", "Periscope", "Underwater", "Maxdepth"};
    for (std::int32_t level = 0; level < 4; ++level) {
        if (host.native_string_equals_cstring_insensitive_00425850(text, kLevels[level])) {
            host.submarine_set_depth_level_008528b0(entity, level);
            return;
        }
    }
}

// ============================ segment 6 ====================================

// 004684F0, __thiscall(ECX = reference, unit), RET 4, EAX = reference. +8..+10h
// copy the float3 global at 00F87574..00F8757C; word +0 = 0; +4 = unit; then a
// null unit gives byte +0 = 0 and word +2 = 0, a unit gives byte +0 = 1 and
// word +2 = [unit+174h]; +14h = 0.0f either way. One call.
Raw unit_reference_construct_004684f0(Raw reference, void* unit, SweepAHost& host) {
    const auto* origin = static_cast<const float*>(host.global_address(0x00f87574));
    reference.at<float>(8) = origin[0];
    reference.at<float>(0xc) = origin[1];
    reference.at<float>(0x10) = origin[2];
    reference.at<std::uint16_t>(0) = 0;
    reference.ptr(4) = unit;
    if (unit == nullptr) {
        reference.at<std::uint8_t>(0) = 0;
        reference.at<std::uint16_t>(2) = 0;
    } else {
        reference.at<std::uint8_t>(0) = 1;
        reference.at<std::uint16_t>(2) = *reinterpret_cast<const std::uint16_t*>(static_cast<std::uint8_t*>(unit) + 0x174);
    }
    reference.at<float>(0x14) = 0.0f;
    return reference;
}

// 00470440, __thiscall(ECX = modifiers, int index, unit), RET 8, ST0. When the
// byte 00E0C978 is set and the dword +88h + index*0Ch is non-zero, tail-jumps
// to 008E6430 with the same arguments; otherwise 1.0 (FLD1). 3 calls.
float gameplay_modifiers_powerup_multiplier_00470440(Raw modifiers, std::int32_t index, void* unit,
                                                     SweepAHost& host) {
    if (*static_cast<const std::uint8_t*>(host.global_address(0x00e0c978)) != 0 &&
        modifiers.at<std::uint32_t>(0x88 + 0xc * index) != 0) {
        return host.gameplay_modifiers_lookup_008e6430(modifiers, index, unit);
    }
    return 1.0f;
}

// 004704B0, __fastcall(ECX = hit record): _free(+3Ch) when non-null; the
// pointer is not cleared. 73 calls.
void hit_record_destroy_004704b0(Raw record, SweepAHost& host) {
    void* const parts = record.ptr(0x3c);
    if (parts != nullptr) host.free_00bf6989(parts);
}

// 004704E0, __fastcall(ECX = hit record), EAX: 0 when +4 is null or its vtable
// +108h() answers null, else that object's dword +ACh. 4 calls.
std::uint32_t hit_record_owning_unit_004704e0(Raw record, SweepAHost& host) {
    if (record.ptr(4) == nullptr) return 0;
    const Raw owner{static_cast<std::uint8_t*>(host.vcall_owner_unit_108(record.ptr(4)))};
    if (owner.p == nullptr) return 0;
    return owner.at<std::uint32_t>(0xac);
}

// 0047B7F0, __thiscall(ECX = {.., lo +38h, hi +3Ch}), ST0: 00BD2F10(stream 0,
// lo, hi). Unreferenced in the image.
float random_in_range_0047b7f0(Raw range, SweepAHost& host) {
    return host.random_uniform_00bd2f10(0, range.at<float>(0x38), range.at<float>(0x3c));
}

// 0047B880, __thiscall(ECX = unit), AL: 1 unless vtable +5Ch answers true for
// kind 10h or (asked only when 10h is false) kind 16h. 9 calls.
bool unit_uses_small_turn_roll_limit_0047b880(void* unit, SweepAHost& host) {
    if (host.vcall_entity_is_kind_5c(unit, 0x10)) return false;
    if (host.vcall_entity_is_kind_5c(unit, 0x16)) return false;
    return true;
}

// 0047B9F0, __cdecl(40h-byte a by value, 40h-byte b by value), EAX: 1 when the
// float at a+8 is above the one at b+8 (FCOMIP/JBE: NaN gives 0). Its address
// is pushed as a sort predicate at 004A2BEE inside 004A0D20.
bool blob40_greater_by_float8_0047b9f0(Blob40 a, Blob40 b) {
    float fa;
    float fb;
    std::memcpy(&fa, a.bytes + 8, sizeof fa);
    std::memcpy(&fb, b.bytes + 8, sizeof fb);
    return fa > fb;
}

// ============================ segment 7 ====================================

// 0048EE90, __thiscall(ECX = set); called by the deleting destructors 0048EF50
// and 0048F870, by 004A02E0 and two unwind funclets. vptr 00CE66DC; while the
// count +0Ch is non-zero, the last slot of the array at +8 is taken: a non-null
// box has its counted object (box+0) released (InterlockedDecrement of +4,
// vtable slot 0 with no argument at zero) and cleared when set, is freed
// (_free 00BF65AC) and the slot cleared; then the count drops by one. Finally
// 00485990(+8, 0) and _free(data) (00BF6989).
void traffic_path_set_destruct_0048ee90(Raw set, SweepAHost& host) {
    set.at<std::uint32_t>(0) = 0x00ce66dc;
    while (set.at<std::uint32_t>(0xc) != 0) {
        void** const slot = static_cast<void**>(set.ptr(8)) + (set.at<std::uint32_t>(0xc) - 1);
        auto** const box = static_cast<void**>(*slot);
        if (box != nullptr) {
            void* const object = *box;
            if (object != nullptr) {
                if (host.interlocked_decrement_ce2220(
                        reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(object) + 4)) == 0) {
                    host.vcall_release_slot0(object);
                }
                *box = nullptr;
            }
            host.free_00bf65ac(box);
            *slot = nullptr;
        }
        if (set.at<std::uint32_t>(0xc) != 0) set.at<std::uint32_t>(0xc) -= 1;
    }
    const Raw array{set.field(8)};
    host.pod_pointer_array_resize_00485990(array, 0);
    host.free_00bf6989(array.ptr(0));
}

// ============================ segment 9 ====================================

// 004BFED0, __fastcall(ECX = unit), AL; caller 007BB326 in 007BB310 (cockpit
// transform). Null interface root [00E198C4] -> 0. When the screen [root+9Ch]
// has byte +5 set: v = [[00E188A8]+1ED4h] non-null, v answers vtable +5Ch(54h)
// and 00798C30(v, unit) -> 1. Else when [root+98h] has byte +5 set: v non-null,
// +5Ch(55h) and 0078E470(v) == unit -> 1. Else: unit == [00E188D8] -> 1; or
// p = 004BEAD0(unit) non-null with [p+9D4h] non-null and equal to [00E188D8],
// and 007B8AD0(p) -> 1. Everything else -> 0.
bool unit_is_viewed_by_player_004bfed0(void* unit, SweepAHost& host) {
    const Raw root{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    if (root.p == nullptr) return false;
    if (Raw{static_cast<std::uint8_t*>(root.ptr(0x9c))}.at<std::uint8_t>(5) != 0) {
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
        void* const view = game.ptr(0x1ed4);
        return view != nullptr && host.vcall_entity_is_kind_5c(view, 0x54) &&
               host.view_contains_unit_00798c30(view, unit);
    }
    if (Raw{static_cast<std::uint8_t*>(root.ptr(0x98))}.at<std::uint8_t>(5) != 0) {
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
        void* const view = game.ptr(0x1ed4);
        return view != nullptr && host.vcall_entity_is_kind_5c(view, 0x55) &&
               host.view_unit_0078e470(view) == unit;
    }
    if (host.global_pointer(0x00e188d8) == unit) return true;
    const Raw plane{static_cast<std::uint8_t*>(host.unit_cast_if_class_flag0f_004bead0(unit))};
    if (plane.p == nullptr) return false;
    void* const leader = plane.ptr(0x9d4);
    if (leader == nullptr || host.global_pointer(0x00e188d8) != leader) return false;
    return host.unit_is_squadron_flight_leader_007b8ad0(plane.p);
}

// 004CA760, __thiscall(ECX = world, const float3* lo, const float3* hi), RET 8;
// caller 0089C2E8 (ReplaceSceneBorder). With the old bounds lo +711Ch..+7124h
// and hi +7128h..+7130h: rx = (hi.x - lo.x) / (old_hi.x - old_lo.x) and rz =
// (lo.z - hi.z) / (old_lo.z - old_hi.z) (x87 FSUB/FDIVP DE F9, each rounded to
// float). Every record of the four std::lists at +7134h + edge*0Ch (head +4)
// has the float at [node+8] scaled: edges 0 and 2 by rx, 1 and 3 by rz. The
// checked-iterator tests (00BF6713) are kept; for a valid list they never fire.
// The new bounds are stored and 004C71C0 lays the border records out again.
// The extended quotients are modelled in double then float.
void world_replace_border_bounds_004ca760(Raw world, const float* lo, const float* hi, SweepAHost& host) {
    const float rx = static_cast<float>((static_cast<double>(hi[0]) - lo[0]) /
                                        (static_cast<double>(world.at<float>(0x7128)) - world.at<float>(0x711c)));
    const float rz = static_cast<float>((static_cast<double>(lo[2]) - hi[2]) /
                                        (static_cast<double>(world.at<float>(0x7124)) - world.at<float>(0x7130)));
    for (int edge = 0; edge < 4; ++edge) {
        void* const head = world.ptr(0x7138 + 0xc * edge);
        for (void* node = *static_cast<void**>(head); node != head; ) {
            if (node == world.ptr(0x7138 + 0xc * edge)) host.invalid_parameter_noinfo_00bf6713();
            float* const value = static_cast<float*>(Raw{static_cast<std::uint8_t*>(node)}.ptr(8));
            *value = *value * ((edge == 0 || edge == 2) ? rx : rz);
            if (node == world.ptr(0x7138 + 0xc * edge)) host.invalid_parameter_noinfo_00bf6713();
            node = *static_cast<void**>(node);
        }
    }
    world.at<float>(0x711c) = lo[0];
    world.at<float>(0x7120) = lo[1];
    world.at<float>(0x7124) = lo[2];
    world.at<float>(0x7128) = hi[0];
    world.at<float>(0x712c) = hi[1];
    world.at<float>(0x7130) = hi[2];
    host.world_layout_border_zone_records_004c71c0(world);
}

// 004C9800, __thiscall(ECX = record), EAX = record: `eh vector constructor
// iterator' (00BF7CD1) over the five 38h-byte slots at +8 with constructor
// 004C6750 and destructor 004C6760. Reached through the dword at 004DA2C2
// (004DA2A0 builds the record).
Raw scene_record_player_slot_construct_004c9800(Raw record, SweepAHost& host) {
    host.eh_vector_constructor_iterator_00bf7cd1(record.field(8), 0x38, 5, 0x004c6750, 0x004c6760);
    return record;
}

// 004C9820, __thiscall(ECX = record): `eh vector destructor iterator'
// (00BF7C6E) over the same five slots with 004C6760. Reached through four
// dwords in .text (004D7833 and three more).
void scene_record_player_slot_destroy_004c9820(Raw record, SweepAHost& host) {
    host.eh_vector_destructor_iterator_00bf7c6e(record.field(8), 0x38, 5, 0x004c6760);
}

// 004CAF50, __thiscall(ECX = {counted T** data, count, capacity}, T* const*
// handle), RET 4; one call. A full vector reserves max(2*capacity, 1) through
// 004C8310; the new slot (when its address is non-null) is cleared, then takes
// a non-null handle with InterlockedIncrement of handle+4; the count rises by one.
void effect_handle_vector_push_back_004caf50(Raw vector, void* const* handle, SweepAHost& host) {
    if (vector.at<std::int32_t>(4) == vector.at<std::int32_t>(8)) {
        std::int32_t capacity = vector.at<std::int32_t>(8) * 2;
        if (capacity <= 1) capacity = 1;
        host.effect_handle_vector_reserve_004c8310(vector, capacity);
    }
    void** const slot = static_cast<void**>(vector.ptr(0)) + vector.at<std::int32_t>(4);
    if (slot != nullptr) {
        *slot = nullptr;
        void* const value = *handle;
        if (value != nullptr) {
            *slot = value;
            host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(value) + 4));
        }
    }
    vector.at<std::int32_t>(4) += 1;
}

// 004CB160, __thiscall(ECX = counted pointer vector, int count), RET 4; six
// calls. Reserve (004C8310) when count exceeds the capacity; new slots with a
// non-null address are cleared; while the stored count is above the new one it
// drops by one and that slot's pointer, when set, is released (InterlockedDecrement
// of +4, vtable slot 0 with no argument at zero) and cleared; the count is stored.
void effect_handle_vector_resize_004cb160(Raw vector, std::int32_t count, SweepAHost& host) {
    if (count > vector.at<std::int32_t>(8)) host.effect_handle_vector_reserve_004c8310(vector, count);
    for (std::int32_t i = vector.at<std::int32_t>(4); i < count; ++i) {
        void** const slot = static_cast<void**>(vector.ptr(0)) + i;
        if (slot != nullptr) *slot = nullptr;
    }
    while (count < vector.at<std::int32_t>(4)) {
        vector.at<std::int32_t>(4) -= 1;
        void** const slot = static_cast<void**>(vector.ptr(0)) + vector.at<std::int32_t>(4);
        void* const value = *slot;
        if (value == nullptr) continue;
        if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(value) + 4)) == 0) {
            host.vcall_release_slot0(value);
        }
        *slot = nullptr;
    }
    vector.at<std::int32_t>(4) = count;
}

// 004CC3F0, __thiscall(ECX = record, const NativeString* name), RET 4, EAX =
// record; caller 004CD405 in 004CD390. +8 = 0.0f, +0 = +4 = 0, +0Ch = 1.0f
// (00D7A24C), bytes +10h/+11h = 0; the NativeString +14h is zeroed and, unless
// it is the argument itself, sized by 0041DD40(name.length, 1) and memcpy'd
// for its length when the name's length is non-zero; +1Ch = 0, bytes +20h/+21h = 0.
Raw end_of_mission_movie_construct_004cc3f0(Raw record, Raw name, SweepAHost& host) {
    record.at<float>(8) = 0.0f;
    record.at<std::uint32_t>(0) = 0;
    record.at<std::uint32_t>(4) = 0;
    record.at<float>(0xc) = 1.0f;
    record.at<std::uint8_t>(0x10) = 0;
    record.at<std::uint8_t>(0x11) = 0;
    const Raw text{record.field(0x14)};
    text.at<std::uint32_t>(0) = 0;
    text.at<std::uint32_t>(4) = 0;
    if (text.p != name.p) {
        host.native_string_resize_0041dd40(text, name.at<std::uint32_t>(0), 1);
        if (name.at<std::uint32_t>(0) != 0) std::memcpy(text.ptr(4), name.ptr(4), text.at<std::uint32_t>(0));
    }
    record.at<std::uint32_t>(0x1c) = 0;
    record.at<std::uint8_t>(0x20) = 0;
    record.at<std::uint8_t>(0x21) = 0;
    return record;
}

// ============================ segment 10 ===================================

// 004D7D50, __thiscall(ECX = game, byte suppress_prompt), RET 4; caller
// 0076D122 in 0076D0E0 (session game-closed message). byte game+608h = 1; 004CD0F0(game, 1, 0, 1);
// when the interface root [00E198C4] is set and its screen [root+E4h] has byte
// +5 set: 00532A20(00425D10(), 4) and 006882A0([00E198B4]). A non-zero argument
// only sets [00F1B038] = 2. Otherwise [00F1B038] = 1, NativeStrings text = ""
// (00CE3A0C) and title = "ingame.multi_gameclosed" (00CE79A8) are built, and a
// third "" goes by value to 00531B00(00425D10(), 1, &title, 2, 004D7B50, 0,
// &text, 0.0f, 0, that string, 1), which owns it; title then text are released.
// The unwind actions are not modelled.
void game_show_game_closed_004d7d50(Raw game, std::uint8_t suppress_prompt, SweepAHost& host) {
    game.at<std::uint8_t>(0x608) = 1;
    host.game_set_cinematic_mode_004cd0f0(game, 1, 0, 1);
    const Raw root{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    if (root.p != nullptr && Raw{static_cast<std::uint8_t*>(root.ptr(0xe4))}.at<std::uint8_t>(5) != 0) {
        host.menu_prompt_screen_dismiss_slot_00532a20(host.menu_command_screen_get_singleton_00425d10(), 4);
        host.multi_menu_close_pause_interface_006882a0(host.global_pointer(0x00e198b4));
    }
    auto* const latch = static_cast<std::uint32_t*>(host.global_address(0x00f1b038));
    if (suppress_prompt != 0) {
        *latch = 2;
        return;
    }
    alignas(4) std::uint8_t text_bytes[8];
    alignas(4) std::uint8_t title_bytes[8];
    alignas(4) std::uint8_t arg_bytes[8];
    const Raw text{text_bytes};
    const Raw title{title_bytes};
    const Raw arg{arg_bytes};
    *latch = 1;
    host.native_string_construct_0041e870(text, "");
    host.native_string_construct_0041e870(title, "ingame.multi_gameclosed");
    host.native_string_construct_0041e870(arg, "");
    void* const menu = host.menu_command_screen_get_singleton_00425d10();
    host.menu_prompt_screen_raise_00531b00(menu, 1, title, 2, 0x004d7b50, 0, text, 0.0f, 0, arg, 1);
    release_pooled_data(title, host);
    release_pooled_data(text, host);
}

// ============================ segment 11 ===================================

namespace {

// The inlined unit-list walk of the unregister overrides: from [list+4] along
// +4 until a node whose +8 is the entity, which 004837D0 erases; no match does
// nothing.
void unlink_from_world_list(Raw entity, std::size_t list_offset, SweepAHost& host) {
    const Raw list{static_cast<std::uint8_t*>(entity.ptr(0x30)) + list_offset};
    for (void* node = list.ptr(4); node != nullptr; node = Raw{static_cast<std::uint8_t*>(node)}.ptr(4)) {
        if (Raw{static_cast<std::uint8_t*>(node)}.ptr(8) == entity.p) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}

void link_into_world_list(Raw entity, std::size_t list_offset, SweepAHost& host) {
    host.unit_list_push_back_00484540(Raw{static_cast<std::uint8_t*>(entity.ptr(0x30)) + list_offset}, entity.p);
}

}  // namespace

// World-list registration overrides, __fastcall(ECX = entity), RET; each is a
// vtable slot only (no direct call). Register: base 00928560, then 00484540 on
// each list [entity+30h]+offset. Unregister: base 00928570, then for each list
// the walk above. Lists: MovieCamLookat +33Ch (004E79B0); LandingPoint +174h
// (004E9670/004E9690); Landscape +348h (004F1400/004F1420); SpawnPoint +30h
// then +3B4h (004F1910/004F1940); LandConvoy +30h then +150h (004F25D0/004F2600).
void movie_cam_lookat_unregister_004e79b0(Raw entity, SweepAHost& host) {
    host.game_entity_unregister_00928570(entity);
    unlink_from_world_list(entity, 0x33c, host);
}
void landing_point_register_004e9670(Raw entity, SweepAHost& host) {
    host.game_entity_register_00928560(entity);
    link_into_world_list(entity, 0x174, host);
}
void landing_point_unregister_004e9690(Raw entity, SweepAHost& host) {
    host.game_entity_unregister_00928570(entity);
    unlink_from_world_list(entity, 0x174, host);
}
void landscape_register_004f1400(Raw entity, SweepAHost& host) {
    host.game_entity_register_00928560(entity);
    link_into_world_list(entity, 0x348, host);
}
void landscape_unregister_004f1420(Raw entity, SweepAHost& host) {
    host.game_entity_unregister_00928570(entity);
    unlink_from_world_list(entity, 0x348, host);
}
void spawn_point_register_004f1910(Raw entity, SweepAHost& host) {
    host.game_entity_register_00928560(entity);
    link_into_world_list(entity, 0x30, host);
    link_into_world_list(entity, 0x3b4, host);
}
void spawn_point_unregister_004f1940(Raw entity, SweepAHost& host) {
    host.game_entity_unregister_00928570(entity);
    unlink_from_world_list(entity, 0x30, host);
    unlink_from_world_list(entity, 0x3b4, host);
}
void land_convoy_register_004f25d0(Raw entity, SweepAHost& host) {
    host.game_entity_register_00928560(entity);
    link_into_world_list(entity, 0x30, host);
    link_into_world_list(entity, 0x150, host);
}
void land_convoy_unregister_004f2600(Raw entity, SweepAHost& host) {
    host.game_entity_unregister_00928570(entity);
    unlink_from_world_list(entity, 0x30, host);
    unlink_from_world_list(entity, 0x150, host);
}

// 004F1390, __fastcall(ECX = landscape), EAX: landscape+1E4h, or null for a null
// landscape (the base-to-member pointer adjustment); vtable slot 00CEA140.
void* landscape_get_collision_node_004f1390(Raw landscape) {
    return landscape.p != nullptr ? landscape.field(0x1e4) : nullptr;
}

// 004F9D30, no arguments, EAX = 00E18D50: the function-local static colour
// {1, 1, 1, 1} (1.0f from 00D7A24C) guarded by bit 0 of 00E18D60, set on the
// first call. Six calls.
float* color_static_white_004f9d30(SweepAHost& host) {
    auto* guard = static_cast<std::uint32_t*>(host.global_address(0x00e18d60));
    auto* color = static_cast<float*>(host.global_address(0x00e18d50));
    if ((*guard & 1u) == 0) {
        *guard |= 1u;
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 1.0f;
    }
    return color;
}

// 004E9520, __thiscall(ECX = entity, word id), RET 4, EAX = entity; caller
// 004E9D40. 00928630(entity, 0, id); vptrs +0 = 00CE90E0, +10h = 00CE90C4,
// +24h = 00CE90BC, +170h = 00CE90B8; +1E8h..+1F0h and +1F8h = 0, byte +1F4h = 1,
// the member vptr +1E4h = 00CE75CC; the std::list at +1FCh gets its head from
// 004E7A10 (+200h) and size +204h = 0; +220h = 0; class id +C4h = 1Dh; +208h =
// -63h; +218h = 1000.0f (00CE3804). The unwind actions are not modelled.
Raw landing_point_construct_body_004e9520(Raw entity, std::uint16_t id, SweepAHost& host) {
    host.game_entity_construct_00928630(entity, 0, id);
    entity.at<std::uint32_t>(0) = 0x00ce90e0;
    entity.at<std::uint32_t>(0x10) = 0x00ce90c4;
    entity.at<std::uint32_t>(0x24) = 0x00ce90bc;
    entity.at<std::uint32_t>(0x170) = 0x00ce90b8;
    entity.at<std::uint32_t>(0x1e8) = 0;
    entity.at<std::uint32_t>(0x1ec) = 0;
    entity.at<std::uint32_t>(0x1f0) = 0;
    entity.at<std::uint32_t>(0x1f8) = 0;
    entity.at<std::uint8_t>(0x1f4) = 1;
    entity.at<std::uint32_t>(0x1e4) = 0x00ce75cc;
    const Raw list{entity.field(0x1fc)};
    list.ptr(4) = host.list_buy_head_node_004e7a10(list);
    list.at<std::uint32_t>(8) = 0;
    entity.at<std::uint32_t>(0x220) = 0;
    entity.at<std::uint32_t>(0xc4) = 0x1d;
    entity.at<std::uint32_t>(0x208) = 0xffffff9du;
    entity.at<float>(0x218) = 1000.0f;
    return entity;
}

namespace {

// The FixedInstance scene creator shape shared by 004E9D40, 004F1460 and
// 004F1A60 (their listings differ only in the size and the body constructor):
// operator new(size) (00BF55BE), memset 0 (before the null check, as in the
// image); a non-null block: a stack NativeString built by 0041E870(name) goes
// by value to 006AF3D0([[00E188A8]+21D8h]) for the word id, then the body
// constructor (block, id); a null block stays null and the following vtable
// call dereferences it as the image does. vtable +98h(entity, a,
// [[00E188A8]+19CCh], b); the NativeString +154h is resized to strlen(name)
// (0 for a null name) with preserve 0 and the name memcpy'd for that length
// when the data is non-null.
template <class Construct>
void* fixed_instance_create(const char* name, std::uint32_t a, std::uint32_t b, std::uint32_t size,
                            Construct construct, SweepAHost& host) {
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(size));
    std::memset(block, 0, size);
    Raw entity{nullptr};
    if (block != nullptr) {
        alignas(4) std::uint8_t name_bytes[8];
        const Raw name_arg{name_bytes};
        host.native_string_construct_0041e870(name_arg, name);
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
        const std::uint16_t id = host.entity_network_id_from_name_006af3d0(game.ptr(0x21d8), name_arg);
        entity = construct(Raw{block}, id);
    }
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    host.vcall_entity_place_98(entity.p, a, game.at<std::uint32_t>(0x19cc), b);
    const Raw text{entity.field(0x154)};
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(text, length, 0);
    if (text.ptr(4) != nullptr) std::memcpy(text.ptr(4), name, text.at<std::uint32_t>(0));
    return entity.p;
}

}  // namespace

// 004E9D40, __fastcall(ECX unused, EDX = const char* name; a, b, two unused
// dwords), RET 10h, EAX = entity; the LandingPoint (1Dh) scene creator held at
// .text 004F2940 by the factory table 004F2800: size 224h, body 004E9520.
void* landing_point_create_from_scene_004e9d40(const char* name, std::uint32_t a, std::uint32_t b,
                                               std::uint32_t /*unused3*/, std::uint32_t /*unused4*/,
                                               SweepAHost& host) {
    return fixed_instance_create(name, a, b, 0x224,
        [&host](Raw block, std::uint16_t id) { return landing_point_construct_body_004e9520(block, id, host); },
        host);
}

// 004F2700, __fastcall(ECX unused, EDX = const char* name; a, b, property bag,
// unused), RET 10h, EAX = entity; the LandConvoy (1Ah) creator held at .text
// 004F29E8. operator new(3CCh), memset 0; a non-null block is built by
// 004F2410(block, 0) (no network-id lookup here), a null one stays null; then
// the 004F04C0 decision inlined (frame job pool +4 vtable +10h: defer through
// 004F03C0(bag, a, b), else vtable +98h(a, [[00E188A8]+19CCh], b)); the name is
// copied into +154h as in 004E9D40.
void* land_convoy_create_from_scene_004f2700(const char* name, std::uint32_t a, std::uint32_t b, void* bag,
                                             std::uint32_t /*unused4*/, SweepAHost& host) {
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(0x3cc));
    std::memset(block, 0, 0x3cc);
    const Raw entity = block != nullptr ? land_convoy_construct_004f2410(Raw{block}, 0, host) : Raw{nullptr};
    scene_unit_place_or_defer_004f04c0(entity.p, bag, a, b, host);
    const Raw text{entity.field(0x154)};
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(text, length, 0);
    if (text.ptr(4) != nullptr) std::memcpy(text.ptr(4), name, text.at<std::uint32_t>(0));
    return entity.p;
}

// 004EA650, __fastcall(ECX unused, EDX = const char* name; a, const 40h-byte
// pose* b, two unused), RET 10h, EAX = entity; the Path (47h) creator held at
// .text 004F2A3C. The 004E9D40 shape with size 210h and body 0047B660, except
// that the 40h bytes at b are copied (REP MOVSD) into a stack block whose
// address goes to vtable +98h in place of b.
void* path_create_from_scene_004ea650(const char* name, std::uint32_t a, const void* b,
                                      std::uint32_t /*unused3*/, std::uint32_t /*unused4*/,
                                      SweepAHost& host) {
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(0x210));
    std::memset(block, 0, 0x210);
    Raw entity{nullptr};
    if (block != nullptr) {
        alignas(4) std::uint8_t name_bytes[8];
        const Raw name_arg{name_bytes};
        host.native_string_construct_0041e870(name_arg, name);
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
        const std::uint16_t id = host.entity_network_id_from_name_006af3d0(game.ptr(0x21d8), name_arg);
        entity = Raw{static_cast<std::uint8_t*>(host.path_construct_0047b660(Raw{block}, id))};
    }
    alignas(4) std::uint8_t pose[0x40];
    std::memcpy(pose, b, sizeof pose);
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    host.vcall_entity_place_98(entity.p, a, game.at<std::uint32_t>(0x19cc),
                               static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pose)));
    const Raw text{entity.field(0x154)};
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(text, length, 0);
    if (text.ptr(4) != nullptr) std::memcpy(text.ptr(4), name, text.at<std::uint32_t>(0));
    return entity.p;
}

// 004E9F80, __fastcall(ECX unused, EDX = const char* name; unused a, const
// pose* b, property bag, unused), RET 10h, EAX = entity; the WaterMine (34h)
// creator held at .text 004F29B0. The "Type" property (008F2260, no null test)
// gives the bullet id [prop+0Ch]; 006EAFE0(&holder, id) fills a counted holder
// in the bag argument's stack slot and the class is [holder]; the "Party"
// (00CE5804) property gives [prop2+0Ch]; class vtable +20h(0, party, b+30h,
// b+20h, 0, 0, 0) spawns, and the entity is that result's +CCh; the holder is
// released (InterlockedDecrement of +4, vtable slot 0 at zero) and cleared when
// set; the name is copied into entity+154h as in 004E9D40.
void* water_mine_create_from_scene_004e9f80(const char* name, std::uint32_t /*a*/, const std::uint8_t* b,
                                            void* bag, std::uint32_t /*unused4*/, SweepAHost& host) {
    const Raw type{static_cast<std::uint8_t*>(host.scene_property_bag_find_008f2260(bag, "Type"))};
    void* holder = nullptr;
    void** const filled = host.bullet_class_acquire_006eafe0(&holder, type.at<std::int32_t>(0xc));
    void* const cls = *filled;
    const Raw party{static_cast<std::uint8_t*>(host.scene_property_bag_find_008f2260(bag, "Party"))};
    const Raw spawned{static_cast<std::uint8_t*>(host.vcall_bullet_class_spawn_20(
        cls, 0, party.at<std::uint32_t>(0xc), b + 0x30, b + 0x20, 0, 0, 0))};
    const Raw entity{static_cast<std::uint8_t*>(spawned.ptr(0xcc))};
    if (holder != nullptr) {
        if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(holder) + 4)) == 0) {
            host.vcall_release_slot0(holder);
        }
        holder = nullptr;
    }
    const Raw text{entity.field(0x154)};
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(text, length, 0);
    if (text.ptr(4) != nullptr) std::memcpy(text.ptr(4), name, text.at<std::uint32_t>(0));
    return entity.p;
}

// 004F1460, the Landscape (44h) creator, held at .text 004F29CC; the
// 004E9D40 shape with size 430h and body 004F11C0.
void* landscape_create_from_scene_004f1460(const char* name, std::uint32_t a, std::uint32_t b,
                                           std::uint32_t /*unused3*/, std::uint32_t /*unused4*/,
                                           SweepAHost& host) {
    return fixed_instance_create(name, a, b, 0x430,
        [&host](Raw block, std::uint16_t id) { return landscape_construct_004f11c0(block, id, host); },
        host);
}

// 004F11C0, __thiscall(ECX = entity, word id), RET 4, EAX = entity; caller
// 004F1460. 00928630(entity, 0, id); the member at +1E4h gets vptr 00CE89E8
// before 004E6480 constructs it, then +230h (member+4Ch) = entity; +344h =
// 00CE89DC; +360h = entity ? entity+1E4h : 0 (NEG/SBB/AND); +354h..+35Ch copy
// the float3 global 00F87574 and +348h..+350h copy those; final vptrs +0 =
// 00CEA090, +10h = 00CEA078, +24h = 00CEA070, +170h = 00CEA06C, +1E4h =
// 00CEA05C, +344h = 00CEA050; zeroed +368h, +36Ch, +370h, +378h..+380h, +3C4h,
// +3C8h, +3D4h, +3E4h, +3F4h, +3F8h, +400h..+408h, +414h, +424h..+42Ch,
// +3CCh; +3FCh = 00CE8BA4; +B4h = +B8h = 1; class id +C4h = 44h. The unwind
// actions are not modelled.
Raw landscape_construct_004f11c0(Raw entity, std::uint16_t id, SweepAHost& host) {
    host.game_entity_construct_00928630(entity, 0, id);
    const Raw member{entity.field(0x1e4)};
    member.at<std::uint32_t>(0) = 0x00ce89e8;
    host.landscape_member_construct_004e6480(member);
    member.ptr(0x4c) = entity.p;
    entity.at<std::uint32_t>(0x344) = 0x00ce89dc;
    entity.ptr(0x360) = entity.p != nullptr ? member.p : nullptr;
    const auto* origin = static_cast<const float*>(host.global_address(0x00f87574));
    entity.at<float>(0x354) = origin[0];
    entity.at<float>(0x358) = origin[1];
    entity.at<float>(0x35c) = origin[2];
    entity.at<float>(0x348) = entity.at<float>(0x354);
    entity.at<float>(0x34c) = entity.at<float>(0x358);
    entity.at<float>(0x350) = entity.at<float>(0x35c);
    entity.at<std::uint32_t>(0) = 0x00cea090;
    entity.at<std::uint32_t>(0x10) = 0x00cea078;
    entity.at<std::uint32_t>(0x24) = 0x00cea070;
    entity.at<std::uint32_t>(0x170) = 0x00cea06c;
    member.at<std::uint32_t>(0) = 0x00cea05c;
    entity.at<std::uint32_t>(0x344) = 0x00cea050;
    for (const std::size_t off : {0x368u, 0x36cu, 0x370u, 0x378u, 0x37cu, 0x380u, 0x3c4u, 0x3c8u, 0x3d4u,
                                  0x3e4u, 0x3f4u, 0x3f8u}) {
        entity.at<std::uint32_t>(off) = 0;
    }
    entity.at<std::uint32_t>(0x3fc) = 0x00ce8ba4;
    for (const std::size_t off : {0x400u, 0x404u, 0x408u, 0x414u, 0x424u, 0x428u, 0x42cu, 0x3ccu}) {
        entity.at<std::uint32_t>(off) = 0;
    }
    entity.at<std::uint32_t>(0xb4) = 1;
    entity.at<std::uint32_t>(0xb8) = 1;
    entity.at<std::uint32_t>(0xc4) = 0x44;
    return entity;
}

// 004F1A60, the SpawnPoint (4Dh) creator, held at .text 004F2A90; the
// 004E9D40 shape with size 348h and body 004F1610.
void* spawn_point_create_from_scene_004f1a60(const char* name, std::uint32_t a, std::uint32_t b,
                                             std::uint32_t /*unused3*/, std::uint32_t /*unused4*/,
                                             SweepAHost& host) {
    return fixed_instance_create(name, a, b, 0x348,
        [&host](Raw block, std::uint16_t id) { return spawn_point_construct_body_004f1610(block, id, host); },
        host);
}

// 004F1610, __thiscall(ECX = entity, id), RET 4, EAX = entity; caller 004F1ACE
// in 004F1A60. 0077EED0(entity, id); vptrs +0 = 00CEA218, +10h = 00CEA200,
// +24h = 00CEA1F8, +170h = 00CEA1F4, +1E4h = 00CEA1EC; the std::map at +310h
// gets its head from 004E7AE0 (isnil byte +15h = 1, links on itself, size
// +318h = 0); the lists at +31Ch and +328h get heads from 004E79F0 and size 0;
// class id +C4h = 4Dh. The unwind actions are not modelled.
Raw spawn_point_construct_body_004f1610(Raw entity, std::uint32_t id, SweepAHost& host) {
    host.unit_owner_entity_construct_0077eed0(entity, id);
    entity.at<std::uint32_t>(0) = 0x00cea218;
    entity.at<std::uint32_t>(0x10) = 0x00cea200;
    entity.at<std::uint32_t>(0x24) = 0x00cea1f8;
    entity.at<std::uint32_t>(0x170) = 0x00cea1f4;
    entity.at<std::uint32_t>(0x1e4) = 0x00cea1ec;
    const Raw map{entity.field(0x310)};
    map.ptr(4) = host.tree_buy_head_node_004e7ae0(map);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.at<std::uint8_t>(0x15) = 1;
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(4) = map.ptr(4);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(0) = map.ptr(4);
    Raw{static_cast<std::uint8_t*>(map.ptr(4))}.ptr(8) = map.ptr(4);
    map.at<std::uint32_t>(8) = 0;
    const Raw first{entity.field(0x31c)};
    first.ptr(4) = host.list_buy_head_node_004e79f0(first);
    first.at<std::uint32_t>(8) = 0;
    const Raw second{entity.field(0x328)};
    second.ptr(4) = host.list_buy_head_node_004e79f0(second);
    second.at<std::uint32_t>(8) = 0;
    entity.at<std::uint32_t>(0xc4) = 0x4d;
    return entity;
}

// 004F19A0, __thiscall(ECX = entity); caller the deleting destructor 004F1A40.
// 004E8150 clears the lists +328h then +31Ch; the map +310h erases [begin, end)
// with 004F02E0 (begin = the head's +0), its head is freed (_free 00BF65AC) and
// +314h/+318h cleared; 0077E380 runs the base destructor.
void spawn_point_destruct_004f19a0(Raw entity, SweepAHost& host) {
    host.list_clear_004e8150(Raw{entity.field(0x328)});
    host.list_clear_004e8150(Raw{entity.field(0x31c)});
    const Raw map{entity.field(0x310)};
    void* const head = map.ptr(4);
    void* ret[2];
    host.tree_erase_range_004f02e0(map, ret, map.p, *static_cast<void**>(head), map.p, head);
    host.free_00bf65ac(map.ptr(4));
    map.ptr(4) = nullptr;
    map.at<std::uint32_t>(8) = 0;
    host.unit_owner_entity_destruct_0077e380(entity);
}

// 004F2410, __thiscall(ECX = entity, id), RET 4, EAX = entity; callers 004F274B
// (LandConvoy creator) and 00743BBE (session message). 0077EED0(entity, id);
// 00875890(+310h, entity, 0); 0049F9B0(+344h); vptrs +0 = 00CEA570, +10h =
// 00CEA554, +24h = 00CEA54C, +170h = 00CEA548, +1E4h = 00CEA540, +310h =
// 00CEA528; +398h..+3A0h = 0; byte +3C8h = 0; class id +C4h = 1Ah.
Raw land_convoy_construct_004f2410(Raw entity, std::uint32_t id, SweepAHost& host) {
    host.unit_owner_entity_construct_0077eed0(entity, id);
    host.tick_registration_construct_00875890(entity.field(0x310), entity.p, 0);
    host.convoy_member344_construct_0049f9b0(entity.field(0x344));
    entity.at<std::uint32_t>(0) = 0x00cea570;
    entity.at<std::uint32_t>(0x10) = 0x00cea554;
    entity.at<std::uint32_t>(0x24) = 0x00cea54c;
    entity.at<std::uint32_t>(0x170) = 0x00cea548;
    entity.at<std::uint32_t>(0x1e4) = 0x00cea540;
    entity.at<std::uint32_t>(0x310) = 0x00cea528;
    entity.at<std::uint32_t>(0x398) = 0;
    entity.at<std::uint32_t>(0x39c) = 0;
    entity.at<std::uint32_t>(0x3a0) = 0;
    entity.at<std::uint8_t>(0x3c8) = 0;
    entity.at<std::uint32_t>(0xc4) = 0x1a;
    return entity;
}

// 004F2660, __thiscall(ECX = entity); caller the deleting destructor 004F2AE0.
// _free(+398h) when set (00BF65AC); +398h..+3A0h = 0; 0049F910 on +344h and
// 00875490 on +310h (each null when the entity is null: the base-adjust test the
// image makes); 0077E380 runs the base destructor.
void land_convoy_destruct_004f2660(Raw entity, SweepAHost& host) {
    void* const data = entity.ptr(0x398);
    if (data != nullptr) host.free_00bf65ac(data);
    entity.at<std::uint32_t>(0x398) = 0;
    entity.at<std::uint32_t>(0x39c) = 0;
    entity.at<std::uint32_t>(0x3a0) = 0;
    host.convoy_member344_destruct_0049f910(entity.p != nullptr ? entity.field(0x344) : nullptr);
    host.unit_tick_element_destruct_00875490(entity.p != nullptr ? entity.field(0x310) : nullptr);
    host.unit_owner_entity_destruct_0077e380(entity);
}

// 004F04C0, __fastcall(ECX = entity, EDX = property bag; a, b), RET 8; callers
// 004F0C88 (Stationary), 004F0D72 (Wreck), 007011DB (dummy target). When the
// frame job pool (004C1130) answers vtable +10h on its +4 interface true,
// 004F03C0(bag, a, b) defers; otherwise entity vtable +98h(a,
// [[00E188A8]+19CCh], b) places it now.
void scene_unit_place_or_defer_004f04c0(void* entity, void* bag, std::uint32_t a, std::uint32_t b,
                                        SweepAHost& host) {
    auto* pool = static_cast<std::uint8_t*>(host.frame_job_pool_get_singleton_004c1130());
    if (host.vcall_job_pool_defers_10(pool + 4)) {
        host.scene_unit_defer_hierarchy_004f03c0(bag, a, b);
        return;
    }
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    host.vcall_entity_place_98(entity, a, game.at<std::uint32_t>(0x19cc), b);
}

namespace {

// The shared shape of 004F0BE0 / 004F0CC0: the bag's type property (kind 4 or
// 2) names the class; the created entity is placed or deferred by 004F04C0 and
// named by 0048D6C0. `create` stands for the part that differs.
template <class Create>
void* typed_scene_entity_create(const char* name, std::uint32_t a, std::uint32_t b, void* bag,
                                const char* key, Create create, SweepAHost& host) {
    void* const property = host.scene_property_bag_find_008f2260(bag, key);
    if (property == nullptr) return nullptr;
    const std::int32_t kind = static_cast<std::int32_t*>(property)[1];
    if (kind != 4 && kind != 2) return nullptr;
    alignas(4) std::uint8_t type_bytes[8];
    const Raw type_name{type_bytes};
    host.native_string_construct_0041e870(type_name, host.scene_property_get_string_value_008f0df0(property));
    void* const entity = create(type_name);
    release_pooled_data(type_name, host);
    scene_unit_place_or_defer_004f04c0(entity, bag, a, b, host);
    host.scene_entity_set_name_0048d6c0(entity, name);
    return entity;
}

}  // namespace

// 004F0BE0, __fastcall(ECX unused, EDX = const char* name; a, b, property bag,
// unused), RET 10h, EAX = entity or 0; the Stationary (36h) creator, held at
// .text 004F295C by the factory table. 008F2260(bag, "Type" 00CE4780) must give
// a property of kind 4 or 2 (else 0); its string (008F0DF0) becomes a stack
// NativeString (0041E870); class = 00851CB0(&name) and entity = class vtable
// +4(); the string's data is released; 004F04C0(entity, bag, a, b) and
// 0048D6C0(entity, name). A null entity is not tested, as in the image.
void* stationary_create_from_scene_004f0be0(const char* name, std::uint32_t a, std::uint32_t b,
                                            void* bag, std::uint32_t /*unused4*/, SweepAHost& host) {
    return typed_scene_entity_create(name, a, b, bag, "Type",
        [&host](Raw type_name) {
            return host.vcall_class_create_entity_4(host.stationary_class_get_or_create_00851cb0(type_name));
        }, host);
}

// 004F0CC0, the Wreck (3Bh) creator, held at .text 004F2972; the same body as
// 004F0BE0 with the key "WreckType" (00CEA038) and the class step class =
// 004B3F10(&name, 0), entity = 0047BB70([[00E188A8]+21D0h], class).
void* wreck_create_from_scene_004f0cc0(const char* name, std::uint32_t a, std::uint32_t b, void* bag,
                                       std::uint32_t /*unused4*/, SweepAHost& host) {
    return typed_scene_entity_create(name, a, b, bag, "WreckType",
        [&host](Raw type_name) {
            void* const cls = host.wreck_class_get_or_create_activated_004b3f10(type_name, 0);
            const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
            return host.wreck_factory_create_0047bb70(game.ptr(0x21d0), cls);
        }, host);
}

// 004F2FD0, __thiscall(ECX = circle {x, y, r}, const float2* point), RET 4, EAX;
// caller 004F4AEA in 004F49F0. dx, dy are float differences; d2 = dy*dy + dx*dx
// rounded to float (FSTP into the argument slot); returns 1 when r*r (left in
// extended, exact) is above d2 (FCOMIP/JBE: NaN gives 0). The extended sum is
// modelled in double (exact products, double-rounded sum).
bool circle2f_contains_point_004f2fd0(const float* circle, const float* point) {
    // cc13-fixture5: per x87 operation at the main thread's 24-bit precision (the
    // double model differed from the bytes on 64 of 4000 inputs, mostly on the rim).
    const float dx = point[0] - circle[0];   // 004F2FD9, stored
    const float dy = point[1] - circle[1];   // 004F2FE1, stored
    const float d2 = dx * dx + dy * dy;      // 004F2FF4-004F2FFC, stored at 004F3000
    const float r = circle[2];
    return r * r > d2;                       // 004F300A unstored product; FCOMIP / JBE
}

// ============================ segments 12..14 ==============================

// 00518F30, __thiscall(ECX = weapon group state), EAX: 1 when +30h == 1 or
// byte +34h or byte +35h is set, else 0. Caller 0054903D.
std::uint32_t hud_weapon_group_state_is_engaged_00518f30(Raw state) {
    if (state.at<std::int32_t>(0x30) == 1) return 1;
    if (state.at<std::uint8_t>(0x34) != 0) return 1;
    if (state.at<std::uint8_t>(0x35) != 0) return 1;
    return 0;
}

// 00519020, __thiscall(ECX = pilot screen); callers 00519070 and 00519CFE. For
// each unit on the chain from [[screen+1Ch]+48h] along +44h: one answering
// vtable +5Ch(25h) gets vtable +21Ch(2Ah) then +21Ch(2Bh).
void hud_pilot_screen_pulse_attached_kind25_00519020(Raw screen, SweepAHost& host) {
    for (void* unit = Raw{static_cast<std::uint8_t*>(screen.ptr(0x1c))}.ptr(0x48); unit != nullptr;
         unit = Raw{static_cast<std::uint8_t*>(unit)}.ptr(0x44)) {
        if (host.vcall_entity_is_kind_5c(unit, 0x25)) {
            host.vcall_unit_signal_21c(unit, 0x2a);
            host.vcall_unit_signal_21c(unit, 0x2b);
        }
    }
}

// 0051E730, __thiscall(ECX = view, unit), RET 4; callers 00519ACF, 00521617,
// 0064DCAE, 0067C3CC. An old camera [view+40h] is unregistered (006952A0 with
// EDX = view+8); +40h = unit; a non-null unit is registered (00694A60); when
// the pending byte +4Ch is set, the unit's +384h/+388h take +44h/+48h and the
// byte is cleared (a null unit with the byte set faults as in the image).
void hud_view_bind_unit_0051e730(Raw view, void* unit, SweepAHost& host) {
    if (view.ptr(0x40) != nullptr) host.observer_unregister_pair_006952a0(view.ptr(0x40), view.field(8));
    view.ptr(0x40) = unit;
    if (unit != nullptr) host.observer_register_pair_00694a60(unit, view.field(8));
    if (view.at<std::uint8_t>(0x4c) != 0) {
        Raw{static_cast<std::uint8_t*>(view.ptr(0x40))}.at<float>(0x384) = view.at<float>(0x44);
        Raw{static_cast<std::uint8_t*>(view.ptr(0x40))}.at<float>(0x388) = view.at<float>(0x48);
        view.at<std::uint8_t>(0x4c) = 0;
    }
}

// 00519B00, __thiscall(ECX = bomber view, unit), RET 4; caller 0068AFF5.
// 005470C0 releases the weapon-group screen [[00E198C4]+50h]; an old unit
// [view+1Ch] gets 0077C470(1FEh, 0), and its [+9D4h] (when set) 0077C470(4, 0);
// an old unit still registered with the observer view+8 (00694AF0) is
// unregistered (006952A0). +1Ch = unit; 005198F0 ensures the camera; 00549260
// binds the screen to (unit, [view+28h]); a non-null unit not yet registered is
// registered (00694A60); +24h = the unit's float +9F0h (a null unit faults as
// in the image).
void hud_bomber_view_bind_unit_00519b00(Raw view, void* unit, SweepAHost& host) {
    host.hud_weapon_group_screen_release_005470c0(
        Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))}.ptr(0x50));
    if (view.ptr(0x1c) != nullptr) {
        host.unit_post_message_0077c470(view.ptr(0x1c), 0x1fe, 0);
        void* const wing = Raw{static_cast<std::uint8_t*>(view.ptr(0x1c))}.ptr(0x9d4);
        if (wing != nullptr) host.unit_post_message_0077c470(wing, 4, 0);
    }
    if (view.ptr(0x1c) != nullptr && host.observer_is_pair_registered_00694af0(view.ptr(0x1c), view.field(8))) {
        host.observer_unregister_pair_006952a0(view.ptr(0x1c), view.field(8));
    }
    view.ptr(0x1c) = unit;
    host.bomber_view_ensure_camera_005198f0(view);
    host.hud_weapon_group_screen_bind_unit_00549260(
        Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))}.ptr(0x50), view.ptr(0x1c),
        view.at<std::uint32_t>(0x28));
    if (view.ptr(0x1c) != nullptr && !host.observer_is_pair_registered_00694af0(view.ptr(0x1c), view.field(8))) {
        host.observer_register_pair_00694a60(view.ptr(0x1c), view.field(8));
    }
    view.at<float>(0x24) = Raw{static_cast<std::uint8_t*>(view.ptr(0x1c))}.at<float>(0x9f0);
}

// 0051C3A0, ECX unused, (root element, byte flag), RET 8; one call (0051C59F).
// A stack std::vector (first/last/end zeroed) is filled by 006DB3F0(root, &v).
// For every element from index 1 (index 0 is skipped) while the vector's first
// pointer is non-null: vtable +54h reads its float4 colour, alpha (+0Ch) =
// 1.0f, vtable +50h writes it back, then 006DBAC0(element, flag, 0, 0, 1.0f).
// A non-null first pointer is freed (_free 00BF65AC). The unwind action is not
// modelled.
void front_end_screen_reset_descendant_colors_0051c3a0(void* root, std::uint8_t flag, SweepAHost& host) {
    alignas(4) std::uint8_t vector_bytes[0x10] = {};
    const Raw vector{vector_bytes};
    host.gui_element_collect_descendants_006db3f0(root, vector);
    for (std::uint32_t i = 1;; ++i) {
        auto** const first = static_cast<void**>(vector.ptr(4));
        if (first == nullptr) return;
        const auto count = static_cast<std::uint32_t>(
            (static_cast<void**>(vector.ptr(8)) - first));
        if (i >= count) {
            host.free_00bf65ac(first);
            return;
        }
        void* const element = first[i];
        float color[4];
        host.vcall_element_get_color_54(element, color);
        color[3] = 1.0f;
        host.vcall_element_set_color_50(element, color);
        host.gui_element_apply_highlight_pulse_006dbac0(element, flag, 0, 0, 1.0f);
    }
}

// 00521370, __fastcall(ECX = const float3* direction, EDX = float* pitch, float*
// yaw), RET 4; 16 calls. y is clamped to [-1.0 (double 00D7A250), 1.0] (FCOMI:
// NaN passes through), rounded to float, and *pitch = 0042CF10(y); *yaw =
// atan2(x, z) (_CIatan2 00BF701A with x in ST1, z in ST0) rounded to float.
// The double atan2 can differ from the x87 one in the last float bit.
void direction_to_pitch_yaw_00521370(const float* direction, float* pitch, float* yaw,
                                     SweepAHost& host) {
    const double y = direction[1];
    double clamped = y;
    if (-1.0 > y) {
        clamped = -1.0;
    } else if (y > 1.0) {
        clamped = 1.0;
    }
    *pitch = host.geometry_asin_clamped_0042cf10(static_cast<float>(clamped));
    *yaw = static_cast<float>(std::atan2(static_cast<double>(direction[0]), static_cast<double>(direction[2])));
}

// 00521F10, __thiscall(ECX = rows, int index), RET 4, EAX: with e = rows + 4 +
// index*34h, returns e+8 when byte e+10h is set, else e+4. Callers 00527849,
// 0059DA57, 009F827C.
std::uint32_t unit_pick_row_value_00521f10(Raw rows, std::int32_t index) {
    const Raw entry{rows.field(4 + 0x34 * index)};
    return entry.at<std::uint8_t>(0x10) != 0 ? entry.at<std::uint32_t>(8) : entry.at<std::uint32_t>(4);
}

// 00521FD0, __thiscall(ECX = entry, float3* out), RET 4, AL = byte +FCh; out =
// +F0h..+F8h. Caller 00527928.
std::uint8_t unit_pick_read_position_00521fd0(Raw entry, float* out) {
    out[0] = entry.at<float>(0xf0);
    out[1] = entry.at<float>(0xf4);
    out[2] = entry.at<float>(0xf8);
    return entry.at<std::uint8_t>(0xfc);
}

// 00522680, ECX unused, (entity, arg), RET 8, EAX: 00E08F78 when the byte
// [[00E188A8]+19C4h] is set, the entity answers vtable +5Ch(6) and vtable
// +16Ch("attackmove" 00CECCA8, arg) is true; otherwise 00E08EF8. Callers
// 00522F94 and 00527863.
const void* unit_pick_attackmove_color_00522680(void* entity, std::uint32_t arg, SweepAHost& host) {
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    if (game.at<std::uint8_t>(0x19c4) != 0 && host.vcall_entity_is_kind_5c(entity, 6) &&
        host.vcall_entity_accepts_command_16c(entity, "attackmove", arg)) {
        return host.global_address(0x00e08f78);
    }
    return host.global_address(0x00e08ef8);
}

// 00522860, __thiscall(ECX = record, unit), RET 4, EAX = record; callers
// 00526004, 0052642B, 0052653F. record+0 = unit; the unit's and then the
// camera unit's ([[00E188A8]+1ED4h]) poses are refreshed by 00414DB0 when their
// byte +C8h is 0; d = camera.pos - unit.pos (+FCh..+104h, each rounded to
// float); record+4 = (dx*dx + dy*dy) + dz*dz rounded to float once. The x87
// sum is modelled in double (exact products, double-rounded sums).
Raw unit_distance_record_set_00522860(Raw record, void* unit, SweepAHost& host) {
    record.ptr(0) = unit;
    const Raw u{static_cast<std::uint8_t*>(unit)};
    if (u.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00e188a8))};
    const Raw camera{static_cast<std::uint8_t*>(game.ptr(0x1ed4))};
    if (camera.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(camera.p);
    const float dx = camera.at<float>(0xfc) - u.at<float>(0xfc);
    const float dy = camera.at<float>(0x100) - u.at<float>(0x100);
    const float dz = camera.at<float>(0x104) - u.at<float>(0x104);
    const double planar = static_cast<double>(dx) * dx + static_cast<double>(dy) * dy;
    record.at<float>(4) = static_cast<float>(planar + static_cast<double>(dz) * dz);
    return record;
}

// 005251C0, __thiscall(ECX = screen); callers 00647059 and 0064A0DE. +C0h = 0;
// the vector at +C4h (first +C8h, last +CCh) is erased whole by 00524DC0 after
// the two checked-iterator tests (first > last -> 00BF6713); +D4h = -1; when
// +30h is 4, 5 or 6 the input manager (004BEC00) gets 00A933F0(4, 1) then
// (0Eh, 1); +30h = 0.
void hud_screen_reset_selection_005251c0(Raw screen, SweepAHost& host) {
    screen.at<std::uint32_t>(0xc0) = 0;
    const Raw vector{screen.field(0xc4)};
    void* const last = vector.ptr(8);
    if (reinterpret_cast<std::uintptr_t>(vector.ptr(4)) > reinterpret_cast<std::uintptr_t>(last)) {
        host.invalid_parameter_noinfo_00bf6713();
    }
    void* const first = vector.ptr(4);
    if (reinterpret_cast<std::uintptr_t>(first) > reinterpret_cast<std::uintptr_t>(vector.ptr(8))) {
        host.invalid_parameter_noinfo_00bf6713();
    }
    void* ret[2];
    host.vector_erase_range_00524dc0(vector, ret, vector.p, first, vector.p, last);
    screen.at<std::int32_t>(0xd4) = -1;
    const std::int32_t mode = screen.at<std::int32_t>(0x30);
    if (mode == 4 || mode == 5 || mode == 6) {
        host.input_manager_set_context_level_00a933f0(host.input_manager_get_singleton_004bec00(), 4, 1);
        host.input_manager_set_context_level_00a933f0(host.input_manager_get_singleton_004bec00(), 0xe, 1);
    }
    screen.at<std::uint32_t>(0x30) = 0;
}

// 00527C80, __thiscall(ECX = screen, four bytes), RET 10h: +28h..+2Bh = the
// bytes in order, +2Ch = 1. Callers 005F7AF3, 005F7D18, 005F934B.
void control_layout_screen_set_payload_00527c80(Raw screen, std::uint8_t a, std::uint8_t b,
                                                std::uint8_t c, std::uint8_t d) {
    screen.at<std::uint8_t>(0x28) = a;
    screen.at<std::uint8_t>(0x29) = b;
    screen.at<std::uint8_t>(0x2a) = c;
    screen.at<std::uint8_t>(0x2b) = d;
    screen.at<std::uint8_t>(0x2c) = 1;
}

}  // namespace bsp::cc13_sweepA
