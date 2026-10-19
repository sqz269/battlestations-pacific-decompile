// ON-only composition of the genuine 1Ch Application constructor, 73BF80
// shader preload and 737F80..737FB0 caller-reference retirement. Include after
// the native preload/application/effect-owner and string-process declarations.
// The complete preload owns its own shader-cache bracket BEFORE B107F0.
// With kApplicationCc15Bound ON the preload borrows the startup host's
// Application (0073D410 passes its own ECX to 73BF80 at 0073DD12; the startup
// host's 00737970 published it at E1AE90). The host then releases the rows in
// 00737F30 (737F80..737FB0) and retires the object through 007379A0 itself.
struct MissionShipShaderPreloadGraph final {
    // Establish the actual typed field lifetimes without initializing any
    // preimage. WinMain also owns an enclosing 1Ch object (on its stack).
    // Only genuine737970/BEA970/BEA810 initialize these fields/publication.
    struct ApplicationStorage final {
        std::uint32_t profile_00;
        std::uint8_t loop_flag_04;
        std::array<std::byte, 3> preserved_05;
        NativeApplicationPointerVectorStorage preloaded_08;
        void* game_14;
        std::uint8_t first_update_18, byte_19, byte_1a;
        std::byte preserved_1b;
        ApplicationStorage() noexcept {} // Deliberately no value initialization.
    };
    static_assert(sizeof(ApplicationStorage) == 0x1c);
    static_assert(offsetof(ApplicationStorage, preloaded_08) == 8);
    static_assert(offsetof(ApplicationStorage, game_14) == 0x14);
    static_assert(offsetof(ApplicationStorage, first_update_18) == 0x18);
    enum class Phase { prepared, constructing, preloading, ready, retiring, retired, failed, closed };
    GameHostLog& log;
    NativeRenderActualOwnerRegistry& owners;
    NativeLuaServiceBindings& lua;
    NativeShaderPreloadContext context;
    NativeApplicationSingletonBaseContext singleton;
    NativeApplicationPointerVectorAllocation allocation;
    ApplicationStorage application;
    // The Application 73BF80 fills: the owned storage above, or the borrowed one.
    ApplicationStorage* target{&application};
    bool borrowed{};
    NativeShaderPreloadOperation operation;
    std::vector<NativeRenderActualOwnerReleaseObservation> releases;
    Phase phase{Phase::prepared};
    std::uint32_t next_release{};

    MissionShipShaderPreloadGraph(GameHostLog& l, void* volatile& manager,
        NativeMaterialEffectCacheContext& materials, NativeRenderActualOwnerRegistry& actual,
        NativeLuaServiceBindings& binding, NativeShaderBinaryCacheContext& cache,
        void* const volatile& renderer)
        : log(l), owners(actual), lua(binding),
          context{materials.strings, binding.bootstrap(), binding.files(), cache,
              materials, renderer, materials.effects.renderer_profile_00d5f0a8},
          singleton{manager, game_native_string_process().application_00e1ae90()},
          allocation{cache.allocate_array_00bf55be, cache.free_array_00bf6989} {}
    ~MissionShipShaderPreloadGraph() {
        if (phase != Phase::prepared && phase != Phase::closed) std::terminate();
    }
    template<class T> static T field(const void* owner, std::uint32_t offset = 0) noexcept {
        return *reinterpret_cast<const volatile T*>(static_cast<const char*>(owner) + offset);
    }
    void validate_effect(void* raw) {
        auto* effect = dynamic_cast<NativeMaterialEffectReference*>(&owners.resolve_actual(raw));
        check(effect && &effect->storage() == raw &&
            &effect->reference_count == &effect->storage().references_04 &&
            effect->reference_count.load() > 0 &&
            (effect->storage().vtable_00 == 0x00d5e534 || effect->storage().vtable_00 == 0x00d61a00),
            "mission preload requires a live canonical actual effect caller reference");
    }
    void create() {
        check(phase == Phase::prepared, "mission shader preload cannot replay");
        auto& process = game_native_string_process();
        const auto fresh = [](const ApplicationStorage& a) noexcept {
            return a.profile_00 == 0x00cfeab0 && a.game_14 == nullptr &&
                a.preloaded_08.data_00 == nullptr && a.preloaded_08.count_04 == 0 &&
                a.preloaded_08.capacity_08 == 0;
        };
        if constexpr (kApplicationCc15Bound) {
            // 0073DD12: ECX is 0073D410's own receiver, WinMain's stack Application.
            check(&singleton.actual_manager_01090aa0 == &process.manager_01090aa0() &&
                singleton.actual_manager_01090aa0 && singleton.actual_application_e1ae90 &&
                !context.cache.actual_cache_0108d6ec,
                "mission preload requires the startup Application at E1AE90 and a closed shader cache");
            target = static_cast<ApplicationStorage*>(singleton.actual_application_e1ae90);
            check(fresh(*target), "borrowed startup Application does not hold an empty preload vector");
            borrowed = true;
            log.notef("native mission preload borrows the startup Application: storage=%p",
                static_cast<void*>(target));
        } else {
            check(&singleton.actual_manager_01090aa0 == &process.manager_01090aa0() &&
                singleton.actual_manager_01090aa0 && !singleton.actual_application_e1ae90 &&
                !context.cache.actual_cache_0108d6ec,
                "mission preload requires the sole canonical manager/application and a closed shader cache");
            check(process.claim_application_domain(), "mission Application domain was already claimed");
        }
        phase = Phase::constructing;
        try {
            if (!borrowed) {
                construct_native_application_00737970(&application, singleton);
                check(singleton.actual_application_e1ae90 == &application && fresh(application),
                    "native Application constructor did not establish its actual preload vector");
            }
            phase = Phase::preloading;
            NativeLuaServiceBindings::Activation activation(lua);
            preload_native_application_shaders_0073bf80(target, context, operation);
            check(operation.phase() == NativeShaderPreloadPhase::complete &&
                !operation.retains_native_state() && !context.cache.actual_cache_0108d6ec &&
                target->preloaded_08.count_04 >= 0 &&
                static_cast<std::uint32_t>(target->preloaded_08.count_04) == operation.appended_entries(),
                "native shader preload did not finish its independent cache bracket");
            for (std::int32_t index = 0; index < target->preloaded_08.count_04; ++index) {
                void* const effect = target->preloaded_08.data_00[index];
                if (effect) validate_effect(effect);
            }
            phase = Phase::ready;
            log.notef("native mission preload complete: application=%p entries=%u site=%08x cache=closed",
                static_cast<void*>(target), operation.appended_entries(), operation.active_call_site());
        } catch (...) {
            phase = Phase::failed;
            log.notef("native mission preload retained: phase=%u site=%08x appended=%u",
                static_cast<unsigned>(operation.phase()), operation.active_call_site(), operation.appended_entries());
            throw;
        }
    }
    void retire_before_native_drain() {
        if (phase == Phase::retired || phase == Phase::closed) return;
        check(phase == Phase::ready && !context.cache.actual_cache_0108d6ec &&
            singleton.actual_application_e1ae90 == target,
            "mission preload retirement requires the completed quiescent native Application");
        phase = Phase::retiring;
        try {
            release_rows();
            // A borrowed Application is retired by its owner at 008F8444.
            if (!borrowed) {
                //7379A0 resizes to0, frees CURRENT vector allocation, then genuine
                //BEA990/BEA8B0 unregisters and clears the canonical publication.
                retire_native_application_007379a0(&application, application.preloaded_08, allocation, singleton);
                check(!singleton.actual_application_e1ae90,
                    "native Application retirement left its canonical publication live");
            }
            phase = Phase::retired;
        } catch (...) { phase = Phase::failed; throw; }
    }
    // 00737F30's row loop over the borrowed startup Application, before the
    // startup host retires it through 007379A0.
    void release_borrowed_rows() {
        if (!borrowed || phase == Phase::retired) return;
        retire_before_native_drain();
        log.notef("native mission preload rows released: application=%p rows=%u",
            static_cast<void*>(target), next_release);
    }
    void release_rows() {
        const auto count = static_cast<std::uint32_t>(target->preloaded_08.count_04);
        releases.resize(count); // Retained before any native reference release.
        // Original737F80..737FB0 reloads data/count for each ascending row,
        // decrements the actual+04 once, current-profile0 on zero, then
        // clears the captured nonnull row. The canonical helper preserves
        // that release/terminal order without a callable numeric vtable.
        while (next_release < static_cast<std::uint32_t>(field<std::int32_t>(target, 0x0c))) {
            void** const data = field<void**>(target, 8);
            void** const row = data + next_release;
            void* const effect = field<void*>(row);
            if (effect) {
                validate_effect(effect);
                release_native_render_actual_owner_observed(owners, effect, releases.at(next_release));
                *static_cast<void* volatile*>(row) = nullptr;
            }
            ++next_release;
        }
    }
    bool requires_process_retention() const noexcept {
        return phase == Phase::constructing || phase == Phase::preloading ||
            phase == Phase::retiring || phase == Phase::failed || operation.retains_native_state();
    }
    void after_native_drain() {
        if (phase == Phase::closed) return;
        if (phase == Phase::prepared) { phase = Phase::closed; return; }
        check(phase == Phase::retired && !singleton.actual_application_e1ae90 &&
            !context.actual_renderer_00f8d394 && !context.cache.actual_cache_0108d6ec &&
            operation.phase() == NativeShaderPreloadPhase::complete && !operation.retains_native_state(),
            "mission preload metadata requires completed native retirement and renderer drain");
        phase = Phase::closed;
    }
};
