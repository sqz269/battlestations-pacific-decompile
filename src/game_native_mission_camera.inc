// ON-only host composition of existing native camera/fog operations. This is
// a diagnostic view, not recovered mission camera selection or Original ABI.
// B71A80: ECX=45Ch pool slot, stack name, EAX=same slot, RET4.
// B700E0: ECX=camera, stack eye/target, RET8; the export misses target.
// Listing B700E7/B70104 and the existing raw constructor establish these inputs.
// All float arithmetic, camera caches and final native release use their
// reconstructed providers; no callable native table or storage default is made.
struct MissionShipCameraGraph final {
    GameHostLog& log;
    NativeCameraEnvironment& environment;
    const NativeNodeRawConstants& constants;
    NativeViewportRegistry& viewports;
    NativeRenderActualOwnerRegistry& actual;
    NativeStringStorage& strings;
    NativeViewportRegistry::Storage viewport_record;
    NativeString name;
    void* raw{};
    std::unique_ptr<NativeCameraOwner> owner;
    std::unique_ptr<NativeCameraReference> reference;
    bool entered{}, name_live{}, caller_released{}, retired{}, finished{};

    MissionShipCameraGraph(GameHostLog& l, NativeCameraEnvironment& e,
        const NativeNodeRawConstants& c, NativeViewportRegistry& v,
        NativeRenderActualOwnerRegistry& a, NativeStringStorage& s)
        : log(l), environment(e), constants(c), viewports(v), actual(a), strings(s) {}
    ~MissionShipCameraGraph() {
        // An interrupted native operation is retained by the application until
        // process exit. Never infer an unwind from partially written storage.
        if (entered && !finished) std::terminate();
    }
    static void retired_camera(void* context, NativeCameraReference& camera) noexcept {
        auto& self = *static_cast<MissionShipCameraGraph*>(context);
        self.actual.unbind(self.raw, camera); // opaque key; slot is already free
        self.raw = nullptr;
        self.retired = true;
    }
    void create() {
        check(!entered, "mission camera construction cannot be replayed");
        entered = true;
        name_live = true;
        // The mission constructor's Operator object is this camera/name pair;
        // it does not require an additional camera-controller reconstruction.
        name.assign_0041e870(strings, "Operator");
        raw = environment.pool_0108ffb0.allocate_raw_slot_00b71770();
        check(raw != nullptr, "mission camera pool returned null");
        // Preserve the original allocation preimage, including pool+458.
        auto actual_admission = actual.prepare_binding(raw);
        auto viewport_admission = viewports.admit(viewport_record);
        owner = std::make_unique<NativeCameraOwner>(raw, NativeCameraPool::slot_bytes,
            environment);
        construct_native_camera_00b71a80(*owner, &name, constants,
            std::move(viewport_admission));
        reference = std::make_unique<NativeCameraReference>(*owner,
            NativeCameraCompanionDisposal{this, &retired_camera});
        actual.publish(std::move(actual_admission), *reference);

        auto* fog = allocate_system_fog_owner();
        // Explicit diagnostic lighting. B84E50 leaves directional records
        // +28..67 untouched: define all four via the genuine setters before
        // B46A70 can read them. These values are host choices, not native defaults.
        const float ambient[4]{0.40f, 0.42f, 0.46f, 1.0f};
        const float water[4]{0.07f, 0.12f, 0.17f, 1.0f};
        const float directional[4]{0.45f, 0.44f, 0.40f, 1.0f};
        set_system_fog_color_00b84c40(*fog, ambient);
        set_system_fog_underwater_color_00b84c70(*fog, water);
        for (std::uint32_t i = 0; i != 4; ++i)
            set_system_fog_directional_color_00b84fa0(*fog, directional, i);
        // Explicit clear-air diagnostic: the shader's distance/height maximum
        // densities are +68/+78. Keep the native nonzero range denominators.
        set_system_fog_scalar_68_00b84d00(*fog, 0u);
        set_system_fog_scalar_78_00b84d40(*fog, 0u);
        set_system_fog_camera_owner_00b71940(owner->frame.fog_184, fog);
        release_system_fog_owner(*fog); // camera owns the remaining native ref

        const CameraAxis eye{330.0f, 150.0f, -360.0f};
        const CameraAxis target{0.0f, 15.0f, 0.0f};
        set_camera_look_at_00b700e0(owner->camera, eye, target, owner->pose,
            environment.crt, environment.viewport.one_bits_00d7a24c);
        set_camera_fov_00b6fbb0(owner->projection, 1.0471975512f);
        set_camera_near_00b6fbf0(owner->projection, 1.0f);
        set_camera_far_00b6fc10(owner->projection, 10000.0f);
        update_viewport();
        // Evaluate the actual raw caches that the command/system providers
        // consume; do not project a second camera matrix into their storage.
        (void)get_native_camera_view_projection_00b70490(raw);
        log.notef("native mission ship camera: raw=%p viewport=%p fog=%p refs=%d "
            "eye=330,150,-360 target=0,15,0 fov=1.04719755 near=1 far=10000",
            raw, owner->storage.camera.viewport_180, owner->storage.camera.fog_184,
            owner->storage.node.references_04.load());
    }
    void update_viewport() {
        check(owner && !caller_released, "mission camera viewport requires live caller");
        auto* renderer = environment.viewport.renderer_00f8d394;
        check(renderer != nullptr, "mission camera lost renderer publication");
        const auto p = environment.viewport.renderer_access.parameters_00b1ff60(*renderer);
        const DWORD dimensions[2]{p.width_0c, p.height_10};
        check(dimensions[0] && dimensions[1], "mission camera has empty viewport");
        set_native_viewport_dimensions_00b1f940(*owner->storage.camera.viewport_180,
            dimensions);
        set_camera_aspect_00b6fbd0(owner->projection,
            static_cast<float>(dimensions[0]) / static_cast<float>(dimensions[1]));
    }
    void fit_ship(const CameraAxis& center, float radius) {
        check(owner && !caller_released && std::isfinite(radius) && radius > 0.0f,
            "mission diagnostic camera requires a finite installed model sphere");
        const CameraAxis eye{center[0] + radius * 1.9f,
            center[1] + radius * 0.8f, center[2] - radius * 0.9f};
        set_camera_look_at_00b700e0(owner->camera, eye, center, owner->pose,
            environment.crt, environment.viewport.one_bits_00d7a24c);
        const float far_range = radius * 5.0f + 1000.0f;
        set_camera_far_00b6fc10(owner->projection, far_range < 10000.0f ? 10000.0f : far_range);
        (void)get_native_camera_view_projection_00b70490(raw);
        log.notef("native mission diagnostic camera fit: center=%.6f,%.6f,%.6f radius=%.6f "
            "eye=%.6f,%.6f,%.6f", center[0], center[1], center[2], radius,
            eye[0], eye[1], eye[2]);
    }
    void retire_caller_reference() {
        check(owner && reference && !caller_released,
            "mission camera caller release requires its completed construction");
        // Queue/context credits must have been retired before the host stops.
        check(owner->storage.node.references_04.load() == 1,
            "mission camera still retained by a native command/context");
        caller_released = true;
        unlink_and_release_render_model_00b6dfa0(*reference);
        check(retired, "mission camera did not reach its native deleting destructor");
        if (name_live) {
            destroy_native_string_header_0041dd20(&name, strings);
            name_live = false;
        }
        // Renderer+1904 may retain the viewport after camera retirement. Keep
        // its registered view and every companion alive through native drain.
        log.note("native mission ship camera: caller retired via 00B6DFA0/00B71FE0");
    }
    void after_native_drain() {
        check(caller_released && retired && !name_live,
            "mission camera survived native drain");
        viewports.forget_quiescent(viewport_record);
        reference.reset();
        owner.reset();
        finished = true;
    }
};
