#include "bsp/gui_icon_runtime.hpp"
#include "bsp/gui_lua_reader.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
// 00AA7974..00AA797C and 00AB1F33..00AB1F43 use FLD/FSTP, not bit copies.
void copy_size_pair(GuiWidgetSize& destination, const GuiWidgetSize& source) {
    const auto* input = &source;
    auto* output = &destination;
    __asm {
        mov eax, input
        mov edx, output
        fld dword ptr [eax]
        fstp dword ptr [edx]
        fld dword ptr [eax + 4]
        fstp dword ptr [edx + 4]
    }
}
const GuiValue* present(const GuiTable& table, const char* key) {
    const auto* value = table.find(key);
    return value && value->kind() != GuiValue::Kind::Nil ? value : nullptr;
}
void property(const GuiTable& table, const char* key, GuiLuaFieldType type, void* target,
    const bool& crt_sse2_conversion) {
    if (const auto* value = present(table, key)) {
        require(gui_lua_store_value_00bd63b0(*value, gui_lua_field(type, target), nullptr,
                crt_sse2_conversion),
            "Icon property has an unsupported aggregate shape.");
    }
}
const GuiTable& scope(const GuiValue& value) {
    require(value.is_table() && value.table(), "Icon reader cannot index a non-table scope.");
    return *value.table();
}
void validate_geometry_services(const GuiGeometryRuntimeServices& services) {
    require(services.device && services.states && services.geometry &&
        services.associate_geometry && services.create_vertex_stream &&
        services.create_material && services.register_clip_and_owner &&
        services.recompose_00aa7220 && services.publish_color_virtual50,
        "GUI geometry requires actual device, resource, clip, lifetime and scene services.");
}
void register_parameter(MaterialCloneState& material, const char* name,
    const float* source, std::uint32_t words) {
    std::string error;
    const auto result = material.parameters.register_words_00b17e10_00b44d60(
        name, {source, words, words, false}, error);
    if (result.status == MaterialParameterRegistrationStatus::unsupported)
        throw std::runtime_error("GUI material parameter " + std::string(name) + ": " + error);
    // no_match is the native effect's legitimate absent-constant behavior.
}
struct MappedStream {
    D3D9StateCache& states;
    LogicalVertexStream& stream;
    bool locked{true};
    ~MappedStream() { if (locked) states.unlock_vertex_stream_00b49a80(stream); }
    void unlock() { states.unlock_vertex_stream_00b49a80(stream); locked = false; }
};
struct VertexOffsets { std::uint32_t position, uv, color, stride; bool packed; };
VertexOffsets vertex_offsets(const VertexDeclaration& declaration) {
    require(declaration.contains_00b47c90(D3DDECLUSAGE_POSITION, 0) &&
        declaration.contains_00b47c90(D3DDECLUSAGE_TEXCOORD, 0) &&
        declaration.contains_00b47c90(D3DDECLUSAGE_COLOR, 0),
        "GUI stream declaration is missing position, UV or color.");
    require(declaration.type_00b47c20(D3DDECLUSAGE_POSITION, 0) == D3DDECLTYPE_FLOAT3 &&
        declaration.type_00b47c20(D3DDECLUSAGE_TEXCOORD, 0) == D3DDECLTYPE_FLOAT2,
        "GUI quad writer requires actual float3 position and float2 UV fields.");
    const auto color_type = declaration.type_00b47c20(D3DDECLUSAGE_COLOR, 0);
    require(color_type == D3DDECLTYPE_D3DCOLOR || color_type == D3DDECLTYPE_FLOAT4,
        "GUI color writer requires packed D3DCOLOR or float4 color.");
    VertexOffsets offsets{declaration.offset_00b47c40(D3DDECLUSAGE_POSITION, 0),
        declaration.offset_00b47c40(D3DDECLUSAGE_TEXCOORD, 0),
        declaration.offset_00b47c40(D3DDECLUSAGE_COLOR, 0), declaration.stride,
        color_type == D3DDECLTYPE_D3DCOLOR};
    const auto fits = [&](std::uint32_t offset, std::uint32_t size) {
        return offset <= offsets.stride && size <= offsets.stride - offset;
    };
    require(fits(offsets.position, 12) && fits(offsets.uv, 8) &&
        fits(offsets.color, offsets.packed ? 4u : 16u),
        "GUI declaration offsets exceed the actual stream stride.");
    return offsets;
}
}

GuiIconAuthoredPage read_gui_icon_authored_page_00ab3310(
    const GuiTable& table, const GuiWidgetTransform& widget, const bool& crt_sse2_conversion) {
    GuiIconAuthoredPage result;
    property(table, "DynamicVB", GuiLuaFieldType::Bool, &result.dynamic_vb, crt_sse2_conversion);
    property(table, "HasTexture", GuiLuaFieldType::Bool, &result.has_texture, crt_sse2_conversion);
    property(table, "ShaderName", GuiLuaFieldType::String, &result.shader_name, crt_sse2_conversion);
    property(table, "PartialDisplayType", GuiLuaFieldType::Int, &result.partial_display_type,
        crt_sse2_conversion);
    property(table, "PartialDisplayRatio", GuiLuaFieldType::Float, &result.partial_display_ratio,
        crt_sse2_conversion);
    property(table, "DelayedTextureLoad", GuiLuaFieldType::Bool, &result.delayed_texture_load,
        crt_sse2_conversion);
    property(table, "AutoRotate", GuiLuaFieldType::Float, &result.auto_rotate, crt_sse2_conversion);
    if (const auto* states_value = present(table, "States")) {
        for (const auto& value : scope(*states_value).array) {
            if (value.kind() == GuiValue::Kind::Nil) break;
            const auto& entry = scope(value);
            GuiIconAuthoredState state;
            property(entry, "Texture", GuiLuaFieldType::String, &state.texture, crt_sse2_conversion);
            float size[2]{widget.size.width, widget.size.height};
            property(entry, "Size", GuiLuaFieldType::Vec2, size, crt_sse2_conversion);
            state.size = {size[0], size[1]};
            float pivot[2]{widget.pivot_x, widget.pivot_y};
            property(entry, "Pivot", GuiLuaFieldType::Vec2, pivot, crt_sse2_conversion);
            state.pivot_x = pivot[0]; state.pivot_y = pivot[1];
            if (const auto* uv_value = present(entry, "UV_LURB")) {
                const auto& uv_table = scope(*uv_value);
                float uv[4]{};
                const GuiValue nil;
                for (std::size_t lane = 0; lane < 4; ++lane) {
                    const auto& component = lane < uv_table.array.size() ? uv_table.array[lane] : nil;
                    gui_lua_store_value_00bd63b0(component,
                        gui_lua_field(GuiLuaFieldType::Float, &uv[lane]), nullptr, crt_sse2_conversion);
                }
                state.uv = {uv[0], uv[1], uv[2], uv[3]};
            }
            result.states.push_back(std::move(state));
        }
    }
    return result;
}

void rebuild_gui_geometry_00ab3cb0_fragment(GuiLayoutWidget& widget,
    float& overbright, const GuiGeometryRuntimeServices& services,
    const GuiGeometryRebuildRequest& request) {
    validate_geometry_services(services);
    require(request.vertex_count && request.choose_material && request.prepare_vertices,
        "GUI geometry rebuild lacks its class-specific state writer.");
    auto geometry = services.geometry();
    require(bool(geometry), "GUI +74 must associate a real mesh before rebuilding.");
    require(!geometry->instance_stream && !geometry->indices,
        "GUI runtime cannot replace indexed or instanced model geometry.");
    if (!geometry->mesh_stream) {
        geometry->mesh_stream = services.create_vertex_stream("SimpleColor.mvfm", request.vertex_count, 1);
        require(bool(geometry->mesh_stream), "GUI vertex stream creation returned no owner.");
    }
    auto& stream = *geometry->mesh_stream;
    require(stream.declaration && stream.physical && stream.vertex_count >= request.vertex_count,
        "GUI geometry has an incomplete or undersized actual logical stream.");
    const auto offsets = vertex_offsets(*stream.declaration);
    void* mapped = nullptr;
    const auto locked = services.states->lock_vertex_stream_00b49980(
        stream, request.vertex_count, 0, false, mapped);
    require(SUCCEEDED(locked), "GUI logical vertex stream lock failed.");
    MappedStream mapping{*services.states, stream};
    require(mapped != nullptr, "GUI logical stream lock returned no writable bytes.");

    const bool existing_section = bool(geometry->section.material_clone_owner);
    const auto choice = request.choose_material(existing_section);
    GuiGeometryMaterial selected;
    if (existing_section && choice.reuse_existing) {
        selected.owner = std::static_pointer_cast<MaterialCloneState>(geometry->section.material_clone_owner);
        selected.order = geometry->section.material_order;
        selected.queue_index = geometry->section.material_queue_index;
    } else {
        selected = services.create_material(choice.name);
    }
    require(selected.owner && selected.owner->effect && selected.owner->parameters.shader_identity(),
        "GUI rebuild requires an actual effect-bound compiled material.");
    auto& material = *selected.owner;
    register_parameter(material, "cOverbrightFactor", &overbright, 1);
    register_parameter(material, "cLowColor", widget.low_color, 4);
    register_parameter(material, "cHighColor", widget.high_color, 4);
    register_parameter(material, "cBlendFactor", &widget.blend_factor, 1);
    services.register_clip_and_owner(material);
    geometry->section.primitive = request.primitive;
    geometry->section.range_words = request.range_words;
    require(set_material_texture_00b189f0(material.textures, 0, request.texture),
        "GUI material texture slot zero assignment failed.");
    std::vector<GuiQuadVertex> vertices;
    const bool writes_enabled = !widget.transform.bounds_enabled;
    const bool positions_valid = request.prepare_vertices(vertices, writes_enabled);
    if (writes_enabled) {
        require(!positions_valid || vertices.size() == request.vertex_count,
            "GUI class geometry writer returned the wrong vertex count.");
        auto* bytes = static_cast<unsigned char*>(mapped);
        const std::uint32_t white = 0xffffffffu;
        const float white_float[4]{1, 1, 1, 1};
        for (std::size_t i = 0; i < request.vertex_count; ++i) {
            auto* vertex = bytes + i * offsets.stride;
            if (positions_valid) {
                const auto& source = vertices[i];
                const float position[3]{source.x, source.y, source.z};
                const float uv[2]{source.u, source.v};
                std::memcpy(vertex + offsets.position, position, sizeof(position));
                std::memcpy(vertex + offsets.uv, uv, sizeof(uv));
            }
            std::memcpy(vertex + offsets.color,
                offsets.packed ? static_cast<const void*>(&white) : white_float,
                offsets.packed ? sizeof(white) : sizeof(white_float));
        }
    }
    mapping.unlock();
    services.recompose_00aa7220();
    geometry->section.material_clone_owner = selected.owner;
    geometry->section.material_order = selected.order;
    geometry->section.material_queue_index = selected.queue_index;
    auto layout = std::make_shared<D3D9VertexLayout>();
    layout->append_stream_00b48a00(stream.declaration);
    require(SUCCEEDED(layout->create_if_missing_00b60a10(*services.device)),
        "GUI draw-section vertex layout creation failed.");
    geometry->combined_layout = std::move(layout);
    // The single section is retained by the same associated geometry owner.
    services.publish_color_virtual50();
}

struct GuiIconRuntime::Impl final : GuiIconHost {
    struct TextureReference {
        std::shared_ptr<void> native_reference;
        std::shared_ptr<LogicalTexture> logical;
    };
    GuiLayoutWidget& widget;
    float& overbright;
    GuiIconRuntimeServices services;
    std::vector<TextureReference> textures;
    GuiIconWidget icon;
    Impl(GuiLayoutWidget& base, float& brightness, GuiIconRuntimeServices supplied)
        : widget(base), overbright(brightness), services(std::move(supplied)) {
        require(widget.type == GuiWidgetType::Icon && widget.transform.type_id == kGuiIconTypeId,
            "GuiIconRuntime requires the SAME type6 base widget owner.");
        validate_geometry_services(services.geometry);
        const auto& t = services.textures;
        require(t.resolve.find_atlas_item && t.resolve.load_texture && t.resolve.width &&
            t.resolve.height && t.resolve.retain && t.release && t.logical_texture &&
            services.platform_allows_point_filter && services.base_loaded78_00aa7170 &&
            services.set_position_00aa7dc0, "Icon runtime is missing an actual texture or base hook service.");
    }
    void* adopt(void* raw) {
        require(raw != nullptr, "Icon texture resolution returned no texture.");
        TextureReference reference;
        reference.native_reference = std::shared_ptr<void>(raw, services.textures.release);
        reference.logical = services.textures.logical_texture(raw);
        require(reference.logical && reference.logical->texture,
            "Icon texture lacks its retained actual logical/COM resource.");
        textures.push_back(std::move(reference));
        return raw;
    }
    std::shared_ptr<LogicalTexture> logical(void* raw) const {
        for (const auto& reference : textures)
            if (reference.native_reference.get() == raw) return reference.logical;
        throw std::runtime_error("Icon selected state has no owned texture reference.");
    }
    void* resolve_texture(const std::string& name, GuiUvRect& uv,
        GuiWidgetSize& size, float scale) override {
        std::array<float, 4> rectangle{uv.left, uv.top, uv.right, uv.bottom};
        std::array<float, 2> extent{size.width, size.height};
        void* raw = resolve_gui_texture_00aa2660(name, rectangle, extent, scale,
            services.textures.resolve);
        // Adopt first so a failing logical-owner lookup releases the reference.
        adopt(raw);
        uv = {rectangle[0], rectangle[1], rectangle[2], rectangle[3]};
        size = {extent[0], extent[1]};
        return raw;
    }
    void* load_placeholder_texture(const std::string& name) override {
        return adopt(services.textures.resolve.load_texture(name, 0));
    }
    std::uint32_t texture_width(void* texture) override { return services.textures.resolve.width(texture); }
    std::uint32_t texture_height(void* texture) override { return services.textures.resolve.height(texture); }
};

GuiIconRuntime::GuiIconRuntime(GuiLayoutWidget& widget, float& overbright,
    GuiIconRuntimeServices services) : impl_(std::make_unique<Impl>(widget, overbright, std::move(services))) {}
GuiIconRuntime::~GuiIconRuntime() = default;
const GuiIconWidget& GuiIconRuntime::state() const noexcept { return impl_->icon; }
void GuiIconRuntime::constructed74_00ab2540() {
    auto& self = *impl_;
    if (!self.widget.transform.bounds_enabled) {
        self.services.geometry.associate_geometry(std::make_shared<GeneratedInstanceGeometry>(),
            -1.0f, -1.0f); // 00D7A260, verified saved bytes BF800000
    }
    self.services.set_position_00aa7dc0({});
}
void GuiIconRuntime::read_properties_00ab3310(const GuiTable& table) {
    auto& self = *impl_;
    const auto authored = read_gui_icon_authored_page_00ab3310(table, self.widget.transform,
        self.services.crt_sse2_conversion);
    // These properties are recovered, but their per-frame asynchronous/rotation
    // consumer00AB6430 is outside this startup packet. Fail explicitly instead
    // of accepting a page whose later behavior this owner would silently omit.
    require(authored.auto_rotate == 0.0f, "Icon AutoRotate requires the unreconstructed frame updater00AB6430.");
    require(!authored.delayed_texture_load, "Icon DelayedTextureLoad requires the unreconstructed frame updater00AB6430.");
    gui_icon_apply_authored_page_00ab3310(self.icon, self.widget.transform, authored, self);
}
void GuiIconRuntime::loaded78_00ab10f0() {
    impl_->services.base_loaded78_00aa7170();
    select_state_00ab1710(0, 0, 1.0f);
}
void GuiIconRuntime::select_state_00ab1710(std::int16_t index, std::int32_t mode, float ratio) {
    if (gui_icon_select_state_00ab1710(impl_->icon, index, mode, ratio)) rebuild_00ab3cb0(index);
}
void GuiIconRuntime::set_size58_00ab1ef0(GuiWidgetOwner& owner, const GuiWidgetSize& size) {
    auto& self = *impl_;
    require(&owner.layout() == &self.widget,
        "Icon size58 requires this runtime's same retained widget owner.");
    // 00AB1EF9 ->00AA7970. No base bounds refresh occurs here.
    copy_size_pair(self.widget.transform.size, size);
    owner.recompose_00aa7220();
    // Reload after base publication; -1 alone skips the state update/rebuild.
    const auto index = self.icon.current_state;
    if (index == -1) return;
    require(index >= 0 && static_cast<std::size_t>(index) < self.icon.states.size(),
        "Icon size58 state index is outside the native signed16 state vector.");
    copy_size_pair(self.icon.states[static_cast<std::size_t>(index)].size, size);
    // 00AB1F4E current+8C ->00AB10D0 -> current+80, supported Icon00AB3CB0.
    rebuild_00ab3cb0(self.icon.current_state);
}
void GuiIconRuntime::rebuild_00ab3cb0(std::int16_t index) {
    auto& self = *impl_;
    self.icon.current_state = index;
    const auto* selected = self.icon.has_texture ? gui_icon_state_at(self.icon, index) : nullptr;
    require(!self.icon.has_texture || selected, "Icon state index is outside the native signed16 state vector.");
    GuiGeometryRebuildRequest request;
    request.vertex_count = 4;
    request.primitive = D3DPT_TRIANGLESTRIP;
    request.range_words = {0, 4, 0, 2};
    request.texture = selected ? self.logical(selected->texture) : nullptr;
    request.choose_material = [&](bool existing) {
        bool differs = false;
        const bool platform_point = self.icon.has_texture && self.icon.shader_name.empty()
            ? self.services.platform_allows_point_filter() : false;
        if (self.icon.has_texture && self.icon.shader_name.empty() &&
            platform_point && self.widget.transform.rotate == 0.0f &&
            self.widget.transform.scale_x == 1.0f && self.widget.transform.scale_y == 1.0f) {
            differs = gui_icon_state_differs_from_native_size_00ab24b0(*selected,
                self.texture_width(selected->texture), self.texture_height(selected->texture));
        }
        const bool bilinear = gui_icon_prefers_bilinear_filter_00ab2600(self.icon,
            self.widget.transform, platform_point, differs);
        const bool reuse = existing && bilinear == self.icon.cached_prefers_bilinear;
        if (!reuse) self.icon.cached_prefers_bilinear = bilinear;
        return GuiGeometryMaterialChoice{gui_icon_material_name(self.icon, bilinear), reuse};
    };
    request.prepare_vertices = [&](std::vector<GuiQuadVertex>& vertices, bool writes_enabled) {
        GuiIconQuadSetup setup;
        require(gui_icon_build_quad_setup(self.icon, self.widget.transform, setup),
            "Icon geometry cannot obtain its selected state.");
        self.widget.transform.size = setup.size;
        if (setup.writes_widget_pivot) {
            self.widget.transform.pivot_x = setup.pivot_x;
            self.widget.transform.pivot_y = setup.pivot_y;
        }
        if (!writes_enabled) return false;
        std::array<GuiQuadVertex, 4> quad;
        const bool valid = gui_write_cropped_quad_00ab1860(setup.quad, quad);
        self.icon.partial_display_ratio = setup.quad.ratio;
        if (valid) vertices.assign(quad.begin(), quad.end());
        return valid;
    };
    rebuild_gui_geometry_00ab3cb0_fragment(self.widget, self.overbright, self.services.geometry, request);
}
}
