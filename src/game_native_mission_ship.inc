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
    Stage stage{Stage::prepared};
    bool shader_open{};
    std::uint64_t last_mission_frame{};
    // Reverse destruction keeps dependencies until all consumer companions
    // have closed, after their genuine native owners have already retired.
    std::unique_ptr<GameNativeMissionModel> model;
    std::unique_ptr<MissionShipShaderPreloadGraph> preload;
    std::unique_ptr<MissionShipFoliageGraph> foliage;
    std::unique_ptr<MissionShipCameraGraph> camera;
    std::unique_ptr<MissionCommandGraph> commands;
    std::unique_ptr<MissionRenderResourcesGraph> resources;
    std::unique_ptr<MissionShipSceneGraph> scene;

    MissionShipGraph(GameHostLog& l, DestructionGraph& g, DeviceGraph& d,
        CameraGraph& c, FrameGraph& f, NativeRendererConstructorContext& ctor,
        RenderResourcesGraph& r, TextureLoadingGraph& textures,
        NativeVertexDeclarationCacheContext& dc, GuiTextNativeLayoutServices& ls,
        NativeStringRawPoolContext& strings, GameNativeVfsRawServices& vs,
        GameVfsHost& vf, GameSingletonHost& host, NativeRenderActualOwnerRegistry& actual,
        NativeLuaServiceBindings& binding, NativeShaderBinaryCacheContext& shaders,
        void* const volatile& current_renderer, void* const volatile& cache,
        GameInputRuntime& in, const NativeFrameClockPublicationContext& time,
        NativeRenderResourceInitArguments args)
        : log(l), graph(g), devices(d), cameras(c), frames(f), constructor(ctor),
          shared_resources(r), texture_loading(textures), declarations(dc), layouts(ls),
          raw_strings(strings), vfs(vs), files(vf), singletons(host), owners(actual),
          lua(binding), shader_cache(shaders), renderer(current_renderer), entry_cache(cache),
          input(in), clock(time), arguments(args) {}
    ~MissionShipGraph() {
        if (stage != Stage::prepared && stage != Stage::closed) std::terminate();
    }
    bool owns_live_layouts() const noexcept {
        return stage == Stage::ready || stage == Stage::retired;
    }
    bool requires_process_retention() const noexcept {
        return stage == Stage::preparing || stage == Stage::retiring || stage == Stage::failed ||
            shader_open || (model && model->requires_process_retention()) ||
            (preload && preload->requires_process_retention()) ||
            (commands && commands->requires_process_retention()) ||
            (resources && resources->requires_process_retention()) ||
            (scene && scene->requires_process_retention());
    }
    void prepare(GameNativeRendererApplication& application, unsigned long generation) {
        check(stage == Stage::prepared, "mission ship preparation cannot replay");
        check(!layouts.has_live_companions(),
            "mission ship cannot adopt preexisting layout companions");
        // Metadata borrows require the ready renderer with no interrupted
        // acquisition. Bind them before opening the native shader bracket.
        model = std::make_unique<GameNativeMissionModel>(log, application, files,
            singletons, graph.profiles.data, input);
        stage = Stage::preparing;
        try {
            log.note("native mission ship: BSP_NATIVE_MISSION_SHIP=1 prepare installed Fletcher");
            preload = std::make_unique<MissionShipShaderPreloadGraph>(log,
                singletons.manager_publication_01090aa0(), model->effect_cache(),
                owners, lua, shader_cache, renderer);
            preload->create();
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
                vfs.strings, shared_resources.publication, entry_cache, providers);
            commands->before_device_generation(generation);
            check(!shader_cache.actual_cache_0108d6ec,
                "mission ship requires a fresh closed shader-cache bracket");
            application.create_shader_cache();
            shader_open = true;
            resources = std::make_unique<MissionRenderResourcesGraph>(shared_resources,
                graph, cameras, texture_loading, declarations, layouts, raw_strings,
                vfs, singletons, owners, model->effect_cache(), model->material_lifetime(),
                model->model_environment(), commands->render_access(), renderer,
                *reinterpret_cast<void* const volatile*>(devices.platform_publication.identity()),
                lua, shader_cache, arguments);
            (void)resources->construct_and_initialize();
            model->load_installed_ship("models/ships/us/fletcher.mmod");
            check(model->drawable_count() != 0, "installed Fletcher has no native drawables");
            fit_camera();
            scene = std::make_unique<MissionShipSceneGraph>(log, resources->scenes,
                owners, game_native_renderer_scalar_process().increment_iat_00ce221c(),
                cameras.decrement, reinterpret_cast<const char*>(graph.profiles(0xce7e10)));
            scene->create();
            for (std::size_t i = 0; i != model->drawable_count(); ++i)
                scene->attach(model->drawable(i), model->drawable_reference(i));
            application.release_shader_cache();
            shader_open = false;
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
    void fit_camera() {
        std::vector<std::array<float, 4>> spheres;
        CameraAxis low{}, high{};
        for (std::size_t i = 0; i != model->drawable_count(); ++i) {
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
    void draw(std::uint64_t mission_frame, float simulated_seconds, unsigned long generation) {
        check(stage == Stage::ready, "mission ship draw requires completed preparation");
        try {
            commands->before_device_generation(generation);
            camera->update_viewport();
            resources->prepare_screen_matrix();
            last_mission_frame = mission_frame;
            commands->submit_scene(scene->raw(), camera->raw, nullptr);
            commands->execute_before_overlay();
            if (mission_frame == 1 || mission_frame == 60 || mission_frame == 180)
                log.notef("native mission ship capture landmark: mission_frame=%llu simulated_seconds=%.6f "
                    "scene=%p camera=%p models=%zu", static_cast<unsigned long long>(mission_frame),
                    static_cast<double>(simulated_seconds), scene->raw(), camera->raw,
                    model->drawable_count());
        } catch (...) { stage = Stage::failed; throw; }
    }
    void complete_frame() {
        if (commands) commands->complete_frame();
    }
    void observe_failed_frame() noexcept {
        if (commands) commands->observe_failed_frame();
        stage = Stage::failed;
    }
    void retire_before_native_drain() {
        if (stage == Stage::prepared) { stage = Stage::closed; return; }
        check(stage == Stage::ready && !requires_process_retention(),
            "incomplete mission ship graph cannot be retired");
        stage = Stage::retiring;
        try {
            preload->retire_before_native_drain();
            scene->retire_before_native_drain();
            model->retire_drawables();
            camera->retire_caller_reference();
            foliage->retire_before_native_drain();
            stage = Stage::retired;
        } catch (...) { stage = Stage::failed; throw; }
    }
    void after_native_drain() {
        if (stage == Stage::closed) return;
        check(stage == Stage::retired, "mission ship survived native drain");
        resources->after_native_drain();
        scene->after_native_drain();
        camera->after_native_drain();
        preload->after_native_drain();
        model->after_native_renderer_drain();
        stage = Stage::closed;
        log.notef("native mission ship closed: last_mission_frame=%llu camera=retired scene=retired "
            "models=retired foliage=retired resources=retired",
            static_cast<unsigned long long>(last_mission_frame));
    }
};
