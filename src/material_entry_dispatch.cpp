#include "bsp/material_entry_dispatch.hpp"
#include <cstring>
#include <utility>

namespace bsp {
namespace {
void remember_failure(HRESULT result, HRESULT& first) {
    if (FAILED(result) && SUCCEEDED(first)) first = result;
}
std::int32_t signed_word(std::uint32_t word) {
    std::int32_t result;
    static_assert(sizeof(result) == sizeof(word));
    std::memcpy(&result, &word, sizeof(result));
    return result;
}
std::uint32_t primitive_vertices(std::uint32_t primitive, std::uint32_t count) {
    switch (primitive) {
    case 1: return count;
    case 2: return count * 2u;
    case 3: return count + 1u;
    case 4: return count * 3u;
    case 5: case 6: return count + 2u;
    default: return 0;
    }
}
bool upload_span(const std::vector<float>& words, std::uint32_t first,
    std::uint32_t count, std::string& error) {
    if (first > words.size() / 4 || count > words.size() / 4 - first) {
        error = "Actual material constant block does not cover native upload span";
        return false;
    }
    return true;
}
}

MaterialEntryProgram::MaterialEntryProgram(std::shared_ptr<CompiledMaterialPass> value)
    : compiled(std::move(value)) {
    if (compiled) {
        vertex.shader = compiled->vertex;
        pixel.shader = compiled->pixel;
        sampler_states = std::shared_ptr<SamplerStateBlock>(compiled,
            &compiled->pass.sampler_states);
    }
}

bool MaterialEntryDispatcher::mode(const InstanceRenderEntry& entry,
    std::uint32_t& result, std::string& error) {
    if (!environment_.operations.read_camera_mode(entry, result, error)) return false;
    if (result >= 14) {
        error = "Camera mode exceeds the fourteen native material programs";
        return false;
    }
    return true;
}

bool MaterialEntryDispatcher::execute_material_entry(InstanceRenderEntry& entry,
    std::string& error) {
    error.clear();
    MaterialEntryEffect* effect = nullptr;
    auto& operations = environment_.operations;
    if (!operations.resolve_effect(entry, effect, error)) return false;
    if (!effect || !effect->owner) {
        error = "Material entry has no retained actual effect owner";
        return false;
    }
    std::uint32_t initial_mode;
    if (!mode(entry, initial_mode, error)) return false;
    HRESULT first_failure = S_OK;
    bool complete;
    if (initial_mode == 2 && effect->special_two) {
        complete = bind_and_draw(*effect, *effect->special_two, entry,
            nullptr, first_failure, error);
    } else {
        // COMISS(1, visibility), JBE: unordered values take the regular path.
        if (effect->final_lod_fade_out && entry.visibility < 1.0f && initial_mode == 6)
            return true;
        MaterialEntryOverrideCollection* collection = nullptr;
        if (!operations.capture_overrides(entry, collection, error)) return false;
        // Collection identity was captured BEFORE this callback in the native.
        if (effect->update_model_state && initial_mode == 0 &&
            !operations.update_model_and_renderer(entry, error)) return false;
        complete = true;
        if (!collection) {
            complete = select_and_draw(*effect, entry, nullptr, first_failure, error);
        } else if (collection->count() != 0) {
            std::uint32_t index = 0;
            do {
                auto* value = collection->front();
                if (!value) {
                    error = "Nonempty native override collection has no front value";
                    return false;
                }
                bool accepted = false;
                if (!value->accepts(environment_.override_context, accepted, error)) return false;
                if (accepted && !select_and_draw(*effect, entry, value, first_failure, error))
                    return false;
                ++index;
            } while (index < collection->count());
        }
    }
    if (!complete) return false;
    if (FAILED(first_failure)) {
        error = "Material entry completed with D3D9 HRESULT " +
            std::to_string(static_cast<long>(first_failure));
        return false;
    }
    return true;
}

bool MaterialEntryDispatcher::select_and_draw(MaterialEntryEffect& effect,
    InstanceRenderEntry& entry, MaterialEntryOverride* override_value,
    HRESULT& first_failure, std::string& error) {
    std::uint32_t current_mode;
    if (!mode(entry, current_mode, error)) return false;
    auto selected = current_mode == 0 && entry.visibility < 1.0f
        ? effect.alternate_zero : effect.modes[current_mode];
    return !selected || bind_and_draw(effect, *selected, entry, override_value,
        first_failure, error);
}

bool MaterialEntryDispatcher::bind_and_draw(MaterialEntryEffect& effect,
    MaterialEntryProgram& program, InstanceRenderEntry& entry,
    MaterialEntryOverride* override_value, HRESULT& first_failure, std::string& error) {
    auto& operations = environment_.operations;
    auto& renderer = environment_.renderer;
    MaterialEntryGeometry geometry;
    if (!operations.resolve_geometry(entry, geometry, error)) return false;
    if (geometry.streams.empty()) return true;
    if (!geometry.streams.front()) {
        error = "Material entry's first stream is null";
        return false;
    }
    if (geometry.streams.front()->offset == 0xffffffffu) return true;
    if (!geometry.section || !geometry.material || geometry.streams.size() > 4 ||
        (geometry.indices && !geometry.indices->physical) ||
        !program.compiled || program.compiled->effect_owner != effect.owner) {
        error = "Material entry geometry/program does not have valid actual owners";
        return false;
    }
    auto& pass = *program.compiled;
    if (program.vertex.shader != pass.vertex || program.pixel.shader != pass.pixel) {
        error = "Compiled shader changed while its borrowed program wrapper was retained";
        return false;
    }
    const bool instanced = geometry.section->instance_count != 0;
    for (UINT i = 0; i < geometry.streams.size(); ++i) {
        const auto& stream = geometry.streams[i];
        if (!stream || !stream->physical || !stream->declaration) {
            error = "Material entry has an incomplete active vertex stream owner";
            return false;
        }
        renderer.bind_vertex_stream_00b24840(i, stream);
        if (instanced) {
            const DWORD frequency = stream->tag == 0x80000000u ? stream->tag | 1u
                : stream->tag | geometry.section->instance_count;
            renderer.set_stream_frequency_00b24a40(i, frequency);
        }
    }
    if (!instanced) {
        renderer.set_stream_frequency_00b24a40(0, 1);
        renderer.set_stream_frequency_00b24a40(1, 1);
    }
    if (environment_.material_effect_owner != effect.owner.get()) {
        // Native publishes effect identity BEFORE the clip operation.
        environment_.material_effect_owner = effect.owner.get();
        std::uint32_t current_mode;
        if (!mode(entry, current_mode, error)) return false;
        const float distance = effect.clip_distance[current_mode];
        if (!effect.extra_clip_plane || !(distance > 0.0f)) {
            if (!operations.restore_pending_planes_00b25080(error)) return false;
        } else if (!operations.construct_and_append_effect_plane(effect, entry, distance, error)) {
            return false;
        }
    }
    remember_failure(renderer.bind_vertex_layout_00b23f20(geometry.layout), first_failure);
    const std::uint32_t index_base = geometry.indices ? geometry.indices->base_index : 0;
    renderer.bind_index_stream_00b24b00(geometry.indices,
        signed_word(geometry.streams.front()->base_vertex));
    const bool indexed = geometry.indices && geometry.indexed;
    const std::uint32_t vertex_base = geometry.streams.front()->base_vertex;

    //00B43410: states, shader objects and ordered static texture references.
    renderer.bind_render_state_block_00b27a80(pass.states);
    renderer.bind_sampler_state_block_00b27b90(program.sampler_states);
    if (signed_word(geometry.material->word104) >= 0)
        renderer.set_render_state_00b24460(D3DRS_ALPHAREF, geometry.material->word104);
    remember_failure(renderer.bind_vertex_shader_00b21d10(&program.vertex), first_failure);
    remember_failure(renderer.bind_pixel_shader_00b21c20(&program.pixel), first_failure);
    UINT pixel_sampler = 0, vertex_sampler = 16;
    const std::uint32_t usage = pass.pb.sampler_mask;
    for (std::size_t i = 0; i < pass.pass.textures.size(); ++i) {
        const auto& reference = pass.pass.textures[i];
        if (!reference.vertex_stage && (usage & (1u << (i & 31))) == 0) {
            ++pixel_sampler;
            continue;
        }
        std::shared_ptr<LogicalTexture> texture;
        if (reference.index < 0) {
            const std::uint32_t slot = 0xffffffffu - static_cast<std::uint32_t>(reference.index);
            if (!operations.effect_texture_00b17d90(effect, slot, texture, error)) return false;
        } else if (reference.index < geometry.material->textures.count()) {
            texture = geometry.material->textures.textures()[static_cast<std::size_t>(reference.index)];
        }
        UINT& sampler = reference.vertex_stage ? vertex_sampler : pixel_sampler;
        if (sampler >= (reference.vertex_stage ? 20u : 16u)) {
            error = "Material texture reference exceeds the native renderer sampler banks";
            return false;
        }
        remember_failure(renderer.bind_texture_00b24710(sampler++, std::move(texture)), first_failure);
    }
    HRESULT constant_failure = S_OK;
    if (!operations.build_constants_00b42350(program, entry, override_value,
        environment_.constants, constant_failure, error)) return false;
    remember_failure(constant_failure, first_failure);
    auto& constants = environment_.constants;
    std::uint32_t first = constants.first_register;
    const std::int32_t vertex_count = signed_word(static_cast<std::uint32_t>(pass.vb.end_register) - first);
    const std::int32_t pixel_count = signed_word(static_cast<std::uint32_t>(pass.pb.end_register) - first);
    if (vertex_count > 0) {
        const auto count = static_cast<std::uint32_t>(vertex_count);
        if (!upload_span(constants.vertex_words, first, count, error)) return false;
        remember_failure(renderer.set_vertex_shader_constants_f_00b21820(first,
            constants.vertex_words.data() + static_cast<std::size_t>(first) * 4, count), first_failure);
        first = constants.first_register;
    }
    // Native reloads the start and PS end for the gate but pushes the OLD PS count.
    if (signed_word(static_cast<std::uint32_t>(pass.pb.end_register) - first) > 0) {
        const auto count = static_cast<std::uint32_t>(pixel_count);
        if (!upload_span(constants.pixel_words, first, count, error)) return false;
        remember_failure(renderer.set_pixel_shader_constants_f_00b218c0(first,
            constants.pixel_words.data() + static_cast<std::size_t>(first) * 4, count), first_failure);
    }
    if (geometry.material->word08 &&
        !operations.material_callback(*geometry.material, entry, error)) return false;
    auto& section = *geometry.section;
    const auto primitive = static_cast<D3DPRIMITIVETYPE>(section.primitive);
    if (indexed) {
        remember_failure(renderer.draw_indexed_00b24010(environment_.draw, primitive,
            section.range_words[0], section.range_words[1], section.range_words[2] + index_base,
            section.range_words[3]), first_failure);
    } else {
        remember_failure(renderer.draw_primitive_00b21b40(environment_.draw, primitive,
            section.range_words[0] + vertex_base, section.range_words[3]), first_failure);
    }
    MaterialEntryDrawStatistics statistics;
    if (!mode(entry, statistics.mode, error)) return false;
    statistics.primitives = section.range_words[3];
    statistics.vertices = indexed ? section.range_words[1]
        : primitive_vertices(section.primitive, section.range_words[3]);
    statistics.vertex_registers = vertex_count;
    statistics.pixel_registers = pixel_count;
    return operations.record_statistics_00b16f80(*effect.owner, statistics, error);
}
}
