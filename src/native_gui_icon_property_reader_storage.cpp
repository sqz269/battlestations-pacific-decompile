#include "bsp/native_gui_icon_property_reader_storage.hpp"
#include "bsp/native_gui_icon_state_construction_storage.hpp"
#include "bsp/native_lua_reader_queries.hpp"
#include "bsp/native_string_raw_header_return.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error This raw Source supplier requires MSVC Win32 inline assembly.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(void* p, Word n) { return static_cast<std::byte*>(p) + n; }
Word bits(const volatile void* p) { return reinterpret_cast<Word>(p); }
void* pointer(Word v) { return reinterpret_cast<void*>(v); }
Word get(const volatile void* p, Word byte_offset = 0) {
    Word v;
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov eax, [eax + edx]
        mov v, eax
    }
    return v;
}
void put(void* p, Word byte_offset, Word v) {
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov ecx, v
        mov [eax + edx], ecx
    }
}
void byte(void* p, Word byte_offset, unsigned char v) {
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov cl, v
        mov [eax + edx], cl
    }
}
void current(void* visitor, Word slot, NativeGuiIconPropertyReaderStorageFrame& f) {
    const Word profile = get(visitor);
    const Word target = get(pointer(profile), slot);
    put(&f.captured_profile, 0, profile);
    put(&f.captured_target, 0, target);
}
}

NativeGuiIconPropertyReaderLiterals::NativeGuiIconPropertyReaderLiterals(
    const game::GameNativeReadOnlyData& data) : data_(data), keys_{
    static_cast<const char*>(data.data_at(0x00d5c43c, 10)),
    static_cast<const char*>(data.data_at(0x00d5c430, 11)),
    static_cast<const char*>(data.data_at(0x00d5c3cc, 11)),
    static_cast<const char*>(data.data_at(0x00d5c41c, 19)),
    static_cast<const char*>(data.data_at(0x00d5c408, 20)),
    static_cast<const char*>(data.data_at(0x00d5c3f4, 19)),
    static_cast<const char*>(data.data_at(0x00d5c3e8, 11)),
    static_cast<const char*>(data.data_at(0x00d5c3c4, 7)),
    static_cast<const char*>(data.data_at(0x00ce5fec, 8)),
    static_cast<const char*>(data.data_at(0x00cff278, 5)),
    static_cast<const char*>(data.data_at(0x00d5c228, 6)),
    static_cast<const char*>(data.data_at(0x00d5c3bc, 8)),
    static_cast<const char*>(data.data_at(0x00ce3a0c, 1))} {
    (void)data.data_at(0x00ce44fc, 24);
    (void)data.data_at(0x00d5c4c0, 0xa8);
}
const char* NativeGuiIconPropertyReaderLiterals::key(Word index) const { return keys_[index]; }

void read_native_gui_icon_properties_storage_00ab3310(void* actual_icon138,
    volatile Word& actual_visitor_home, NativeGuiIconPropertyReaderStorageFrame& f,
    NativeGuiIconPropertyReaderStorageContext& c) {
    void* const icon = actual_icon138;
    void* const visitor = pointer(get(&actual_visitor_home));
    Word* const key = f.calls.key_pair_18;
    Word* const four = f.calls.actual_homes_10;
    void* const field = four;
    void* const fallback = four + 2;
    auto metadata = [&](void* pair) { put(&f.metadata_10, 0, bits(pair)); };
    auto read_default = [&] {
        NativeLuaFieldResolverAdapter::Activation active(c.resolvers, f.resolver);
        read_native_lua_field_or_default_00bd68d0(c.reader, get(key), get(key, 4),
            field, fallback, f.field, c.fields);
    };
    auto default_key = [&](Word index) {
        put(key, 0, 0); put(key, 4, bits(c.literals.key(index)));
        current(visitor, 0x0c, f); metadata(key); read_default();
    };
    auto query = [&] {
        current(visitor, 0x14, f); metadata(fallback);
        return native_lua_reader_has_key_00bd5eb0(c.reader, f.query,
            get(fallback), get(fallback, 4));
    };
    auto enter = [&] {
        current(visitor, 4, f); metadata(fallback);
        c.reader.enter_key_00bd8e20(f.enter, get(fallback), get(fallback, 4));
    };
    auto leave = [&] { current(visitor, 8, f); c.reader.pop_back_00bd7130(); };
    auto coordinate = [&](Word index) {
        put(fallback, 0, 2); metadata(fallback);
        put(fallback, 4, bits(f.uv_2c + index - 1));
        put(field, 0, 1); put(field, 4, index);
        current(visitor, 0x10, f); metadata(field);
        NativeLuaFieldResolverAdapter::Activation active(c.resolvers, f.resolver);
        read_native_lua_field_00bd6830(c.reader, get(field), get(field, 4),
            fallback, f.field, c.fields);
    };
    put(&f.state_44, 0, 0xffffffffu);
    read_native_gui_widget_properties_and_children_00aaa710(icon, visitor, f.base,
        c.base_interaction, c.base_children, f.base_children);

    put(fallback, 0, 3); metadata(fallback); byte(fallback, 4, 0);
    put(field, 0, 3); metadata(field); put(field, 4, bits(at(icon, 0x10c))); default_key(0);
    put(fallback, 0, 3); byte(fallback, 4, 1); metadata(fallback);
    put(field, 0, 3); metadata(field); put(field, 4, bits(at(icon, 0x100))); default_key(1);
    put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(12))); metadata(fallback);
    put(field, 0, 0); metadata(field); put(field, 4, bits(at(icon, 0x104))); default_key(2);
    metadata(fallback); put(fallback, 0, 1); put(fallback, 4, 0);
    metadata(field); put(field, 0, 1); put(field, 4, bits(&actual_visitor_home)); default_key(3);
    volatile Word* const visitor_home = &actual_visitor_home;
    const volatile float* const one_address = &c.base_interaction.properties.one_00d7a24c;
    __asm {
        mov eax, visitor_home
        mov ecx, [eax]
        mov eax, one_address
        movss xmm0, [eax]
        mov edx, icon
        mov [edx + 110h], ecx
        mov eax, fallback
        mov dword ptr [eax], 2
        movss [eax + 4], xmm0
    }
    metadata(fallback);
    metadata(field); put(field, 0, 2); put(field, 4, bits(at(icon, 0x114))); default_key(4);
    put(fallback, 0, 3); byte(fallback, 4, 0); metadata(fallback);
    put(field, 0, 3); metadata(field); put(field, 4, bits(at(icon, 0x101))); default_key(5);
    __asm {
        mov eax, fallback
        xorps xmm0, xmm0
        mov dword ptr [eax], 2
        movss [eax + 4], xmm0
    }
    metadata(fallback);
    put(field, 0, 2); metadata(field); put(field, 4, bits(at(icon, 0x130))); default_key(6);
    put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(7)));
    if (!query()) return;
    put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(7))); enter();
    Word ordinal = 1;
    for (;;) {
        put(fallback, 0, 1); put(fallback, 4, ordinal);
        if (!query()) { leave(); return; }
        put(fallback, 0, 1); put(fallback, 4, ordinal); enter();
        put(f.name_14, 0, 0); put(f.name_14, 4, 0);
        put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(12))); metadata(fallback);
        put(field, 0, 0); metadata(field); put(field, 4, bits(f.name_14));
        put(key, 0, 0); put(key, 4, bits(c.literals.key(8)));
        current(visitor, 0x0c, f);
        put(&f.state_44, 0, 0); // Native35EF, BEFORE Texture construction.
        void* captured_data;
        try {
            metadata(key); read_default();
            put(fallback, 0, 6); metadata(fallback); put(fallback, 4, bits(at(icon, 0x20)));
            metadata(field); put(field, 0, 6); put(field, 4, bits(f.size_1c)); default_key(9);
            put(fallback, 0, 6); metadata(fallback); put(fallback, 4, bits(at(icon, 0x18)));
            metadata(field); put(field, 0, 6); put(field, 4, bits(f.pivot_24)); default_key(10);
            Word* const uv = f.uv_2c;
            __asm {
                mov edx, uv
                xorps xmm0, xmm0
                movss [edx], xmm0
                movss [edx + 4], xmm0
                mov eax, one_address
                movss xmm0, [eax]
                movss [edx + 8], xmm0
                movss [edx + 0ch], xmm0
            }
            put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(11)));
            if (query()) {
                put(fallback, 0, 0); put(fallback, 4, bits(c.literals.key(11))); enter();
                coordinate(1); coordinate(2); coordinate(3); coordinate(4); leave();
            }
            auto& invocation = c.states.prepare();
            auto& child_frame = c.states.preparation_frame(invocation);
            put(&child_frame.append.local_08[0], 0, c.state_private_preimages[0]);
            put(&child_frame.append.growth_frame.scratch_18, 0, c.state_private_preimages[1]);
            put(&child_frame.append.growth_frame.repeat_tag_home, 0, c.state_private_preimages[2]);
            put(&child_frame.append.growth_frame.range_tag_home, 0, c.state_private_preimages[3]);
            current(icon, 0xa4, f);
            // Native PUSH order size,pivot,UV,name; SAME real four-word array.
            put(four, 12, bits(f.size_1c)); put(four, 8, bits(f.pivot_24));
            put(four, 4, bits(f.uv_2c)); put(four, 0, bits(f.name_14));
            (void)c.states.invoke_00ab66c0(invocation, f.calls.actual_homes_10);
            leave();
            captured_data = pointer(get(f.name_14, 4));
            put(&f.state_44, 0, 0xffffffffu);
        } catch (...) {
            return_native_string_header_buffer_0041dd20_exact(f.name_14, c.fields.strings);
            throw;
        }
        // Normal disarmed footer: captureDATA already occurred; currentLEN
        // is read BEFORE the actual returning getter, no catch/replay around it.
        if (captured_data) {
            const Word bytes = get(f.name_14) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                c.fields.strings.actual_published_01090aa8,
                c.fields.strings.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, captured_data, bytes,
                c.fields.strings.actual_small_returns_disabled_01090aa4);
        }
        ++ordinal;
    }
}
} // namespace bsp
