#include "bsp/native_gui_icon_rebuild_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_crt_memcpy.hpp"
#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_mesh_buffer_fields.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_remaining_fields.hpp"
#include "bsp/native_mesh_subset_loading.hpp"
#include "bsp/native_render_context.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_raw_resize_exact.hpp"
#include "bsp/native_vertex_declaration_cache.hpp"
#include "bsp/native_vertex_declaration_loading.hpp"
#include "bsp/native_gui_widget_color_storage.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Icon current80 storage requires MSVC Win32.
#endif
namespace bsp {
namespace {
using Word = std::uint32_t;
using Byte = std::uint8_t;
using Frame = NativeGuiIconRebuildStorageFrame;
using Context = NativeGuiIconRebuildStorageContext;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* p) { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* address(Word p) { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(p)); }
void* at(const void* p, Word offset) { return address(bits(p) + offset); }
volatile Word& word(void* p, Word offset = 0) { return *static_cast<volatile Word*>(at(p, offset)); }
Word load(const void* p, Word offset = 0) { return *static_cast<const volatile Word*>(at(p, offset)); }
void* pointer(const void* p, Word offset = 0) { return address(load(p, offset)); }
volatile Byte& byte(void* p, Word offset = 0) { return *static_cast<volatile Byte*>(at(p, offset)); }
volatile Word& home(Frame& f, Word native_offset) {
    return static_cast<volatile Word&>(f.native_words[(native_offset - 0x10u) / 4u]);
}
volatile Byte& local_byte(Frame& f, Word native_offset) {
    return byte(&f.native_words[0], native_offset - 0x10u);
}
void* local(Frame& f, Word native_offset) { return &f.native_words[(native_offset - 0x10u) / 4u]; }
void require(bool value, const char* message) { if (!value) throw std::logic_error(message); }
Word current_target(void* receiver, Word offset, Context& c) {
    const Word profile = load(receiver);
    const auto* const table_cell = static_cast<const volatile Word*>(
        c.clip.actual_read_only_data.data_at(profile + offset, sizeof(Word)));
    return *table_cell;
}
void require_target(Word captured, Word expected) {
    require(captured == expected, "Raw Icon current80 needs a genuine provider for this OTHER target");
}
// Native caller return sites capture DATA before the null test and LEN+1
// BEFORE the genuine getter. Do not substitute41DD20's different read epoch.
void return_name(void* header, void* captured_data, Frame& f, Context& c,
    Word getter_site, Word return_site) {
    if (captured_data) {
        const Word bytes = load(header) + 1u;
        f.native_site = getter_site;
        auto* const pool = native_string_pool_get_or_create_00419cc0(
            c.strings.actual_published_01090aa8, c.strings.actual_manager_publication_01090aa0);
        f.native_site = return_site;
        return_native_string_pool_00bd1510(pool, captured_data, bytes,
            c.strings.actual_small_returns_disabled_01090aa4);
    }
}
void release_creator(void* captured, NativeMeshSectionLayoutReferenceOperation operation,
    Context& c) {
    const long result = operation(reinterpret_cast<volatile long*>(at(captured, 4)));
    if (result == 0) {
        // Current table/slot are genuine numeric data, never a host codeword.
        (void)current_target(captured, 0, c);
        dispatch_native_render_actual_owner_zero(c.owners, captured);
    }
    // No captured payload/companion read after a possible terminal callback.
}
NativeMaterialStorage* create_material(const void* actual_name, Frame& f, Context& c) {
    auto* const result = create_native_material_from_effect_cache_00535320(actual_name,
        c.material_destruction.material_slots, c.owners, c.effects, f.material_creator);
    f.captured_material = result; // Actual completed creator before host admission.
    c.geometry.register_native_material_creator(f.material_creator,
        c.material_destruction, f.material_profile);
    return result;
}
// The two actual inline vector-getter regions, not a reordered caller that
// stages EC before capturing first. Returning invalid-parameter still reloads
// CURRENT first and uses the SAME full sign-extended captured index.
void* current_state(void* actual_icon) {
    Word captured_index;
    Word invalid;
    __asm {
        mov eax, actual_icon
        mov ecx, dword ptr [eax + 0f4h]
        movsx edx, word ptr [eax + 0ech]
        mov captured_index, edx
        mov invalid, 0
        test ecx, ecx
        jz state_invalid
        mov eax, dword ptr [eax + 0f8h]
        sub eax, ecx
        sar eax, 6
        cmp edx, eax
        jb state_valid
    state_invalid:
        mov invalid, 1
    state_valid:
    }
    if (invalid != 0) invoke_native_invalid_parameter_00bf6713();
    return address(load(actual_icon, 0xf4) + (captured_index << 6u));
}
void* copy_state_fields_and_uv(void* raw, void* state, Frame& f) {
    void* const uv = local(f, 0x34);
    void* const section_home = local(f, 0x14);
    void* returned_section;
    __asm {
        mov ecx, raw
        mov edx, state
        mov ebx, uv
        fld dword ptr [edx + 34h]
        fstp dword ptr [ecx + 20h]
        fld dword ptr [edx + 38h]
        fstp dword ptr [ecx + 24h]
        fld dword ptr [edx + 2ch]
        fstp dword ptr [ecx + 18h]
        fld dword ptr [edx + 30h]
        fstp dword ptr [ecx + 1ch]
        mov eax, dword ptr [edx + 0ch]
        mov dword ptr [ebx], eax
        mov eax, dword ptr [edx + 10h]
        mov dword ptr [ebx + 4], eax
        mov eax, dword ptr [edx + 14h]
        mov dword ptr [ebx + 8], eax
        mov eax, dword ptr [edx + 18h]
        movss xmm0, dword ptr [ebx + 8]
        mov ecx, section_home
        mov ecx, dword ptr [ecx]
        mov returned_section, ecx
        mov dword ptr [ebx + 12], eax
        movss dword ptr [ebx + 8], xmm0
        movss xmm0, dword ptr [ebx + 12]
        movss dword ptr [ebx + 12], xmm0
    }
    return returned_section;
}
void setup_quad_size(void* raw, void* state, Word (&arguments)[7]) {
    auto* const homes = &arguments[0];
    __asm {
        mov ecx, raw
        mov edx, state
        mov ebx, homes
        fld dword ptr [edx + 2ch]
        lea eax, [edx + 34h]
        fstp dword ptr [ecx + 18h]
        mov dword ptr [ebx + 24], eax
        fld dword ptr [edx + 30h]
        fstp dword ptr [ecx + 1ch]
        fld dword ptr [edx + 34h]
        fstp dword ptr [ecx + 20h]
        fld dword ptr [edx + 38h]
        fstp dword ptr [ecx + 24h]
    }
}
// Four literal parameter sequences: PUSH preserve1/request, header LEN0/DATA0,
// exactresize, CURRENT DATA then CURRENT LEN+1/BF EVEN0, actual registration,
// CURRENT DATA capture, state-1, current LEN+1 before raw getter/return.
void parameter(Frame& f, Context& c, NativeMaterialStorage& material,
    Word name_index, Word request, void* actual_source, Word native_state) {
    static constexpr Word resize_sites[]{0x00ab3f47, 0x00ab3fbe, 0x00ab4035, 0x00ab40ac};
    static constexpr Word copy_sites[]{0x00ab3f62, 0x00ab3fd9, 0x00ab4050, 0x00ab40c7};
    static constexpr Word register_sites[]{0x00ab3f80, 0x00ab3ff7, 0x00ab406e, 0x00ab40e5};
    static constexpr Word getter_sites[]{0x00ab3fa0, 0x00ab4017, 0x00ab408e, 0x00ab4105};
    static constexpr Word return_sites[]{0x00ab3fa7, 0x00ab401e, 0x00ab4095, 0x00ab410c};
    void* const header = local(f, 0x18);
    static_cast<volatile Word&>(f.resize_preserve_home) = 1;
    static_cast<volatile Word&>(f.resize_request_home) = request;
    word(header) = 0;
    word(header, 4) = 0;
    f.native_site = resize_sites[name_index];
    resize_native_string_header_0041dd40_exact(header, c.strings, request,
        &f.resize_preserve_home, c.actual_feature_0109eea4);
    void* const destination = pointer(header, 4);
    if (destination) {
        const Word count = load(header) + 1u;
        f.native_site = copy_sites[name_index];
        copy_native_crt_bytes_00bf7680(destination, f.parameter_names[name_index],
            count, c.actual_feature_0109eea4);
    }
    static_cast<volatile Word&>(f.native_state_home) = native_state;
    f.native_site = register_sites[name_index];
    if (name_index == 1u || name_index == 2u)
        register_native_material_float4_00b18aa0(material, header, actual_source, c.parameters);
    else
        register_native_material_float_00b18b20(material, header, actual_source, c.parameters);
    void* const data = pointer(header, 4);
    static_cast<volatile Word&>(f.native_state_home) = 0xffffffffu;
    return_name(header, data, f, c, getter_sites[name_index], return_sites[name_index]);
}
#pragma warning(push)
#pragma warning(disable: 4102)
void* write_white(Frame& f, void* stream, void* initial_section,
    const volatile float& actual_one) {
    void* const rgba = local(f, 0x18);
    const volatile float* const one_address = &actual_one;
    Word saved_ecx;
    void* final_section;
    __asm {
        mov ebx, stream
        mov esi, rgba
        mov edi, initial_section
    white_00ab42dc:
        mov ecx, dword ptr [ebx + 0x34]
    white_00ab42df:
        test ecx, ecx
    white_00ab42e1:
        mov saved_ecx, ecx
        mov ecx, one_address
        movss xmm0, dword ptr [ecx]
        mov ecx, saved_ecx
    white_00ab42e9:
        mov al, 0xff
    white_00ab42eb:
        mov byte ptr [esi + 2], al
    white_00ab42ef:
        mov byte ptr [esi + 1], al
    white_00ab42f3:
        mov byte ptr [esi], al
    white_00ab42f7:
        mov byte ptr [esi + 3], al
    white_00ab42fb:
        jl white_00ab430d
    white_00ab42fd:
        mov edi, dword ptr [esi]
    white_00ab4301:
        mov edx, dword ptr [ebx + 8]
    white_00ab4304:
        mov dword ptr [ecx + edx], edi
    white_00ab4307:
        mov edi, dword ptr [esi - 4]
    white_00ab430b:
        jmp white_00ab4330
    white_00ab430d:
        mov edx, dword ptr [ebx + 0x38]
    white_00ab4310:
        mov ecx, dword ptr [ebx + 8]
    white_00ab4313:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4318:
        mov edx, dword ptr [ebx + 0x3c]
    white_00ab431b:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4320:
        mov edx, dword ptr [ebx + 0x40]
    white_00ab4323:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4328:
        mov edx, dword ptr [ebx + 0x44]
    white_00ab432b:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4330:
        mov ecx, dword ptr [ebx + 0x34]
    white_00ab4333:
        test ecx, ecx
    white_00ab4335:
        mov byte ptr [esi + 2], al
    white_00ab4339:
        mov byte ptr [esi + 1], al
    white_00ab433d:
        mov byte ptr [esi], al
    white_00ab4341:
        mov byte ptr [esi + 3], al
    white_00ab4345:
        jl white_00ab435c
    white_00ab4347:
        mov edx, dword ptr [ebx + 0xc]
    white_00ab434a:
        mov edi, dword ptr [esi]
    white_00ab434e:
        add edx, ecx
    white_00ab4350:
        mov ecx, dword ptr [ebx + 8]
    white_00ab4353:
        mov dword ptr [edx + ecx], edi
    white_00ab4356:
        mov edi, dword ptr [esi - 4]
    white_00ab435a:
        jmp white_00ab4382
    white_00ab435c:
        mov ecx, dword ptr [ebx + 0xc]
    white_00ab435f:
        add ecx, dword ptr [ebx + 8]
    white_00ab4362:
        mov edx, dword ptr [ebx + 0x38]
    white_00ab4365:
        movss dword ptr [ecx + edx], xmm0
    white_00ab436a:
        mov edx, dword ptr [ebx + 0x3c]
    white_00ab436d:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4372:
        mov edx, dword ptr [ebx + 0x40]
    white_00ab4375:
        movss dword ptr [ecx + edx], xmm0
    white_00ab437a:
        mov edx, dword ptr [ebx + 0x44]
    white_00ab437d:
        movss dword ptr [ecx + edx], xmm0
    white_00ab4382:
        mov ecx, dword ptr [ebx + 0x34]
    white_00ab4385:
        test ecx, ecx
    white_00ab4387:
        mov byte ptr [esi + 2], al
    white_00ab438b:
        mov byte ptr [esi + 1], al
    white_00ab438f:
        mov byte ptr [esi], al
    white_00ab4393:
        mov byte ptr [esi + 3], al
    white_00ab4397:
        jl white_00ab43af
    white_00ab4399:
        mov edx, dword ptr [ebx + 0xc]
    white_00ab439c:
        mov edi, dword ptr [esi]
    white_00ab43a0:
        lea ecx, [ecx + edx*2]
    white_00ab43a3:
        mov edx, dword ptr [ebx + 8]
    white_00ab43a6:
        mov dword ptr [ecx + edx], edi
    white_00ab43a9:
        mov edi, dword ptr [esi - 4]
    white_00ab43ad:
        jmp white_00ab43d8
    white_00ab43af:
        mov edx, dword ptr [ebx + 8]
    white_00ab43b2:
        mov ecx, dword ptr [ebx + 0xc]
    white_00ab43b5:
        lea ecx, [edx + ecx*2]
    white_00ab43b8:
        mov edx, dword ptr [ebx + 0x38]
    white_00ab43bb:
        movss dword ptr [ecx + edx], xmm0
    white_00ab43c0:
        mov edx, dword ptr [ebx + 0x3c]
    white_00ab43c3:
        movss dword ptr [ecx + edx], xmm0
    white_00ab43c8:
        mov edx, dword ptr [ebx + 0x40]
    white_00ab43cb:
        movss dword ptr [ecx + edx], xmm0
    white_00ab43d0:
        mov edx, dword ptr [ebx + 0x44]
    white_00ab43d3:
        movss dword ptr [ecx + edx], xmm0
    white_00ab43d8:
        mov byte ptr [esi + 2], al
    white_00ab43dc:
        mov byte ptr [esi + 1], al
    white_00ab43e0:
        mov byte ptr [esi], al
    white_00ab43e4:
        mov byte ptr [esi + 3], al
    white_00ab43e8:
        mov eax, dword ptr [ebx + 0x34]
    white_00ab43eb:
        test eax, eax
    white_00ab43ed:
        jl white_00ab4403
    white_00ab43ef:
        mov ecx, dword ptr [ebx + 0xc]
    white_00ab43f2:
        mov edx, dword ptr [esi]
    white_00ab43f6:
        lea eax, [eax + ecx*2]
    white_00ab43f9:
        add eax, ecx
    white_00ab43fb:
        mov ecx, dword ptr [ebx + 8]
    white_00ab43fe:
        mov dword ptr [eax + ecx], edx
    white_00ab4401:
        jmp white_done
    white_00ab4403:
        mov eax, dword ptr [ebx + 0xc]
    white_00ab4406:
        mov ecx, dword ptr [ebx + 0x38]
    white_00ab4409:
        lea eax, [eax + eax*2]
    white_00ab440c:
        add eax, dword ptr [ebx + 8]
    white_00ab440f:
        movss dword ptr [eax + ecx], xmm0
    white_00ab4414:
        mov edx, dword ptr [ebx + 0x3c]
    white_00ab4417:
        movss dword ptr [eax + edx], xmm0
    white_00ab441c:
        mov ecx, dword ptr [ebx + 0x40]
    white_00ab441f:
        movss dword ptr [eax + ecx], xmm0
    white_00ab4424:
        mov edx, dword ptr [ebx + 0x44]
    white_00ab4427:
        movss dword ptr [eax + edx], xmm0
    white_done:
        mov final_section, edi
    }
    return final_section;
}
#pragma warning(pop)
} // namespace

NativeGuiIconRebuildStorageFrame::NativeGuiIconRebuildStorageFrame(Context& c,
    NativeGuiWidgetTransformScratch& scratch, void* key)
    : context(c), transform_scratch(scratch), layout_key_home(key) {
    // Cold metadata only; no renderer or native receiver/value snapshot.
    require(&c.geometry.actual_owners() == &c.owners && &c.clip.strings == &c.strings &&
        &c.clip.materials == &c.parameters &&
        &c.layouts.actual_renderer_00f8d394 == &c.actual_renderer_00f8d394,
        "Raw Icon current80 requires one genuine prepared owner/string/renderer domain");
    require_gui_text_native_renderer_domain(c.graphics, c.actual_renderer_00f8d394,
        c.parameters.parameter_names, c.owners);
    const auto& data = c.clip.actual_read_only_data;
    simple_color_name = data.data_at(0x00cf0078, 1);
    default_shader_name = data.data_at(0x00ce3a0c, 1);
    untextured_name = data.data_at(0x00d5c4a8, 1);
    point_name = data.data_at(0x00d5c25c, 1);
    linear_name = data.data_at(0x00d5c490, 1);
    parameter_names[0] = data.data_at(0x00d5c47c, 18);
    parameter_names[1] = data.data_at(0x00d5c470, 10);
    parameter_names[2] = data.data_at(0x00d5c464, 11);
    parameter_names[3] = data.data_at(0x00d5c454, 13);
    material_profile = static_cast<const volatile Word*>(data.data_at(0x00d5e520, 0x14));
}
NativeGuiIconRebuildStorageFrame::~NativeGuiIconRebuildStorageFrame() = default;

void rebuild_native_gui_icon_geometry_storage_00ab3cb0(void* raw,
    Word& argument_home, Frame& f, Context& c) {
    require(&f.context == &c && f.phase == NativeGuiIconRebuildStoragePhase::fresh && !f.clip_operation,
        "Raw Icon current80 requires its same fresh retained frame/context");
    f.phase = NativeGuiIconRebuildStoragePhase::entered;
    f.actual_icon = raw;
    static_cast<volatile Word&>(f.native_state_home) = 0xffffffffu;
    try {
        // Native3CC8..3CDF: capture LOWWORD, CURRENT4C, THEN store EC WORD.
        void* captured_model;
        Word* const input_home = &argument_home;
        __asm {
            mov edx, raw
            mov ecx, input_home
            mov ax, word ptr [ecx]
            mov ecx, dword ptr [edx + 4ch]
            mov captured_model, ecx
            mov word ptr [edx + 0ech], ax
        }
        f.captured_model = captured_model;
        auto* mesh = static_cast<NativeMeshStorage*>(pointer(captured_model, 0x180)); // actual raw B74640.
        home(f, 0x20) = bits(mesh);
        if (native_mesh_vertex_stream_count_00b72b20(mesh) == 0) {
            f.native_site = 0x00ab3d02;
            construct_native_string_header_0041e870(local(f, 0x18), c.strings,
                static_cast<const char*>(f.simple_color_name));
            void* const renderer = c.actual_renderer_00f8d394;
            const Word target = current_target(renderer, 0x38, c);
            static_cast<volatile Word&>(f.native_state_home) = 0;
            f.native_site = 0x00ab3d1f;
            require_target(target, 0x00b317e0);
            f.declaration.reference = load_native_renderer_vertex_declaration_00b317e0(
                renderer, local(f, 0x18), c.graphics.declarations);
            c.geometry.register_native_declaration_reference(f.declaration,
                c.graphics.declarations.declarations);
            void* const declaration = f.declaration.reference;
            void* const name_data = pointer(local(f, 0x18), 4);
            static_cast<volatile Word&>(f.native_state_home) = 0xffffffffu;
            return_name(local(f, 0x18), name_data, f, c, 0x00ab3d3e, 0x00ab3d45);
            void* const second_renderer = c.actual_renderer_00f8d394;
            const Word vertex_target = current_target(second_renderer, 0x5c, c);
            f.native_site = 0x00ab3d5a;
            void* const stream = create_native_vertex_from_captured_renderer_5c(
                second_renderer, vertex_target, 4, 1, declaration,
                c.graphics.streams, f.stream_creator);
            f.captured_stream = stream;
            f.native_site = 0x00ab3d63;
            set_native_mesh_vertex_stream_00b73bb0(*mesh, c.owners, 0, stream);
            f.native_site = 0x00ab3d6c;
            release_creator(stream, c.layouts.decrement_00ce2220, c);
            f.native_site = 0x00ab3d82;
            release_creator(declaration, c.layouts.decrement_00ce2220, c);
        }
        void* const stream = native_mesh_vertex_stream_unchecked_00b73260(mesh, 0);
        f.captured_stream = stream;
        const Word lock_target = current_target(stream, 0x10, c);
        f.native_site = 0x00ab3dae;
        require_target(lock_target, 0x00b49980);
        lock_native_logical_vertex_stream_00b49980(stream, c.graphics.streams.mapping, 4, 0, 0);
        byte(&argument_home) = 1; // Native3DB2: optimistic flag BEFORE count/getter.
        NativeMeshSectionStorage* section;
        if (native_mesh_section_count_00b72b40(mesh) == 0) {
            f.native_site = 0x00ab3dc0;
            section = c.geometry.create_native_section_00533fa0(f.section_creator);
            home(f, 0x14) = bits(section);
        } else {
            section = static_cast<NativeMeshSectionStorage*>(pointer(pointer(mesh, 0x54)));
            home(f, 0x14) = bits(section);
            f.native_site = 0x00ab3dde;
            c.layouts.increment_00ce221c(reinterpret_cast<volatile long*>(at(section, 4)));
            section = static_cast<NativeMeshSectionStorage*>(address(home(f, 0x14)));
            byte(&argument_home) = 0;
        }
        f.native_site = 0x00ab3def;
        const Word filter = gui_icon_prefers_bilinear_filter_storage_00ab2600(raw,
            f.filter, c.platform, c.textures, c.extent_constants, c.zero_00d7a218,
            c.minimum_width_00d5c3b8, c.epsilon_00d7a268);
        const Byte captured_new_flag = byte(&argument_home); // Before local filter store.
        local_byte(f, 0x13) = static_cast<Byte>(filter);
        NativeMaterialStorage* material;
        if (captured_new_flag == 0 && static_cast<Byte>(filter) == byte(raw, 0x134)) {
            material = static_cast<NativeMaterialStorage*>(pointer(section, 0x20));
            f.native_site = 0x00ab3e0e;
            c.layouts.increment_00ce221c(reinterpret_cast<volatile long*>(at(material, 4)));
        } else {
            byte(raw, 0x134) = static_cast<Byte>(filter);
            f.native_site = 0x00ab3e28;
            construct_native_string_header_0041e870(local(f, 0x18), c.strings,
                static_cast<const char*>(f.default_shader_name));
            const Word shader_length = load(raw, 0x104);
            void* const captured_default_data = pointer(local(f, 0x18), 4);
            int comparison;
            if (shader_length == 0) comparison = load(local(f, 0x18)) ? -1 : 0;
            else if (load(local(f, 0x18)) == 0) comparison = 1;
            else {
                f.native_site = 0x00ab3e5d;
                comparison = c.parameters.compare_names_00bf7fbf(
                    static_cast<const char*>(pointer(raw, 0x108)),
                    static_cast<const char*>(captured_default_data));
            }
            local_byte(f, 0x12) = comparison != 0;
            return_name(local(f, 0x18), captured_default_data, f, c, 0x00ab3e7b, 0x00ab3e82);
            if (local_byte(f, 0x12) != 0) {
                f.native_site = 0x00ab3e90;
                material = create_material(at(raw, 0x104), f, c);
            } else if (byte(raw, 0x100) == 0) {
                f.native_site = 0x00ab3eae;
                construct_native_string_header_0041e870(local(f, 0x24), c.strings,
                    static_cast<const char*>(f.untextured_name));
                static_cast<volatile Word&>(f.native_state_home) = 1;
                f.native_site = 0x00ab3ebf;
                material = create_material(local(f, 0x24), f, c);
                void* const data = pointer(local(f, 0x24), 4);
                static_cast<volatile Word&>(f.native_state_home) = 0xffffffffu;
                return_name(local(f, 0x24), data, f, c, 0x00ab3f25, 0x00ab3f2c);
            } else {
                const void* const effect_name = local_byte(f, 0x13) != 0 ? f.point_name : f.linear_name;
                f.native_site = 0x00ab3ef2;
                construct_native_string_header_0041e870(local(f, 0x2c), c.strings,
                    static_cast<const char*>(effect_name));
                static_cast<volatile Word&>(f.native_state_home) = 2;
                f.native_site = 0x00ab3f03;
                material = create_material(local(f, 0x2c), f, c);
                void* const data = pointer(local(f, 0x2c), 4);
                static_cast<volatile Word&>(f.native_state_home) = 0xffffffffu;
                return_name(local(f, 0x2c), data, f, c, 0x00ab3f25, 0x00ab3f2c);
            }
            section = static_cast<NativeMeshSectionStorage*>(address(home(f, 0x14)));
        }
        f.captured_material = material;
        // Address-only preparation AFTER real material birth, BEFORE Native regs.
        f.clip_operation.emplace(raw, *material, c.clip);
        f.native_site = 0x00ab3f80;
        parameter(f, c, *material, 0, 0x11, at(raw, 0x94), 3);
        f.native_site = 0x00ab3ff7;
        parameter(f, c, *material, 1, 9, at(raw, 0xa4), 4);
        f.native_site = 0x00ab406e;
        parameter(f, c, *material, 2, 10, at(raw, 0xb4), 5);
        f.native_site = 0x00ab40e5;
        parameter(f, c, *material, 3, 12, at(raw, 0xc4), 6);
        f.native_site = 0x00ab4114;
        register_native_gui_widget_clip_storage_00aa9f10(raw, *material, c.clip, *f.clip_operation);
        f.native_site = 0x00ab411e;
        set_native_material_parameter_owner_00b18a40(*material, raw, 1, c.owners);
        word(section, 8) = 5;
        word(section, 0x0c) = 0;
        word(section, 0x14) = 0;
        word(section, 0x10) = 4;
        word(section, 0x18) = 2;
        Byte has_texture;
        void* const uv = local(f, 0x34);
        const volatile float* const one = &c.extent_constants.one_00d7a24c;
        __asm {
            mov ecx, raw
            mov edx, uv
            xorps xmm0, xmm0
            cmp byte ptr [ecx + 100h], 0
            setne has_texture
            movss dword ptr [edx], xmm0
            movss dword ptr [edx + 4], xmm0
            mov eax, one
            movss xmm0, dword ptr [eax]
            movss dword ptr [edx + 8], xmm0
            movss dword ptr [edx + 12], xmm0
        }
        if (has_texture != 0) {
            void* const state = current_state(raw);
            void* const texture = pointer(state);
            f.native_site = 0x00ab41a4;
            set_native_material_texture_00b189f0(*material, 0, texture, c.owners);
            section = static_cast<NativeMeshSectionStorage*>(copy_state_fields_and_uv(raw, state, f));
        } else {
            f.native_site = 0x00ab41ff;
            set_native_material_texture_00b189f0(*material, 0, nullptr, c.owners);
        }
        if (byte(raw, 0x74) == 0) {
            const Byte current_texture = byte(raw, 0x100);
            if (current_texture != 0) setup_quad_size(raw, current_state(raw), f.quad_argument_homes);
            const Word first_u = load(raw, 0x118);
            const Word quad_target = current_target(raw, 0x7c, c);
            if (current_texture == 0) static_cast<volatile Word&>(f.quad_argument_homes[6]) = bits(at(raw, 0x20));
            static_cast<volatile Word&>(f.quad_argument_homes[2]) = first_u;
            static_cast<volatile Word&>(f.quad_argument_homes[3]) = load(raw, 0x11c);
            static_cast<volatile Word&>(f.quad_argument_homes[4]) = load(raw, 0x120);
            static_cast<volatile Word&>(f.quad_argument_homes[5]) = load(raw, 0x124);
            static_cast<volatile Word&>(f.quad_argument_homes[1]) = bits(local(f, 0x34));
            static_cast<volatile Word&>(f.quad_argument_homes[0]) = bits(stream);
            f.native_site = current_texture ? 0x00ab4299 : 0x00ab42da;
            require_target(quad_target, 0x00ab1860);
            write_native_gui_icon_cropped_quad_storage_00ab1860(raw, f.quad_argument_homes,
                f.quad, c.extent_constants.one_00d7a24c, c.vertical_scale_00e12fd0);
            if (current_texture != 0)
                section = static_cast<NativeMeshSectionStorage*>(address(home(f, 0x14)));
            section = static_cast<NativeMeshSectionStorage*>(write_white(f, stream, section,
                c.extent_constants.one_00d7a24c));
        }
        const Word unlock_target = current_target(stream, 0x14, c);
        f.native_site = 0x00ab4434;
        require_target(unlock_target, 0x00b49a80);
        unlock_native_logical_vertex_stream_00b49a80(stream, c.graphics.streams.mapping);
        f.native_site = 0x00ab4438;
        recompose_native_gui_widget_transform_00aa7220(raw, f.transform_scratch, c.transform);
        f.native_site = 0x00ab4440;
        set_native_mesh_section_material_00b864c0(*section, c.owners, material);
        f.saved_decrement = c.layouts.decrement_00ce2220; // Native4445, saved across layout.
        f.native_site = 0x00ab444f;
        release_creator(material, f.saved_decrement, c);
        mesh = static_cast<NativeMeshStorage*>(address(home(f, 0x20)));
        static_cast<volatile Word&>(f.layout_mesh_home) = bits(mesh);
        f.native_site = 0x00ab4464;
        rebuild_native_mesh_section_vertex_layout_storage_00b865a0(section,
            static_cast<volatile Word&>(f.layout_mesh_home), f.layout_key_home,
            c.layouts, &f.layout_acquired_before_insertion);
        if (byte(&argument_home) != 0) {
            f.native_site = 0x00ab4473;
            append_native_mesh_draw_section_00b73c60(*mesh, section);
        }
        f.native_site = 0x00ab447c;
        release_creator(section, f.saved_decrement, c); // SAME saved import, no reload.
        const Word color_target = current_target(raw, 0x50, c);
        const void* const rgba = at(raw, 0x50);
        f.native_site = 0x00ab4495;
        require_target(color_target, 0x00aa6870);
        set_native_gui_widget_color_storage_00aa6870(raw, rgba);
        f.phase = NativeGuiIconRebuildStoragePhase::complete;
    } catch (...) {
        f.failure = std::current_exception();
        f.phase = NativeGuiIconRebuildStoragePhase::failed;
        throw;
    }
}
} // namespace bsp