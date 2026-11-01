#include "bsp/native_gui_text_property_reader_storage.hpp"
#include "bsp/native_lua_reader_queries.hpp"
#include "bsp/native_crt_stricmp.hpp"
#include "bsp/native_string_compare.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <stdexcept>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text property reader requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word offset = 0) noexcept { return pointer(bits(p) + offset); }
Word get(const volatile void* p, Word delta = 0) noexcept {
    Word value;
    __asm {
        mov eax, p
        mov edx, delta
        mov eax, [eax + edx]
        mov value, eax }
    return value;
}
void put(void* p, Word value, Word delta = 0) noexcept {
    __asm {
        mov eax, p
        mov edx, delta
        mov ecx, value
        mov [eax + edx], ecx }
}
void put_byte(void* p, unsigned char value, Word delta = 0) noexcept {
    __asm {
        mov eax, p
        mov edx, delta
        mov cl, value
        mov [eax + edx], cl }
}
unsigned char get_byte(const void* p) noexcept {
    unsigned char value;
    __asm {
        mov eax, p
        mov cl, [eax]
        mov value, cl }
    return value;
}
Word sse_bits(const volatile float& cell) noexcept {
    const volatile float* const address = &cell;
    Word value;
    __asm {
        mov eax, address
        movss xmm0, [eax]
        movss value, xmm0 }
    return value;
}
Word sse_zero() noexcept {
    Word value;
    __asm {
        xorps xmm0, xmm0
        movss value, xmm0 }
    return value;
}
void put_sse(void* p, Word value) noexcept {
    const Word* const address = &value;
    __asm {
        mov eax, p
        mov edx, address
        movss xmm0, [edx]
        movss [eax], xmm0 }
}
void guard_or_one(Word& guard) noexcept {
    Word* const address = &guard;
    __asm {
        mov eax, address
        or dword ptr [eax], 1 }
}
void require(bool value, const char* message) { if (!value) throw std::logic_error(message); }
struct Current { Word profile, target; };
} // namespace
NativeGuiTextPropertyReaderLiterals::NativeGuiTextPropertyReaderLiterals(
    const game::GameNativeReadOnlyData& data) : data_(data), keys_{
    static_cast<const char*>(data.data_at(0x00d5c6c0u, 5)),
    static_cast<const char*>(data.data_at(0x00d5c6b4u, 10)),
    static_cast<const char*>(data.data_at(0x00d5c6a8u, 10)),
    static_cast<const char*>(data.data_at(0x00d5c690u, 21)),
    static_cast<const char*>(data.data_at(0x00d5c684u, 12)),
    static_cast<const char*>(data.data_at(0x00d5c3ccu, 11)),
    static_cast<const char*>(data.data_at(0x00ced1a8u, 6)),
    static_cast<const char*>(data.data_at(0x00ce92e4u, 5)),
    static_cast<const char*>(data.data_at(0x00ce92dcu, 6)),
    static_cast<const char*>(data.data_at(0x00d5c67cu, 7)),
    static_cast<const char*>(data.data_at(0x00d5c670u, 10)),
    static_cast<const char*>(data.data_at(0x00d5c660u, 14)),
    static_cast<const char*>(data.data_at(0x00ce92d8u, 4)),
    static_cast<const char*>(data.data_at(0x00ce92d0u, 7)),
    static_cast<const char*>(data.data_at(0x00d5c654u, 10)),
    static_cast<const char*>(data.data_at(0x00cecd20u, 7)),
    static_cast<const char*>(data.data_at(0x00d5c64cu, 6)),
    static_cast<const char*>(data.data_at(0x00d5c640u, 9)),
    static_cast<const char*>(data.data_at(0x00d5c634u, 9)),
    static_cast<const char*>(data.data_at(0x00d5c624u, 14)),
    static_cast<const char*>(data.data_at(0x00d5c618u, 9)),
    static_cast<const char*>(data.data_at(0x00d5c60cu, 12)),
    static_cast<const char*>(data.data_at(0x00d5c5f8u, 10)),
    static_cast<const char*>(data.data_at(0x00ce92c8u, 6)),
    static_cast<const char*>(data.data_at(0x00d5c5e8u, 13)),
    static_cast<const char*>(data.data_at(0x00ce3a0cu, 1))} { (void)data.data_at(0x00ce44fcu, 24); }
const char* NativeGuiTextPropertyReaderLiterals::key(Word index) const { return keys_[index]; }
const volatile Word* NativeGuiTextPropertyReaderLiterals::table(Word profile) const {
    return static_cast<const volatile Word*>(data_.data_at(profile, 24));
}

void read_native_gui_text_properties_storage_00abb630(void* actual,
    volatile Word& visitor_home, NativeGuiTextPropertyReaderStorageFrame& f,
    NativeGuiTextPropertyReaderStorageContext& c) {
    using Phase = NativeGuiTextPropertyReaderStorageFrame::Phase;
    auto& a = f.observation_;
    require(a.phase == Phase::fresh, "Text reader requires its distinct fresh retained frame");
    a.phase = Phase::entered;
    auto& h = f.homes_;
    Word* const key = h.calls.key_pair_18;
    Word* const four = h.calls.actual_homes_10;
    void* const field = four;
    void* const fallback = four + 2;
    auto metadata = [&](const void* pair) { put(&h.metadata_10, bits(pair)); };
    auto current = [&](Word slot, Word site) {
        a.active_site = site;
        const Word profile = get(a.captured_visitor);
        const Word target = c.literals.table(profile)[slot / 4];
        a.captured_profile = profile; a.captured_target = target;
        return Current{profile, target};
    };
    auto default_call = [&](Word site, int native_state = -1) {
        const auto call = current(0x0c, site);
        if (native_state >= 0) put_byte(&h.state_44, static_cast<unsigned char>(native_state));
        metadata(key);
        const Word tag = get(key), payload = get(key, 4);
        if (call.target == 0x00bd68d0u) {
            NativeLuaFieldResolverAdapter::Activation active(c.resolvers, f.lua_.resolver);
            read_native_lua_field_or_default_00bd68d0(c.reader, tag, payload,
                field, fallback, f.lua_.field, c.fields);
        } else c.other.default_0c(a.captured_visitor, call.profile, call.target,
            tag, payload, field, fallback, f);
    };
    auto default_key = [&](Word index, Word site, int native_state = -1) {
        put(key, 0); put(key, bits(c.literals.key(index)), 4);
        default_call(site, native_state);
    };
    auto string_default = [&](Word index, Word* header, Word site, int native_state = -1) {
        put(fallback, 0); metadata(fallback); put(fallback, bits(c.literals.key(25)), 4);
        put(field, 0); metadata(field); put(field, bits(header), 4);
        default_key(index, site, native_state);
    };
    auto equals = [&](Word* header, Word index, Word site) {
        a.active_site = site;
        return equal_native_string_header_00425850(header, c.literals.key(index));
    };
    auto direct_compare = [&](Word data, Word index, Word site) {
        a.active_site = site;
        return compare_native_crt_strings_00bf7fbf(static_cast<const char*>(pointer(data)),
            c.literals.key(index), c.locale_changed_0109de1c, c.stricmp_callees) == 0;
    };
    auto footer = [&](Word* header, Word next_state, bool full, Word getter_site, Word return_site) {
        void* const data = pointer(get(header, 4)); // BEFORE native state transition.
        if (full) put(&h.state_44, next_state);
        else put_byte(&h.state_44, static_cast<unsigned char>(next_state));
        if (data) {
            const Word bytes = get(header) + 1u;
            put(four + 3, 1); put(four + 2, bytes); put(four + 1, bits(data));
            a.active_site = getter_site;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.fields.strings.actual_published_01090aa8,
                c.fields.strings.actual_manager_publication_01090aa0);
            a.active_site = return_site;
            return_native_string_pool_00bd1510(pool, data, bytes,
                c.fields.strings.actual_small_returns_disabled_01090aa4);
        }
    };
    try {
        put(&h.state_44, 0xffffffffu); // Original initial PUSH-1, before AAA710.
        a.active_site = 0x00abb64b;
        void* const visitor = pointer(get(&visitor_home)); // The ONE incoming visitor capture.
        a.actual_text = actual; a.captured_visitor = visitor;
        // Pure Source metadata affinity only. Actual Native receiver checks stay
        // in the genuine raw Font setter at its later Original stage.
        require(visitor == &c.reader && c.text_reference.storage() == actual,
            "Text reader requires the SAME actual reader and canonical Text companion");
        require(f.font_.phase() == NativeGuiTextRawFontNameStorageFrame::Phase::fresh &&
            f.font_.prepared_context() == &c.font_names &&
            f.font_.prepared_input_header8() == h.font_34 &&
            f.font_.acquisition() == &f.font_acquisition_ && !f.font_acquisition_.failed() &&
            c.text_reference.matches_reference_frame(*f.font_.prepared_reference_frame()),
            "Text reader requires its preprepared fresh exact raw Font invocation");
        require(&c.changed.raw_strings == &c.fields.strings &&
            &c.base_children.strings == &c.fields.strings &&
            &c.shadow.shadow_color_00e12fd8[0] == &c.globals.shadow_color_00e12fd8()[0],
            "Text reader requires SAME retained raw pool and shared shadow-color roles");
        a.active_site = 0x00abb653; a.base_entered = true;
        read_native_gui_widget_properties_and_children_00aaa710(actual, visitor, f.base_,
            c.base_interaction, c.base_children, f.children_);
        a.base_returned = true;

        put(h.font_34, 0); put(h.font_34 + 1, 0);
        put(fallback, 0); metadata(fallback); put(fallback, bits(h.font_34), 4);
        put(field, 0); put(field, bits(c.literals.key(0)), 4);
        const auto font_call = current(0x10, 0x00abb691);
        put(&h.state_44, 0); metadata(field);
        if (font_call.target == 0x00bd6830u) {
            NativeLuaFieldResolverAdapter::Activation active(c.resolvers, f.lua_.resolver);
            read_native_lua_field_00bd6830(c.reader, get(field), get(field, 4),
                fallback, f.lua_.field, c.fields);
        } else c.other.field_10(visitor, font_call.profile, font_call.target,
            get(field), get(field, 4), fallback, f);
        a.active_site = 0x00abb69a; a.font_entered = true;
        set_native_gui_text_font_name_storage_00ab8c30(c.text_reference, h.font_34,
            c.font_names, f.font_, f.font_acquisition_);
        a.font_returned = true;

        const Word one = sse_bits(c.shadow.one_00d7a24c);
        put(fallback, 2); put_sse(at(fallback, 4), one); metadata(fallback);
        metadata(field); put(field, 2); put(field, bits(at(actual, 0x1d8)), 4);
        default_key(1, 0x00abb6ec);
        put(fallback, 3); put_byte(fallback, 1, 4); metadata(fallback);
        put(field, 3); metadata(field); put(field, bits(at(actual, 0xfc)), 4);
        default_key(2, 0x00abb732);
        const Word zero = sse_zero();
        metadata(fallback); put(fallback, 2); put_sse(at(fallback, 4), zero);
        metadata(field); put(field, 2); put(field, bits(at(actual, 0x10c)), 4);
        default_key(3, 0x00abb77c);
        put(h.default_text_24, 0); put(h.default_text_24 + 1, 0);
        string_default(4, h.default_text_24, 0x00abb7c9, 1);
        string_default(5, static_cast<Word*>(at(actual, 0x1c0)), 0x00abb806);
        put(h.align_14, 0); put(h.align_14 + 1, 0);
        string_default(6, h.align_14, 0x00abb84e, 2);
        const Word align_data = get(h.align_14, 4);
        if (align_data != 0 && direct_compare(align_data, 7, 0x00abb85e)) put(at(actual, 0x100), 0);
        else if (equals(h.align_14, 8, 0x00abb880)) put(at(actual, 0x100), 2);
        else if (equals(h.align_14, 9, 0x00abb89e)) put(at(actual, 0x100), 1);
        else put(at(actual, 0x100), equals(h.align_14, 10, 0x00abb8bc) ? 3u : 0u);
        put(h.vertical_1c, 0); put(h.vertical_1c + 1, 0);
        string_default(11, h.vertical_1c, 0x00abb914, 3);
        const Word vertical_data = get(h.vertical_1c, 4);
        if (vertical_data != 0 && direct_compare(vertical_data, 12, 0x00abb924)) put(at(actual, 0x104), 0);
        else if (equals(h.vertical_1c, 13, 0x00abb946)) put(at(actual, 0x104), 2);
        else put(at(actual, 0x104), equals(h.vertical_1c, 9, 0x00abb964) ? 1u : 0u);

        put(fallback, 0); put(fallback, bits(c.literals.key(14)), 4);
        const auto query = current(0x14, 0x00abb98f); metadata(fallback);
        const Word present = query.target == 0x00bd5eb0u ?
            static_cast<Word>(native_lua_reader_has_key_00bd5eb0(c.reader, f.lua_.query,
                get(fallback), get(fallback, 4))) :
            c.other.query_14(visitor, query.profile, query.target, get(fallback), get(fallback, 4), f);
        if ((present & 0xffu) != 0) {
            put(fallback, 0); put(fallback, bits(c.literals.key(14)), 4);
            const auto enter = current(4, 0x00abb9ae); metadata(fallback);
            if (enter.target == 0x00bd8e20u)
                c.reader.enter_key_00bd8e20(f.lua_.enter, get(fallback), get(fallback, 4));
            else c.other.enter_04(visitor, enter.profile, enter.target, get(fallback), get(fallback, 4), f);
            const Word sites[4] = {0x00abba30, 0x00abbaad, 0x00abbb2a, 0x00abbba7};
            for (Word i = 0; i != 4; ++i) { // Four static Original fields, no input cap.
                auto& guard = c.globals.default_color_mask_00f8be48();
                if ((get_byte(&guard) & 1u) == 0) {
                    const Word live_one = sse_bits(c.shadow.one_00d7a24c);
                    guard_or_one(guard);
                    auto& white = c.globals.default_color_00f8be38();
                    put_sse(&white[0], live_one); put_sse(&white[1], live_one);
                    put_sse(&white[2], live_one); put_sse(&white[3], live_one);
                }
                put(fallback, 8); metadata(fallback);
                put(fallback, bits(c.globals.default_color_00f8be38()), 4);
                put(field, 8); metadata(field); put(field, bits(at(actual, 0x11c + i * 0x10)), 4);
                default_key(15 + i, sites[i]);
            }
            put_byte(actual, 1, 0x118);
            const auto leave = current(8, 0x00abbbb7);
            if (leave.target == 0x00bd7130u) c.reader.pop_back_00bd7130();
            else c.other.leave_08(visitor, leave.profile, leave.target, f);
        }
        put(fallback, 1); metadata(fallback); put(fallback, 0xffffffffu, 4);
        put(field, 1); metadata(field); put(field, bits(&visitor_home), 4);
        default_key(19, 0x00abbbfd);
        const Word preset = get(&visitor_home); // ORIGINAL incoming HOME, not visitor identity.
        if (preset != 0xffffffffu) {
            put(four + 3, preset); a.active_site = 0x00abbc0a;
            apply_native_gui_text_shadow_storage_00ab72d0(actual, four[3], f.shadow_, c.shadow);
        } else {
            put(at(actual, 0x1d0), 0xffffffffu);
            put(fallback, 3); put_byte(fallback, 0, 4); metadata(fallback);
            put(field, 3); metadata(field); put(field, bits(at(actual, 0x15c)), 4);
            default_key(20, 0x00abbc5d);
            if (get_byte(at(actual, 0x15c)) != 0) {
                put(fallback, 8); put(fallback, bits(c.shadow.shadow_color_00e12fd8), 4); metadata(fallback);
                put(field, 8); metadata(field); put(field, bits(at(actual, 0x168)), 4);
                default_key(21, 0x00abbcaf);
                put(h.shadow_pos_2c, 0); put(h.shadow_pos_2c + 1, 0);
                string_default(22, h.shadow_pos_2c, 0x00abbcfb, 4);
                const Word front = equals(h.shadow_pos_2c, 23, 0x00abbd06) ? 1u : 0u;
                const Word offset = sse_bits(c.shadow.offset_00d5c5c0); // BEFORE160 store.
                put(at(actual, 0x160), front);
                put(fallback, 2); put_sse(at(fallback, 4), offset); metadata(fallback);
                put(field, bits(at(actual, 0x164)), 4); put(field, 2); metadata(field);
                default_key(24, 0x00abbd64);
                footer(h.shadow_pos_2c, 3, false, 0x00abbd7e, 0x00abbd85);
            }
        }
        if (get(h.default_text_24) != 0) {
            put(four + 3, 1); put(four + 2, bits(h.default_text_24));
            a.active_site = 0x00abbd99; a.source_entered = true;
            submit_native_gui_text_changed_source_storage_00abaed0(actual,
                pointer(get(four + 2)), static_cast<const volatile Word&>(four[3]), c.changed, f.changed_);
            a.source_returned = true;
        }
        footer(h.vertical_1c, 2, false, 0x00abbdb6, 0x00abbdbd);
        footer(h.align_14, 1, false, 0x00abbdda, 0x00abbde1);
        footer(h.default_text_24, 0, false, 0x00abbdfd, 0x00abbe04);
        footer(h.font_34, 0xffffffffu, true, 0x00abbe24, 0x00abbe2b);
        a.phase = Phase::normal_returned;
    } catch (...) {
        a.escaped_site = a.active_site; a.failure = std::current_exception();
        a.phase = Phase::unknown_source_escape;
        throw; // No Native EH string cleanup, reader pop, rollback or retry.
    }
}
} // namespace bsp
