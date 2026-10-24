#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_material_compiler_owners.hpp"
#include "bsp/game_native_material_compiler_sources.hpp"
#include "bsp/game_native_material_effect_owners.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_shader_process.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/native_material_program_compiler_actual.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include "bsp/native_shader_descriptor_reader.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_platform_load_messages.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
} // namespace
struct GameNativeGuiEffectApplication::Impl final : GuiTextEffectConsumer {
    enum class Phase { ready, loading, draining, closed, failed };
    GameNativeGuiEffectApplication& facade;
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    const volatile std::uint32_t* const material_profile;
    GameVfsHost& vfs;
    GameNativeGuiEffectApplicationBindings bindings;
    GameNativeVfsRawServices raw;
    NativeTextureCacheContext& textures;
    NativeEffectRegistryDestructionContext& registry;
    GameNativeMaterialCompilerOwners compiler_owners;
    GameNativeMaterialCompilerSources sources;
    GameNativeMaterialEffectOwners effect_owners;
    NativeShaderBinaryCacheContext& shader_cache;
    NativeLuaServiceBindings& lua_binding;
    NativeMaterialProgramCompilerActualContext actual_compiler_context;
    NativeMaterialProgramCompilerActual actual_compiler;
    NativeMaterialProgramCompileContext compile_context;
    NativeMaterialEffectCompilerBinding compiler;
    NativeMaterialEffectNativeChildren children;
    NativeMaterialEffectProgramsContext programs;
    NativeMaterialEffectLoadingContext effects;
    NativeMaterialEffectCacheContext cache;
    std::vector<std::unique_ptr<GameNativeGuiEffectLoadOperation>> operations;
    std::vector<std::unique_ptr<GameNativeGuiMaterialFactoryOperation>> material_operations;
    GuiTextBufferServices* consumer_services{};
    NativeMaterialDestructionAccess* consumer_materials{};
    const volatile std::uint32_t* consumer_material_profile{};
    std::vector<std::unique_ptr<GuiTextEffectConsumerSectionCall>> section_calls;
    std::vector<std::unique_ptr<GameNativeGuiTextMaterialCall>> text_material_calls;
    std::vector<std::unique_ptr<GameNativeGuiTextShaderCall>> text_shader_calls;
    GuiTextEffectConsumerSectionCall* active_sections{};
    GameNativeGuiTextShaderCall* active_shader{};
    Phase phase{Phase::ready};
    std::exception_ptr failed;

    Impl(GameNativeGuiEffectApplication& application, GameHostLog& l,
        GameNativeRendererApplication& r, GameVfsHost& files,
        GameNativeGuiEffectApplicationBindings inputs)
        : facade(application), log(l), renderer(r),
          material_profile(r.borrow_gui_material_destruction_profile_00d5e520()),
          vfs(files), bindings(inputs), raw(vfs.borrow_raw_services()),
          textures(renderer.texture_cache()), registry(renderer.effect_registry_domain()),
          compiler_owners(renderer.material_compiler_owners()),
          sources(renderer.material_compiler_sources()), effect_owners(renderer.material_effect_owners()),
          shader_cache(renderer.shader_cache_context()), lua_binding(renderer.lua_service_binding()),
          actual_compiler_context{raw.strings, shader_cache.load_variants_0108d6f0,
              shader_cache.source_mode_0108d6f1, shader_cache.actual_cache_0108d6ec,
              renderer.publication_00f8d394(), compiler_owners.passes, textures,
              sources.source, sources.vertex, sources.pixel, compiler_owners.reflection,
              compiler_owners.shaders, compiler_owners.registration,
              bindings.root_samplers, bindings.mode_samplers,
              compiler_owners.numeric_renderer.renderer_profile_00d5f0a8,
              shader_cache.actual_physical_profile_00d691b0,
              game_native_shader_process().source_empty_0108d6f2(),
              game_native_shader_process().stream_empty_0109db64(), bindings.sampler_calls},
          actual_compiler(actual_compiler_context),
          compile_context{raw.strings, shader_cache.load_variants_0108d6f0,
              shader_cache.source_mode_0108d6f1, actual_compiler},
          compiler(effect_owners.lifetime, compile_context),
          children(effect_owners.lifetime, renderer.shader_descriptor_reader(), raw.name_resolution,
              compiler_owners.state_cache, compiler),
          programs{raw.strings, effect_owners.lifetime, compiler_owners.passes,
              compiler_owners.copies, compiler_owners.secondary_registration, children,
              renderer.publication_00f8d394(), bindings.material_manager_00f8bbf0,
              raw.actual_vfs_publication_0109ceec, shader_cache.load_variants_0108d6f0,
              &compiler_owners.numeric_renderer},
          effects{effect_owners.construction, programs, effect_owners.owners,
              raw.actual_vfs_publication_0109ceec,
              compiler_owners.numeric_renderer.renderer_profile_00d5f0a8,
              nullptr, nullptr, textures.textures.empty_string_0108d5a4,
              nullptr, &textures, nullptr},
          cache{registry.actual_records.strings, registry.actual_records.validation,
              effects, raw.dates, textures.platform_0109cf04, textures.synchronization_0108d6dc,
              registry.actual_derived_profile_00d5f074,
              registry.actual_effect_base_profile_00d5e534,
              registry.actual_effect_derived_profile_00d61a00,
              registry.actual_records.allocate_array_00bf55be,
              registry.actual_records.free_array_00bf6989} {
        effects.cache = &cache;
        bind_native_material_effect_vfs_name_resolution(effects, raw.name_resolution);
        validate_domain();
    }
    void validate_domain() {
        const auto& reader = renderer.shader_descriptor_reader();
        require(&renderer.lua_service_binding() == &lua_binding &&
            &reader.files == &lua_binding.files() && &reader.strings == &raw.strings &&
            reader.bootstrap.do_file_00b69e00 == lua_binding.files().do_file_00b69e00,
            "GUI effect Lua execution requires the renderer reader's same retained actual service binding");
        (void)renderer.section_layout_services(); // Existing genuine ready gate.
        auto current = vfs.borrow_raw_services();
        require(&current.bindings == &raw.bindings && &current.strings == &raw.strings &&
            &current.name_resolution == &raw.name_resolution && &current.dates == &raw.dates &&
            &current.actual_vfs_publication_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &registry == &renderer.effect_registry_domain() &&
            &registry.actual_owners == &renderer.actual_owners() &&
            &registry.actual_records.strings == &raw.strings && &textures.strings == &raw.strings &&
            &textures.textures.streams == &raw.bindings && &textures.dates == &raw.dates &&
            &effect_owners.construction.strings == &raw.strings &&
            &effect_owners.lifetime.strings == &raw.strings &&
            &effect_owners.lifetime.retained_owners == &renderer.actual_owners() &&
            &effect_owners.owners.actual_owners() == &renderer.actual_owners() &&
            &compiler_owners.passes.lifetime.strings == &raw.strings &&
            &compiler_owners.passes.lifetime.retained_owners == &renderer.actual_owners() &&
            &compiler_owners.numeric_renderer.construction == &compiler_owners.passes &&
            &compiler_owners.numeric_renderer.textures == &textures &&
            &bindings.root_samplers != &bindings.mode_samplers &&
            &bindings.root_samplers.pass_lifetime == &compiler_owners.passes.lifetime &&
            &bindings.mode_samplers.pass_lifetime == &compiler_owners.passes.lifetime &&
            &bindings.root_samplers.loader.cache.strings == &raw.strings &&
            &bindings.mode_samplers.loader.cache.strings == &raw.strings &&
            &bindings.root_samplers.loader.cache.dates == &raw.dates &&
            &bindings.mode_samplers.loader.cache.dates == &raw.dates &&
            &bindings.root_samplers.actual_manager_01090aa0 ==
                &bindings.root_samplers.loader.cache.actual_manager_01090aa0 &&
            &bindings.mode_samplers.actual_manager_01090aa0 ==
                &bindings.mode_samplers.loader.cache.actual_manager_01090aa0 &&
            &bindings.root_samplers.actual_manager_01090aa0 == &bindings.mode_samplers.actual_manager_01090aa0 &&
            &bindings.root_samplers.actual_loader_00f8d420 == &bindings.mode_samplers.actual_loader_00f8d420 &&
            &shader_cache.vfs == &raw.bindings &&
            &shader_cache.actual_vfs_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &shader_cache.actual_cache_0108d6ec == &game_native_shader_process().cache_0108d6ec() &&
            &shader_cache.load_variants_0108d6f0 == &game_native_shader_process().modes().load_variants_0108d6f0 &&
            &shader_cache.source_mode_0108d6f1 == &game_native_shader_process().modes().source_mode_0108d6f1 &&
            &effect_owners.construction.current_renderer_00f8d394 == &renderer.publication_00f8d394() &&
            &compiler_owners.passes.current_renderer_00f8d394 == &renderer.publication_00f8d394(),
            "GUI effect production requires the same ready actual renderer/VFS/strings/records/compiler/sampler domains");
        require(&bindings.material_manager_00f8bbf0 ==
            &game_native_renderer_scalar_process().shadow_target_publication_00f8bbf0(),
            "GUI effect policy must borrow the canonical actual F8BBF0 shadow-target cell");
        require(cache.allocate_array_00bf55be && cache.free_array_00bf6989 &&
            cache.registry_vtable_00d5f074 && cache.registry_vtable_00d5f074[2] == 0x00b2ebb0 &&
            effects.renderer_profile_00d5f0a8 && effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0,
            "GUI effect production lacks its genuine record allocation or numeric effect dispatch");
    }
    const volatile std::uint32_t* validate_material_domain(GuiNativeGeometryOwners& geometry,
        NativeMaterialDestructionAccess& materials) {
        auto& actual = renderer.actual_owners();
        auto& canonical_geometry = renderer.game_grid_context().graphics.streams.geometry;
        const auto& registration = geometry.registration();
        auto& process = game_native_material_process();
        require(&geometry == &canonical_geometry && &geometry.actual_owners() == &actual &&
            &registration.owners == &actual && registration.context == &actual &&
            registration.bind == &NativeRenderActualOwnerRegistry::bind_callback &&
            registration.unbind == &NativeRenderActualOwnerRegistry::unbind_callback &&
            registration.find == &NativeRenderActualOwnerRegistry::find_callback &&
            &materials.retained_owners == &actual && &materials.parameter_names == &raw.strings &&
            &materials.material_slots == &process.material_pool_00f8d3ac() &&
            &materials.parameter_slots == &process.parameter_pool_00f8d3e4(),
            "GUI material invocation requires the same canonical geometry, owners, strings and process pools");
        // This canonical immutable view was admitted before any acquisition.
        // The same domain checks above still apply during every invocation;
        // a retained native preparation is not another ready-time admission.
        return material_profile;
    }
    const volatile std::uint32_t* validate_consumer_domain(GuiTextBufferServices& services) {
        validate_domain();
        const auto* profile = validate_material_domain(services.geometry, services.materials);
        require(&services.strings == &raw.strings &&
            &services.current_renderer_00f8d394 == &renderer.publication_00f8d394() &&
            services.material_vtable_00d5e520 == profile &&
            &services.widgets.environment().models.retained_owners == &renderer.actual_owners() &&
            &services.widgets.environment().models.nodes == &services.parenting.nodes,
            "Text effect consumer requires the exact renderer/profile/string/widget owner domains");
        if (consumer_services)
            require(consumer_services == &services && services.native_effect_consumer == this &&
                consumer_materials == &services.materials && consumer_material_profile == profile,
                "Text effect consumer cannot replace its services, destruction access or mapped profile");
        return profile;
    }
    bool caller_active() const noexcept { return active_sections || active_shader; }
    bool has_incomplete_caller(const GuiTextLifetime& lifetime) const noexcept override {
        return (active_sections && active_sections->lifetime == &lifetime) ||
            (active_shader && active_shader->lifetime == &lifetime);
    }
    template<class T, class U> static bool owns(const std::vector<std::unique_ptr<T>>& frames, const U* pointer) {
        for (const auto& frame : frames) if (frame.get() == pointer) return true;
        return false;
    }
    GameNativeGuiTextMaterialCall& material_call(GuiTextEffectConsumerMaterialCall& call) {
        require(owns(text_material_calls, &call), "Text material call is not this provider's retained invocation");
        return static_cast<GameNativeGuiTextMaterialCall&>(call);
    }
    GameNativeGuiTextShaderCall& shader_call(GuiTextEffectConsumerShaderCall& call) {
        require(owns(text_shader_calls, &call), "Text shader call is not this provider's retained invocation");
        return static_cast<GameNativeGuiTextShaderCall&>(call);
    }
    void begin_sections(GuiTextLifetime&, GuiTextSectionOperation&, GuiTextBufferServices&,
        GuiTextEffectConsumerSectionCall*&) override;
    void begin_section_material(GuiTextEffectConsumerSectionCall&,
        GuiTextEffectConsumerMaterialCall*&) override;
    NativeMaterialStorage* create_section_material(GuiTextEffectConsumerMaterialCall&) override;
    void finish_sections(GuiTextEffectConsumerSectionCall&) override;
    void fail_sections(GuiTextEffectConsumerSectionCall&, std::exception_ptr) noexcept override;
    void begin_shader(GuiTextLifetime&, GuiTextShaderServices&,
        GuiTextEffectConsumerShaderCall*&) override;
    void* acquire_font_effect(GuiTextEffectConsumerShaderCall&) override;
    void finish_shader(GuiTextEffectConsumerShaderCall&) override;
    void fail_shader(GuiTextEffectConsumerShaderCall&, std::exception_ptr) noexcept override;
};

void GameNativeGuiEffectApplication::Impl::begin_sections(GuiTextLifetime& lifetime,
    GuiTextSectionOperation& caller, GuiTextBufferServices& services,
    GuiTextEffectConsumerSectionCall*& published) {
    require(phase == Phase::ready && !caller_active() && !published && consumer_services == &services &&
        &lifetime.buffer_services() == &services && caller.phase == GuiTextSectionPhase::running &&
        caller.native_site == 0x00ab8553 && !caller.effect_consumer_calls.empty() &&
        &published == &caller.effect_consumer_calls.back(),
        "Text sections require their exact fresh retained caller slot and idle provider");
    validate_consumer_domain(services);
    auto frame = std::make_unique<GuiTextEffectConsumerSectionCall>();
    auto& call = *frame;
    call.consumer = this;
    call.lifetime = &lifetime;
    call.caller = &caller;
    call.services = &services;
    section_calls.push_back(std::move(frame));
    call.phase = GuiTextEffectConsumerPhase::running;
    active_sections = &call;
    published = &call; // Stable whole caller before Model/string/factory effects.
}
void GameNativeGuiEffectApplication::Impl::begin_section_material(
    GuiTextEffectConsumerSectionCall& parent, GuiTextEffectConsumerMaterialCall*& published) {
    require(phase == Phase::ready && owns(section_calls, &parent) && active_sections == &parent &&
        !active_shader && parent.phase == GuiTextEffectConsumerPhase::running && !published &&
        !parent.materials.empty() && &published == &parent.materials.back() &&
        parent.caller->phase == GuiTextSectionPhase::running &&
        (parent.caller->native_site == 0x00ab8643 || parent.caller->native_site == 0x00ab8716) &&
        !parent.caller->name_live && !parent.caller->name_cleanup_armed,
        "Text material name requires its exact retained AB8530 invocation and fresh slot");
    validate_consumer_domain(*parent.services);
    for (std::size_t i = 0; i + 1 < parent.materials.size(); ++i) {
        require(parent.materials[i] != nullptr, "Text material caller has an unpublished prior slot");
        auto& prior = material_call(*parent.materials[i]);
        require(prior.parent == &parent && prior.name_phase == GuiTextEffectNamePhase::consumed &&
            prior.acquisition && prior.acquisition->phase == GameNativeGuiMaterialFactoryPhase::complete,
            "Text material caller cannot overlap or replay an earlier name/factory");
    }
    auto frame = std::make_unique<GameNativeGuiTextMaterialCall>();
    auto& call = *frame;
    call.parent = &parent;
    text_material_calls.push_back(std::move(frame));
    call.phase = GuiTextEffectConsumerPhase::running;
    published = &call; // Header storage exists before the native name constructor.
}
NativeMaterialStorage* GameNativeGuiEffectApplication::Impl::create_section_material(
    GuiTextEffectConsumerMaterialCall& supplied) {
    auto& call = material_call(supplied);
    require(phase == Phase::ready && active_sections == call.parent && !active_shader &&
        call.parent->phase == GuiTextEffectConsumerPhase::running &&
        call.phase == GuiTextEffectConsumerPhase::running && !call.acquisition &&
        call.name_phase == GuiTextEffectNamePhase::live &&
        call.parent->caller->active_effect_material == &call &&
        call.parent->caller->active_effect_name == &call.name &&
        call.parent->caller->factory_in_flight == GuiTextSectionFactory::material &&
        (call.parent->caller->native_site == 0x00ab8654 || call.parent->caller->native_site == 0x00ab8727),
        "Text material entry requires its exact active factory token and live caller name");
    validate_consumer_domain(*call.parent->services);
    call.result = facade.create_material_for_caller(&call.name, consumer_services->geometry,
        *consumer_materials, call.acquisition, &call);
    return call.result;
}
void GameNativeGuiEffectApplication::Impl::finish_sections(GuiTextEffectConsumerSectionCall& call) {
    require(phase == Phase::ready && owns(section_calls, &call) && active_sections == &call &&
        !active_shader && call.phase == GuiTextEffectConsumerPhase::running &&
        call.caller->native_site == 0x00ab87cb && !call.caller->name_live &&
        !call.caller->name_cleanup_armed && !call.caller->mesh && !call.caller->section &&
        !call.caller->material && call.caller->factory_in_flight == GuiTextSectionFactory::none &&
        call.caller->release_phase != GuiTextSectionReleasePhase::entered,
        "Text sections cannot complete an interrupted native caller");
    validate_consumer_domain(*call.services);
    for (auto* child : call.materials) {
        require(child != nullptr, "Text sections retained an unpublished material call");
        auto& material = material_call(*child);
        require(material.parent == &call && material.name_phase == GuiTextEffectNamePhase::consumed &&
            material.acquisition && material.acquisition->phase == GameNativeGuiMaterialFactoryPhase::complete &&
            !material.acquisition->input_name_borrow_active,
            "Text sections retained an incomplete material acquisition or name");
    }
    for (auto* child : call.materials) child->phase = GuiTextEffectConsumerPhase::complete;
    call.phase = GuiTextEffectConsumerPhase::complete;
    active_sections = nullptr;
}
void GameNativeGuiEffectApplication::Impl::fail_sections(GuiTextEffectConsumerSectionCall& call,
    std::exception_ptr failure) noexcept {
    if (active_sections != &call || call.consumer != this) std::terminate();
    call.failure = failure;
    call.phase = GuiTextEffectConsumerPhase::failed;
    for (auto* child : call.materials)
        if (child) child->phase = GuiTextEffectConsumerPhase::failed;
    failed = failure;
    phase = Phase::failed; // Keep the active caller/children; no cleanup/replay.
}
void GameNativeGuiEffectApplication::Impl::begin_shader(GuiTextLifetime& lifetime,
    GuiTextShaderServices& services, GuiTextEffectConsumerShaderCall*& published) {
    auto& history = lifetime.shader_consumer_calls();
    require(phase == Phase::ready && !caller_active() && !published &&
        consumer_services == &services.buffers && &lifetime.buffer_services() == consumer_services &&
        !lifetime.cached_shader_slot_1ec() && !history.empty() && &published == &history.back() &&
        NativePlatformLoadPublication(services.configuration_0109cf04).identity() ==
            renderer.borrow_platform_publication_0109cf04().identity(),
        "Text shader requires its exact fresh caller slot, services/configuration and idle provider");
    validate_consumer_domain(services.buffers);
    auto frame = std::make_unique<GameNativeGuiTextShaderCall>();
    auto& call = *frame;
    call.consumer = this;
    call.lifetime = &lifetime;
    call.services = &services;
    text_shader_calls.push_back(std::move(frame));
    call.phase = GuiTextEffectConsumerPhase::running;
    active_shader = &call;
    published = &call; // Both possible headers exist before selection/name effects.
}
void* GameNativeGuiEffectApplication::Impl::acquire_font_effect(GuiTextEffectConsumerShaderCall& supplied) {
    auto& call = shader_call(supplied);
    require(phase == Phase::ready && active_shader == &call && !active_sections &&
        call.phase == GuiTextEffectConsumerPhase::running && !call.acquisition &&
        call.name_phase == GuiTextEffectNamePhase::live &&
        call.selected_name == (call.default_selected ? static_cast<const void*>(&call.default_name)
            : static_cast<const void*>(&call.override_name)),
        "Text shader entry requires its exact active acquisition token and retained live header");
    validate_consumer_domain(call.services->buffers);
    // The texture loader's event-provider cell is not this actual publication.
    // Borrow identity only; the caller still supplies a valid actual +28 input.
    require(NativePlatformLoadPublication(call.services->configuration_0109cf04).identity() ==
        renderer.borrow_platform_publication_0109cf04().identity(),
        "Text shader cannot retarget its live configuration cell");
    call.result = facade.acquire_effect_for_caller(call.selected_name, call.acquisition, &call);
    return call.result;
}
void GameNativeGuiEffectApplication::Impl::finish_shader(GuiTextEffectConsumerShaderCall& supplied) {
    auto& call = shader_call(supplied);
    require(phase == Phase::ready && active_shader == &call && !active_sections &&
        call.phase == GuiTextEffectConsumerPhase::running && call.shader_published &&
        !call.name_cleanup_armed && call.acquisition &&
        call.acquisition->phase == GameNativeGuiEffectLoadPhase::complete &&
        !call.acquisition->input_name_borrow_active &&
        call.lifetime->cached_shader_slot_1ec() == call.result &&
        call.name_phase == (call.default_selected ? GuiTextEffectNamePhase::consumed :
            GuiTextEffectNamePhase::borrow_ended),
        "Text shader cannot complete before publication and its exact native name cleanup");
    validate_consumer_domain(call.services->buffers);
    call.phase = GuiTextEffectConsumerPhase::complete;
    active_shader = nullptr;
}
void GameNativeGuiEffectApplication::Impl::fail_shader(GuiTextEffectConsumerShaderCall& call,
    std::exception_ptr failure) noexcept {
    if (active_shader != &call || call.consumer != this) std::terminate();
    call.failure = failure;
    call.phase = GuiTextEffectConsumerPhase::failed;
    failed = failure;
    phase = Phase::failed; // Keep the caller/header/acquisition without native rollback.
}

GameNativeGuiEffectApplication::GameNativeGuiEffectApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs,
    GameNativeGuiEffectApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(*this, log, renderer, vfs, bindings)) {}
GameNativeGuiEffectApplication::~GameNativeGuiEffectApplication() noexcept {
    if (impl_->phase != Impl::Phase::closed &&
        (impl_->phase != Impl::Phase::ready || !impl_->operations.empty() ||
            !impl_->material_operations.empty() || !impl_->section_calls.empty() ||
            !impl_->text_shader_calls.empty() || impl_->caller_active())) std::terminate();
    if (impl_->consumer_services) {
        if (impl_->consumer_services->native_effect_consumer != impl_.get()) std::terminate();
        impl_->consumer_services->native_effect_consumer = nullptr;
    }
}
void* GameNativeGuiEffectApplication::acquire_effect_00b318b0(const void* name) {
    const GameNativeGuiEffectLoadOperation* operation{};
    return acquire_effect_00b318b0(name, operation);
}
void* GameNativeGuiEffectApplication::acquire_effect_00b318b0(const void* name,
    const GameNativeGuiEffectLoadOperation*& published_operation) {
    return acquire_effect_for_caller(name, published_operation, nullptr);
}
void* GameNativeGuiEffectApplication::acquire_effect_for_caller(const void* name,
    const GameNativeGuiEffectLoadOperation*& published_operation,
    const GuiTextEffectConsumerShaderCall* authorization) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready, "GUI effect producer cannot replay/reenter/retarget or reopen after drain");
    require(authorization ? (p.active_shader == authorization && !p.active_sections &&
        authorization->phase == GuiTextEffectConsumerPhase::running &&
        authorization->selected_name == name) : !p.caller_active(),
        "GUI effect entry is not the exact active Text caller's authorized acquisition");
    require(!published_operation && name && (reinterpret_cast<std::uintptr_t>(name) & 3u) == 0,
        "GUI effect producer requires a fresh output and stable initialized actual8h name header");
    p.validate_domain(); // Pure admission checks precede native entry.
    auto frame = std::make_unique<GameNativeGuiEffectLoadOperation>();
    auto& op = *frame;
    op.input_name = name;
    p.operations.push_back(std::move(frame)); // Publish BEFORE capturing/native effects.
    published_operation = &op;
    p.phase = Impl::Phase::loading;
    op.phase = GameNativeGuiEffectLoadPhase::running;
    op.input_name_borrow_active = true;
    try {
        op.captured_lua_binding = &p.lua_binding;
        // Same scope used by the canonical renderer descriptor entry. It
        // covers bootstrap, nested DoFile/overrides, and normal descriptor
        // Lua close. Existing Activation restores the previous thread binding
        // on ordinary return/C++ unwind; interrupted Lua remains unresumable.
        NativeLuaServiceBindings::Activation activation(p.lua_binding);
        op.captured_renderer = p.renderer.publication_00f8d394();
        std::uint32_t profile{};
        require(op.captured_renderer != nullptr, "GUI effect producer has no live actual renderer");
        std::memcpy(&profile, op.captured_renderer, sizeof(profile));
        require(profile == 0x00d5f0a8, "GUI effect producer requires its actual D5F0A8 receiver");
        op.result = load_native_renderer_material_effect_00b318b0(
            op.captured_renderer, op.input_name, p.cache, &op.cache);
        require(op.cache.phase == NativeMaterialEffectCacheAcquired::Phase::complete,
            "GUI effect producer returned before its retained cache completed");
        if (op.result) {
            auto* actual = dynamic_cast<NativeMaterialEffectReference*>(
                &p.renderer.actual_owners().resolve_actual(op.result));
            require(actual && &actual->storage() == op.result,
                "GUI effect producer returned a foreign or semantic effect");
            auto& storage = actual->storage();
            require(&actual->reference_count == &storage.references_04,
                "GUI effect result companion does not alias its actual+04");
            op.result_profile = storage.vtable_00;
            op.result_references = storage.references_04.load();
            require(op.result_references > 0 &&
                (op.result_profile == 0x00d5e534 || op.result_profile == 0x00d61a00),
                "GUI effect result has no live supported actual profile");
        }
        op.phase = GameNativeGuiEffectLoadPhase::complete;
        p.phase = Impl::Phase::ready;
        // Log identity only. The return below transfers the held reference; a
        // logging exception still leaves that actual acquisition in this frame.
        p.log.notef("actual GUI effect acquire renderer=%p name=%p result=%p profile=%08x refs=%d cache_site=%08x cache_published=%d caller_acquired=%d",
            op.captured_renderer, op.input_name, op.result, op.result_profile, op.result_references, op.cache.native_site,
            op.cache.cache_published ? 1 : 0, op.cache.caller_acquired ? 1 : 0);
        op.input_name_borrow_active = false;
        op.caller_reference_transferred = op.result != nullptr;
        return op.result;
    } catch (...) {
        op.input_name_borrow_active = false;
        op.failure = std::current_exception();
        op.phase = GameNativeGuiEffectLoadPhase::failed;
        p.failed = op.failure;
        p.phase = Impl::Phase::failed;
        throw;
    }
}
std::size_t GameNativeGuiEffectApplication::operation_count() const noexcept {
    return impl_->operations.size();
}
const GameNativeGuiEffectLoadOperation& GameNativeGuiEffectApplication::operation(std::size_t index) const {
    return *impl_->operations.at(index);
}
NativeMaterialEffectCacheContext& GameNativeGuiEffectApplication::native_effect_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready && !p.caller_active(),
        "native effect cache borrow requires an idle ready effect application");
    p.validate_domain();
    return p.cache;
}
NativeMaterialStorage* GameNativeGuiEffectApplication::create_material_00535320(const void* name,
    GuiNativeGeometryOwners& geometry, NativeMaterialDestructionAccess& materials,
    const GameNativeGuiMaterialFactoryOperation*& published_operation) {
    return create_material_for_caller(name, geometry, materials, published_operation, nullptr);
}
NativeMaterialStorage* GameNativeGuiEffectApplication::create_material_for_caller(const void* name,
    GuiNativeGeometryOwners& geometry, NativeMaterialDestructionAccess& materials,
    const GameNativeGuiMaterialFactoryOperation*& published_operation,
    const GuiTextEffectConsumerMaterialCall* authorization) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready,
        "GUI material producer cannot replay/reenter/retarget or reopen after drain");
    require(authorization ? (p.active_sections == authorization->parent && !p.active_shader &&
        authorization->phase == GuiTextEffectConsumerPhase::running &&
        name == &authorization->name && p.consumer_services &&
        &geometry == &p.consumer_services->geometry && &materials == p.consumer_materials)
        : !p.caller_active(),
        "GUI material entry is not the exact active Text caller's authorized factory");
    require(!published_operation && name && (reinterpret_cast<std::uintptr_t>(name) & 3u) == 0,
        "GUI material producer requires a fresh output and stable initialized actual8h name header");
    p.validate_domain();
    const auto* profile = p.validate_material_domain(geometry, materials);
    auto frame = std::make_unique<GameNativeGuiMaterialFactoryOperation>();
    auto& op = *frame;
    op.input_name = name;
    op.geometry = &geometry;
    op.materials = &materials;
    op.material_profile_00d5e520 = profile;
    p.material_operations.push_back(std::move(frame)); // Stable BEFORE any native effects.
    published_operation = &op;
    p.phase = Impl::Phase::loading;
    op.phase = GameNativeGuiMaterialFactoryPhase::factory;
    op.input_name_borrow_active = true;
    try {
        op.captured_lua_binding = &p.lua_binding;
        NativeLuaServiceBindings::Activation activation(p.lua_binding);
        op.captured_renderer = p.renderer.publication_00f8d394();
        require(op.captured_renderer != nullptr, "GUI material producer has no live actual renderer");
        std::uint32_t renderer_profile{};
        std::memcpy(&renderer_profile, op.captured_renderer, sizeof(renderer_profile));
        require(renderer_profile == 0x00d5f0a8,
            "GUI material producer requires its actual D5F0A8 receiver");
        // The existing whole factory owns the native renderer capture, cache
        // acquisition, allocation/constructor, raw-slot-only constructor
        // unwind and normal temporary-effect release. Do not split/replay it.
        op.result = create_native_material_from_effect_cache_00535320(name,
            materials.material_slots, materials.retained_owners, p.cache, op.factory);
        require(op.factory.phase == NativeMaterialFactoryAcquired::Phase::complete &&
            op.factory.cache && op.factory.cache->phase == NativeMaterialEffectCacheAcquired::Phase::complete,
            "GUI material producer returned before its retained factory/cache completed");
        if (op.result) {
            op.phase = GameNativeGuiMaterialFactoryPhase::registration;
            // Admission does not retain or release the creator. Failed bind
            // leaves its raw material/companion/record on this SAME frame.
            geometry.register_native_material_creator(op.factory, materials, profile);
            auto* actual = dynamic_cast<NativeMaterialReference*>(
                &p.renderer.actual_owners().resolve_actual(op.result));
            require(actual && actual == op.factory.companion && op.factory.registered &&
                &actual->storage() == op.result &&
                &actual->reference_count == &op.result->references_04,
                "GUI material result lacks its same canonical actual-count companion");
            op.result_profile = op.result->vtable_00;
            op.result_references = op.result->references_04.load();
            require(op.result_profile == 0x00d5e520 && op.result_references > 0,
                "GUI material result has no live supported actual profile");
        }
        op.phase = GameNativeGuiMaterialFactoryPhase::complete;
        p.phase = Impl::Phase::ready;
        p.log.notef("actual GUI material acquire renderer=%p name=%p result=%p profile=%08x refs=%d factory_site=%08x registered=%d",
            op.captured_renderer, op.input_name, static_cast<void*>(op.result), op.result_profile,
            op.result_references, op.factory.native_site, op.factory.registered ? 1 : 0);
        op.input_name_borrow_active = false;
        op.caller_reference_transferred = op.result != nullptr;
        return op.result;
    } catch (...) {
        // Caller cleanup may consume its name after unwinding this entry.
        // Retain identities/acquisitions, never dereference/replay that input.
        op.input_name_borrow_active = false;
        op.failure = std::current_exception();
        op.phase = GameNativeGuiMaterialFactoryPhase::failed;
        p.failed = op.failure;
        p.phase = Impl::Phase::failed;
        throw;
    }
}
std::size_t GameNativeGuiEffectApplication::material_operation_count() const noexcept {
    return impl_->material_operations.size();
}
const GameNativeGuiMaterialFactoryOperation&
GameNativeGuiEffectApplication::material_operation(std::size_t index) const {
    return *impl_->material_operations.at(index);
}
void GameNativeGuiEffectApplication::bind_text_effect_consumer(GuiTextBufferServices& services) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready && !p.caller_active() && !p.consumer_services &&
        !services.native_effect_consumer,
        "Text effect consumer requires an idle graph and a fresh exact services binding");
    const auto* profile = p.validate_consumer_domain(services);
    p.consumer_services = &services;
    p.consumer_materials = &services.materials;
    p.consumer_material_profile = profile;
    services.native_effect_consumer = &p;
}
bool GameNativeGuiEffectApplication::requires_process_retention() const noexcept {
    const auto phase = impl_->phase;
    return impl_->caller_active() || phase == Impl::Phase::loading ||
        phase == Impl::Phase::draining || phase == Impl::Phase::failed;
}
const std::exception_ptr& GameNativeGuiEffectApplication::failure() const noexcept {
    return impl_->failed;
}
void GameNativeGuiEffectApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready && !p.caller_active(),
        "failed/active/closed GUI effect graph cannot retire its metadata");
    if (p.operations.empty() && p.material_operations.empty() && p.section_calls.empty() &&
        p.text_shader_calls.empty()) { p.phase = Impl::Phase::closed; return; }
    p.phase = Impl::Phase::draining;
    try {
        require(!p.renderer.publication_00f8d394(), "GUI effect graph must survive the actual renderer cache/effect drain");
        p.renderer.after_native_drain(); // Existing owner/worker/native closure verification.
        for (const auto& op : p.operations)
            require(op->phase == GameNativeGuiEffectLoadPhase::complete &&
                !op->input_name_borrow_active && (!op->result || op->caller_reference_transferred),
                "GUI effect graph has an interrupted retained acquisition");
        for (const auto& op : p.material_operations)
            require(op->phase == GameNativeGuiMaterialFactoryPhase::complete &&
                !op->input_name_borrow_active &&
                op->factory.phase == NativeMaterialFactoryAcquired::Phase::complete &&
                (!op->result || (op->factory.registered && op->caller_reference_transferred)),
                "GUI effect graph has an interrupted retained material acquisition");
        for (const auto& call : p.section_calls)
            require(call->phase == GuiTextEffectConsumerPhase::complete,
                "GUI effect graph has an interrupted retained Text section caller");
        for (const auto& call : p.text_material_calls)
            require(call->phase == GuiTextEffectConsumerPhase::complete &&
                call->name_phase == GuiTextEffectNamePhase::consumed,
                "GUI effect graph has an interrupted retained Text material caller");
        for (const auto& call : p.text_shader_calls)
            require(call->phase == GuiTextEffectConsumerPhase::complete && !call->name_cleanup_armed &&
                call->name_phase == (call->default_selected ? GuiTextEffectNamePhase::consumed :
                    GuiTextEffectNamePhase::borrow_ended),
                "GUI effect graph has an interrupted retained Text shader caller");
        p.phase = Impl::Phase::closed;
    } catch (...) {
        p.failed = std::current_exception();
        p.phase = Impl::Phase::failed;
        throw;
    }
}
} // namespace bsp::game
