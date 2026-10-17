// cc13_big5ad: 005ADDA0, the HUD map per-icon update, as a whole body (packet
// cc13-big5ad, docs/CC13_BIG5AD.md). Names are hypotheses; see the header for
// the hooks.
//
// x87 note: the HUD runs on the main thread, which runs with 24-bit precision
// control (docs/CC13_FPU_PRECISION.md), so each x87 arithmetic step is
// modelled as a float operation in the image's order. Register-register x87
// operand directions were read from the bytes (capstone over the PE), not the
// Ghidra listing text. Frame slots are named by their entry-relative offset
// (fNNN = [entry ESP - NNNh]); ESP was tracked through the whole body and
// checked against every EH-state store (all land on [entry - 4]). EH frames
// are noted, not modelled.

#include "bsp/cc13_big5ad.hpp"

#include <cstring>

namespace bsp {

namespace {

using Str = Cc13SweepBNativeString;
using Byte = std::uint8_t;

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_sweepB_at<T>(base, offset);
}

void* ptr(void* base, std::size_t offset) {
    return at<void*>(base, offset);
}

Byte* addr(void* base, std::size_t offset) {
    return static_cast<Byte*>(base) + offset;
}

float* fp(void* base, std::size_t offset) {
    return reinterpret_cast<float*>(addr(base, offset));
}

void copy4(float* out, const void* source) {
    std::memcpy(out, source, 4 * sizeof(float));
}

// [[00E188A8]+18CCh+[[00E188A8]+18ECh]*4]: the local player's record.
void* local_player_record(void* game) {
    return ptr(game, 0x18CC + 4 * at<std::int32_t>(game, 0x18EC));
}

// The unit "is a plain unit" test the image inlines: +5Ch set and +5Dh, +60h,
// +5Eh clear.
bool unit_plain(void* unit) {
    return at<Byte>(unit, 0x5C) != 0 && at<Byte>(unit, 0x5D) == 0 && at<Byte>(unit, 0x60) == 0 &&
           at<Byte>(unit, 0x5E) == 0;
}

// The 4x4 scale matrix the image builds on the stack at [entry-DCh] before
// 00413920: diag(x, y, z, 1.0f) with every other element 0.
void scale_matrix(float* m, float x, float y, float z) {
    for (int i = 0; i < 16; ++i) {
        m[i] = 0.0f;
    }
    m[0] = x;
    m[5] = y;
    m[10] = z;
    m[15] = 1.0f;  // 00D7A24C
}

// The std::vector<void*> the icon embeds at several offsets: {proxy, begin
// +4h, end +8h, capacity +0Ch}. Size as the image computes it: 0 when begin is
// null, else (end - begin) >> 2.
std::uint32_t vec_size(void* vector) {
    Byte* const begin = static_cast<Byte*>(ptr(vector, 4));
    if (begin == nullptr) {
        return 0;
    }
    return static_cast<std::uint32_t>((static_cast<Byte*>(ptr(vector, 8)) - begin) >> 2);
}

// The inlined checked subscript: begin null or index >= size calls 00BF6713;
// the element address is formed either way.
void** vec_checked_at(void* vector, std::uint32_t index, Cc13Big5adHost& host) {
    if (ptr(vector, 4) == nullptr || vec_size(vector) <= index) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    return static_cast<void**>(ptr(vector, 4)) + index;
}

// The +E8h widget vector of the info panel (begin +ECh, end +F0h).
void** panel(void* icon, std::uint32_t index, Cc13Big5adHost& host) {
    return vec_checked_at(addr(icon, 0xE8), index, host);
}

// The inlined "delete every widget, then erase(begin(), end())" the image
// repeats for +78h, +88h, +98h and +A8h: each element through its vtable
// +4h(1) and nulled; then the checked iterator tests and the erase body with
// _memmove_s of the (empty) tail.
void delete_all_and_erase(void* vector, Cc13Big5adHost& host) {
    for (std::uint32_t i = 0; ptr(vector, 4) != nullptr && i < vec_size(vector); ++i) {
        void** const slot = vec_checked_at(vector, i, host);
        if (*slot != nullptr) {
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
    }
    Byte* const last = static_cast<Byte*>(ptr(vector, 8));
    if (static_cast<Byte*>(ptr(vector, 4)) > last) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    Byte* const first = static_cast<Byte*>(ptr(vector, 4));
    if (first > static_cast<Byte*>(ptr(vector, 8))) {
        host.crt_invalid_parameter_noinfo_00bf6713();
    }
    if (first != last) {
        const std::int32_t count = static_cast<std::int32_t>((static_cast<Byte*>(ptr(vector, 8)) - last) >> 2);
        const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4;
        if (count > 0) {
            host.crt_memmove_s_00bf67a7(first, bytes, last, bytes);
        }
        at<void*>(vector, 8) = first + bytes;
    }
}

// 0059D150 with the four status bytes in the order the caller lists them.
void hud_map_icon_query_unit_status(void* icon, Byte* flags, Cc13Big5adHost& host) {
    host.hud_map_icon_query_unit_status_0059d150(icon, &flags[0], &flags[1], &flags[2], &flags[3]);
}

// The inlined native-string release: data goes back to the sized pool with
// length + 1 bytes (pool fetched after the arguments are formed).
void release(Str& text, Cc13Big5adHost& host) {
    if (text.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), text.data, text.length + 1, 1);
    }
}

constexpr float kMapIconZ = 110.0f;               // 00CEFFFC
constexpr double kMapIconOffset = double(0.01f);  // 00D7A358
constexpr double kHalf = 0.5;                     // 00D7A280
constexpr double kModelLift = 1500.0;             // 00CF00F8
constexpr double kForwardEpsilon = double(1.0e-4f);  // 00D7A268

}  // namespace

// 005ADDA0 BSP_HudMapScreen_UpdateUnitIcon. __thiscall(ECX = screen; icon),
// RET 4. EBX holds the unit (icon+4h), ESI the icon, EBP the screen except
// where noted. Frame 194h + EH registration; the EH state slot is [entry-4].
void cc13_big5ad_hud_map_screen_update_unit_icon_005adda0(void* screen, void* icon, Cc13Big5adHost& host) {
    void* game = host.global_00e188a8();
    // 005ADDC8..005ADDF3
    if (at<std::int32_t>(game, 0x1FE4) == 0 || at<Byte>(game, 0x218C) != 0) {
        if (at<Byte>(game, 0x61F) != 0 || at<Byte>(game, 0x620) != 0) {
            return;
        }
    }

    void* const unit = ptr(icon, 4);
    float f130[3];
    host.unit_vcall_c4_position(unit, f130);  // 005ADE18; the returned pointer is not read here

    Byte f18f = 0;  // [entry-18Fh]: several booleans over the body
    Byte f111 = 0;  // [entry-111h]: the +29h visibility changed this update
    // [entry-18Dh]: the image leaves it unset on some paths (stale stack); every
    // later reader either sets it first or passes it to 0059D150, which clears it.
    Byte f18d = 0;
    // ---- 005ADE1A..005AE15B: the owner-name label (+18h) and the voice icon (+1Ch) ----
    if (at<std::int32_t>(host.global_00e188a8(), 0x1FE4) != 0) {
        void* owner = nullptr;  // EDI
        if (unit_plain(unit)) {
            if (host.entity_vcall_5c_is_kind(unit, 0x18)) {
                owner = ptr(unit, 0x3D0);
            } else {
                owner = host.entity_vcall_5c_is_kind(unit, 1) ? unit : nullptr;
            }
            // 005ADE6D: no null test on owner before the virtual call.
            if (host.entity_vcall_5c_is_kind(owner, 0xF)) {
                if (ptr(ptr(owner, 0x9D4), 0x3D0) != owner) {
                    owner = nullptr;
                }
            }
        }
        void* game2 = host.global_00e188a8();
        const std::uint32_t relation = host.recon_relation_if_identified_00803ce0(
            ptr(local_player_record(game2), 0x28), unit);  // 005ADEA5
        f18f = relation != 3 ? 1 : 0;

        bool show = false;
        if (owner != nullptr) {
            const std::int32_t slot = at<std::int32_t>(owner, 0x1AC);
            if (slot != 8 && !host.party_slot_is_ai_held_00927f10(slot)) {
                const std::int32_t local = at<std::int32_t>(host.global_00e188a8(), 0x18EC);
                const bool is_local = local >= 0 && local <= 7 && at<std::int32_t>(owner, 0x1AC) == local;
                show = !is_local && f18f != 0;
            }
        }
        if (show) {
            // 005ADF05
            if (ptr(icon, 0x18) == nullptr) {
                void* label = host.widget_clone_subtree_00aab4c0(ptr(screen, 0x98), nullptr);
                at<void*>(icon, 0x18) = label;
                host.widget_vcall_34_set_visible(label, 1);
            }
            void* record = ptr(host.global_00e188a8(), 0x18CC + 4 * at<std::int32_t>(owner, 0x1AC));  // EBP
            const char* name = reinterpret_cast<const char*>(addr(record, 0x58));
            host.gui_text_set_localised_cstring_00abbe50(
                ptr(icon, 0x18), name, static_cast<const void*>(name) == host.image_global(0x00CEF794) ? 1 : 0);
            // 005ADF4E..005ADFA7
            const float x = static_cast<float>(
                static_cast<double>(host.hud_map_world_to_map_x_0059a840(screen, f130[0])) + kMapIconOffset);
            const float y = host.hud_map_world_to_map_y_0059a860(screen, f130[2]);
            float f17c[3] = {x, y, kMapIconZ};
            host.gui_widget_set_local_position_00aa7dc0(ptr(icon, 0x18), f17c);
            // 005ADFAC
            if (!host.game_has_local_player_slot_004b6bc0(host.global_00e188a8())) {
                host.widget_vcall_50_colour(ptr(icon, 0x18), host.color_static_white_0059adc0());
            } else {
                Byte* config = static_cast<Byte*>(host.global_config_00432650());
                host.widget_vcall_50_colour(ptr(icon, 0x18),
                                            reinterpret_cast<const float*>(
                                                config + (at<std::int32_t>(owner, 0x54) == 0 ? 0x108 : 0x128)));
            }
            // 005ADFF9: the talking icon for a player on the local player's team.
            void* local_record = local_player_record(host.global_00e188a8());
            bool talking = false;
            if (at<std::int32_t>(record, 0x28) == at<std::int32_t>(local_record, 0x28) &&
                at<Byte>(record, 0x89) != 0 && host.player_record_is_talking_007f74b0(record)) {
                void* game3 = host.global_00e188a8();
                void* local3 = local_player_record(game3);
                talking = !host.voice_mute_is_muted_one_way_00778740(
                    addr(game3, 0x2080), at<std::uint32_t>(local3, 0x80), at<std::uint32_t>(local3, 0x84),
                    at<std::uint32_t>(record, 0x80), at<std::uint32_t>(record, 0x84));
            }
            if (talking) {
                // 005AE075
                if (ptr(icon, 0x1C) == nullptr) {
                    void* voice = host.widget_clone_subtree_00aab4c0(ptr(screen, 0x9C), nullptr);
                    at<void*>(icon, 0x1C) = voice;
                    host.widget_vcall_34_set_visible(voice, 1);
                }
                const float vx = static_cast<float>(
                    static_cast<double>(host.hud_map_world_to_map_x_0059a840(screen, f130[0])) + kMapIconOffset);
                const float vy = static_cast<float>(
                    static_cast<double>(host.hud_map_world_to_map_y_0059a860(screen, f130[2])) + kMapIconOffset);
                float v17c[3] = {vx, vy, kMapIconZ};
                host.gui_widget_set_local_position_00aa7dc0(ptr(icon, 0x1C), v17c);
                // 005AE0FA: screen+5CCh * 0.5
                const float scale = static_cast<float>(static_cast<double>(at<float>(screen, 0x5CC)) * kHalf);
                host.widget_set_uniform_scale_004fb9e0(ptr(icon, 0x1C), scale);
            } else {
                // 005AE120
                host.owned_ptr_delete_reset_004f9950(reinterpret_cast<void**>(addr(icon, 0x1C)));
            }
        } else {
            // 005AE12E: neither label applies; both widgets are deleted.
            if (ptr(icon, 0x18) != nullptr) {
                host.object_vcall_04_delete(ptr(icon, 0x18), 1);
                at<void*>(icon, 0x18) = nullptr;
            }
            if (ptr(icon, 0x1C) != nullptr) {
                host.object_vcall_04_delete(ptr(icon, 0x1C), 1);
                at<void*>(icon, 0x1C) = nullptr;
            }
        }
    }

    // ---- 005AE15C..005AEA41: the 3D map models (+124h, +128h) and the icon colour ----
    Byte f18e = 0;
    Byte f190 = 0;
    float f194 = 0.0f;
    if (ptr(icon, 0x124) != nullptr) {
        // 005AE171: COMISS 1.5f, screen+5D8h; JA (ordered greater).
        f190 = (ptr(icon, 0x128) != nullptr && 1.5f > at<float>(screen, 0x5D8)) ? 1 : 0;  // 00CE380C
        float f8c[16];
        float f4c[16];
        float fdc[16];
        void* const unit_matrix = addr(unit, 0xCC);
        if (at<Byte>(icon, 0x2E) != 0) {
            if (at<Byte>(unit, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(unit);
            }
            const float s = at<float>(screen, 0x5E4);
            const float twice = s + s;  // FADD ST0,ST0
            scale_matrix(fdc, twice, s, s);
        } else {
            if (at<Byte>(unit, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(unit);
            }
            const float s = at<float>(screen, 0x5E4);
            scale_matrix(fdc, s, s, s);
        }
        // 005AE2BD
        host.matrix_copy_4x4_004134f0(f8c, host.matrix_multiply_00413920(fdc, f4c, static_cast<float*>(unit_matrix)));
        float f150[3];
        const float* position = host.unit_vcall_c4_position(unit, f150);  // 005AE2DE
        f8c[12] = position[0];
        f8c[13] = position[1];
        f8c[14] = position[2];
        f8c[13] = static_cast<float>(static_cast<double>(f8c[13]) + kModelLift);
        host.node_vcall_34_set_matrix(ptr(icon, 0x124), f8c);

        if (f190 != 0) {
            // 005AE339: the second model is laid flat, facing the unit's heading.
            if (at<Byte>(unit, 0xC8) == 0) {
                host.entity_pose_refresh_world_00414db0(unit);
            }
            host.matrix_copy_4x4_004134f0(f8c, static_cast<float*>(unit_matrix));
            const float a = f8c[8];
            const float c = f8c[10];
            f8c[9] = 0.0f;
            // 005AE367: (a*a + 0*0) + c*c, each step rounded to float.
            float length2 = a * a;
            length2 = length2 + 0.0f * 0.0f;
            length2 = length2 + c * c;
            // FCOMPI 1.0e-4 (double), sum; JBE skips: taken when ordered greater.
            if (kForwardEpsilon > static_cast<double>(length2)) {
                f8c[8] = *static_cast<float*>(host.image_global(0x00F87598));
                f8c[9] = *static_cast<float*>(host.image_global(0x00F8759C));
                f8c[10] = *static_cast<float*>(host.image_global(0x00F875A0));
            }
            f8c[4] = *static_cast<float*>(host.image_global(0x00F8758C));
            f8c[5] = *static_cast<float*>(host.image_global(0x00F87590));
            f8c[6] = *static_cast<float*>(host.image_global(0x00F87594));
            host.matrix_orthonormalize_rows_0085dc80(f8c);
            const float s = at<float>(screen, 0x5E4);
            scale_matrix(fdc, s, s, s);
            host.matrix_copy_4x4_004134f0(f8c, host.matrix_multiply_00413920(fdc, f4c, f8c));
            f8c[13] = static_cast<float>(static_cast<double>(f8c[13]) + kModelLift);
            host.node_vcall_34_set_matrix(ptr(icon, 0x128), f8c);
        }

        // 005AE4FF: the visibility factor (1.0, or the blink pair while +8h != +9h).
        f194 = 1.0f;
        if (at<float>(screen, 0x5AC) > 0.0f) {  // COMISS; JBE skips (ordered greater enters)
            const Byte state = at<Byte>(icon, 8);
            if (at<Byte>(icon, 9) != state) {
                f194 = state != 0 ? at<float>(screen, 0x5D4) : at<float>(screen, 0x5D0);
            }
        }
        if (ptr(icon, 0x128) != nullptr) {
            const float first = f190 != 0 ? 0.0f : f194;
            host.scene_node_set_visibility_factor_00b6da70(ptr(icon, 0x124), first, 0);
            f194 = f190 != 0 ? f194 : 0.0f;
            host.scene_node_set_visibility_factor_00b6da70(ptr(icon, 0x128), f194, 0);
        } else {
            host.scene_node_set_visibility_factor_00b6da70(ptr(icon, 0x124), f194, 0);
        }

        // 005AE5B0: the icon colour.
        Byte* const config = static_cast<Byte*>(host.global_config_00432650());
        Byte* palette = config + 0x108;  // EDI
        void* const commander = host.unit_vcall_140(unit);
        void* const f194_commander = commander;
        if (commander != nullptr && commander == host.global_00e188d8()) {
            host.hud_map_icon_set_color_0059a9b0(icon, reinterpret_cast<const float*>(palette + 0x50));
            f18e = 1;
        } else if (!unit_plain(unit)) {
            host.hud_map_icon_set_color_0059a9b0(icon, reinterpret_cast<const float*>(palette + 0x90));
        } else {
            // 005AE612
            void* const local_party = ptr(local_player_record(host.global_00e188a8()), 0x28);  // EBP
            std::uint32_t f184 = at<std::uint32_t>(unit, 0x54);
            if (host.recon_relation_if_identified_00803ce0(local_party, unit) == 3) {
                f184 = 2;
            }
            f18d = 0;
            float f150c[4];
            if (host.entity_vcall_5c_is_kind(unit, 0x1C)) {
                // FUCOMPI against 0, LAHF/TEST AH,44h/JNP: skipped only when ordered equal.
                const float fill = host.command_building_capture_ring_fill_006f1f90(unit);
                if (!(fill == 0.0f)) {
                    f18d = 1;
                    const std::int32_t side = at<std::int32_t>(unit, 0x7AC);
                    copy4(f150c, palette + (side == 0 ? 0x00 : side == 1 ? 0x20 : 0x40));
                }
            }
            // [entry-124h..-118h]: on the paths below that write nothing the image
            // passes stale stack bytes to 0059A9B0; zeroed here.
            float f124[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            f190 = 0;
            if (host.entity_vcall_5c_is_kind(unit, 0x46) || host.entity_vcall_5c_is_kind(unit, 0x45)) {
                // 005AE703: the "InferiorFailure" (00CF0B74) flag query.
                Str f170{0, nullptr};
                host.native_string_assign_0041e870(&f170, static_cast<const char*>(host.image_global(0x00CF0B74)));
                const bool found = host.unit_vcall_198_query_result(unit, &f170, 0, &f190);  // 005AE73A
                f18f = (found && f190 == 1) ? 1 : 0;
                host.native_string_destroy_storage_0041dd20(&f170);
                if (f18f != 0) {
                    copy4(f124, palette + 0x40);
                }
            }
            if (f184 == 2) {
                // 005AE79B
                copy4(f124, palette + 0x40);
            } else if (f194_commander != nullptr && f190 == 0) {
                // 005AE7DE
                const bool selectable =
                    f184 == reinterpret_cast<std::uintptr_t>(local_party) &&
                    host.unit_selectable_00645160(f194_commander, 0);
                const Byte* source;
                if (f184 == 0) {
                    if (local_party != nullptr) {
                        source = palette;  // 005AE86F with EDI = +108h
                    } else {
                        source = selectable ? palette : palette + 0x10;
                    }
                } else if (f184 != reinterpret_cast<std::uintptr_t>(local_party)) {
                    source = palette + 0x20;
                } else {
                    source = selectable ? palette + 0x20 : palette + 0x30;
                }
                copy4(f124, source);
            }
            // 005AE896
            if (f18d != 0 && host.recon_relation_if_identified_00803ce0(local_party, unit) != 3) {
                if (f184 == 2) {
                    // 005AE8C2: white, through the [entry-104h..-F8h] temporaries.
                    f124[0] = 1.0f;
                    f124[1] = 1.0f;
                    f124[2] = 1.0f;
                    f124[3] = 1.0f;
                }
                // 005AE91E: blend the capture colour toward the side colour by
                // screen+84h: out = c - (c - capture) * t, rounded per step.
                const float t = at<float>(screen, 0x84);
                const float dr = f124[0] - f150c[0];
                const float db = f124[2] - f150c[2];
                const float dg = f124[1] - f150c[1];
                const float r = f124[0] - dr * t;
                const float g = f124[1] - dg * t;
                const float b = f124[2] - db * t;
                f150c[0] = r;
                f150c[1] = g;
                f150c[2] = b;
                f150c[3] = 1.0f;  // 005AE933 (00D7A24C)
                host.hud_map_icon_set_color_0059a9b0(icon, f150c);
            } else {
                host.hud_map_icon_set_color_0059a9b0(icon, f124);
            }
        }

        // 005AE9BF: the fuel/ammo fill of a kind-6 unit (+980h clamped to 0..1).
        f194 = 0.0f;
        if (host.entity_vcall_5c_is_kind(unit, 6)) {
            f194 = at<float>(unit, 0x980);
            float high = 1.0f;
            float low = 0.0f;
            host.math_clamp_in_place_00415690(&f194, &low, &high);
        }
        // 005AEA0E
        void* const texture = ptr(screen, 0x4A4 + 4 * at<std::int32_t>(screen, 0x4B0));
        if (ptr(icon, 0x13C) != nullptr) {
            host.material_set_texture_slot_00b189f0(ptr(icon, 0x13C), 0, texture);
            at<float>(icon, 0x140) = f194;
        }
    }

    // ---- 005AEA42..005AEBC2: the seven trail markers +48h..+60h ----
    // Marker k shows from icon+74h + k*0.5 s on: it is placed at the unit
    // (y from the world z), grown by 1.01 and faded by 0.9 each update, and
    // deleted once its alpha is at or below 0.01.
    for (std::int32_t k = 0; k < 7; ++k) {
        void** const slot = reinterpret_cast<void**>(addr(icon, 0x48 + 4 * k));  // EDI
        if (*slot == nullptr) {
            continue;
        }
        void* const marker = *slot;
        const float since = host.global_00f876a4() - static_cast<float>(k * 0.5);  // FILD, FMUL 0.5, FSUBP
        // FCOMPI (since, icon+74h); JC hides: below or unordered.
        if (!(since >= at<float>(icon, 0x74))) {
            host.widget_vcall_34_set_visible(marker, 0);
            continue;
        }
        host.widget_vcall_34_set_visible(marker, 1);
        float f150[3];
        const float z = host.unit_vcall_c4_position(unit, f150)[2];
        const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
        const float my = (at<float>(screen, 0x550) - z) / at<float>(screen, 0x34) + g_y;
        float f104[3];
        const float wx = host.unit_vcall_c4_position(unit, f104)[0];  // 005AEAC8
        const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
        float f17c[3];
        f17c[0] = (wx - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
        f17c[1] = my;
        f17c[2] = -90.0f;  // 00CF0B70
        host.widget_set_resolved_position_00aa8240(*slot, f17c);
        // 005AEB0D: size * 1.01 (double 00CF0B68 = 1.01f widened)
        void* const grown = *slot;
        const double kGrow = double(1.01f);
        float f158[2];
        f158[0] = static_cast<float>(static_cast<double>(at<float>(grown, 0x28)) * kGrow);
        f158[1] = static_cast<float>(kGrow * static_cast<double>(at<float>(grown, 0x2C)));
        host.widget_vcall_48_set_pair(grown, f158);
        // 005AEB53: alpha * 0.9 (double 00D7A390 = 0.9f widened)
        float f9c[4];
        const float alpha = static_cast<float>(
            static_cast<double>(host.widget_vcall_54_colour(*slot, f9c)[3]) * double(0.9f));
        host.widget_vcall_4c_set_alpha(*slot, alpha);
        // FCOMPI (0.01f, alpha); JC keeps: below or unordered.
        if (!(0.01f < alpha) && *slot != nullptr) {
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
    }

    // ---- 005AEBC8: on-map test of the unit position ----
    at<Byte>(icon, 0x2B) = at<Byte>(icon, 0x2A);
    {
        const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
        const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
        float f13c[3];
        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
        f13c[2] = 0.0f;
        f13c[0] = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
        f13c[1] = my;
        at<Byte>(icon, 0x2A) = host.hud_map_is_point_inside_map_area_0059d410(f13c) ? 1 : 0;
    }

    // ---- 005AEC1B: the one-shot "selected" ping (static flag 00E19515) ----
    Byte* const ping_pending = static_cast<Byte*>(host.image_global(0x00E19515));
    if (*ping_pending != 0 && f18e != 0) {
        *ping_pending = 0;
        at<float>(icon, 0x70) = host.global_00f876a4();
        for (int k = 0; k < 3; ++k) {
            void** const slot = reinterpret_cast<void**>(addr(icon, 0x64 + 4 * k));
            if (*slot != nullptr) {  // tested twice in the image (005AEC52, 005AEC59)
                host.object_vcall_04_delete(*slot, 1);
                *slot = nullptr;
            }
            if (at<Byte>(icon, 0x2A) != 0) {
                *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xA4), nullptr);
            } else {
                // 005AEC8A: off the map, the edge arrow at the clamped point.
                void* const arrow = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xE8), nullptr);
                *slot = arrow;
                const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
                const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
                float f17c[3];
                const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                f17c[2] = -12.0f;  // 00CE781C
                f17c[0] = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                f17c[1] = my;
                float angle = 0.0f;
                host.hud_map_world_to_clamped_map_point_0059f7c0(screen, f130, f17c, &angle);
                host.widget_vcall_44_set_float(arrow, angle);
                host.widget_vcall_88(*slot, 0, 0, 1.0f);
                host.widget_set_resolved_position_00aa8240(*slot, f17c);
            }
            // 005AED36
            Byte* const config = static_cast<Byte*>(host.global_config_00432650());
            host.widget_vcall_50_colour(*slot, reinterpret_cast<const float*>(config + 0x158));
        }
    }

    // ---- 005AED58..005AEEEC: the three ping rings +64h..+6Ch ----
    // Ring k shows from icon+70h + k*0.3 s on while the unit is selected and
    // on the map (or was not on it last update); it follows the unit while on
    // the map, grows by 1.02 and fades by 0.85, and is deleted at alpha <= 0.01
    // or when the conditions fail.
    for (std::int32_t k = 0; k < 3; ++k) {
        void** const slot = reinterpret_cast<void**>(addr(icon, 0x64 + 4 * k));  // EDI
        if (*slot == nullptr) {
            continue;
        }
        bool remove = true;
        if (f18e != 0 && (at<Byte>(icon, 0x2A) != 0 || at<Byte>(icon, 0x2B) == 0)) {
            void* const ring = *slot;
            // FILD k, FMUL 0.3f (00CE3DC8 as double), FSUBP; FCOMPI against +70h; JC hides.
            const float since = host.global_00f876a4() - static_cast<float>(k * double(0.3f));
            if (!(since >= at<float>(icon, 0x70))) {
                host.widget_vcall_34_set_visible(ring, 0);  // 005AEF40
                continue;
            }
            host.widget_vcall_34_set_visible(ring, 1);
            if (at<Byte>(icon, 0x2A) != 0) {
                float f150[3];
                const float z = host.unit_vcall_c4_position(unit, f150)[2];
                const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
                const float my = (at<float>(screen, 0x550) - z) / at<float>(screen, 0x34) + g_y;
                float f104[3];
                const float wx = host.unit_vcall_c4_position(unit, f104)[0];
                const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
                float f17c[3];
                f17c[2] = -90.0f;  // 00CF0B70
                f17c[0] = (wx - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                f17c[1] = my;
                host.widget_set_resolved_position_00aa8240(*slot, f17c);
            }
            // 005AEE41: size * 1.02 (00CF0B60), alpha * 0.85 (00CF0B58), both float constants widened.
            void* const grown = *slot;
            const double kGrow = double(1.02f);
            float f158[2];
            f158[0] = static_cast<float>(static_cast<double>(at<float>(grown, 0x28)) * kGrow);
            f158[1] = static_cast<float>(kGrow * static_cast<double>(at<float>(grown, 0x2C)));
            host.widget_vcall_48_set_pair(grown, f158);
            float f9c[4];
            const float alpha = static_cast<float>(
                static_cast<double>(host.widget_vcall_54_colour(*slot, f9c)[3]) * double(0.85f));
            host.widget_vcall_4c_set_alpha(*slot, alpha);
            remove = !(0.01f < alpha);  // FCOMPI (0.01f, alpha); JC keeps
        }
        if (remove && *slot != nullptr) {
            // 005AEEC6
            host.object_vcall_04_delete(*slot, 1);
            *slot = nullptr;
        }
    }

    // ---- 005AEEF2..005AF3AC: the formation line to the leader (+24h) ----
    {
        void* const local_party = ptr(local_player_record(host.global_00e188a8()), 0x28);
        const Byte before = at<Byte>(icon, 0x29);
        const Byte visible = host.recon_relation_if_identified_00803ce0(local_party, unit) != 3 ? 1 : 0;
        at<Byte>(icon, 0x29) = visible;
        f111 = before != visible ? 1 : 0;  // [entry-111h]
    }
    void* const leader =
        host.unit_is_formation_follower_007788b0(unit) ? ptr(ptr(unit, 0x284), 0x14) : nullptr;
    at<void*>(icon, 0x30) = leader;
    if (leader != nullptr && leader != unit && at<Byte>(icon, 0x29) != 0) {
        if (ptr(icon, 0x24) == nullptr) {
            // 005AEF6F: new line object (50h bytes, vtable 00CEFFE8); EH state 1
            // covers the construction.
            Byte* line = static_cast<Byte*>(host.crt_operator_new_00bf681b(0x50));
            if (line != nullptr) {
                Byte* const config = static_cast<Byte*>(host.global_config_00432650());
                at<std::uint32_t>(line, 0) = 0x00CEFFE8;
                cc13_sweepB_hud_map_create_alpha_blend_model_005a1b40(
                    ptr(screen, 0x5A0), ptr(screen, 0xC), reinterpret_cast<void**>(line + 4),
                    reinterpret_cast<void**>(line + 8), reinterpret_cast<void**>(line + 0xC),
                    reinterpret_cast<void**>(line + 0x10), line + 0x30, host);
                float* const diffuse = host.material_diffuse_color_00b179f0(ptr(line, 0x10), 0);
                std::memcpy(diffuse, config + 0x1A8, 4 * sizeof(float));
                at<Byte>(line, 0x14) = 0;
                at<Byte>(line, 0x2C) = 0;
                at<float>(line, 0x30) = 1.0f;
                at<float>(line, 0x40) = 1.0f;
                at<std::uint32_t>(line, 0x1C) = 0;
            }
            at<void*>(icon, 0x24) = line;
            // 005AF036: no null test on the new object (the image writes through it).
            if (ptr(icon, 0x30) != ptr(icon, 0x34)) {
                at<float>(line, 0x40) = 200.0f;  // 00CE386C
                at<float>(ptr(icon, 0x24), 0x30) = 1.0f;
            } else {
                at<float>(line, 0x40) = 50.0f;  // 00CEB4D4
                at<float>(ptr(icon, 0x24), 0x30) = 0.2f;  // 00CE54A0
            }
        }
        void* const line = ptr(icon, 0x24);
        if (line != nullptr) {
            // 005AF081
            if (ptr(icon, 0x30) != ptr(icon, 0x34)) {
                at<float>(line, 0x40) = 200.0f;
                at<float>(ptr(icon, 0x24), 0x30) = 1.0f;
            }
            float f17c[3];
            host.unit_vcall_c4_position(ptr(ptr(unit, 0x284), 0x14), f17c);  // the returned pointer is not read
            // 005AF0B6: d = leader - unit with d.y = 0 (FLDZ; FSUBR ST0,ST0).
            f17c[1] = 0.0f;
            const float dx = f17c[0] - f130[0];
            const float dy = 0.0f;
            const float dz = f17c[2] - f130[2];
            const float yy = dy * dy;
            const float xx = dx * dx;
            const float zz = dz * dz;
            const float length2 = (yy + xx) + zz;
            const float length_a = static_cast<float>(host.crt_sqrt_00bf7030(length2));  // [entry-194h]
            const float length_b = static_cast<float>(host.crt_sqrt_00bf7030(length2));  // [entry-18Ch]
            // FCOMI (length, 0); JBE zeroes: at or below zero, or unordered.
            const float inverse = length_b > 0.0f ? 1.0f / length_b : 0.0f;
            const float nx = inverse * dx;
            const float ny = inverse * dy;
            const float nz = inverse * dz;
            // 005AF1A2: end inset = min(length * 0.1, 75) (FCOMPI 75, m; JA takes m; unordered takes 75).
            const float tenth = static_cast<float>(static_cast<double>(length_a) * double(0.1f));
            const float inset = 75.0f > tenth ? tenth : 75.0f;  // 00CF0B50
            const float ox = inset * nx;
            const float oy = inset * ny;
            const float oz = inset * nz;
            float f13c[3];
            f13c[0] = ox + f130[0];
            f13c[1] = f130[1] + oy;
            f13c[2] = oz + f130[2];
            const float span = length_a - (inset + inset);  // FADD ST0,ST0; FSUBP ST4
            // 005AF230: dot(n, (1,0,0)) and dot(n, (0,0,1)) in the image's operand order.
            const float ny0 = ny * 0.0f;
            const float dot_x = (nx + ny0) + nz * 0.0f;
            const float dot_z = (ny0 + nx * 0.0f) + nz;
            const std::int32_t sign = dot_z > 0.0f ? -1 : 1;  // FCOMPI (dot_z, 0); JA keeps -1
            const float angle = static_cast<float>(host.crt_acos_00bf9940(dot_x)) * static_cast<float>(sign);
            float fe8[3];
            fe8[0] = f13c[0];
            fe8[2] = f13c[2];
            fe8[1] = static_cast<float>(static_cast<double>(f13c[1]) + 1300.0) - 100.0f;  // 00CF0B48, 00D7A220
            float f110[3];
            f110[2] = at<float>(ptr(icon, 0x24), 0x40);
            f110[0] = static_cast<float>(static_cast<double>(span) * double(0.1f));
            f110[1] = 1.0f;
            float ff4[3] = {angle, 0.0f, 0.0f};
            float fdc[16];
            host.matrix_compose_euler_scale_translation_00b64470(fdc, fe8, ff4, f110);
            host.node_vcall_34_set_matrix(ptr(ptr(icon, 0x24), 8), fdc);
            // 005AF34B: alpha toward 0.2 by 0.05, width toward 50 by 0.2 (doubles 00CE3D10,
            // 00D7A270, 00CE3938; 0.2f and 0.05f widened).
            void* const fade = ptr(icon, 0x24);
            const float a = at<float>(fade, 0x30);
            const float step = static_cast<float>(static_cast<double>(static_cast<float>(double(0.2f) - a)) *
                                                  double(0.05f));
            at<float>(fade, 0x30) = a + step;
            void* const widen = ptr(icon, 0x24);
            const float w = at<float>(widen, 0x40);
            const float grow = static_cast<float>(double(0.2f) * static_cast<double>(static_cast<float>(50.0 - w)));
            at<float>(widen, 0x40) = w + grow;
        }
    } else {
        // 005AF393: the slot is tested twice; the delete goes through vtable slot 0.
        if (ptr(icon, 0x24) != nullptr) {
            host.object_vcall_00_delete(ptr(icon, 0x24), 1);
            at<void*>(icon, 0x24) = nullptr;
        }
        at<void*>(icon, 0x24) = nullptr;
    }
    // 005AF3AD
    at<void*>(icon, 0x34) = ptr(icon, 0x30);

    // ---- 005AF3B3..005AFEF5: the per-mode icon rows (screen+128h selects the mode) ----
    // Units of kind 34h or 2Bh skip the rows and the row cleanup.
    const bool rows_skipped =
        host.entity_vcall_5c_is_kind(unit, 0x34) || host.entity_vcall_5c_is_kind(unit, 0x2B);
    if (!rows_skipped) {
        f18d = 0;
        f18f = 0;
        f190 = 0;
        f18e = 0;
        const std::int32_t mode = at<std::int32_t>(screen, 0x128);
        if (at<std::int32_t>(icon, 0x118) == 0 && mode >= 1 && mode <= 4) {
            const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
            const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
            if (mode == 1) {
                // 005AF416: the health bar pair in the vector +78h.
                if (at<Byte>(icon, 0x29) != 0) {
                    Byte* const bar = addr(icon, 0x78);  // EDI
                    f18d = 1;
                    if (vec_size(bar) == 0) {  // begin null, or size 0
                        void* const back = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xB4), nullptr);
                        void* f194_widget = back;
                        host.widget_vcall_34_set_visible(back, 1);
                        float fe8[2] = {0.0f, 0.0f};
                        host.widget_vcall_30_pair(back, fe8);
                        host.widget_vcall_44_set_float(back, 1.57f);  // 00CF0B40
                        host.pointer_vector_push_back_0043dca0(bar, &f194_widget);
                        void* const fill = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xC0), nullptr);
                        f194_widget = fill;
                        host.widget_vcall_34_set_visible(fill, 1);
                        float ff4[2] = {0.0f, 0.0f};
                        host.widget_vcall_30_pair(fill, ff4);
                        host.widget_vcall_44_set_float(fill, 1.57f);
                        host.pointer_vector_push_back_0043dca0(bar, &f194_widget);
                    }
                    // 005AF50D
                    if (ptr(bar, 4) != nullptr && vec_size(bar) == 2) {
                        const float width =
                            static_cast<float>(static_cast<double>(host.gui_widget_get_size_00aa6740(
                                                   *host.pointer_vector_at_004391a0(bar, 0))[0]) *
                                               0.25);  // 00D7A348
                        float f9c[3];
                        f9c[2] = kMapIconZ;
                        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                        const float mx = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                        const float half = static_cast<float>(static_cast<double>(width) * 0.5);  // FST QWORD [entry-170h]
                        f9c[0] = half + mx;
                        f9c[1] = width + my;
                        host.widget_set_resolved_position_00aa8240(*host.pointer_vector_at_004391a0(bar, 0), f9c);
                        float f104[3];
                        f104[2] = 200.0f;  // 00CE386C
                        const float my2 = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                        const float mx2 = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                        f104[0] = mx2 + half;
                        f104[1] = width + my2;
                        host.widget_set_resolved_position_00aa8240(*host.pointer_vector_at_004391a0(bar, 1), f104);
                        // 005AF63A: the fill length is the HUD health * 0.2 (0.2f widened).
                        float f110[2];
                        f110[0] = static_cast<float>(
                            static_cast<double>(host.unit_hud_health_0059ac50(screen, unit)) * double(0.2f));
                        f110[1] = 0.5f;  // 00CE3800
                        host.widget_vcall_48_set_pair(*host.pointer_vector_at_004391a0(bar, 0), f110);
                        float f17c[2] = {0.2f, 0.5f};  // 00CE54A0, 00CE3800
                        host.widget_vcall_48_set_pair(*host.pointer_vector_at_004391a0(bar, 1), f17c);
                        float f150c[4];
                        host.unit_health_colour_0077a2f0(f150c, 1.0f);
                        f150c[3] = 0.75f;  // 00CEFF50
                        host.widget_vcall_50_colour(*host.pointer_vector_at_004391a0(bar, 0), f150c);
                    }
                }
            } else if (mode == 2) {
                // 005AF6EA: kind 1Ch units: the badge row (+98h) and the carried-unit grid (+B8h).
                if (host.entity_vcall_5c_is_kind(unit, 0x1C)) {
                    f190 = 1;
                    cc13_sweepB_hud_map_icon_refresh_unit_badge_005a8420(
                        icon, ptr(screen, 0xA8), at<std::int32_t>(unit, 0x54) == 2 ? 1u : 0u, host);
                    std::int32_t cols = 0;  // [entry-160h]
                    std::int32_t rows = 0;  // [entry-184h]
                    const float cell_w = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[0];
                    const float cell_h = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[1];
                    host.hud_map_grid_dims_0059e0c0(screen, &cols, &rows, vec_size(addr(icon, 0x98)));
                    for (std::uint32_t i = 0; ptr(icon, 0x9C) != nullptr && i < vec_size(addr(icon, 0x98)); ++i) {
                        const std::int32_t index = static_cast<std::int32_t>(i);
                        const std::int32_t row = index / cols;
                        const std::int32_t column = index % cols;
                        const float mx = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                        float f170[3];
                        f170[0] = mx + static_cast<float>(column) * cell_w;
                        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                        f170[1] = my + static_cast<float>(row) * cell_h;
                        f170[2] = static_cast<float>(static_cast<float>(index * double(0.005f)) + 110.0);
                        host.widget_set_resolved_position_00aa8240(*vec_checked_at(addr(icon, 0x98), i, host), f170);
                        host.widget_vcall_34_set_visible(*vec_checked_at(addr(icon, 0x98), i, host), 1);
                    }
                    // 005AF87B: the carried units, laid out right to left.
                    if (vec_size(addr(icon, 0xB8)) == 0) {
                        host.hud_map_icon_refresh_carried_unit_icons_005aac00(icon, ptr(screen, 0xAC));
                    }
                    std::int32_t per_row = 0;  // EBX
                    const float carried_w = host.gui_widget_get_size_00aa6740(ptr(screen, 0xAC))[0];
                    const float carried_h = host.gui_widget_get_size_00aa6740(ptr(screen, 0xAC))[1];
                    if (ptr(icon, 0xBC) != nullptr) {
                        const std::int32_t count = static_cast<std::int32_t>(vec_size(addr(icon, 0xB8)));
                        if (count != 0) {
                            const float root = static_cast<float>(host.crt_sqrt_00bf7030(static_cast<double>(count)));
                            per_row = host.crt_ftol_00bf7420(root);
                            if (per_row * per_row < count) {
                                per_row += 1;
                            }
                        }
                    }
                    for (std::uint32_t i = 0; ptr(icon, 0xBC) != nullptr && i < vec_size(addr(icon, 0xB8)); ++i) {
                        const std::int32_t index = static_cast<std::int32_t>(i);
                        const std::int32_t row = index / per_row;
                        const std::int32_t column = index % per_row;
                        const float mx = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                        float f170[3];
                        f170[0] = mx - static_cast<float>(column + 1) * carried_w;
                        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                        f170[1] = my - static_cast<float>(row) * carried_h;
                        f170[2] = static_cast<float>(static_cast<float>(index * double(0.005f)) + 110.0);
                        host.widget_set_resolved_position_00aa8240(*vec_checked_at(addr(icon, 0xB8), i, host), f170);
                        host.widget_vcall_34_set_visible(*vec_checked_at(addr(icon, 0xB8), i, host), 1);
                    }
                }
            } else if (mode == 3) {
                // 005AFA0A: the power-up row (+88h).
                if (at<Byte>(icon, 0x29) != 0) {
                    f18f = 1;
                    cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(icon, ptr(screen, 0xA8), true, host);
                    std::int32_t cols = 0;  // [entry-194h]
                    std::int32_t rows = 0;  // [entry-158h]
                    const float cell_w = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[0];
                    const float cell_h = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[1];
                    host.hud_map_grid_dims_0059e0c0(screen, &cols, &rows, vec_size(addr(icon, 0x88)));
                    for (std::uint32_t i = 0; ptr(icon, 0x8C) != nullptr && i < vec_size(addr(icon, 0x88)); ++i) {
                        const std::int32_t index = static_cast<std::int32_t>(i);
                        const std::int32_t row = index / cols;
                        const std::int32_t column = index % cols;
                        const float mx = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                        float f170[3];
                        f170[0] = mx + static_cast<float>(column) * cell_w;
                        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                        f170[1] = my + static_cast<float>(row) * cell_h;
                        f170[2] = static_cast<float>(static_cast<float>(index * double(0.005f)) + 110.0);
                        host.widget_set_resolved_position_00aa8240(*vec_checked_at(addr(icon, 0x88), i, host), f170);
                        host.widget_vcall_34_set_visible(*vec_checked_at(addr(icon, 0x88), i, host), 1);
                    }
                }
            } else {
                // 005AFB88: the four status lamps (+A8h) of a kind-6 unit.
                if (at<Byte>(icon, 0x29) != 0 && host.entity_vcall_5c_is_kind(unit, 6)) {
                    Byte lamp[4] = {0, 0, 0, 0};  // [entry-160h], -18Ch, -194h, -184h
                    hud_map_icon_query_unit_status(icon, lamp, host);
                    if (lamp[0] != 0 || lamp[1] != 0 || lamp[2] != 0 || lamp[3] != 0) {
                        Byte* const lamps = addr(icon, 0xA8);  // EDI
                        f18e = 1;
                        if (ptr(lamps, 4) == nullptr || vec_size(lamps) != 4) {
                            for (int n = 0; n < 4; ++n) {
                                void* lamp_widget = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xB0), nullptr);
                                if (lamp_widget != nullptr) {
                                    host.widget_vcall_34_set_visible(lamp_widget, 0);
                                    host.pointer_vector_push_back_0043dca0(lamps, &lamp_widget);
                                }
                            }
                        }
                        std::int32_t shown = -1;  // [entry-164h]
                        for (std::uint32_t k = 0; k < 4; ++k) {
                            // 005AFC69: show/hide lamp k by its flag (the image passes the
                            // flag's whole stack dword; only the byte is modelled), frame
                            // 0 when lit, else 5..8 (9 for a kind-8 unit's fourth lamp).
                            const Byte on = lamp[k];
                            host.widget_vcall_34_set_visible(*vec_checked_at(lamps, k, host), on);
                            std::int32_t frame;
                            void* lamp_widget;
                            if (k < 3) {
                                lamp_widget = *vec_checked_at(lamps, k, host);
                                frame = on != 0 ? 0 : static_cast<std::int32_t>(5 + k);
                            } else {
                                const bool kind8 = host.entity_vcall_5c_is_kind(unit, 8);
                                lamp_widget = *host.pointer_vector_at_004391a0(lamps, 3);
                                frame = on != 0 ? 0 : (kind8 ? 9 : 8);
                            }
                            host.widget_vcall_88(lamp_widget, frame, 0, 1.0f);
                            if (on != 0) {
                                shown += 1;
                            }
                            // 005AFE65
                            const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
                            const float mx = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
                            const float lamp_w = host.gui_widget_get_size_00aa6740(ptr(screen, 0xB0))[0];
                            float f150p[3];
                            f150p[2] = kMapIconZ;
                            f150p[0] = static_cast<float>(shown) * lamp_w + mx;
                            f150p[1] = my;
                            host.widget_set_resolved_position_00aa8240(*vec_checked_at(lamps, k, host), f150p);
                        }
                    }
                }
            }
        }

        // ---- 005AFEF6..005B02D4: rows no mode filled this update are deleted ----
        if (vec_size(addr(icon, 0x88)) != 0 && f18f == 0) {
            delete_all_and_erase(addr(icon, 0x88), host);
        }
        if (vec_size(addr(icon, 0x78)) != 0 && f18d == 0) {
            delete_all_and_erase(addr(icon, 0x78), host);
        }
        if (vec_size(addr(icon, 0xA8)) != 0 && f18e == 0) {
            delete_all_and_erase(addr(icon, 0xA8), host);
        }
        if ((vec_size(addr(icon, 0x98)) != 0 || vec_size(addr(icon, 0xB8)) != 0) && f190 == 0) {
            delete_all_and_erase(addr(icon, 0x98), host);
            // 005B025A: the +B8h vector is erased through 005A0B20.
            Byte* const carried = addr(icon, 0xB8);
            for (std::uint32_t i = 0; ptr(carried, 4) != nullptr && i < vec_size(carried); ++i) {
                void** const slot = vec_checked_at(carried, i, host);
                if (*slot != nullptr) {
                    host.object_vcall_04_delete(*slot, 1);
                    *slot = nullptr;
                }
            }
            void* const last = ptr(carried, 8);
            if (static_cast<Byte*>(ptr(carried, 4)) > static_cast<Byte*>(last)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* const first = ptr(carried, 4);
            if (static_cast<Byte*>(first) > static_cast<Byte*>(ptr(carried, 8))) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            void* f17c_result[2];
            host.pointer_vector_erase_range_005a0b20(carried, f17c_result, carried, first, carried, last);
        }
    }

    // ---- 005B02D5..005B03D7: the off-map edge arrow (+20h) ----
    if (ptr(icon, 0x20) != nullptr) {
        void* const arrow = ptr(icon, 0x20);  // EBP
        const float g_x = *static_cast<float*>(host.image_global(0x00E08890));
        const float g_y = *static_cast<float*>(host.image_global(0x00E08894));
        float f17c[3];
        f17c[2] = -12.0f;  // 00CE781C
        float angle = 0.0f;   // [entry-194h]
        float alpha = 0.75f;  // [entry-18Ch] (00CEFF50)
        std::int32_t frame = 0;  // EDI
        const float my = (at<float>(screen, 0x550) - f130[2]) / at<float>(screen, 0x34) + g_y;
        f17c[0] = (f130[0] - at<float>(screen, 0x54C)) / at<float>(screen, 0x30) + g_x;
        f17c[1] = my;
        if (at<Byte>(icon, 0x28) != 0) {
            f17c[2] = -2.0f;  // 00CE7D7C
            frame = 1;
            if (at<Byte>(icon, 0x2A) == 0) {
                host.hud_map_world_to_clamped_map_point_0059f7c0(screen, f130, f17c, &angle);
                alpha = 1.0f;
                frame = 0;
            }
        }
        host.widget_set_resolved_position_00aa8240(arrow, f17c);
        host.widget_vcall_44_set_float(ptr(icon, 0x20), angle);
        host.widget_vcall_34_set_visible(ptr(icon, 0x20), 1);
        host.widget_vcall_88(ptr(icon, 0x20), frame, 0, 1.0f);
        host.widget_vcall_4c_set_alpha(ptr(icon, 0x20), alpha);
    }

    // ---- 005B03D8: place the info panel when it has its frame widget ----
    if (*panel(icon, 0, host) != nullptr) {
        at<std::int32_t>(icon, 0x120) = 0;
        host.hud_map_icon_place_widget_0059d4a0(screen, icon);
        if (!host.hud_map_icon_update_offscreen_edge_0059d6d0(screen, icon)) {
            host.hud_map_icon_place_widget_0059d4a0(screen, icon);
        }
    }

    // ---- 005B0425: the info panel state machine (+118h) ----
    float* const box = fp(icon, 0xF8);  // EBP from 005B0466: {width, height}
    if (at<std::int32_t>(icon, 0x118) == 1 &&
        !(host.global_00f876a4() < at<float>(icon, 0x110))) {  // FCOMPI (clock, +110h); JC stays
        // 005B0448: open: state 2, reset the panel box to 64 x 10.
        at<std::int32_t>(icon, 0x118) = 2;
        at<float>(icon, 0x110) = static_cast<float>(static_cast<double>(host.global_00f876a4()) + double(0.2f));
        at<float>(icon, 0x114) = host.global_00f876a4();
        at<float>(icon, 0x100) = 0.0f;
        at<float>(icon, 0x104) = 0.0f;
        at<float>(icon, 0x108) = 0.0f;
        at<float>(icon, 0x10C) = 0.0f;
        box[0] = 64.0f;  // 00CE7820
        box[1] = 10.0f;  // 00CE38B8
        at<std::int32_t>(icon, 0x120) = 0;
        if (*panel(icon, 2, host) == nullptr) {
            void** const slot = panel(icon, 2, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xC4), nullptr);
        }
        if (*panel(icon, 2, host) != nullptr) {
            float f17c[2] = {0.0f, 0.0f};
            host.widget_vcall_30_pair(*panel(icon, 2, host), f17c);
            void** const title = panel(icon, 2, host);  // EDI, taken before the name call
            const char* const name = host.object_vcall_14_name(unit);
            host.gui_text_set_localised_cstring_00abbe50(*title, name, 1);
            // 005B05F0: the box width from the title's text width (+114h px, /960
            // then *960 at float precision), even-rounded down, at least 100.
            void* const title_widget = *panel(icon, 2, host);
            box[1] = at<float>(icon, 0xFC);  // MOVSS [EBP+4] <- [ESI+FCh]: the same field
            const float fraction = static_cast<float>(static_cast<double>(at<float>(title_widget, 0x114)) / 960.0);
            const float pixels = static_cast<float>(960.0 * static_cast<double>(fraction));
            box[0] = static_cast<float>(static_cast<std::int32_t>(pixels) & 0xFFFE);  // CVTTSS2SI, AND, CVTSI2SS
            // FCOMPI (100.0, width); JA keeps the 100.0f (00CE3D08) temporary.
            box[0] = 100.0 > static_cast<double>(box[0]) ? 100.0f : box[0];
            host.widget_vcall_34_set_visible(*panel(icon, 2, host), 0);
            at<Byte>(icon, 0x2C) = 0;
            if (host.entity_vcall_5c_is_kind(unit, 6)) {
                host.hud_map_icon_query_unit_status_0059d150(icon, &f18f, &f18d, &f18e, &f190);
                if (f18f != 0 || f18d != 0 || f18e != 0 || f190 != 0) {
                    at<Byte>(icon, 0x2C) = 1;
                    at<float>(icon, 0xFC) = static_cast<float>(static_cast<double>(at<float>(icon, 0xFC)) + 40.0);
                }
            }
            const std::int32_t count =
                at<Byte>(icon, 0x29) != 0
                    ? cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(icon, nullptr, false, host)
                    : 0;
            at<std::int32_t>(icon, 0x38) = count;
            if (count != 0) {
                // 005B070C: rows of power-ups across the box width.
                const double wide = static_cast<float>(static_cast<double>(box[0]) + 38.0);  // FSTP QWORD
                const float cell = static_cast<float>(
                    static_cast<double>(host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[0]) * 960.0);
                const std::int32_t cols = host.crt_ftol_00bf7420(static_cast<float>(wide / static_cast<double>(cell)));
                const std::int32_t rows = (cols + at<std::int32_t>(icon, 0x38) - 1) / cols;
                const float lines = static_cast<float>(static_cast<float>(rows * 38.0) + 4.0);
                at<float>(icon, 0xFC) = lines + at<float>(icon, 0xFC);
            }
        }
    }

    const std::int32_t state = at<std::int32_t>(icon, 0x118);
    if (state == 2) {
        // 005B076B: create the frame widget from the screen's first template.
        if (*panel(icon, 0, host) == nullptr) {
            void** const templates = vec_checked_at(addr(screen, 0xD4), 0, host);
            void** const slot = panel(icon, 0, host);
            *slot = host.widget_clone_subtree_00aab4c0(*templates, nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 0, host), 0);
        }
        if (host.hud_map_icon_update_size_and_visibility_0059cef0(icon, false)) {
            at<std::int32_t>(icon, 0x118) = 3;
        }
        return;
    }
    if (state == 3) {
        // 005B0845: create the remaining panel widgets, hidden.
        if (at<Byte>(icon, 0x29) != 0) {
            cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(icon, ptr(screen, 0xA8), false, host);
        }
        if (*panel(icon, 1, host) == nullptr) {
            void** const slot = panel(icon, 1, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xAC), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 1, host), 0);
            std::int32_t frame = 0;
            if (host.entity_vcall_5c_is_kind(unit, 0x1C) || at<Byte>(icon, 0x29) != 0) {
                frame = host.unit_hud_icon_index_0059cb90(unit, nullptr);
            }
            host.widget_vcall_88(*panel(icon, 1, host), frame, 0, 1.0f);
        }
        if (host.entity_vcall_5c_is_kind(unit, 6)) {
            Byte* const lamps = addr(icon, 0xD8);
            if (ptr(lamps, 4) == nullptr || vec_size(lamps) != 4) {
                for (int n = 0; n < 4; ++n) {
                    void* lamp_widget = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xB0), nullptr);
                    if (lamp_widget != nullptr) {
                        host.widget_vcall_34_set_visible(lamp_widget, 0);
                        host.pointer_vector_push_back_0043dca0(lamps, &lamp_widget);
                    }
                }
            }
        }
        if (*panel(icon, 7, host) == nullptr) {
            void** slot = panel(icon, 7, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xB4), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 7, host), 0);
            slot = panel(icon, 6, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xC0), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 6, host), 0);
        }
        if (*panel(icon, 8, host) == nullptr) {
            void** const slot = panel(icon, 8, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xB8), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 8, host), 0);
        }
        if (*panel(icon, 9, host) == nullptr) {
            void** const slot = panel(icon, 9, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xBC), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 9, host), 0);
        }
        if (*panel(icon, 10, host) == nullptr) {
            void** const slot = panel(icon, 10, host);
            *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, 0xD0), nullptr);
            host.widget_vcall_34_set_visible(*panel(icon, 10, host), 0);
            float f17c[2] = {0.5f, 0.5f};  // 00CE3800
            host.widget_vcall_30_pair(*panel(icon, 10, host), f17c);
        }
        if (*panel(icon, 4, host) == nullptr) {
            const std::uint32_t order[3] = {4, 3, 5};
            const std::uint32_t source[3] = {0xC4, 0xC8, 0xCC};
            for (int n = 0; n < 3; ++n) {
                void** const slot = panel(icon, order[n], host);
                *slot = host.widget_clone_subtree_00aab4c0(ptr(screen, source[n]), nullptr);
                host.widget_vcall_34_set_visible(*panel(icon, order[n], host), 0);
                float f17c[2] = {0.0f, 0.0f};
                host.widget_vcall_30_pair(*panel(icon, order[n], host), f17c);
            }
        }
        at<std::int32_t>(icon, 0x118) = 4;
        return;
    }
    if (state == 4) {
        // 005B0EF9: lay the panel out from the frame's resolved position.
        float f13c[3];
        host.widget_get_resolved_position_00aa6750(*panel(icon, 0, host), f13c);
        f13c[0] = static_cast<float>(static_cast<double>(f13c[0]) + 0.010416666977107525);  // 00CF0B28
        f13c[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.013888888992369175);  // 00CF0B20
        f13c[2] = static_cast<float>(static_cast<double>(f13c[2]) + 0.0);                   // 00D7A258
        const float z = static_cast<float>(static_cast<double>(f13c[2]) -
                                           (at<Byte>(icon, 0x28) == 0 ? 10.0 : 1.0));  // [entry-164h]
        host.hud_map_icon_update_size_and_visibility_0059cef0(icon, false);  // result not read
        at<float>(icon, 0xFC) = 10.0f;
        if (*panel(icon, 1, host) != nullptr) {
            float f150p[3];
            f150p[2] = z;
            f150p[0] = static_cast<float>(static_cast<double>(f13c[0]) + 0.010999999940395355);  // 00CF0B18
            f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.01600000075995922);   // 00CF0B10
            host.widget_set_resolved_position_00aa8240(*panel(icon, 1, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 1, host), at<Byte>(icon, 0x2A));
            if (f111 != 0) {
                std::int32_t frame = 0;
                if (host.entity_vcall_5c_is_kind(unit, 0x1C) || at<Byte>(icon, 0x29) != 0) {
                    frame = host.unit_hud_icon_index_0059cb90(unit, nullptr);
                }
                host.widget_vcall_88(*panel(icon, 1, host), frame, 0, 1.0f);
            }
        }
        if (*panel(icon, 2, host) != nullptr) {
            // 005B10E4: the title, "globals.unknown" (00CE4180) for an unseen non-1Ch unit.
            if (at<Byte>(icon, 0x29) == 0 && !host.entity_vcall_5c_is_kind(unit, 0x1C)) {
                void** const title = panel(icon, 2, host);
                host.gui_text_set_localised_cstring_00abbe50(
                    *title, static_cast<const char*>(host.image_global(0x00CE4180)), 1);
            } else {
                void** const title = panel(icon, 2, host);
                const char* const name = host.object_vcall_14_name(unit);
                host.gui_text_set_localised_cstring_00abbe50(*title, name, 1);
            }
            float f150p[3];
            f150p[2] = z;
            f150p[0] = static_cast<float>(static_cast<double>(f13c[0]) + 0.029999999329447746);   // 00CEB690
            f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) - 0.0020000000949949026);  // 00CED0E0
            host.widget_set_resolved_position_00aa8240(*panel(icon, 2, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 2, host), at<Byte>(icon, 0x2A));
        }
        // 005B1203: the distance line "<km>.<tenth> |globals.kilometer|" to the selected unit.
        void* const selected = (*panel(icon, 4, host) != nullptr && *panel(icon, 3, host) != nullptr &&
                                *panel(icon, 5, host) != nullptr)
                                   ? host.global_00e188d8()
                                   : nullptr;
        if (selected != nullptr) {
            float f150p[3];
            const float* const from = host.unit_vcall_c4_position(selected, f150p);
            float f104[3];
            const float* const to = host.unit_vcall_c4_position(unit, f104);
            const float d0 = to[0] - from[0];
            const float d1 = to[1] - from[1];
            const float d2 = to[2] - from[2];
            f150p[0] = d0;
            f150p[1] = d1;
            f150p[2] = d2;
            const float metres = static_cast<float>(host.vector3_length_0042b2f0(f150p));
            const bool miles = *static_cast<Byte*>(host.image_global(0x00F88988)) != 0;
            const float tenths =
                static_cast<float>(static_cast<double>(metres) / (miles ? 185.2 : 100.0));  // 00CF0B08, 00D7A220
            const std::int32_t truncated = static_cast<std::int32_t>(tenths);  // CVTTSS2SI
            std::int32_t whole = truncated / 10;
            std::int32_t tenth = truncated - whole * 10;
            const float rest = tenths - static_cast<float>(truncated);  // FISUB
            if (rest > 0.5f) {  // FCOMPI (rest, 0.5); JBE skips
                tenth += 1;
            }
            if (tenth == 10) {
                whole += 1;
                tenth = 0;
            }
            Str f158{0, nullptr};
            host.native_string_assign_0041e870(
                &f158, static_cast<const char*>(host.image_global(miles ? 0x00CF0AF4 : 0x00CF0ADC)));
            Str f170{0, nullptr};
            host.native_string_assign_0041e870(&f170, static_cast<const char*>(host.image_global(0x00CE3A70)));
            Str f160{0, nullptr};
            void* const tenth_text = host.native_string_from_int_004260b0(&f160, tenth);
            Str fe8{0, nullptr};
            void* const whole_text = host.native_string_from_int_004260b0(&fe8, whole);
            Str ff4{0, nullptr};
            void* const head = host.native_string_prefixed_0043c130(
                &ff4, static_cast<const char*>(host.image_global(0x00CF0AD8)), whole_text);
            Str f110{0, nullptr};
            void* const with_dot = host.native_string_concat_004261a0(head, &f110, &f170);
            Str f17c{0, nullptr};
            void* const with_tenth = host.native_string_concat_004261a0(with_dot, &f17c, tenth_text);
            Str f18c{0, nullptr};
            host.native_string_concat_004261a0(with_tenth, &f18c, &f158);
            release(f17c, host);
            release(f110, host);
            release(ff4, host);
            release(fe8, host);
            release(f160, host);
            release(f170, host);
            release(f158, host);
            host.gui_text_set_localised_source_00abaed0(*panel(icon, 4, host), &f18c, 1);
            // 005B1572: the distance, its icon and the unit label in a row.
            const float text_width = static_cast<float>(
                static_cast<double>(at<float>(*panel(icon, 4, host), 0x114)) / 960.0);
            f150p[2] = z;
            f150p[0] = static_cast<float>(static_cast<double>(f13c[0]) + 0.0);
            f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.041999999433755875);  // 00CF0AD0
            host.widget_set_resolved_position_00aa8240(*panel(icon, 3, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 3, host), at<Byte>(icon, 0x2A));
            const float* size3 = host.gui_widget_get_size_00aa6740(*panel(icon, 3, host));
            f150p[2] = z;
            f150p[0] = static_cast<float>(static_cast<double>(f13c[0]) + 0.0) + size3[0];
            f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.03200000151991844);  // 00CF0AC8
            host.widget_set_resolved_position_00aa8240(*panel(icon, 4, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 4, host), at<Byte>(icon, 0x2A));
            size3 = host.gui_widget_get_size_00aa6740(*panel(icon, 3, host));
            f150p[2] = z;
            f150p[0] = (static_cast<float>(static_cast<double>(f13c[0]) + 0.0) + size3[0]) + text_width;
            f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.041999999433755875);
            host.widget_set_resolved_position_00aa8240(*panel(icon, 5, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 5, host), at<Byte>(icon, 0x2A));
            release(f18c, host);  // EH state -1
        }

        // 005B1810: the four status lamps (+D8h) of the panel.
        Byte* const lamps = addr(icon, 0xD8);
        if (ptr(lamps, 4) != nullptr && vec_size(lamps) == 4) {
            at<Byte>(icon, 0x2C) = 0;
            f18e = 0;
            f18d = 0;
            f18f = 0;
            f190 = 0;
            if (host.entity_vcall_5c_is_kind(unit, 6)) {
                host.hud_map_icon_query_unit_status_0059d150(icon, &f18e, &f18d, &f18f, &f190);
                if (f18e != 0 || f18d != 0 || f18f != 0 || f190 != 0) {
                    at<Byte>(icon, 0x2C) = 1;
                    at<float>(icon, 0xFC) = static_cast<float>(static_cast<double>(at<float>(icon, 0xFC)) + 40.0);
                }
            }
            for (std::uint32_t k = 0; k < 4; ++k) {
                const Byte show = (at<Byte>(icon, 0x2A) != 0 && at<Byte>(icon, 0x2C) != 0) ? 1 : 0;
                host.widget_vcall_34_set_visible(*vec_checked_at(lamps, k, host), show);
                std::int32_t frame;
                if (k == 0) {
                    vec_checked_at(lamps, 0, host);
                    frame = f18e != 0 ? 0 : 5;
                } else if (k == 1) {
                    vec_checked_at(lamps, 1, host);
                    frame = f18d != 0 ? 0 : 6;
                } else if (k == 2) {
                    vec_checked_at(lamps, 2, host);
                    frame = f18f != 0 ? 0 : 7;
                } else {
                    const bool kind8 = host.entity_vcall_5c_is_kind(unit, 8);
                    vec_checked_at(lamps, 3, host);
                    frame = f190 != 0 ? 0 : (kind8 ? 9 : 8);
                }
                host.widget_vcall_88(static_cast<void**>(ptr(lamps, 4))[k], frame, 0, 1.0f);
                const float* const lamp_size = host.gui_widget_get_size_00aa6740(*vec_checked_at(lamps, 0, host));
                float f150p[3];
                f150p[2] = z;
                f150p[0] = static_cast<float>(static_cast<std::int32_t>(k)) * lamp_size[0] + f13c[0];
                f150p[1] = static_cast<float>(static_cast<double>(f13c[1]) + 0.05999999865889549);  // 00CF0AC0
                host.widget_set_resolved_position_00aa8240(*vec_checked_at(lamps, k, host), f150p);
            }
        }

        // 005B1AE4: the power-up grid (+C8h) below the title.
        const double wide = static_cast<float>(static_cast<double>(box[0]) + 38.0);  // FSTP QWORD [entry-17Ch]
        const float cell = static_cast<float>(
            static_cast<double>(host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[0]) * 960.0);
        const std::int32_t cols = host.crt_ftol_00bf7420(static_cast<float>(wide / static_cast<double>(cell)));
        const std::int32_t count =
            at<Byte>(icon, 0x29) != 0
                ? cc13_sweepB6_hud_map_icon_refresh_power_up_icons_005a7c20(icon, ptr(screen, 0xA8), false, host)
                : 0;
        at<std::int32_t>(icon, 0x38) = count;
        const double wide2 = static_cast<float>(static_cast<double>(box[0]) + 38.0);
        const std::int32_t rows = (count + cols - 1) / cols;  // [entry-194h]
        const float cell2 = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8))[0];
        float f170_slot = static_cast<float>(cols);  // FILD/FSTP [entry-170h]
        // 005B1B69: gap = ((wide - cell*960*cols) / cols) / 960, rounded per step.
        const float cell960 = static_cast<float>(static_cast<double>(cell2) * 960.0);
        const float used = cell960 * f170_slot;
        const float spare = static_cast<float>(wide2 - static_cast<double>(used));
        const float per = spare / f170_slot;
        const float gap = static_cast<float>(static_cast<double>(per) / 960.0);  // [entry-158h]
        if (at<std::int32_t>(icon, 0x38) != 0) {
            const float lines = static_cast<float>(static_cast<float>(rows * 38.0) + 4.0);
            at<float>(icon, 0xFC) = lines + at<float>(icon, 0xFC);
            Byte* const grid = addr(icon, 0xC8);
            for (std::uint32_t i = 0; ptr(grid, 4) != nullptr && i < vec_size(grid); ++i) {
                const std::int32_t index = static_cast<std::int32_t>(i);
                const std::int32_t line = index / cols + 1;
                const std::int32_t column = index % cols;  // [entry-184h]
                f170_slot = static_cast<float>(static_cast<float>(static_cast<float>(line * 38.0) + 9.0) / 720.0);
                const float lamp_h =
                    at<Byte>(icon, 0x2C) != 0 ? host.gui_widget_get_size_00aa6740(ptr(screen, 0xB0))[1] : 0.0f;
                const float* const cell_size = host.gui_widget_get_size_00aa6740(ptr(screen, 0xA8));
                float f150p[3];
                f150p[0] = (gap + cell_size[0]) * static_cast<float>(column) + f13c[0];
                f170_slot = f170_slot + lamp_h;
                f150p[1] = f170_slot + f13c[1];
                f150p[2] = static_cast<float>(index * double(0.005f)) + z;
                host.widget_set_resolved_position_00aa8240(*vec_checked_at(grid, i, host), f150p);
                host.widget_vcall_34_set_visible(*vec_checked_at(grid, i, host), at<Byte>(icon, 0x2A));
            }
        }

        // 005B1CD8: the capture bar of a kind-1Ch unit (fill sign picks the bar).
        float capture = 0.0f;  // [entry-184h]
        std::uint32_t bar;     // EDI
        if (host.entity_vcall_5c_is_kind(unit, 0x1C)) {
            const float fill = host.command_building_capture_ring_fill_006f1f90(unit);
            capture = fill > 0.0f ? fill : -0.0f - fill;  // FCOMPI (fill, 0); JBE negates (00D7A208)
            const float again = host.command_building_capture_ring_fill_006f1f90(unit);
            if (again > 0.0f) {  // FCOMPI (again, 0); JBE
                bar = 0;
            } else {
                bar = capture == 0.0f ? 2 : 1;  // UCOMISS, LAHF/TEST AH,44h/JNP: 2 only when ordered equal
            }
        } else {
            // 005B1D59: the image loads the [entry-170h] slot (a float here) into
            // EDI; on this path capture stays 0 and EDI is not read.
            std::memcpy(&bar, &f170_slot, sizeof bar);
        }
        if (*panel(icon, 7, host) != nullptr && at<Byte>(icon, 0x29) != 0) {
            // 005B1D96
            at<float>(icon, 0xFC) = static_cast<float>(static_cast<double>(at<float>(icon, 0xFC)) + 10.0);
            const float lamp_h =
                at<Byte>(icon, 0x2C) != 0 ? host.gui_widget_get_size_00aa6740(ptr(screen, 0xB0))[1] : 0.0f;
            const float top = static_cast<float>(static_cast<double>(lamp_h) + 0.06599999964237213);  // 00CF0AB0
            float below = 0.0f;
            if (at<std::int32_t>(icon, 0x38) != 0) {
                below = static_cast<float>(static_cast<float>(static_cast<float>(rows * 38.0) + 2.0) / 720.0);
            }
            const float row_y = below + top;
            float f110[3];
            f110[0] = f13c[0];
            f110[2] = z;
            f110[1] = row_y + f13c[1];
            float f150p[3];
            f150p[0] = f13c[0] + 0.0f;
            f150p[1] = 0.0f + f110[1];
            f150p[2] = static_cast<float>(static_cast<double>(z) + 1.0);  // 00D7A210
            host.widget_set_resolved_position_00aa8240(*panel(icon, 6, host), f150p);
            host.widget_vcall_34_set_visible(*panel(icon, 6, host), at<Byte>(icon, 0x2A));
            // UCOMISS capture, 0.0f (00D7A218); JP leaves: not equal or unordered.
            if (capture == 0.0f) {
                host.widget_set_resolved_position_00aa8240(*panel(icon, 7, host), f110);
                float f17c[2];
                f17c[0] = host.unit_hud_health_0059ac50(screen, unit);
                f17c[1] = 1.0f;
                host.widget_vcall_48_set_pair(*panel(icon, 7, host), f17c);
                float f104[4];
                const float* const colour = host.unit_health_colour_0077a2f0(f104, 1.0f);
                float f150c[4];
                copy4(f150c, colour);
                f150c[3] = 1.0f;
                host.widget_vcall_50_colour(*panel(icon, 7, host), f150c);
                host.widget_vcall_34_set_visible(*panel(icon, 7, host), at<Byte>(icon, 0x2A));
                host.widget_vcall_34_set_visible(*panel(icon, 9, host), 0);
                host.widget_vcall_34_set_visible(*panel(icon, 8, host), 0);
            } else {
                host.widget_vcall_34_set_visible(*panel(icon, 7, host), 0);
                if (bar == 0 || bar == 1) {
                    const std::uint32_t shown = bar == 0 ? 9 : 8;
                    const std::uint32_t hidden = bar == 0 ? 8 : 9;
                    host.widget_set_resolved_position_00aa8240(*panel(icon, shown, host), f110);
                    float f17c[2] = {capture, 1.0f};
                    host.widget_vcall_48_set_pair(*panel(icon, shown, host), f17c);
                    host.widget_vcall_34_set_visible(*panel(icon, shown, host), at<Byte>(icon, 0x2A));
                    host.widget_vcall_34_set_visible(*panel(icon, hidden, host), 0);
                }
            }
        }

        // 005B2268: the pointer arrow (+E8h[10]) on the side the panel opened to (+120h).
        if (*panel(icon, 10, host) == nullptr) {
            return;
        }
        const float* const frame_size = host.gui_widget_get_size_00aa6740(*panel(icon, 0, host));
        float f17c[3];
        const float frame_w = frame_size[0];  // [entry-170h]
        const float frame_h = frame_size[1];  // [entry-16Ch]
        f17c[0] = static_cast<float>(static_cast<double>(f13c[0]) - 0.010416666977107525);
        f17c[1] = static_cast<float>(static_cast<double>(f13c[1]) - 0.013888888992369175);
        f17c[2] = f13c[2] - 0.0f;  // FLDZ; FSUB ST1,ST0
        switch (static_cast<std::uint32_t>(at<std::int32_t>(icon, 0x120))) {
        case 0:
            f17c[0] = f17c[0] - 0.0f;
            f17c[1] = f17c[1] - 0.0f;
            f17c[2] = f17c[2] - 0.0f;
            host.widget_vcall_44_set_float(*panel(icon, 10, host), 0.0f);
            break;
        case 1: {
            const float f124 = -(frame_w + 0.0f);
            f17c[0] = f17c[0] - f124;
            f17c[1] = f17c[1] - 0.0f;
            f17c[2] = f17c[2] - 0.0f;
            host.widget_vcall_44_set_float(*panel(icon, 10, host), -1.57f);  // 00CF0AAC
            break;
        }
        case 2: {
            const float f120 = -(frame_h + 0.0f);
            f17c[0] = f17c[0] - 0.0f;
            f17c[1] = f17c[1] - f120;
            f17c[2] = f17c[2] - 0.0f;
            host.widget_vcall_44_set_float(*panel(icon, 10, host), 1.57f);  // 00CF0B40
            break;
        }
        case 3: {
            const float f150x = frame_w + 0.0f;
            const float f14cy = frame_h + 0.0f;
            f17c[0] = f150x + f17c[0];
            f17c[1] = f14cy + f17c[1];
            f17c[2] = 0.0f + f17c[2];
            host.widget_vcall_44_set_float(*panel(icon, 10, host), 3.14f);  // 00CF0AA8
            break;
        }
        default:
            break;
        }
        float f124[3];
        f124[0] = f17c[0] + 0.0f;
        f124[1] = 0.0f + f17c[1];
        f124[2] = f17c[2] + z;
        host.widget_set_resolved_position_00aa8240(*panel(icon, 10, host), f124);
        host.widget_vcall_34_set_visible(*panel(icon, 10, host), at<Byte>(icon, 0x2A));
        return;
    }
    if (state == 5) {
        // 005B2520: close: state 6 once the frame is gone or has shrunk away.
        if (*panel(icon, 0, host) == nullptr || host.hud_map_icon_update_size_and_visibility_0059cef0(icon, true)) {
            at<std::int32_t>(icon, 0x118) = 6;
        }
    } else if (state == 6) {
        host.hud_map_icon_release_widgets_005a1c60(icon, 0);
    }
}

}  // namespace bsp
