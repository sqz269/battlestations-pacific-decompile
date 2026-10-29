// OFF-default host composition, not GGame::Render parity or Original ABI.
// Native owners and operation frames outlive commands and the shared drain.
struct MissionShipGraph final {
    enum class Stage { prepared, preparing, ready, retiring, retired, failed, closed };
    GameHostLog& log;
    DestructionGraph& graph;
    DeviceGraph& devices;
    CameraGraph& cameras;
    FrameGraph& frames;
    NativeRendererConstructorContext& constructor;
    RenderResourcesGraph& shared_resources;
    TextureLoadingGraph& texture_loading;
    NativeVertexDeclarationCacheContext& declarations;
    GuiTextNativeLayoutServices& layouts;
    NativeStringRawPoolContext& raw_strings;
    GameNativeVfsRawServices& vfs;
    GameVfsHost& files;
    GameSingletonHost& singletons;
    NativeRenderActualOwnerRegistry& owners;
    NativeLuaServiceBindings& lua;
    NativeShaderBinaryCacheContext& shader_cache;
    void* const volatile& renderer;
    void* const volatile& entry_cache;
    GameInputRuntime& input;
    const NativeFrameClockPublicationContext& clock;
    NativeRenderResourceInitArguments arguments;
    std::function<GameNativeMissionUnitSource()> unit_source;
    const GameUnitsHost* bound_units{}; // Identity only; reborrow from mission before every use.
    const GameMissionLuaHost* bound_lua{};
    const bsp::ScenePropertyBlock* bound_root_properties{};
    const NativeShaderBinaryCacheOperation* shader_creation{};
    std::size_t inspected_units{};
    NativeMissionFrameSwitches frame_switches;
    NativeMissionFrameCoordinator frame_coordinator;
    GameRenderFrameState frame_state;
    void* prepare_context{};
    void (*prepare_callback)(void*){};
    const NativeGroupEnvironment* gui_group_environment{};
    RandomThreads* startup_random{};
    StartupSceneDomain* startup_domain{}; // A1 borrow; null unless kNativeRendererStartupCc15Bound
    bool all_units{};
    std::optional<std::size_t> diagnostic_view_unit;
    bool overlay_window_open{}, overlay_submitted{};
    Stage stage{Stage::prepared};
    bool shader_open{};
    std::uint64_t last_mission_frame{};
    // Reverse destruction keeps dependencies until all consumer companions
    // have closed, after their genuine native owners have already retired.
    std::unique_ptr<GameNativeMissionModel> model;
    std::unique_ptr<GameNativeMissionDevices> devices_visual;
    std::unique_ptr<GameNativeMissionFrameJobs> frame_jobs;
    std::unique_ptr<GameNativeMissionMaterialRefresh> material_refresh;
    std::unique_ptr<GameNativeMissionFactories> owned_factories;
    GameNativeMissionFactories* factories{};
    std::unique_ptr<MissionShipShaderPreloadGraph> owned_preload;
    MissionShipShaderPreloadGraph* preload{};
    std::unique_ptr<MissionShipFoliageGraph> foliage;
    std::unique_ptr<MissionShipCameraGraph> camera;
    std::unique_ptr<MissionCommandGraph> commands;
    std::unique_ptr<MissionRenderResourcesGraph> resources;
    std::unique_ptr<MissionShipSceneGraph> scene;
    std::unique_ptr<MissionShipLightingGraph> lighting;

    MissionShipGraph(GameHostLog& l, DestructionGraph& g, DeviceGraph& d,
        CameraGraph& c, FrameGraph& f, NativeRendererConstructorContext& ctor,
        RenderResourcesGraph& r, TextureLoadingGraph& textures,
        NativeVertexDeclarationCacheContext& dc, GuiTextNativeLayoutServices& ls,
        NativeStringRawPoolContext& strings, GameNativeVfsRawServices& vs,
        GameVfsHost& vf, GameSingletonHost& host, NativeRenderActualOwnerRegistry& actual,
        NativeLuaServiceBindings& binding, NativeShaderBinaryCacheContext& shaders,
        void* const volatile& current_renderer, void* const volatile& cache,
        GameInputRuntime& in, const NativeFrameClockPublicationContext& time,
        NativeRenderResourceInitArguments args, std::function<GameNativeMissionUnitSource()> units)
        : log(l), graph(g), devices(d), cameras(c), frames(f), constructor(ctor),
          shared_resources(r), texture_loading(textures), declarations(dc), layouts(ls),
          raw_strings(strings), vfs(vs), files(vf), singletons(host), owners(actual),
          lua(binding), shader_cache(shaders), renderer(current_renderer), entry_cache(cache),
          input(in), clock(time), arguments(args), unit_source(std::move(units)) {
        check(static_cast<bool>(unit_source), "mission rendering requires its existing simulation owner");
        frame_switches = native_mission_frame_switches_from_environment(true);
        char all[2]{};
        all_units = GetEnvironmentVariableA("BSP_NATIVE_MISSION_UNITS", all, sizeof all) == 1 && all[0] == '1';
        frame_state.game_state = static_cast<std::uint32_t>(GameStateId::kInMission);
        frame_state.interface_manager_present = true;
        frame_state.interface_level = 1;
        char selected[32]{};
        const auto length = GetEnvironmentVariableA("BSP_NATIVE_MISSION_VIEW_UNIT", selected, sizeof selected);
        if (length) {
            check(length < sizeof selected, "native mission diagnostic view unit is too long");
            char* end{};
            const auto index = std::strtoul(selected, &end, 10);
            check(end != selected && *end == '\0', "native mission diagnostic view unit must be an index");
            diagnostic_view_unit = static_cast<std::size_t>(index);
        }
    }
    ~MissionShipGraph() {
        if (stage != Stage::prepared && stage != Stage::closed) std::terminate();
    }
    bool owns_live_layouts() const noexcept {
        return stage == Stage::ready || stage == Stage::retired;
    }
    bool owns_live_shader_cache() const noexcept {
        return stage == Stage::ready && shader_open && shader_creation &&
            shader_creation->phase == NativeShaderBinaryCacheOperation::Phase::complete &&
            shader_creation->cache && shader_creation->cache == shader_cache.actual_cache_0108d6ec;
    }
    void release_late_model_shader_cache(GameNativeRendererApplication& application) {
        if (!shader_open) return;
        // 00737F30 releases the borrowed Application's preload rows before
        // the later renderer drain. Its late-model bracket must close first.
        // The drain reaches this same one-shot release after entering retiring.
        check((stage == Stage::ready || stage == Stage::retiring) && shader_creation &&
            shader_creation == application.shader_cache_creation_operation() &&
            shader_creation->phase == NativeShaderBinaryCacheOperation::Phase::complete &&
            shader_creation->cache && shader_creation->cache == shader_cache.actual_cache_0108d6ec &&
            !application.shader_cache_released(),
            "mission shader-cache release requires its completed current owner");
        application.release_shader_cache();
        shader_open = false;
    }
    bool requires_process_retention() const noexcept {
        return stage == Stage::preparing || stage == Stage::retiring || stage == Stage::failed ||
            (shader_open && !owns_live_shader_cache()) || (model && model->requires_process_retention()) ||
            (factories && factories->requires_process_retention()) ||
            (preload && preload->requires_process_retention()) ||
            (commands && commands->requires_process_retention()) ||
            (frame_jobs && frame_jobs->requires_process_retention()) ||
            (material_refresh && material_refresh->requires_process_retention()) ||
            (resources && resources->requires_process_retention()) ||
            (scene && scene->requires_process_retention()) ||
            (lighting && lighting->requires_process_retention());
    }
    void prepare(GameNativeRendererApplication& application, unsigned long generation) {
        check(stage == Stage::prepared, "mission ship preparation cannot replay");
        check(!layouts.has_live_companions(),
            "mission ship cannot adopt preexisting layout companions");
        if (factories) factories->require_lookup_ready();
        // Metadata borrows require the ready renderer with no interrupted
        // acquisition. Bind them before opening the native shader bracket.
        model = std::make_unique<GameNativeMissionModel>(log, application, files,
            singletons, graph.profiles.data, input);
        stage = Stage::preparing;
        try {
            log.note("native mission ship: BSP_NATIVE_MISSION_SHIP=1 prepare live unit models");
            check((factories != nullptr) == (preload != nullptr),
                "mission startup factory/preload borrows must share one ready domain");
            if (!factories) {
                owned_factories = std::make_unique<GameNativeMissionFactories>(log, files,
                    singletons, graph.profiles.data);
                factories = owned_factories.get();
                factories->create();
                owned_preload = std::make_unique<MissionShipShaderPreloadGraph>(log,
                    singletons.manager_publication_01090aa0(), model->effect_cache(),
                    owners, lua, shader_cache, renderer);
                preload = owned_preload.get();
                preload->create();
            }
            foliage = std::make_unique<MissionShipFoliageGraph>(log,
                singletons.manager_publication_01090aa0());
            // Explicit diagnostic foliage scalar/normal choices. No foliage
            // geometry is admitted, and no simulation/weather state is changed.
            foliage->create(0, 0);
            camera = std::make_unique<MissionShipCameraGraph>(log, cameras.environment,
                cameras.node_constants, cameras.viewports, owners, vfs.strings);
            camera->create();
            auto& particle = singletons.particle_manager_context();
            check(particle.manager_00f8c274 != nullptr,
                "mission ship requires the startup's native particle model manager");
            GameNativeMissionCommandProviders providers{model->instance_geometry(),
                model->group_models(), model->render_services(), clock,
                reinterpret_cast<void* const volatile&>(particle.manager_00f8c274),
                foliage->publication_00f8c210, nullptr};
            commands = std::make_unique<MissionCommandGraph>(log, frames, graph,
                devices, cameras, constructor, files, singletons, raw_strings,
                vfs.strings, shared_resources.publication, entry_cache, providers,
                model->matrix_model_environment(), gui_group_environment);
            if (all_units) {
                check(startup_random != nullptr, "native mission jobs require the existing startup random registry");
                frame_jobs = std::make_unique<GameNativeMissionFrameJobs>(graph.profiles.data,
                    frames.queue_getter, singletons.native_deletion_bindings(), *startup_random);
                commands->bind_preparation_jobs(*frame_jobs);
            }
            commands->before_device_generation(generation);
            check(!shader_cache.actual_cache_0108d6ec,
                "mission ship requires a fresh closed shader-cache bracket");
            application.create_shader_cache();
            shader_open = true;
            shader_creation = application.shader_cache_creation_operation();
            check(shader_creation && shader_creation->phase == NativeShaderBinaryCacheOperation::Phase::complete &&
                shader_creation->cache && shader_creation->cache == shader_cache.actual_cache_0108d6ec,
                "mission shader-cache creation did not return the canonical live owner");
            resources = std::make_unique<MissionRenderResourcesGraph>(shared_resources,
                graph, cameras, texture_loading, declarations, layouts, raw_strings,
                vfs, singletons, owners, model->effect_cache(), model->material_lifetime(),
                model->model_environment(), commands->render_access(), renderer,
                *reinterpret_cast<void* const volatile*>(devices.platform_publication.identity()),
                lua, shader_cache, arguments, startup_domain ? &startup_domain->scenes : nullptr);
            (void)resources->construct_and_initialize();
            if (prepare_callback) prepare_callback(prepare_context);
            const auto source = unit_source();
            check(source && source.root_properties, "mission rendering requires loaded simulation units and scene header");
            bound_units = source.units;
            bound_lua = source.lua;
            bound_root_properties = source.root_properties;
            GameNativeMissionUnits units(source, files);
            material_refresh = std::make_unique<GameNativeMissionMaterialRefresh>(application, graph.profiles.data);
            if (all_units) devices_visual = std::make_unique<GameNativeMissionDevices>(log, *source.units);
            load_new_unit_models(units);
            material_refresh->publish_reflection_cubemap(*source.root_properties, arguments.word_00 != 0);
            material_refresh->refresh_after_model_load();
            check(model->drawable_count() != 0, "mission has no installed unit drawables");
            update_live_poses(0);
            fit_camera();
            scene = std::make_unique<MissionShipSceneGraph>(log, resources->scenes,
                owners, game_native_renderer_scalar_process().increment_iat_00ce221c(),
                cameras.decrement, reinterpret_cast<const char*>(graph.profiles(0xce7e10)));
            scene->create();
            for (std::size_t i = 0; i != model->drawable_count(); ++i)
                scene->attach(model->drawable(i), model->drawable_reference(i), model->profile(model->drawable(i)));
            lighting = std::make_unique<MissionShipLightingGraph>(log, graph, cameras,
                owners, raw_strings, static_cast<std::uint8_t>(arguments.word_00),
                startup_domain ? &startup_domain->registry : nullptr);
            lighting->create_and_bind(scene->raw());
            // Keep this SAME one-shot cache while late mission models can load.
            // B3B7F4/B34890 dereference it in the installed binary shader mode.
            // A completed current-owner cache is retired exactly once below.
            stage = Stage::ready;
            log.notef("native mission ship ready: renderer=%p service=%p queue=%p scene=%p camera=%p "
                "models=%zu sections=%zu generation=%lu init_args=%u,%u,%u",
                renderer, shared_resources.publication, frames.queue, scene->raw(), camera->raw,
                model->drawable_count(), model->section_count(), generation,
                arguments.word_00, arguments.word_04, arguments.word_08);
        } catch (const std::exception& error) {
            stage = Stage::failed;
            log.notef("native mission ship prepare failed: %s retention=process", error.what());
            throw;
        } catch (...) { stage = Stage::failed; throw; }
    }
    std::size_t load_new_unit_models(GameNativeMissionUnits& units) {
        const auto count = units.count();
        check(count >= inspected_units, "mission unit indices lost their append-only owner");
        const auto first = model->drawable_count();
        const auto selected = diagnostic_view_unit.value_or(bound_units->controlled_bound()
            ? bound_units->controlled_index() : std::size_t{0});
        for (std::size_t i = inspected_units; i != count; ++i) {
            if (!all_units && i != selected) continue;
            GameNativeMissionUnitDescriptor descriptor;
            if (!units.descriptor(i, descriptor)) {
                log.notef("native mission unit skipped: index=%zu blocker=no installed class Mesh", i);
                continue;
            }
            model->load_installed_unit(descriptor.model_logical_name, i);
            if (bound_units->unit_is_kind_of(i, static_cast<int>(VehicleClassKind::PlaneBase)))
                check(model->configure_plane_default_visibility(i), "native plane defaults lost their loaded unit");
            log.notef("native mission unit: index=%zu name=%s type=%d asset=%s models=%zu stationary=%d",
                i, descriptor.instance_name.c_str(), descriptor.type_id,
                descriptor.model_logical_name.c_str(), model->unit_drawable_count(i),
                descriptor.stationary_prop);
        }
        if (devices_visual) for (const auto& binding : devices_visual->bindings()) {
            if (binding.mesh.empty() || model->unit_loaded(binding.render_key)) continue;
            model->load_installed_unit(binding.mesh, binding.render_key);
            check(model->unit_hierarchy_count(binding.render_key) == binding.hierarchy_items,
                "native device hierarchy differs from the firing host's original asset items");
            log.notef("native mission device model: key=%zu unit=%zu platform=%d class=%d asset=%s models=%zu",
                binding.render_key, binding.unit_index, binding.platform_key, binding.device_class,
                binding.mesh.c_str(), model->unit_drawable_count(binding.render_key));
        }
        if (scene && model->drawable_count() != first) {
            // Legacy model companions admit the native null-lighting B6ED80
            // branch. B723F0 changes only outer+1C and its resource credit;
            // keep the genuine resource creator while roots are attached.
            lighting->begin_root_attachment();
            for (std::size_t i = first; i != model->drawable_count(); ++i)
                scene->attach(model->drawable(i), model->drawable_reference(i), model->profile(model->drawable(i)));
            lighting->end_root_attachment();
        }
        inspected_units = count;
        return model->drawable_count() - first;
    }
    void refresh_units() {
        check(stage == Stage::ready, "mission refresh requires its complete render graph");
        try {
            const auto source = unit_source();
            check(source && source.units == bound_units && source.lua == bound_lua &&
                source.root_properties == bound_root_properties,
                "mission render units changed owner before graph retirement");
            GameNativeMissionUnits units(source, files);
            if (devices_visual && devices_visual->refresh_source()) {
                const auto& census = devices_visual->census();
                log.notef("native mission device source extended before BeginScene: gun_rows=%zu physical_platforms=%zu visible_instances=%zu",
                    census.gun_rows, census.physical_platforms, census.visible_instances);
            }
            if (load_new_unit_models(units) != 0) {
                material_refresh->publish_reflection_cubemap(*source.root_properties, arguments.word_00 != 0);
                material_refresh->refresh_after_model_load();
                update_live_poses(0);
                fit_camera();
            }
        } catch (...) { stage = Stage::failed; throw; }
    }
    void update_live_poses(std::uint64_t mission_frame) {
        const auto source = unit_source();
        check(source && source.units == bound_units && source.lua == bound_lua &&
            source.root_properties == bound_root_properties,
            "mission render units changed owner before graph retirement");
        GameNativeMissionUnits units(source, files);
        if (devices_visual) devices_visual->validate_source();
        for (std::size_t i = 0; i != units.count(); ++i) {
            if (!model->unit_loaded(i)) continue;
            CameraMatrix current;
            const bool posed = units.read_transform(i, current);
            const bool visible = posed && units.visible(i);
            model->set_unit_visible(i, visible);
            if (!posed) continue;
            check(model->update_unit_transform(i, current), "live unit lost its native models");
            if (mission_frame == 1 || mission_frame == 60 || mission_frame == 180)
                log.notef("native mission pose: frame=%llu unit=%zu position=%.6f,%.6f,%.6f "
                    "forward=%.6f,%.6f,%.6f visible=%d path=B6DB10 source=unit_pose",
                    static_cast<unsigned long long>(mission_frame), i,
                    static_cast<double>(current[12]), static_cast<double>(current[13]),
                    static_cast<double>(current[14]), static_cast<double>(current[8]),
                    static_cast<double>(current[9]), static_cast<double>(current[10]), visible);
        }
        if (devices_visual) for (const auto& binding : devices_visual->bindings()) {
            if (binding.mesh.empty()) continue;
            bool posed = true;
            for (std::size_t item = 0; item != binding.hierarchy_items; ++item) {
                CameraMatrix current;
                if (!devices_visual->current_node_world(binding.render_key, item, current)) {
                    posed = false;
                    break;
                }
                // Metadata-only hierarchy items have no actual visual/palette
                // node. The model facade returns false for exactly that case.
                (void)model->update_hierarchy_transform(binding.render_key, item, current);
            }
            model->set_unit_visible(binding.render_key, posed && units.visible(binding.unit_index));
        }
    }
    void fit_camera() {
        std::vector<std::array<float, 4>> spheres;
        CameraAxis low{}, high{};
        const auto source = unit_source();
        check(source && source.units == bound_units && source.lua == bound_lua,
            "mission camera lost its canonical unit source");
        const auto selected = diagnostic_view_unit.value_or(source.units->controlled_bound()
            ? source.units->controlled_index() : std::size_t{0});
        check(!diagnostic_view_unit || model->unit_loaded(selected),
            "native mission diagnostic view unit has no loaded model");
        const bool focus_loaded = model->unit_loaded(selected);
        for (std::size_t i = 0; i != model->drawable_count(); ++i) {
            if (focus_loaded && model->drawable_unit_index(i) != selected) continue;
            const auto* raw_sphere = get_native_model_world_sphere_00b6e8c0(
                model->drawable(i), &commands->render_access());
            std::array<float, 4> sphere;
            std::memcpy(sphere.data(), raw_sphere, sizeof sphere);
            for (float value : sphere)
                check(std::isfinite(value), "installed ship has a nonfinite native world sphere");
            check(sphere[3] >= 0.0f, "installed ship has a negative native world sphere");
            for (std::size_t axis = 0; axis != 3; ++axis) {
                const float lower = sphere[axis] - sphere[3];
                const float upper = sphere[axis] + sphere[3];
                if (spheres.empty() || lower < low[axis]) low[axis] = lower;
                if (spheres.empty() || upper > high[axis]) high[axis] = upper;
            }
            spheres.push_back(sphere);
        }
        const CameraAxis center{(low[0] + high[0]) * 0.5f,
            (low[1] + high[1]) * 0.5f, (low[2] + high[2]) * 0.5f};
        float radius = 0.0f;
        for (const auto& sphere : spheres) {
            const float x = sphere[0] - center[0], y = sphere[1] - center[1], z = sphere[2] - center[2];
            const float extent = std::sqrt(x*x + y*y + z*z) + sphere[3];
            if (extent > radius) radius = extent;
        }
        camera->fit_ship(center, radius);
    }
    void update_view(std::uint64_t mission_frame, unsigned long generation) {
        check(stage == Stage::ready, "mission ship draw requires completed preparation");
        try {
            commands->before_device_generation(generation);
            update_live_poses(mission_frame);
            fit_camera();
            camera->update_viewport();
            resources->prepare_screen_matrix();
            last_mission_frame = mission_frame;
        } catch (...) { stage = Stage::failed; throw; }
    }
    void submit_scene() {
        check(stage == Stage::ready, "mission submission requires its completed view update");
        try { commands->submit_scene(scene->raw(), camera->raw, nullptr); }
        catch (...) { stage = Stage::failed; throw; }
    }
    void execute_scene(std::uint64_t mission_frame, float simulated_seconds) {
        check(stage == Stage::ready, "mission execution requires completed scene submission");
        try {
            commands->execute_before_overlay();
            if (mission_frame == 1 || mission_frame == 60 || mission_frame == 180)
                log.notef("native mission ship capture landmark: mission_frame=%llu simulated_seconds=%.6f "
                    "scene=%p camera=%p models=%zu", static_cast<unsigned long long>(mission_frame),
                    static_cast<double>(simulated_seconds), scene->raw(), camera->raw,
                    model->drawable_count());
        } catch (...) { stage = Stage::failed; throw; }
    }
    void draw(std::uint64_t mission_frame, float simulated_seconds, unsigned long generation) {
        update_view(mission_frame, generation);
        submit_scene();
        execute_scene(mission_frame, simulated_seconds);
    }
    void complete_frame() {
        if (commands) commands->complete_frame();
    }
    void observe_failed_frame() noexcept {
        if (frame_jobs) frame_jobs->observe_interrupted();
        if (commands) commands->observe_failed_frame();
        stage = Stage::failed;
    }
    void retire_before_native_drain(GameNativeRendererApplication& application) {
        if (stage == Stage::closed) return;
        if (stage == Stage::prepared) {
            if (!owned_factories) factories = nullptr;
            if (!owned_preload) preload = nullptr;
            stage = Stage::closed;
            return;
        }
        check(stage == Stage::ready && !requires_process_retention(),
            "incomplete mission ship graph cannot be retired");
        stage = Stage::retiring;
        try {
            release_late_model_shader_cache(application);
            if (owned_preload) preload->retire_before_native_drain();
            lighting->retire_before_native_drain();
            scene->retire_before_native_drain();
            model->retire_drawables();
            camera->retire_caller_reference();
            foliage->retire_before_native_drain();
            if (owned_factories) factories->retire_before_native_drain();
            stage = Stage::retired;
        } catch (...) { stage = Stage::failed; throw; }
    }
    void after_native_drain() {
        if (stage == Stage::closed) return;
        check(stage == Stage::retired, "mission ship survived native drain");
        lighting->after_native_drain();
        resources->after_native_drain();
        scene->after_native_drain();
        camera->after_native_drain();
        if (owned_preload) preload->after_native_drain();
        if (owned_factories) factories->after_native_drain();
        model->after_native_renderer_drain();
        if (frame_jobs) frame_jobs->after_native_drain();
        // The host may now acknowledge/drop its shared factory owner. Closed
        // mission metadata must never inspect either borrowed domain again.
        if (!owned_factories) factories = nullptr;
        if (!owned_preload) preload = nullptr;
        stage = Stage::closed;
        log.notef("native mission ship closed: last_mission_frame=%llu camera=retired scene=retired "
            "models=retired foliage=retired resources=retired",
            static_cast<unsigned long long>(last_mission_frame));
    }
};
