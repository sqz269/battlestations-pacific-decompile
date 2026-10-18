// Packet cc13-front7 (marker cc13_front7): wiring-frontier callees, round 2.
// Evidence, ABI and uncertainty per function: docs/CC13_FRONT7.md. Nothing here is wired.
#include "bsp/cc13_front7.hpp"

#include <cstdint>
#include <cstring>

namespace bsp::cc13_front7 {

// ---------------------------------------------------------------------------
// 0078CFF0, __thiscall(ECX = environment; camera, float clock) RET 8 (EDI =
// environment, ESI = camera throughout). Assembly 0078CFF0-0078D1A6:
//  - [env+B0h] set: 00BA40F0(ECX = it; camera, clock).
//  - [env+ACh] set: 00BA2FF0(ECX = it; camera, float [[00E188A8]+64Ch]).
//  - 004FB850(ECX = &argb; env+84h) then 00B6FE50(ECX = camera; argb). The image
//    tests [camera+198h] == 1 || byte [camera+174h] != 0 and both arms make the
//    same two calls (only the register carrying argb differs); argb lives in the
//    camera argument's stack slot.
//  - The camera fog owner [camera+184h], re-read before every call:
//    00B84C40(env+34h); 00B84FA0(env+44h+10h*i, i) for i = 0..3; scalars
//    00B84D00 (+08h), 00B84D10 (+0Ch), 00B84D20 (+10h), 00B84D40 (+14h),
//    00B84D30 (+18h), 00B84D50 (+1Ch); 00B84C70(env+84h); 00B84D60 (+2Ch),
//    00B84D80 (+30h), 00B84DC0 (+20h), 00B84DE0 (+28h), 00B84E00 (+24h).
//    (src/environment_fog_apply.cpp models this interior, 0078D076-0078D180, over
//    typed fog views; this body issues the calls themselves.)
//  - 00BA0090(ECX = [env+A8h]; camera, float(clock * 0.1)): FMUL by the double
//    00D7A3A0 = 3FB99999A0000000, which is float 0.1 widened, so the exact product
//    of two 24-bit significands fits a double and float(double * double) rounds
//    once, as FSTP m32 does at any precision control.
// Float arguments are passed through FLD/FSTP m32 in the image, which would quiet
// a signalling NaN; values are passed through unchanged here.
// ---------------------------------------------------------------------------
void environment_apply_to_camera_0078cff0(Raw environment, void* camera, float clock,
                                          EnvironmentApplyHost& host) {
    const Raw cam{static_cast<std::uint8_t*>(camera)};
    if (void* const particles = environment.ptr(0xB0); particles != nullptr) {
        host.ocean_water_particles_update_00ba40f0(particles, camera, clock);       // 0078D00D
    }
    if (void* const beams = environment.ptr(0xAC); beams != nullptr) {
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00E188A8))};
        host.ocean_beams_update_00ba2ff0(beams, camera, game.at<float>(0x64C));      // 0078D02C
    }
    std::uint32_t argb = 0;
    const float* const ambient = reinterpret_cast<const float*>(environment.field(0x84));
    if (cam.at<std::int32_t>(0x198) == 1 || cam.at<std::uint8_t>(0x174) != 0) {
        host.color_float_rgba_to_argb_004fb850(&argb, ambient);                     // 0078D065
    } else {
        host.color_float_rgba_to_argb_004fb850(&argb, ambient);                     // 0078D04E
    }
    host.camera_set_clear_color_00b6fe50(camera, argb);                             // 0078D071

    auto fog = [&]() { return cam.ptr(0x184); };
    auto scalar = [&](std::size_t off) { return environment.at<float>(off); };
    host.fog_set_color_08_00b84c40(fog(), reinterpret_cast<const float*>(environment.field(0x34)));  // 0078D080
    for (std::int32_t i = 0; i < 4; ++i) {
        host.fog_set_directional_color_28_00b84fa0(
            fog(), reinterpret_cast<const float*>(environment.field(0x44 + 0x10 * i)), i);  // 0078D098
    }
    host.fog_set_scalar_68_00b84d00(fog(), scalar(0x08));                           // 0078D0B5
    host.fog_set_scalar_6c_00b84d10(fog(), scalar(0x0C));                           // 0078D0C7
    host.fog_set_scalar_70_00b84d20(fog(), scalar(0x10));                           // 0078D0D9
    host.fog_set_scalar_78_00b84d40(fog(), scalar(0x14));                           // 0078D0EB
    host.fog_set_scalar_74_00b84d30(fog(), scalar(0x18));                           // 0078D0FD
    host.fog_set_scalar_7c_00b84d50(fog(), scalar(0x1C));                           // 0078D10F
    host.fog_set_underwater_color_18_00b84c70(fog(), ambient);                      // 0078D121
    host.fog_set_scalar_80_00b84d60(fog(), scalar(0x2C));                           // 0078D133
    host.fog_set_scalar_84_00b84d80(fog(), scalar(0x30));                           // 0078D145
    host.fog_set_scalar_88_00b84dc0(fog(), scalar(0x20));                           // 0078D157
    host.fog_set_scalar_8c_00b84de0(fog(), scalar(0x28));                           // 0078D169
    host.fog_set_scalar_90_00b84e00(fog(), scalar(0x24));                           // 0078D17B

    const float scaled = static_cast<float>(static_cast<double>(clock) * static_cast<double>(0.1f));  // 00D7A3A0
    host.ocean_wave_field_update_00ba0090(environment.ptr(0xA8), camera, scaled);  // 0078D19D
}

// ---------------------------------------------------------------------------
// 0061B0F0, __fastcall(ECX = screen) RET; EBP-framed, EH handler 00C77F58.
// Assembly 0061B0F0-0061B839. Two local dword vectors, A (ESP+68h) and B
// (ESP+58h), start empty. The image never writes their +0 dword; it is zero here.
//  1. 008E9AF0(ECX = [00F88C30]; +184h, +144h, +154h, +164h, +174h, 0, 0).
//     slot = [[game+18CCh+4*[game+18ECh]]+28h] (read once).
//  2. For i over the pointer vector V1 at screen+144h (first +148h, last +14Ch;
//     null first or i >= size ends the loop). Each element access is preceded
//     by the inlined checked index (null or i >= size -> 00BF6713):
//     R = [V1[i]+4]. push_back(A, [R+38h*slot+1Ch]) and push_back(B,
//     [R'+38h*slot+20h]), with R re-read. Fast path when first is set and
//     size < capacity; otherwise first > last -> 00BF6713, then 00441F30(&vec;
//     &out, {&vec, last}, &value).
//     M = [[00E198C4]+0C0h], V2 = M+144h, checked i; X = [[V2[i]+4]+38h*slot'+2Ch]
//     (slot' re-read). If X is set: it = 00614060(ECX = [[00E198C4]+0BCh]+42Ch;
//     &tmp, X+70h); check it.container; if it.node == the head saved before the
//     call ([map+4]): icon = 00AB64C0(ECX = [[[00E198C4]+0BCh]+1CCh]; [X+0D4h],
//     &{0, 0, 1.0, 1.0}) and *006171B0(ECX = [[00E198C4]+0BCh]+42Ch; X+70h) = icon.
//     find(map 1B4h, R): node = 00613900(ECX = screen+1B4h; R). The image calls
//     00BF6713 if the map pointer is null (never). end when node == head or R < node
//     key (R as NativeString {len, data} against node+0Ch/+10h: R.len == 0 ->
//     less iff node.len != 0; node.len == 0 -> not less; else _stricmp < 0). The
//     chosen iterator's container is checked. When it is end (head saved before the
//     call) and [R+0D0h] != 0: for k = 0..3 icon = 00AB64C0(ECX = [screen+48h+4k];
//     [R+0D0h], R+0D4h) (two checked indexes per pass); *00617870(ECX = screen+1B4h; R)
//     = last icon; *006170F0(ECX = screen+1C0h; &last icon) = R.
//  3. The same find / four icons / two subscripts for j over V3 at screen+184h with
//     the map at +1CCh, icon lists [screen+68h+4k] and the map at +1D8h (no push_backs,
//     no step-2 int-map arm).
//  4. Copies of B and then A by 004F6210 into the by-value argument area (A first,
//     B second), 0061A400(ECX = screen+100h; A copy, B copy) which owns them;
//     then B's block (operator delete 00BF65AC when set; B zeroed) and A's block.
// ---------------------------------------------------------------------------
namespace {

std::uint8_t* rd_ptr(const void* base, std::size_t off) noexcept {
    std::uint8_t* p;
    std::memcpy(&p, static_cast<const std::uint8_t*>(base) + off, sizeof p);
    return p;
}

// The inlined `vector<T*>::operator[]` check: null first or index >= size.
void check_index(const std::uint8_t* vector_base, std::uint32_t index, PowerupListsHost& host) {
    const Raw v{const_cast<std::uint8_t*>(vector_base)};
    const std::uint8_t* const first = static_cast<std::uint8_t*>(v.ptr(4));
    if (first == nullptr ||
        index >= static_cast<std::uint32_t>(static_cast<std::int32_t>(
                     static_cast<std::uint8_t*>(v.ptr(8)) - first) >> 2)) {
        host.invalid_parameter_00bf6713();
    }
}

// The element pointer V[index] (the records the callers use are [V[index]+4]).
std::uint8_t* element_record(const std::uint8_t* vector_base, std::uint32_t index) {
    return rd_ptr(rd_ptr(vector_base, 4) + index * 4, 0);
}

void push_back(DwordVectorImage& v, const std::uint32_t* value, PowerupListsHost& host) {
    if (v.first != nullptr &&
        static_cast<std::uint32_t>(v.last - v.first) < static_cast<std::uint32_t>(v.end - v.first)) {
        *v.last = *value;
        ++v.last;
        return;
    }
    if (v.first > v.last) host.invalid_parameter_00bf6713();
    MapIterator out{};
    host.vector_insert_one_00441f30(&v, &out, MapIterator{&v, reinterpret_cast<std::uint8_t*>(v.last)}, value);
}

// The inlined case-insensitive map find: lower_bound, then end when node == head
// or key < node key. Returns true when the result is end (the image's skip test
// compares the chosen node with the head read before the lower_bound call).
bool string_map_find_is_end(std::uint8_t* map, const std::uint8_t* key, PowerupListsHost& host) {
    std::uint8_t* const head_before = rd_ptr(map, 4);
    std::uint8_t* const node = host.string_map_ci_lower_bound_00613900(map, key);
    if (map == nullptr) host.invalid_parameter_00bf6713();
    MapIterator chosen{map, node};
    if (node == rd_ptr(map, 4)) {
        chosen = MapIterator{map, rd_ptr(map, 4)};
    } else {
        const Raw k{const_cast<std::uint8_t*>(key)};
        const Raw n{node};
        std::int32_t cmp = 0;
        bool decided = false;
        if (k.at<std::uint32_t>(0) == 0) {
            cmp = n.at<std::uint32_t>(0x0C) != 0 ? -1 : 0;
            decided = true;
        } else if (n.at<std::uint32_t>(0x0C) != 0) {
            cmp = host.stricmp_00bf7fbf(static_cast<const char*>(k.ptr(4)),
                                        static_cast<const char*>(n.ptr(0x10)));
            decided = true;
        }
        if (decided && cmp < 0) chosen = MapIterator{map, rd_ptr(map, 4)};
    }
    if (chosen.container == nullptr || chosen.container != map) host.invalid_parameter_00bf6713();
    return chosen.node == head_before;
}

// The four icon states, the name map entry and the icon -> record entry.
void add_record_icons(Raw screen, const std::uint8_t* vector_base, std::uint32_t index,
                      std::size_t icon_lists, std::size_t name_map, std::size_t icon_map,
                      PowerupListsHost& host) {
    std::uint32_t icon = 0;
    for (int k = 0; k < 4; ++k) {
        check_index(vector_base, index, host);
        std::uint8_t* const record_a = rd_ptr(element_record(vector_base, index), 4);
        check_index(vector_base, index, host);
        std::uint8_t* const record_b = rd_ptr(element_record(vector_base, index), 4);
        icon = host.gui_icon_add_state_00ab64c0(screen.ptr(icon_lists + 4 * k),
                                                Raw{record_b}.at<std::uint32_t>(0xD0), record_a + 0xD4);
    }
    check_index(vector_base, index, host);
    *host.string_map_ci_subscript_00617870(screen.field(name_map),
                                           rd_ptr(element_record(vector_base, index), 4)) = icon;
    check_index(vector_base, index, host);
    std::uint8_t* const record = rd_ptr(element_record(vector_base, index), 4);
    *host.map_subscript_006170f0(screen.field(icon_map), &icon) =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(record));
}

}  // namespace

void powerup_screen_rebuild_lists_0061b0f0(Raw screen, PowerupListsHost& host) {
    host.powerup_manager_collect_active_008e9af0(
        host.global_pointer(0x00F88C30), screen.field(0x184), screen.field(0x144), screen.field(0x154),
        screen.field(0x164), screen.field(0x174), 0, 0);                            // 0061B145
    auto party_slot = [&]() {
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(0x00E188A8))};
        const std::int32_t player = game.at<std::int32_t>(0x18EC);
        return Raw{static_cast<std::uint8_t*>(game.ptr(0x18CC + 4 * player))}.at<std::int32_t>(0x28);
    };
    const std::int32_t slot = party_slot();
    DwordVectorImage a{0, nullptr, nullptr, nullptr};
    DwordVectorImage b{0, nullptr, nullptr, nullptr};

    const std::uint8_t* const v1 = screen.field(0x144);
    for (std::uint32_t i = 0;; ++i) {
        const std::uint8_t* const first = rd_ptr(v1, 4);
        if (first == nullptr ||
            i >= static_cast<std::uint32_t>(static_cast<std::int32_t>(rd_ptr(v1, 8) - first) >> 2)) {
            break;                                                                  // 0061B194 / 0061B1A5
        }
        check_index(v1, i, host);
        const std::uint32_t* const value_a = reinterpret_cast<const std::uint32_t*>(
            rd_ptr(element_record(v1, i), 4) + slot * 0x38 + 0x1C);
        push_back(a, value_a, host);                                                // 0061B235
        check_index(v1, i, host);
        const std::uint32_t* const value_b = reinterpret_cast<const std::uint32_t*>(
            rd_ptr(element_record(v1, i), 4) + slot * 0x38 + 0x20);
        push_back(b, value_b, host);                                                // 0061B2A6

        const Raw ui{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))};
        const std::uint8_t* const v2 = static_cast<std::uint8_t*>(ui.ptr(0xC0)) + 0x144;
        check_index(v2, i, host);
        std::uint8_t* const x = rd_ptr(rd_ptr(element_record(v2, i), 4) + party_slot() * 0x38, 0x2C);
        if (x != nullptr) {
            std::uint8_t* const owner = static_cast<std::uint8_t*>(
                Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))}.ptr(0xBC));
            std::uint8_t* const map = owner + 0x42C;
            std::uint8_t* const head = rd_ptr(owner, 0x430);
            MapIterator tmp{};
            MapIterator* const it = host.int_map_find_00614060(map, &tmp, x + 0x70);  // 0061B335
            if (it->container == nullptr || it->container != map) host.invalid_parameter_00bf6713();
            if (it->node == head) {
                const float state[4] = {0.0f, 0.0f, 1.0f, 1.0f};                     // 00D7A24C = 1.0
                const Raw owner2{static_cast<std::uint8_t*>(
                    Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))}.ptr(0xBC))};
                const std::uint32_t icon = host.gui_icon_add_state_00ab64c0(
                    owner2.ptr(0x1CC), Raw{x}.at<std::uint32_t>(0xD4), state);         // 0061B3A3
                const Raw owner3{static_cast<std::uint8_t*>(
                    Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))}.ptr(0xBC))};
                *host.int_map_subscript_006171b0(owner3.field(0x42C), x + 0x70) = icon;  // 0061B3C2
            }
        }

        check_index(v1, i, host);
        if (string_map_find_is_end(screen.field(0x1B4), rd_ptr(element_record(v1, i), 4), host)) {
            check_index(v1, i, host);
            if (Raw{rd_ptr(element_record(v1, i), 4)}.at<std::uint32_t>(0xD0) != 0) {
                add_record_icons(screen, v1, i, 0x48, 0x1B4, 0x1C0, host);
            }
        }
    }

    const std::uint8_t* const v3 = screen.field(0x184);
    for (std::uint32_t j = 0;; ++j) {
        const std::uint8_t* const first = rd_ptr(v3, 4);
        if (first == nullptr ||
            j >= static_cast<std::uint32_t>(static_cast<std::int32_t>(rd_ptr(v3, 8) - first) >> 2)) {
            break;                                                                  // 0061B5C4 / 0061B5D4
        }
        check_index(v3, j, host);
        if (string_map_find_is_end(screen.field(0x1CC), rd_ptr(element_record(v3, j), 4), host)) {
            check_index(v3, j, host);
            if (Raw{rd_ptr(element_record(v3, j), 4)}.at<std::uint32_t>(0xD0) != 0) {
                add_record_icons(screen, v3, j, 0x68, 0x1CC, 0x1D8, host);
            }
        }
    }

    DwordVectorImage by_value[2]{};                                                 // [0] = A, [1] = B
    host.dword_vector_copy_construct_004f6210(&by_value[1], &b);                    // 0061B7C0
    host.dword_vector_copy_construct_004f6210(&by_value[0], &a);                    // 0061B7DE
    host.powerup_screen_assign_lists_0061a400(screen.field(0x100), by_value);      // 0061B7F1
    if (b.first != nullptr) host.operator_delete_00bf65ac(b.first);                 // 0061B7FF
    b.first = b.last = b.end = nullptr;
    if (a.first != nullptr) host.operator_delete_00bf65ac(a.first);                 // 0061B81C
}

// ---------------------------------------------------------------------------
// 0087CA80, __thiscall(ECX = descriptor; LuaObject* row) RET 4; ESP frame 0F4h
// locals, EH handler 00C96A7D. Assembly 0087CA80-0087D723 (858 instructions),
// cross-checked with docs/NATIVE_DAMAGEABLE_CLASS_LUA_ORCH4.md. In order:
//  - "Name": [desc+54h] = 00438E40(00B662B0(obj)).
//  - "Unique" (kept alive to the end): byte [desc+34h] = 00B66000 && 00B66250.
//  - "Mesh" / "Comment": 00B685C0(obj; &tmp, "") and, when the result is not the
//    destination (desc+38h / desc+58h), 0041DD40(dest; src.len, 1) then
//    memcpy(dest.data, src.data, dest.len) if src.len != 0; tmp released.
//  - "HP": float default 100.0 (00CE3D08) -> [desc+48h]; "Armour": default 0 -> +4Ch.
//  - "ExplosionType": 00B66A60 probe on one object, then a fresh object's
//    00B66290 -> [desc+40h] when the probe held.
//  - "Damage" table -> "Sections" table: for each (key, value) of Sections a 30h
//    record (memset 0, floats +28h/+2Ch = 10.0 from 00CE38B8) is appended by
//    0087C870 to desc+18h, the temporary's destructor is inlined (vtable 00D0DF04,
//    handle +24h released); row = last - 30h after the three checked tests;
//    "MshCategory" copied to a local string; "Index": 00BF7420(00B66270) -> row+8;
//    007149D0(string data or 00F878E0) -> row+4; "FireEfx": 00870CD0(&h,
//    00B66380(obj; 0), 1) assigned to row+24h; when row+24h is still null,
//    00871BA0(&h, "BigFire", 1) is; "FailureChance": default -100.0 (00CE65D8)
//    divided by the double 100.0 (00D7A220) -> row+28h; "FailureDamageThreshold":
//    default -1.0 (00D7A260) -> row+2Ch.
//  - "FakeExplosionEffects" table: entries "0", "1", ... (sprintf "%d") while each
//    is a table: 0087C920(desc+8; n+1, {-1.0, -1, ?, 0}) then elements 1..4 of the
//    entry -> float +0 (00B66270), +8, +4 (00B66290) and the 00870CD0 handle at
//    +0Ch, each behind the checked index n < size.
//  - "Emberkek" table: for each (key, value): 0087B750(desc+60h; &int key)[+40h]
//    = 004B1400(&string value).
// x87: the FailureChance FDIV runs at the main thread's 24-bit precision control
// on a float32 reload, so the correctly rounded float quotient v / 100.0f is the
// stored value; GetNumber's double is rounded once by FSTP m32.
// ---------------------------------------------------------------------------
namespace {

using EffectHandle = void*;

// A float from its image bit pattern (keeps the constants exact).
float f32(std::uint32_t bits) noexcept {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

void release_native_string(NativeStringImage& s, DamageableClassReadHost& host) {
    if (s.data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

// The inlined intrusive-handle release: InterlockedDecrement([h+4]), vtable[0] at zero.
void release_handle(EffectHandle h, DamageableClassReadHost& host) {
    const Raw raw{static_cast<std::uint8_t*>(h)};
    if (host.interlocked_decrement_ce2220(&raw.at<volatile std::int32_t>(4)) == 0) {
        host.effect_handle_vtable0(h);
    }
}

// The inlined intrusive assignment *slot = value (increment new, release old).
void assign_handle(EffectHandle* slot, EffectHandle value, DamageableClassReadHost& host) {
    const EffectHandle old = *slot;
    if (old == value) return;
    *slot = value;
    if (value != nullptr) {
        host.interlocked_increment_ce221c(&Raw{static_cast<std::uint8_t*>(value)}.at<volatile std::int32_t>(4));
    }
    if (old != nullptr) release_handle(old, host);
}

// A temporary handle's destructor followed by the image's explicit zeroing.
void drop_temp_handle(EffectHandle& h, DamageableClassReadHost& host) {
    if (h != nullptr) {
        release_handle(h, host);
        h = nullptr;
    }
}

// Mesh / Comment: string-or-default copied into an existing descriptor string.
void read_string_field(LuaObjectStorage* row, const char* key, NativeStringImage* dest,
                       DamageableClassReadHost& host) {
    LuaObjectStorage obj;
    NativeStringImage tmp;
    LuaObjectStorage* const got = host.lua_object_get_by_name_00b67800(row, &obj, key);
    const NativeStringImage* const src =
        host.lua_object_string_or_default_00b685c0(got, &tmp, static_cast<const char*>(""));
    if (dest != src) {
        host.native_string_resize_0041dd40(dest, src->length, 1);
        if (src->length != 0) host.memcpy_00bf7680(dest->data, src->data, dest->length);
    }
    release_native_string(tmp, host);
    host.lua_object_destruct_00b67700(&obj);
}

// strlen + 0041DD40 + memcpy(len + 1) into a fresh {0, 0} string (inlined twice).
void copy_c_string(NativeStringImage& s, const char* text, DamageableClassReadHost& host) {
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (s.data != nullptr) host.memcpy_00bf7680(s.data, text, s.length + 1);
}

std::int32_t fake_effect_count(const Raw vector) {
    const auto* const first = static_cast<std::uint8_t*>(vector.ptr(4));
    if (first == nullptr) return 0;
    return static_cast<std::int32_t>(static_cast<std::uint8_t*>(vector.ptr(8)) - first) >> 4;
}

// The inlined checked index n < size on the 16-byte vector.
void check_fake_effect_index(const Raw vector, std::int32_t n, DamageableClassReadHost& host) {
    const auto* const first = static_cast<std::uint8_t*>(vector.ptr(4));
    if (first == nullptr ||
        static_cast<std::uint32_t>(n) >=
            static_cast<std::uint32_t>((static_cast<std::uint8_t*>(vector.ptr(8)) - first) >> 4)) {
        host.invalid_parameter_00bf6713();
    }
}

}  // namespace

void damageable_class_read_lua_fields_0087ca80(Raw desc, LuaObjectStorage* row,
                                               DamageableClassReadHost& host) {
    LuaObjectStorage s44, s94;
    {   // "Name"
        LuaObjectStorage* const got = host.lua_object_get_by_name_00b67800(row, &s44, "Name");
        desc.at<char*>(0x54) =
            host.native_string_duplicate_00438e40(host.lua_object_get_string_00b662b0(got));  // 0087CADA
        host.lua_object_destruct_00b67700(&s44);
    }
    host.lua_object_get_by_name_00b67800(row, &s94, "Unique");                            // 0087CAFC
    desc.at<std::uint8_t>(0x34) =
        (host.lua_object_is_boolean_00b66000(&s94) && host.lua_object_get_boolean_00b66250(&s94)) ? 1 : 0;
    read_string_field(row, "Mesh", reinterpret_cast<NativeStringImage*>(desc.field(0x38)), host);     // 0087CB44
    read_string_field(row, "Comment", reinterpret_cast<NativeStringImage*>(desc.field(0x58)), host);  // 0087CBDB
    {
        LuaObjectStorage* got = host.lua_object_get_by_name_00b67800(row, &s44, "HP");
        desc.at<float>(0x48) = host.lua_object_get_float_or_default_00b66330(got, f32(0x42C80000u));  // 0087CC98
        host.lua_object_destruct_00b67700(&s44);
        got = host.lua_object_get_by_name_00b67800(row, &s44, "Armour");
        desc.at<float>(0x4C) = host.lua_object_get_float_or_default_00b66330(got, 0.0f);    // 0087CCCE
        host.lua_object_destruct_00b67700(&s44);
        got = host.lua_object_get_by_name_00b67800(row, &s44, "ExplosionType");
        const bool is_integer = host.lua_object_is_integer_00b66a60(got);                  // 0087CCFD
        host.lua_object_destruct_00b67700(&s44);
        if (is_integer) {
            got = host.lua_object_get_by_name_00b67800(row, &s44, "ExplosionType");
            desc.at<std::int32_t>(0x40) = host.lua_object_get_integer_00b66290(got);        // 0087CD3D
            host.lua_object_destruct_00b67700(&s44);
        }
    }

    host.lua_object_get_by_name_00b67800(row, &s44, "Damage");                            // 0087CD59
    if (host.lua_object_is_table_00b661b0(&s44)) {
        LuaObjectStorage sections;                                                         // ESP+80h
        host.lua_object_get_by_name_00b67800(&s44, &sections, "Sections");
        if (host.lua_object_is_table_00b661b0(&sections)) {
            LuaObjectStorage key, value;                                                   // ESP+58h, ESP+2Ch
            host.lua_object_construct_00b65f50(&key);
            host.lua_object_construct_00b65f50(&value);
            host.lua_object_iterate_first_00b67080(&sections, &key, &value);
            while (!host.lua_object_iteration_ended_00b66420(&sections, &key)) {
                alignas(4) std::uint8_t temp_bytes[0x30];                                  // ESP+A8h
                const Raw temp{temp_bytes};
                host.memset_00bf79f0(temp_bytes, 0, 0x30);                                 // 0087CE0D
                temp.at<float>(0x28) = f32(0x41200000u);                                   // 00CE38B8 10.0
                temp.at<float>(0x2C) = f32(0x41200000u);
                void* const vector = desc.field(0x18);
                host.damageable_section_vector_append_0087c870(vector, temp_bytes);        // 0087CE48
                temp.at<std::uint32_t>(0) = 0x00D0DF04u;                                   // inlined destructor
                drop_temp_handle(temp.at<EffectHandle>(0x24), host);                       // 0087CE6F

                const Raw vec{static_cast<std::uint8_t*>(vector)};
                const auto first = reinterpret_cast<std::uintptr_t>(vec.ptr(4));
                const auto last = reinterpret_cast<std::uintptr_t>(vec.ptr(8));
                if (first > last) host.invalid_parameter_00bf6713();                       // 0087CE90
                const std::uintptr_t back = last - 0x30;
                if (back > reinterpret_cast<std::uintptr_t>(vec.ptr(8)) ||
                    back < reinterpret_cast<std::uintptr_t>(vec.ptr(4))) {
                    host.invalid_parameter_00bf6713();                                     // 0087CEA9
                }
                if (back >= reinterpret_cast<std::uintptr_t>(vec.ptr(8))) host.invalid_parameter_00bf6713();  // 0087CEB6
                const Raw record{reinterpret_cast<std::uint8_t*>(back)};

                NativeStringImage category;                                                // ESP+18h
                LuaObjectStorage field;                                                    // ESP+E0h / ESP+6Ch
                LuaObjectStorage* got = host.lua_object_get_by_name_00b67800(&value, &field, "MshCategory");
                copy_c_string(category, host.lua_object_get_string_00b662b0(got), host);   // 0087CF02
                host.lua_object_destruct_00b67700(&field);

                got = host.lua_object_get_by_name_00b67800(&value, &field, "Index");
                record.at<std::int32_t>(8) = host.ftol_00bf7420(host.lua_object_get_number_00b66270(got));  // 0087CF60
                host.lua_object_destruct_00b67700(&field);

                const char* const category_text =
                    category.data != nullptr ? category.data
                                             : static_cast<const char*>(host.global_address(0x00F878E0));
                record.at<std::int32_t>(4) = host.mesh_category_from_name_007149d0(category_text);  // 0087CF8E

                got = host.lua_object_get_by_name_00b67800(&value, &field, "FireEfx");
                {
                    EffectHandle handle = nullptr;                                         // ESP+28h
                    const std::int32_t id = host.lua_object_get_integer_or_default_00b66380(got, 0);
                    EffectHandle* const out = host.effect_handle_acquire_00870cd0(&handle, id, 1);  // 0087CFAF
                    assign_handle(&record.at<EffectHandle>(0x24), *out, host);
                    drop_temp_handle(handle, host);                                        // 0087CFF0
                }
                host.lua_object_destruct_00b67700(&field);

                if (record.at<EffectHandle>(0x24) == nullptr) {                            // 0087D031
                    NativeStringImage big_fire;                                            // ESP+10h
                    host.native_string_assign_0041e870(&big_fire, "BigFire");
                    EffectHandle handle = nullptr;                                         // ESP+20h
                    EffectHandle* const out =
                        host.effect_handle_acquire_by_name_00871ba0(&handle, &big_fire, 1);  // 0087D063
                    assign_handle(&record.at<EffectHandle>(0x24), *out, host);
                    drop_temp_handle(handle, host);
                    release_native_string(big_fire, host);                                 // 0087D0D4
                }

                got = host.lua_object_get_by_name_00b67800(&value, &field, "FailureChance");
                record.at<float>(0x28) =
                    host.lua_object_get_float_or_default_00b66330(got, f32(0xC2C80000u)) / f32(0x42C80000u);  // 0087D127
                host.lua_object_destruct_00b67700(&field);
                got = host.lua_object_get_by_name_00b67800(&value, &field, "FailureDamageThreshold");
                record.at<float>(0x2C) = host.lua_object_get_float_or_default_00b66330(got, f32(0xBF800000u));  // 0087D16D
                host.lua_object_destruct_00b67700(&field);

                release_native_string(category, host);                                     // 0087D181
                host.lua_object_iterate_next_00b67190(&sections, &key, &value);
            }
            host.lua_object_destruct_00b67700(&value);
            host.lua_object_destruct_00b67700(&key);
        }
        host.lua_object_destruct_00b67700(&sections);                                     // 0087D1F8
    }
    host.lua_object_destruct_00b67700(&s44);                                              // 0087D20C

    LuaObjectStorage fake;                                                                 // ESP+58h
    host.lua_object_get_by_name_00b67800(row, &fake, "FakeExplosionEffects");
    if (host.lua_object_is_table_00b661b0(&fake)) {
        LuaObjectStorage entry;                                                            // ESP+2Ch
        host.lua_object_construct_00b65f50(&entry);
        char name[16];                                                                     // ESP+10h
        std::int32_t i = 0;                                                                // ESP+28h
        auto load_entry = [&]() {
            host.sprintf_int_00bf7a6a(name, "%d", i);
            LuaObjectStorage* const got = host.lua_object_get_by_name_00b67800(&fake, &s44, name);
            host.lua_object_assign_00b67690(&entry, got);
            host.lua_object_destruct_00b67700(&s44);
        };
        load_entry();                                                                      // 0087D26C
        const Raw vector{desc.field(0x08)};
        while (host.lua_object_is_table_00b661b0(&entry)) {
            const std::int32_t n = fake_effect_count(vector);
            host.damageable_fake_effect_vector_resize_0087c920(vector.p, static_cast<std::uint32_t>(n + 1),
                                                                f32(0xBF800000u), -1, 0);  // 0087D2FE
            auto element = [&]() { return Raw{static_cast<std::uint8_t*>(vector.ptr(4)) + 16 * n}; };
            LuaObjectStorage field;                                                        // ESP+6Ch
            LuaObjectStorage* got = host.lua_object_get_by_index_00b67720(&entry, &field, 1);
            check_fake_effect_index(vector, n, host);
            {
                const Raw e = element();
                e.at<float>(0) = static_cast<float>(host.lua_object_get_number_00b66270(got));  // 0087D34A
            }
            host.lua_object_destruct_00b67700(&field);
            got = host.lua_object_get_by_index_00b67720(&entry, &field, 2);
            check_fake_effect_index(vector, n, host);
            {
                const Raw e = element();
                e.at<std::int32_t>(8) = host.lua_object_get_integer_00b66290(got);         // 0087D3A4
            }
            host.lua_object_destruct_00b67700(&field);
            got = host.lua_object_get_by_index_00b67720(&entry, &field, 3);
            check_fake_effect_index(vector, n, host);
            {
                const Raw e = element();
                e.at<std::int32_t>(4) = host.lua_object_get_integer_00b66290(got);         // 0087D3FA
            }
            host.lua_object_destruct_00b67700(&field);
            got = host.lua_object_get_by_index_00b67720(&entry, &field, 4);
            {
                EffectHandle handle = nullptr;                                             // ESP+20h
                const std::int32_t id = host.lua_object_get_integer_00b66290(got);
                EffectHandle* const out = host.effect_handle_acquire_00870cd0(&handle, id, 1);  // 0087D431
                check_fake_effect_index(vector, n, host);
                assign_handle(&element().at<EffectHandle>(0x0C), *out, host);             // 0087D45B
                drop_temp_handle(handle, host);
            }
            host.lua_object_destruct_00b67700(&field);                                     // 0087D4CE
            i = i + 1;
            load_entry();                                                                  // 0087D4E9
        }
        host.lua_object_destruct_00b67700(&entry);                                        // 0087D544
    }
    host.lua_object_destruct_00b67700(&fake);                                             // 0087D55C

    LuaObjectStorage ember;                                                                // ESP+2Ch
    host.lua_object_get_by_name_00b67800(row, &ember, "Emberkek");
    if (host.lua_object_is_table_00b661b0(&ember)) {
        LuaObjectStorage key, value;                                                       // ESP+80h, ESP+58h
        host.lua_object_construct_00b65f50(&key);
        host.lua_object_construct_00b65f50(&value);
        host.lua_object_iterate_first_00b67080(&ember, &key, &value);
        void* const tree = desc.field(0x60);
        while (!host.lua_object_iteration_ended_00b66420(&ember, &key)) {
            NativeStringImage class_name;                                                  // ESP+20h
            copy_c_string(class_name, host.lua_object_get_string_00b662b0(&value), host);  // 0087D618
            const std::int32_t crew_key = host.lua_object_get_integer_00b66290(&key);      // 0087D645, ESP+18h
            void* const soldier_class = host.soldier_class_get_or_load_004b1400(&class_name);  // 0087D652
            std::uint8_t* const mapped = host.damageable_crew_tree_subscript_0087b750(tree, &crew_key);
            Raw{mapped}.at<void*>(0x40) = soldier_class;                                   // 0087D665
            release_native_string(class_name, host);
            host.lua_object_iterate_next_00b67190(&ember, &key, &value);
        }
        host.lua_object_destruct_00b67700(&value);
        host.lua_object_destruct_00b67700(&key);
    }
    host.lua_object_destruct_00b67700(&ember);                                            // 0087D6EF
    host.lua_object_destruct_00b67700(&s94);                                              // 0087D706
}

// ---------------------------------------------------------------------------
// 005DF510, __stdcall(int mode) RET 4 -> EDX:EAX; EBP frame (ESP aligned to 8),
// EH handler 00C74C1D. ECX is overwritten before use. Assembly 005DF510-005E08DF
// (1394 instructions); jump table 005E08E4 (modes 0-3 -> 005DFD26, 4 -> 005DF7D4,
// 5 -> 005DF999, 6 -> 005DFB62, 7 -> 005DF614, above 7 -> 005E03A1).
//  1. VehicleClass = globals([game+1A0Ch])."VehicleClass"; list A allocated
//     (004C3020) and cleared (004D05E0).
//  2. For each 434h-byte entry of the lobby vector ([[00E198AC]+5Ch]+24h, first +28h,
//     last +2Ch; checked iterator re-validated every pass): for both 154h-byte sides,
//     for the mode's dword-pair vectors (side base 18Ch + 10h*k: k = 0 mode 4,
//     k = 1 mode 6, k = 2 mode 5, k = 3 mode 7, k = 4..7 modes 0-3), each element's
//     name (+4, or 00E19605) is atol'd, VehicleClass[n]."Type" is copied into a
//     local string and pushed onto list A. An unknown mode clears list A instead.
//     The container null test runs only in the first vector loop of a side.
//  3. Sort (005DF1A0) and unique (005DA670) list A. For each name in A, every
//     VehicleClass row whose "Type" (00B685C0, default "0") equals it (length,
//     then _stricmp) and whose "UnlockDLCMinimum" is a string pushes that string
//     onto list B.
//  4. Sort and unique B; for each "<prefix>_<n>" mask |= __allshl(1, CL = n - 1)
//     where n = atol of the text after the last '_' (00467CF0 / 00469840).
//  5. Both lists cleared and their sentinels deleted; VehicleClass destroyed.
// ---------------------------------------------------------------------------
namespace {

void release_string(NativeStringImage& s, LobbyUnlockMaskHost& host) {
    if (s.data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

void copy_c_string(NativeStringImage& s, const char* text, LobbyUnlockMaskHost& host) {
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (s.data != nullptr) host.memcpy_00bf7680(s.data, text, s.length + 1);
}

// The inlined push_back: node = 004CE6F0(list; head, head->prev, &value), 004CE780(1),
// head->prev = node, node->prev->next = node.
void list_push_back(StringListImage& list, const NativeStringImage* value, LobbyUnlockMaskHost& host) {
    std::uint8_t* const head = list.head;
    std::uint8_t* const node = host.string_list_allocate_node_004ce6f0(
        &list, head, Raw{head}.at<std::uint8_t*>(4), value);
    host.string_list_grow_count_004ce780(&list, 1);
    Raw{head}.at<std::uint8_t*>(4) = node;
    Raw{Raw{node}.at<std::uint8_t*>(4)}.at<std::uint8_t*>(0) = node;
}

struct LobbyIterator {
    std::uint8_t* container;  // lobby+24h (ESP+10h)
    std::uint8_t* entry;      // ESP+14h
};

// One dword-pair vector loop (for example 005DF626-005DF7B5).
void append_vector_types(StringListImage& list, LuaObjectStorage* vehicle_class, const LobbyIterator& it,
                         const Raw vector, bool null_test, LobbyUnlockMaskHost& host) {
    auto entry_check = [&]() {
        if (reinterpret_cast<std::uintptr_t>(it.entry) >=
            reinterpret_cast<std::uintptr_t>(Raw{it.container}.ptr(8))) {
            host.invalid_parameter_00bf6713();
        }
    };
    auto out_of_range = [&](std::uint32_t i) {
        const auto* const first = static_cast<std::uint8_t*>(vector.ptr(4));
        return first == nullptr ||
               i >= static_cast<std::uint32_t>((static_cast<std::uint8_t*>(vector.ptr(8)) - first) >> 3);
    };
    for (std::uint32_t i = 0;; ++i) {
        if (null_test && it.container == nullptr) host.invalid_parameter_00bf6713();
        entry_check();
        if (out_of_range(i)) break;
        entry_check();
        if (out_of_range(i)) host.invalid_parameter_00bf6713();
        const char* name = Raw{static_cast<std::uint8_t*>(vector.ptr(4)) + 8 * i}.at<const char*>(4);
        if (name == nullptr) name = static_cast<const char*>(host.global_address(0x00E19605));
        const std::int32_t index = host.atol_00bf8417(name);
        LuaObjectStorage row, type;
        LuaObjectStorage* const got_row = host.lua_object_get_by_index_00b67720(vehicle_class, &row, index);
        LuaObjectStorage* const got_type = host.lua_object_get_by_name_00b67800(got_row, &type, "Type");
        NativeStringImage text;
        copy_c_string(text, host.lua_object_get_string_00b662b0(got_type), host);
        list_push_back(list, &text, host);
        release_string(text, host);
        host.lua_object_destruct_00b67700(&type);
        host.lua_object_destruct_00b67700(&row);
    }
}

}  // namespace

std::uint64_t lobby_dlc_unlock_mask_005df510(std::int32_t mode, LobbyUnlockMaskHost& host) {
    LuaObjectStorage globals, vehicle_class;                                               // ESP+148h, ESP+80h
    {
        auto* const game = static_cast<std::uint8_t*>(host.global_pointer(0x00E188A8));
        LuaObjectStorage* const g = host.lua_state_owner_get_globals_00b67980(game + 0x1A0C, &globals);
        host.lua_object_get_by_name_00b67800(g, &vehicle_class, "VehicleClass");           // 005DF569
        host.lua_object_destruct_00b67700(&globals);
    }
    StringListImage list_a{};                                                              // ESP+20h
    list_a.head = host.string_list_allocate_sentinel_004c3020(&list_a);
    list_a.size = 0;
    host.string_list_clear_storage_004d05e0(&list_a);                                      // 005DF59F

    auto lobby = [&]() {
        return Raw{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198AC))}.ptr(0x5C))};
    };
    LobbyIterator it{};
    {
        const Raw l = lobby();
        it.entry = static_cast<std::uint8_t*>(l.ptr(0x28));
        if (reinterpret_cast<std::uintptr_t>(it.entry) > reinterpret_cast<std::uintptr_t>(l.ptr(0x2C))) {
            host.invalid_parameter_00bf6713();                                             // 005DF5B8
        }
        it.container = l.field(0x24);
    }
    for (;;) {
        const Raw l = lobby();
        const auto last = reinterpret_cast<std::uintptr_t>(l.ptr(0x2C));
        if (reinterpret_cast<std::uintptr_t>(l.ptr(0x28)) > last) host.invalid_parameter_00bf6713();
        if (it.container == nullptr || it.container != l.field(0x24)) host.invalid_parameter_00bf6713();
        if (reinterpret_cast<std::uintptr_t>(it.entry) == last) break;                     // 005DF5FB

        auto sides = [&](std::size_t base, std::size_t first_k, std::size_t count_k) {
            for (std::size_t side = 0; side < 2; ++side) {
                for (std::size_t k = 0; k < count_k; ++k) {
                    const Raw vector{it.entry + base + 0x154 * side + 0x10 * (first_k + k)};
                    append_vector_types(list_a, &vehicle_class, it, vector, k == 0, host);
                }
            }
        };
        switch (static_cast<std::uint32_t>(mode)) {
            case 0: case 1: case 2: case 3: sides(0x18C, 4, 4); break;                    // 005DFD26
            case 4: sides(0x18C, 0, 1); break;                                             // 005DF7D4
            case 5: sides(0x18C, 2, 1); break;                                             // 005DF999
            case 6: sides(0x18C, 1, 1); break;                                             // 005DFB62
            case 7: sides(0x18C, 3, 1); break;                                             // 005DF614
            default: host.string_list_clear_storage_004d05e0(&list_a); break;              // 005E03A1
        }
        if (it.container == nullptr) host.invalid_parameter_00bf6713();                     // 005E03AE
        if (reinterpret_cast<std::uintptr_t>(it.entry) >=
            reinterpret_cast<std::uintptr_t>(Raw{it.container}.ptr(8))) {
            host.invalid_parameter_00bf6713();                                             // 005E03BC
        }
        it.entry += 0x434;
    }

    host.string_list_sort_005df1a0(&list_a);                                              // 005E03D2
    host.string_list_unique_005da670(&list_a);
    StringListImage list_b{};                                                              // ESP+54h
    list_b.head = host.string_list_allocate_sentinel_004c3020(&list_b);
    list_b.size = 0;
    host.string_list_clear_storage_004d05e0(&list_b);                                      // 005E03FF

    for (std::uint8_t* node = Raw{list_a.head}.at<std::uint8_t*>(0); node != list_a.head;) {
        LuaObjectStorage key, value;                                                       // ESP+0A8h, ESP+94h
        host.lua_object_construct_00b65f50(&key);
        host.lua_object_construct_00b65f50(&value);
        host.lua_object_iterate_first_00b67080(&vehicle_class, &key, &value);
        while (!host.lua_object_iteration_ended_00b66420(&vehicle_class, &key)) {
            LuaObjectStorage type, unlock;                                                 // ESP+0D0h, ESP+0BCh
            NativeStringImage text;                                                        // ESP+44h
            LuaObjectStorage* const got_type = host.lua_object_get_by_name_00b67800(&value, &type, "Type");
            const NativeStringImage* const s =
                host.lua_object_string_or_default_00b685c0(got_type, &text, "0");          // 005E04B8
            if (node == list_a.head) host.invalid_parameter_00bf6713();                     // 005E04D7
            const Raw entry{node};
            bool matched = false;
            bool unlock_live = false;
            const std::uint32_t node_length = entry.at<std::uint32_t>(8);
            if (s->length == node_length) {
                std::int32_t order;
                if (s->length == 0) {
                    order = node_length != 0 ? -1 : 0;
                } else if (node_length == 0) {
                    order = 1;
                } else {
                    order = host.stricmp_00bf7fbf(s->data, entry.at<const char*>(0x0C));   // 005E0502
                }
                if (order == 0) {
                    LuaObjectStorage* const got = host.lua_object_get_by_name_00b67800(&value, &unlock, "UnlockDLCMinimum");
                    unlock_live = true;
                    matched = host.lua_object_is_string_00b660a0(got);                     // 005E0540
                }
            }
            if (unlock_live) host.lua_object_destruct_00b67700(&unlock);                    // 005E0571
            release_string(text, host);                                                    // 005E0586
            host.lua_object_destruct_00b67700(&type);                                      // 005E05CA
            if (matched) {
                LuaObjectStorage* const got = host.lua_object_get_by_name_00b67800(&value, &unlock, "UnlockDLCMinimum");
                NativeStringImage copy;                                                    // ESP+30h
                copy_c_string(copy, host.lua_object_get_string_00b662b0(got), host);
                list_push_back(list_b, &copy, host);                                       // 005E0661
                release_string(copy, host);
                host.lua_object_destruct_00b67700(&unlock);
            }
            host.lua_object_iterate_next_00b67190(&vehicle_class, &key, &value);
        }
        host.lua_object_destruct_00b67700(&value);
        host.lua_object_destruct_00b67700(&key);
        if (node == list_a.head) host.invalid_parameter_00bf6713();                         // 005E071B
        node = Raw{node}.at<std::uint8_t*>(0);
    }

    std::uint64_t mask = 0;                                                                // ESP+38h
    host.string_list_sort_005df1a0(&list_b);                                              // 005E0735
    host.string_list_unique_005da670(&list_b);
    for (std::uint8_t* node = Raw{list_b.head}.at<std::uint8_t*>(0); node != list_b.head;) {
        NativeStringImage separator{0, nullptr};                                           // ESP+30h
        host.native_string_resize_0041dd40(&separator, 1, 1);                              // 005E0767
        if (separator.data != nullptr) {
            host.memcpy_00bf7680(separator.data, host.global_address(0x00CE7890), separator.length + 1);
        }
        if (node == list_b.head) host.invalid_parameter_00bf6713();                         // 005E079C
        if (node == list_b.head) host.invalid_parameter_00bf6713();                         // 005E07AE
        const auto* const name = reinterpret_cast<const NativeStringImage*>(node + 8);
        const std::int32_t position = host.native_string_reverse_find_00467cf0(name, &separator, 0x7FFFFFFFu);
        NativeStringImage suffix;                                                          // ESP+4Ch
        host.native_string_substring_00469840(name, &suffix, static_cast<std::uint32_t>(position + 1),
                                              0x7FFFFFFFu);                                // 005E07D4
        release_string(separator, host);
        const char* const digits =
            suffix.data != nullptr ? suffix.data : static_cast<const char*>(host.global_address(0x00E19605));
        const std::int32_t n = host.atol_00bf8417(digits);                                 // 005E0806
        mask |= host.allshl_00bf9640(1, static_cast<std::uint8_t>(n - 1));                 // 005E081A
        release_string(suffix, host);
        if (node == list_b.head) host.invalid_parameter_00bf6713();                         // 005E0858
        node = Raw{node}.at<std::uint8_t*>(0);
    }

    host.string_list_clear_storage_004d05e0(&list_b);                                      // 005E0874
    host.operator_delete_00bf65ac(list_b.head);
    list_b.head = nullptr;
    host.string_list_clear_storage_004d05e0(&list_a);                                      // 005E0896
    host.operator_delete_00bf65ac(list_a.head);
    list_a.head = nullptr;
    host.lua_object_destruct_00b67700(&vehicle_class);                                     // 005E08BE
    return mask;
}

// ---------------------------------------------------------------------------
// 00531130, __thiscall(ECX = text; const WideString* source, listener, reference)
// RET 0Ch. Assembly 00531130-005311BC: when source is not text+1A4h, 004C53E0(text+1A4h;
// source.len, 1) and, if source.len != 0, a word copy from source.data to the
// destination data through the terminator; then [+1ACh] = listener, [+1B0h] = reference.
// ---------------------------------------------------------------------------
void text_set_glyph_reference_00531130(Raw text, const WideStringImage* source, std::uint32_t listener,
                                       std::uint32_t reference, SmallBlockersHost& host) {
    auto* const dest = reinterpret_cast<WideStringImage*>(text.field(0x1A4));
    if (dest != source) {
        host.wide_string_resize_004c53e0(dest, source->length, 1);                         // 0053114A
        if (source->length != 0) {
            const std::uint16_t* from = source->data;
            std::uint16_t* to = dest->data;
            std::uint16_t ch;
            do {
                ch = *from++;
                *to++ = ch;
            } while (ch != 0);
        }
    }
    text.at<std::uint32_t>(0x1AC) = listener;
    text.at<std::uint32_t>(0x1B0) = reference;
}

// ---------------------------------------------------------------------------
// 00AB5C60, __thiscall(ECX = widget) RET -> EAX = widget; EH frame 00CB7D28.
// Assembly 00AB5C60-00AB5D29: 00AA9390(widget; 6), then the derived stores in the
// image's order: vtable 00D5C4C0; word +ECh = FFFFh; +F4h/+F8h/+FCh = 0; byte
// +101h = 0; byte +100h = 1; +104h/+108h = 0; byte +10Ch = 0; +110h = 0;
// floats +114h = 1.0 (00D7A24C), +118h = 0, +11Ch = 0, +120h = 1.0, +124h = 1.0;
// word +12Ch = 0; byte +134h = 0; floats +128h = 0, +130h = 0. +F0h is not written.
// ---------------------------------------------------------------------------
void* gui_stateful_image_construct_00ab5c60(Raw w, SmallBlockersHost& host) {
    host.ui_context_construct_00aa9390(w.p, 6);                                            // 00AB5C7B
    const float one = f32(0x3F800000u);
    w.at<std::uint32_t>(0) = 0x00D5C4C0u;
    w.at<std::uint16_t>(0xEC) = 0xFFFFu;
    w.at<std::uint32_t>(0xF4) = 0;
    w.at<std::uint32_t>(0xF8) = 0;
    w.at<std::uint32_t>(0xFC) = 0;
    w.at<std::uint8_t>(0x101) = 0;
    w.at<std::uint8_t>(0x100) = 1;
    w.at<std::uint32_t>(0x104) = 0;
    w.at<std::uint32_t>(0x108) = 0;
    w.at<std::uint8_t>(0x10C) = 0;
    w.at<std::uint32_t>(0x110) = 0;
    w.at<float>(0x114) = one;
    w.at<float>(0x118) = 0.0f;
    w.at<float>(0x11C) = 0.0f;
    w.at<float>(0x120) = one;
    w.at<float>(0x124) = one;
    w.at<std::uint16_t>(0x12C) = 0;
    w.at<std::uint8_t>(0x134) = 0;
    w.at<float>(0x128) = 0.0f;
    w.at<float>(0x130) = 0.0f;
    return w.p;
}

// ---------------------------------------------------------------------------
// 00798080, __thiscall(ECX = track; keyframe) RET 4. Assembly 00798080-0079812C.
// Nothing when keyframe is null or its pair id (short +DAh) is already >= 0.
// Otherwise walks the track's list (container track+68h, head [+6Ch]; checked
// iterator: container null/identity test each pass, end-dereference test) for the
// first other keyframe ([node+8]) whose pair id is negative and whose float +F0h
// equals keyframe+F0h (FUCOMIP, unordered is not equal); both take the id
// movzx word [[track+74h]+3C0h] and the dword there is incremented.
// ---------------------------------------------------------------------------
void movie_camera_track_pair_keyframe_00798080(Raw track, void* keyframe, SmallBlockersHost& host) {
    if (keyframe == nullptr) return;
    const Raw key{static_cast<std::uint8_t*>(keyframe)};
    if (key.at<std::int16_t>(0xDA) >= 0) return;
    std::uint8_t* const container = track.field(0x68);
    auto* node = Raw{static_cast<std::uint8_t*>(track.ptr(0x6C))}.at<std::uint8_t*>(0);   // 007980A5
    for (;;) {
        const Raw list{container};
        std::uint8_t* const head = static_cast<std::uint8_t*>(list.ptr(4));
        if (container == nullptr) host.invalid_parameter_00bf6713();                       // 007980B3
        if (node == head) return;                                                          // 007980BA
        if (container == nullptr) host.invalid_parameter_00bf6713();
        if (node == static_cast<std::uint8_t*>(list.ptr(4))) host.invalid_parameter_00bf6713();  // 007980CA
        const Raw other{static_cast<std::uint8_t*>(Raw{node}.ptr(8))};
        if (other.at<std::int16_t>(0xDA) < 0 && other.at<float>(0xF0) == key.at<float>(0xF0)) {  // 007980EE
            const Raw owner{static_cast<std::uint8_t*>(track.ptr(0x74))};
            const std::uint16_t id = owner.at<std::uint16_t>(0x3C0);
            key.at<std::uint16_t>(0xDA) = id;
            other.at<std::uint16_t>(0xDA) = id;
            Raw{static_cast<std::uint8_t*>(track.ptr(0x74))}.at<std::uint32_t>(0x3C0) += 1;   // 00798121
            return;
        }
        if (node == static_cast<std::uint8_t*>(list.ptr(4))) host.invalid_parameter_00bf6713();  // 007980FD
        node = Raw{node}.at<std::uint8_t*>(0);
    }
}

}  // namespace bsp::cc13_front7
