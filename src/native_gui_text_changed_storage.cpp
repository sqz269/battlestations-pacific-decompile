#include "bsp/native_gui_text_changed_storage.hpp"
#include "bsp/native_gui_text_glyph_children.hpp"
#include "bsp/native_gui_text_main_section_storage.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/native_gui_text_glyph_buffers_storage.hpp"
#include "bsp/native_gui_text_single_line_storage.hpp"
#include "bsp/native_mesh_section_layout_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_locale_uppercase.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"
#include <cwchar>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4 && sizeof(wchar_t) == 2);
Word bits(const volatile void* p) { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(Word p) { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
Word load(Word p) { return *reinterpret_cast<const volatile Word*>(static_cast<std::uintptr_t>(p)); }
void store(Word p, Word v) { *reinterpret_cast<volatile Word*>(static_cast<std::uintptr_t>(p)) = v; }
unsigned char byte(Word p) { return *reinterpret_cast<const volatile unsigned char*>(static_cast<std::uintptr_t>(p)); }
void put_byte(Word p, unsigned char v) { *reinterpret_cast<volatile unsigned char*>(static_cast<std::uintptr_t>(p)) = v; }
std::int16_t signed_half(Word p) {
    return static_cast<std::int16_t>(*reinterpret_cast<const volatile std::uint16_t*>(static_cast<std::uintptr_t>(p)));
}
const void* authenticated(const game::GameNativeReadOnlyData& data, Word va, Word size, Word expected) {
    const void* p = data.data_at(va, size);
    auto* bytes = static_cast<const unsigned char*>(p);
    Word hash = 2166136261u;
    for (Word i = 0; i != size; ++i) hash = (hash ^ bytes[i]) * 16777619u;
    if (hash != expected) throw std::logic_error("changed Text literal preimage mismatch");
    return p;
}
void current_text50(Word text, NativeGuiTextChangedStorageContext& c,
    NativeGuiTextFailureOrigin& origin, Word site) {
    const Word profile = load(text);
    const volatile Word* table = origin.host(c.failure_runtime, {0, __COUNTER__}, &c.calls,
        [&] { return c.calls.current_table(pointer(text), profile); });
    const Word target = table[0x50 / 4];
    origin.unknown(c.failure_runtime, {site, __COUNTER__}, &c.calls,
        [&] { c.calls.text_current50(pointer(text), profile, target, pointer(text + 0x50u)); });
}
void parameter(Word material, Word target, const char* name, Word value,
    NativeGuiTextChangedStorageContext& c, NativeGuiTextContentStorageScratch& s,
    unsigned slot, Word site) {
    auto& origin = s.parameter_origins[slot];
    auto& arm = s.parameter_arms[slot];
    origin.begin(c.failure_runtime, &origin, 0);
    try {
        void* const header = s.words + 4;
        origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings,
            [&] { construct_native_string_header_0041e870(header, c.raw_strings, name); });
        arm.arm(origin, header, slot + 1);
        try {
            origin.unknown(c.failure_runtime, {site, __COUNTER__}, &c.calls, [&] {
                if (target == 0x00b18b00u) c.calls.parameter2_00b18b00(pointer(material), header, pointer(value));
                else if (target == 0x00b18aa0u) c.calls.parameter4_00b18aa0(pointer(material), header, pointer(value));
                else c.calls.parameter1_00b18b20(pointer(material), header, pointer(value));
            });
        } catch (...) {
            origin.retain_current(c.failure_runtime);
            const auto cause = c.failure_runtime.read_current_catch();
            if (arm.consume_proven_primary(origin, cause, header, slot + 1)) {
                try {
                    origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings, [&] {
                        destroy_native_string_header_0041dd20(header, c.raw_strings);
                    }, NativeGuiTextFailureOrigin::Stage::cleanup_secondary);
                    arm.returned();
                } catch (...) { arm.return_failed(); throw; }
            }
            throw;
        }
        arm.normal_return_entered();
        try {
            origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings,
                [&] { destroy_native_string_header_0041dd20(header, c.raw_strings); });
            arm.returned();
        } catch (...) { arm.return_failed(); throw; }
        origin.complete();
    } catch (...) { origin.retain_current(c.failure_runtime); throw; }
}
} // namespace

NativeGuiTextChangedStorageData::NativeGuiTextChangedStorageData(
    const game::GameNativeReadOnlyData& owner, const std::array<const void*, 7>& views)
    : owner_(owner), views_(views) {}
const char* NativeGuiTextChangedStorageData::parameter_name(std::size_t index) const {
    return static_cast<const char*>(views_[index]);
}
const double* NativeGuiTextChangedStorageData::height_divisor_00cef1b8() const {
    return static_cast<const double*>(views_[4]);
}
const volatile Word& NativeGuiTextChangedStorageData::depth_00ce3800() const {
    return *static_cast<const volatile Word*>(views_[5]);
}
const volatile Word& NativeGuiTextChangedStorageData::depth_00ce69d0() const {
    return *static_cast<const volatile Word*>(views_[6]);
}
NativeGuiTextChangedStorageData borrow_native_gui_text_changed_data(const game::GameNativeReadOnlyData& data) {
    return NativeGuiTextChangedStorageData(data, {{
        authenticated(data, 0x00d5c7b0u, 20, 0xc37d358bu),
        authenticated(data, 0x00d5c470u, 10, 0x02148b8du),
        authenticated(data, 0x00d5c464u, 11, 0x94ebd5e7u),
        authenticated(data, 0x00d5c454u, 13, 0xc53cbf16u),
        authenticated(data, 0x00cef1b8u, 8, 0x0473bb53u),
        authenticated(data, 0x00ce3800u, 4, 0x1c95ab18u),
        authenticated(data, 0x00ce69d0u, 4, 0x9c94e198u)}});
}

static __declspec(noinline) void update_native_gui_text_content_storage_00aba8d0_body(
    void* captured_actual_text, volatile Word& actual_utf16_dword_home,
    NativeGuiTextChangedStorageContext& c, NativeGuiTextContentStorageScratch& s) {
    const Word text = bits(captured_actual_text), scratch = bits(s.words);
    const auto& sections = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.sections_invocation(captured_actual_text, s); });
    s.failure_origin.child(c.failure_runtime, {0x00aba8ecu, __COUNTER__}, &sections.frame, 0x00ab8530u,
        sections.frame.failure_origin(), [&] { ensure_native_gui_text_post_constructor_sections_storage_00ab8530(
        sections.reference, sections.context, sections.frame, sections.acquired, c.failure_runtime); });
    const Word source = actual_utf16_dword_home; // NativeABA8F1, AFTER ensure.
    s.failure_origin.unknown(c.failure_runtime, {0x00aba8fau, __COUNTER__}, &c.wide_strings, [&]() -> decltype(auto) { return copy_construct_native_wide_string_header_004c8dd0(s.words + 2, pointer(source), c.wide_strings); });
    // Native state0 arms only AFTER copy returns; all later work is covered.
    s.copied_wide_arm.arm(s.failure_origin, s.words + 2, 0);
    try {
        const Word font = load(text + 0x108u);
        if (byte(font + 0x48u) != 0) {
            void* const locale = c.actual_locale_00f8bc4c;
            s.failure_origin.unknown(c.failure_runtime, {0x00aba91cu, __COUNTER__}, &c.failure_runtime, [&]() -> decltype(auto) { return uppercase_native_locale_wide_header_00a9ec30(locale, s.words + 2); });
        }
        Word changed;
        if (load(text + 0xecu) == 0) changed = load(scratch + 8u) != 0 ? 0xffffffffu : 0;
        else if (load(scratch + 8u) == 0) changed = 1;
        else {
            const Word input = load(scratch + 12u);
            const Word previous = load(text + 0xf0u);
            changed = static_cast<Word>(s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.failure_runtime, [&]() -> decltype(auto) { return _wcsicmp(static_cast<const wchar_t*>(pointer(previous)),
                static_cast<const wchar_t*>(pointer(input))); }));
        }
        if (changed != 0) {
            store(text + 0x114u, 0);
            const auto& glyph_children = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.glyph_children_invocation(captured_actual_text, s); });
            s.failure_origin.unknown(c.failure_runtime, {0x00aba964u, __COUNTER__}, &glyph_children.acquired, [&]() -> decltype(auto) { return clear_native_gui_text_glyph_children_00ab80c0(
                captured_actual_text, glyph_children.context, glyph_children.acquired); });
            void* const mesh = s.failure_origin.unknown(c.failure_runtime, {0x00aba96du, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.geometry_00b74640(pointer(load(text + 0x4cu)), 0, s); });
            store(scratch, bits(mesh)); // Native saved mesh BEFORE section getter.
            void* const section = s.failure_origin.unknown(c.failure_runtime, {0x00aba97bu, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.section_00b732c0(mesh, 0); });
            const Word main_section = bits(section);
            s.failure_origin.unknown(c.failure_runtime, {0x00aba98du, __COUNTER__}, &c.wide_strings, [&]() -> decltype(auto) { return assign_native_wide_string_header_004c5e20(pointer(text + 0xecu), s.words + 2, c.wide_strings); });
            const Word length = load(scratch + 8u);
            if (length == 0) {
                store(main_section + 0x18u, 0);
                store(main_section + 0x10u, 0);
                void* const shadow = s.failure_origin.unknown(c.failure_runtime, {0x00aba9a7u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.geometry_00b74640(pointer(load(text + 0x188u)), 0, s); });
                void* const shadow_section = s.failure_origin.unknown(c.failure_runtime, {0x00aba9afu, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.section_00b732c0(shadow, 0); });
                store(bits(shadow_section) + 0x18u, 0);
                store(bits(shadow_section) + 0x10u, 0);
            } else {
                store(bits(s.buffer_homes) + 4u, bits(mesh)); // PUSH mesh, then capacity.
                store(bits(s.buffer_homes), length);
                auto& glyph_buffers = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.glyph_buffers_frame(captured_actual_text, s.buffer_homes, s); });
                s.failure_origin.unknown(c.failure_runtime, {0x00aba9c3u, __COUNTER__}, &glyph_buffers, [&]() -> decltype(auto) { return create_native_gui_text_glyph_buffers_storage_00ab8400(glyph_buffers); });
                Word material = load(main_section + 0x20u); // AFTER buffer factory.
                store(main_section + 8u, 4);
                store(main_section + 0xcu, 0);
                store(main_section + 0x14u, 0);
                store(main_section + 0x10u, 0);
                store(main_section + 0x18u, 0);
                store(scratch + 4u, material);
                const auto& shader_call = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.shader_invocation(captured_actual_text, s); });
                const unsigned char shader = s.failure_origin.unknown(c.failure_runtime, {0x00aba9e4u, __COUNTER__}, &shader_call.frame, [&]() -> decltype(auto) { return ensure_native_gui_text_font_shader_storage_00ab8ce0(
                    shader_call.reference, shader_call.context, shader_call.frame); });
                put_byte(bits(&actual_utf16_dword_home), shader);
                if (shader != 0) {
                    s.failure_origin.unknown(c.failure_runtime, {0x00aba9feu, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.shader_00b19210(pointer(material), pointer(load(text + 0x1ecu))); });
                    __asm {
                        mov eax, text
                        fld dword ptr [eax + 94h]
                        fstp dword ptr [eax + 1dch]
                        fld dword ptr [eax + 1d4h]
                        fstp dword ptr [eax + 1e0h]
                    }
                    s.failure_origin.child(c.failure_runtime, {0x00abaa3cu, __COUNTER__}, &s.parameter_origins[0], 0, s.parameter_origins[0], [&] { parameter(material, 0x00b18b00u, c.data.parameter_name(0), text + 0x1dcu, c, s, 0u, 0x00abaa3cu); });
                    s.failure_origin.child(c.failure_runtime, {0x00abaa86u, __COUNTER__}, &s.parameter_origins[1], 0, s.parameter_origins[1], [&] { parameter(material, 0x00b18aa0u, c.data.parameter_name(1), text + 0xa4u, c, s, 1u, 0x00abaa86u); });
                    s.failure_origin.child(c.failure_runtime, {0x00abaad0u, __COUNTER__}, &s.parameter_origins[2], 0, s.parameter_origins[2], [&] { parameter(material, 0x00b18aa0u, c.data.parameter_name(2), text + 0xb4u, c, s, 2u, 0x00abaad0u); });
                    s.failure_origin.child(c.failure_runtime, {0x00abab1au, __COUNTER__}, &s.parameter_origins[3], 0, s.parameter_origins[3], [&] { parameter(material, 0x00b18b20u, c.data.parameter_name(3), text + 0xc4u, c, s, 3u, 0x00abab1au); });
                    s.failure_origin.unknown(c.failure_runtime, {0x00abab46u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.clip_00aa9f10(captured_actual_text, pointer(material)); });
                }
                store(bits(&s.layout_mesh_home), load(scratch));
                const auto& main_layout = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.layout_invocation(section, s.layout_mesh_home, s); });
                s.failure_origin.unknown(c.failure_runtime, {0x00abab54u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return rebuild_native_mesh_section_vertex_layout_storage_00b865a0(section,
                    s.layout_mesh_home, main_layout.actual_key_home, main_layout.context,
                    main_layout.acquired_before_insertion); });
                const unsigned char wrapped = byte(text + 0xfcu);
                store(bits(s.builder_homes) + 4u, main_section);
                store(bits(s.builder_homes), scratch + 8u);
                if (wrapped != 0) s.failure_origin.unknown(c.failure_runtime, {0x00abab6au, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.wrapped_00aba270(captured_actual_text, s.builder_homes, s); });
                else {
                    auto& single_line = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.single_line_frame(captured_actual_text, s.builder_homes, s); });
                    s.failure_origin.unknown(c.failure_runtime, {0x00abab78u, __COUNTER__}, &single_line, [&]() -> decltype(auto) { return build_native_gui_text_single_line_storage_00ab9fd0(single_line); });
                }
                current_text50(text, c, s.failure_origin, 0x00abab88u);
                void* const shadow = s.failure_origin.unknown(c.failure_runtime, {0x00abab91u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.geometry_00b74640(pointer(load(text + 0x188u)), 0, s); });
                void* const shadow_section = s.failure_origin.unknown(c.failure_runtime, {0x00abab9bu, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.section_00b732c0(shadow, 0); });
                const Word shadow_slot = bits(shadow_section);
                void* const stream = native_mesh_vertex_stream_unchecked_00b73260(pointer(load(scratch)), 0);
                s.failure_origin.unknown(c.failure_runtime, {0x00ababb2u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.vertex_00b73bb0(shadow, 0, stream); });
                s.failure_origin.unknown(c.failure_runtime, {0x00ababc1u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.index_00b73b70(shadow, pointer(load(load(scratch) + 0x60u))); });
                store(shadow_slot + 8u, load(main_section + 8u));
                store(shadow_slot + 0xcu, load(main_section + 0xcu));
                store(shadow_slot + 0x14u, load(main_section + 0x14u));
                store(shadow_slot + 0x10u, load(main_section + 0x10u));
                store(shadow_slot + 0x18u, load(main_section + 0x18u));
                store(bits(&s.layout_mesh_home), bits(shadow));
                const auto& shadow_layout = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.layout_invocation(shadow_section, s.layout_mesh_home, s); });
                s.failure_origin.unknown(c.failure_runtime, {0x00ababe7u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return rebuild_native_mesh_section_vertex_layout_storage_00b865a0(shadow_section,
                    s.layout_mesh_home, shadow_layout.actual_key_home, shadow_layout.context,
                    shadow_layout.acquired_before_insertion); });
                material = load(scratch + 4u); // Native saved main material AFTER layout.
                const std::int16_t slots = signed_half(material + 0x34u);
                const Word shadow_material = load(shadow_slot + 0x20u);
                const Word texture = slots > 0 ? load(material + 0x10u) : 0;
                s.failure_origin.unknown(c.failure_runtime, {0x00abac06u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.texture_00b189f0(pointer(shadow_material), 0, pointer(texture)); });
                if (byte(bits(&actual_utf16_dword_home)) != 0) {
                    const Word current_shader = load(material + 0x7cu);
                    s.failure_origin.unknown(c.failure_runtime, {0x00abac1cu, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.shader_00b19210(pointer(shadow_material), pointer(current_shader)); });
                    s.failure_origin.unknown(c.failure_runtime, {0x00abac24u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.clip_00aa9f10(captured_actual_text, pointer(shadow_material)); });
                    s.failure_origin.child(c.failure_runtime, {0x00abac48u, __COUNTER__}, &s.parameter_origins[4], 0, s.parameter_origins[4], [&] { parameter(shadow_material, 0x00b18b20u, c.data.parameter_name(3), bits(c.actual_zero_parameter_00f8be54), c, s, 4u, 0x00abac48u); });
                    s.failure_origin.child(c.failure_runtime, {0x00abac92u, __COUNTER__}, &s.parameter_origins[5], 0, s.parameter_origins[5], [&] { parameter(shadow_material, 0x00b18b00u, c.data.parameter_name(0), text + 0x1dcu, c, s, 5u, 0x00abac92u); });
                    s.failure_origin.child(c.failure_runtime, {0x00abacdcu, __COUNTER__}, &s.parameter_origins[6], 0, s.parameter_origins[6], [&] { parameter(shadow_material, 0x00b18aa0u, c.data.parameter_name(1), text + 0xa4u, c, s, 6u, 0x00abacdcu); });
                    s.failure_origin.child(c.failure_runtime, {0x00abad26u, __COUNTER__}, &s.parameter_origins[7], 0, s.parameter_origins[7], [&] { parameter(shadow_material, 0x00b18aa0u, c.data.parameter_name(2), text + 0xb4u, c, s, 7u, 0x00abad26u); });
                    s.failure_origin.child(c.failure_runtime, {0x00abad70u, __COUNTER__}, &s.parameter_origins[8], 0, s.parameter_origins[8], [&] { parameter(shadow_material, 0x00b18b20u, c.data.parameter_name(3), text + 0xc4u, c, s, 8u, 0x00abad70u); });
                }
                void* const colour_mesh = s.failure_origin.unknown(c.failure_runtime, {0x00abada3u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.geometry_00b74640(pointer(load(text + 0x188u)), 0, s); });
                void* const colour_section = s.failure_origin.unknown(c.failure_runtime, {0x00abadaau, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.section_00b732c0(colour_mesh, 0); });
                void* const colour = s.failure_origin.unknown(c.failure_runtime, {0x00abadb6u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.diffuse_00b179f0(pointer(load(bits(colour_section) + 0x20u)), 0); });
                store(bits(colour), load(text + 0x168u));
                store(bits(colour) + 4u, load(text + 0x16cu));
                store(bits(colour) + 8u, load(text + 0x170u));
                store(bits(colour) + 12u, load(text + 0x174u));
                const Word current_font = load(text + 0x108u);
                actual_utf16_dword_home = current_font != 0
                    ? static_cast<Word>(static_cast<std::int32_t>(signed_half(current_font + 0x14u))) : 0;
                const Word depth_mode = load(text + 0x160u); // CMP BEFORE x87 group.
                volatile Word* const height_home = &actual_utf16_dword_home;
                const double* const divisor = c.data.height_divisor_00cef1b8();
                __asm {
                    mov edx, height_home
                    fild dword ptr [edx]
                    mov eax, divisor
                    fdiv qword ptr [eax]
                    mov eax, text
                    fmul dword ptr [eax + 164h]
                    fstp dword ptr [edx]
                }
                const Word depth = depth_mode == 0 ? c.data.depth_00ce3800() : c.data.depth_00ce69d0();
                const Word scale = actual_utf16_dword_home;
                store(scratch + 16u, scale);
                store(scratch + 20u, scale);
                store(scratch + 24u, depth);
                void* const model = pointer(load(text + 0x188u));
                const Word profile = load(bits(model));
                const volatile Word* table = s.failure_origin.host(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.current_table(model, profile); });
                const Word target = table[0x2c / 4];
                s.failure_origin.unknown(c.failure_runtime, {0x00000000u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.model_current2c(model, profile, target, s.words + 4); });
                const Word parent = byte(text + 0x15cu) != 0 ? load(text + 0x4cu) : 0;
                s.failure_origin.unknown(c.failure_runtime, {0x00abae6du, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.parent_00b6e680(pointer(load(text + 0x188u)), pointer(parent)); });
                if (byte(text + 0x15cu) == 0)
                    s.failure_origin.unknown(c.failure_runtime, {0x00abae83u, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.root_00b6d890(pointer(load(text + 0x188u)), nullptr); });
            }
        }
    } catch (...) {
        s.failure_origin.retain_current(c.failure_runtime);
        const auto cause = c.failure_runtime.read_current_catch();
        if (s.copied_wide_arm.consume_proven_primary(s.failure_origin, cause, s.words + 2, 0)) {
            try {
                s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings, [&] {
                    destroy_native_wide_string_header_00436430(s.words + 2, c.raw_strings);
                }, NativeGuiTextFailureOrigin::Stage::cleanup_secondary);
                s.copied_wide_arm.returned();
            } catch (...) { s.copied_wide_arm.return_failed(); throw; }
        }
        throw;
    }
    s.copied_wide_arm.normal_return_entered();
    try {
        s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings,
            [&] { destroy_native_wide_string_header_00436430(s.words + 2, c.raw_strings); });
        s.copied_wide_arm.returned();
    } catch (...) { s.copied_wide_arm.return_failed(); throw; }
}

static __declspec(noinline) void submit_native_gui_text_changed_source_storage_00abaed0_body(
    void* captured_actual_text, const void* captured_narrow_header8,
    const volatile Word& actual_flag_dword_home, NativeGuiTextChangedStorageContext& c,
    NativeGuiTextChangedSourceStorageScratch& s) {
    const Word text = bits(captured_actual_text), input = bits(captured_narrow_header8);
    const Word cached = text + 0xf4u;
    if (s.failure_origin.unknown(c.failure_runtime, {0x00abaefau, __COUNTER__}, &c.raw_strings, [&]() -> decltype(auto) { return equal_native_string_headers_00435c40(pointer(cached), captured_narrow_header8); })) return;
    if (cached != input) {
        const Word length = load(input);
        s.failure_origin.unknown(c.failure_runtime, {0x00abaf12u, __COUNTER__}, &c.raw_strings, [&]() -> decltype(auto) { return resize_native_string_header_0041dd40(pointer(cached), c.raw_strings, length, true); });
        if (load(input) != 0) {
            const Word bytes = load(cached);
            const Word destination = load(cached + 4u);
            const Word source = load(input + 4u);
            s.failure_origin.unknown(c.failure_runtime, {0x00abaf27u, __COUNTER__}, &c.failure_runtime, [&]() -> decltype(auto) { return copy_native_crt_bytes_00bf7680(pointer(destination), pointer(source), bytes, c.actual_feature_word_0109eea4); });
        }
    }
    // Native reads only flag BYTE0 AFTER all reached cache resize/copy.
    const unsigned char localise = *reinterpret_cast<const volatile unsigned char*>(&actual_flag_dword_home);
    if (localise != 0) {
        store(bits(s.wide_headers), 0);
        store(bits(s.wide_headers) + 4u, 0);
        void* const locale = c.actual_locale_00f8bc4c;
        s.localized_arm.arm(s.failure_origin, s.wide_headers, 0);
        try { // Native state0 BEFORE resolver, unlike the widening branch.
            s.failure_origin.unknown(c.failure_runtime, {0x00abaf5au, __COUNTER__}, &c.calls, [&]() -> decltype(auto) { return c.calls.localise_00a9fad0(locale, s.wide_headers, captured_narrow_header8); });
            store(bits(&s.inner_utf16_home), bits(s.wide_headers));
            s.failure_origin.child(c.failure_runtime, {0, __COUNTER__}, &s.inner, 0x00aba8d0u, s.inner.failure_origin, [&] { update_native_gui_text_content_storage_00aba8d0(captured_actual_text, s.inner_utf16_home, c, s.inner); });
        } catch (...) {
            s.failure_origin.retain_current(c.failure_runtime);
            const auto cause = c.failure_runtime.read_current_catch();
            if (s.localized_arm.consume_proven_primary(s.failure_origin, cause, s.wide_headers, 0)) {
                try {
                    s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings, [&] {
                        destroy_native_wide_string_header_00436430(s.wide_headers, c.raw_strings);
                    }, NativeGuiTextFailureOrigin::Stage::cleanup_secondary);
                    s.localized_arm.returned();
                } catch (...) { s.localized_arm.return_failed(); throw; }
            }
            throw;
        }
        s.localized_arm.normal_return_entered();
        try {
            s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings,
                [&] { destroy_native_wide_string_header_00436430(s.wide_headers, c.raw_strings); });
            s.localized_arm.returned();
        } catch (...) { s.localized_arm.return_failed(); throw; }
    } else {
        const Word current_data = load(input + 4u);
        const char* const source = current_data != 0 ? static_cast<const char*>(pointer(current_data)) : c.actual_empty_narrow_00f8be4c;
        void* const result = s.failure_origin.unknown(c.failure_runtime, {0x00abaf9au, __COUNTER__}, &c.wide_strings, [&]() -> decltype(auto) { return construct_native_wide_string_header_004c5e60(s.wide_headers + 2, source, c.wide_strings); });
        s.widened_arm.arm(s.failure_origin, s.wide_headers + 2, 1);
        store(bits(&s.inner_utf16_home), bits(result));
        try { // state1 only AFTER widening returns; use its actual returned header.
            s.failure_origin.child(c.failure_runtime, {0, __COUNTER__}, &s.inner, 0x00aba8d0u, s.inner.failure_origin, [&] { update_native_gui_text_content_storage_00aba8d0(captured_actual_text, s.inner_utf16_home, c, s.inner); });
        } catch (...) {
            s.failure_origin.retain_current(c.failure_runtime);
            const auto cause = c.failure_runtime.read_current_catch();
            if (s.widened_arm.consume_proven_primary(s.failure_origin, cause, s.wide_headers + 2, 1)) {
                try {
                    s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings, [&] {
                        destroy_native_wide_string_header_00436430(s.wide_headers + 2, c.raw_strings);
                    }, NativeGuiTextFailureOrigin::Stage::cleanup_secondary);
                    s.widened_arm.returned();
                } catch (...) { s.widened_arm.return_failed(); throw; }
            }
            throw;
        }
        s.widened_arm.normal_return_entered();
        try {
            s.failure_origin.unknown(c.failure_runtime, {0, __COUNTER__}, &c.raw_strings,
                [&] { destroy_native_wide_string_header_00436430(s.wide_headers + 2, c.raw_strings); });
            s.widened_arm.returned();
        } catch (...) { s.widened_arm.return_failed(); throw; }
    }
    current_text50(text, c, s.failure_origin, 0x00abafe3u); // AFTER normal return; separate from inner current50.
}
__declspec(noinline) void update_native_gui_text_content_storage_00aba8d0(
    void* captured_actual_text, volatile std::uint32_t& actual_utf16_dword_home,
    NativeGuiTextChangedStorageContext& c, NativeGuiTextContentStorageScratch& s) {
    s.failure_origin.begin(c.failure_runtime, &s, 0x00aba8d0u);
    try {
        update_native_gui_text_content_storage_00aba8d0_body(captured_actual_text, actual_utf16_dword_home, c, s);
        s.failure_origin.complete();
    } catch (...) { s.failure_origin.retain_current(c.failure_runtime); throw; }
}
__declspec(noinline) void submit_native_gui_text_changed_source_storage_00abaed0(
    void* captured_actual_text, const void* captured_narrow_header8,
    const volatile std::uint32_t& actual_flag_dword_home,
    NativeGuiTextChangedStorageContext& c, NativeGuiTextChangedSourceStorageScratch& s) {
    s.failure_origin.begin(c.failure_runtime, &s, 0x00abaed0u);
    try {
        submit_native_gui_text_changed_source_storage_00abaed0_body(captured_actual_text,
            captured_narrow_header8, actual_flag_dword_home, c, s);
        s.failure_origin.complete();
    } catch (...) { s.failure_origin.retain_current(c.failure_runtime); throw; }
}
} // namespace bsp
