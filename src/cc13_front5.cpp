// Packet cc13-front5 (marker cc13_front5): wiring-frontier callees. Evidence,
// ABI and uncertainty per function: docs/CC13_FRONT5.md. Nothing here is wired.
#include "bsp/cc13_front5.hpp"

#include <cstring>

namespace bsp::cc13_front5 {
namespace {

// The inlined ~NativeString: 00419CC0 returns the pool, 00BD1510(ECX pool;
// block, length + 1, 1) RET 0Ch takes the block back. No call for a null block.
void release(const NativeStr& s, ArchiveEmitHost& host) {
    if (s.data != nullptr) {
        void* pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

// The inlined `out = source` the image emits after each concatenation:
// skipped when the two are the same object, then 0041DD40(&out, length, 1) and
// a 00BF7680 memcpy of out.length bytes when the source is not empty.
void assign_from(NativeStr* out, const NativeStr* source, ArchiveEmitHost& host) {
    if (out != source) {
        host.native_string_resize_0041dd40(out, source->length, 1);
        if (source->length != 0) std::memcpy(out->data, source->data, out->length);
    }
}

// The shared tail of every emitter: compact mode copies the string into the
// by-value argument of 00BD6100; otherwise fwrite to 0109CEE0 when a FILE is
// set, else 00BD49B0(ECX = [0109CECC]; &text).
void emit_text(NativeStr* text, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    if (g.compact_0109cee4 != 0) {
        NativeStr copy{0, nullptr};
        assign_from(&copy, text, host);  // the copy is a fresh outgoing-argument slot
        archive_emit_compact_encoded_00bd6100(copy, g, host);
    } else if (g.file_0109cee0 != nullptr) {
        const char* data = text->data != nullptr ? text->data : g.empty_0109cee5;
        host.fwrite_00bfb08b(data, text->length, 1, g.file_0109cee0);
    } else {
        host.save_storage_append_00bd49b0(g.storage_0109cecc, text);
    }
}

constexpr const char* kQuote_00ce436c = "\"";
constexpr const char* kOpenBracket_00cf5980 = "[";
constexpr const char* kRootSeparator_00cf8284 = " = ";
constexpr const char* kIndexSeparator_00d68364 = "] = ";
constexpr const char* kRootTerminator_00ce4390 = "\n";
constexpr const char* kNestedTerminator_00ce438c = ",\n";

}  // namespace

// ---------------------------------------------------------------------------
// 00BD6100: per byte i < length (the data pointer, or 0109CEE5 when null, is
// re-tested before every read): skip ' ', '\r', '\n', '\t'; otherwise write
// (char >> 4 arithmetic) | (char << 4) as one byte, fwrite(&b, 1, 1, [0109CEE0]).
// The byte is staged in the caller's length slot (ESP+18h after two pushes).
// ---------------------------------------------------------------------------
void archive_emit_compact_encoded_00bd6100(NativeStr owned, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    const std::uint32_t length = owned.length;
    for (std::uint32_t i = 0; i < length; ++i) {
        const char* data = owned.data != nullptr ? owned.data : g.empty_0109cee5;
        const char c = data[i];
        if (c == ' ' || c == '\r' || c == '\n' || c == '\t') continue;
        const std::uint8_t b = static_cast<std::uint8_t>(
            (static_cast<std::int8_t>(c) >> 4) | static_cast<std::uint8_t>(static_cast<std::uint8_t>(c) << 4));
        host.fwrite_00bfb08b(&b, 1, 1, g.file_0109cee0);
    }
    release(owned, host);
}

// ---------------------------------------------------------------------------
// 00BD6990: s = {0, 0}; 0041DD40(&s, depth, 1); memset(data, ' ', length) when
// length > 0; memset(data, '\t', length) when data != 0 (00BF79F0 both times,
// so the spaces are always overwritten); emit; release.
// ---------------------------------------------------------------------------
void archive_emit_indent_00bd6990(std::uint32_t depth, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    NativeStr s{0, nullptr};
    host.native_string_resize_0041dd40(&s, depth, 1);
    if (s.length > 0) std::memset(s.data, 0x20, s.length);
    if (s.data != nullptr) std::memset(s.data, 0x09, s.length);
    emit_text(&s, g, host);
    release(s, host);
}

// ---------------------------------------------------------------------------
// 00BD6AC0: 0041E870 constructs the local from the literal (the slot is not
// zeroed first; 0041E870 acts as the constructor here, as in 00BD8D10).
// ---------------------------------------------------------------------------
void archive_emit_terminator_00bd6ac0(bool root, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    NativeStr s;
    host.native_string_assign_0041e870(&s, root ? kRootTerminator_00ce4390 : kNestedTerminator_00ce438c);
    emit_text(&s, g, host);
    release(s, host);
}

// ---------------------------------------------------------------------------
// 00BD6BD0: emit the owned argument, then release it (the callee owns it).
// ---------------------------------------------------------------------------
void archive_emit_string_00bd6bd0(NativeStr owned, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    emit_text(&owned, g, host);
    release(owned, host);
}

// ---------------------------------------------------------------------------
// 00BD7B30. s = {0, 0} at ESP+18h.
//   root (CL != 0): sep = " = " (00CF8284), name = payload as char*;
//     joined = name + sep; s = joined; release joined, name, sep.
//   nested: 0041DD40(&s, 1, 0); memcpy(s.data, "[", s.length) when data != 0;
//     tag 0: q1 = "\"", name = payload, q0 = "\""; s = ((s + q0) + name) + q1,
//            releasing the three concatenations then q0, name, q1;
//     tag 1: 004CACA0(&s, payload); tag 2: 00781740(&s, payload as float);
//     any other tag appends nothing;
//     then t = "] = " (00D68364); when t.length != 0 resize s to old + t.length
//     (preserve 1) and memcpy at old; release t.
//   emit s; release s.
// ---------------------------------------------------------------------------
void archive_emit_key_00bd7b30(bool root, TaggedValue key, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    NativeStr s{0, nullptr};
    const char* payload_text = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(key.payload));
    if (root) {
        NativeStr sep;
        host.native_string_assign_0041e870(&sep, kRootSeparator_00cf8284);
        NativeStr name;
        host.native_string_assign_0041e870(&name, payload_text);
        NativeStr joined;
        const NativeStr* r = host.native_string_concat_004261a0(&name, &joined, &sep);
        assign_from(&s, r, host);
        release(joined, host);
        release(name, host);
        release(sep, host);
    } else {
        host.native_string_resize_0041dd40(&s, 1, 0);
        if (s.data != nullptr) std::memcpy(s.data, kOpenBracket_00cf5980, s.length);
        if (key.tag == 0) {
            NativeStr q1;
            host.native_string_assign_0041e870(&q1, kQuote_00ce436c);
            NativeStr name;
            host.native_string_assign_0041e870(&name, payload_text);
            NativeStr q0;
            host.native_string_assign_0041e870(&q0, kQuote_00ce436c);
            NativeStr out1;
            NativeStr out2;
            NativeStr out3;
            const NativeStr* r = host.native_string_concat_004261a0(&s, &out1, &q0);
            r = host.native_string_concat_004261a0(r, &out2, &name);
            r = host.native_string_concat_004261a0(r, &out3, &q1);
            assign_from(&s, r, host);
            release(out3, host);
            release(out2, host);
            release(out1, host);
            release(q0, host);
            release(name, host);
            release(q1, host);
        } else if (key.tag == 1) {
            host.native_string_append_int_004caca0(&s, static_cast<std::int32_t>(key.payload));
        } else if (key.tag == 2) {
            // FLD float [EBP+0Ch]; FSTP float [ESP]: an exact copy except that
            // a signalling NaN is quietened, which the float argument also does.
            float value;
            std::memcpy(&value, &key.payload, sizeof value);
            host.native_string_append_float_00781740(&s, value);
        }
        NativeStr t;
        host.native_string_assign_0041e870(&t, kIndexSeparator_00d68364);
        const std::uint32_t t_length = t.length;
        char* const t_data = t.data;
        if (t_length != 0) {
            const std::uint32_t old = s.length;
            host.native_string_resize_0041dd40(&s, t_length + old, 1);
            std::memcpy(s.data + old, t_data, t_length);
        }
        if (t_data != nullptr) {
            void* pool = host.sized_pool_00419cc0();
            host.sized_pool_return_00bd1510(pool, t_data, t_length + 1, 1);
        }
    }
    emit_text(&s, g, host);
    release(s, host);
}

// ---------------------------------------------------------------------------
// 00BD7F50. s = {0, 0} at ESP+10h. Literal addresses: "\"" 00CE436C, "\\"
// 00CE7894, "\\\\" 00D6837C, "\\\"" 00D68378, "true" 00CE4378, "false" 00CE4370,
// "{ " 00D68370, " }" 00D68374, ", " 00CE42FC, "}, " 00D6836C, "}" 00CE4CD4.
// Temporaries are named by their frame slot (ESP+n) so the construction and
// destruction order can be checked against the listing; the image releases some
// inline (release()) and some through 0041DD20, and both are kept as they are.
// ---------------------------------------------------------------------------
void archive_emit_value_00bd7f50(TaggedValue value, const ArchiveWriterGlobals& g, ArchiveEmitHost& host) {
    NativeStr s{0, nullptr};
    auto float_at = [&](std::uint32_t index) {
        return reinterpret_cast<const float*>(static_cast<std::uintptr_t>(value.payload))[index];
    };
    auto destroy = [&](NativeStr& t) { host.native_string_destroy_0041dd20(&t); };
    if (value.tag == 0) {
        NativeStr text_38;
        host.native_string_assign_0041e870(&text_38, reinterpret_cast<const char*>(static_cast<std::uintptr_t>(value.payload)));
        {
            NativeStr t18;
            host.native_string_assign_0041e870(&t18, "\\\\");  // 00D6837C
            NativeStr t28;
            host.native_string_assign_0041e870(&t28, "\\");  // 00CE7894
            host.string_replace_substrings_004cad40(&text_38, &t28, &t18, 0x7FFFFFFFu);
            release(t28, host);
            release(t18, host);
        }
        {
            NativeStr t28;
            host.native_string_assign_0041e870(&t28, "\\\"");  // 00D68378
            NativeStr t18;
            host.native_string_assign_0041e870(&t18, kQuote_00ce436c);
            host.string_replace_substrings_004cad40(&text_38, &t18, &t28, 0x7FFFFFFFu);
            release(t18, host);
            release(t28, host);
        }
        NativeStr q28;
        host.native_string_assign_0041e870(&q28, kQuote_00ce436c);
        NativeStr q18;
        host.native_string_assign_0041e870(&q18, kQuote_00ce436c);
        NativeStr o20;
        NativeStr o30;
        NativeStr o40;
        const NativeStr* r = host.native_string_concat_004261a0(&s, &o20, &q18);
        r = host.native_string_concat_004261a0(r, &o30, &text_38);
        r = host.native_string_concat_004261a0(r, &o40, &q28);
        assign_from(&s, r, host);
        release(o40, host);
        release(o30, host);
        release(o20, host);
        release(q18, host);
        release(q28, host);
        release(text_38, host);
    } else if (value.tag == 1) {
        host.native_string_append_int_004caca0(&s, static_cast<std::int32_t>(value.payload));
    } else if (value.tag == 2) {
        float f;
        std::memcpy(&f, &value.payload, sizeof f);
        host.native_string_append_float_00781740(&s, f);
    } else if (value.tag == 3) {
        NativeStr t18;  // CMP byte [EBP+0Ch]: only the low byte is tested
        host.native_string_assign_0041e870(&t18, static_cast<std::uint8_t>(value.payload) != 0 ? "true" : "false");
        host.native_string_append_00425e10(&s, &t18);
        release(t18, host);
    } else if (value.tag == 4) {
        const std::int32_t id = host.object_id_fn_0109cedc(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.payload)));
        host.native_string_append_int_004caca0(&s, id);
    } else if (value.tag == 5) {
        NativeStr c18, c40, c30, b20;
        host.native_string_assign_0041e870(&c18, " }");
        host.native_string_assign_0041e870(&c40, ", ");
        host.native_string_assign_0041e870(&c30, ", ");
        host.native_string_assign_0041e870(&b20, "{ ");
        NativeStr o50, o58, o60, o68, o48, o38, o28;
        const NativeStr* r = host.native_string_concat_004261a0(&s, &o50, &b20);
        r = host.native_string_concat_float_006af2b0(r, &o58, float_at(0));
        r = host.native_string_concat_004261a0(r, &o60, &c30);
        r = host.native_string_concat_float_006af2b0(r, &o68, float_at(1));
        r = host.native_string_concat_004261a0(r, &o48, &c40);
        r = host.native_string_concat_float_006af2b0(r, &o38, float_at(2));
        r = host.native_string_concat_004261a0(r, &o28, &c18);
        host.native_string_assign_header_00425f40(&s, r);
        destroy(o28);
        destroy(o38);
        destroy(o48);
        destroy(o68);
        destroy(o60);
        destroy(o58);
        destroy(o50);
        destroy(b20);
        destroy(c30);
        destroy(c40);
        destroy(c18);
    } else if (value.tag == 6) {
        NativeStr c40, c30, b20;
        host.native_string_assign_0041e870(&c40, " }");
        host.native_string_assign_0041e870(&c30, ", ");
        host.native_string_assign_0041e870(&b20, "{ ");
        NativeStr o48, o68, o60, o58, o50;
        const NativeStr* r = host.native_string_concat_004261a0(&s, &o48, &b20);
        r = host.native_string_concat_float_006af2b0(r, &o68, float_at(0));
        r = host.native_string_concat_004261a0(r, &o60, &c30);
        r = host.native_string_concat_float_006af2b0(r, &o58, float_at(1));
        r = host.native_string_concat_004261a0(r, &o50, &c40);
        host.native_string_assign_header_00425f40(&s, r);
        destroy(o50);
        destroy(o58);
        destroy(o60);
        destroy(o68);
        destroy(o48);
        destroy(b20);
        destroy(c30);
        destroy(c40);
    } else if (value.tag == 7) {
        {
            NativeStr b20;
            host.native_string_assign_0041e870(&b20, "{ ");
            host.native_string_append_00425e10(&s, &b20);
            destroy(b20);
        }
        std::uint32_t cell = 0;  // ESI walks the 16 floats
        for (std::int32_t rows = 4; rows != 0; --rows) {
            {
                NativeStr b20;
                host.native_string_assign_0041e870(&b20, "{ ");
                host.native_string_append_00425e10(&s, &b20);
                destroy(b20);
            }
            for (std::int32_t n = 4; n != 0; --n) {
                NativeStr sep18;
                host.native_string_assign_0041e870(&sep18, ", ");
                NativeStr f40;
                NativeStr o30;
                const NativeStr* r = host.native_string_concat_float_006af2b0(&s, &f40, float_at(cell));
                r = host.native_string_concat_004261a0(r, &o30, &sep18);
                host.native_string_assign_header_00425f40(&s, r);
                release(o30, host);
                release(f40, host);
                release(sep18, host);
                ++cell;
            }
            NativeStr close28;
            host.native_string_assign_0041e870(&close28, "}, ");
            host.native_string_append_00425e10(&s, &close28);
            release(close28, host);
        }
        NativeStr end20;
        host.native_string_assign_0041e870(&end20, "}");
        host.native_string_append_00425e10(&s, &end20);
        destroy(end20);
    } else if (value.tag == 8) {
        // The four floats are copied to the frame (ESP+88h..94h) before any call.
        const float v0 = float_at(0), v1 = float_at(1), v2 = float_at(2), v3 = float_at(3);
        NativeStr c28, c18, c40, c30, b20;
        host.native_string_assign_0041e870(&c28, " }");
        host.native_string_assign_0041e870(&c18, ", ");
        host.native_string_assign_0041e870(&c40, ", ");
        host.native_string_assign_0041e870(&c30, ", ");
        host.native_string_assign_0041e870(&b20, "{ ");
        NativeStr o78, o80, o70, o38, o48, o68, o60, o58, o50;
        const NativeStr* r = host.native_string_concat_004261a0(&s, &o78, &b20);
        r = host.native_string_concat_float_006af2b0(r, &o80, v0);
        r = host.native_string_concat_004261a0(r, &o70, &c30);
        r = host.native_string_concat_float_006af2b0(r, &o38, v1);
        r = host.native_string_concat_004261a0(r, &o48, &c40);
        r = host.native_string_concat_float_006af2b0(r, &o68, v2);
        r = host.native_string_concat_004261a0(r, &o60, &c18);
        r = host.native_string_concat_float_006af2b0(r, &o58, v3);
        r = host.native_string_concat_004261a0(r, &o50, &c28);
        host.native_string_assign_header_00425f40(&s, r);
        destroy(o50);
        destroy(o58);
        destroy(o60);
        destroy(o68);
        destroy(o48);
        destroy(o38);
        destroy(o70);
        destroy(o80);
        destroy(o78);
        destroy(b20);
        destroy(c30);
        destroy(c40);
        destroy(c18);
        destroy(c28);
    } else if (value.tag == 10) {
        const std::int32_t number = static_cast<std::int32_t>(value.payload);  // MOVSS copy to ESP+38h
        NativeStr q30, q20;
        host.native_string_assign_0041e870(&q30, kQuote_00ce436c);
        host.native_string_assign_0041e870(&q20, kQuote_00ce436c);
        NativeStr o70, o80, o78;
        const NativeStr* r = host.native_string_concat_004261a0(&s, &o70, &q20);
        r = host.native_string_concat_int_004263b0(r, &o80, number);
        r = host.native_string_concat_004261a0(r, &o78, &q30);
        host.native_string_assign_header_00425f40(&s, r);
        destroy(o78);
        destroy(o80);
        destroy(o70);
        destroy(q20);
        destroy(q30);
    }
    emit_text(&s, g, host);
    release(s, host);
}

// ---------------------------------------------------------------------------
// 0064A2F0: +157h = 0, +156h = 0, [+38h]->vtable[34h](0);
// 00545360(ECX = [[00E198C4]+50h]); unit = [00E188D8]; setting = [screen+14Ch];
// [unit+A44h] = setting; 00983C10(ECX = [00F8A0C4]; [unit+A20h], setting);
// 0068AB80(ECX = [00E198C4]; 0, 0); tail-jump 0068AB30(ECX = [00E198C4]).
// ---------------------------------------------------------------------------
void hud_ship_screen_close_order_menu_0064a2f0(Raw screen, Front5Host& host) {
    void* widget = screen.ptr(0x38);
    screen.at<std::uint8_t>(0x157) = 0;
    screen.at<std::uint8_t>(0x156) = 0;
    host.widget_vslot_34(widget, 0);
    Raw ui{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))};
    host.weapon_group_screen_suspend_00545360(ui.ptr(0x50));
    Raw unit{static_cast<std::uint8_t*>(host.global_pointer(0x00E188D8))};
    const std::uint32_t setting = screen.at<std::uint32_t>(0x14C);
    void* entity = unit.ptr(0xA20);
    unit.at<std::uint32_t>(0xA44) = setting;
    host.warning_dispatch_repair_00983c10(host.global_pointer(0x00F8A0C4), entity, setting);
    host.ingame_collapse_overlays_0068ab80(host.global_pointer(0x00E198C4), 0, 0);
    host.ingame_leave_unit_request_level_0068ab30(host.global_pointer(0x00E198C4));
}

// ---------------------------------------------------------------------------
// 0064B650: 00519290(block), then +3ECh = 00CEC728 (BF060A92h, -30 degrees in
// radians), +3F4h = 0, +400h = 0 (byte), vtables +0 00CF5CE8, +10h 00CF5CCC,
// +24h 00CF5CC4, +170h 00CF5CAC, +3F0h = 00CEC400 (3FC6D3F2h, 89 degrees).
// No null test on the block.
// ---------------------------------------------------------------------------
void* ship_captain_camera_construct_0064b650(Raw block, Front5Host& host) {
    host.ship_camera_construct_base_00519290(block.p);
    block.at<std::uint32_t>(0x3EC) = 0xBF060A92u;
    block.at<std::uint32_t>(0x3F4) = 0;
    block.at<std::uint8_t>(0x400) = 0;
    block.at<std::uint32_t>(0x0) = 0x00CF5CE8u;
    block.at<std::uint32_t>(0x10) = 0x00CF5CCCu;
    block.at<std::uint32_t>(0x24) = 0x00CF5CC4u;
    block.at<std::uint32_t>(0x170) = 0x00CF5CACu;
    block.at<std::uint32_t>(0x3F0) = 0x3FC6D3F2u;
    return block.p;
}

void quad_fade_target_clear_0064a520(Raw object) noexcept { object.at<std::uint32_t>(0x20) = 0; }
void quad_fade_target_set_0064a530(Raw object) noexcept { object.at<std::uint32_t>(0x20) = 1; }
std::uint8_t byte_08_get_00652a30(Raw object) noexcept { return object.at<std::uint8_t>(0x8); }
float ship_float_244_get_008356b0(Raw ship) noexcept { return ship.at<float>(0x244); }
float ship_float_248_get_008356f0(Raw ship) noexcept { return ship.at<float>(0x248); }
std::uint8_t ship_byte_243_get_008356c0(Raw ship) noexcept { return ship.at<std::uint8_t>(0x243); }

// ---------------------------------------------------------------------------
// 0047BB70: MOV ECX,[ESP+4]; PUSH [ESP+8]; CALL [[ECX]+8]; RET 8. The caller's
// ECX (for example [[00E188A8]+21D0h] at 0074DC1A) is never read.
// ---------------------------------------------------------------------------
void* object_forward_vslot_08_0047bb70(void* /*unused_ecx*/, void* object, std::uint32_t argument,
                                       Front5Host& host) {
    return host.object_vslot_08(object, argument);
}

// ---------------------------------------------------------------------------
// 008624C0. ai = [bridge], director = [bridge+4] (both re-read at each use).
// ---------------------------------------------------------------------------
void director_bridge_apply_008624c0(Raw bridge, std::uint8_t force, Front5Host& host) {
    auto ai = [&] { return Raw{static_cast<std::uint8_t*>(bridge.ptr(0x0))}; };
    auto director = [&] { return Raw{static_cast<std::uint8_t*>(bridge.ptr(0x4))}; };
    ai().at<std::uint8_t>(0x7C) = director().at<std::uint8_t>(0x3D);
    bridge.at<std::int32_t>(0x8) += -1;
    if (force != 0 || bridge.at<std::int32_t>(0x8) <= 0) {  // JG skips: signed
        force = 1;  // stored into the argument slot, then reloaded
        bridge.at<std::int32_t>(0x8) = 10;
    }
    const std::uint8_t stance = director().at<std::uint8_t>(0x3C);
    if (force != 0 || stance != bridge.at<std::uint8_t>(0xC)) {
        bridge.at<std::uint8_t>(0xC) = stance;
        Raw o = ai();
        // Three dword stores over +70h..+7Bh, then +77h and +78h put back.
        const std::uint8_t keep77 = o.at<std::uint8_t>(0x77);
        const std::uint8_t keep78 = o.at<std::uint8_t>(0x78);
        const std::uint32_t fill = stance != 0 ? 0x01010101u : 0u;
        o.at<std::uint32_t>(0x70) = fill;
        o.at<std::uint32_t>(0x74) = fill;
        o.at<std::uint32_t>(0x78) = fill;
        if (stance != 0) {
            o.at<std::uint8_t>(0x78) = keep78;
            o.at<std::uint8_t>(0x77) = keep77;
        } else {
            o.at<std::uint8_t>(0x77) = keep77;
            void* owner = o.ptr(0x50);
            o.at<std::uint8_t>(0x78) = keep78;
            if (owner != nullptr && host.entity_is_kind_vslot_5c(owner, 0xF)) {
                o.at<std::uint8_t>(0x70) = 1;
                o.at<std::uint8_t>(0x71) = 1;
            }
        }
    }
    std::uint8_t v = director().at<std::uint8_t>(0x222);
    if (force != 0 || v != bridge.at<std::uint8_t>(0xD)) {
        bridge.at<std::uint8_t>(0xD) = v;
        host.gunnery_set_torpedo_allowed_00861d70(ai().p, v);
    }
    v = director().at<std::uint8_t>(0x220);
    if (force != 0 || v != bridge.at<std::uint8_t>(0xE)) {
        bridge.at<std::uint8_t>(0xE) = v;
        host.gunnery_set_artillery_allowed_00861cd0(ai().p, v);
    }
    v = director().at<std::uint8_t>(0x221);
    if (force != 0 || v != bridge.at<std::uint8_t>(0xF)) {
        bridge.at<std::uint8_t>(0xF) = v;
        host.gunnery_set_aa_allowed_00861d20(ai().p, v);
    }
    v = director().at<std::uint8_t>(0x223);
    if (v != bridge.at<std::uint8_t>(0x10)) {  // no force test for this one (008625D9)
        bridge.at<std::uint8_t>(0x10) = v;
        host.gunnery_set_depth_charge_allowed_00861dc0(ai().p, v);
    }
}

// ---------------------------------------------------------------------------
// 0086C120: members destroyed in reverse order. +3Ch dword vector: 0086A430(0),
// free(data); +30h/+24h/+18h float arrays: 00818030(0), free(data); +0Ch and
// +00h qword vectors: 0086A350(0) when capacity +8 < 0 (signed), the inlined
// element loop counts +4 down to 0 one step at a time, +4 = 0, free(data).
// The data pointers are not cleared.
// ---------------------------------------------------------------------------
void water_tracer_destroy_curves_0086c120(Raw tracer, Front5Host& host) {
    host.dword_vector_resize_0086a430(tracer.field(0x3C), 0);
    host.free_00bf6989(tracer.ptr(0x3C));
    for (std::size_t off : {std::size_t{0x30}, std::size_t{0x24}, std::size_t{0x18}}) {
        host.float_array_resize_00818030(tracer.field(off), 0);
        host.free_00bf6989(tracer.ptr(off));
    }
    for (std::size_t off : {std::size_t{0x0C}, std::size_t{0x00}}) {
        Raw v{tracer.field(off)};
        if (v.at<std::int32_t>(0x8) < 0) host.qword_vector_reserve_0086a350(v.p, 0);
        while (v.at<std::int32_t>(0x4) > 0) v.at<std::int32_t>(0x4) += -1;
        v.at<std::int32_t>(0x4) = 0;
        host.free_00bf6989(v.ptr(0x0));
    }
}

// ---------------------------------------------------------------------------
// 0072E510: 00728960(block, argument), then the gun vtables and field stores in
// the image's order (0072E535..0072E66C). XMM0 = 0 for +474h/+478h; -1 for
// +3FCh/+440h; class id 20h at +0C4h; +428h = the block itself (the embedded
// sub-object at +424h, vtable 00CFDEA0, points back at its owner).
// ---------------------------------------------------------------------------
void* gun_construct_0072e510(Raw block, std::uint32_t argument, Front5Host& host) {
    host.gun_base_construct_00728960(block.p, argument);
    block.at<std::uint32_t>(0x0) = 0x00CFE0A8u;
    block.at<std::uint32_t>(0x10) = 0x00CFE08Cu;
    block.at<std::uint32_t>(0x24) = 0x00CFE084u;
    block.at<std::uint32_t>(0x170) = 0x00CFE080u;
    block.at<std::uint32_t>(0x1E4) = 0x00CFE078u;
    block.at<std::uint32_t>(0x310) = 0x00CFE060u;
    for (std::size_t off : {0x3A8u, 0x3ACu, 0x3B0u, 0x3D4u, 0x3D8u, 0x3DCu, 0x3E4u, 0x3E8u, 0x3ECu, 0x42Cu}) {
        block.at<std::uint32_t>(off) = 0;
    }
    block.at<std::uint32_t>(0x424) = 0x00CFDEA0u;
    block.at<std::uint8_t*>(0x428) = block.p;
    for (std::size_t off : {0x430u, 0x434u, 0x438u, 0x45Cu, 0x460u, 0x464u, 0x46Cu}) {
        block.at<std::uint32_t>(off) = 0;
    }
    block.at<std::uint32_t>(0x458) = 0x00CE75CCu;
    block.at<std::uint8_t>(0x468) = 1;
    block.at<std::uint32_t>(0x47C) = 0;
    block.at<std::uint32_t>(0x3FC) = 0xFFFFFFFFu;
    block.at<std::uint32_t>(0x440) = 0xFFFFFFFFu;
    block.at<std::uint32_t>(0x414) = 0;
    block.at<std::uint32_t>(0x418) = 0;
    block.at<std::uint32_t>(0x358) = 0;
    block.at<std::uint8_t>(0x454) = 0;
    block.at<std::uint8_t>(0x470) = 0;
    block.at<std::uint32_t>(0x444) = 0;
    block.at<std::uint32_t>(0x3F4) = 0;
    block.at<std::uint32_t>(0x41C) = 0;
    block.at<std::uint32_t>(0x420) = 0;
    block.at<std::uint32_t>(0x3C0) = 0;
    block.at<std::uint32_t>(0x324) = 3;
    block.at<std::uint32_t>(0x0C4) = 0x20;
    block.at<float>(0x474) = 0.0f;
    block.at<std::uint32_t>(0x43C) = 1;
    block.at<float>(0x478) = 0.0f;
    return block.p;
}

namespace {

// The gun's barrel-record vector {begin +3D4h, end +3D8h}, 14h-byte records:
// (end - begin) / 20 with the signed IMUL 66666667h / SAR 3 / sign fix-up.
std::int32_t barrel_count(Raw gun) {
    const auto begin = gun.at<std::uint32_t>(0x3D4);
    const auto end = gun.at<std::uint32_t>(0x3D8);
    return static_cast<std::int32_t>(end - begin) / 20;
}

// The inlined checked operator[]: begin == 0 or i >= count reports through
// 00BF6713 and the access goes ahead.
void check_barrel_index(Raw gun, std::uint32_t i, Front5Host& host) {
    if (gun.at<std::uint32_t>(0x3D4) == 0 || !(i < static_cast<std::uint32_t>(barrel_count(gun)))) {
        host.invalid_parameter_00bf6713();
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 0072B2D0. Thread: the unit update; the arithmetic below is exact in double
// (float sums/products of floats and the 1/120 constant, which is a float
// widened), so each float() is the image's single rounding at 24 or 53 bits.
// Barrel record (14h): +4h position, +8h velocity, +0Ch time bank, +10h active.
// Compares follow the image's jumps for NaN: COMISS/FCOMIP + JC/JBE skip on
// unordered, so each C++ test is written to be false for NaN on the skipping arm.
// ---------------------------------------------------------------------------
void gun_update_barrel_recoil_0072b2d0(Raw gun, float dt, Front5Host& host) {
    if (gun.ptr(0x3C0) != nullptr) {
        Raw owner{static_cast<std::uint8_t*>(gun.ptr(0x3F0))};
        if (owner.at<std::uint8_t>(0x5D) != 0 || host.entity_is_kind_vslot_5c(owner.p, 8)) {
            Raw barrel{static_cast<std::uint8_t*>(gun.ptr(0x3C0))};
            if (barrel.ptr(0x30) != nullptr) {
                Raw node{static_cast<std::uint8_t*>(gun.ptr(0x3BC))};
                if ((node.at<std::uint8_t>(0x5C) & 2) == 0) host.transform_refresh_world_00b6db70(node.p);
                if (0.0f > node.at<float>(0x124)) {  // COMISS 0,[+124h]; JBE: NaN -> 00729870
                    host.gun_apply_wreck_visibility_007297b0(gun.p);
                } else {
                    host.gun_show_only_mesh_holder_00729870(gun.p);
                }
            }
        }
    }
    host.entity_base_update_0043ff40(gun.p, dt);

    constexpr float kStep = 0x1.111112p-7f;    // 00CEF0B8 = 3C088889h (1/120)
    constexpr double kStepD = 0x1.111112p-7;   // 00CFDF80 = 3F811111 20000000h, the same value
    for (std::uint32_t i = 0;; ++i) {
        if (gun.at<std::uint32_t>(0x3D4) == 0) break;
        if (!(i < static_cast<std::uint32_t>(barrel_count(gun)))) break;
        check_barrel_index(gun, i, host);
        Raw rec{gun.at<std::uint8_t*>(0x3D4) + i * 0x14};
        // UCOMISS [+8h],0; LAHF; TEST AH,44h; JP: equal (and ordered) is the only idle case.
        if (rec.at<float>(0x8) == 0.0f) {
            rec.at<std::uint8_t>(0x10) = 0;
            continue;
        }
        rec.at<std::uint8_t>(0x10) = 1;
        const float bank = static_cast<float>(double(rec.at<float>(0xC)) + double(dt));
        rec.at<float>(0xC) = bank;
        if (!(bank >= kStep)) continue;  // FCOMIP; JC: below or unordered -> next barrel
        do {
            const float c = rec.at<float>(0xC);
            const float velocity = rec.at<float>(0x8);
            rec.at<float>(0xC) = static_cast<float>(double(c) - kStepD);
            if (0.0f > velocity) {  // COMISS 0,v; JBE: NaN -> the else arm
                rec.at<float>(0x4) = static_cast<float>(double(velocity) + double(rec.at<float>(0x4)));
                Raw cfg{static_cast<std::uint8_t*>(gun.ptr(0x3F4))};
                rec.at<float>(0x8) = static_cast<float>(double(velocity) * double(cfg.at<float>(0xAC)));
                cfg = Raw{static_cast<std::uint8_t*>(gun.ptr(0x3F4))};
                if (cfg.at<float>(0xB0) >= rec.at<float>(0x4)) rec.at<float>(0x8) = cfg.at<float>(0xB4);
            } else {
                const float position = static_cast<float>(double(velocity) + double(rec.at<float>(0x4)));
                rec.at<float>(0x4) = position;
                if (position >= 0.0f) {  // FCOMIP with the 0.0 kept in ST2; JC skips
                    rec.at<float>(0x4) = 0.0f;
                    rec.at<float>(0x8) = 0.0f;
                    rec.at<float>(0xC) = 0.0f;
                }
            }
        } while (rec.at<float>(0xC) >= kStep);  // COMISS; JNC
    }
}

// ---------------------------------------------------------------------------
// 0072ADC0: the gun's archive dump. Keys are {0, name}; indexed entries {1, i}.
// Value tags: 1 int, 2 float bits, 3 bool (low byte only). Order as in the image.
// ---------------------------------------------------------------------------
void gun_save_fields_0072adc0(Raw gun, void* archive, Front5Host& host) {
    auto name = [](const char* text) {
        return TaggedValue{0, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text))};
    };
    auto index = [](std::uint32_t i) { return TaggedValue{1, i}; };
    auto f32 = [](float v) {
        std::uint32_t bits;
        std::memcpy(&bits, &v, 4);
        return TaggedValue{2, bits};
    };
    auto i32 = [](std::uint32_t v) { return TaggedValue{1, v}; };
    auto flag = [](std::uint8_t v) { return TaggedValue{3, v}; };

    host.device_entity_save_archive_0043fea0(gun.p, archive);
    host.archive_vslot_04_open(archive, name("_gun"));          // 00CFDF78
    host.archive_vslot_04_open(archive, name("fireModeAmmo"));  // 00CFDF68
    for (std::int32_t i = 0; i < gun.at<std::int32_t>(0x3AC); ++i) {
        // MOVZX word [records +3A8h + i*14h + 4]
        const std::uint16_t ammo = *reinterpret_cast<const std::uint16_t*>(gun.at<std::uint8_t*>(0x3A8) + i * 0x14 + 4);
        host.archive_vslot_0c_write(archive, index(static_cast<std::uint32_t>(i)), i32(ammo));
    }
    host.archive_vslot_08_close(archive);
    host.archive_vslot_0c_write(archive, name("fireMode"), i32(gun.at<std::uint32_t>(0x3FC)));    // 00CFDF5C
    host.archive_vslot_0c_write(archive, name("destroyed"), flag(gun.at<std::uint8_t>(0x3B8)));  // 00CE44D0
    host.archive_vslot_04_open(archive, name("barrelRecoilVec"));                                // 00CFDF4C
    for (std::uint32_t i = 0;; ++i) {
        if (gun.at<std::uint32_t>(0x3D4) == 0) break;
        if (!(i < static_cast<std::uint32_t>(barrel_count(gun)))) break;
        host.archive_vslot_04_open(archive, index(i));
        check_barrel_index(gun, i, host);
        host.archive_vslot_0c_write(archive, name("dist"), f32(*reinterpret_cast<const float*>(gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0x4)));  // 00CFD71C
        check_barrel_index(gun, i, host);
        host.archive_vslot_0c_write(archive, name("speed"), f32(*reinterpret_cast<const float*>(gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0x8)));
        check_barrel_index(gun, i, host);
        host.archive_vslot_0c_write(archive, name("DT"), f32(*reinterpret_cast<const float*>(gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0xC)));  // 00CFDF48
        host.archive_vslot_08_close(archive);
    }
    host.archive_vslot_08_close(archive);
    host.archive_vslot_0c_write(archive, name("throwA"), f32(gun.at<float>(0x400)));
    host.archive_vslot_0c_write(archive, name("throwB"), f32(gun.at<float>(0x404)));
    host.archive_vslot_04_open(archive, name("loadTime"));
    for (std::int32_t i = 0; i < gun.at<std::int32_t>(0x448); ++i) {
        host.archive_vslot_0c_write(archive, index(static_cast<std::uint32_t>(i)),
                                    f32(*reinterpret_cast<const float*>(gun.at<std::uint8_t*>(0x414) + i * 4)));
    }
    host.archive_vslot_08_close(archive);
    host.archive_vslot_0c_write(archive, name("barrelDelayTime"), f32(gun.at<float>(0x450)));
    if (void* gen = gun.ptr(0x41C); gen != nullptr) {
        host.archive_vslot_04_open(archive, name("genParam"));
        host.object_vslot_08_save(gen, archive);
        host.archive_vslot_08_close(archive);
    }
    host.archive_vslot_0c_write(archive, name("delayGroupSize"), i32(gun.at<std::uint32_t>(0x43C)));
    host.archive_vslot_0c_write(archive, name("delayGroupIndex"), i32(gun.at<std::uint32_t>(0x440)));
    host.archive_vslot_0c_write(archive, name("delayGroupCnt"), i32(gun.at<std::uint32_t>(0x444)));
    host.archive_vslot_0c_write(archive, name("contFiring"), flag(gun.at<std::uint8_t>(0x454)));
    host.archive_vslot_0c_write(archive, name("nextFireBarrel"), i32(gun.at<std::uint32_t>(0x44C)));
    host.archive_vslot_0c_write(archive, name("barrelNum"), i32(gun.at<std::uint32_t>(0x448)));
    host.archive_vslot_08_close(archive);
}

// ---------------------------------------------------------------------------
// 0072F060: the inverse of 0072ADC0. Frame: ESP+10h one dword slot reused for
// every int read (the image leaves it uninitialised; zero here), ESP+14h the
// 14h-byte reader {vtable, +4, LuaObject vector begin +8, end +0Ch, +10h}.
// Typed outputs are {type, pointer}: 1 int, 2 float, 3 bool.
// ---------------------------------------------------------------------------
void gun_load_saved_state_0072f060(Raw gun, Front5Host& host) {
    auto addr = [](const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); };
    auto name = [&](const char* text) { return TaggedValue{0, addr(text)}; };
    auto out = [&](std::uint32_t type, const void* p) { return TaggedValue{type, addr(p)}; };

    host.entity_load_saved_state_00440e10(gun.p);
    Raw descriptor{static_cast<std::uint8_t*>(gun.ptr(0xC0))};
    if (descriptor.p != nullptr && descriptor.at<std::uint32_t>(0x4) == 3) {
        alignas(4) std::uint8_t lua_object_arg[0x14];  // the by-value argument block
        host.saved_entity_lua_data_009238a0(gun.p, lua_object_arg);
        alignas(4) std::uint8_t reader_bytes[0x14];
        void* const reader = reader_bytes;
        host.lua_reader_construct_004425c0(reader, lua_object_arg);
        std::uint32_t slot_10 = 0;
        host.lua_reader_enter_key_00bd8e20(reader, name("_gun"));          // 00CFDF78
        host.lua_reader_enter_key_00bd8e20(reader, name("fireModeAmmo"));  // 00CFDF68
        for (std::int32_t i = 0; i < gun.at<std::int32_t>(0x3AC); ++i) {
            std::uint8_t* rec = gun.at<std::uint8_t*>(0x3A8) + i * 0x14;
            host.lua_reader_vslot_10_read(reader, TaggedValue{1, static_cast<std::uint32_t>(i)}, out(1, &slot_10));
            *reinterpret_cast<std::uint16_t*>(rec + 4) = static_cast<std::uint16_t>(slot_10);
        }
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_vslot_10_read(reader, name("fireMode"), out(1, &slot_10));  // 00CFDF5C
        host.gun_select_fire_mode_0072d5b0(gun.p, slot_10, 1);
        host.lua_reader_read_field_00bd6830(reader, name("destroyed"), out(3, gun.field(0x3B8)));  // 00CE44D0
        host.lua_reader_enter_key_00bd8e20(reader, name("barrelRecoilVec"));  // 00CFDF4C
        for (std::uint32_t i = 0;; ++i) {
            if (gun.at<std::uint32_t>(0x3D4) == 0) break;
            if (!(i < static_cast<std::uint32_t>(barrel_count(gun)))) break;
            host.lua_reader_enter_key_00bd8e20(reader, TaggedValue{1, i});
            check_barrel_index(gun, i, host);
            host.lua_reader_read_field_00bd6830(reader, name("dist"), out(2, gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0x4));  // 00CFD71C
            check_barrel_index(gun, i, host);
            host.lua_reader_read_field_00bd6830(reader, name("speed"), out(2, gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0x8));  // 00CE6748
            check_barrel_index(gun, i, host);
            host.lua_reader_read_field_00bd6830(reader, name("DT"), out(2, gun.at<std::uint8_t*>(0x3D4) + i * 0x14 + 0xC));  // 00CFDF48
            host.lua_reader_leave_key_00bd7a20(reader);
        }
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_read_field_00bd6830(reader, name("throwA"), out(2, gun.field(0x400)));  // 00CFDF40
        host.lua_reader_read_field_00bd6830(reader, name("throwB"), out(2, gun.field(0x404)));  // 00CFDF38
        host.lua_reader_enter_key_00bd8e20(reader, name("loadTime"));                           // 00CFDF2C
        for (std::int32_t i = 0; i < gun.at<std::int32_t>(0x448); ++i) {
            host.lua_reader_read_field_00bd6830(reader, TaggedValue{1, static_cast<std::uint32_t>(i)},
                                                out(2, gun.at<std::uint8_t*>(0x414) + i * 4));
            // 0072F468: the value just read is overwritten with 0.0f (MOVSS of XORPS zero).
            *reinterpret_cast<float*>(gun.at<std::uint8_t*>(0x414) + i * 4) = 0.0f;
        }
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_read_field_00bd6830(reader, name("barrelDelayTime"), out(2, gun.field(0x450)));  // 00CFDF1C
        if (host.lua_reader_has_key_00bd5eb0(reader, name("genParam"))) {  // 00CFDF10
            host.lua_reader_enter_key_00bd8e20(reader, name("genParam"));
            void* const command = host.gun_shot_command_read_and_create_00731ae0(reader);
            host.lua_reader_leave_key_00bd7a20(reader);
            if (command != nullptr) {
                if (void* old = gun.ptr(0x41C); old != nullptr) {
                    host.object_vslot_00_delete(old, 1);
                    gun.ptr(0x41C) = nullptr;
                }
                gun.ptr(0x41C) = command;
            }
        }
        host.lua_reader_read_field_00bd6830(reader, name("delayGroupSize"), out(1, gun.field(0x43C)));   // 00CFDF00
        host.lua_reader_read_field_00bd6830(reader, name("delayGroupIndex"), out(1, gun.field(0x440)));  // 00CFDEF0
        host.lua_reader_read_field_00bd6830(reader, name("delayGroupCnt"), out(1, gun.field(0x444)));    // 00CFDEE0
        host.lua_reader_read_field_00bd6830(reader, name("contFiring"), out(3, gun.field(0x454)));       // 00CFDED4
        host.lua_reader_read_field_00bd6830(reader, name("nextFireBarrel"), out(1, gun.field(0x44C)));   // 00CFDEC4
        host.lua_reader_read_field_00bd6830(reader, name("barrelNum"), out(1, gun.field(0x448)));        // 00CFDEB8
        host.lua_reader_leave_key_00bd7a20(reader);
        // The inlined reader destructor: vtable 00CE44FC, then the LuaObject vector.
        Raw r{reader_bytes};
        r.at<std::uint32_t>(0x0) = 0x00CE44FCu;
        std::uint8_t* const begin = r.at<std::uint8_t*>(0x8);
        if (begin != nullptr) {
            std::uint8_t* const end = r.at<std::uint8_t*>(0xC);
            for (std::uint8_t* p = begin; p != end; p += 0x14) host.lua_object_destruct_00b67700(p);
            host.operator_delete_00bf65ac(r.at<std::uint8_t*>(0x8));
        }
    }
    gun.at<std::uint32_t>(0x54) = Raw{static_cast<std::uint8_t*>(gun.ptr(0x3F0))}.at<std::uint32_t>(0x54);
}

}  // namespace bsp::cc13_front5
