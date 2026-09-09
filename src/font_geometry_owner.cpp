#include "bsp/font_geometry_owner.hpp"
#include "bsp/material_textures.hpp"
#include <algorithm>
#include <array>
#include <cstring>
#include <cwchar>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
bool finite_float(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}

float add_origin(float origin_value, float placement_value) noexcept {
    float sum_value;
    // Host origin is explicit; retain the caller's x87 environment and spill.
    __asm {
        fld origin_value
        fadd placement_value
        fstp sum_value
    }
    return sum_value;
}

FontGeometryLayout simplecolor_layout() noexcept {
    FontGeometryLayout value;
    value.stride = sizeof(FontGeometryVertex);
    value.uv_offset = 12;
    value.packed_color_offset = 20;
    return value;
}

void failure(std::string& error, const char* operation, HRESULT status) {
    error = std::string("Font geometry ") + operation + " failed with HRESULT " +
        std::to_string(static_cast<std::int32_t>(status)) + ".";
}

struct Generation {
    explicit Generation(D3D9StateCache& value) noexcept : cache(value) {}
    ~Generation() {
        // These registries contain borrowed pointers, so unregister before the
        // final shared stream/physical owners in geometry are destroyed.
        if (index_registered)
            cache.unregister_index_stream_00b4b390(
                *geometry.main.indices->physical, *geometry.main.indices);
        if (vertex_registered)
            cache.unregister_vertex_stream_00b4b3f0(
                *geometry.main.vertices->physical, *geometry.main.vertices);
    }
    D3D9StateCache& cache;
    bool vertex_registered{}, index_registered{};
    FontGeometrySnapshot geometry;
};
}

struct FontGeometryOwner::Impl {
    Impl(IDirect3DDevice9& borrowed_device, D3D9StateCache& borrowed_cache,
        FontGeometryOwnerResources resources)
        : device(borrowed_device), cache(borrowed_cache), font(std::move(resources.font)),
          vertex_shader{resources.vertex_shader}, pixel_shader{resources.pixel_shader},
          samplers(std::move(resources.samplers)), pixel_usage_mask(resources.pixel_usage_mask) {
        if (!font || !font->gfx || !font->alpha || !font->gfx->texture() ||
            !font->alpha->texture() || !vertex_shader.shader || !pixel_shader.shader ||
            !font_has_glyph_00ad4500(font->data, 0x0091))
            throw std::invalid_argument("Font geometry requires a decoded font, live images and prepared shaders.");
        std::uint32_t vertex = 16, pixel = 0;
        for (std::size_t i = 0; i < samplers.textures.size(); ++i) {
            const auto& entry = samplers.textures[i];
            if (entry.index < 0 || entry.index > 1 ||
                (entry.vertex_stage ? vertex >= 20 : pixel >= 16))
                throw std::invalid_argument("Font geometry supports material texture indices0/1 and native sampler capacities.");
            const auto slot = entry.vertex_stage ? vertex++ : pixel++;
            // Same source0 activation rule as00b43470's existing projection.
            if (entry.vertex_stage || (pixel_usage_mask & (1u << (i & 31))))
                texture_slots |= 1u << slot;
        }
        auto gfx = std::make_shared<LogicalTexture>();
        auto alpha = std::make_shared<LogicalTexture>();
        gfx->texture = font->gfx->texture();
        alpha->texture = font->alpha->texture();
        set_material_texture_00b189f0(main_material, 0, gfx);
        set_material_texture_00b189f0(main_material, 1, alpha);
        set_material_texture_00b189f0(shadow_material, 0, gfx);
        metric.height = font->data.scaled_height;
        // Last potentially failing constructor allocations precede COM retains.
        vertex_shader.shader->AddRef();
        pixel_shader.shader->AddRef();
    }

    ~Impl() {
        unbind();
        generation.reset();
        pixel_shader.shader->Release();
        vertex_shader.shader->Release();
        // Material projections are destroyed before font (member order).
    }

    HRESULT unbind() noexcept {
        if (!bound) return S_FALSE;
        HRESULT first = S_OK;
        const auto remember = [&](HRESULT value) {
            if (FAILED(value) && SUCCEEDED(first)) first = value;
        };
        // Exclusive binding scope: clearing these slots is deliberate, not
        // restoration of whatever another cache user might have installed.
        remember(cache.bind_vertex_shader_00b21d10(nullptr));
        remember(cache.bind_pixel_shader_00b21c20(nullptr));
        for (UINT slot = 0; slot < 20; ++slot)
            if (texture_slots & (1u << slot))
                remember(cache.bind_texture_00b24710(slot, {}));
        cache.bind_vertex_stream_00b24840(0, {});
        cache.bind_index_stream_00b24b00({}, 0);
        remember(cache.bind_vertex_layout_00b23f20({}));
        // Native cache's null layout assignment does not issue the device call;
        // explicit host scope cleanup also drops the device's layout reference.
        remember(device.SetVertexDeclaration(nullptr));
        bound = false;
        return first;
    }

    HRESULT bind(bool shadow) {
        if (!generation || generation->geometry.main.vertex_count == 0) return S_FALSE;
        bound = true; // Includes partial failures, so cleanup still clears them.
        HRESULT result;
        // Equal-COM binding preserves the OLD logical wrapper in this cache.
        // Clear a different wrapper first so our stable heap address is adopted.
        if (cache.vertex_shader() != &vertex_shader) {
            result = cache.bind_vertex_shader_00b21d10(nullptr);
            if (FAILED(result)) return result;
        }
        result = cache.bind_vertex_shader_00b21d10(&vertex_shader);
        if (FAILED(result)) return result;
        if (cache.pixel_shader() != &pixel_shader) {
            result = cache.bind_pixel_shader_00b21c20(nullptr);
            if (FAILED(result)) return result;
        }
        result = cache.bind_pixel_shader_00b21c20(&pixel_shader);
        if (FAILED(result)) return result;
        result = bind_material_textures_00b43470(cache, samplers,
            shadow ? shadow_material.textures() : main_material.textures(), pixel_usage_mask);
        if (FAILED(result)) return result;
        const auto& range = shadow ? generation->geometry.shadow : generation->geometry.main;
        cache.bind_vertex_stream_00b24840(0, range.vertices);
        cache.bind_index_stream_00b24b00(range.indices, range.base_vertex);
        result = cache.bind_vertex_layout_00b23f20(range.layout);
        return FAILED(result) ? result : S_OK;
    }

    bool prepare_cpu(Generation& candidate, std::u16string_view input,
        const FontGeometryUpdateParameters& p, FontGeometryMetrics& next_metric,
        std::string& error) {
        if (!finite_float(p.origin_x) || !finite_float(p.origin_y)) {
            error = "Font geometry requires finite host origins.";
            return false;
        }
        auto& geometry = candidate.geometry;
        float width_scale, vertical_scale, vertical_offset = 0;
        if (p.multiline) {
            FontWrappedLayout layout;
            if (!build_font_wrapped_00aba270_fragment(font->data, input, p.wrapped, layout, error))
                return false;
            width_scale = p.wrapped.width_scale;
            vertical_scale = p.wrapped.vertical_scale;
            vertical_offset = layout.normalized_vertical_offset;
            next_metric.measured_width = layout.measured_width;
            next_metric.line_count = static_cast<std::uint32_t>(layout.lines.size());
            next_metric.wrapped_height = layout.measured_height;
            next_metric.normalized_wrapped_height = layout.normalized_height;
            next_metric.normalized_vertical_offset = vertical_offset;
            next_metric.container_width = layout.container_width;
            next_metric.initial_x = layout.placements.front().x;
            geometry.placements = std::move(layout.placements);
            geometry.lines = std::move(layout.lines);
        } else {
            if (!finite_float(p.single_line_vertical_scale)) {
                error = "Font geometry requires a finite vertical scale.";
                return false;
            }
            FontSingleLineLayout layout;
            if (!build_font_single_line_00ab9fd0_fragment(font->data, input, p.single_line, layout, error))
                return false;
            width_scale = p.single_line.width_scale;
            vertical_scale = p.single_line_vertical_scale;
            next_metric.measured_width = layout.measured_width;
            next_metric.line_count = 0;
            next_metric.initial_x = layout.initial_x;
            next_metric.container_width = layout.container_width;
            next_metric.normalized_vertical_offset = 0;
            // Native single-line builder does not write context+178.
            geometry.placements = std::move(layout.placements);
        }
        if (geometry.placements.empty() || geometry.placements.size() > input.size()) {
            error = "Font geometry emission count exceeds the native text allocation.";
            return false;
        }
        const auto capacity = static_cast<std::uint32_t>(input.size());
        geometry.vertices.resize(capacity * 4u);
        geometry.indices.resize(capacity * 6u);
        const auto layout = simplecolor_layout();
        for (std::uint32_t i = 0; i < geometry.placements.size(); ++i) {
            const auto& point = geometry.placements[i];
            const FontGeometryParameters placement{add_origin(p.origin_x, point.x),
                add_origin(p.origin_y, point.y), width_scale, vertical_scale, metric.height, i};
            if (!finite_float(placement.x) || !finite_float(placement.y)) {
                error = "Font geometry origin plus placement exceeds the finite domain.";
                return false;
            }
            const auto& glyph = select_font_glyph_00ad4480(font->data, point.code_unit);
            std::array<std::uint16_t, 6> quad_indices;
            if (!write_font_quad_00ab98f0_fragment(glyph, placement, layout,
                reinterpret_cast<std::uint8_t*>(geometry.vertices.data()),
                geometry.vertices.size() * sizeof(FontGeometryVertex), i * 4u, quad_indices)) {
                error = "Font quad does not fit the checked simplecolor allocation.";
                return false;
            }
            std::copy(quad_indices.begin(), quad_indices.end(), geometry.indices.begin() + i * 6u);
            for (std::uint32_t corner = 0; corner < 4; ++corner) {
                auto& vertex = geometry.vertices[i * 4u + corner];
                if (p.multiline && !apply_font_wrapped_vertical_offset_00aba860_fragment(
                    vertex.y, vertical_offset, vertex.y)) {
                    error = "Font normalized vertical offset exceeds the finite domain.";
                    return false;
                }
                if (!finite_float(vertex.x) || !finite_float(vertex.y) || !finite_float(vertex.z) ||
                    !finite_float(vertex.u) || !finite_float(vertex.v)) {
                    error = "Font quad contains unsupported nonfinite coordinates.";
                    return false;
                }
            }
        }
        geometry.main.vertex_count = static_cast<UINT>(geometry.placements.size() * 4);
        geometry.main.primitive_count = static_cast<UINT>(geometry.placements.size() * 2);
        return true;
    }

    bool upload(Generation& candidate, std::string& error) {
        auto& geometry = candidate.geometry;
        auto vertex = std::make_shared<LogicalVertexStream>();
        auto index = std::make_shared<LogicalIndexStream>();
        auto declaration = std::make_shared<VertexDeclaration>();
        declaration->append_00b48330(D3DDECLTYPE_FLOAT3, D3DDECLUSAGE_POSITION);
        declaration->append_00b48330(D3DDECLTYPE_FLOAT2, D3DDECLUSAGE_TEXCOORD);
        declaration->append_00b48330(D3DDECLTYPE_D3DCOLOR, D3DDECLUSAGE_COLOR);
        if (declaration->stride != sizeof(FontGeometryVertex)) {
            error = "Font simplecolor declaration and CPU vertex stride disagree.";
            return false;
        }
        vertex->declaration = declaration;
        vertex->physical = std::make_shared<VertexBufferBinding>();
        vertex->physical->flags = 0x1000;
        vertex->physical->capacity = static_cast<UINT>(geometry.vertices.size() * sizeof(FontGeometryVertex));
        vertex->flags = 0x1000;
        vertex->tag = 0x40000001;
        index->physical = std::make_shared<IndexBufferBinding>();
        index->physical->flags = 0x1000;
        index->physical->capacity = static_cast<UINT>(geometry.indices.size() * sizeof(std::uint16_t));
        index->index_count = static_cast<UINT>(geometry.indices.size());
        auto layout = std::make_shared<D3D9VertexLayout>();
        layout->append_stream_00b48a00(declaration);
        geometry.main.vertices = vertex;
        geometry.main.indices = index;
        geometry.main.layout = layout;
        geometry.shadow = geometry.main;
        cache.register_logical_stream_00b4b1e0(*vertex->physical, *vertex);
        candidate.vertex_registered = true;
        cache.register_logical_stream_00b4b1e0(*index->physical, *index);
        candidate.index_registered = true;
        HRESULT result = vertex_buffer_recreate_00b492b0(*vertex->physical, device);
        if (FAILED(result)) { failure(error, "vertex allocation", result); return false; }
        result = index_buffer_recreate_00b49180(*index->physical, device);
        if (FAILED(result)) { failure(error, "index allocation", result); return false; }
        result = layout->create_if_missing_00b60a10(device);
        if (FAILED(result)) { failure(error, "vertex declaration creation", result); return false; }
        void* mapped_vertices = nullptr;
        result = cache.lock_vertex_stream_00b49980(*vertex,
            static_cast<UINT>(geometry.vertices.size()), 0, false, mapped_vertices);
        if (FAILED(result) || !mapped_vertices) {
            // Existing lock increments depth even on API failure. Discard that
            // candidate; only an API-successful lock needs an Unlock call.
            if (SUCCEEDED(result)) cache.unlock_vertex_stream_00b49a80(*vertex);
            failure(error, "vertex lock", FAILED(result) ? result : D3DERR_INVALIDCALL);
            return false;
        }
        void* mapped_indices = nullptr;
        result = cache.lock_index_stream_00b49b60(*index, index->index_count, 0, false, mapped_indices);
        if (FAILED(result) || !mapped_indices) {
            if (SUCCEEDED(result)) cache.unlock_index_stream_00b49c70(*index);
            cache.unlock_vertex_stream_00b49a80(*vertex);
            failure(error, "index lock", FAILED(result) ? result : D3DERR_INVALIDCALL);
            return false;
        }
        std::memcpy(mapped_vertices, geometry.vertices.data(), vertex->physical->capacity);
        std::memcpy(mapped_indices, geometry.indices.data(), index->physical->capacity);
        // Native builders unlock indices before vertices.
        cache.unlock_index_stream_00b49c70(*index);
        cache.unlock_vertex_stream_00b49a80(*vertex);
        return true;
    }

    FontGeometryUpdate update(std::u16string_view input,
        const FontGeometryUpdateParameters& p, bool force, std::string& error) {
        error.clear();
        if (input.size() > 16384 || input.find(u'\0') != std::u16string_view::npos) {
            error = "Font geometry requires at most16384 coherent UTF16 code units without embedded NUL.";
            return FontGeometryUpdate::unsupported;
        }
        std::u16string next_text(input);
        std::wstring next_wide(next_text.begin(), next_text.end());
        if ((!force || input.empty()) && _wcsicmp(cached_wide.c_str(), next_wide.c_str()) == 0)
            return FontGeometryUpdate::unchanged;
        if (input.empty()) {
            cached_text = std::move(next_text);
            cached_wide.clear();
            metric.measured_width = 0;
            if (generation) {
                generation->geometry.main.vertex_count = 0;
                generation->geometry.main.primitive_count = 0;
                generation->geometry.shadow.vertex_count = 0;
                generation->geometry.shadow.primitive_count = 0;
            }
            return FontGeometryUpdate::cleared;
        }
        auto candidate = std::make_unique<Generation>(cache);
        auto next_metric = metric;
        if (!prepare_cpu(*candidate, input, p, next_metric, error) || !upload(*candidate, error))
            return FontGeometryUpdate::unsupported;
        const HRESULT detached = unbind();
        if (FAILED(detached)) {
            failure(error, "previous binding cleanup", detached);
            return FontGeometryUpdate::unsupported;
        }
        generation = std::move(candidate);
        cached_text = std::move(next_text);
        cached_wide = std::move(next_wide);
        metric = next_metric;
        return FontGeometryUpdate::rebuilt;
    }

    IDirect3DDevice9& device;
    D3D9StateCache& cache;
    std::shared_ptr<const FontResources> font;
    LogicalVertexShader vertex_shader;
    LogicalPixelShader pixel_shader;
    MaterialSamplerPass samplers;
    std::uint32_t pixel_usage_mask{}, texture_slots{};
    MaterialTextureSlots main_material, shadow_material;
    std::u16string cached_text;
    std::wstring cached_wide;
    FontGeometryMetrics metric;
    std::unique_ptr<Generation> generation;
    bool bound{};
};

FontGeometryOwner::FontGeometryOwner(IDirect3DDevice9& device, D3D9StateCache& cache,
    FontGeometryOwnerResources resources)
    : impl_(std::make_unique<Impl>(device, cache, std::move(resources))) {}
FontGeometryOwner::~FontGeometryOwner() = default;
FontGeometryOwner::FontGeometryOwner(FontGeometryOwner&&) noexcept = default;
FontGeometryOwner& FontGeometryOwner::operator=(FontGeometryOwner&&) noexcept = default;

FontGeometryUpdate FontGeometryOwner::update_00aba8d0_fragment(std::u16string_view text,
    const FontGeometryUpdateParameters& parameters, std::string& error) {
    if (!impl_) { error = "Font geometry owner was moved from."; return FontGeometryUpdate::unsupported; }
    return impl_->update(text, parameters, false, error);
}
FontGeometryUpdate FontGeometryOwner::rebuild_00abb1d0_fragment(
    const FontGeometryUpdateParameters& parameters, std::string& error) {
    if (!impl_) { error = "Font geometry owner was moved from."; return FontGeometryUpdate::unsupported; }
    return impl_->update(impl_->cached_text, parameters, true, error);
}
HRESULT FontGeometryOwner::bind(bool shadow) {
    return impl_ ? impl_->bind(shadow) : D3DERR_INVALIDCALL;
}
HRESULT FontGeometryOwner::unbind() noexcept { return impl_ ? impl_->unbind() : S_FALSE; }
const FontGeometrySnapshot& FontGeometryOwner::geometry() const noexcept {
    static const FontGeometrySnapshot empty;
    return impl_ && impl_->generation ? impl_->generation->geometry : empty;
}
const FontGeometryMetrics& FontGeometryOwner::metrics() const noexcept {
    static const FontGeometryMetrics empty;
    return impl_ ? impl_->metric : empty;
}
std::u16string_view FontGeometryOwner::text() const noexcept {
    return impl_ ? std::u16string_view(impl_->cached_text) : std::u16string_view{};
}
}
