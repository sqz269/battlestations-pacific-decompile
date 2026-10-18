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

}  // namespace bsp::cc13_front7
