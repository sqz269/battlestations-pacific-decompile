// cc13_seg48U: undone segment 40 and 48 bodies with no cc9 host (packet
// cc13-seg48U, docs/CC13_SEG48U.md). Names are hypotheses; see the header for
// the hooks. Each body was transcribed from its full export listing.

#include "bsp/cc13_seg48U.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg48_at<T>(base, offset);
}

template <class T>
T at(const void* base, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof(T));
    return value;
}

void* game_pointer(Cc13Seg48UHost& host) {
    return *reinterpret_cast<void**>(host.image_global(0x00E188A8));
}

// FLD float / FSTP float: exact for every value except a signalling NaN, which the
// x87 load quiets (sets mantissa bit 22).
float x87_float_round_trip(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0) bits |= 0x00400000u;
    std::memcpy(&value, &bits, sizeof(bits));
    return value;
}

// The narrative senders' local echo: a native string built from "" (00CE3A0C),
// 00734870(narrative, text, &string, 0), then the string's block returned to the
// sized pool (00419CC0, 00BD1510(data, size + 1, 1)) when its data is non-null.
// The string is {size, data}; 0041E870 constructs it in place.
void append_local_echo(void* narrative, const void* text, Cc13Seg48UHost& host) {
    struct NativeString {
        std::uint32_t size;
        char* data;
    } string{0, nullptr};
    host.native_string_assign_0041e870(&string, "");  // 00CE3A0C is the empty string
    host.narrative_list_append_00734870(narrative, text, &string, 0);
    if (string.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), string.data, string.size + 1, 1);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 0072BC80 BSP_GunBot_SetSkillIndex. __thiscall(ECX = bot, index), RET 4.
// Evidence: export listing 0072BC80..0072BC86 ([ECX+34h] = index).
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_bot_set_skill_index_0072bc80(void* bot, std::int32_t index) {
    at<std::int32_t>(bot, 0x34) = index;
}

// ---------------------------------------------------------------------------
// 0072BD10 BSP_GunBot_HasFireTarget. __thiscall(ECX = bot), RET; AL.
// Evidence: export listing 0072BD10: byte +39h, then byte +38h.
// ---------------------------------------------------------------------------
bool cc13_seg48U_gun_bot_has_fire_target_0072bd10(const void* bot) {
    return at<std::uint8_t>(bot, 0x39) != 0 || at<std::uint8_t>(bot, 0x38) != 0;
}

// ---------------------------------------------------------------------------
// 00731050 BSP_Gun_CurrentModeRecord / 00731040 BSP_Gun_CurrentBullet.
// __thiscall(ECX = gun), RET; EAX. Evidence: export listings. Both return null
// when the mode count +78h is zero; otherwise +74h, or [[+74h]+34h].
// ---------------------------------------------------------------------------
void* cc13_seg48U_gun_current_mode_record_00731050(const void* gun) {
    if (at<std::int32_t>(gun, 0x78) == 0) return nullptr;
    return at<void*>(gun, 0x74);
}

void* cc13_seg48U_gun_current_bullet_00731040(const void* gun) {
    if (at<std::int32_t>(gun, 0x78) == 0) return nullptr;
    return at<void*>(at<void*>(gun, 0x74), 0x34);
}

// ---------------------------------------------------------------------------
// 00729FA0 BSP_Gun_OnActivated. __thiscall(ECX = gun), RET. Evidence: export
// listing 00729FA0..00729FCB.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_on_activated_00729fa0(void* gun, Cc13Seg48UHost& host) {
    host.device_entity_on_killed_lua_00440f50(gun);  // 00729FA3
    void* const unit = at<void*>(gun, 0x3F0);
    if (unit == nullptr) return;
    if (at<std::uint8_t>(unit, 0x5D) == 0 && at<std::uint8_t>(unit, 0x60) == 0) {
        host.unit_rebuild_weapon_category_index_00956c20(unit);  // 00729FBF tail jump
        return;
    }
    host.unit_remove_gun_00956ba0(unit, gun);  // 00729FC5
}

// ---------------------------------------------------------------------------
// 0072D7E0 BSP_GunBase_InitDevice. __thiscall(ECX = gun), RET. Evidence: export
// listing 0072D7E0..0072D821. The mode count is [[gun+3F4h]+78h] (signed test).
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_base_init_device_0072d7e0(void* gun, Cc13Seg48UHost& host) {
    host.device_entity_place_in_world_lua_004426c0(gun);  // 0072D7E3
    const std::int32_t modes = at<std::int32_t>(at<void*>(gun, 0x3F4), 0x78);
    if (modes > 0) {
        host.native_vector14_resize_0072b6d0(static_cast<std::uint8_t*>(gun) + 0x3A8, modes);  // 0072D7FC
        host.gun_select_fire_mode_0072d5b0(gun, 0, 1);                                         // 0072D807
    }
    at<void*>(gun, 0x3F0) = at<void*>(gun, 0x3C);
}

// ---------------------------------------------------------------------------
// 0072BE40 BSP_GunBot_ConstructMuzzleClass. __thiscall(ECX = bot), RET; EAX = bot.
// Evidence: export listing 0072BE40..0072BECA. The three floats at 00F87574 are
// moved with MOVSS (bit copies); +90h..+98h are FLD/FSTP float copies of
// +84h..+8Ch; +B4h = [00D7A260] = -1.0f (image bytes 00 00 80 BF).
// ---------------------------------------------------------------------------
void* cc13_seg48U_gun_bot_construct_muzzle_class_0072be40(void* bot, Cc13Seg48UHost& host) {
    host.gun_bot_construct_base_0072bbd0(bot, *host.image_global(0x00E19990));  // 0072BE49
    at<std::uint32_t>(bot, 0x00) = 0x00CFE000;
    at<std::uint32_t>(bot, 0x1C) = 0x00CFDFE8;
    at<std::uint32_t>(bot, 0x58) = 0;
    at<std::uint8_t>(bot, 0x7C) = 0;
    at<std::uint32_t>(bot, 0x84) = *host.image_global(0x00F87574);
    at<std::uint32_t>(bot, 0x88) = *host.image_global(0x00F87578);
    at<std::uint32_t>(bot, 0x8C) = *host.image_global(0x00F8757C);
    at<float>(bot, 0x90) = x87_float_round_trip(at<float>(bot, 0x84));
    at<float>(bot, 0x94) = x87_float_round_trip(at<float>(bot, 0x88));
    at<float>(bot, 0x98) = x87_float_round_trip(at<float>(bot, 0x8C));
    at<float>(bot, 0xB4) = -1.0f;
    return bot;
}

// ---------------------------------------------------------------------------
// 00729670 BSP_GunShotDecision_LineOfFireClear. __thiscall(ECX = decision, arg),
// RET 4; AL. Evidence: export listing 00729670..007296F4. gun = [decision+4h];
// its pose is refreshed (00414DB0) unless byte +C8h is set; the gun position
// +FCh..+104h is copied to a local; range = [[[gun+3F8h]+34h]+50h] through
// FLD/FSTP float; the query is vtable slot 1 of [decision+8h] with
// (&position, [gun+3F0h], arg, range). Result: SETZ (clear when it returns 0).
// ---------------------------------------------------------------------------
bool cc13_seg48U_shot_decision_line_of_fire_clear_00729670(void* decision, std::uint32_t arg, Cc13Seg48UHost& host) {
    void* const gun = at<void*>(decision, 0x4);
    if (at<std::uint8_t>(gun, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(gun);  // 00729685
    float position[3];
    position[0] = at<float>(gun, 0xFC);
    position[1] = at<float>(gun, 0x100);
    position[2] = at<float>(gun, 0x104);
    void* const gun2 = at<void*>(decision, 0x4);  // 00729692 reloads [decision+4h]
    const float range = x87_float_round_trip(at<float>(at<void*>(at<void*>(gun2, 0x3F8), 0x34), 0x50));
    const std::uint8_t blocked = host.line_of_fire_query_vslot1(at<void*>(decision, 0x8), position,
                                                                at<void*>(gun2, 0x3F0), arg, range);  // 007296E8
    return blocked == 0;  // 007296ED SETZ
}

// ---------------------------------------------------------------------------
// 00729700 BSP_Gun_ApplyDescriptorNodeFlag. __thiscall(ECX = gun), RET. Evidence:
// export listing 00729700..007297A6. Class byte +84h drives flag 4 and byte +85h
// flag 2 on the descriptor [gun+3BCh] (+48h) and on every node of its chain
// (+34h, next +3Ch): set through 00475E60, cleared through 00728340.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_apply_descriptor_node_flags_00729700(void* gun, Cc13Seg48UHost& host) {
    struct Flag {
        std::size_t class_byte;
        std::uint32_t bit;
    };
    constexpr Flag kFlags[] = {{0x84, 4}, {0x85, 2}};
    for (const Flag& flag : kFlags) {
        const bool set = at<std::uint8_t>(at<void*>(gun, 0x3F4), flag.class_byte) != 0;
        void* const descriptor = at<void*>(gun, 0x3BC);
        void* node = at<void*>(descriptor, 0x34);  // read before the flag update
        if (set) {
            at<std::uint32_t>(descriptor, 0x48) |= flag.bit;
            for (; node != nullptr; node = at<void*>(node, 0x3C)) host.node_or_flags_recursive_00475e60(node, flag.bit);
        } else {
            at<std::uint32_t>(descriptor, 0x48) &= ~flag.bit;
            for (; node != nullptr; node = at<void*>(node, 0x3C)) host.scene_node_clear_flags_recursive_00728340(node, flag.bit);
        }
    }
}

// ---------------------------------------------------------------------------
// 00731AE0 BSP_GunShotCommand_ReadAndCreate. __thiscall(ECX = save reader), RET;
// EAX = the new shot command or null. SEH 00C859B3 (releases the local string;
// not modelled). Evidence: export listing 00731AE0..00731C0C. Only caller:
// 0072F060 BSP_Gun_LoadSavedState (the save-load path).
// The "type" field (00CE4AD0) is read into a local native string {size, data}:
// - "flak" (00CFE738, stricmp 00BF7FBF, only when data is non-null): an 8-byte
//   command, vtable 00CFE4F8, +4h = 0.0f (XORPS / MOVSS);
// - otherwise, when 00425850 matches "torpedo" (00CFB414): a 24h-byte command
//   built by 007311B0(0.0f (FLDZ), [00D0C314] = -2.5f, null).
// The result's vtable slot 1 then reads the rest from the reader. The image
// makes that call on a null result as well (neither type, or operator new
// 00BF681B returning null); the call is kept, not guarded.
// ---------------------------------------------------------------------------
void* cc13_seg48U_gun_shot_command_read_and_create_00731ae0(void* reader, Cc13Seg48UHost& host) {
    struct NativeString {
        std::uint32_t size;
        char* data;
    } type{0, nullptr};
    void* command = nullptr;
    host.archive_vcall_10_get(reader, 0, "type", 0, &type);  // 00731B38: records {0, "type"}, {0, &type}
    if (type.data != nullptr && host.crt_stricmp_00bf7fbf(type.data, "flak") == 0) {
        command = host.operator_new_00bf681b(8);  // 00731B5B
        if (command != nullptr) {
            at<std::uint32_t>(command, 0x0) = 0x00CFE4F8;
            at<float>(command, 0x4) = 0.0f;
        }
    } else if (host.native_string_equals_ci_00425850(&type, "torpedo")) {  // 00731B84
        command = host.operator_new_00bf681b(0x24);                       // 00731B8F
        if (command != nullptr) {
            host.torpedo_shot_command_construct_007311b0(command, 0.0f, -2.5f, nullptr);  // 00731BB9
        }
    }
    host.shot_command_read_vslot1(command, reader);  // 00731BD0
    if (type.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), type.data, type.size + 1, 1);
    }
    return command;
}

// ---------------------------------------------------------------------------
// 007323E0 BSP_GunClassEffectRecord_Destruct. __thiscall(ECX = record), RET. SEH
// 00C85AAD (states 6..0 mirror the member order; not modelled). Evidence: export
// listing 007323E0..00732505 and disasm-raw 00732505..0073255B (Ghidra's body stops
// at 00732505). Each reference release is InterlockedDecrement([00CE2220]) on +4h,
// then vtable slot 0 when it reaches zero, then the slot cleared.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_class_effect_record_destruct_007323e0(void* record, Cc13Seg48UHost& host) {
    auto release = [&](std::size_t offset) {
        void* const object = at<void*>(record, offset);
        if (object == nullptr) return;
        if (host.interlocked_decrement(&at<std::int32_t>(object, 4)) == 0) host.vcall_00_release(object);
        at<void*>(record, offset) = nullptr;
    };
    at<std::uint32_t>(record, 0x0) = 0x00CE4530;
    // 00732408..00732426: the effect +38h is tested through a safe-bool conversion
    // (NEG/SBB/TEST against the member address 00E186EC), i.e. non-null.
    if (void* const effect = at<void*>(record, 0x38); effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);  // 0073242A
        at<std::uint8_t>(effect, 0x9) = 1;
        release(0x38);
        at<void*>(record, 0x38) = nullptr;  // 0073244F stores it again
    }
    // 00732452..0073247B: the container at +3Ch is emptied (00441E50 over
    // {+3Ch, [[+40h]]} .. {+3Ch, [+40h]}), its head node +40h freed (00BF65AC)
    // and +40h / +44h cleared.
    std::uint8_t* const container = static_cast<std::uint8_t*>(record) + 0x3C;
    void* const head = at<void*>(container, 0x4);
    std::uint32_t out_iterator[2] = {};
    host.tree_erase_range_00441e50(container, out_iterator, container, at<void*>(head, 0x0), container, head);
    host.crt_free_00bf65ac(at<void*>(container, 0x4));
    at<void*>(container, 0x4) = nullptr;
    at<std::uint32_t>(container, 0x8) = 0;
    for (const std::size_t offset : {std::size_t{0x38}, std::size_t{0x34}, std::size_t{0x18}, std::size_t{0x14},
                                     std::size_t{0x10}, std::size_t{0x0C}}) {
        release(offset);
    }
}

// ---------------------------------------------------------------------------
// 00733C80 BSP_NarrativeEntry_CopyConstruct. __thiscall(ECX = entry, const entry*
// source), RET 4; EAX = entry. SEH 00C85C6B (not modelled). Evidence: export
// listing 00733C80..00733D48. vtable 00CFE960; the native strings {size, data} at
// +4h and +0Ch are zeroed and, unless they are the source's own, resized to the
// source size (0041DD40(.., size, 1)) and, when that size is non-zero, filled with
// memcpy(data, source data, resized size); +14h is copied.
// ---------------------------------------------------------------------------
void* cc13_seg48U_narrative_entry_copy_construct_00733c80(void* entry, const void* source, Cc13Seg48UHost& host) {
    at<std::uint32_t>(entry, 0x0) = 0x00CFE960;
    for (const std::size_t offset : {std::size_t{0x4}, std::size_t{0xC}}) {
        std::uint8_t* const string = static_cast<std::uint8_t*>(entry) + offset;
        const std::uint8_t* const from = static_cast<const std::uint8_t*>(source) + offset;
        at<std::uint32_t>(string, 0x0) = 0;
        at<void*>(string, 0x4) = nullptr;
        if (string == from) continue;
        host.native_string_resize_0041dd40(string, at<std::uint32_t>(from, 0x0), 1);
        if (at<std::uint32_t>(from, 0x0) == 0) continue;
        host.crt_memcpy_00bf7680(at<void*>(string, 0x4), at<void*>(from, 0x4), at<std::uint32_t>(string, 0x0));
    }
    at<std::uint32_t>(entry, 0x14) = at<std::uint32_t>(source, 0x14);
    return entry;
}

// ---------------------------------------------------------------------------
// 00729D30 BSP_Gun_RebuildBarrelBuffers. __thiscall(ECX = gun), RET. No SEH.
// Evidence: export listing 00729D30..00729F02. Records: [gun+3A8h], count +3ACh,
// 14h bytes each; c = gun+43Ch, row = gun+440h. For each record with k = +8h and
// n = k*c (IMUL): +0Ch is freed (_free 00BF6989) and cleared; +10h = new(n*4); a
// scratch grid g = new(c*n*4) is zeroed (memset 00BF79F0); g[j*(k+n)] = 3 for
// j < c; then for each m < n with m % k != 0 (IDIV), the first block row j whose
// c cells starting at (m/c)*c + j*n are all zero gets g[j*n + m] = 1; finally
// row `row` of g (n dwords) is copied to +10h and g is freed with _free. operator
// new 00BF55BE takes the MUL-by-4 size, or FFFFFFFFh when it overflows.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_rebuild_barrel_buffers_00729d30(void* gun, Cc13Seg48UHost& host) {
    auto checked_bytes = [](std::int32_t count) {
        const std::uint64_t bytes = static_cast<std::uint64_t>(static_cast<std::uint32_t>(count)) * 4u;
        return bytes > 0xFFFFFFFFu ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
    };
    auto cell = [](void* grid, std::int32_t index) -> std::int32_t& {
        return at<std::int32_t>(grid, static_cast<std::size_t>(static_cast<std::uint32_t>(index) * 4u));
    };
    if (at<std::int32_t>(gun, 0x440) < 0) return;
    for (std::int32_t i = 0; i < at<std::int32_t>(gun, 0x3AC); ++i) {
        const std::size_t record = static_cast<std::size_t>(i) * 0x14;  // [ESP+18h] advances by 14h
        auto field = [&](std::size_t offset) -> void*& {
            return at<void*>(at<std::uint8_t*>(gun, 0x3A8) + record, offset);
        };
        if (field(0xC) != nullptr) {
            host.crt_free_00bf6989(field(0xC));  // 00729D75
            field(0xC) = nullptr;
        }
        const std::int32_t n = at<std::int32_t>(at<std::uint8_t*>(gun, 0x3A8) + record, 0x8) * at<std::int32_t>(gun, 0x43C);
        field(0x10) = host.operator_new_00bf55be(checked_bytes(n));  // 00729DA7
        void* const grid = host.operator_new_00bf55be(checked_bytes(at<std::int32_t>(gun, 0x43C) * n));  // 00729DD0
        host.crt_memset_00bf79f0(grid, 0, static_cast<std::uint32_t>(at<std::int32_t>(gun, 0x43C) * n) * 4u);  // 00729DE8
        const std::int32_t k = at<std::int32_t>(at<std::uint8_t*>(gun, 0x3A8) + record, 0x8);
        for (std::int32_t j = 0, index = 0; j < at<std::int32_t>(gun, 0x43C); ++j, index += k + n) cell(grid, index) = 3;
        for (std::int32_t m = 0; m < n; ++m) {
            if (m % k == 0) continue;  // 00729E34 IDIV [k]
            const std::int32_t c = at<std::int32_t>(gun, 0x43C);
            std::int32_t base = (m / c) * c;  // 00729E45 IDIV ECX, IMUL
            for (std::int32_t j = 0; j < c && j < at<std::int32_t>(gun, 0x43C); ++j) {
                bool occupied = false;
                for (std::int32_t a = 0; a < at<std::int32_t>(gun, 0x43C); ++a) {
                    if (cell(grid, base + a) != 0) occupied = true;
                    if (occupied) break;
                }
                if (!occupied) {
                    cell(grid, j * n + m) = 1;  // 00729E99
                    break;
                }
                base += n;  // 00729E7B
            }
        }
        host.crt_memcpy_00bf7680(field(0x10),
                                 &cell(grid, at<std::int32_t>(gun, 0x440) * n),
                                 static_cast<std::uint32_t>(n) * 4u);  // 00729ECF
        host.crt_free_00bf6989(grid);                                 // 00729ED5
    }
}

// ---------------------------------------------------------------------------
// 0072D950 BSP_Gun_Destruct. __thiscall(ECX = gun), RET. SEH 00C8574C (states 6..0
// mirror the member order; not modelled). Evidence: export listing
// 0072D950..0072DBEF.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_destruct_0072d950(void* gun, Cc13Seg48UHost& host) {
    at<std::uint32_t>(gun, 0x000) = 0x00CFE0A8;
    at<std::uint32_t>(gun, 0x010) = 0x00CFE08C;
    at<std::uint32_t>(gun, 0x024) = 0x00CFE084;
    at<std::uint32_t>(gun, 0x170) = 0x00CFE080;
    at<std::uint32_t>(gun, 0x1E4) = 0x00CFE078;
    at<std::uint32_t>(gun, 0x310) = 0x00CFE060;
    // 0072D9A5..0072D9C7: the effect +47Ch (safe-bool test against 00E186EC).
    if (void* const effect = at<void*>(gun, 0x47C); effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 0x9) = 1;
    }

    // 0072D9D1..0072DAD8: the pointer vector at +3E0h {+4h first, +8h last} is
    // drained from the back. Each back() read is preceded by the checked-iterator
    // tests (00BF6713 _invalid_parameter_noinfo on failure).
    std::uint8_t* const vec = static_cast<std::uint8_t*>(gun) + 0x3E0;
    auto first = [&] { return reinterpret_cast<std::uintptr_t>(at<void*>(vec, 0x4)); };
    auto last = [&] { return reinterpret_cast<std::uintptr_t>(at<void*>(vec, 0x8)); };
    auto checked_back = [&]() -> void*& {
        const std::uintptr_t end = last();
        if (first() > end) host.crt_invalid_parameter_noinfo_00bf6713();
        const std::uintptr_t slot = end - 4;
        if (slot > last() || slot < first()) host.crt_invalid_parameter_noinfo_00bf6713();
        if (!(slot < last())) host.crt_invalid_parameter_noinfo_00bf6713();
        return *reinterpret_cast<void**>(slot);
    };
    while (first() != 0 && ((last() - first()) >> 2) != 0) {
        if (checked_back() != nullptr) {
            void* const element = checked_back();
            void* const reference = at<void*>(element, 0xC);  // 0072DA5D, before the third check
            void*& slot = checked_back();
            if (slot != nullptr) {
                host.vcall_04_delete(slot, 1);  // 0072DA97
                slot = nullptr;
            }
            // 0072DA9F: the reference is released without a null test.
            if (host.interlocked_decrement(&at<std::int32_t>(reference, 4)) == 0) host.vcall_00_release(reference);
        }
        if (first() == 0 || ((last() - first()) >> 2) == 0) continue;  // 0072DAB7..0072DACC
        at<std::uintptr_t>(vec, 0x8) = last() - 4;                          // 0072DAD5 pop_back
    }

    if (void* const owned = at<void*>(gun, 0x41C); owned != nullptr) {
        host.vcall_00_delete(owned, 1);  // 0072DAED
        at<void*>(gun, 0x41C) = nullptr;
    }
    for (const std::size_t offset : {std::size_t{0x414}, std::size_t{0x418}}) {
        if (void* const block = at<void*>(gun, offset); block != nullptr) {
            host.crt_free_00bf6989(block);  // 0072DB00 / 0072DB19
            at<void*>(gun, offset) = nullptr;
        }
    }
    if (void* const effect = at<void*>(gun, 0x47C); effect != nullptr) {
        if (host.interlocked_decrement(&at<std::int32_t>(effect, 4)) == 0) host.vcall_00_release(effect);  // 0072DB3A
        at<void*>(gun, 0x47C) = nullptr;
    }
    host.observer_link_destroy_base_004b7ef0(static_cast<std::uint8_t*>(gun) + 0x458);  // 0072DB5D
    host.gun_effect_set_destruct_00729600(static_cast<std::uint8_t*>(gun) + 0x424);     // 0072DB6D
    if (void* const storage = at<void*>(vec, 0x4); storage != nullptr) host.crt_free_00bf65ac(storage);  // 0072DB7A
    at<std::uint32_t>(vec, 0x4) = 0;
    at<std::uint32_t>(vec, 0x8) = 0;
    at<std::uint32_t>(vec, 0xC) = 0;
    if (void* const storage = at<void*>(gun, 0x3D4); storage != nullptr) host.crt_free_00bf65ac(storage);  // 0072DB96
    at<std::uint32_t>(gun, 0x3D4) = 0;
    at<std::uint32_t>(gun, 0x3D8) = 0;
    at<std::uint32_t>(gun, 0x3DC) = 0;
    host.native_vector14_resize_0072b6d0(static_cast<std::uint8_t*>(gun) + 0x3A8, 0);  // 0072DBBE
    host.crt_free_00bf6989(at<void*>(gun, 0x3A8));                                      // 0072DBC6, no null test
    host.unit_tickable_entity_destruct_0087a410(gun);                                    // 0072DBD8
}

// ---------------------------------------------------------------------------
// 0072C170 BSP_Gun_ComputeAimForTarget (cc13-seg48V). ECX = shooter (Lua arg 0),
// EDX = device id (Lua arg 1, __ftol); stack (target, point_a*, point_b*, float speed,
// flag dword); RET 14h. Its only caller is the Lua native HitTarget 00892F70, at
// 008931DE. Evidence: the disk-byte disassembly 0072C170..0072C692 (bsp.py disasm-raw),
// with every x87 operand and direction read from the bytes.
// The device class's bullet ([class+74h]+34h) is fired from point_a toward point_b.
// Both points are local to the target and taken through its world matrix (+CCh).
// The flight is stepped at 1/(speed[bullet+5Ch] * 30) s under 9.81 gravity until it
// falls below point_b's height. The landing miss and the target's motion over the
// flight time are folded into the aim. The round is spawned through the bullet
// class's slot +20h and given speed * the aim direction.
// Precision: the main thread runs the x87 at 24-bit precision control. Every double
// constant here is float-exact (30.0, 0.01f, 9.81f, 0.5, 1.0, 1/30 as 0.033333335f),
// so float arithmetic rounds each operation as the image does, the exponent range
// aside. The flag dword's low byte is HitTarget's boolean; its upper three bytes are
// whatever the caller's stack slot held (008931B2 stores only AL), and they are
// passed through.
// ---------------------------------------------------------------------------
void cc13_seg48U_gun_compute_aim_for_target_0072c170(void* shooter, std::int32_t device_id, void* target,
                                                     const float* point_a, const float* point_b, float speed,
                                                     std::uint32_t flag_dword, Cc13Seg48UHost& host) {
    void* const device_class =
        host.device_class_resolve_and_activate_00443490(static_cast<std::uint32_t>(device_id), 0);  // 0072C17D, DL = 0
    void* const bullet = at<void*>(at<void*>(device_class, 0x74), 0x34);
    if (bullet == nullptr) return;                                                                   // 0072C18A
    const float* const target_world = reinterpret_cast<const float*>(static_cast<std::uint8_t*>(target) + 0xCC);
    if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);       // 0072C1A1
    float a[3];
    host.vector3f_transform_affine_point_004142e0(point_a, a, target_world);                         // 0072C1B6
    if (at<std::uint8_t>(target, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(target);       // 0072C1EA
    float b[3];
    host.vector3f_transform_affine_point_004142e0(point_b, b, target_world);                         // 0072C1F9

    const float dt = 1.0f / (at<float>(bullet, 0x5C) * 30.0f);    // 0072C1FE..0072C213, [00CE7630] = 30.0
    const float d[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};   // 0072C217..0072C237
    float normal_out[3];
    const float* const n = host.vector3f_normalize_00419510(normal_out, d);  // 0072C23B
    const float vx = speed * n[0];                                          // 0072C281
    float vy = speed * n[1];                                                // 0072C25B
    const float vz = n[2] * speed;                                          // 0072C27B
    float px = a[0];
    float py = a[1];
    float pz = a[2];
    float t = 0.0f;                                // 0072C28B
    float prev_y = a[1] + 0.01f;                   // 0072C2A1..0072C2A9, [00D7A358] = 0.01f
    if (a[1] > b[1]) {                             // 0072C2B3 FCOMIP, JBE skips (unordered too)
        const float step_x = vx * dt;              // 0072C2C3
        const float step_z = vz * dt;              // 0072C2CB, FMULP
        const float gdt = dt * 9.81f;              // 0072C2D5, [00CF9058] = 9.81f, kept in ST
        const float drop = dt * gdt * 0.5f;        // 0072C2DD..0072C2DF, [00D7A280], kept in ST
        do {
            prev_y = py;                           // 0072C2F5
            const float step_y = vy * dt;          // 0072C2FB
            px = step_x + px;                      // 0072C301..0072C309
            py = py + step_y;                      // 0072C311..0072C315
            pz = step_z + pz;                      // 0072C319..0072C321
            py = py - drop;                        // 0072C325..0072C32B
            vy = vy - gdt;                         // 0072C32F..0072C331
            t = t + 1.0f;                          // 0072C335..0072C33F, [00D7A210]
        } while (py > b[1]);                       // 0072C347 FCOMI, JA (unordered leaves)
    }
    // FDIVRP at 0072C379: (py - b.y) / (py - prev_y), the fraction of the last step
    // that fell below b.y.
    const float frac = (py - b[1]) / (py - prev_y);
    const float ex = vx * dt;                      // 0072C385
    const float ez = vz * dt;                      // 0072C38B
    px = px - ex * frac;                           // 0072C39B, 0072C3AD..0072C3B5
    pz = pz - ez * frac;                           // 0072C3A7, 0072C3B9..0072C3C1
    t = t - frac;                                  // 0072C3C5 FSUBR
    const float time = t * (1.0f / 30.0f);         // 0072C3D1, [00CFE058] = 0.033333335f
    float velocity_out[3];
    const float* v = host.entity_vcall_34_vector(target, velocity_out);  // 0072C3DB
    {
        const float mx = time * v[0];              // 0072C3EC
        const float my = v[1] * time;              // 0072C3F7
        const float mz = time * v[2];              // 0072C3FD
        a[0] = mx + a[0];                          // 0072C409..0072C411
        a[1] = my + a[1];                          // 0072C415..0072C41D
        a[2] = mz + a[2];                          // 0072C421..0072C429
    }
    // The miss on the ground plane. The image computes the height term as b.y - b.y
    // (0072C439..0072C43D FSUBR ST0,ST0): 0, or NaN for a non-finite b.y.
    const float e[3] = {b[0] - px, b[1] - b[1], b[2] - pz};   // 0072C42D..0072C44B
    float velocity_out2[3];
    v = host.entity_vcall_34_vector(target, velocity_out2);   // 0072C44F, the same slot again
    float h[3];
    {
        const float mx = time * v[0];              // 0072C457
        const float my = v[1] * time;              // 0072C460
        const float mz = time * v[2];              // 0072C466
        const float f[3] = {mx + e[0], my + e[1], mz + e[2]};        // 0072C46D..0072C48D
        const float g[3] = {f[0] + b[0], f[1] + b[1], f[2] + b[2]};  // 0072C491..0072C4B1
        h[0] = g[0] - a[0];                        // 0072C4B5..0072C4BD
        h[1] = g[1] - a[1];                        // 0072C4C1..0072C4C9
        h[2] = g[2] - a[2];                        // 0072C4CD..0072C4D5
    }
    // 0072C4D9..0072C50F: (h.y^2 + h.x^2) + h.z^2, each square stored as a float.
    const float hy2 = h[1] * h[1];
    const float hx2 = h[0] * h[0];
    const float hz2 = h[2] * h[2];
    const float length = std::sqrt(hy2 + hx2 + hz2);      // 0072C517 _CIsqrt, FSTP float
    const float inverse = length > 0.0f ? 1.0f / length : 0.0f;  // 0072C52E FCOMI, JBE: 0 for <= 0 or NaN
    float u[3] = {h[0] * inverse, h[1] * inverse, h[2] * inverse};  // 0072C55D, 0072C576, 0072C584
    void* const projectile = host.bullet_class_vcall_20_spawn(bullet, 0, at<std::uint32_t>(shooter, 0x54), a, u,
                                                              flag_dword, 0, 0);  // 0072C5A6
    if (projectile == nullptr) return;                                           // 0072C5AC
    // u is re-read from the buffer the spawn received (0072C5B2, 0072C5C8, 0072C5D2).
    at<float>(projectile, 0x08) = u[0] * speed;   // 0072C5C0, 0072C5DA..0072C5DE
    at<float>(projectile, 0x0C) = u[1] * speed;   // 0072C5CC, 0072C5E1..0072C5E5
    at<float>(projectile, 0x10) = speed * u[2];   // 0072C5D2, 0072C5E8..0072C5EC
    void* owner = nullptr;                         // 0072C693, a shooter that is not kind 0Fh
    void* source = shooter;
    if (host.vcall_5c_has_ordnance_kind(shooter, 0x0F)) {   // 0072C5F4, vtable +5Ch (IsKindOf)
        owner = shooter;                                     // 0072C60D
        // 0072C606 COMISS with [00D7A218] = 0.0f; JBE skips (unordered too).
        if (at<float>(shooter, 0x914) > 0.0f) source = at<void*>(at<void*>(shooter, 0x9D4), 0x3D0);
    }
    host.projectile_bind_owner_006e6bb0(projectile, owner);                                    // 0072C620
    at<std::uint32_t>(projectile, 0x14) =
        host.unit_sub170_vcall_0(static_cast<std::uint8_t*>(source) + 0x170);                  // 0072C633..0072C638
    at<std::uint32_t>(projectile, 0x18) = at<std::uint32_t>(source, 0x54);                     // 0072C643
    at<std::uint32_t>(projectile, 0x1C) = at<std::uint32_t>(source, 0x180);                    // 0072C650
    const float ones[3] = {1.0f, 1.0f, 1.0f};      // [00D7A24C]
    const float zeros[3] = {0.0f, 0.0f, 0.0f};
    host.projectile_vcall_34(projectile, ones, zeros);                                         // 0072C687
}

// ---------------------------------------------------------------------------
// 007D5AC0 BSP_Plane_CopyClassRecordLists. __thiscall(ECX = plane), RET. SEH
// 00C8D6E0 (states 0/1 release the local handles; not modelled). Evidence: export
// listing 007D5AC0..007D5D1F. The plane's 10h-byte effect records at +A4Ch
// {+4h first, +8h last} are erased (007CF280); then for each 10h-byte record
// {definition handle, x, y, z} of the class record [plane+538h]+560h a point
// effect is created under [game+19ECh] with an identity matrix (1.0f is
// [00D7A24C]) and pushed (007D1DC0) as {effect, x, y, z} with effect+9h = 0.
// The checked-iterator tests (00BF6713 on failure) are kept where the image has them.
// ---------------------------------------------------------------------------
void cc13_seg48U_plane_copy_class_record_lists_007d5ac0(void* plane, Cc13Seg48UHost& host) {
    auto ptr = [](void* base, std::size_t offset) { return reinterpret_cast<std::uintptr_t>(at<void*>(base, offset)); };
    auto release = [&](void*& handle) {
        if (handle == nullptr) return;
        if (host.interlocked_decrement(&at<std::int32_t>(handle, 4)) == 0) host.vcall_00_release(handle);
        handle = nullptr;
    };
    std::uint8_t* const effects = static_cast<std::uint8_t*>(plane) + 0xA4C;
    const std::uintptr_t effects_last = ptr(effects, 0x8);
    if (ptr(effects, 0x4) > effects_last) host.crt_invalid_parameter_noinfo_00bf6713();
    const std::uintptr_t effects_first = ptr(effects, 0x4);
    if (effects_first > ptr(effects, 0x8)) host.crt_invalid_parameter_noinfo_00bf6713();
    std::uint32_t out_iterator[2] = {};
    host.effect_record_vector_erase_007cf280(effects, out_iterator, effects,
                                             reinterpret_cast<void*>(effects_first), effects,
                                             reinterpret_cast<void*>(effects_last));  // 007D5B0D

    void* const record_list = static_cast<std::uint8_t*>(at<void*>(plane, 0x538)) + 0x560;  // EBX, fixed
    std::uintptr_t cursor = ptr(at<void*>(plane, 0x538), 0x564);
    if (cursor > ptr(at<void*>(plane, 0x538), 0x568)) host.crt_invalid_parameter_noinfo_00bf6713();
    for (;;) {
        void* const record = at<void*>(plane, 0x538);
        const std::uintptr_t last = ptr(record, 0x568);
        if (ptr(record, 0x564) > last) host.crt_invalid_parameter_noinfo_00bf6713();
        if (record_list != static_cast<std::uint8_t*>(record) + 0x560) host.crt_invalid_parameter_noinfo_00bf6713();
        if (cursor == last) break;

        struct EffectRecord {
            void* effect;
            float x, y, z;
        } entry{nullptr, 0.0f, 0.0f, 0.0f};  // [ESP+20h]
        float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        void* const group = at<void*>(game_pointer(host), 0x19EC);
        if (!(cursor < ptr(record_list, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        void* definition = *reinterpret_cast<void**>(cursor);  // by-value copy: retained
        if (definition != nullptr) host.interlocked_increment(&at<std::int32_t>(definition, 4));
        void* created = nullptr;  // [ESP+10h]
        void** const out = host.point_effect_create_from_matrix_00868420(&created, group, definition, matrix, 0, 0);  // 007D5C31
        if (*out != nullptr) {
            entry.effect = *out;
            host.interlocked_increment(&at<std::int32_t>(entry.effect, 4));  // 007D5C46
        }
        release(created);  // 007D5C4C..007D5C71
        if (!(cursor < ptr(record_list, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        entry.x = *reinterpret_cast<const float*>(cursor + 0x4);
        entry.y = *reinterpret_cast<const float*>(cursor + 0x8);
        entry.z = *reinterpret_cast<const float*>(cursor + 0xC);
        if (entry.effect != nullptr) at<std::uint8_t>(entry.effect, 0x9) = 0;  // safe-bool test, 00E186EC
        host.effect_record_vector_push_back_007d1dc0(static_cast<std::uint8_t*>(plane) + 0xA4C, &entry);  // 007D5CC4
        release(entry.effect);  // 007D5CC9..007D5CEF
        if (!(cursor < ptr(record_list, 0x8))) host.crt_invalid_parameter_noinfo_00bf6713();
        cursor += 0x10;
    }
}

// ---------------------------------------------------------------------------
// 007F35C0 BSP_PlaneSquadron_GetDisplayName. __thiscall(ECX = squadron), RET; EAX =
// const char*. SEH 00C8F550 (states 0..5 release the six local strings; not
// modelled). Evidence: export listing 007F35C0..007F3817. 00F87445 is the empty
// string. With byte +358h set: +160h when +15Ch is non-zero, else +158h (empty
// when null). Otherwise the function-static native string at 00F87448 {size, data}
// (guard bit 0 of 00F87450; zeroed and _atexit(00CDDE70) on first use) is set to
// ((("." ConcatInt [+3CCh]) + "x|") + name), name = +160h (or empty) when +15Ch is
// non-zero, else [[+35Ch]+54h]; it returns that string's data, or the empty string.
// The literals are 00CE3A70 ".", 00D08B40 "x|".
// ---------------------------------------------------------------------------
const char* cc13_seg48U_plane_squadron_display_name_007f35c0(void* squadron, Cc13Seg48UHost& host) {
    const char* const empty = reinterpret_cast<const char*>(host.image_global(0x00F87445));
    if (at<std::uint8_t>(squadron, 0x358) != 0) {
        const char* const text =
            at<std::uint32_t>(squadron, 0x15C) != 0 ? at<const char*>(squadron, 0x160) : at<const char*>(squadron, 0x158);
        return text != nullptr ? text : empty;
    }
    std::uint32_t& guard = *host.image_global(0x00F87450);
    std::uint32_t& static_size = *host.image_global(0x00F87448);
    std::uint32_t& static_data = *host.image_global(0x00F8744C);
    if ((guard & 1u) == 0) {
        guard |= 1u;
        static_size = 0;
        static_data = 0;
        host.crt_atexit_00bf6ff5(0x00CDDE70);  // 007F363F
    }
    const char* name = nullptr;
    if (at<std::uint32_t>(squadron, 0x15C) != 0) {
        name = at<const char*>(squadron, 0x160);
        if (name == nullptr) name = empty;
    } else {
        name = at<const char*>(at<void*>(squadron, 0x35C), 0x54);
    }
    struct NativeString {
        std::uint32_t size;
        char* data;
    };
    NativeString name_string{0, nullptr}, separator{0, nullptr}, prefix{0, nullptr};
    NativeString with_id{0, nullptr}, with_separator{0, nullptr}, full{0, nullptr};
    host.native_string_assign_0041e870(&name_string, name);  // 007F3670
    host.native_string_assign_0041e870(&separator, "x|");    // 007F3686, 00D08B40
    host.native_string_assign_0041e870(&prefix, ".");        // 007F3699, 00CE3A70
    void* const a = host.native_string_concat_int_004263b0(&prefix, &with_id, at<std::int32_t>(squadron, 0x3CC));  // 007F36B1
    void* const b = host.native_string_concat_004261a0(a, &with_separator, &separator);                              // 007F36C7
    const auto* const result = static_cast<const NativeString*>(host.native_string_concat_004261a0(b, &full, &name_string));  // 007F36DD
    if (static_cast<const void*>(result) != static_cast<const void*>(&static_size)) {  // CMP ESI, 00F87448
        host.native_string_resize_0041dd40(&static_size, result->size, 1);  // 007F36FB
        if (result->size != 0) {
            host.crt_memcpy_00bf7680(reinterpret_cast<void*>(static_cast<std::uintptr_t>(static_data)), result->data,
                                     static_size);  // 007F3717
        }
    }
    for (NativeString* string : {&full, &with_separator, &with_id, &prefix, &separator, &name_string}) {
        if (string->data != nullptr) {
            host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), string->data, string->size + 1, 1);
        }
    }
    return static_data != 0 ? reinterpret_cast<const char*>(static_cast<std::uintptr_t>(static_data)) : empty;
}

// ---------------------------------------------------------------------------
// 007E29A0 BSP_ObserverSet_Remove (cc13-seg48V). __thiscall(ECX = set, entity),
// RET 4. Evidence: the disk-byte disassembly 007E29A0..007E2A12. Its only caller is
// the jump thunk 007EB0D0. This is game code, not a template body (the first
// seg48U commit's stl_shape tag was removed). The set's pointer array is
// {+14h first, +18h count}. The entity's slot +4h names the observed object.
// 006952A0 drops the observer pair (ECX = observed, EDX = set). Then std::remove
// (007E1D30) compacts the array, and the count shrinks by (end - new end) / 4.
// 007E29D2..007E2A06 is the erase's tail move. It runs from the end to the end,
// which is the same address, so it copies nothing.
// ---------------------------------------------------------------------------
void cc13_seg48U_observer_set_remove_007e29a0(void* set, void* entity, Cc13Seg48UHost& host) {
    void* const observed = host.entity_vcall_4_observed(entity);                   // 007E29AE
    host.observer_unregister_pair_006952a0(observed, set);                          // 007E29B6
    void** const first = at<void**>(set, 0x14);
    void** const last = first + at<std::int32_t>(set, 0x18);                        // 007E29C1
    void* const value = observed;                                                   // 007E29C9, the argument slot
    void** const new_end = host.std_remove_pointer_007e1d30(first, last, &value);   // 007E29CD
    at<std::int32_t>(set, 0x18) -= static_cast<std::int32_t>(last - new_end);       // 007E2A07..007E2A0C, SAR 2
}

// ---------------------------------------------------------------------------
// 007337A0 BSP_MissionNarrative_Write. __thiscall(ECX = narrative, writer), RET 4.
// No SEH. Evidence: export listing 007337A0..00733ADF. Archive slots: +4h begin,
// +0Ch put({0, key}, {type, value}), +8h end; type 1 = int, 2 = float (bits),
// 0 = text, a null text written as [00E1AE72] (empty). Keys: 00CFE9B0 "title",
// 00CE7324 "state", 00CFE9A8 "fadeIn", 00CF76F8 "delay", 00CFE9A0 "fadeOut",
// 00CFE9B8 "level", 00CE49C0 "callback", 00CFE998 "status", 00CE37A8 "time",
// 00CFE98C "init", 00CFE980 "titleList", 00CF1010 "text". The title list entries
// are sections keyed {1, index}.
// ---------------------------------------------------------------------------
void cc13_seg48U_mission_narrative_write_007337a0(void* narrative, void* writer, Cc13Seg48UHost& host) {
    const char* const empty = reinterpret_cast<const char*>(host.image_global(0x00E1AE72));
    auto bits = [&](std::size_t offset) { return at<std::uint32_t>(narrative, offset); };
    auto text = [&](const void* base, std::size_t offset) {
        const char* const value = at<const char*>(base, offset);
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value != nullptr ? value : empty));
    };
    if (at<std::uint8_t>(narrative, 0x14) != 0) {
        host.archive_vcall_04(writer, 0, "title");
        host.archive_vcall_0c_put(writer, 0, "state", 1, bits(0x18));
        host.archive_vcall_0c_put(writer, 0, "fadeIn", 2, bits(0x1C));
        host.archive_vcall_0c_put(writer, 0, "delay", 2, bits(0x20));
        host.archive_vcall_0c_put(writer, 0, "fadeOut", 2, bits(0x24));
        host.archive_vcall_0c_put(writer, 0, "level", 2, bits(0x28));
        host.archive_vcall_0c_put(writer, 0, "callback", 0, text(narrative, 0x30));
        host.archive_vcall_08(writer);
    }
    if (at<std::uint8_t>(narrative, 0x3C) != 0) {
        host.archive_vcall_04(writer, 0, "status");
        host.archive_vcall_0c_put(writer, 0, "level", 2, bits(0x40));
        host.archive_vcall_0c_put(writer, 0, "time", 2, bits(0x44));
        host.archive_vcall_0c_put(writer, 0, "init", 2, bits(0x48));
        host.archive_vcall_0c_put(writer, 0, "callback", 0, text(narrative, 0x50));
        host.archive_vcall_08(writer);
    }
    host.archive_vcall_04(writer, 0, "titleList");
    // 007339FB..00733ACD: the list {+4h, head +8h}; node {+0h next, +8h entry}. The
    // iterator-container compare is CMP EBX,EBX (never fails); the dereference and
    // increment tests call 00BF6713 when the cursor is the head.
    void* const list = static_cast<std::uint8_t*>(narrative) + 0x4;
    void* node = at<void*>(at<void*>(narrative, 0x8), 0x0);
    for (std::uint32_t index = 0;; ++index) {
        if (node == at<void*>(list, 0x4)) break;
        if (node == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        host.archive_vcall_04(writer, 1, reinterpret_cast<const char*>(static_cast<std::uintptr_t>(index)));
        host.archive_vcall_0c_put(writer, 0, "text", 0, text(node, 0x10));
        host.archive_vcall_0c_put(writer, 0, "callback", 0, text(node, 0x18));
        host.archive_vcall_08(writer);
        if (node == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        node = at<void*>(node, 0x0);
    }
    host.archive_vcall_08(writer);
}

// ---------------------------------------------------------------------------
// 00734460 BSP_MissionNarrative_Read. __thiscall(ECX = narrative, reader), RET 4.
// SEH 00C85D68 (state 0 destroys the local entry; not modelled). Evidence: export
// listing 00734460..00734860. Archive slots: +14h has-section, +4h begin, +10h
// get({0, key}, {type, out*}), +8h end. The keys are those of 007337A0. "state" is
// read into a local, then stored; the rest are read in place. Each present
// {1, index} section of "titleList" appends an entry built from a local empty
// entry (vtable 00CFE960) through 007342B0 / 007341B0 / the push-back links, then
// reads its "text" and "callback" into the new node's strings (+0Ch, +14h).
// ---------------------------------------------------------------------------
void cc13_seg48U_mission_narrative_read_00734460(void* narrative, void* reader, Cc13Seg48UHost& host) {
    std::uint8_t* const base = static_cast<std::uint8_t*>(narrative);
    if (host.archive_vcall_14_has(reader, 0, "title")) {
        host.archive_vcall_04(reader, 0, "title");
        at<std::uint8_t>(narrative, 0x14) = 1;
        // "state" is read into [ESP+3Ch] (007344E7), the reader argument's own stack
        // slot (ESP0+4h). 007344B9 MOV [ESP+44h],ESP already overwrote that slot with
        // ESP0-40h, the address of the {0, "title"} begin-key record. So when the
        // archive has no "state", +18h receives that stale stack address, not the
        // reader pointer. The value cannot be reproduced exactly; the model starts
        // from the address of its own local, which is likewise a stack address.
        std::int32_t state = 0;
        state = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(&state));
        host.archive_vcall_10_get(reader, 0, "state", 1, &state);
        at<std::int32_t>(narrative, 0x18) = state;
        host.archive_vcall_10_get(reader, 0, "fadeIn", 2, base + 0x1C);
        host.archive_vcall_10_get(reader, 0, "delay", 2, base + 0x20);
        host.archive_vcall_10_get(reader, 0, "fadeOut", 2, base + 0x24);
        host.archive_vcall_10_get(reader, 0, "level", 2, base + 0x28);
        host.archive_vcall_10_get(reader, 0, "callback", 0, base + 0x2C);
        host.archive_vcall_08(reader);
    }
    if (host.archive_vcall_14_has(reader, 0, "status")) {
        host.archive_vcall_04(reader, 0, "status");
        at<std::uint8_t>(narrative, 0x3C) = 1;
        host.archive_vcall_10_get(reader, 0, "level", 2, base + 0x40);
        host.archive_vcall_10_get(reader, 0, "time", 2, base + 0x44);
        host.archive_vcall_10_get(reader, 0, "init", 2, base + 0x48);
        host.archive_vcall_10_get(reader, 0, "callback", 0, base + 0x4C);
        host.archive_vcall_08(reader);
    }
    host.archive_vcall_04(reader, 0, "titleList");
    void* const list = base + 0x4;
    for (std::uint32_t index = 0;; ++index) {
        const char* const key = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(index));
        if (!host.archive_vcall_14_has(reader, 1, key)) break;
        alignas(4) std::uint8_t entry[0x18] = {};
        at<std::uint32_t>(entry, 0x0) = 0x00CFE960;
        void* const head = at<void*>(list, 0x4);
        void* const node = host.narrative_list_buy_node_007342b0(list, head, at<void*>(head, 0x4), entry);  // 0073475F
        host.narrative_list_incsize_007341b0(list, 1);                                                      // 0073476C
        at<void*>(head, 0x4) = node;
        at<void*>(at<void*>(node, 0x4), 0x0) = node;
        host.narrative_entry_destruct_00733dc0(entry);  // 00734789
        void* const back = at<void*>(at<void*>(list, 0x4), 0x4);
        if (back == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        if (back == at<void*>(list, 0x4)) host.crt_invalid_parameter_noinfo_00bf6713();
        host.archive_vcall_04(reader, 1, key);
        host.archive_vcall_10_get(reader, 0, "text", 0, static_cast<std::uint8_t*>(back) + 0xC);
        host.archive_vcall_10_get(reader, 0, "callback", 0, static_cast<std::uint8_t*>(back) + 0x14);
        host.archive_vcall_08(reader);
    }
    host.archive_vcall_08(reader);
}

// ---------------------------------------------------------------------------
// 00735030 BSP_MissionNarrative_Construct. __thiscall(ECX = narrative), RET; EAX =
// narrative. Evidence: export listing 00735030..00735070. The 58h object at
// game+21E8h (constructed at 004DFA75).
// ---------------------------------------------------------------------------
void* cc13_seg48U_mission_narrative_construct_00735030(void* narrative, Cc13Seg48UHost& host) {
    std::uint8_t* const base = static_cast<std::uint8_t*>(narrative);
    at<std::uint32_t>(narrative, 0x00) = 0x00CFE9C0;
    at<void*>(narrative, 0x08) = host.narrative_list_buy_head_node_00733430(base + 0x4);  // 0073503F
    at<std::uint32_t>(narrative, 0x0C) = 0;
    at<std::uint32_t>(narrative, 0x10) = 0x00CFE964;
    at<std::uint32_t>(narrative, 0x2C) = 0;
    at<std::uint32_t>(narrative, 0x30) = 0;
    at<std::uint32_t>(narrative, 0x34) = 0;
    at<std::uint32_t>(narrative, 0x38) = 0x00CFE968;
    at<std::uint32_t>(narrative, 0x4C) = 0;
    at<std::uint32_t>(narrative, 0x50) = 0;
    at<std::uint32_t>(narrative, 0x54) = 0;
    return narrative;
}

// ---------------------------------------------------------------------------
// 00733030 BSP_MissionNarrative_Reset. __thiscall(ECX = narrative), RET. Evidence:
// export listing 00733030: byte +3Ch = 0, then byte +14h = 0.
// ---------------------------------------------------------------------------
void cc13_seg48U_mission_narrative_reset_00733030(void* narrative) {
    at<std::uint8_t>(narrative, 0x3C) = 0;
    at<std::uint8_t>(narrative, 0x14) = 0;
}

// ---------------------------------------------------------------------------
// 00734B10 BSP_MissionNarrative_SendToParty. __thiscall(ECX = narrative; party,
// const native string* text), RET 8. SEH 00C85DD0 (states 0/1 release the local
// message / string; not modelled). Evidence: export listing 00734B10..00734BE1.
// A MISSIONINFO message (20h bytes) is relayed to the party with no excluded peer,
// then destroyed; when the local player's party (record [game+18CCh+slot*4]+28h,
// slot game+18ECh) is this party, the text is also appended locally.
// ---------------------------------------------------------------------------
void cc13_seg48U_mission_narrative_send_to_party_00734b10(void* narrative, std::int32_t party, const void* text,
                                                          Cc13Seg48UHost& host) {
    alignas(4) std::uint8_t message[0x20] = {};
    void* const built = host.session_message_missioninfo_construct_00733b10(message, text);  // 00734B36
    host.session_host_relay_to_team_00770bd0(static_cast<std::uint8_t*>(game_pointer(host)) + 0x1EF0, party, built,
                                             0);  // 00734B57
    host.session_message53_destroy_007336a0(message);  // 00734B68
    void* const game = game_pointer(host);
    void* const local = at<void*>(game, 0x18CC + static_cast<std::size_t>(at<std::int32_t>(game, 0x18EC)) * 4);
    if (at<std::int32_t>(local, 0x28) != party) return;
    append_local_echo(narrative, text, host);  // 00734B84..00734BCB
}

// ---------------------------------------------------------------------------
// 00734BF0 BSP_MissionNarrative_SendToPlayer. __thiscall(ECX = narrative; player
// slot, const native string* text), RET 8. SEH 00C85DF0 (not modelled). Evidence:
// export listing 00734BF0..00734CD4. record = [game+18CCh+slot*4]; unless its byte
// +9h is set or its peer +50h is null, a MISSIONINFO message goes to that peer
// (00770B50) and is destroyed. When the slot is the local slot game+18ECh, the
// text is also appended locally.
// ---------------------------------------------------------------------------
void cc13_seg48U_mission_narrative_send_to_player_00734bf0(void* narrative, std::int32_t player, const void* text,
                                                           Cc13Seg48UHost& host) {
    void* const record = at<void*>(game_pointer(host), 0x18CC + static_cast<std::size_t>(player) * 4);
    if (at<std::uint8_t>(record, 0x9) == 0 && at<void*>(record, 0x50) != nullptr) {
        alignas(4) std::uint8_t message[0x20] = {};
        void* const built = host.session_message_missioninfo_construct_00733b10(message, text);  // 00734C34
        host.session_send_to_nonlocal_peer_00770b50(static_cast<std::uint8_t*>(game_pointer(host)) + 0x1EF0,
                                                    at<void*>(record, 0x50), built);  // 00734C52
        host.session_message53_destroy_007336a0(message);                             // 00734C63
    }
    if (at<std::int32_t>(game_pointer(host), 0x18EC) != player) return;
    append_local_echo(narrative, text, host);  // 00734C76..00734CBD
}

}  // namespace bsp
