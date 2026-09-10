#include "bsp/material_constant_builder.hpp"
#include "bsp/material_transform_constants.hpp"
#include "bsp/material_scalar_constants.hpp"
#include "bsp/material_point_light_constants.hpp"
#include "bsp/camera_multiply.hpp"
#include <cstring>
#include <limits>

namespace bsp {
namespace {
bool need(bool condition, const char* message, std::string& error) {
    if (!condition) error = message;
    return condition;
}
bool span(std::vector<float>& bank, std::size_t first, std::size_t words,
    std::string& error) {
    return need(first <= bank.size() && words <= bank.size() - first,
        "Material constant write exceeds the actual shared bank", error);
}
void x87_copy(float* destination, const void* source) {
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
void matrix_rows(float* destination, const void* source, unsigned rows) {
    const auto* bytes = static_cast<const unsigned char*>(source);
    if (rows == 2) {
        std::uint32_t staged[8];
        for (unsigned i = 0; i < 8; ++i)
            std::memcpy(staged + i, bytes + ((i % 4) * 4 + i / 4) * 4, 4);
        std::memcpy(destination, staged, sizeof(staged));
    } else {
        for (unsigned i = 0; i < rows * 4; ++i)
            x87_copy(destination + i, bytes + ((i % 4) * 4 + i / 4) * 4);
    }
}
bool raw_copy(std::vector<float>& bank, std::size_t first, const void* source,
    std::size_t bytes, std::size_t available, std::string& error) {
    if (!bytes) return true; // Native zero-byte copy touches neither address.
    if (!need(bytes <= available && (bytes == 0 || source),
        "Material raw constant source is missing or short", error)
        || !span(bank, first, bytes / 4, error)) return false;
    const auto input = reinterpret_cast<std::uintptr_t>(source);
    const auto output = reinterpret_cast<std::uintptr_t>(bank.data() + first);
    if (!need(input <= UINTPTR_MAX - bytes && output <= UINTPTR_MAX - bytes,
        "Material raw-copy address range overflows", error)) return false;
    //00BF7680 takes a backward-copy path at00BF7844 for overlapping ranges,
    // despite the saved _memcpy symbol. Self-copy is valid as well.
    std::memmove(bank.data() + first, source, bytes);
    return true;
}
void accumulate(HRESULT value, HRESULT& result) {
    if (FAILED(value) && SUCCEEDED(result)) result = value;
}
bool bind_texture_stage(D3D9StateCache* renderer, std::uint32_t slot,
    const std::shared_ptr<LogicalTexture>& texture, HRESULT& result, std::string& error) {
    if (!need(renderer != nullptr, "Material texture has no current renderer", error)) return false;
    accumulate(renderer->bind_texture_00b24710(slot, texture), result);
    return true;
}
bool write_bones(const CompiledMaterialPass& pass, InstanceRenderEntry& entry,
    MaterialEntryConstantState& state, MaterialConstantOwners& owners, std::string& error) {
    if (pass.vb.registers[40] == 0xff) return true;
    MaterialBoneModel* model{};
    if (!owners.bone_model(entry, model, error)
        || !need(model != nullptr, "Selected bone constants have no actual bone model", error)) return false;
    const auto count = model->count_188;
    auto destination = std::size_t(pass.vb.registers[40]) * 4;
    for (std::int32_t i = 0; i < count; ++i) {
        const auto index = static_cast<std::size_t>(i);
        if (!need(index < model->nodes_184.size() && model->nodes_184[index],
            "Actual bone model pointer array is short or null", error)) return false;
        auto& transform = *model->nodes_184[index];
        if (!(transform.valid_flags & 2)) refresh_camera_world_00b6db70(transform);
        if (!span(state.vertex_words, destination, 12, error)) return false;
        matrix_rows(state.vertex_words.data() + destination, transform.world.data(), 3);
        destination += 12;
    }
    return true;
}
bool write_skin(const CompiledMaterialPass& pass, InstanceRenderEntry& entry,
    MaterialEntryConstantState& state, MaterialConstantBuilderEnvironment& environment,
    std::string& error) {
    const auto initial_register = pass.vb.registers[42];
    if (initial_register == 0xff) return true;
    MaterialSkinModel* model{};
    if (!environment.owners.skin_model(entry, model, error)
        || !need(model != nullptr, "Selected skin constants have no actual skin model", error)) return false;
    auto* animator = model->animator_130;
    if (animator) {
        bool accepted{};
        if (!need(environment.optimized_animator_type_010900fc != nullptr,
            "Skin animator predicate requires its actual runtime type ID", error)) return false;
        if (!animator->accepts_type_0c(*environment.optimized_animator_type_010900fc,
            accepted, error)) return false;
        if (accepted) {
            const void* palette = model->palette_190;
            if (!palette) return true;
            std::uint32_t count{};
            if (!animator->palette_count_34_08(count, error)) return false;
            const std::uint32_t bytes = count * 48u; // native DWORD arithmetic
            return raw_copy(state.vertex_words, std::size_t(initial_register) * 4,
                palette, bytes, model->available_palette_bytes, error);
        }
    }
    const auto count = model->count_188;
    auto destination = std::size_t(pass.vb.registers[42]) * 4; // reload after predicate
    for (std::int32_t i = 0; i < count; ++i) {
        const auto index = static_cast<std::size_t>(i);
        if (!need(index < model->bones_184.size() && model->bones_184[index].transform,
            "Actual skin model record array is short or null", error)) return false;
        auto& bone = *model->bones_184[index].transform;
        if (!(bone.valid_flags & 2)) refresh_camera_world_00b6db70(bone);
        const auto& inverse = get_transform_inverse_world_00b6e0d0(model->transform);
        CameraMatrix intermediate, result;
        multiply_camera_matrices_00413920(intermediate, model->bones_184[index].inverse_bind_20, bone.world);
        multiply_camera_matrices_00413920(result, intermediate, inverse);
        if (!span(state.vertex_words, destination, 12, error)) return false;
        matrix_rows(state.vertex_words.data() + destination, result.data(), 3);
        destination += 12;
    }
    return true;
}
}

bool write_material_parameters_00b423c5(const MaterialParameterBindings& table,
    const CompiledMaterialPass& pass, std::uint32_t selector,
    MaterialEntryConstantState& state, std::string& error) {
    if (!need(selector < material_parameter_selector_count,
        "Material parameter camera selector exceeds fourteen modes", error)) return false;
    for (std::size_t index = 0; index < table.size(); ++index) {
        const auto* record = table.record(index);
        if (!need(record != nullptr, "Actual material parameter record is null", error)) return false;
        for (unsigned stage = 0; stage < 2; ++stage) {
            record = table.record(index); // native pointer/record reload before PS
            if (!need(record != nullptr, "Material parameter record disappeared", error)) return false;
            const auto selected = stage ? record->pixel_registers[selector] : record->vertex_registers[selector];
            if (selected < 0) continue;
            auto& bank = stage ? state.pixel_words : state.vertex_words;
            const auto destination = std::size_t(selected) * 4;
            const auto& source = record->source;
            if (!source.matrix) {
                if (!raw_copy(bank, destination, source.words, std::size_t(source.word_count) * 4,
                    source.available_words * 4, error)) return false;
                continue;
            }
            unsigned rows = stage ? 4u : 0u;
            if (!stage) for (const auto& metadata : pass.vb.material_constants)
                if (metadata.register_index == static_cast<std::uint32_t>(selected)) rows = metadata.register_count;
            if (rows < 2 || rows > 4) continue;
            if (!need(source.words && source.available_words >= 16,
                "Selected material matrix source is missing or short", error)
                || !span(bank, destination, rows * 4, error)) return false;
            matrix_rows(bank.data() + destination, source.words, rows);
        }
    }
    error.clear();
    return true;
}

bool build_material_constants_00b42350(CompiledMaterialPass& pass, InstanceRenderEntry& entry,
    MaterialEntryOverride*, MaterialEntryConstantState& state,
    MaterialConstantBuilderEnvironment& environment, HRESULT& device_result, std::string& error) {
    device_result = D3D_OK;
    auto& owners = environment.owners;
    MaterialConstantCallbackBlocks blocks{environment.vertex_header_0108ebf0,
        environment.pixel_header_0108dbe8, state};
    for (std::uint32_t index = 0; index < pass.dynamic_source_count; ++index) {
        if (!need(index < pass.dynamic_sources.size() && pass.dynamic_sources[index]
            && pass.dynamic_sources[index]->source, "Actual dynamic texture slot/source is missing", error)) return false;
        auto* source = pass.dynamic_sources[index]->source.get();
        auto* renderer = environment.renderer; // captured BEFORE virtual+2C
        std::shared_ptr<LogicalTexture> texture;
        if (!source->texture_2c(texture, error)) return false;
        if (!need(pass.dynamic_sources[index] != nullptr, "Dynamic texture binding disappeared", error)
            || !bind_texture_stage(renderer, pass.dynamic_sources[index]->slot, texture, device_result, error)) return false;
        if (!need(pass.dynamic_sources[index] && pass.dynamic_sources[index]->source,
            "Dynamic texture callback source disappeared", error)) return false;
        source = pass.dynamic_sources[index]->source.get();
        if (!source->constants_30(entry, pass.vb, pass.pb, blocks, error)) return false;
    }
    MaterialCloneState* material{};
    std::uint32_t selector{};
    if (!owners.material(entry, material, error)
        || !need(material != nullptr, "Material builder has no actual material", error)
        || !owners.camera_mode(entry, selector, error)
        || !write_material_parameters_00b423c5(material->parameters, pass, selector, state, error)
        || !write_bones(pass, entry, state, owners, error)
        || !write_skin(pass, entry, state, environment, error)) return false;
    if (pass.vb.registers[24] != 0xff) {
        std::vector<const LogicalVertexStream*> streams;
        MeshDecodeDescriptorLimits descriptor;
        MeshDecodeBindingStats statistics;
        if (!owners.camera_mode(entry, selector, error)
            || !owners.decode_inputs(entry, pass, streams, descriptor, error)
            || !pack_mesh_vertex_decode_constants_00b428c0(streams, pass.vb, descriptor,
                selector, state.vertex_words, statistics, error)) return false;
    }
    CameraTransform* transform{};
    if (pass.vb.registers[0] != 0xff || pass.pb.registers[0] != 0xff) {
        if (!owners.transform(entry, transform, error)
            || !need(transform != nullptr, "World constants have no actual model transform", error)
            || !pack_material_world_constants_00b42a7c(pass, *transform,
                state.vertex_words, state.pixel_words, error)) return false;
    }
    const float* threshold{};
    if (pass.pb.registers[44] != 0xff && !owners.lod_threshold(entry, threshold, error)) return false;
    if (!write_material_visibility_lod_00b42e4a(pass, entry, threshold, state, error)) return false;
    if (pass.vb.registers[1] != 0xff) {
        if (!owners.transform(entry, transform, error)
            || !need(transform != nullptr, "Inverse constants have no actual model transform", error)
            || !pack_material_inverse_world_constants_00b42ef9(pass, *transform,
                state.vertex_words, error)) return false;
    }
    material = nullptr;
    if ((pass.pb.registers[47] != 0xff || pass.vb.registers[47] != 0xff)
        && (!owners.material(entry, material, error)
            || !need(material != nullptr, "Diffuse constants have no actual material", error))) return false;
    if (!write_material_diffuse_00b4305d(pass, material ? &material->lighting : nullptr, state, error)) return false;
    if (pass.shadow_samplers.shadow_texture_slot != -1) {
        auto* renderer = environment.renderer;
        std::shared_ptr<LogicalTexture> texture;
        if (!owners.shadow_texture(texture, error)
            || !bind_texture_stage(renderer, static_cast<std::uint32_t>(pass.shadow_samplers.shadow_texture_slot),
                texture, device_result, error)) return false;
    }
    if (pass.shadow_samplers.shadow_map_slot != -1) {
        bool has_first{};
        MaterialShadowMapOwner* owner{};
        if (!owners.first_shadow_light(entry, has_first, owner, error)
            || !need(has_first, "Selected ShadowMap has an empty native first-light list", error)) return false;
        auto* renderer = environment.renderer;
        std::shared_ptr<LogicalTexture> texture;
        if (owner) {
            if (!owner->texture_08(texture, error)) return false;
        } else texture = pass.shadow_samplers.fallback_shadow_map;
        if (!bind_texture_stage(renderer, static_cast<std::uint32_t>(pass.shadow_samplers.shadow_map_slot),
            texture, device_result, error)) return false;
    }
    if (pass.vb.registers[45] != 0xff) {
        GeneratedModelLifetime* model{};
        if (!owners.model_lifetime(entry, model, error)
            || !need(model != nullptr, "Point constants have no actual model lifetime", error)
            || !write_material_point_light_constants_00b43160_fragment(pass,
                borrowed_point_lights_00b6dc50(*model), state.vertex_words, error)) return false;
    }
    error.clear();
    return true;
}
}
