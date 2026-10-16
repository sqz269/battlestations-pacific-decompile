// cc13_large3 reconstructions (packet cc13-large3). See include/bsp/cc13_large3.hpp for the
// model and docs/CC13_LARGE3.md for the evidence. Names are hypotheses, not recovered symbols.

#include "bsp/cc13_large3.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <utility>

namespace bsp::cc13_large3 {

namespace {

std::uint32_t to_u32(const void* p) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
ArchiveArg name_key(const char* name) { return ArchiveArg{0, to_u32(name)}; }
ArchiveArg index_key(std::uint32_t index) { return ArchiveArg{1, index}; }
std::uint32_t float_bits(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}

// The inline intrusive-pointer reset: p = *slot; when non-null, InterlockedDecrement(p+4)
// ([00CE2220]) reaching 0 calls p->vtable[0](); then *slot = 0 (stored twice in the image, once
// inside the non-null branch and once after it).
void ref_reset(Large3Host& host, void*& slot) {
    void* const object = slot;
    if (object != nullptr) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 0x04)) == 0) {
            host.refcounted_destroy_vslot_00(object);
        }
        slot = nullptr;
    }
    slot = nullptr;
}

// 00867B10 on the effect, then effect+9h = 1.
void effect_stop(Large3Host& host, void* effect) {
    host.point_effect_stop_children_00867b10(effect);
    at<std::uint8_t>(effect, 0x09) = 1;
}

// One "if (effect) { stop; reset }" block; the unspecified-bool test NEG/SBB/TEST 0E186ECh is
// a plain non-null test.
void stop_and_reset(Large3Host& host, void* ship, std::size_t offset) {
    void* const effect = ptr(ship, offset);
    if (effect == nullptr) return;
    effect_stop(host, effect);
    ref_reset(host, at<void*>(ship, offset));
}

std::int32_t state_count(void* frame_box) {   // IMUL 2E8BA2E9h / SAR 3 / sign fix: /2Ch
    const std::int32_t bytes =
        static_cast<std::int32_t>(at<std::uint32_t>(frame_box, 0xF8) -
                                  at<std::uint32_t>(frame_box, 0xF4));
    return bytes / static_cast<std::int32_t>(kFrameBoxStateStride);
}

const char kEmptyF8C1D0[] = "";   // 00F8C1D0, the NativeString empty-data fallback
const char kEmptyCE3A0C[] = "";   // 00CE3A0C, the "Texture" default
const char kEmptyF88C34[] = "";   // 00F88C34, the substring empty-data fallback in 008EC650

}  // namespace

// ---------------------------------------------------------------------------------------------
// 00818970-00818E96, __thiscall(ECX = ship), plain RET. Stops and releases the ship's effect
// handles. EBP holds [00CE2220] (InterlockedDecrement) for the first eight blocks.
//   Paired blocks: a non-null effect at A is stopped (00867B10, +9h = 1), A is reset, then B is
//   reset; both resets sit inside A's non-null branch (JZ 008189EC skips them).
//   Single blocks +9F0h, +9F4h, +9E8h, +9ECh: stop and reset the same slot.
//   +A00h..+A0Ch (00818C53, EBX = 4): stop when non-null; the reset runs for every slot.
//   [+A14h] array with count +A18h (00818CAC): stop and reset each non-null element.
//   vector +1118h/+111Ch (00818D10): checked-index loop; 00BF6713 at each of its three range
//   checks (they cannot fire while the loop condition holds, but the image tests them).
//   +B54h..+B64h (EBX = 5) then +B44h..+B50h (EBP = EBX + 4 = 4 after the first loop).
// ---------------------------------------------------------------------------------------------
void ship_stop_wreck_effects_00818970(Large3Host& host, void* ship) {
    static constexpr std::size_t kPairs[][2] = {
        {0xBB0, 0xBC0}, {0xBAC, 0xBBC}, {0xBA4, 0xBB4}, {0xBA8, 0xBB8}};
    for (const auto& pair : kPairs) {
        void* const effect = ptr(ship, pair[0]);
        if (effect == nullptr) continue;
        effect_stop(host, effect);
        ref_reset(host, at<void*>(ship, pair[0]));
        ref_reset(host, at<void*>(ship, pair[1]));
    }
    for (std::size_t offset : {0x9F0u, 0x9F4u, 0x9E8u, 0x9ECu}) stop_and_reset(host, ship, offset);

    for (std::size_t offset = 0xA00; offset != 0xA10; offset += 4) {
        void* const effect = ptr(ship, offset);
        if (effect != nullptr) effect_stop(host, effect);
        ref_reset(host, at<void*>(ship, offset));
    }

    void** element = static_cast<void**>(ptr(ship, 0xA14));
    void** const end = element + at<std::int32_t>(ship, 0xA18);
    for (; element != end; ++element) {
        void* const effect = *element;
        if (effect == nullptr) continue;
        effect_stop(host, effect);
        ref_reset(host, *element);
    }

    auto in_range = [ship](std::uint32_t index) {
        const std::uint32_t first = at<std::uint32_t>(ship, 0x1118);
        return first != 0 &&
               index < ((at<std::uint32_t>(ship, 0x111C) - first) >> 2);   // SAR 2, JNC
    };
    for (std::uint32_t i = 0; in_range(i); ++i) {
        if (!in_range(i)) host.invalid_iterator_00bf6713();
        if (static_cast<void**>(ptr(ship, 0x1118))[i] == nullptr) continue;
        if (!in_range(i)) host.invalid_iterator_00bf6713();
        effect_stop(host, static_cast<void**>(ptr(ship, 0x1118))[i]);
        if (!in_range(i)) host.invalid_iterator_00bf6713();
        ref_reset(host, static_cast<void**>(ptr(ship, 0x1118))[i]);
    }

    for (std::size_t offset = 0xB54; offset != 0xB68; offset += 4) stop_and_reset(host, ship, offset);
    for (std::size_t offset = 0xB44; offset != 0xB54; offset += 4) stop_and_reset(host, ship, offset);
}

// ---------------------------------------------------------------------------------------------
// 00818340-00818965, __thiscall(ECX = ship, archive), RET 4. The "_ship" state dump.
// The first five values are written only when 00BD5680 says they differ from a default:
//   sinkTime (+828h, float) vs 0.0; steeringJam (+9E4h) and engineJam (+9E5h), kind 3, vs 0;
//   turnEfficiency (+9DCh) and thrustMod (+9D8h) vs 1.0f (00D7A24C).
// The two jam values reuse the stack slot that held sinkTime and overwrite only its low byte
// (MOV byte [ESP+14h], AL), so the dword passed to the sink carries sinkTime's upper three bytes;
// kind 3 compares one byte (00BD5680) and the value below reproduces the dword exactly.
// The speed ("spd") is 0092D730(ECX = [ship+1018h]) stored through FSTP float.
// The weapon director and navigator pointers (+738h, +73Ch) are read before their block opens.
// ---------------------------------------------------------------------------------------------
void ship_describe_state_00818340(Large3Host& host, void* ship, void* archive) {
    host.vehicle_unit_write_state_0095bb10(ship, archive);
    host.sink_begin_vslot_04(archive, name_key("_ship"));

    const std::uint32_t sink_bits = at<std::uint32_t>(ship, 0x828);
    {
        const ArchiveArg current{2, sink_bits};
        const ArchiveArg fallback{2, float_bits(0.0f)};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(archive, name_key("sinkTime"), current);
        }
    }
    for (const auto& jam : {std::pair<std::size_t, const char*>{0x9E4, "steeringJam"},
                            std::pair<std::size_t, const char*>{0x9E5, "engineJam"}}) {
        const ArchiveArg current{3, (sink_bits & 0xFFFFFF00u) | at<std::uint8_t>(ship, jam.first)};
        const ArchiveArg fallback{3, 0};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(archive, name_key(jam.second), current);
        }
    }
    for (const auto& scale : {std::pair<std::size_t, const char*>{0x9DC, "turnEfficiency"},
                              std::pair<std::size_t, const char*>{0x9D8, "thrustMod"}}) {
        const ArchiveArg current{2, at<std::uint32_t>(ship, scale.first)};
        const ArchiveArg fallback{2, float_bits(kUnitScale)};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(archive, name_key(scale.second), current);
        }
    }

    host.sink_write_vslot_0c(archive, name_key("camoColor"),
                             ArchiveArg{1, at<std::uint32_t>(ship, 0x115C)});
    host.sink_write_vslot_0c(archive, name_key("camoColorGun"),
                             ArchiveArg{8, to_u32(add(ship, 0x1160))});
    host.sink_write_vslot_0c(archive, name_key("torpedoStock"),
                             ArchiveArg{1, at<std::uint32_t>(ship, 0x104C)});
    host.sink_write_vslot_0c(archive, name_key("thrust"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x980)});
    const float speed =
        static_cast<float>(host.motion_body_axis_speed_0092d730(ptr(ship, 0x1018)));
    host.sink_write_vslot_0c(archive, name_key("spd"), ArchiveArg{2, float_bits(speed)});
    host.sink_write_vslot_0c(archive, name_key("toTurn"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x984)});
    host.sink_write_vslot_0c(archive, name_key("maxSpeed"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x9C0)});
    host.sink_write_vslot_0c(archive, name_key("shipYardLaunch"),
                             ArchiveArg{1, at<std::uint32_t>(ship, 0x1130)});

    host.sink_begin_vslot_04(archive, name_key("repairPreferences"));
    for (int i = 0; i < kShipRepairPreferenceCount; ++i) {
        host.sink_write_vslot_0c(archive, index_key(static_cast<std::uint32_t>(i)),
                                 ArchiveArg{1, at<std::uint32_t>(ship, 0x1134 + 4 * i)});
    }
    host.sink_end_vslot_08(archive);
    host.sink_write_vslot_0c(archive, name_key("repairTimer"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x114C)});

    host.sink_begin_vslot_04(archive, name_key("helmsmanControl"));
    host.sink_write_vslot_0c(archive, name_key("thrust"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x9A0)});
    host.sink_write_vslot_0c(archive, name_key("toTurn"),
                             ArchiveArg{2, at<std::uint32_t>(ship, 0x9A4)});
    host.sink_end_vslot_08(archive);

    if (host.unit_is_occupant_owner_00778890(ship)) {
        host.sink_begin_vslot_04(archive, name_key("formaciosGenya"));
        host.sink_begin_vslot_04(archive, name_key("pathstuff"));
        for (int i = 0; i < kShipPathPointCount; ++i) {
            const std::size_t record = 0xBD8 + kShipPathPointStride * i;   // EBP - 10h
            host.sink_begin_vslot_04(archive, index_key(static_cast<std::uint32_t>(i)));
            host.sink_write_vslot_0c(archive, name_key("point"),
                                     ArchiveArg{5, to_u32(add(ship, record))});
            host.sink_write_vslot_0c(archive, name_key("dist"),
                                     ArchiveArg{2, at<std::uint32_t>(ship, record + 0x10)});
            host.sink_write_vslot_0c(archive, name_key("dir"),
                                     ArchiveArg{2, at<std::uint32_t>(ship, record + 0x0C)});
            host.sink_end_vslot_08(archive);
        }
        host.sink_end_vslot_08(archive);
        host.sink_write_vslot_0c(archive, name_key("actpoint"),
                                 ArchiveArg{1, at<std::uint32_t>(ship, 0xF98)});
        host.sink_end_vslot_08(archive);
    }

    void* const director = ptr(ship, 0x738);
    host.sink_begin_vslot_04(archive, name_key("gameUnit"));
    host.weapon_director_dump_properties_008362a0(director, archive);
    host.sink_end_vslot_08(archive);

    void* const navigator = ptr(ship, 0x73C);
    host.sink_begin_vslot_04(archive, name_key("navigatorParams"));
    host.navigator_dump_properties_009d5e00(navigator, archive);
    host.sink_end_vslot_08(archive);

    host.sink_begin_vslot_04(archive, name_key("leakManager"));
    host.leak_manager_write_zones_0074ef20(add(ship, 0x10D4), archive);
    host.sink_end_vslot_08(archive);
    host.sink_end_vslot_08(archive);
}

// ---------------------------------------------------------------------------------------------
// 00AD0290-00AD067B, __thiscall(ECX = frame box, sink), RET 4.
// When the state vector (+F4h first, +F8h last, 2Ch records) holds exactly one record and the
// word +ECh is 0, a size equal to 00ACF0C0(out, 0) (two x87 FUCOMIP equalities; JP skips on
// unequal or unordered) is cleared to 0 for the base describe, and the saved +20h/+24h are put
// back after it. HasTexture's current dword holds the +100h byte over uninitialized stack bytes
// ([ESP+14h] is not written earlier); the model passes zero there (kind 3 compares one byte).
// ---------------------------------------------------------------------------------------------
void gui_frame_box_describe_properties_00ad0290(Large3Host& host, void* frame_box, void* sink) {
    const std::uint32_t saved_20 = at<std::uint32_t>(frame_box, 0x20);
    const std::uint32_t saved_24 = at<std::uint32_t>(frame_box, 0x24);
    if (at<std::uint32_t>(frame_box, 0xF4) != 0 && state_count(frame_box) == 1 &&
        at<std::uint16_t>(frame_box, 0xEC) == 0) {
        alignas(4) std::uint8_t out[8];
        const float* const size = host.frame_box_state_size_00acf0c0(frame_box, out, 0);
        if (at<float>(frame_box, 0x20) == size[0] && at<float>(frame_box, 0x24) == size[1]) {
            at<float>(frame_box, 0x20) = 0.0f;
            at<float>(frame_box, 0x24) = 0.0f;
        }
    }
    host.gui_widget_describe_properties_00aaaed0(frame_box, sink);
    at<std::uint32_t>(frame_box, 0x20) = saved_20;
    at<std::uint32_t>(frame_box, 0x24) = saved_24;

    {
        const ArchiveArg current{3, at<std::uint8_t>(frame_box, 0x100)};
        // default {3, byte 1}; its upper bytes are the saved +24h copy's (unread by kind 3).
        const ArchiveArg fallback{3, (saved_24 & 0xFFFFFF00u) | 1u};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(sink, name_key("HasTexture"), current);
        }
    }
    {
        const float defaults_x[2] = {kFrameSizesXDefault, kFrameSizesXDefault};
        const ArchiveArg current{6, to_u32(add(frame_box, 0x10C))};
        const ArchiveArg fallback{6, to_u32(defaults_x)};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(sink, name_key("FrameSizesX"), current);
        }
    }
    {
        const float defaults_y[2] = {kFrameSizesYDefault, kFrameSizesYDefault};
        const ArchiveArg current{6, to_u32(add(frame_box, 0x114))};
        const ArchiveArg fallback{6, to_u32(defaults_y)};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(sink, name_key("FrameSizesY"), current);
        }
    }
    if (at<std::uint32_t>(frame_box, 0x104) != 0) {   // shader NativeString size
        const char* shader = at<const char*>(frame_box, 0x108);
        if (shader == nullptr) shader = kEmptyF8C1D0;
        const ArchiveArg current{0, to_u32(shader)};
        const ArchiveArg fallback{0, to_u32("GuiDefault.mshd")};
        if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
            host.sink_write_vslot_0c(sink, name_key("ShaderName"), current);
        }
    }

    if (at<std::uint32_t>(frame_box, 0xF4) == 0 || state_count(frame_box) == 0) return;
    host.sink_begin_vslot_04(sink, name_key("States"));
    std::uint32_t i = 0;
    std::size_t offset = 0;
    while (at<std::uint32_t>(frame_box, 0xF4) != 0 &&
           i < static_cast<std::uint32_t>(state_count(frame_box))) {   // JNC
        host.sink_begin_vslot_04(sink, index_key(i + 1));
        if (at<std::uint32_t>(frame_box, 0xF4) == 0 ||
            i >= static_cast<std::uint32_t>(state_count(frame_box))) {
            host.invalid_iterator_00bf6713();
        }
        void* const record = add(ptr(frame_box, 0xF4), offset);
        {
            const char* texture = at<const char*>(record, 0x08);   // NativeString +4h data
            if (texture == nullptr) texture = kEmptyF8C1D0;
            const ArchiveArg current{0, to_u32(texture)};
            const ArchiveArg fallback{0, to_u32(kEmptyCE3A0C)};
            if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
                host.sink_write_vslot_0c(sink, name_key("Texture"), current);
            }
        }
        {
            const ArchiveArg current{6, to_u32(add(record, 0x14))};
            const ArchiveArg fallback{6, to_u32(add(frame_box, 0x20))};
            if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
                host.sink_write_vslot_0c(sink, name_key("Size"), current);
            }
        }
        {
            const ArchiveArg current{6, to_u32(add(record, 0x0C))};
            const ArchiveArg fallback{6, to_u32(add(frame_box, 0x18))};
            if (!host.tagged_value_equals_00bd5680(&current, &fallback)) {
                host.sink_write_vslot_0c(sink, name_key("Pivot"), current);
            }
        }
        host.sink_end_vslot_08(sink);
        i += 1;
        offset += kFrameBoxStateStride;
    }
    host.sink_end_vslot_08(sink);
}


// ---------------------------------------------------------------------------------------------
// NativeString helpers shared by 00AD1A80 and 00A96750.
// ---------------------------------------------------------------------------------------------
namespace {

// The inline NativeString destructor: when data is non-null, the pooled release
// 00419CC0(data, size + 1, 1) / 00BD1510(ECX = pool) with the same three stack arguments.
void inline_destroy(Large3Host& host, NativeStringRaw& s) {
    if (s.data != nullptr) host.string_pool_release_00419cc0_00bd1510(s.data, s.size + 1);
}

// The inline literal construction: {0, 0}, 0041DD40(1, 1), then memcpy(data, literal, size + 1)
// (00BF7680, CRT) when data is non-null.
void construct_literal(Large3Host& host, NativeStringRaw& s, const char* literal) {
    s.size = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, 1, 1);
    if (s.data != nullptr) std::memcpy(s.data, literal, s.size + 1);
}

}  // namespace

// ---------------------------------------------------------------------------------------------
// 00AD1A80-00AD1E73, __thiscall(ECX = frame box, const NativeString* prefix), RET 4; SEH frame
// (handler 00CB9238). Same shape as 00AB4CE0 (engine_seg49_64_78_cc13.cpp) over the 2Ch state
// records of the vector at +F0h (+F4h first, +F8h last). Per record i:
//   literals ")" (00CE584C), "(" (00CE5850), ":" (00CF00B0), "." (00CE3A70), in that order;
//   t1 = prefix + "."; t2 = t1 + i (004263B0); t3 = t2 + ":"; t4 = t3 + name(record+4h);
//   t5 = t4 + "("; t6 = t5 + *00B33E40(ECX = [record+0]); final = t6 + ")";
//   inline destroys t6, t5, t4, t3, t2, t1, ".", ":", "(", ")", final. The path is unused.
// Then the base 00AAA0F0(prefix). Checked-iterator tests call 00BF6713 at 00AD1ABD (first >
// last), 00AD1AE1 (first > last, each pass), 00AD1B09 and 00AD1E3F (it >= last). The container
// identity tests at 00AD1AE6-00AD1AFF compare frame+F0h with itself and with null; they cannot
// fire for a frame box pointer other than -F0h and are not modelled.
// ---------------------------------------------------------------------------------------------
void gui_frame_box_visit_child_paths_00ad1a80(Large3Host& host, void* frame_box,
                                              const void* prefix) {
    std::uintptr_t it = at<std::uintptr_t>(frame_box, 0xF4);
    if (it > at<std::uintptr_t>(frame_box, 0xF8)) host.invalid_iterator_00bf6713();
    std::int32_t i = 0;
    for (;;) {
        const std::uintptr_t last = at<std::uintptr_t>(frame_box, 0xF8);
        if (at<std::uintptr_t>(frame_box, 0xF4) > last) host.invalid_iterator_00bf6713();
        if (it == last) break;
        if (it >= at<std::uintptr_t>(frame_box, 0xF8)) host.invalid_iterator_00bf6713();
        void* const record = reinterpret_cast<void*>(it);

        NativeStringRaw close, open, colon, dot;
        construct_literal(host, close, ")");
        construct_literal(host, open, "(");
        construct_literal(host, colon, ":");
        construct_literal(host, dot, ".");
        NativeStringRaw t1{}, t2{}, t3{}, t4{}, t5{}, t6{}, path{};
        NativeStringRaw* s = host.native_string_concat_004261a0(prefix, &t1, &dot);
        s = host.native_string_concat_int_004263b0(s, &t2, i);
        s = host.native_string_concat_004261a0(s, &t3, &colon);
        s = host.native_string_concat_004261a0(s, &t4, add(record, 0x04));
        s = host.native_string_concat_004261a0(s, &t5, &open);
        const void* const texture_name = host.texture_name_address_00b33e40(ptr(record, 0x00));
        s = host.native_string_concat_004261a0(s, &t6, texture_name);
        host.native_string_concat_004261a0(s, &path, &close);
        for (NativeStringRaw* temp : {&t6, &t5, &t4, &t3, &t2, &t1, &dot, &colon, &open, &close,
                                      &path}) {
            inline_destroy(host, *temp);
        }

        if (it >= at<std::uintptr_t>(frame_box, 0xF8)) host.invalid_iterator_00bf6713();
        it += kFrameBoxStateStride;
        ++i;
    }
    host.gui_widget_visit_child_paths_00aaa0f0(frame_box, prefix);
}

// ---------------------------------------------------------------------------------------------
// 00A96750-00A96C85, __thiscall(ECX = text owner, event dword, category dword), RET 8; SEH frame
// (handler 00CB6918). Only the low bytes of both arguments are tested (AL, [ESP+54h]). The
// owner's text is the NativeString at +18h (size +18h, data +1Ch), the cursor is +20h, the
// character-acceptance context is +24h. Every owner virtual is called with ECX = owner and no
// stack arguments. docs/TEXT_INPUT_FALLBACK.md describes the key table.
// Category 0: 0Dh -> vtable+8; 1Bh -> vtable+0Ch; 08h with signed cursor > 0 -> text =
//   text.substr(0, cursor-1) + text.substr(cursor, 7FFFFFFFh) (out-of-line 0041DD20 destroys),
//   cursor -= 1, vtable+4; otherwise when 00AB6D30(ECX = [owner+24h], event) accepts, the event's
//   low byte becomes 00A96450(ECX = owner, event) and text = text.substr(0, cursor) + byte +
//   text.substr(cursor + (byte [00E12F34] == 0), 7FFFFFFFh), cursor += 1, vtable+4.
// Other categories: '.' (2Eh) with cursor < size (unsigned) deletes at the cursor and calls
//   vtable+4; '-' toggles byte [00E12F34] (SETZ) then vtable+4; '&' vtable+10h; '(' vtable+14h
//   only when byte [00F8BC01] is 0; Tab vtable+18h; '!' vtable+20h; '"' vtable+1Ch; '%' signed
//   cursor > 0: cursor -= 1, vtable+4; '\'' cursor < size (unsigned): cursor += 1, vtable+4; '$'
//   signed cursor > 0: cursor = 0, vtable+4; '#' cursor < size (unsigned): cursor = size,
//   vtable+4.
// ---------------------------------------------------------------------------------------------
void text_owner_handle_key_fallback_00a96750(Large3Host& host, void* owner, std::uint32_t event,
                                             std::uint32_t category) {
    const std::uint8_t key = static_cast<std::uint8_t>(event);
    void* const text = add(owner, 0x18);
    std::uint32_t& cursor = at<std::uint32_t>(owner, 0x20);
    const auto signed_cursor = [&cursor] { return static_cast<std::int32_t>(cursor); };

    if (static_cast<std::uint8_t>(category) != 0) {
        switch (key) {
        case 0x2E:
            if (cursor < at<std::uint32_t>(owner, 0x18)) {
                NativeStringRaw right{}, left{}, joined{};
                NativeStringRaw* r = host.native_string_substring_00469840(text, &right, cursor + 1,
                                                                           0x7FFFFFFF);
                NativeStringRaw* l = host.native_string_substring_00469840(text, &left, 0, cursor);
                host.native_string_assign_00425f40(
                    text, host.native_string_concat_004261a0(l, &joined, r));
                inline_destroy(host, joined);
                inline_destroy(host, left);
                inline_destroy(host, right);
                host.text_owner_virtual(owner, 0x04);
            }
            return;
        case 0x2D: {
            std::uint8_t& insert = *host.global_byte(0x00E12F34);
            insert = insert == 0 ? 1 : 0;
            host.text_owner_virtual(owner, 0x04);
            return;
        }
        case 0x26: host.text_owner_virtual(owner, 0x10); return;
        case 0x28:
            if (*host.global_byte(0x00F8BC01) == 0) host.text_owner_virtual(owner, 0x14);
            return;
        case 0x09: host.text_owner_virtual(owner, 0x18); return;
        case 0x21: host.text_owner_virtual(owner, 0x20); return;
        case 0x22: host.text_owner_virtual(owner, 0x1C); return;
        case 0x25:
            if (signed_cursor() > 0) {
                cursor -= 1;
                host.text_owner_virtual(owner, 0x04);
            }
            return;
        case 0x27:
            if (cursor < at<std::uint32_t>(owner, 0x18)) {
                cursor += 1;
                host.text_owner_virtual(owner, 0x04);
            }
            return;
        case 0x24:
            if (signed_cursor() > 0) {
                cursor = 0;
                host.text_owner_virtual(owner, 0x04);
            }
            return;
        case 0x23: {
            const std::uint32_t size = at<std::uint32_t>(owner, 0x18);
            if (cursor < size) {
                cursor = size;
                host.text_owner_virtual(owner, 0x04);
            }
            return;
        }
        default: return;
        }
    }

    if (key == 0x0D) { host.text_owner_virtual(owner, 0x08); return; }
    if (key == 0x1B) { host.text_owner_virtual(owner, 0x0C); return; }
    if (key == 0x08) {
        if (signed_cursor() <= 0) return;
        NativeStringRaw right{}, left{}, joined{};
        NativeStringRaw* r = host.native_string_substring_00469840(text, &right, cursor, 0x7FFFFFFF);
        NativeStringRaw* l = host.native_string_substring_00469840(text, &left, 0, cursor - 1);
        host.native_string_assign_00425f40(text,
                                           host.native_string_concat_004261a0(l, &joined, r));
        host.native_string_destroy_0041dd20(&joined);
        host.native_string_destroy_0041dd20(&left);
        host.native_string_destroy_0041dd20(&right);
        cursor += 0xFFFFFFFFu;   // ADD [ESI+20h], EDI with EDI = -1
        host.text_owner_virtual(owner, 0x04);
        return;
    }
    if (!host.text_accepts_input_char_00ab6d30(ptr(owner, 0x24), event)) return;
    const std::uint8_t mapped = host.key_char_map_translate_00a96450(owner, event);
    event = (event & 0xFFFFFF00u) | mapped;   // MOV byte [ESP+64h], AL: the argument slot
    const std::uint32_t skip = *host.global_byte(0x00E12F34) == 0 ? 1u : 0u;
    NativeStringRaw tail{}, ch{}, head{}, head_ch{}, joined{};
    NativeStringRaw* t = host.native_string_substring_00469840(text, &tail, cursor + skip,
                                                               0x7FFFFFFF);
    NativeStringRaw* c = host.native_string_construct_byte_00531030(&ch, event);
    NativeStringRaw* h = host.native_string_substring_00469840(text, &head, 0, cursor);
    NativeStringRaw* hc = host.native_string_concat_004261a0(h, &head_ch, c);
    host.native_string_assign_00425f40(text, host.native_string_concat_004261a0(hc, &joined, t));
    inline_destroy(host, joined);
    inline_destroy(host, head_ch);
    inline_destroy(host, head);
    inline_destroy(host, ch);
    inline_destroy(host, tail);
    cursor += 1;
    host.text_owner_virtual(owner, 0x04);
}

// ---------------------------------------------------------------------------------------------
// 00A8F3B0-00A8F9F9, __thiscall(ECX = shadow owner, camera), RET 4. No SEH frame.
// 1. Eight times: a local 4x4 identity (diagonal 1.0f from 00D7A24C, rest 0) is rebuilt and
//    copied by 004134F0(ECX = owner+44h + 40h*k, &identity), k = 0..7.
// 2. Only when [00F8BBF0]+0Ch and +0Dh are both non-zero:
//    00B71990(ECX = [owner+20h+4k], [owner+10h+4k]) for k = 0..3; the ref pointer +30h takes the
//    camera (stored first, then InterlockedIncrement(camera+4) [00CE221C] when non-null, then the
//    old one released through [00CE2220] / vtable+0) unless it already holds it.
//    The light direction +38h..+40h: when [owner+0Ch]->vtable+0Ch(00B7AAE0(ECX = [owner+0Ch]))
//    answers true, normalize [owner+0Ch]+1E0h (00419510, ECX = out, EDX = in) into +38h; then with
//    t = 00412E20(0.4363... float 00D5B564) stored as float, y = 00415550(&+3Ch, &t) (max) when
//    +3Ch >= 0.0f (COMISS against 00D7A218; JC, so NaN takes the other side), else
//    y = 00415510(&+3Ch, &-t) (min, FCHS before the float store); normalize (+38h, y, +40h)
//    into +38h. Otherwise +38h..+40h = (0, 1.0f, 0).
//    The camera node [+30h] refreshes its world matrix (00B6DB70) unless node+5Ch bit 1 is set.
//    position = node+120h..+128h; eye = float(100000.0 (double 00CF81F0) * dir) + position
//    (x87: DC C9 FMUL ST1,ST0 at 00A8F939, then D8 C9 at 00A8F956; each product is exact in
//    double and rounded once to float, each sum rounded to float).
//    00B63F10(ECX = local matrix (the identity scratch), EDX = eye, &position, up (0, 1.0f, 0)),
//    then 00B63B30(ECX = a second local, EDX = that matrix); neither local is read afterwards.
//    00AD7A30(ECX = [00F8C210], camera, 1.0f (FLD1)), then 00A8EA20(ECX = owner), then the +30h
//    ref pointer is released and cleared (cleared only when it was non-null).
// ---------------------------------------------------------------------------------------------
void shadow_map_update_view_from_camera_00a8f3b0(Large3Host& host, void* owner, void* camera) {
    for (std::size_t k = 0; k < 8; ++k) {
        float identity[16] = {};
        identity[0] = identity[5] = identity[10] = identity[15] = kUnitScale;
        host.matrix_copy_4x4_004134f0(add(owner, 0x44 + 0x40 * k), identity);
    }
    void* const settings = host.global_pointer(0x00F8BBF0);
    if (at<std::uint8_t>(settings, 0x0C) == 0 || at<std::uint8_t>(settings, 0x0D) == 0) return;

    for (std::size_t k = 0; k < 4; ++k) {
        host.camera_set_viewport_00b71990(ptr(owner, 0x20 + 4 * k), ptr(owner, 0x10 + 4 * k));
    }
    void* const previous = ptr(owner, 0x30);
    if (previous != camera) {
        at<void*>(owner, 0x30) = camera;
        if (camera != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(camera, 0x04));
        if (previous != nullptr &&
            host.interlocked_decrement_00ce2220(&at<std::int32_t>(previous, 0x04)) == 0) {
            host.refcounted_destroy_vslot_00(previous);
        }
    }

    void* const context = ptr(owner, 0x0C);
    const std::uint32_t word = host.render_override_context_word_00b7aae0(context);
    if (host.shadow_context_vslot_0c(context, word)) {
        float scratch[3];
        const float* n = host.vector3_normalize_00419510(
            scratch, static_cast<const float*>(add(context, 0x1E0)));
        at<float>(owner, 0x38) = n[0];
        at<float>(owner, 0x3C) = n[1];
        at<float>(owner, 0x40) = n[2];
        const float* const dy = &at<float>(owner, 0x3C);
        float clamped[3];
        float t;
        if (*dy >= 0.0f) {   // COMISS + JC: below or unordered (NaN) goes to the min side
            t = static_cast<float>(host.tangent_00412e20(kShadowElevationAngle));
            clamped[0] = at<float>(owner, 0x38);
            clamped[1] = static_cast<float>(host.max_float_by_ref_00415550(dy, &t));
        } else {
            t = static_cast<float>(-host.tangent_00412e20(kShadowElevationAngle));
            clamped[0] = at<float>(owner, 0x38);
            clamped[1] = static_cast<float>(host.min_float_by_ref_00415510(dy, &t));
        }
        clamped[2] = at<float>(owner, 0x40);
        float normal[3];
        n = host.vector3_normalize_00419510(normal, clamped);
        at<float>(owner, 0x38) = n[0];
        at<float>(owner, 0x3C) = n[1];
        at<float>(owner, 0x40) = n[2];
    } else {
        at<float>(owner, 0x38) = 0.0f;
        at<float>(owner, 0x3C) = kUnitScale;
        at<float>(owner, 0x40) = 0.0f;
    }

    void* const node = ptr(owner, 0x30);
    if ((at<std::uint8_t>(node, 0x5C) & 0x02) == 0) host.transform_refresh_world_matrix_00b6db70(node);
    const float position[3] = {at<float>(node, 0x120), at<float>(node, 0x124),
                               at<float>(node, 0x128)};
    const float scaled[3] = {static_cast<float>(kShadowEyeDistance * at<float>(owner, 0x38)),
                             static_cast<float>(kShadowEyeDistance * at<float>(owner, 0x3C)),
                             static_cast<float>(kShadowEyeDistance * at<float>(owner, 0x40))};
    const float eye[3] = {scaled[0] + position[0], scaled[1] + position[1],
                          scaled[2] + position[2]};
    float view[16] = {};   // still the last identity built in step 1
    view[0] = view[5] = view[10] = view[15] = kUnitScale;
    alignas(4) std::uint8_t inverse[0x40];
    host.matrix_build_look_at_00b63f10(view, eye, position, 0.0f, kUnitScale, 0.0f);
    host.matrix_invert_affine_00b63b30(inverse, view);
    host.foliage_render_for_camera_00ad7a30(host.global_pointer(0x00F8C210), camera, 1.0f);
    host.shadow_owner_submit_cascades_00a8ea20(owner);
    void* const held = ptr(owner, 0x30);
    if (held != nullptr) {
        if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(held, 0x04)) == 0) {
            host.refcounted_destroy_vslot_00(held);
        }
        at<void*>(owner, 0x30) = nullptr;
    }
}
// ---------------------------------------------------------------------------------------------
// 0092AAE0-0092B342, __thiscall(ECX = tickable entity, const source*), RET 4; SEH frame (handler
// 00CA739B, state 0 covers the descriptor's shape-pointer array). Stack offsets below are
// relative to the entry ESP (S-108h is the body descriptor, S-54h the shape descriptor).
// Source record: +0 unit, +4 scene node, +8h point (3 floats), +14h float scale, +18h dword.
// x87: traced from the disk bytes (local/cc13-large3_x87sim.py; FDIVP/FSUBRP/FMULP forms decoded
// from the opcode bytes, never from the listing). These bodies run on the main thread with x87
// precision control at 24 bits (docs/CC13_FPU_PRECISION.md), so every x87 step rounds to float
// and float arithmetic in the same order models them; the double constants 2.0 (00D7A308), 0.5
// (00D7A280) and 5.0 (00D7A370) give the same float results.
//   1. settings = 00424C40(); 00923840(ECX = entity); point = 004142E0(ECX = source+8h, out,
//      00B6E0D0(ECX = [source+4])) -> entity+370h..+378h, entity+37Ch = 0; entity+34Ch =
//      [source+18h].
//   2. Body descriptor D (84h bytes, filled in the image's store order where it matters):
//      +0 byte 0; +4..+10h 1.0f; +14h..+40h a 3x3 identity + zero translation (overwritten by
//      00C336C0); +44h..+58h 0; +5Ch 0; +60h 0; +64h 1.0f; +68h, +6Ch 1000.0f (00CE3804);
//      +70h 0; +74h entity; +78h/+7Ch/+80h the shape-pointer array {begin, count, capacity}.
//      When byte entity+C8h is 0: 00414DB0(ECX = entity). 00C336C0(ECX = D+14h, entity+CCh).
//   3. Shape descriptor P (48h bytes at S-54h): +0, +4 0.0f; +8h, +0Ch 0; +10h 4; +14h 0;
//      +18h..+44h a 3x3 identity + zero translation.
//   4. The unit's mesh table ([unit+360h] ? [[unit+360h]+160h] : 0) holds a checked vector at
//      +4Ch of 8-byte {mesh, node} entries; the entry whose node is [source+4] gives
//      P+14h = mesh+0Ch and entity+364h..+36Ch = mesh+14h..+1Ch. 00BF6713 fires at the image's
//      checked-iterator tests (first > last at 0092AD5C and per pass at 0092AD91, it >= last
//      at 0092ADC7/0092ADE1/0092ADFB, a container change at 0092ADA2).
//   5. offset = entity+364h..+36Ch; p = R * offset + t with R, t from D+14h (x' = (R6*z +
//      (R3*y + R0*x)) + t0, y' = ((R1*x + R4*y) + R7*z) + t1, z' = ((z*R8) + ((y*R5) +
//      (x*R2))) + t2); d = p - t; len = sqrt((dx*dx + dy*dy) + dz*dz) (00BF7030, _CIsqrt);
//      v = (d / len) * 2.0; D+38h..+40h = p; D+64h = 1.0f;
//      w = (vy*0 - vz, vz*0 - 0*vx, vx - vy*0) (cross(v, up) with the zeros kept);
//      s = [settings+18h] * [source+14h]; D+50h..+58h = s*w;
//      D+44h..+4Ch = *[unit]->vtable[34h](ECX = unit, &v).
//      P+4 = 1.0f, P+8 = 1, P+0Ch = 0Dh; P is appended to D's pointer array (the inline
//      push_back: capacity*2+2 through 00BF55BE, element copy with the null-destination test,
//      00BF6989 on the old block).
//   6. body = 00C5D580(ECX = [[00E188A8]+18h], &D) -> entity+354h; 00C31F90(ECX = body, &lo,
//      &hi); entity+390h = [unit+C4h]; byte entity+394h = 0; entity+348h = [unit+538h];
//      entity+358h..+360h = hi - lo; m = [[unit+538h]+B0h] * 0.5; k = m / 5.0;
//      inertia = (k*(y2+z2), (z2+x2)*k, k*(x2+y2)) with x2 = ex*ex etc.;
//      00C37F40(ECX = body, m), 00C37E70(ECX = body, &inertia).
//   7. The collision publish (as install_tickable_entity_shape in collision_shapes.hpp):
//      node = entity+1C4h; node+D0h[node+F8h] = entity+1A4h; node+F8h += 1;
//      0098A920(ECX = node, &lo copy, &hi copy); 0098BA10(ECX = 0042E630(), node, 0, 0, 0);
//      byte entity+385h = 1; 00BF6989(D+78h begin) when non-null.
// ---------------------------------------------------------------------------------------------
void tickable_entity_install_collision_0092aae0(Large3Host& host, void* entity, void* source) {
    void* const settings = host.game_settings_singleton_00424c40();
    host.entity_inherit_race_and_activate_00923840(entity);
    {
        float out[3];
        const void* const matrix = host.node_inverse_world_matrix_00b6e0d0(ptr(source, 0x04));
        const float* const point = host.vector3_transform_affine_point_004142e0(
            static_cast<const float*>(add(source, 0x08)), out, matrix);
        at<float>(entity, 0x370) = point[0];
        at<float>(entity, 0x374) = point[1];
        at<float>(entity, 0x378) = point[2];
    }
    at<float>(entity, 0x37C) = 0.0f;
    at<std::uint32_t>(entity, 0x34C) = at<std::uint32_t>(source, 0x18);

    alignas(4) std::uint8_t desc[0x84] = {};
    at<std::uint8_t>(desc, 0x00) = 0;
    for (std::size_t o = 0x04; o <= 0x10; o += 4) at<float>(desc, o) = kUnitScale;
    for (std::size_t o = 0x14; o <= 0x40; o += 4) at<float>(desc, o) = 0.0f;
    at<float>(desc, 0x14) = at<float>(desc, 0x24) = at<float>(desc, 0x34) = kUnitScale;
    for (std::size_t o = 0x44; o <= 0x58; o += 4) at<float>(desc, o) = 0.0f;
    at<std::uint32_t>(desc, 0x5C) = 0;
    at<float>(desc, 0x60) = 0.0f;
    at<float>(desc, 0x64) = kUnitScale;
    at<float>(desc, 0x68) = kBodyDescDefault68;
    at<float>(desc, 0x6C) = kBodyDescDefault68;
    at<std::uint32_t>(desc, 0x70) = 0;
    at<void*>(desc, 0x74) = entity;
    at<void**>(desc, 0x78) = nullptr;
    at<std::uint32_t>(desc, 0x7C) = 0;
    at<std::uint32_t>(desc, 0x80) = 0;
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    host.physics_pose_from_matrix_00c336c0(add(desc, 0x14), add(entity, 0xCC));

    alignas(4) std::uint8_t shape[0x48] = {};
    at<std::uint32_t>(shape, 0x10) = 4;
    at<float>(shape, 0x18) = at<float>(shape, 0x28) = at<float>(shape, 0x38) = kUnitScale;

    void* const unit = ptr(source, 0x00);
    auto mesh_table = [unit]() -> std::uintptr_t {
        void* const holder = ptr(unit, 0x360);
        return holder == nullptr ? 0 : at<std::uintptr_t>(holder, 0x160);
    };
    {
        const std::uintptr_t container = mesh_table() + 0x4C;
        void* const c = reinterpret_cast<void*>(container);
        std::uintptr_t it = at<std::uintptr_t>(c, 0x04);
        if (it > at<std::uintptr_t>(c, 0x08)) host.invalid_iterator_00bf6713();
        for (;;) {
            const std::uintptr_t current = mesh_table() + 0x4C;
            void* const cc = reinterpret_cast<void*>(current);
            const std::uintptr_t last = at<std::uintptr_t>(cc, 0x08);
            if (at<std::uintptr_t>(cc, 0x04) > last) host.invalid_iterator_00bf6713();
            if (container == 0 || container != current) host.invalid_iterator_00bf6713();
            if (it == last) break;
            if (container == 0) host.invalid_iterator_00bf6713();
            if (it >= at<std::uintptr_t>(c, 0x08)) host.invalid_iterator_00bf6713();
            void* const entry = reinterpret_cast<void*>(it);
            if (at<std::uint32_t>(entry, 0x04) == at<std::uint32_t>(source, 0x04)) {
                if (it >= at<std::uintptr_t>(c, 0x08)) host.invalid_iterator_00bf6713();
                void* const mesh = ptr(entry, 0x00);
                at<void*>(shape, 0x14) = add(mesh, 0x0C);
                at<float>(entity, 0x364) = at<float>(mesh, 0x14);
                at<float>(entity, 0x368) = at<float>(mesh, 0x18);
                at<float>(entity, 0x36C) = at<float>(mesh, 0x1C);
                break;
            }
            if (it >= at<std::uintptr_t>(c, 0x08)) host.invalid_iterator_00bf6713();
            it += 8;
        }
    }

    const float x = at<float>(entity, 0x364);
    const float y = at<float>(entity, 0x368);
    const float z = at<float>(entity, 0x36C);
    const float* const r = &at<float>(desc, 0x14);   // R0..R8, then t0..t2 at r[9..11]
    const float px = (r[6] * z + (r[3] * y + r[0] * x)) + r[9];
    const float py = ((r[1] * x + r[4] * y) + r[7] * z) + r[10];
    const float pz = ((z * r[8]) + ((y * r[5]) + (x * r[2]))) + r[11];
    float v[3] = {px - r[9], py - r[10], pz - r[11]};
    const float length = std::sqrt((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
    v[0] = v[0] / length;
    v[1] = v[1] / length;
    v[2] = v[2] / length;
    v[0] = static_cast<float>(v[0] * kVelocityScale);
    v[1] = static_cast<float>(v[1] * kVelocityScale);
    v[2] = static_cast<float>(kVelocityScale * v[2]);
    const float w[3] = {(v[1] * 0.0f) - v[2], (v[2] * 0.0f) - (0.0f * v[0]), v[0] - (v[1] * 0.0f)};
    const float s = at<float>(settings, 0x18) * at<float>(source, 0x14);
    at<float>(desc, 0x38) = px;
    at<float>(desc, 0x3C) = py;
    at<float>(desc, 0x40) = pz;
    at<float>(desc, 0x64) = kUnitScale;
    at<float>(desc, 0x50) = s * w[0];
    at<float>(desc, 0x54) = w[1] * s;
    at<float>(desc, 0x58) = s * w[2];
    {
        const float* const linear = host.unit_vslot_34(unit, v);
        at<float>(desc, 0x44) = linear[0];
        at<float>(desc, 0x48) = linear[1];
        at<float>(desc, 0x4C) = linear[2];
    }
    at<float>(shape, 0x04) = kUnitScale;
    at<std::uint32_t>(shape, 0x08) = 1;
    at<std::uint32_t>(shape, 0x0C) = 0x0D;

    // The inline push_back of &shape onto D+78h {begin, count, capacity}.
    if (at<std::uint32_t>(desc, 0x7C) == at<std::uint32_t>(desc, 0x80)) {
        const std::uint32_t capacity = at<std::uint32_t>(desc, 0x80) * 2 + 2;
        at<std::uint32_t>(desc, 0x80) = capacity;
        void** const grown = static_cast<void**>(host.operator_new_00bf55be(capacity * 4));
        void** const old = at<void**>(desc, 0x78);
        void** dst = grown;
        for (std::uint32_t i = 0; i < at<std::uint32_t>(desc, 0x7C); ++i, ++dst) {
            if (dst != nullptr) *dst = old[i];
        }
        if (old != nullptr) host.free_00bf6989(old);
        at<void**>(desc, 0x78) = grown;
    }
    {
        void** const slot = at<void**>(desc, 0x78) + at<std::uint32_t>(desc, 0x7C);
        if (slot != nullptr) *slot = shape;
        at<std::uint32_t>(desc, 0x7C) += 1;
    }

    void* const world = ptr(host.global_pointer(0x00E188A8), 0x18);
    void* const body = host.physics_world_create_body_00c5d580(world, desc);
    at<void*>(entity, 0x354) = body;
    float lo[3], hi[3];
    host.physics_body_bounds_00c31f90(body, lo, hi);
    at<std::uint32_t>(entity, 0x390) = at<std::uint32_t>(unit, 0xC4);
    at<std::uint8_t>(entity, 0x394) = 0;
    at<std::uint32_t>(entity, 0x348) = at<std::uint32_t>(unit, 0x538);
    at<float>(entity, 0x358) = hi[0] - lo[0];
    at<float>(entity, 0x35C) = hi[1] - lo[1];
    at<float>(entity, 0x360) = hi[2] - lo[2];
    const float mass =
        static_cast<float>(at<float>(ptr(unit, 0x538), 0xB0) * kMassScale);
    const float ex2 = at<float>(entity, 0x358) * at<float>(entity, 0x358);
    const float ey2 = at<float>(entity, 0x35C) * at<float>(entity, 0x35C);
    const float ez2 = at<float>(entity, 0x360) * at<float>(entity, 0x360);
    const float k = static_cast<float>(mass / kInertiaDivisor);
    const float inertia[3] = {k * (ey2 + ez2), (ez2 + ex2) * k, k * (ex2 + ey2)};
    host.physics_body_set_mass_00c37f40(body, mass);
    host.physics_body_set_inertia_00c37e70(body, inertia);

    const float lo_copy[3] = {lo[0], lo[1], lo[2]};
    const float hi_copy[3] = {hi[0], hi[1], hi[2]};
    void* const node = add(entity, 0x1C4);
    const std::uint32_t count = at<std::uint32_t>(entity, 0x2BC);   // node+F8h
    at<void*>(node, 0xD0 + 4 * count) = add(entity, 0x1A4);
    at<std::uint32_t>(node, 0xF8) += 1;
    host.spatial_node_set_local_bounds_0098a920(node, lo_copy, hi_copy);
    void* const index = host.spatial_index_singleton_0042e630();
    host.spatial_index_attach_node_0098ba10(index, node, 0, 0, 0);
    at<std::uint8_t>(entity, 0x385) = 1;
    if (at<void**>(desc, 0x78) != nullptr) host.free_00bf6989(at<void**>(desc, 0x78));
}
// ---------------------------------------------------------------------------------------------
// 008EC650-008ECEB9, __thiscall(ECX = powerup manager, NativeString* out, NativeString key by
// value {size, data}), RET 0Ch, EAX = out; SEH frame (handler 00CA43C1). The by-value key is
// destroyed here (pooled release at both returns). Stack: s at S-54h, result at S-4Ch, the
// local vector<NativeString> {proxy, first, last, end} at S-1Ch (the proxy dword is never
// written by the image; zero here).
//   first = key.substr(0, 1) data[0] (00F88C34 "" when null). s = key (0041DD40 + memcpy), its
//   first character erased (memmove 00BF87E0 + 0041DD40), trimmed with the set "[]" (00D16234)
//   by 00585EB0 / 00584110, then "," (00CE4BFC) appended. The local vector is cleared through
//   004954F0 with checked iterators {vector, first} / {vector, last} (00BF6713 when first > last,
//   tested twice).
//   map = manager+1ACh: when 008E65E0(map, key) is the end node (map+4h read before the call):
//     for each comma-separated token of s (strcspn 00BF9A50 from pos; npos = -1): a "*" token
//     (_stricmp 00BF7FBF on s.substr(pos, 1)) gets 00531030(first) inserted at pos (008E64A0)
//     and end += 1; when pos <= end (signed) and end != -1, the token s.substr(pos, end-pos) is
//     looked up in manager+1B8h (008E65E0); a miss pushes the token (00450540), a hit pushes
//     every element of the node's vector (node+18h first, node+1Ch last, 8-byte strings) with
//     the image's checked tests; pos = end + 1 while pos < s.size (unsigned).
//     Then for first == 'A' one element list[00BD2FC0(ECX = 0) % size] is pushed onto
//     008EC460(map, key)'s vector, otherwise that vector is assigned the list (00506C80).
//   Lookup (008ECB18): node = 008E5BB0(map, key) with the inline find (end when node is the head
//   or key < node key: an empty key compares -1 against a non-empty node key, a non-empty key
//   against an empty node key counts as found, otherwise _stricmp(key, node+10h)).
//   Found and first == 'A': out = copy of the vector's first element (00BF6713 when empty);
//   return. Found otherwise and the vector is non-empty: idx = 00BD2FC0(ECX = 0) % size,
//   result = vector[idx]; for first == 'T' that element is erased (00589690 with the checked
//   iterator {vector, first + idx*8}). Then out = copy of result.
//   Each 008EC460 call is repeated where the image repeats it (up to five per path).
// RNG: 00BD2FC0 with ECX = 0 selects random-thread stream 0 through 00BD2ED0, the state set
// that 00BD2F10 also draws from; one draw for an 'A' list build and one per non-'A' pick. A
// count of zero divides by zero (DIV) in the image; nothing guards it here either.
// ---------------------------------------------------------------------------------------------
NativeStringRaw* powerup_pick_from_bracket_list_008ec650(Large3Host& host, void* manager,
                                                         NativeStringRaw* out,
                                                         NativeStringRaw* key) {
    constexpr std::uint32_t kNpos = 0xFFFFFFFFu;
    NativeStringRaw result{0, nullptr};
    alignas(4) std::uint8_t list[0x10] = {};
    auto list_first = [&list]() { return at<std::uintptr_t>(list, 0x04); };
    auto list_last = [&list]() { return at<std::uintptr_t>(list, 0x08); };

    std::uint8_t first_char;
    {
        NativeStringRaw head{};
        const NativeStringRaw* const h = host.native_string_substring_00469840(key, &head, 0, 1);
        first_char = static_cast<std::uint8_t>(*(h->data != nullptr ? h->data : kEmptyF88C34));
        inline_destroy(host, head);
    }

    NativeStringRaw s{0, nullptr};
    const std::uint32_t key_size = key->size;
    host.native_string_resize_0041dd40(&s, key_size, 1);
    if (key_size != 0) std::memcpy(s.data, key->data, s.size);
    if (s.size != 0) {
        const std::uint32_t n = s.size < 1 ? s.size : 1;   // CMOVC
        const std::uint32_t rest = s.size - n;
        std::memmove(s.data, s.data + n, rest);
        host.native_string_resize_0041dd40(&s, rest, 1);
    }
    host.native_string_trim_left_set_00585eb0(&s, "[]");
    host.native_string_trim_right_set_00584110(&s, "[]");
    {
        NativeStringRaw comma;
        construct_literal(host, comma, ",");
        if (comma.size != 0) {
            const std::uint32_t old = s.size;
            host.native_string_resize_0041dd40(&s, comma.size + old, 1);
            std::memcpy(s.data + old, comma.data, comma.size);
        }
        inline_destroy(host, comma);
    }
    {
        const std::uintptr_t first = list_first();
        const std::uintptr_t last = list_last();
        if (first > last) host.invalid_iterator_00bf6713();
        if (list_first() > list_last()) host.invalid_iterator_00bf6713();
        alignas(4) std::uint8_t erased[8];
        host.string_vector_erase_range_004954f0(list, erased, list, reinterpret_cast<void*>(first),
                                                list, reinterpret_cast<void*>(last));
    }

    void* const map = add(manager, 0x1AC);
    {
        const std::uint32_t head = at<std::uint32_t>(manager, 0x1B0);
        alignas(4) std::uint8_t found[8];
        const std::uint32_t* const it = host.string_tree_find_008e65e0(map, found, key);
        if (it[0] == 0 || it[0] != to_u32(map)) host.invalid_iterator_00bf6713();
        if (it[1] == head) {
            std::uint32_t pos = 0;
            if (s.size != 0) {
                do {
                    std::uint32_t end = kNpos;
                    if (s.data != nullptr && s.size != 0) {
                        bool in_range = true;
                        std::uint32_t start = pos;
                        if (static_cast<std::int32_t>(pos) < 0) start = 0;
                        else if (pos > s.size) in_range = false;
                        if (in_range) {
                            const std::uint32_t index =
                                static_cast<std::uint32_t>(std::strcspn(s.data + start, ",")) + start;
                            if (index != s.size) end = index;
                        }
                    }
                    bool star;
                    {
                        NativeStringRaw one{};
                        const NativeStringRaw* const o =
                            host.native_string_substring_00469840(&s, &one, pos, 1);
                        star = o->data != nullptr && _stricmp(o->data, "*") == 0;
                        inline_destroy(host, one);
                    }
                    if (star) {
                        NativeStringRaw ch{};
                        const NativeStringRaw* const c =
                            host.native_string_construct_byte_00531030(&ch, first_char);
                        host.native_string_insert_at_008e64a0(&s, c, pos);
                        inline_destroy(host, ch);
                        end += 1;
                    }
                    if (static_cast<std::int32_t>(pos) <= static_cast<std::int32_t>(end) &&
                        end != kNpos) {
                        void* const names = add(manager, 0x1B8);
                        alignas(4) std::uint8_t hit[8];
                        {
                            NativeStringRaw token{};
                            const NativeStringRaw* const t =
                                host.native_string_substring_00469840(&s, &token, pos, end - pos);
                            host.string_tree_find_008e65e0(names, hit, t);
                            inline_destroy(host, token);
                        }
                        const std::uint32_t names_head = at<std::uint32_t>(names, 0x04);
                        const std::uint32_t container = at<std::uint32_t>(hit, 0x00);
                        if (container == 0 || container != to_u32(names)) {
                            host.invalid_iterator_00bf6713();
                        }
                        void* const node = ptr(hit, 0x04);
                        if (to_u32(node) == names_head) {
                            NativeStringRaw piece{};
                            const NativeStringRaw* const pc =
                                host.native_string_substring_00469840(&s, &piece, pos, end - pos);
                            host.string_vector_push_back_00450540(list, pc);
                            inline_destroy(host, piece);
                        } else {
                            void* const tree = reinterpret_cast<void*>(
                                static_cast<std::uintptr_t>(at<std::uint32_t>(hit, 0x00)));
                            for (std::uint32_t i = 0;; ++i) {
                                if (at<std::uint32_t>(hit, 0x00) == 0) host.invalid_iterator_00bf6713();
                                if (node == ptr(tree, 0x04)) host.invalid_iterator_00bf6713();
                                const std::uintptr_t vfirst = at<std::uintptr_t>(node, 0x18);
                                if (vfirst == 0 ||
                                    i >= ((at<std::uintptr_t>(node, 0x1C) - vfirst) >> 3)) {
                                    break;
                                }
                                if (node == ptr(tree, 0x04)) host.invalid_iterator_00bf6713();
                                const std::uintptr_t again = at<std::uintptr_t>(node, 0x18);
                                if (again == 0 ||
                                    i >= ((at<std::uintptr_t>(node, 0x1C) - again) >> 3)) {
                                    host.invalid_iterator_00bf6713();
                                }
                                host.string_vector_push_back_00450540(
                                    list, reinterpret_cast<void*>(at<std::uintptr_t>(node, 0x18) + 8 * i));
                            }
                        }
                    }
                    pos = end + 1;
                } while (pos < s.size);
            }
            if (first_char == 'A') {
                const std::uint32_t count = list_first() == 0
                    ? 0u : static_cast<std::uint32_t>((list_last() - list_first()) >> 3);
                const std::uint32_t index = host.random_threads_next_u32_00bd2fc0(0) % count;
                if (list_first() == 0 ||
                    index >= static_cast<std::uint32_t>((list_last() - list_first()) >> 3)) {
                    host.invalid_iterator_00bf6713();
                }
                const void* const element = reinterpret_cast<const void*>(list_first() + 8 * index);
                void* const vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
                host.string_vector_push_back_00450540(vector, element);
            } else {
                void* const vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
                host.string_vector_assign_00506c80(vector, list);
            }
        }
    }

    // 008ECB18: the lookup.
    auto destroy_list = [&]() {
        if (list_first() != 0) {
            host.string_range_destroy_00432050(reinterpret_cast<void*>(list_first()),
                                               reinterpret_cast<void*>(list_last()), list, out);
            host.free_00bf65ac(reinterpret_cast<void*>(list_first()));
        }
        at<std::uintptr_t>(list, 0x04) = 0;
        at<std::uintptr_t>(list, 0x08) = 0;
        at<std::uintptr_t>(list, 0x0C) = 0;
    };
    auto copy_into_out = [&](const NativeStringRaw* source, std::uint32_t size) {
        out->size = 0;
        out->data = nullptr;
        if (out != source) {
            host.native_string_resize_0041dd40(out, size, 1);
            if (size != 0) std::memcpy(out->data, source->data, out->size);
        }
    };
    const std::uint32_t head = at<std::uint32_t>(map, 0x04);
    void* const node = host.name_vector_map_lower_bound_008e5bb0(map, key);
    if (map == nullptr) host.invalid_iterator_00bf6713();
    void* found = node;
    if (to_u32(node) == at<std::uint32_t>(map, 0x04)) {
        found = ptr(map, 0x04);
    } else if (!(key->size != 0 && at<std::uint32_t>(node, 0x0C) == 0)) {
        const int compare = key->size == 0
            ? (at<std::uint32_t>(node, 0x0C) != 0 ? -1 : 0)
            : _stricmp(key->data, at<const char*>(node, 0x10));
        if (compare < 0) found = ptr(map, 0x04);
    }
    // The iterator's container is the map on both sides, so the 00BF6713 test at 008ECB93
    // (container null or not the map) cannot fire.
    if (to_u32(found) != head) {
        if (first_char == 'A') {
            void* const vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
            if (at<std::uintptr_t>(vector, 0x04) == 0 ||
                ((at<std::uintptr_t>(vector, 0x08) - at<std::uintptr_t>(vector, 0x04)) >> 3) == 0) {
                host.invalid_iterator_00bf6713();
            }
            const NativeStringRaw* const source = at<const NativeStringRaw*>(vector, 0x04);
            out->size = 0;
            out->data = nullptr;
            if (out != source) {
                host.native_string_resize_0041dd40(out, source->size, 1);
                if (source->size != 0) std::memcpy(out->data, source->data, out->size);
            }
            inline_destroy(host, s);
            destroy_list();
            inline_destroy(host, *key);
            return out;
        }
        void* vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
        if (at<std::uintptr_t>(vector, 0x04) != 0 &&
            ((at<std::uintptr_t>(vector, 0x08) - at<std::uintptr_t>(vector, 0x04)) >> 3) != 0) {
            vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
            const std::uint32_t count = at<std::uintptr_t>(vector, 0x04) == 0
                ? 0u
                : static_cast<std::uint32_t>(
                      (at<std::uintptr_t>(vector, 0x08) - at<std::uintptr_t>(vector, 0x04)) >> 3);
            const std::uint32_t index = host.random_threads_next_u32_00bd2fc0(0) % count;
            vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
            if (at<std::uintptr_t>(vector, 0x04) == 0 ||
                index >= ((at<std::uintptr_t>(vector, 0x08) - at<std::uintptr_t>(vector, 0x04)) >> 3)) {
                host.invalid_iterator_00bf6713();
            }
            const NativeStringRaw* const element = reinterpret_cast<const NativeStringRaw*>(
                at<std::uintptr_t>(vector, 0x04) + 8 * index);
            if (&result != element) {
                host.native_string_resize_0041dd40(&result, element->size, 1);
                if (element->size != 0) std::memcpy(result.data, element->data, result.size);
            }
            if (first_char == 'T') {
                vector = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
                const std::uintptr_t first = at<std::uintptr_t>(vector, 0x04);
                if (first > at<std::uintptr_t>(vector, 0x08)) host.invalid_iterator_00bf6713();
                const std::uintptr_t target = first + 8 * index;
                if (target > at<std::uintptr_t>(vector, 0x08) ||
                    target < at<std::uintptr_t>(vector, 0x04)) {
                    host.invalid_iterator_00bf6713();
                }
                void* const owner = vector;
                void* const erase_in = host.powerup_random_name_map_lookup_or_insert_008ec460(map, key);
                alignas(4) std::uint8_t next[8];
                host.string_vector_erase_00589690(erase_in, next, owner,
                                                  reinterpret_cast<void*>(target));
            }
        }
    }

    // 008ECDBD: out = result; then the cleanup.
    const std::uint32_t result_size = result.size;
    copy_into_out(&result, result_size);
    inline_destroy(host, s);
    destroy_list();
    if (result.data != nullptr) {
        host.string_pool_release_00419cc0_00bd1510(result.data, result_size + 1);
    }
    inline_destroy(host, *key);
    return out;
}
}  // namespace bsp::cc13_large3