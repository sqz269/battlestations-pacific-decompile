// One real73BF80 operation over the startup Application's existing08h vector.
// The owner is installed before open() and survives effects, native drain and
// every entered failure. Include after MissionShipShaderPreloadGraph.
struct StartupShaderPreload final {
    enum class Phase { prepared, opening, ready, retired, closing, closed, failed };
    Phase phase{Phase::prepared};
    std::unique_ptr<GameNativeGuiSamplerApplication> samplers;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    std::unique_ptr<MissionShipShaderPreloadGraph> graph;

    ~StartupShaderPreload() {
        if (phase != Phase::prepared && phase != Phase::closed) std::terminate();
    }
    void open(GameHostLog& log, GameNativeRendererApplication& application,
        GameVfsHost& files, GameSingletonHost& singletons, GameNativeReadOnlyData& data,
        GameInputRuntime& input, NativeRenderActualOwnerRegistry& owners,
        NativeLuaServiceBindings& lua, NativeShaderBinaryCacheContext& cache,
        void* const volatile& renderer) {
        check(phase == Phase::prepared, "startup shader preload cannot replay");
        phase = Phase::opening;
        try {
            samplers = std::make_unique<GameNativeGuiSamplerApplication>(log, application,
                files, singletons, data, GameNativeGuiSamplerApplicationBindings{input,
                    application.borrow_platform_publication_0109cf04()});
            effects = std::make_unique<GameNativeGuiEffectApplication>(log, application,
                files, samplers->effect_bindings());
            graph = std::make_unique<MissionShipShaderPreloadGraph>(log,
                singletons.manager_publication_01090aa0(), effects->native_effect_cache(),
                owners, lua, cache, renderer);
            graph->create();
            phase = Phase::ready;
        } catch (...) { phase = Phase::failed; throw; }
    }
    bool requires_process_retention() const noexcept {
        return (phase != Phase::prepared && phase != Phase::ready &&
                phase != Phase::retired && phase != Phase::closed) ||
            (graph && graph->requires_process_retention()) ||
            (effects && effects->requires_process_retention()) ||
            (samplers && samplers->requires_process_retention());
    }
    void release_rows() {
        if (phase == Phase::retired || phase == Phase::closed) return;
        check(!requires_process_retention() && graph,
            "startup preload row release requires its completed graph");
        try { graph->release_borrowed_rows(); }
        catch (...) { phase = Phase::failed; throw; }
    }
    void retire_before_native_drain() {
        if (phase == Phase::retired || phase == Phase::closed) return;
        check(phase == Phase::ready && !requires_process_retention(),
            "startup preload retirement requires quiescent consumers");
        try { graph->retire_before_native_drain(); phase = Phase::retired; }
        catch (...) { phase = Phase::failed; throw; }
    }
    void after_native_drain() {
        if (phase == Phase::closed) return;
        check(phase == Phase::retired, "startup preload survived native drain");
        phase = Phase::closing;
        try {
            graph->after_native_drain();
            effects->after_native_renderer_drain();
            samplers->after_native_renderer_drain();
            phase = Phase::closed;
        } catch (...) { phase = Phase::failed; throw; }
    }
};
