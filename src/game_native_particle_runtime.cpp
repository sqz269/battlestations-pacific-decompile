#include "bsp/game_native_particle_runtime.hpp"

#include "bsp/native_particle_axial_raw.hpp"
#include "bsp/native_particle_cone_raw.hpp"
#include "bsp/native_particle_emitter_construction.hpp"
#include "bsp/native_particle_emitter_factory_raw.hpp"
#include "bsp/native_particle_object_raw.hpp"
#include "bsp/native_particle_object_resources_raw.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_parameter_runtime_loading.hpp"
#include "bsp/native_particle_resource_acquisition_raw.hpp"
#include "bsp/native_particle_resource_cache.hpp"
#include "bsp/native_particle_resource_loader_raw.hpp"
#include "bsp/native_particle_resource_parser_raw.hpp"
#include "bsp/native_particle_smartarea_raw.hpp"
#include "bsp/native_particle_sphere_raw.hpp"
#include "bsp/native_particle_sprite_floating_raw.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_particle_tracer_raw.hpp"
#include "bsp/native_particle_type_construction.hpp"
#include "bsp/native_particle_type_factory_raw.hpp"
#include "bsp/native_particle_type_lifetime.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_type_property_raw.hpp"
#include "bsp/native_particle_type_texture_raw.hpp"
#include "bsp/native_resource_container_lifetime.hpp"
#include "bsp/native_resource_load_cache.hpp"
#include "bsp/native_resource_manager_lifetime.hpp"
#include "bsp/native_texture_loading_cache.hpp"

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace bsp::game {
namespace {
template<class T>
const volatile T* required(GameNativeReadOnlyData& data, std::uintptr_t address,
    std::size_t count = 1) {
    return static_cast<const volatile T*>(data.data_at(address, sizeof(T) * count));
}

template<class T>
const T* required_plain(GameNativeReadOnlyData& data, std::uintptr_t address,
    std::size_t count = 1) {
    return static_cast<const T*>(data.data_at(address, sizeof(T) * count));
}

template<class A, class B>
bool same_cell(A& left, B& right) noexcept {
    const volatile void* const left_address = std::addressof(left);
    const volatile void* const right_address = std::addressof(right);
    return left_address == right_address;
}

GameNativeParticleRuntimeInputs checked(GameNativeParticleRuntimeInputs inputs) {
    if (std::addressof(inputs.resource_manager.strings) != std::addressof(inputs.strings))
        throw std::invalid_argument("particle runtime resource manager uses a different string domain");
    if (!same_cell(inputs.resource_manager.actual_lifetime_manager_01090aa0,
            inputs.strings.actual_manager_publication_01090aa0))
        throw std::invalid_argument("particle runtime string and resource managers use different lifetime publications");
    if (!same_cell(inputs.resource_load_cache.actual_vfs_0109ceec,
            inputs.vfs.actual_vfs_publication_0109ceec) ||
        std::addressof(inputs.resource_load_cache.names) !=
            std::addressof(inputs.vfs.name_resolution))
        throw std::invalid_argument("particle runtime resource cache uses a different VFS domain");
    if (std::addressof(inputs.texture_cache.dates) != std::addressof(inputs.vfs.dates))
        throw std::invalid_argument("particle runtime texture cache uses a different VFS date domain");
    if (!inputs.bound_platform_identity || !inputs.actual_text_scratch_00f8c2c8 ||
        !inputs.empty_particle_name_00f8766c || !inputs.empty_atlas_stem_00f8c2c1 ||
        !inputs.empty_model_stem_00f8d320 || !inputs.empty_texture_name_00f8d37c ||
        !inputs.empty_tracer_frame_name_00f8d390 || !inputs.null_pattern_00e17bf0)
        throw std::invalid_argument("particle runtime requires its actual platform, scratch and literal bindings");
    return inputs;
}
}

struct GameNativeParticleRuntime::Impl {
    const GameNativeParticleRuntimeInputs inputs;
    NativeParticleTypeLifetimeContext type_lifetime;
    NativeParticleEmitterConstructionContext emitter_construction;
    NativeParticleParameterBuilderRawContext parameter_builder;
    NativeParticleParameterRuntimeRawContext parameter_runtime;
    NativeParticleTypeParameterRawContext type_parameters;
    NativeParticleTextureNamesRawContext texture_names;
    NativeParticleTypeTextureRawContext type_texture;
    NativeParticleTypePropertyRawContext type_properties;
    NativeParticleSpriteFloatingRawContext sprite_floating;
    NativeParticleAxialAxisRawContext axial_axis;
    NativeParticleAxialRawContext axial;
    NativeParticleObjectResourcesRawContext object_resources;
    NativeParticleObjectRawContext object;
    NativeParticleTracerRawContext tracer;
    NativeParticleTypeConstructionContext type_construction;
    NativeParticleTypeFactoryRawContext type_factory;
    NativeParticleSphereRawContext sphere;
    NativeParticleConeRawContext cone;
    NativeParticleSmartAreaRawContext smartarea;
    NativeParticleEmitterFactoryRawContext emitter_factory;
    NativeParticleResourceParserRawContext parser;
    NativeParticleResourceLoaderRawContext loader;
    NativeParticleResourceCacheContext resource_cache;
    NativeParticleResourceAcquisitionRawContext acquisition;

    explicit Impl(const GameNativeParticleRuntimeInputs& source)
        : inputs(checked(source)),
          type_lifetime{inputs.strings, inputs.parameter_pool_00f8d344,
              &inputs.resource_references},
          emitter_construction{type_lifetime},
          parameter_builder{inputs.strings, inputs.crt,
              required<double>(inputs.data, 0x00ce3928),
              required<float>(inputs.data, 0x00ce3cb4),
              required<double>(inputs.data, 0x00d7a3a0),
              required<float>(inputs.data, 0x00d7a2f0),
              required<float>(inputs.data, 0x00ce3d08),
              required<float>(inputs.data, 0x00ce65d8),
              required<float>(inputs.data, 0x00d7a218),
              required<double>(inputs.data, 0x00d7a220),
              required<float>(inputs.data, 0x00d7a260),
              required<float>(inputs.data, 0x00d7a24c)},
          parameter_runtime{inputs.parameter_pool_00f8d344,
              required<double>(inputs.data, 0x00cf1450),
              required<float>(inputs.data, 0x00d5dca0),
              required<double>(inputs.data, 0x00d7a280),
              required<double>(inputs.data, 0x00d7a348)},
          type_parameters{parameter_runtime,
              required<double>(inputs.data, 0x00d7a358), inputs.crt,
              required<double>(inputs.data, 0x00d7a220),
              required<double>(inputs.data, 0x00d7a210),
              required<double>(inputs.data, 0x00d7a2b0),
              required<double>(inputs.data, 0x00d7a328)},
          texture_names{inputs.strings, inputs.actual_atlas_manager_00f8c26c,
              inputs.empty_atlas_stem_00f8c2c1, inputs.null_pattern_00e17bf0},
          type_texture{texture_names, &inputs.texture_cache,
              &inputs.render_actual_owners, inputs.half_import,
              required<std::uint32_t>(inputs.data, 0x00d5f0a8, 26),
              required<std::uint32_t>(inputs.data, 0x00d7a24c),
              inputs.empty_texture_name_00f8d37c,
              inputs.residues.first_texture_record_stack_word18,
              inputs.residues.later_texture_record_stack_word18},
          type_properties{inputs.strings, &type_texture,
              required<std::uint32_t>(inputs.data, 0x00d5dd18, 5),
              required<std::uint32_t>(inputs.data, 0x00d5dcc0, 5),
              required<std::uint32_t>(inputs.data, 0x00d5dcec, 5),
              required<std::uint32_t>(inputs.data, 0x00d5db00, 5),
              required<std::uint32_t>(inputs.data, 0x00d5e048, 5)},
          sprite_floating{type_properties, parameter_builder, type_parameters,
              inputs.actual_text_scratch_00f8c2c8},
          axial_axis{required<double>(inputs.data, 0x00d5daf8),
              required<double>(inputs.data, 0x00ce3830),
              required<float>(inputs.data, 0x00d7a24c),
              required<float>(inputs.data, 0x00d7a208)},
          axial{parameter_builder, type_parameters, type_properties,
              inputs.actual_text_scratch_00f8c2c8, axial_axis,
              static_cast<std::uint32_t>(inputs.residues.axial_builder_kind)},
          object_resources{inputs.resource_manager, inputs.resource_load_cache,
              inputs.resource_references, inputs.actual_model_factory_alias_00f8d31c,
              inputs.empty_model_stem_00f8d320},
          object{type_properties, parameter_builder, type_parameters, object_resources,
              inputs.actual_text_scratch_00f8c2c8,
              required<std::uint32_t>(inputs.data, 0x00d5db00, 9)},
          tracer{parameter_builder, type_parameters, type_properties, texture_names,
              inputs.actual_text_scratch_00f8c2c8,
              inputs.empty_tracer_frame_name_00f8d390},
          type_construction{inputs.strings, inputs.half_import,
              required<std::uint32_t>(inputs.data, 0x00d7a24c),
              required<std::uint32_t>(inputs.data, 0x00ce3804),
              required<std::uint32_t>(inputs.data, 0x00ce6650),
              inputs.residues.type_record_stack_word18},
          type_factory{type_construction, sprite_floating, axial, object, tracer,
              required<std::uint32_t>(inputs.data, 0x00d5de48, 3),
              required<std::uint32_t>(inputs.data, 0x00d5de88, 3),
              required<std::uint32_t>(inputs.data, 0x00d5debc, 3)},
          sphere{parameter_builder, parameter_runtime,
              required<double>(inputs.data, 0x00d7a358),
              inputs.actual_text_scratch_00f8c2c8,
              inputs.residues.sphere_child_builder_kind},
          cone{parameter_builder, parameter_runtime,
              required<double>(inputs.data, 0x00d7a358),
              inputs.actual_text_scratch_00f8c2c8,
              inputs.residues.cone_child_builder_kind},
          smartarea{parameter_builder, parameter_runtime,
              required<double>(inputs.data, 0x00d7a358),
              inputs.actual_text_scratch_00f8c2c8,
              inputs.residues.smartarea_child_builder_kind},
          emitter_factory{emitter_construction, &sphere, &cone, &smartarea,
              required<std::uint32_t>(inputs.data, 0x00d5debc, 6),
              required<std::uint32_t>(inputs.data, 0x00d5de88, 6),
              required<std::uint32_t>(inputs.data, 0x00d5de48, 6)},
          parser{parameter_builder, emitter_factory,
              inputs.actual_text_scratch_00f8c2c8,
              &inputs.actual_feature_word_0109eea4,
              inputs.residues.resource_emitter_child_builder_kind},
          loader{inputs.strings, inputs.vfs.actual_vfs_publication_0109ceec,
              inputs.vfs.bindings, inputs.vfs.name_resolution, parser,
              inputs.empty_particle_name_00f8766c},
          resource_cache{inputs.resource_manager.actual_lifetime_manager_01090aa0,
              inputs.actual_particle_cache_publication_00f87668, &type_lifetime},
          acquisition{inputs.strings, loader, inputs.vfs.dates,
              inputs.vfs.actual_vfs_publication_0109ceec,
              inputs.actual_platform_publication_0109cf04,
              inputs.bound_platform_identity, inputs.load_events,
              required_plain<std::uint32_t>(inputs.data, 0x00d0db40, 4),
              required_plain<std::uint32_t>(inputs.data, 0x00d0daf0, 4),
              inputs.resource_manager.invalid_parameters} {
        sphere.emitters = &emitter_factory;
        sphere.particles = &type_factory;
        cone.emitters = &emitter_factory;
        cone.particles = &type_factory;
        smartarea.emitters = &emitter_factory;
        smartarea.particles = &type_factory;
    }
};

GameNativeParticleRuntime::GameNativeParticleRuntime(
    const GameNativeParticleRuntimeInputs& inputs)
    : impl_(std::make_unique<Impl>(inputs)) {}
GameNativeParticleRuntime::~GameNativeParticleRuntime() = default;
NativeParticleResourceLoaderRawContext& GameNativeParticleRuntime::loader() noexcept {
    return impl_->loader;
}
NativeParticleResourceParserRawContext& GameNativeParticleRuntime::parser() noexcept {
    return impl_->parser;
}
NativeParticleEmitterFactoryRawContext& GameNativeParticleRuntime::emitter_factory() noexcept {
    return impl_->emitter_factory;
}
NativeParticleTypeFactoryRawContext& GameNativeParticleRuntime::type_factory() noexcept {
    return impl_->type_factory;
}
NativeParticleTypeLifetimeContext& GameNativeParticleRuntime::loaded_resource_lifetime() noexcept {
    return impl_->type_lifetime;
}
NativeParticleResourceCacheContext& GameNativeParticleRuntime::resource_cache() noexcept {
    return impl_->resource_cache;
}
NativeParticleResourceAcquisitionRawContext& GameNativeParticleRuntime::acquisition() noexcept {
    return impl_->acquisition;
}

} // namespace bsp::game
