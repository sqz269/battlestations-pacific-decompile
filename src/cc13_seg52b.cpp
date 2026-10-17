// cc13_seg52b: segment 52 second-pass reconstructions (docs/CC13_SEG52B.md). See
// include/bsp/cc13_seg52b.hpp.

#include "bsp/cc13_seg52b.hpp"

#include <cstring>
#include <initializer_list>

namespace bsp::cc13_seg52b {

namespace {
// The NativeString {size, ptr} copy idiom these record copies inline: unless destination and
// source are the same object, 0041DD40(ECX = destination; source size, 1), then for a non-zero
// source size memcpy (CRT 00BF7680) of destination-size bytes from the source buffer.
void copy_native_string(Seg52bHost& host, Object destination, Object source) {
    if (destination != source) {
        host.native_string_resize_0041dd40(destination, field<std::uint32_t>(source, 0x0), 1);
        if (field<std::uint32_t>(source, 0x0) != 0) {
            std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(destination, 0x4)),
                        reinterpret_cast<const void*>(field<std::uintptr_t>(source, 0x4)),
                        field<std::uint32_t>(destination, 0x0));
        }
    }
}

// The intrusive release: InterlockedDecrement(object+4) and, at zero, vtable[0](ECX = object).
void release_ref(Seg52bHost& host, Object object) {
    if (host.interlocked_decrement_00ce2220(&field<std::int32_t>(object, 0x4)) == 0) {
        host.refcounted_vslot_00(object);
    }
}

// The 0084xxxx stock record body shared by 008486B0 / 00848D00 / 00848D70: vptr 00CF69A8,
// +4h and +8h, the name vector +0Ch through 008484A0, then +1Ch.
void copy_stock_record_body(Seg52bHost& host, Object dest, Object source) {
    field<std::uint32_t>(dest, 0x0) = 0x00CF69A8u;
    field<std::uint32_t>(dest, 0x4) = field<std::uint32_t>(source, 0x4);
    field<std::uint32_t>(dest, 0x8) = field<std::uint32_t>(source, 0x8);
    host.stock_name_vector_copy_008484a0(dest + 0xC, source + 0xC);
    field<std::uint32_t>(dest, 0x1C) = field<std::uint32_t>(source, 0x1C);
}

// The keyed integer store the GetProperty readers inline: a stack NativeString {0, 0},
// 0041DD40(length, 1), memcpy of size+1 bytes from the literal when a buffer exists,
// 00B67460(ECX = table; &key, value), then the 0084F930 release idiom.
void set_lua_integer_by_text(Seg52bHost& host, cc13_seg52::LuaObject* table, const char* text,
                             std::uint32_t length, std::int32_t value) {
    alignas(4) std::uint8_t key_bytes[8];
    Object key = key_bytes;
    field<std::uint32_t>(key, 0x0) = 0;
    field<std::uint32_t>(key, 0x4) = 0;
    host.native_string_resize_0041dd40(key, length, 1);
    if (field<void*>(key, 0x4) != nullptr) {
        std::memcpy(field<void*>(key, 0x4), text, field<std::uint32_t>(key, 0x0) + 1);
    }
    host.lua_set_native_string_integer_00b67460(table, key, value);
    cc13_seg52::native_string_release_storage_0084f930(host, key);
}
}  // namespace

// ---------------------------------------------------------------------------
// 00843000 BSP_FormatToBuffer16, __cdecl(char* dest, const char* format, ...), RET:
// _vsprintf_s(dest, 10h, format, &first variadic) and its EAX. Sibling of 00467310 (20h).
// ---------------------------------------------------------------------------
int format_to_buffer16_00843000(Seg52bHost& host, char* dest, const char* format, ...) {
    std::va_list args;
    va_start(args, format);
    int result = host.vsprintf_s_00bf7289(dest, 0x10, format, args);  // 00843011
    va_end(args);
    return result;
}

// ---------------------------------------------------------------------------
// 0082A160 TMPL_RefPtr_Assign, __thiscall(ECX = slot; value), RET 4 -> EAX slot. With an old
// pointer: InterlockedDecrement(old+4), vtable[0](ECX = old) at zero, slot = 0 (0082A184),
// then slot = value. No increment of the new value and no self-assign test (the binary has
// neither).
// ---------------------------------------------------------------------------
Object ref_ptr_assign_0082a160(Seg52bHost& host, Object slot, Object value) {
    Object old = field<Object>(slot, 0x0);
    if (old != nullptr) {
        release_ref(host, old);                 // 0082A16E..0082A17E
        field<Object>(slot, 0x0) = nullptr;     // 0082A184
    }
    field<Object>(slot, 0x0) = value;           // 0082A18B / 0082A198
    return slot;
}

// ---------------------------------------------------------------------------
// 0082A590 BSP_ShipIndexedPointArray_PushBack, __thiscall(ECX = array {data, count,
// capacity}; const element*), RET 4. When count == capacity, 00829180(capacity*2, at least 1
// by the signed JG 1); the 10h-byte element (three floats by FLD/FSTP, then a dword) is stored
// when data + count*10h is non-null; count += 1 always.
// ---------------------------------------------------------------------------
void ship_indexed_point_array_push_back_0082a590(Seg52bHost& host, Object array,
                                                 const Object element) {
    std::int32_t capacity = field<std::int32_t>(array, 0x8);
    if (field<std::int32_t>(array, 0x4) == capacity) {
        std::int32_t grown = capacity + capacity;
        if (!(grown > 1)) grown = 1;                                   // 0082A55D JG
        host.ship_indexed_point_array_reserve_00829180(array, grown);   // 0082A5A8
    }
    Object slot = field<Object>(array, 0x0) +
                  (static_cast<std::uint32_t>(field<std::int32_t>(array, 0x4)) << 4);
    if (slot != nullptr) {
        field<float>(slot, 0x0) = field<float>(element, 0x0);
        field<float>(slot, 0x4) = field<float>(element, 0x4);
        field<float>(slot, 0x8) = field<float>(element, 0x8);
        field<std::uint32_t>(slot, 0xC) = field<std::uint32_t>(element, 0xC);
    }
    field<std::int32_t>(array, 0x4) += 1;                              // 0082A5D1
}

// ---------------------------------------------------------------------------
// 0084B6B0 (unnamed; the line-of-sight cast of 00750230), __fastcall(ECX = segments, EDX = b;
// ignore object), RET 4 -> EAX. a = ignore ? [ignore]->vtable[0B0h]() : 0, then
// 0098B370(ECX = 0042E630(); segments, a, b). Called at 007503D9 (gameplay_seg41_cc13's
// cast_0084b6b0 hook).
// ---------------------------------------------------------------------------
std::uint32_t los_sweep_with_ignore_0084b6b0(Seg52bHost& host, Object segments, Object b,
                                             Object ignore_object) {
    Object a = ignore_object != nullptr ? host.object_vslot_b0(ignore_object) : nullptr;  // 0084B6C7
    Object index = host.spatial_index_get_singleton_0042e630();                             // 0084B6CF
    return host.spatial_index_sweep_segments_0098b370(index, segments, a, b);              // 0084B6D9
}

// ---------------------------------------------------------------------------
// 0082DC30 (unnamed, no references found), __fastcall(ECX = holder), RET -> EAX holder, SEH
// frame 00C91CE8 with no throwing call: the RefCounted base (vptr 00CEB130, +4 = 1), then
// vptr 00D099B0 and the empty vector words +0Ch / +10h / +14h = 0 (+8 is the allocator byte,
// not written).
// ---------------------------------------------------------------------------
Object vector_holder_construct_0082dc30(Object holder) {
    field<std::uint32_t>(holder, 0x0) = 0x00CEB130u;
    field<std::uint32_t>(holder, 0x4) = 1;
    field<std::uint32_t>(holder, 0x0) = 0x00D099B0u;
    field<std::uint32_t>(holder, 0xC) = 0;
    field<std::uint32_t>(holder, 0x10) = 0;
    field<std::uint32_t>(holder, 0x14) = 0;
    return holder;
}

// ---------------------------------------------------------------------------
// 0082DC80 (unnamed, no references found), __fastcall(ECX = holder), RET, SEH frame 00C91D08
// (state 0 around the member): 0082CAD0(ECX = holder+8), then 00BD30F0(ECX = holder).
// ---------------------------------------------------------------------------
void vector_holder_destruct_0082dc80(Seg52bHost& host, Object holder) {
    host.holder_vector_tidy_0082cad0(holder + 0x8);   // 0082DCA8
    host.refcounted_destroy_base_00bd30f0(holder);    // 0082DCB7
}

// ---------------------------------------------------------------------------
// 00843460 (unnamed; shipyard vtable slot 00D0B8DC), __thiscall(ECX = shipyard; const char*
// token, target), RET 8 -> AL. True when 00438E10(token, x) == 0 for "settarget" (00CF8B0C),
// "clearorders" (00CF8AF4) or "cleartarget" (00CF8B00), tested in that order; otherwise
// 00779D50(ECX = shipyard; token, target) != 0.
// ---------------------------------------------------------------------------
bool shipyard_accepts_command_token_00843460(Seg52bHost& host, Object shipyard,
                                             const char* token, Object target) {
    if (host.cstring_compare_insensitive_00438e10(token, "settarget") == 0) return true;
    if (host.cstring_compare_insensitive_00438e10(token, "clearorders") == 0) return true;
    if (host.cstring_compare_insensitive_00438e10(token, "cleartarget") == 0) return true;
    return host.entity_may_follow_target_00779d50(shipyard, token, target) != 0;  // 008434A0
}

// ---------------------------------------------------------------------------
// 00842A80 (unnamed; shipyard vtable slot 00D0B884), __fastcall(ECX = shipyard), RET -> EAX
// [shipyard+7A0h], the weapon director 008429E0 creates.
// ---------------------------------------------------------------------------
Object shipyard_weapon_director_00842a80(Object shipyard) {
    return field<Object>(shipyard, 0x7A0);
}

// ---------------------------------------------------------------------------
// 008429E0 (unnamed; shipyard init slot), __fastcall(ECX = shipyard), RET, SEH frame
// 00C93CDB (state 0 around the construct): 0077E830(ECX = shipyard); operator new(240h); with
// memory, 008363E0(ECX = memory; shipyard ? shipyard+310h : 0, 0) and its EAX, else 0, into
// +7A0h; then when [[shipyard+0C0h]+4] == 1, 0095A880(ECX = shipyard; 0, 1, 0). The null test
// of shipyard is the compiler's this-adjust test and is kept.
// ---------------------------------------------------------------------------
void shipyard_create_weapon_director_008429e0(Seg52bHost& host, Object shipyard) {
    host.mission_entity_attach_lua_self_0077e830(shipyard);           // 008429F9
    Object memory = host.operator_new_00bf681b(0x240);                 // 00842A03
    Object director = nullptr;
    if (memory != nullptr) {
        Object endpoint = shipyard != nullptr ? shipyard + 0x310 : nullptr;
        director = host.weapon_director_base_construct_008363e0(memory, endpoint, 0);  // 00842A2A / 00842A37
    }
    field<Object>(shipyard, 0x7A0) = director;                         // 00842A40
    if (field<std::int32_t>(field<Object>(shipyard, 0xC0), 0x4) == 1) {
        host.unit_instantiate_class_devices_0095a880(shipyard, nullptr, 1, 0);  // 00842A62
    }
}

// ---------------------------------------------------------------------------
// 008397A0 BSP_SettingsKeyedString_CopyConstruct, __thiscall(ECX = dest; source), RET 4 ->
// EAX dest. 14h bytes: key +0, NativeString +4 (zeroed, then the copy idiom), float +0Ch
// (FLD/FSTP), byte +10h.
// ---------------------------------------------------------------------------
Object settings_keyed_string_copy_construct_008397a0(Seg52bHost& host, Object dest, Object source) {
    field<std::uint32_t>(dest, 0x0) = field<std::uint32_t>(source, 0x0);
    field<std::uint32_t>(dest, 0x4) = 0;
    field<std::uint32_t>(dest, 0x8) = 0;
    copy_native_string(host, dest + 0x4, source + 0x4);   // 008397CD / 008397E2
    field<float>(dest, 0xC) = field<float>(source, 0xC);
    field<std::uint8_t>(dest, 0x10) = field<std::uint8_t>(source, 0x10);
    return dest;
}

// ---------------------------------------------------------------------------
// 00839800 BSP_SettingsRecord30_CopyConstruct, __thiscall(ECX = dest; source), RET 4 -> EAX
// dest, SEH frame 00C927EE. 30h bytes: NativeString +0 (zeroed, then the copy idiom), floats
// +8h / +0Ch, byte +10h, floats +14h..+20h, then three intrusive pointers +24h / +28h / +2Ch,
// each zeroed, then with a source pointer stored and InterlockedIncrement(pointer+4).
// ---------------------------------------------------------------------------
Object settings_record30_copy_construct_00839800(Seg52bHost& host, Object dest, Object source) {
    field<std::uint32_t>(dest, 0x0) = 0;
    field<std::uint32_t>(dest, 0x4) = 0;
    copy_native_string(host, dest, source);               // 00839830 / 00839844
    field<float>(dest, 0x8) = field<float>(source, 0x8);
    field<float>(dest, 0xC) = field<float>(source, 0xC);
    field<std::uint8_t>(dest, 0x10) = field<std::uint8_t>(source, 0x10);
    for (std::size_t offset = 0x14; offset <= 0x20; offset += 4) {
        field<float>(dest, offset) = field<float>(source, offset);
    }
    for (std::size_t offset = 0x24; offset <= 0x2C; offset += 4) {
        field<Object>(dest, offset) = nullptr;
        Object value = field<Object>(source, offset);
        if (value != nullptr) {
            field<Object>(dest, offset) = value;
            host.interlocked_increment_00ce221c(&field<std::int32_t>(value, 0x4));  // 0083988D / 008398A0 / 008398B3
        }
    }
    return dest;
}

// ---------------------------------------------------------------------------
// 00839490 BSP_SettingsRecord30_Assign, __thiscall(ECX = dest; source), RET 4 -> EAX dest.
// The string copy idiom (skipped for self-assignment), the same float / byte copies (done even
// for self-assignment), then per pointer +24h / +28h / +2Ch: when it differs, store the new
// value, InterlockedIncrement(new+4) for a non-null new, then release the old (Decrement,
// vtable[0] at zero).
// ---------------------------------------------------------------------------
Object settings_record30_assign_00839490(Seg52bHost& host, Object dest, Object source) {
    copy_native_string(host, dest, source);               // 008394A3 / 008394B8
    field<float>(dest, 0x8) = field<float>(source, 0x8);
    field<float>(dest, 0xC) = field<float>(source, 0xC);
    field<std::uint8_t>(dest, 0x10) = field<std::uint8_t>(source, 0x10);
    for (std::size_t offset = 0x14; offset <= 0x20; offset += 4) {
        field<float>(dest, offset) = field<float>(source, offset);
    }
    for (std::size_t offset = 0x24; offset <= 0x2C; offset += 4) {
        Object value = field<Object>(source, offset);
        Object old = field<Object>(dest, offset);
        if (old != value) {
            field<Object>(dest, offset) = value;
            if (value != nullptr) host.interlocked_increment_00ce221c(&field<std::int32_t>(value, 0x4));
            if (old != nullptr) release_ref(host, old);
        }
    }
    return dest;
}

// ---------------------------------------------------------------------------
// 008486B0 BSP_ShipyardStockRecord_CopyConstruct, __thiscall(ECX = dest; source), RET 4 ->
// EAX dest: the shared stock record body.
// ---------------------------------------------------------------------------
Object shipyard_stock_record_copy_construct_008486b0(Seg52bHost& host, Object dest, Object source) {
    copy_stock_record_body(host, dest, source);   // 008486B8..008486D9
    return dest;
}

// ---------------------------------------------------------------------------
// 00848D00 BSP_ShipyardStockRecord_ConstructAt, __fastcall(ECX = where, EDX = source), RET,
// SEH frame 00C940A1: the body when where is non-null (placement new). No EAX result.
// ---------------------------------------------------------------------------
void shipyard_stock_record_construct_at_00848d00(Seg52bHost& host, Object where, Object source) {
    if (where != nullptr) copy_stock_record_body(host, where, source);   // 00848D32..00848D53
}

// ---------------------------------------------------------------------------
// 00848D70 BSP_Shipyard_CopyStockRecord, __thiscall(ECX = shipyard; out, unsigned index),
// RET 8 -> EAX out. The checked operator[] on the stock vector +774h..+778h (20h records):
// 00BF6713 when the data pointer is null or index >= count (JB, unsigned), then the body
// from data + index*20h into out.
// ---------------------------------------------------------------------------
Object shipyard_copy_stock_record_00848d70(Seg52bHost& host, Object shipyard, Object out,
                                           std::uint32_t index) {
    Object data = field<Object>(shipyard, 0x774);
    if (data == nullptr ||
        !(index < static_cast<std::uint32_t>((field<Object>(shipyard, 0x778) - data) >> 5))) {
        host.invalid_parameter_00bf6713();   // 00848D9A
    }
    copy_stock_record_body(host, out, field<Object>(shipyard, 0x774) + (index << 5));
    return out;
}

// ---------------------------------------------------------------------------
// 008485F0 BSP_ShipyardStockRecord_Construct, __thiscall(ECX = record; class, count, const
// char* names), RET 0Ch -> EAX record, SEH frame 00C94043. vptr 00CF69A8, +4h class, +8h
// count, the name vector words +10h / +14h / +18h = 0; a stack string (capacity 0Fh, size 0,
// first byte 0) assigned the names through 00408720(names, strlen); 0094EDC0(ECX = +0Ch,
// EDX = &string; ",") splits it into the vector; the string's heap buffer is freed when its
// capacity >= 10h; +1Ch = 0.
// ---------------------------------------------------------------------------
Object shipyard_stock_record_construct_008485f0(Seg52bHost& host, Object record,
                                                Object vehicle_class, std::uint32_t count,
                                                const char* names) {
    field<std::uint32_t>(record, 0x0) = 0x00CF69A8u;
    field<Object>(record, 0x4) = vehicle_class;
    field<std::uint32_t>(record, 0x8) = count;
    field<std::uint32_t>(record, 0x10) = 0;
    field<std::uint32_t>(record, 0x14) = 0;
    field<std::uint32_t>(record, 0x18) = 0;
    StdString1C text{};
    Object string = text.bytes;
    field<std::uint32_t>(string, 0x18) = 0xF;
    field<std::uint32_t>(string, 0x14) = 0;
    field<std::uint8_t>(string, 0x4) = 0;
    host.character_string_assign_counted_00408720(string, names,
                                                  static_cast<std::uint32_t>(std::strlen(names)));  // 00848662
    host.string_vector_split_0094edc0(record + 0xC, string, ",");   // 00848677 (00CE4BFC)
    if (field<std::uint32_t>(string, 0x18) >= 0x10) {
        host.free_00bf65ac(field<void*>(string, 0x4));               // 00848688
    }
    field<std::uint32_t>(record, 0x1C) = 0;
    return record;
}

// ---------------------------------------------------------------------------
// 00846970 BSP_ShipyardSlot_CopyConstruct, __thiscall(ECX = dest; source), RET 4 -> EAX dest,
// SEH frame 00C93DFB. 4Ch-byte queue slot: vptr 00CF6544, +4h..+0Ch copied; the string at +10h
// default-initialised (capacity +28h = 0Fh, size +24h = 0, first byte +14h = 0) then
// 00408120(ECX = +10h; source+10h, 0, -1); +2Ch / +30h copied; 00796BD0(ECX = +34h;
// source+34h); then the observer vptr 00CE3CF0 at +34h.
// ---------------------------------------------------------------------------
Object shipyard_slot_copy_construct_00846970(Seg52bHost& host, Object dest, Object source) {
    field<std::uint32_t>(dest, 0x0) = 0x00CF6544u;
    field<std::uint32_t>(dest, 0x4) = field<std::uint32_t>(source, 0x4);
    field<std::uint32_t>(dest, 0x8) = field<std::uint32_t>(source, 0x8);
    field<std::uint32_t>(dest, 0xC) = field<std::uint32_t>(source, 0xC);
    field<std::uint32_t>(dest, 0x28) = 0xF;
    field<std::uint32_t>(dest, 0x24) = 0;
    field<std::uint8_t>(dest, 0x14) = 0;
    host.sbo_string_assign_substring_00408120(dest + 0x10, source + 0x10, 0, 0xFFFFFFFFu);  // 008469C4
    field<std::uint32_t>(dest, 0x2C) = field<std::uint32_t>(source, 0x2C);
    field<std::uint32_t>(dest, 0x30) = field<std::uint32_t>(source, 0x30);
    host.entity_observer_copy_construct_base_00796bd0(dest + 0x34, source + 0x34);           // 008469E2
    field<std::uint32_t>(dest, 0x34) = 0x00CE3CF0u;                                          // 008469EE
    return dest;
}

// ---------------------------------------------------------------------------
// 0084DA20 (no Ghidra function; slot 00D0BDD4 of the plane squadron command block vtable),
// __thiscall(ECX = block; visitor), RET 4 via the tail jump: [visitor]->vtable[0](ECX =
// visitor; block).
// ---------------------------------------------------------------------------
void squadron_block_accept_visitor_0084da20(Seg52bHost& host, Object block, Object visitor) {
    host.visitor_vslot_00(visitor, block);   // 0084DA2E JMP
}

// ---------------------------------------------------------------------------
// 0084D980 (unnamed; slot 00D0BDEC of the same vtable), __thiscall(ECX = block; mode), RET 4.
// unit = +224h, squadron = [unit+3D0h]; both must be non-null. effective = 0 when
// squadron->vtable[5Ch](17h) and byte squadron+0C24h == 0, else mode. Then with
// squadron->vtable[5Ch](13h): vtable[40h](1), vtable[44h](effective == 1 ? 0 : 1); otherwise
// effective == 1 gives vtable[40h](0), vtable[44h](1) and any other value vtable[40h](1),
// vtable[44h](1) (ECX = block for both).
// ---------------------------------------------------------------------------
void squadron_block_set_fire_and_move_0084d980(Seg52bHost& host, Object block,
                                               std::uint32_t mode) {
    Object unit = field<Object>(block, 0x224);
    if (unit == nullptr) return;
    Object squadron = field<Object>(unit, 0x3D0);
    if (squadron == nullptr) return;
    std::uint32_t effective = mode;
    if (host.unit_is_kind_of_vslot_5c(squadron, 0x17) != 0 &&
        field<std::uint8_t>(squadron, 0xC24) == 0) {
        effective = 0;                                                  // 0084D9B5
    }
    if (host.unit_is_kind_of_vslot_5c(squadron, 0x13) != 0) {          // 0084D9C6
        host.director_vslot_40(block, 1);                               // 0084D9D5
        host.director_vslot_44(block, effective == 1 ? 0u : 1u);        // 0084D9E5 / 0084DA13
    } else if (effective == 1) {
        host.director_vslot_40(block, 0);                               // 0084D9F4
        host.director_vslot_44(block, effective);                       // 0084D9FE
    } else {
        host.director_vslot_40(block, 1);                               // 0084DA08
        host.director_vslot_44(block, 1);                               // 0084DA13
    }
}

// ---------------------------------------------------------------------------
// 0084DA60 (unnamed; slot 00D0BE08 of the same vtable, the override of 0071EDD0),
// __thiscall(ECX = block; target, byte flag), RET 8 -> AL. object = 00521EA0(target); same =
// object == 00521EA0(flag ? block+58h : block+18Ch). valid = 0071EDD0(ECX = block; target,
// flag); not valid -> 0; valid and not same -> 1; valid and same -> 0, and first, unless the
// session mode [[00E188A8]+1FE4h] is 2, byte [[block+38h]+3Eh] = 1.
// ---------------------------------------------------------------------------
bool squadron_block_target_still_valid_0084da60(Seg52bHost& host, Object block, Object target,
                                                std::uint8_t flag) {
    Object object = host.command_target_resolve_object_00521ea0(target);              // 0084DA6C
    Object current = host.command_target_resolve_object_00521ea0(
        flag != 0 ? block + 0x58 : block + 0x18C);                                    // 0084DA7E / 0084DAC4
    bool same = current == object;
    if (host.weapon_director_target_still_valid_0071edd0(block, target, flag) == 0) {  // 0084DA90
        return false;
    }
    if (!same) return true;                                                            // 0084DAD4
    if (field<std::int32_t>(host.game_session_00e188a8(), 0x1FE4) != 2) {
        field<std::uint8_t>(field<Object>(block, 0x38), 0x3E) = 1;                     // 0084DAB1
    }
    return false;
}

// ---------------------------------------------------------------------------
// 0084B6F0 BSP_Effect_SpawnOrientedPointEffect, __fastcall(ECX = definition slot, EDX = const
// float3* position; const float3* direction), RET 4 -> EAX effect. 0 when [slot] is null (the
// NEG/SBB/TEST 00E17BF8 bool idiom). A 4x4 identity (1.0f from 00D7A24C) at frame +10h; unless
// all three direction components compare equal to 0 (UCOMISS/LAHF/TEST AH,44h/JP: an
// unordered component takes the look-at arm), 00B63F10(ECX = frame+50h, EDX = &(0,0,0);
// direction, up (0, 1, 0)) and 004134F0(ECX = matrix; its EAX). Row 3 (+30h..+38h) = position.
// A RefPtr copy of [slot] (InterlockedIncrement) is passed by value to 00868420(ECX = &out,
// EDX = [[00E188A8]+19ECh]; copy, &matrix, 0, 0); effect = [its EAX]; the out RefPtr is
// released (Decrement, vtable[0] at zero); then a non-null effect gets byte +9 = 1.
// ---------------------------------------------------------------------------
Object spawn_oriented_point_effect_0084b6f0(Seg52bHost& host, Object definition_slot,
                                            const float* position, const float* direction) {
    if (field<Object>(definition_slot, 0x0) == nullptr) return nullptr;   // 0084B70A
    float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    if (!(direction[0] == 0.0f) || !(direction[1] == 0.0f) || !(direction[2] == 0.0f)) {
        float look_at[16];
        const float eye[3] = {0.0f, 0.0f, 0.0f};
        float* built = host.matrix_build_look_at_00b63f10(look_at, eye, direction, 0.0f, 1.0f,
                                                          0.0f);                 // 0084B7DD
        host.matrix_copy_4x4_004134f0(matrix, built);                           // 0084B7E7
    }
    matrix[12] = position[0];
    matrix[13] = position[1];
    matrix[14] = position[2];
    Object registry = field<Object>(host.game_session_00e188a8(), 0x19EC);
    Object definition = nullptr;
    Object source = field<Object>(definition_slot, 0x0);
    if (source != nullptr) {
        definition = source;
        host.interlocked_increment_00ce221c(&field<std::int32_t>(source, 0x4));  // 0084B839
    }
    Object out = nullptr;
    Object* result = host.point_effect_create_from_matrix_00868420(&out, registry, definition,
                                                                   matrix, 0, 0);  // 0084B845
    Object effect = *result;
    if (out != nullptr) release_ref(host, out);                                  // 0084B85A
    if (effect != nullptr) field<std::uint8_t>(effect, 0x9) = 1;                 // 0084B872
    return effect;
}

// ---------------------------------------------------------------------------
// 0084C430 BSP_Projectile_SweepStepSegment, __fastcall(CL / EDX passed through untouched;
// from x/y/z, to x/y/z, a6..a10), RET 2Ch -> EAX. A 0C4h-byte segment list on the stack: one
// segment (+0h..+14h) from the six floats (MOVSS), count +0C0h = 1, the rest left unwritten;
// then 0084BF00(ECX, EDX; &list, a6, a7, a8, a9, a10) and its EAX.
// ---------------------------------------------------------------------------
std::uint32_t projectile_sweep_step_segment_0084c430(Seg52bHost& host, std::uint8_t replay_flag,
                                                     std::uint32_t flags, float from_x,
                                                     float from_y, float from_z, float to_x,
                                                     float to_y, float to_z, std::uint32_t a6,
                                                     std::uint32_t a7, std::uint32_t a8,
                                                     std::uint32_t a9, std::uint32_t a10) {
    alignas(4) std::uint8_t list[0xC4];
    Object segments = list;
    field<float>(segments, 0x0) = from_x;
    field<float>(segments, 0x4) = from_y;
    field<float>(segments, 0x8) = from_z;
    field<float>(segments, 0xC) = to_x;
    field<float>(segments, 0x10) = to_y;
    field<float>(segments, 0x14) = to_z;
    field<std::uint32_t>(segments, 0xC0) = 1;                                    // 0084C4BD
    return host.projectile_trace_segment_and_impact_0084bf00(replay_flag, flags, segments, a6,
                                                             a7, a8, a9, a10);  // 0084C4C8
}

// ---------------------------------------------------------------------------
// 00845670 (unnamed, no reference found), __thiscall(ECX = shipyard; float3* out), RET 4 ->
// EAX out. With a non-empty hangar vector +784h..+788h (10h records): the checked front()
// (00BF6713 when data > end, then when !(data < end)) and 007AF800(ECX = [data+8]+1E4h; out,
// 1). Otherwise the pose block +0CCh (byte +0C8h == 0 calls 00414DB0(ECX = shipyard) before
// each of its two reads): out = position (+0FCh..+104h) + row (+0ECh..+0F4h) * 100.0 (the
// double at 00D7A220). x87: each row component times the double is rounded to a float local,
// then added to the position component and rounded again.
// ---------------------------------------------------------------------------
float* shipyard_exit_point_00845670(Seg52bHost& host, Object shipyard, float* out) {
    Object data = field<Object>(shipyard, 0x784);
    if (data != nullptr && ((field<Object>(shipyard, 0x788) - data) >> 4) != 0) {
        if (data > field<Object>(shipyard, 0x788)) host.invalid_parameter_00bf6713();     // 0084571D
        if (!(data < field<Object>(shipyard, 0x788))) host.invalid_parameter_00bf6713();  // 0084572A
        return host.scene_path_transform_point_to_world_007af800(field<Object>(data, 0x8) + 0x1E4,
                                                                 out, 1);                // 0084573F
    }
    constexpr double kExitDistance_00d7a220 = 100.0;
    if (field<std::uint8_t>(shipyard, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(shipyard);
    Object pose = shipyard + 0xCC;
    float scaled_x = static_cast<float>(field<float>(pose, 0x20) * kExitDistance_00d7a220);
    float scaled_y = static_cast<float>(field<float>(pose, 0x24) * kExitDistance_00d7a220);
    float scaled_z = static_cast<float>(kExitDistance_00d7a220 * field<float>(pose, 0x28));
    if (field<std::uint8_t>(shipyard, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(shipyard);
    float y = static_cast<float>(static_cast<double>(field<float>(pose, 0x34)) + scaled_y);
    float z = static_cast<float>(static_cast<double>(field<float>(pose, 0x38)) + scaled_z);
    out[0] = static_cast<float>(static_cast<double>(field<float>(pose, 0x30)) + scaled_x);
    out[1] = y;
    out[2] = z;
    return out;
}

// ---------------------------------------------------------------------------
// 00848080 BSP_Shipyard_Construct, __thiscall(ECX = shipyard; flag), RET 4 -> EAX shipyard,
// SEH frame 00C93FF4. 0095CC90(flag, 0); +72Ch = 00CF85D4; the vptrs +0 00D0B770, +10h
// 00D0B754, +24h 00D0B74C, +170h 00D0B748, +1E4h 00D0B740, +310h 00D0B728, +38Ch 00D0B724,
// +72Ch 00D0B718; the three vectors' words +774h..+77Ch, +784h..+78Ch, +794h..+79Ch = 0;
// class id +0C4h = 46h.
// ---------------------------------------------------------------------------
Object shipyard_construct_00848080(Seg52bHost& host, Object shipyard, std::uint32_t flag) {
    host.unit_game_object_construct_0095cc90(shipyard, flag, 0);   // 008480A2
    field<std::uint32_t>(shipyard, 0x72C) = 0x00CF85D4u;
    field<std::uint32_t>(shipyard, 0x0) = 0x00D0B770u;
    field<std::uint32_t>(shipyard, 0x10) = 0x00D0B754u;
    field<std::uint32_t>(shipyard, 0x24) = 0x00D0B74Cu;
    field<std::uint32_t>(shipyard, 0x170) = 0x00D0B748u;
    field<std::uint32_t>(shipyard, 0x1E4) = 0x00D0B740u;
    field<std::uint32_t>(shipyard, 0x310) = 0x00D0B728u;
    field<std::uint32_t>(shipyard, 0x38C) = 0x00D0B724u;
    field<std::uint32_t>(shipyard, 0x72C) = 0x00D0B718u;
    for (std::size_t offset : {0x774u, 0x778u, 0x77Cu, 0x784u, 0x788u, 0x78Cu, 0x794u, 0x798u, 0x79Cu}) {
        field<std::uint32_t>(shipyard, offset) = 0;
    }
    field<std::uint32_t>(shipyard, 0xC4) = 0x46;
    return shipyard;
}

// ---------------------------------------------------------------------------
// 00848380 BSP_Shipyard_Allocate, __thiscall(ECX = vehicle class; flag), RET 4 -> EAX unit,
// SEH frame 00C94008. operator new(7A4h) (00BF55BE), memset(memory, 0, 7A4h) before the null
// test, 00848080(flag) when non-null (else 0), then 009553D0(ECX = unit; class) and +354h =
// class with no null test (as in the image).
// ---------------------------------------------------------------------------
Object shipyard_allocate_00848380(Seg52bHost& host, Object vehicle_class, std::uint32_t flag) {
    Object memory = host.operator_new_00bf55be(0x7A4);                         // 0084839F
    host.memset_00bf79f0(memory, 0, 0x7A4);                                     // 008483AE
    Object unit = memory != nullptr ? shipyard_construct_00848080(host, memory, flag) : nullptr;
    host.unit_set_vehicle_class_009553d0(unit, vehicle_class);                  // 008483E3
    field<Object>(unit, 0x354) = vehicle_class;
    return unit;
}

// ---------------------------------------------------------------------------
// 0084F670 (unnamed; the body of 0084FA10, the stationary class scalar deleting destructor),
// __fastcall(ECX = class), RET, SEH frame 00C94401 (states 2, 1, 0, -1). vptr 00D0BE38; the
// RefPtrs +18h then +28h released (Decrement, vtable[0] at zero) and cleared; then the
// NativeStrings {size +10h, buffer +14h} and {size +8h, buffer +0Ch} returned to the sized
// pool (0084F930's idiom).
// ---------------------------------------------------------------------------
void stationary_class_destruct_0084f670(Seg52bHost& host, Object stationary_class) {
    field<std::uint32_t>(stationary_class, 0x0) = 0x00D0BE38u;
    for (std::size_t offset : {0x18u, 0x28u}) {
        Object pointer = field<Object>(stationary_class, offset);
        if (pointer != nullptr) {
            release_ref(host, pointer);                                         // 0084F6AE / 0084F6D3
            field<Object>(stationary_class, offset) = nullptr;
        }
    }
    cc13_seg52::native_string_release_storage_0084f930(host, stationary_class + 0x10);  // 0084F6FE
    cc13_seg52::native_string_release_storage_0084f930(host, stationary_class + 0x8);   // 0084F723
}

// ---------------------------------------------------------------------------
// 0084FB80 (unnamed; slot 00CFF6FC of the stationary entity vtable), __fastcall(ECX =
// stationary), ends in a tail jump to 00923060. The registry vector [[00E188A8]+30h] {+4h
// first, +8h last} of 8-byte entries: the checked begin()/end() reads (00BF6713 when first >
// last), a find of the entry whose first dword is [stationary+19Ch] (read once), the iterator
// owner test (00BF6713 when the owner is null or not the vector re-read), and 0084FA30
// erase(&local, owner, where) when found. Then a non-null +19Ch is deleted (vtable[4](1)) and
// cleared, and 00923060(ECX = stationary).
// ---------------------------------------------------------------------------
void stationary_destruct_0084fb80(Seg52bHost& host, Object stationary) {
    Object vector = field<Object>(host.game_session_00e188a8(), 0x30);
    Object last = field<Object>(vector, 0x8);
    if (field<Object>(vector, 0x4) > last) host.invalid_parameter_00bf6713();          // 0084FB9B
    Object owner = field<Object>(host.game_session_00e188a8(), 0x30);
    Object where = field<Object>(owner, 0x4);
    if (where > field<Object>(owner, 0x8)) host.invalid_parameter_00bf6713();          // 0084FBB1
    if (where != last) {
        Object key = field<Object>(stationary, 0x19C);
        while (field<Object>(where, 0x0) != key) {
            where += 8;
            if (where == last) break;
        }
    }
    Object current = field<Object>(host.game_session_00e188a8(), 0x30);
    Object current_last = field<Object>(current, 0x8);
    if (field<Object>(current, 0x4) > current_last) host.invalid_parameter_00bf6713(); // 0084FBEB
    if (owner == nullptr || owner != current) host.invalid_parameter_00bf6713();        // 0084FBF8
    if (where != current_last) {
        alignas(4) std::uint8_t out_iterator[8];
        host.pair_vector_erase_0084fa30(current, out_iterator, owner, where);         // 0084FC11
    }
    Object resource = field<Object>(stationary, 0x19C);
    if (resource != nullptr) {
        host.object_delete_vslot_04(resource, 1);                                     // 0084FC2F
        field<Object>(stationary, 0x19C) = nullptr;
    }
    host.entity_base_destruct_00923060(stationary);                                   // 0084FC44 JMP
}

// ---------------------------------------------------------------------------
// 0084F7F0 (unnamed; the land-fort sibling of 0084F750, called by 00748C40 at 00748C9E),
// __thiscall(ECX = entity; class), RET 4. +178h = class+50h; +17Ch / +180h = class+48h /
// +4Ch (FLD/FSTP); the RefPtr +184h takes class+13Ch when it differs (store, increment a
// non-null new, release the old); +188h..+190h from the first 30h-byte element's +0Ch of the
// vector class+1Ch..+20h when non-empty (the checked front(): 00BF6713 when the data is null
// or the count is 0, re-tested), else from 00F87574; +194h = -1; +198h = class+138h. Unlike
// 0084F750 there is no class vtable[10h] call.
// ---------------------------------------------------------------------------
void stationary_apply_fort_class_0084f7f0(Seg52bHost& host, Object entity, Object vehicle_class) {
    field<std::uint32_t>(entity, 0x178) = field<std::uint32_t>(vehicle_class, 0x50);
    field<float>(entity, 0x17C) = field<float>(vehicle_class, 0x48);
    field<float>(entity, 0x180) = field<float>(vehicle_class, 0x4C);
    Object incoming = field<Object>(vehicle_class, 0x13C);
    Object old = field<Object>(entity, 0x184);
    if (old != incoming) {
        field<Object>(entity, 0x184) = incoming;
        if (incoming != nullptr) host.interlocked_increment_00ce221c(&field<std::int32_t>(incoming, 0x4));
        if (old != nullptr) release_ref(host, old);
    }
    auto element_count = [&]() {
        return (field<Object>(vehicle_class, 0x20) - field<Object>(vehicle_class, 0x1C)) / 0x30;
    };
    const float* source = host.default_vector_00f87574();
    if (field<Object>(vehicle_class, 0x1C) != nullptr && element_count() != 0) {
        if (field<Object>(vehicle_class, 0x1C) == nullptr || element_count() == 0) {
            host.invalid_parameter_00bf6713();                                   // 0084F890
        }
        source = reinterpret_cast<const float*>(field<Object>(vehicle_class, 0x1C) + 0xC);
    }
    field<float>(entity, 0x188) = source[0];
    field<float>(entity, 0x18C) = source[1];
    field<float>(entity, 0x190) = source[2];
    field<std::int32_t>(entity, 0x194) = -1;
    field<float>(entity, 0x198) = field<float>(vehicle_class, 0x138);
}

// ---------------------------------------------------------------------------
// 00850930 BSP_StationaryClassMap_DestroyAll, __cdecl(), RET. Walks the map 00F87550 with a
// checked iterator {owner, node} from the head's +0 (leftmost): per step 00BF6713 when the
// owner is null or not the map; at the head it stops; otherwise 00BF6713 again for a null
// owner or a node equal to the owner's head, a non-null value +14h is deleted (vtable[0](1))
// and cleared, and 0084EE80 advances. Then 008501D0(ECX = map; [head+4]), head+4 = head,
// size = 0, head+0 = head, head+8 = head (each store re-reading the head).
// ---------------------------------------------------------------------------
void stationary_class_map_destroy_all_00850930(Seg52bHost& host) {
    Object map = host.stationary_class_map_00f87550();
    alignas(4) std::uint8_t iterator_bytes[8];
    Object iterator = iterator_bytes;
    field<Object>(iterator, 0x4) = field<Object>(field<Object>(map, 0x4), 0x0);
    field<Object>(iterator, 0x0) = map;
    for (;;) {
        Object owner = field<Object>(iterator, 0x0);
        Object node = field<Object>(iterator, 0x4);
        Object head = field<Object>(map, 0x4);
        if (owner == nullptr || owner != map) host.invalid_parameter_00bf6713();   // 00850962
        if (node == head) break;
        if (owner == nullptr) host.invalid_parameter_00bf6713();                   // 0085096F
        if (node == field<Object>(owner, 0x4)) host.invalid_parameter_00bf6713();  // 00850979
        Object value = field<Object>(node, 0x14);
        if (value != nullptr) {
            host.object_delete_vslot_00(value, 1);                                 // 0085098B
            field<Object>(node, 0x14) = nullptr;
        }
        host.tree_iterator_increment_0084ee80(iterator);                           // 00850998
    }
    host.tree_erase_008501d0(map, field<Object>(field<Object>(map, 0x4), 0x4));    // 008509B6
    field<Object>(field<Object>(map, 0x4), 0x4) = field<Object>(map, 0x4);
    field<std::uint32_t>(map, 0x8) = 0;
    field<Object>(field<Object>(map, 0x4), 0x0) = field<Object>(map, 0x4);
    field<Object>(field<Object>(map, 0x4), 0x8) = field<Object>(map, 0x4);
}

// ---------------------------------------------------------------------------
// 0084FCE0 BSP_Stationary_LoadExplosionResource (a stationary class slot), __fastcall(ECX =
// class), RET, SEH frame 00C94420. Only while byte +1Ch is 0: a stack NativeString from the
// class name buffer +14h (00F87548 when null) through 0041E870; +18h = 007188A0(&name); the
// name is returned to the sized pool. A stack string (capacity 0Fh, size 0) gets "explosion"
// (00D05A18, 9) through 00408720; group = 00718870(ECX = +18h; &string, 0); the string's heap
// buffer is freed when its capacity >= 10h and it is reset to empty; +2Ch..+34h = the group's
// point 0 (00484270 into a stack float3) or 00F87574 without a group; byte +1Ch = 1.
// ---------------------------------------------------------------------------
void stationary_class_load_explosion_resource_0084fce0(Seg52bHost& host, Object stationary_class) {
    if (field<std::uint8_t>(stationary_class, 0x1C) != 0) return;
    const char* name_text = field<const char*>(stationary_class, 0x14);
    if (name_text == nullptr) name_text = host.default_name_text_00f87548();
    alignas(4) std::uint8_t name_bytes[8];
    Object name = name_bytes;
    host.native_string_assign_0041e870(name, name_text);                               // 0084FD18
    field<Object>(stationary_class, 0x18) = host.resource_load_with_game_factory_007188a0(name);  // 0084FD25
    cc13_seg52::native_string_release_storage_0084f930(host, name);                    // 0084FD48
    StdString1C text{};
    Object string = text.bytes;
    field<std::uint32_t>(string, 0x18) = 0xF;
    field<std::uint32_t>(string, 0x14) = 0;
    field<std::uint8_t>(string, 0x4) = 0;
    host.character_string_assign_counted_00408720(string, "explosion", 9);            // 0084FD70
    Object group = host.resource_lookup_named_point_group_00718870(
        field<Object>(stationary_class, 0x18), string, 0);                             // 0084FD86
    if (field<std::uint32_t>(string, 0x18) >= 0x10) host.free_00bf65ac(field<void*>(string, 0x4));  // 0084FD99
    field<std::uint32_t>(string, 0x18) = 0xF;
    field<std::uint32_t>(string, 0x14) = 0;
    field<std::uint8_t>(string, 0x4) = 0;
    float point[3];
    const float* source = group != nullptr
                              ? host.point_group_point_00484270(group, point, 0)       // 0084FDBD
                              : host.default_vector_00f87574();
    field<float>(stationary_class, 0x2C) = source[0];
    field<float>(stationary_class, 0x30) = source[1];
    field<float>(stationary_class, 0x34) = source[2];
    field<std::uint8_t>(stationary_class, 0x1C) = 1;
}

// ---------------------------------------------------------------------------
// 00846640 BSP_Shipyard_ReadStockProperties (the GetProperty reader, vtable+138h),
// __thiscall(ECX = shipyard; LuaObject* lua, const NativeString* name), RET 8, SEH frame
// 00C93DE0. 00779BB0(ECX = shipyard; lua, name) first; then only for a non-null name buffer
// equal to "stock" (00CF86B8) under _stricmp: table = 00B67930(ECX = lua; &table); per stock
// record of +774h..+778h (20h bytes; the checked begin()/end()/deref/advance tests call
// 00BF6713), with index from 1: 00B66670(ECX = &table; index), 00B67720(ECX = &table; &entry,
// index), index += 1, then entry["classid"] = [[record+4]+70h] and entry["count"] =
// [record+8] through 00B67460 with stack NativeStrings (0041DD40(7 / 5, 1), memcpy of size+1
// bytes from 00CF8878 / 00CF86D8, released to the sized pool after each call); 00B67700 on the
// entry; finally 00B67700 on the table.
// ---------------------------------------------------------------------------
void shipyard_get_property_stock_00846640(Seg52bHost& host, Object shipyard, Object lua,
                                          Object name) {
    host.unit_get_property_recon_level_00779bb0(shipyard, lua, name);              // 00846667
    const char* text = field<const char*>(name, 0x4);
    if (text == nullptr) return;
    if (host.stricmp_00bf7fbf(text, "stock") != 0) return;                          // 0084667F
    cc13_seg52::LuaObject table;
    host.lua_object_new_table_00b67930(lua, &table);                                // 0084669C
    Object stock = shipyard + 0x770;
    Object record = field<Object>(stock, 0x4);
    if (record > field<Object>(stock, 0x8)) host.invalid_parameter_00bf6713();      // 008466BB
    std::int32_t index = 1;
    for (;;) {
        Object last = field<Object>(stock, 0x8);
        if (field<Object>(stock, 0x4) > last) host.invalid_parameter_00bf6713();    // 008466C8
        if (record == last) break;
        if (!(record < field<Object>(stock, 0x8))) host.invalid_parameter_00bf6713();  // 008466F4
        host.lua_set_new_table_at_index_00b66670(&table, index);                    // 00846702
        cc13_seg52::LuaObject entry;
        host.lua_get_by_index_00b67720(&table, &entry, index);                      // 00846711
        index += 1;
        set_lua_integer_by_text(host, &entry, "classid", 7,
                                field<std::int32_t>(field<Object>(record, 0x4), 0x70));  // 0084676A
        set_lua_integer_by_text(host, &entry, "count", 5,
                                field<std::int32_t>(record, 0x8));                  // 008467D8
        host.lua_object_destruct_00b67700(&entry);                                  // 00846809
        if (!(record < field<Object>(stock, 0x8))) host.invalid_parameter_00bf6713();  // 00846815
        record += 0x20;
    }
    host.lua_object_destruct_00b67700(&table);                                      // 0084682E
}

// ---------------------------------------------------------------------------
// 00851580 BSP_StationaryClass_ReadLuaFields (stationary class vtable 00D0BE38 +8h),
// __thiscall(ECX = class; const LuaObject* row), RET 4, SEH frame 00C94510. A by-value copy of
// the row (00B66FA0 into the argument area) feeds a stack LuaReader (004425C0). Reads, in order:
// "Mesh" (00CE5FD0) -> NativeString +10h ({0, &+10h}, 00BD6830); then 00BD68D0 with float
// defaults {2, 0.0f}: "HPBlack" (00D0BE94) -> +20h, "Armor" (00CFD8F4) -> +24h, "Height"
// (00CE9524) -> +4h. A stack NativeString "ObjectExplosion" (00D0BE84; 0041DD40(0Fh, 1), memcpy
// of size+1) gives the default id 00871A90(&name); "ExplosionEfx" (00CE44AC) is read as {1,
// int} into the dead argument slot with default {1, that id}; the name is released. handle =
// [00870CD0(ECX = &local, EDX = id; 1)]; the RefPtr +28h takes it when it differs (store,
// increment a non-null new, release the old), then the local is released. "FakeExplosionGroup"
// (00D0BE70) -> +38h ({1}, default {1, -1}). The reader is torn down inline: vptr 00CE44FC,
// then 00441210(ECX = reader+4).
// ---------------------------------------------------------------------------
void stationary_class_read_lua_fields_00851580(Seg52bHost& host, Object stationary_class,
                                               Object row) {
    using cc13_seg52::TaggedValue;
    auto tagged = [](std::uint32_t tag, const void* value) {
        return TaggedValue{tag, reinterpret_cast<std::uintptr_t>(value)};
    };
    auto tagged_float = [](float value) {
        std::uint32_t bits;
        std::memcpy(&bits, &value, sizeof bits);
        return TaggedValue{2, bits};
    };
    cc13_seg52::LuaObject row_copy;
    host.lua_object_copy_construct_00b66fa0(&row_copy, row);                          // 008515AC
    alignas(4) std::uint8_t reader_bytes[0x14];
    Object reader = reader_bytes;
    host.lua_reader_construct_004425c0(reader, &row_copy);                            // 008515B5
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "Mesh"),
                                        tagged(0, stationary_class + 0x10));          // 008515E7
    host.lua_reader_read_field_or_default_00bd68d0(reader, tagged(0, "HPBlack"),
                                                   tagged(2, stationary_class + 0x20),
                                                   tagged_float(0.0f));               // 0085162E
    host.lua_reader_read_field_or_default_00bd68d0(reader, tagged(0, "Armor"),
                                                   tagged(2, stationary_class + 0x24),
                                                   tagged_float(0.0f));               // 00851675
    host.lua_reader_read_field_or_default_00bd68d0(reader, tagged(0, "Height"),
                                                   tagged(2, stationary_class + 0x4),
                                                   tagged_float(0.0f));               // 008516BC
    alignas(4) std::uint8_t name_bytes[8];
    Object name = name_bytes;
    field<std::uint32_t>(name, 0x0) = 0;
    field<std::uint32_t>(name, 0x4) = 0;
    host.native_string_resize_0041dd40(name, 0xF, 1);                                 // 008516D1
    if (field<void*>(name, 0x4) != nullptr) {
        std::memcpy(field<void*>(name, 0x4), "ObjectExplosion", field<std::uint32_t>(name, 0x0) + 1);
    }
    std::int32_t default_id = host.gameplay_effect_find_id_by_name_00871a90(name);    // 008516FD
    std::int32_t effect_id;  // the argument slot [ESP+40h], written only by the reader
    host.lua_reader_read_field_or_default_00bd68d0(
        reader, tagged(0, "ExplosionEfx"), tagged(1, &effect_id),
        TaggedValue{1, static_cast<std::uint32_t>(default_id)});                      // 00851740
    cc13_seg52::native_string_release_storage_0084f930(host, name);                   // 0085175C
    Object handle_local = nullptr;
    Object handle = *host.effect_handle_acquire_00870cd0(&handle_local, effect_id, 1); // 00851772
    Object old = field<Object>(stationary_class, 0x28);
    if (old != handle) {
        field<Object>(stationary_class, 0x28) = handle;
        if (handle != nullptr) host.interlocked_increment_00ce221c(&field<std::int32_t>(handle, 0x4));
        if (old != nullptr) release_ref(host, old);
    }
    if (handle_local != nullptr) release_ref(host, handle_local);                     // 008517C4
    host.lua_reader_read_field_or_default_00bd68d0(reader, tagged(0, "FakeExplosionGroup"),
                                                   tagged(1, stationary_class + 0x38),
                                                   TaggedValue{1, 0xFFFFFFFFu});      // 00851813
    field<std::uint32_t>(reader, 0x0) = 0x00CE44FCu;                                  // 00851818
    host.lua_object_vector_tidy_00441210(reader + 0x4);                               // 0085182C
}

// ---------------------------------------------------------------------------
// 00851850 BSP_Stationary_OnWrecked (stationary entity vtable 00CFF678 +7Ch), __fastcall(ECX =
// stationary), RET, SEH frame 00C94530.
//  1. 00851230(ECX = [[00E188A8]+30h]; &{[s+19Ch], float [s+198h]}): the registry entry
//     0084FB80 later erases.
//  2. resource = [s+19Ch]: +0F8h = 0, then for each node of the list resource+194h (head
//     [resource+198h], next at node+0; the checked-iterator tests call 00BF6713) the node's
//     value address node+8 goes to resource+0D0h + 4*count, count +0F8h += 1 (no bound test,
//     as in the image; resource is re-read from s+19Ch at each step).
//  3. 00923040(ECX = s).
//  4. When [[00E188A8]+1FE4h] == 1 and byte [00E0AF20] != 0: a stack session message
//     (0075B430(41h), then +4h = 1, vptr 00D02EAC, +18h = [s+174h], byte +1Ch = byte
//     [s+1A4h]) goes to 0076A9F0(ECX = [00E188A8]+1EF0h; &message, 0).
//  5. Only when byte s+1A4h is set: the pose refresh (00414DB0 when byte +0C8h == 0); O =
//     00B62D10(ECX = &(s+188h, s+18Ch, s+190h, 1.0f); &out, s+0CCh); inv = (float)(1 / O.w)
//     (FLD1/FDIVRP), Q = O.xyz * inv, each product rounded to a float. A RefPtr copy of s+184h
//     (incremented) goes by value to 008685E0(ECX = &handle, EDX = [[00E188A8]+19ECh]; copy,
//     &Q, 0, 0); a non-null handle gets byte +9 = 1; when int [s+194h] >= 0 (JL),
//     0049C940(ECX = [[00E188A8]+21D0h]; &Q, &(0,0,0), [s+194h]); then the handle is released.
// ---------------------------------------------------------------------------
void stationary_on_wrecked_00851850(Seg52bHost& host, Object stationary) {
    alignas(4) std::uint8_t entry[8];
    field<Object>(entry, 0x0) = field<Object>(stationary, 0x19C);
    field<float>(entry, 0x4) = field<float>(stationary, 0x198);
    host.registry_pair_vector_push_back_00851230(
        field<Object>(host.game_session_00e188a8(), 0x30), entry);                     // 00851894

    field<std::uint32_t>(field<Object>(stationary, 0x19C), 0xF8) = 0;
    Object owner = field<Object>(stationary, 0x19C) + 0x194;
    Object node = field<Object>(field<Object>(field<Object>(stationary, 0x19C), 0x198), 0x0);
    for (;;) {
        Object resource = field<Object>(stationary, 0x19C);
        Object head = field<Object>(resource, 0x198);
        if (owner == nullptr || owner != resource + 0x194) host.invalid_parameter_00bf6713();  // 008518D9
        if (node == head) break;
        if (owner == nullptr) host.invalid_parameter_00bf6713();                       // 008518E6
        if (node == field<Object>(owner, 0x4)) host.invalid_parameter_00bf6713();      // 008518F0
        resource = field<Object>(stationary, 0x19C);
        std::uint32_t count = field<std::uint32_t>(resource, 0xF8);
        field<Object>(resource, 0xD0 + 4 * count) = node + 8;                          // 00851904
        field<std::uint32_t>(resource, 0xF8) += 1;
        if (node == field<Object>(owner, 0x4)) host.invalid_parameter_00bf6713();      // 00851917
        node = field<Object>(node, 0x0);
    }

    host.entity_on_wrecked_base_00923040(stationary);                                  // 00851923

    if (field<std::int32_t>(host.game_session_00e188a8(), 0x1FE4) == 1 &&
        host.scene_resident_00e0af20() != 0) {
        std::uint8_t wrecked = field<std::uint8_t>(stationary, 0x1A4);
        std::uint32_t value_174 = field<std::uint32_t>(stationary, 0x174);
        alignas(4) std::uint8_t message[0x20];
        host.session_message_construct_base_0075b430(message, 0x41);                   // 00851955
        field<std::uint32_t>(message, 0x4) = 1;
        field<std::uint32_t>(message, 0x0) = 0x00D02EACu;
        field<std::uint32_t>(message, 0x18) = value_174;
        field<std::uint8_t>(message, 0x1C) = wrecked;
        host.session_message_send_0076a9f0(host.game_session_00e188a8() + 0x1EF0, message, 0);  // 00851989
    }

    if (field<std::uint8_t>(stationary, 0x1A4) == 0) return;
    if (field<std::uint8_t>(stationary, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(stationary);
    float local[4] = {field<float>(stationary, 0x188), field<float>(stationary, 0x18C),
                      field<float>(stationary, 0x190), 1.0f};
    float transformed[4];
    const float* o = host.vector4f_transform_00b62d10(local, transformed,
                                                      stationary + 0xCC);              // 008519FB
    float a = o[0];
    float b = o[1];
    float c = o[2];
    float w = o[3];
    float inv = static_cast<float>(1.0 / static_cast<double>(w));                     // 00851A31
    float q[3];
    q[0] = static_cast<float>(static_cast<double>(a) * inv);
    q[1] = static_cast<float>(static_cast<double>(b) * inv);
    q[2] = static_cast<float>(static_cast<double>(inv) * c);
    Object registry = field<Object>(host.game_session_00e188a8(), 0x19EC);
    Object definition = nullptr;
    Object source = field<Object>(stationary, 0x184);
    if (source != nullptr) {
        definition = source;
        host.interlocked_increment_00ce221c(&field<std::int32_t>(source, 0x4));       // 00851A95
    }
    Object handle = nullptr;
    host.point_effect_create_from_position_008685e0(&handle, registry, definition, q, 0, 0);  // 00851AA1
    if (handle != nullptr) field<std::uint8_t>(handle, 0x9) = 1;                       // 00851ABC
    std::int32_t group = field<std::int32_t>(stationary, 0x194);
    if (group >= 0) {
        const float zero[3] = {0.0f, 0.0f, 0.0f};
        host.point_announcement_create_0049c940(
            field<Object>(host.game_session_00e188a8(), 0x21D0), q, zero, group);       // 00851AFA
    }
    if (handle != nullptr) release_ref(host, handle);                                  // 00851B15
}

// ---------------------------------------------------------------------------
// 00845C10 BSP_Shipyard_DescribeState (shipyard vtable dword 00D0B818), __thiscall(ECX =
// shipyard; writer), RET 4, SEH frame 00C93DA8. Writer slots: [4] open(key), [0Ch]
// field(key, value), [8] close, each key / value a by-value tagged pair (tag 0 text, 1 int,
// 4 object). 0095BB10(ECX = shipyard; writer); open "_shipyard", open "stockVec"; per stock
// record (+774h..+778h, 20h bytes, index k from 0): open {1, k}; "classID" = [[r+4]+70h] or -1
// for a null class; "num" = [r+8]; "nameVecSize" = the 1Ch-byte name vector count (0 for null
// data); per name i: 00843000(buffer16, "nameVec%d", i), a stack NativeString copy of the
// element's c_str (inline buffer below capacity 10h) through 0041DD40(strlen, 1) and memcpy
// of size+1, field {0, buffer} = {0, copy or 00F87541}, then the copy is released;
// "nameIdx" = [r+1Ch]; close. Close; open "hangarVec"; per hangar (+784h..+788h, 10h bytes,
// j): open {1, j}; "object" / "path" / "entity" = {4, [h+4] / [h+8] / [h+0Ch]}; close. Close;
// open "slotVec"; per slot (+794h..+798h, 4Ch bytes, k): open {1, k}; "state" = [s+4];
// "classID" = [[s+8]+70h] or -1; "equIdx" = [s+0Ch]; found = the last hangar index whose
// record address equals [s+2Ch] (else -1); unless 00BD5680({1, found}, {1, -1}) says equal,
// "hangarIndex" = {1, found}; "entity" = {4, [s+30h]}; close. Close; open "gameUnit";
// 008360C0(ECX = [shipyard+7A0h]; writer); close; close. Every vector walk keeps the image's
// checked-iterator tests (00BF6713).
// ---------------------------------------------------------------------------
void shipyard_describe_state_00845c10(Seg52bHost& host, Object shipyard, Object writer) {
    using cc13_seg52::TaggedValue;
    auto text = [](const void* value) {
        return TaggedValue{0, reinterpret_cast<std::uintptr_t>(value)};
    };
    auto integer = [](std::int32_t value) {
        return TaggedValue{1, static_cast<std::uint32_t>(value)};
    };
    auto object = [](Object value) {
        return TaggedValue{4, reinterpret_cast<std::uintptr_t>(value)};
    };
    auto class_id = [](Object vehicle_class) {
        return vehicle_class != nullptr ? field<std::int32_t>(vehicle_class, 0x70) : -1;
    };
    host.vehicle_unit_load_archive_common_0095bb10(shipyard, writer);                 // 00845C37
    host.writer_open_vslot_04(writer, text("_shipyard"));                             // 00845C59
    host.writer_open_vslot_04(writer, text("stockVec"));                              // 00845C78

    Object stock = shipyard + 0x770;
    Object record = field<Object>(shipyard, 0x774);
    if (record > field<Object>(shipyard, 0x778)) host.invalid_parameter_00bf6713();   // 00845C96
    for (std::int32_t k = 0;; ++k) {
        Object last = field<Object>(stock, 0x8);
        if (field<Object>(stock, 0x4) > last) host.invalid_parameter_00bf6713();      // 00845CAC
        if (record == last) break;
        if (!(record < field<Object>(stock, 0x8))) host.invalid_parameter_00bf6713(); // 00845CC9
        host.writer_open_vslot_04(writer, integer(k));                                // 00845CEB
        host.writer_field_vslot_0c(writer, text("classID"),
                                   integer(class_id(field<Object>(record, 0x4))));    // 00845D2B
        host.writer_field_vslot_0c(writer, text("num"),
                                   integer(field<std::int32_t>(record, 0x8)));        // 00845D5F
        auto name_count = [&]() -> std::int32_t {
            Object data = field<Object>(record, 0x10);
            if (data == nullptr) return 0;
            return static_cast<std::int32_t>((field<Object>(record, 0x14) - data) / 0x1C);
        };
        host.writer_field_vslot_0c(writer, text("nameVecSize"), integer(name_count()));  // 00845DAF
        for (std::uint32_t i = 0;; ++i) {
            if (field<Object>(record, 0x10) == nullptr ||
                !(i < static_cast<std::uint32_t>(name_count()))) {
                break;
            }
            char buffer[16];
            format_to_buffer16_00843000(host, buffer, "nameVec%d", i);                // 00845DF6
            if (field<Object>(record, 0x10) == nullptr ||
                !(i < static_cast<std::uint32_t>(name_count()))) {
                host.invalid_parameter_00bf6713();                                    // 00845E21
            }
            Object element = field<Object>(record, 0x10) + i * 0x1C;
            const char* c_str = field<std::uint32_t>(element, 0x18) >= 0x10
                                    ? field<const char*>(element, 0x4)
                                    : reinterpret_cast<const char*>(element + 0x4);
            alignas(4) std::uint8_t copy_bytes[8];
            Object copy = copy_bytes;
            field<std::uint32_t>(copy, 0x0) = 0;
            field<std::uint32_t>(copy, 0x4) = 0;
            host.native_string_resize_0041dd40(copy, static_cast<std::uint32_t>(std::strlen(c_str)),
                                               1);                                    // 00845E62
            char* copied = field<char*>(copy, 0x4);
            if (copied != nullptr) std::memcpy(copied, c_str, field<std::uint32_t>(copy, 0x0) + 1);
            host.writer_field_vslot_0c(writer, text(buffer),
                                       text(copied != nullptr ? copied
                                                              : host.default_empty_text_00f87541()));  // 00845EC3
            if (copied != nullptr) cc13_seg52::native_string_release_storage_0084f930(host, copy);  // 00845EDC
        }
        host.writer_field_vslot_0c(writer, text("nameIdx"),
                                   integer(field<std::int32_t>(record, 0x1C)));       // 00845F31
        host.writer_close_vslot_08(writer);                                           // 00845F3A
        if (!(record < field<Object>(shipyard, 0x778))) host.invalid_parameter_00bf6713();  // 00845F4A
        record += 0x20;
    }
    host.writer_close_vslot_08(writer);                                               // 00845F63

    host.writer_open_vslot_04(writer, text("hangarVec"));                             // 00845F82
    auto hangar_count = [&]() {
        return static_cast<std::uint32_t>((field<Object>(shipyard, 0x788) -
                                           field<Object>(shipyard, 0x784)) >> 4);
    };
    for (std::uint32_t j = 0;; ++j) {
        if (field<Object>(shipyard, 0x784) == nullptr || !(j < hangar_count())) break;
        if (field<Object>(shipyard, 0x784) == nullptr || !(j < hangar_count())) {
            host.invalid_parameter_00bf6713();                                        // 00845FC4
        }
        Object hangar = field<Object>(shipyard, 0x784) + j * 0x10;
        host.writer_open_vslot_04(writer, integer(static_cast<std::int32_t>(j)));     // 00845FEC
        host.writer_field_vslot_0c(writer, text("object"), object(field<Object>(hangar, 0x4)));  // 00846013
        host.writer_field_vslot_0c(writer, text("path"), object(field<Object>(hangar, 0x8)));    // 0084603A
        host.writer_field_vslot_0c(writer, text("entity"), object(field<Object>(hangar, 0xC)));  // 00846061
        host.writer_close_vslot_08(writer);                                           // 0084606A
    }
    host.writer_close_vslot_08(writer);                                               // 00846084

    host.writer_open_vslot_04(writer, text("slotVec"));                               // 008460A3
    auto slot_count = [&]() {
        return static_cast<std::uint32_t>((field<Object>(shipyard, 0x798) -
                                           field<Object>(shipyard, 0x794)) / 0x4C);
    };
    for (std::uint32_t k = 0;; ++k) {
        if (field<Object>(shipyard, 0x794) == nullptr || !(k < slot_count())) break;
        if (field<Object>(shipyard, 0x794) == nullptr || !(k < slot_count())) {
            host.invalid_parameter_00bf6713();                                        // 00846106
        }
        Object slot = field<Object>(shipyard, 0x794) + k * 0x4C;
        host.writer_open_vslot_04(writer, integer(static_cast<std::int32_t>(k)));     // 0084612E
        host.writer_field_vslot_0c(writer, text("state"),
                                   integer(field<std::int32_t>(slot, 0x4)));          // 00846162
        host.writer_field_vslot_0c(writer, text("classID"),
                                   integer(class_id(field<Object>(slot, 0x8))));      // 008461A2
        host.writer_field_vslot_0c(writer, text("equIdx"),
                                   integer(field<std::int32_t>(slot, 0xC)));          // 008461D6
        std::int32_t found = -1;
        for (std::uint32_t j = 0;; ++j) {
            if (field<Object>(shipyard, 0x784) == nullptr || !(j < hangar_count())) break;
            if (field<Object>(shipyard, 0x784) == nullptr || !(j < hangar_count())) {
                host.invalid_parameter_00bf6713();                                    // 00846214
            }
            if (field<Object>(slot, 0x2C) == field<Object>(shipyard, 0x784) + j * 0x10) {
                found = static_cast<std::int32_t>(j);                                 // 0084622A
            }
        }
        TaggedValue found_value = integer(found);
        TaggedValue none = integer(-1);
        if (host.tagged_value_equals_00bd5680(&found_value, &none) == 0) {            // 00846263
            host.writer_field_vslot_0c(writer, text("hangarIndex"), found_value);     // 0084627F
        }
        host.writer_field_vslot_0c(writer, text("entity"), object(field<Object>(slot, 0x30)));  // 008462A6
        host.writer_close_vslot_08(writer);                                           // 008462AF
    }
    host.writer_close_vslot_08(writer);                                               // 008462CF

    Object director = field<Object>(shipyard, 0x7A0);
    host.writer_open_vslot_04(writer, text("gameUnit"));                              // 008462E7
    host.weapon_director_visit_properties_008360c0(director, writer);                // 008462EC
    host.writer_close_vslot_08(writer);                                               // 008462F8
    host.writer_close_vslot_08(writer);                                               // 00846301
}

// ---------------------------------------------------------------------------
// 008509F0 BSP_Stationary_SEntityInit (stationary entity vtable 00CFF678 +A0h), __fastcall(ECX
// = stationary), RET, SEH frame 00C944C4.
//  1. 00923840(ECX = s); s+1A0h = 0.0f; operator new(1ACh) (00BF681B); with memory,
//     007135C0(ECX = memory; s, [s+178h]->vtable[8](6, 0.95f (00CED9E0))), else 0, into +19Ch
//     (parent = [s+3Ch] is read before that store).
//  2. When the record +0C0h is set and record+4 == 1: level = 0; when the bag [record+8]
//     contains "DamageLevel" (0048E9F0) level = [008F2260(bag, "DamageLevel")+0Ch] and a
//     positive level calls s->vtable[70h](1); a non-null part gets 00710BB0(level); every
//     child of s (+48h, next +44h) answering vtable[5Ch](20h) gets vtable[1B4h](level).
//  3. node = [[s+19Ch]+160h]+0Ch (re-read each time): node->vtable[38h](s+74h);
//     00B6D890(ECX = node; [[00E188A8]+19ECh]); with a parent, 00B6E680(ECX = node;
//     [parent+3D0h]->vtable[40h]()); node+48h |= 40000000h and 00475E60(40000000h) on each
//     child (+34h, next +3Ch).
//  4. With a record (no record+4 test here): when "XOceanReflect" (00D0BE60) is found (looked up
//     twice), byte +0Ch == 0 calls 00728340(ECX = node; 2), else node+48h |= 2 and 00475E60(2)
//     on each child; when "XStaticShadowFactor" (00D0BE4C) is found (twice), s+170h = the value
//     (+0Ch as a float when +4 == 1, else CVTSI2SS of the int) and 00747560(ECX = node, EDX =
//     &s+170h).
//  5. s+168h = 00740FE0(ECX = [00E1AEA0]). A stack vector of 48h-byte shape descriptors
//     (00747CC0 reserve 1Eh) and a 84h-byte body descriptor (colour 1.0f x4 at +4h, a 3x3
//     identity in the 4x3 matrix at +14h, 1000.0f (00CE3804) at +68h / +6Ch, a raw pointer
//     array {+78h, +7Ch, +80h}, the rest zero). For each 8-byte entry of the part scene vector
//     (0084EFE0(s)+4Ch, re-fetched every step, checked-iterator tests 00BF6713): 00748150
//     pushes {0, 0, 0, 0, 4, 0, 3x3 identity, 0, 0, 0}; back() then gets +4h = 1.0f, +14h =
//     group+0Ch, +8h = 1, +0Ch = 0Dh, +3Ch..+44h = group+14h..+1Ch (group = the entry's first
//     dword), and back()'s address is appended to the raw array (grown to capacity*2+2 by
//     operator new 00BF55BE, element copy, operator delete 00BF6989).
//  6. With a part, a part scene and a non-empty raw array: the pose refresh, 00C336C0(ECX =
//     desc+14h; s+0CCh), desc+5Ch |= 1, desc+74h = s, s+1A8h = 00C5D580(ECX = [[00E188A8]+18h];
//     &desc); otherwise s+1A8h = 0. The raw array is deleted (00BF6989) and the vector's data
//     freed (_free).
// ---------------------------------------------------------------------------
namespace {
// The checked back() of the 48h-byte shape vector the body inlines: 00BF6713 when first >
// last, when last-48h is above last or below first, and when it is not below last.
Object checked_shape_back(Seg52bHost& host, Object vector) {
    Object last = field<Object>(vector, 0x8);
    if (field<Object>(vector, 0x4) > last) host.invalid_parameter_00bf6713();
    Object back = last - 0x48;
    if (back > last || back < field<Object>(vector, 0x4)) host.invalid_parameter_00bf6713();
    if (!(back < field<Object>(vector, 0x8))) host.invalid_parameter_00bf6713();
    return back;
}

void or_flags_on_node_and_children(Seg52bHost& host, Object node, std::uint32_t mask) {
    Object child = field<Object>(node, 0x34);
    field<std::uint32_t>(node, 0x48) |= mask;
    for (; child != nullptr; child = field<Object>(child, 0x3C)) {
        host.node_or_flags_recursive_00475e60(child, mask);
    }
}
}  // namespace

void stationary_sentity_init_008509f0(Seg52bHost& host, Object stationary) {
    host.entity_inherit_race_and_activate_00923840(stationary);                       // 00850A15
    field<float>(stationary, 0x1A0) = 0.0f;
    Object memory = host.operator_new_00bf681b(0x1AC);                                 // 00850A2A
    Object part = nullptr;
    if (memory != nullptr) {
        constexpr float kPartModelScale_00ced9e0 = 0.949999988079071f;
        Object model = host.model_vslot_08(field<Object>(stationary, 0x178), 6,
                                           kPartModelScale_00ced9e0);                 // 00850A5C
        part = host.unit_part_instance_construct_007135c0(memory, stationary, model);  // 00850A62
    }
    Object parent = field<Object>(stationary, 0x3C);
    field<Object>(stationary, 0x19C) = part;

    Object record = field<Object>(stationary, 0xC0);
    if (record != nullptr && field<std::int32_t>(record, 0x4) == 1) {
        Object bag = field<Object>(record, 0x8);
        std::int32_t level = 0;
        if (host.property_bag_contains_0048e9f0(bag, "DamageLevel") != 0) {           // 00850A9B
            level = field<std::int32_t>(host.property_bag_find_008f2260(bag, "DamageLevel"), 0xC);  // 00850AAB
            if (level > 0) host.stationary_vslot_70(stationary, 1);                    // 00850AC0
        }
        if (field<Object>(stationary, 0x19C) != nullptr) {
            host.model_groups_select_visible_group_00710bb0(field<Object>(stationary, 0x19C), level);  // 00850ACD
        }
        for (Object child = field<Object>(stationary, 0x48); child != nullptr;
             child = field<Object>(child, 0x44)) {
            if (host.unit_is_kind_of_vslot_5c(child, 0x20) != 0) {                    // 00850AE9
                host.child_vslot_1b4(child, level);                                   // 00850AFA
            }
        }
    }

    auto scene_node = [&]() {
        return field<Object>(field<Object>(field<Object>(stationary, 0x19C), 0x160), 0xC);
    };
    host.scene_node_vslot_38(scene_node(), stationary + 0x74);                         // 00850B1B
    host.node_propagate_root_registration_00b6d890(
        scene_node(), field<Object>(host.game_session_00e188a8(), 0x19EC));            // 00850B3B
    if (parent != nullptr) {
        Object node = scene_node();
        Object attach = host.object_vslot_40(field<Object>(parent, 0x3D0));            // 00850B5E
        host.node_set_parent_00b6e680(node, attach);                                   // 00850B63
    }
    or_flags_on_node_and_children(host, scene_node(), 0x40000000u);                    // 00850B7A / 00850B97

    if (field<Object>(stationary, 0xC0) != nullptr) {
        if (host.property_bag_find_008f2260(field<Object>(field<Object>(stationary, 0xC0), 0x8),
                                            "XOceanReflect") != nullptr) {             // 00850BC6
            Object value = host.property_bag_find_008f2260(
                field<Object>(field<Object>(stationary, 0xC0), 0x8), "XOceanReflect");  // 00850BDD
            if (field<std::uint8_t>(value, 0xC) == 0) {
                host.scene_node_clear_flags_recursive_00728340(scene_node(), 2);       // 00850BFA
            } else {
                or_flags_on_node_and_children(host, scene_node(), 2);                  // 00850C13 / 00850C23
            }
        }
        if (host.property_bag_find_008f2260(field<Object>(field<Object>(stationary, 0xC0), 0x8),
                                            "XStaticShadowFactor") != nullptr) {       // 00850C3D
            Object value = host.property_bag_find_008f2260(
                field<Object>(field<Object>(stationary, 0xC0), 0x8), "XStaticShadowFactor");  // 00850C54
            float factor = field<std::int32_t>(value, 0x4) == 1
                               ? field<float>(value, 0xC)
                               : static_cast<float>(field<std::int32_t>(value, 0xC));
            field<float>(stationary, 0x170) = factor;
            host.generated_model_register_shadow_factor_00747560(scene_node(),
                                                                 &field<float>(stationary, 0x170));  // 00850C86
        }
    }

    field<Object>(stationary, 0x168) =
        host.decal_list_register_owner_00740fe0(host.decal_manager_00e1aea0());       // 00850C91
    alignas(4) std::uint8_t shape_vector_bytes[0x10];
    Object shapes = shape_vector_bytes;
    field<Object>(shapes, 0x4) = nullptr;
    field<Object>(shapes, 0x8) = nullptr;
    field<Object>(shapes, 0xC) = nullptr;
    host.shape_vector_reserve_00747cc0(shapes, 0x1E);                                  // 00850CB9
    alignas(4) std::uint8_t body_bytes[0x84];
    Object body = body_bytes;
    for (std::size_t offset : {0x4u, 0x8u, 0xCu, 0x10u, 0x14u, 0x24u, 0x34u}) field<float>(body, offset) = 1.0f;
    for (std::size_t offset = 0x18; offset <= 0x64; offset += 4) {
        if (offset != 0x24 && offset != 0x34 && offset != 0x5C) field<float>(body, offset) = 0.0f;
    }
    field<std::uint32_t>(body, 0x78) = 0;
    field<std::uint32_t>(body, 0x7C) = 0;
    field<std::uint32_t>(body, 0x80) = 0;
    field<std::uint8_t>(body, 0x0) = 0;
    field<std::uint32_t>(body, 0x5C) = 0;
    field<float>(body, 0x68) = 1000.0f;   // 00CE3804
    field<float>(body, 0x6C) = 1000.0f;
    field<std::uint32_t>(body, 0x74) = 0;
    field<std::uint32_t>(body, 0x70) = 0;

    bool created = false;
    Object scene = field<Object>(stationary, 0x19C) != nullptr
                       ? field<Object>(field<Object>(stationary, 0x19C), 0x160)
                       : nullptr;
    if (scene != nullptr) {
        Object owner = scene + 0x4C;
        Object entry = field<Object>(scene, 0x50);
        if (entry > field<Object>(scene, 0x54)) host.invalid_parameter_00bf6713();     // 00850DEB
        for (;;) {
            Object current = host.stationary_part_scene_0084efe0(stationary);         // 00850E02
            Object last = field<Object>(current, 0x54);
            if (field<Object>(current, 0x50) > last) host.invalid_parameter_00bf6713();  // 00850E14
            if (owner == nullptr || owner != current + 0x4C) host.invalid_parameter_00bf6713();  // 00850E24
            if (entry == last) break;
            if (owner == nullptr) host.invalid_parameter_00bf6713();                   // 00850E3C
            if (!(entry < field<Object>(owner, 0x8))) host.invalid_parameter_00bf6713();  // 00850E49
            Object group = field<Object>(entry, 0x0);
            alignas(4) std::uint8_t shape_bytes[0x48];
            Object shape = shape_bytes;
            field<float>(shape, 0x0) = 0.0f;
            field<float>(shape, 0x4) = 0.0f;
            field<std::uint32_t>(shape, 0x8) = 0;
            field<std::uint32_t>(shape, 0xC) = 0;
            field<std::uint32_t>(shape, 0x10) = 4;
            field<std::uint32_t>(shape, 0x14) = 0;
            for (std::size_t offset = 0x18; offset <= 0x44; offset += 4) field<float>(shape, offset) = 0.0f;
            field<float>(shape, 0x18) = 1.0f;
            field<float>(shape, 0x28) = 1.0f;
            field<float>(shape, 0x38) = 1.0f;
            host.shape_vector_push_back_00748150(shapes, shape);                       // 00850F0B
            field<float>(checked_shape_back(host, shapes), 0x4) = 1.0f;                // 00850F56
            field<Object>(checked_shape_back(host, shapes), 0x14) = group + 0xC;       // 00850F9C
            field<std::uint32_t>(checked_shape_back(host, shapes), 0x8) = 1;           // 00850FDD
            field<std::uint32_t>(checked_shape_back(host, shapes), 0xC) = 0xD;         // 00851022
            std::uint32_t extent[3] = {field<std::uint32_t>(group, 0x14),
                                       field<std::uint32_t>(group, 0x18),
                                       field<std::uint32_t>(group, 0x1C)};
            Object target = checked_shape_back(host, shapes);
            field<std::uint32_t>(target, 0x3C) = extent[0];                            // 00851094
            field<std::uint32_t>(target, 0x40) = extent[1];
            field<std::uint32_t>(target, 0x44) = extent[2];
            Object back = checked_shape_back(host, shapes);                            // 008510CE
            std::uint32_t size = field<std::uint32_t>(body, 0x7C);
            if (size == field<std::uint32_t>(body, 0x80)) {
                std::uint32_t capacity = size + size + 2;
                field<std::uint32_t>(body, 0x80) = capacity;
                Object grown = host.operator_new_00bf55be(capacity * 4);               // 008510FE
                Object* old = field<Object*>(body, 0x78);
                for (std::uint32_t i = 0; i < field<std::uint32_t>(body, 0x7C); ++i) {
                    Object* slot = reinterpret_cast<Object*>(grown) + i;
                    if (slot != nullptr) *slot = old[i];
                }
                if (old != nullptr) host.operator_delete_00bf6989(old);               // 00851140
                field<Object>(body, 0x78) = grown;
            }
            Object* slot = field<Object*>(body, 0x78) + field<std::uint32_t>(body, 0x7C);
            if (slot != nullptr) *slot = back;
            field<std::uint32_t>(body, 0x7C) += 1;
            if (!(entry < field<Object>(owner, 0x8))) host.invalid_parameter_00bf6713();  // 0085117B
            entry += 8;
        }
        if (field<std::uint32_t>(body, 0x7C) != 0) {
            if (field<std::uint8_t>(stationary, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(stationary);
            host.matrix_copy_00c336c0(body + 0x14, stationary + 0xCC);                 // 008511B7
            field<std::uint32_t>(body, 0x5C) |= 1;
            field<Object>(body, 0x74) = stationary;
            field<Object>(stationary, 0x1A8) = host.dyn_world_create_body_00c5d580(
                field<Object>(host.game_session_00e188a8(), 0x18), body);              // 008511D9
            created = true;
        }
    }
    if (!created) field<Object>(stationary, 0x1A8) = nullptr;                          // 008511E6
    if (field<Object>(body, 0x78) != nullptr) host.operator_delete_00bf6989(field<Object>(body, 0x78));  // 00851200
    if (field<Object>(shapes, 0x4) != nullptr) host.free_00bf65ac(field<Object>(shapes, 0x4));       // 00851211
}

// ---------------------------------------------------------------------------
// 00851CB0 BSP_StationaryClass_GetOrCreate, __fastcall(ECX = const NativeString* type name)
// -> EAX class, SEH frame 00C94578. slot = 00851B40(ECX = map 00F87550; name). An existing
// class only gets vtable[0Ch](). Otherwise: globals = 00B67980(ECX = [00E188A8]+1A0Ch;
// &globals); table = 00B67800(ECX = &globals; &table, "StationaryClass" (00D0BE9C)); row =
// 00B68100(ECX = &table; &row, name); operator new(3Ch) (00BF681B) and, when non-null, the
// inline class constructor (vptr 00D0BE38; +8h, +0Ch, +10h, +14h, +28h, +18h = 0; byte +1Ch
// = 0; +4h = 15.0f (00CE5380); +38h = -1) with no null test after it; the NativeString +8h
// takes the name (copy idiom); class->vtable[8](&row) (00851580 for this vtable);
// *00851B40(name) = class; class->vtable[10h](); row, table and globals destructed
// (00B67700); then vtable[0Ch]() as for an existing class. The game_hosts_lua.cpp
// stationary_class_exists model mirrors only the lookup.
// ---------------------------------------------------------------------------
Object stationary_class_get_or_create_00851cb0(Seg52bHost& host, Object name) {
    Object map = host.stationary_class_map_00f87550();
    Object stationary_class = *host.stationary_class_map_subscript_00851b40(map, name);  // 00851CD3
    if (stationary_class == nullptr) {
        cc13_seg52::LuaObject globals;
        host.lua_state_owner_get_globals_00b67980(host.game_session_00e188a8() + 0x1A0C,
                                                  &globals);                           // 00851CF5
        cc13_seg52::LuaObject table;
        host.lua_object_get_by_name_00b67800(&globals, &table, "StationaryClass");      // 00851D0C
        cc13_seg52::LuaObject row;
        host.lua_object_get_by_native_string_00b68100(&table, &row, name);              // 00851D20
        Object memory = host.operator_new_00bf681b(0x3C);                               // 00851D2C
        if (memory != nullptr) {
            field<std::uint32_t>(memory, 0x0) = 0x00D0BE38u;
            field<std::uint32_t>(memory, 0x8) = 0;
            field<std::uint32_t>(memory, 0xC) = 0;
            field<std::uint32_t>(memory, 0x10) = 0;
            field<std::uint32_t>(memory, 0x14) = 0;
            field<std::uint32_t>(memory, 0x28) = 0;
            field<std::uint32_t>(memory, 0x18) = 0;
            field<std::uint8_t>(memory, 0x1C) = 0;
            field<float>(memory, 0x4) = 15.0f;
            field<std::int32_t>(memory, 0x38) = -1;
        }
        stationary_class = memory;
        copy_native_string(host, stationary_class + 0x8, name);                         // 00851D80 / 00851D95
        host.stationary_class_vslot_08(stationary_class, &row);                         // 00851DA9
        *host.stationary_class_map_subscript_00851b40(map, name) = stationary_class;    // 00851DB1
        host.class_vslot_10(stationary_class);                                          // 00851DBF
        host.lua_object_destruct_00b67700(&row);                                        // 00851DCA
        host.lua_object_destruct_00b67700(&table);                                      // 00851DD7
        host.lua_object_destruct_00b67700(&globals);                                    // 00851DE8
    }
    host.stationary_class_vslot_0c(stationary_class);                                   // 00851DF9
    return stationary_class;
}

// ---------------------------------------------------------------------------
// 008437D0 BSP_Shipyard_ApplySpawnMessage (case AAh of 00847030), __thiscall(ECX = shipyard;
// message), RET 4. slot = the checked slot [message+20h] of +794h..+798h (4Ch bytes; 00BF6713
// for null data or index >= count): slot+4h = 3; hangar = the checked hangar [message+24h] of
// +784h..+788h (10h bytes): slot+2Ch = hangar. entity = the word handle message+28h resolved
// inline through the two 10h-byte handle tables (id < [00F89A10] (signed JGE): [[00F89A54] +
// (id - [00F89A0C])*10h + 0Ch], else [[00F89AA8] + (id - [00F89A60])*10h + 0Ch]): slot+30h =
// entity and hangar+0Ch = entity. 00694A60(ECX = [slot+30h], EDX = shipyard ? shipyard+10h :
// 0); then a non-null [slot+30h] gets vtable[0D8h](). The cc9 host's shipyard state model
// (game_hosts_gunnery.cpp) covers the same arm semantically.
// ---------------------------------------------------------------------------
void shipyard_apply_spawn_message_008437d0(Seg52bHost& host, Object shipyard, Object message) {
    std::uint32_t slot_index = field<std::uint32_t>(message, 0x20);
    Object slots = field<Object>(shipyard, 0x794);
    if (slots == nullptr ||
        !(slot_index < static_cast<std::uint32_t>((field<Object>(shipyard, 0x798) - slots) / 0x4C))) {
        host.invalid_parameter_00bf6713();                                              // 00843804
    }
    Object slot = field<Object>(shipyard, 0x794) + slot_index * 0x4C;
    field<std::uint32_t>(slot, 0x4) = 3;
    Object hangars = field<Object>(shipyard, 0x784);
    std::uint32_t hangar_index = field<std::uint32_t>(message, 0x24);
    if (hangars == nullptr ||
        !(hangar_index < static_cast<std::uint32_t>((field<Object>(shipyard, 0x788) - hangars) >> 4))) {
        host.invalid_parameter_00bf6713();                                              // 00843835
    }
    Object hangar = field<Object>(shipyard, 0x784) + (hangar_index << 4);
    field<Object>(slot, 0x2C) = hangar;
    std::int32_t id = field<std::uint16_t>(message, 0x28);
    Object entity;
    if (id < host.handle_split_00f89a10()) {
        entity = field<Object>(host.handle_table_a_00f89a54(),
                               static_cast<std::uint32_t>(id - host.handle_base_a_00f89a0c()) * 0x10 + 0xC);
    } else {
        entity = field<Object>(host.handle_table_b_00f89aa8(),
                               static_cast<std::uint32_t>(id - host.handle_base_b_00f89a60()) * 0x10 + 0xC);
    }
    field<Object>(slot, 0x30) = entity;
    field<Object>(hangar, 0xC) = entity;
    host.observer_register_pair_00694a60(field<Object>(slot, 0x30),
                                         shipyard != nullptr ? shipyard + 0x10 : nullptr);  // 00843893
    Object bound = field<Object>(slot, 0x30);
    if (bound != nullptr) host.object_vslot_d8(bound);                                  // 008438AB
}

}  // namespace bsp::cc13_seg52b
