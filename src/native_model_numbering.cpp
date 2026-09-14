#include "bsp/native_model_numbering.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_instance_collection.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_node_visibility_factor.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::uint32_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof(value));
    return value;
}
void require(bool value, const char* reason) {
    if (!value) throw std::logic_error(reason);
}
bool contains(const void* name, const char* needle) {
    const auto* text = read<const char*>(name, 4);
    if (!text) return false;
    const auto* match = std::strstr(text, needle);
    return match && static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(match) -
        reinterpret_cast<std::uintptr_t>(read<const char*>(name, 4))) != 0xffffffffu;
}
void* first_texture(void* material) noexcept {
    return read<std::int16_t>(material, 0x34) > 0 ? read<void*>(material, 0x10) : nullptr;
}
struct Name {
    NativeString value;
    NativeStringStorage& strings;
    bool armed{};
    explicit Name(NativeStringStorage& s) : strings(s) {}
    ~Name() { finish(); }
    void finish() noexcept {
        if (armed) { armed = false; destroy_native_string_header_0041dd20(&value, strings); }
    }
    void construct(const char* text) {
        construct_native_string_cstring_0041e870(&value, text, strings);
        armed = true;
    }
};
void copy3(const float* input, void* output) noexcept {
    __asm {
        mov ecx, input
        mov eax, output
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 4]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 8]
        fstp dword ptr [eax + 8]
    }
}
void atlas_uv(const void* item, const float* uv, void* output) noexcept {
    float minimum;
    __asm {
        mov eax, item
        mov ecx, uv
        mov edx, output
        fld dword ptr [eax + 14h]
        fstp minimum
        fld dword ptr [eax + 1ch]
        fld minimum
        fld st(0)
        fsubp st(2), st(0)
        fld dword ptr [ecx]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [edx]
        fld dword ptr [eax + 18h]
        fstp minimum
        fld dword ptr [eax + 20h]
        fld minimum
        fld st(0)
        fsubp st(2), st(0)
        fld dword ptr [ecx + 4]
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp dword ptr [edx + 4]
    }
}
void stream_slot(void* stream, NativeModelNumberingServices& s,
    std::uint32_t offset, std::uint32_t expected) {
    const auto* profile = s.renderer.streams.vertices.actual_logical_profile_00d61d6c;
    require(read<std::uint32_t>(stream) == 0x00d61d6cu && profile &&
        profile[offset / 4u] == expected,
        "Model numbering requires the current supported logical vertex slot");
}
} // namespace

void rebuild_native_numbering_vertices_00711714_fragment(void* source, const void* source_map, void* destination_map,
    NativeMeshSectionStorage& section, std::uint32_t number,
    NativeNumberingVertexServices& s, std::uint32_t& native_site) {
    const auto hundreds = number / 100u;
    const auto tens = (number % 100u) / 10u;
    const auto units = number - 10u * (tens + hundreds * 10u);
    if (read<std::uint32_t>(&section, 0x10) == 0) return;
    auto* output = static_cast<std::byte*>(destination_map);
    auto* original_uv = static_cast<const std::byte*>(source_map) + 0x20;
    for (std::uint32_t i = 0; i < read<std::uint32_t>(&section, 0x10); ++i) {
        float position[3], normal[3], uv[2];
        native_site = 0x0071171c;
        require(read_native_vertex_position_004768d0(source, i, position, s.half_import),
            "Model numbering reached an uninitialized native position format");
        copy3(position, output);
        native_site = 0x00711740;
        require(read_native_vertex_normal_0070fdb0(source, i, normal, s.half_import),
            "Model numbering reached an uninitialized native normal format");
        copy3(normal, output + 0x0c);
        if (read<std::uint32_t>(source, 0x2c) == 3u) {
            std::memcpy(uv, original_uv, sizeof(uv));
        } else {
            native_site = 0x0071177e;
            require(read_native_vertex_uv_007100a0(source, i, uv, s.half_import),
                "Model numbering reached an uninitialized native UV format");
        }
        std::uint32_t colour;
        native_site = 0x007117a0;
        read_native_vertex_colour_00476180(source, i, colour);
        std::memcpy(output + 0x28, &colour, sizeof(colour));
        std::uint32_t digit = 0;
        bool show = hundreds != 0;
        if (read<std::uint8_t>(output, 0x2a) >= 250) digit = hundreds;
        else if (read<std::uint8_t>(output, 0x29) >= 250) { digit = tens; show = hundreds != 0 || tens != 0; }
        else if ((colour & 0xffu) >= 250) { digit = units; show = true; }
        if (!show) {
            const std::uint32_t zero = 0;
            std::memcpy(output + 0x18, &zero, 4);
            std::memcpy(output + 0x1c, &zero, 4);
        } else {
            Name suffix(s.strings), decimal(s.strings), prefix(s.strings), stem(s.strings), name(s.strings);
            native_site = 0x0071188c; suffix.construct(".tga");
            native_site = 0x007118a1;
            construct_native_material_program_number_00711370(decimal.value, digit, s.strings);
            decimal.armed = true;
            native_site = 0x007118b9; prefix.construct("num_");
            native_site = 0x007118ce;
            concatenate_native_string_headers_004261a0(&prefix.value, &stem.value, &decimal.value, s.strings);
            stem.armed = true;
            native_site = 0x007118e7;
            concatenate_native_string_headers_004261a0(&stem.value, &name.value, &suffix.value, s.strings);
            name.armed = true;
            stem.finish(); prefix.finish(); decimal.finish(); suffix.finish();
            native_site = 0x0071199c;
            const void* item = find_native_particle_atlas_item_00aefb20(s.atlas_00f8c26c,
                name.value.data() ? name.value.data() : s.null_number_name_00e19b4f,
                s.strings, s.null_atlas_pattern_00e17bf0);
            atlas_uv(item, uv, output + 0x18);
            name.finish();
        }
        std::memcpy(output + 0x20, uv, sizeof(uv));
        output += 0x2c;
        original_uv += 0x2c;
    }
}


void set_native_section_numbering_00711510(NativeMeshStorage& mesh,
    NativeMeshSectionStorage& section, std::uint32_t number,
    NativeModelNumberingServices& s, NativeModelNumberingOperation& a) {
    require((a.phase == NativeModelNumberingPhase::fresh || a.phase == NativeModelNumberingPhase::complete) &&
        !a.declaration.reference && !a.vertex.creator,
        "Model numbering cannot replay an interrupted operation");
    a = {}; a.phase = NativeModelNumberingPhase::running; a.section = &section;
    try {
        void* material = read<void*>(&section, 0x20);
        if (read<std::int16_t>(material, 0x34) <= 0 || !read<void*>(material, 0x10)) {
            a.phase = NativeModelNumberingPhase::complete; return;
        }
        a.native_site = 0x0071155d;
        if (!contains(native_logical_texture_name_address_00b33e40(first_texture(material)), "num_0")) {
            a.native_site = 0x0071159d;
            material = read<void*>(&section, 0x20);
            if (!contains(native_logical_texture_name_address_00b33e40(first_texture(material)), "shipnumber")) {
                a.phase = NativeModelNumberingPhase::complete; return;
            }
        }
        void* source = read<void*>(&section, 0x3c);
        if (read<std::uint32_t>(source, 0x1c) == 0xffffffffu ||
            read<std::uint32_t>(source, 0x28) == 0xffffffffu ||
            read<std::uint32_t>(source, 0x34) == 0xffffffffu) {
            a.phase = NativeModelNumberingPhase::complete; return;
        }
        if (number > 999u) number = 999u;
        auto& owners = s.renderer.streams.geometry.actual_owners();
        require_gui_text_native_renderer_domain(s.renderer, s.renderer_00f8d394, s.strings, owners);
        {
            Name format(s.strings); a.native_site = 0x00711643;
            format.construct("pf43nf43uf44cc.mvfm");
            a.native_site = 0x00711663;
            load_gui_text_native_declaration_current38(s.renderer_00f8d394, format.value, s.renderer, a.declaration);
        }
        a.native_site = 0x00711691;
        void* destination = create_gui_text_native_vertex_current5c(s.renderer_00f8d394,
            read<std::uint32_t>(&section, 0x10), 1, a.declaration.reference, s.renderer, a.vertex);
        a.native_site = 0x0071169d;
        // 710630: release the captured referent, then clear the caller's slot.
        release_native_render_actual_owner(owners, a.declaration.reference);
        a.declaration = {};
        a.native_site = 0x007116af;
        stream_slot(destination, s, 0x10, 0x00b49980);
        a.vertex.destination_map = NativeStreamCloneMapPhase::call_in_progress;
        a.vertex.destination_mapping = lock_native_logical_vertex_stream_00b49980(destination,
            s.renderer.streams.mapping, 0, 0, 0);
        a.vertex.destination_map = NativeStreamCloneMapPhase::returned;
        a.native_site = 0x007116c2;
        stream_slot(source, s, 0x10, 0x00b49980);
        a.vertex.source = source; a.vertex.source_map = NativeStreamCloneMapPhase::call_in_progress;
        a.vertex.source_mapping = lock_native_logical_vertex_stream_00b49980(source,
            s.renderer.streams.mapping, 0, 0, 1);
        a.vertex.source_map = NativeStreamCloneMapPhase::returned;
        a.native_site = 0x007116d3;
        void* item = find_native_particle_atlas_item_00aefb20(s.atlas_00f8c26c, "num_0.tga",
            s.strings, s.null_atlas_pattern_00e17bf0);
        a.native_site = 0x007116e1;
        set_native_material_texture_00b189f0(*read<NativeMaterialStorage*>(&section, 0x20),
            0, read<void*>(item, 8), owners);
        NativeNumberingVertexServices vertices{s.strings, s.half_import, s.atlas_00f8c26c,
            s.null_atlas_pattern_00e17bf0, s.null_number_name_00e19b4f};
        rebuild_native_numbering_vertices_00711714_fragment(source, a.vertex.source_mapping,
            a.vertex.destination_mapping, section, number, vertices, a.native_site);
        a.native_site = 0x00711815;
        stream_slot(source, s, 0x14, 0x00b49a80);
        a.vertex.source_map = NativeStreamCloneMapPhase::unlock_in_progress;
        unlock_native_logical_vertex_stream_00b49a80(source, s.renderer.streams.mapping);
        a.vertex.source_map = NativeStreamCloneMapPhase::unlocked;
        a.native_site = 0x0071181e;
        stream_slot(destination, s, 0x14, 0x00b49a80);
        a.vertex.destination_map = NativeStreamCloneMapPhase::unlock_in_progress;
        unlock_native_logical_vertex_stream_00b49a80(destination, s.renderer.streams.mapping);
        a.vertex.destination_map = NativeStreamCloneMapPhase::unlocked;
        a.native_site = 0x00711829;
        set_native_mesh_vertex_stream_00b73bb0(mesh, owners, 0, destination);
        a.native_site = 0x00711832;
        release_native_render_actual_owner(owners, destination);
        a.vertex.creator = nullptr; a.vertex.companion = nullptr;
        a.vertex.phase = NativeStreamClonePhase::consumed;
        a.native_site = 0x00711849;
        rebuild_native_mesh_section_vertex_layout_00b865a0(section, owners, &mesh, s.layouts);
        a.phase = NativeModelNumberingPhase::complete;
    } catch (...) { a.phase = NativeModelNumberingPhase::failed; throw; }
}

void set_native_node_numbering_00711a20(NativeNodeStorage& node, std::uint32_t number,
    NativeModelNumberingServices& s, NativeModelNumberingOperation& operation) {
    require(operation.phase != NativeModelNumberingPhase::failed,
        "Model numbering cannot replay a failed hierarchy operation");
    try {
    const auto profile = read<std::uint32_t>(&node);
    const auto type = s.model_type_01090034.own_id;
    require(s.node_virtual0c != nullptr, "Model numbering requires actual node type dispatch");
    if (s.node_virtual0c(s.type_context, node, profile, type)) {
        bool matched = false, hide = false;
        auto& model = *reinterpret_cast<NativeModelTailStorage*>(reinterpret_cast<std::byte*>(&node) + 0x174);
        for (std::uint32_t geometry_index = 0;
            geometry_index < static_cast<std::uint32_t>(model.geometry_180 != nullptr); ++geometry_index) {
            auto* mesh = static_cast<NativeMeshStorage*>(gui_model_geometry_00b74640(model, geometry_index));
            for (std::uint32_t i = 0; i < gui_mesh_element_count_00b72b40(mesh); ++i) {
                auto* section = gui_geometry_element_00b732c0(mesh, static_cast<std::int32_t>(i));
                void* material = read<void*>(section, 0x20);
                if (!contains(native_effect_name_00b172d0(read<void*>(material, 0x7c)), "shipnumber")) continue;
                if (!contains(native_logical_texture_name_address_00b33e40(first_texture(material)), "shipnumber") &&
                    !contains(native_logical_texture_name_address_00b33e40(first_texture(material)), "num_0")) continue;
                matched = true; if (number == 0) hide = true;
                section = gui_geometry_element_00b732c0(mesh, static_cast<std::int32_t>(i));
                set_native_section_numbering_00711510(*mesh, *static_cast<NativeMeshSectionStorage*>(section),
                    number, s, operation);
            }
        }
        if (matched) set_native_node_visibility_factor_00b6da70(&node, nullptr, hide ? 0.0f : 1.0f, 0);
    }
    for (auto child = node.first_child_34; child != 0;) {
        auto* current = reinterpret_cast<NativeNodeStorage*>(child);
        set_native_node_numbering_00711a20(*current, number, s, operation);
        child = current->next_sibling_3c;
    }
    } catch (...) { operation.phase = NativeModelNumberingPhase::failed; throw; }
}

void set_native_model_numbering_00711be0(void* model, std::uint32_t number,
    NativeModelNumberingServices& s, NativeModelNumberingOperation& operation) {
    auto* holder = read<void*>(model, 0x160);
    set_native_node_numbering_00711a20(*read<NativeNodeStorage*>(holder, 0x0c), number, s, operation);
}
} // namespace bsp
