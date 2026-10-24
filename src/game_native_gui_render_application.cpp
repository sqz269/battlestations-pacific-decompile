#include "bsp/game_native_gui_render_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_native_gui_sampler_application.hpp"
#include "bsp/game_native_gui_node_application.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_text_application.hpp"
#include "bsp/game_hosts_text.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_widget_clip_refresh.hpp"
#include "bsp/native_camera_reference.hpp"
#include "bsp/native_camera_view_set.hpp"
#include "bsp/native_camera_frustum.hpp"
#include "bsp/native_cockpit_helper_construction.hpp"
#include "bsp/native_gui_scene_storage.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include "bsp/native_weak_owner.hpp"
#include "bsp/post_effects_cc13.hpp"
#include "bsp/system_fog_owner.hpp"
#include <cstring>
#include <cwctype>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace bsp::game {
namespace {
using Word = std::uint32_t;
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
template<class T> const volatile T& cell(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
const volatile Word* table(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const volatile Word*>(data.data_at(address, bytes));
}
template<class T> T read(const void* value, std::size_t offset = 0) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(value) + offset, sizeof result);
    return result;
}
std::shared_ptr<const GuiTable> vector_table(std::initializer_list<double> values) {
    auto result = std::make_shared<GuiTable>();
    for (double value : values) result->array.emplace_back(value);
    return result;
}
// The standalone bring-up submits literal, non-localized text. Context tokens
// require the application's current Lua owner and remain a rejected boundary;
// choosing an unrelated page VM here would fabricate that owner.
struct LiteralTextLocale final : LocaleTextRuntimeHost {
    std::string context_string_00b692c0(const std::string&) override {
        throw std::logic_error("standalone native Text has no current Lua-context binding");
    }
    char16_t crt_uppercase_00c0391c(char16_t value) override {
        return static_cast<char16_t>(std::towupper(static_cast<std::wint_t>(value)));
    }
};
} // namespace

struct GameNativeGuiRenderApplication::Impl {
    enum class Phase { prepared, preparing, ready, submitting, retiring, retired, closed, failed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& files;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    GameInputRuntime& input;
    GameFontHost& fonts;
    GameLocaleHost& locale;
    const bool& sse2;
    GameNativeVfsRawServices raw;
    GameNativeGuiTextState& globals;
    NativeMaterialDestructionAccess materials;
    NativeMaterialParameterAccess parameters;
    LiteralTextLocale locale_calls;
    std::unique_ptr<GameNativeGuiNodeApplication> nodes;
    std::unique_ptr<GuiMaterialBindingServices> style;
    std::unique_ptr<GuiWidgetClipRefreshServices> clip;
    std::unique_ptr<GameNativeGuiSamplerApplication> samplers;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    std::unique_ptr<GameNativeTextApplication> text;
    std::shared_ptr<GuiTable> properties;
    GuiWidgetOwner* widget{};

    NativeViewportRegistry::Storage viewport_record;
    NativeString camera_name;
    bool camera_name_live{}, camera_retired{};
    void* camera_raw{};
    std::unique_ptr<NativeCameraOwner> camera;
    std::unique_ptr<NativeCameraReference> camera_reference;
    NativeGuiSceneStorageContext scene_storage;
    NativeGuiSceneIdentityContext scene_identity;
    NativeGuiSceneConstructFrame scene_frame{};
    NativeGuiSceneConstructAcquired scene_construction;
    NativeGuiSceneDestroyFrame scene_destroy_frame{};
    NativeGuiSceneLifetimeAcquired scene_destruction;
    std::optional<NativeGuiSceneReference> scene_reference;
    void* scene_raw{};
    NativeString scene_name;
    bool scene_name_live{}, root_linked{};
    NativeString label;
    bool label_live{};
    NativeRenderActualOwnerReleaseObservation scene_release;
    std::uint64_t frames{};
    Phase phase{Phase::prepared};
    std::exception_ptr failure;

    struct RootDispatch final : NativeNodeSceneChildDispatch {
        Impl& app;
        explicit RootDispatch(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            require(profile == 0x00d62de8, "native Text scene requires its actual Model profile");
            return app.nodes->models().vtable_00d62de8;
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            require(target == 0x00b6ed80 && !requested && !recursion,
                "native Text scene requires the genuine null-lighting Model attachment");
            auto* reference = dynamic_cast<NativeModelReference*>(&app.renderer.actual_owners().resolve_actual(actual));
            require(reference && reference == app.widget->model_reference() &&
                &reference->model_owner().environment.nodes == &app.nodes->models().nodes,
                "native Text scene lost its same canonical Model");
            auto& attachment = reference->scene_attachment();
            require(attachment.attach_scene && !attachment.scene,
                "native Text scene cannot substitute a logical lighting owner");
            attachment.attach_scene(app.nodes->models().nodes.scenes, attachment, nullptr, false);
        }
    } roots;

    Impl(GameHostLog& output, GameNativeRendererApplication& app, GameVfsHost& vfs,
        GameSingletonHost& host, GameNativeReadOnlyData& mapped, GameInputRuntime& controls,
        GameFontHost& font_host, GameLocaleHost& locale_host, const bool& conversion)
        : log(output), renderer(app), files(vfs), singletons(host), data(mapped), input(controls),
          fonts(font_host), locale(locale_host), sse2(conversion), raw(vfs.borrow_raw_services()),
          globals(game_native_gui_text_state()),
          materials{app.actual_owners(), raw.strings,
              game_native_material_process().parameter_pool_00f8d3e4(),
              game_native_material_process().material_pool_00f8d3ac()},
          parameters{game_native_material_process().parameter_pool_00f8d3e4(), raw.strings,
              source_ucrt_text_name_comparer(), table(mapped, 0x00d61a00, 0x14)},
          scene_storage{host.weak_owners(), app.camera_environment().nodes.require_raw_name_pool(),
              app.actual_owners(), app.cockpit_viewport_release().decrement_iat_00ce2220,
              app.node_constants().one_00d7a24c, nullptr},
          scene_identity{scene_storage, table(mapped, 0x00d62d48, 8)}, roots(*this) {
        nodes = std::make_unique<GameNativeGuiNodeApplication>(log, renderer, files, data,
            [this](GuiWidgetOwner& owner) {
                require(text && owner.layout().type == GuiWidgetType::Text,
                    "native GUI text bring-up admits only its constructed Text factory");
                return text->make_type(owner);
            });
        style = std::make_unique<GuiMaterialBindingServices>(GuiMaterialBindingServices{
            nodes->widgets(), globals.aspect_ratio_00e12fc0(),
            [](GuiWidgetOwner&) -> GuiClipBoxParameterSources {
                throw std::logic_error("standalone native Text has no ClipBox ancestor");
            },
            [this](GuiWidgetOwner& owner) { return nodes->retain_widget(owner); },
            renderer.actual_owners()});
        clip = std::make_unique<GuiWidgetClipRefreshServices>(GuiWidgetClipRefreshServices{
            nodes->widgets(), materials, parameters, globals.aspect_ratio_00e12fc0()});
        nodes->widgets().environment().clip = clip.get();
    }
    void fail() noexcept { failure = std::current_exception(); phase = Phase::failed; }
    void make_text() {
        samplers = std::make_unique<GameNativeGuiSamplerApplication>(log, renderer, files,
            singletons, data, GameNativeGuiSamplerApplicationBindings{input,
                renderer.borrow_platform_publication_0109cf04()});
        effects = std::make_unique<GameNativeGuiEffectApplication>(log, renderer, files,
            samplers->effect_bindings());
        auto& font_resources = fonts.native_fonts();
        const auto& shadow = globals.shadow_color_00e12fd8();
        const auto& one = cell<float>(data, 0x00d7a24c);
        const auto& shadow_offset = cell<float>(data, 0x00d5c5c0);
        text = std::make_unique<GameNativeTextApplication>(log, renderer,
            GameNativeTextApplicationBindings{nodes->widgets(), nodes->parenting(),
                game_native_gui_text_process().pool_00f8bdf0(), materials, parameters, *style,
                font_resources, GuiTextFontRegistryAccess::borrowed_existing(fonts.registry()),
                locale.tables(), locale_calls, source_ucrt_text_content_comparison(), nullptr,
                source_ucrt_text_name_comparer(), sse2,
                *static_cast<void* const volatile*>(renderer.borrow_platform_publication_0109cf04().identity()),
                renderer.borrow_gui_material_destruction_profile_00d5e520(),
                table(data, 0x00d62d60, 8), raw.strings,
                static_cast<const char*>(data.data_at(0x00cefd78, 1)),
                globals.vertical_scale_00e12fd4(), globals.blend_factor_00f8be54(),
                cell<float>(data, 0x00d7a260),
                {shadow_offset, shadow, cell<float>(data, 0x00ce3e18), one,
                    cell<float>(data, 0x00ce3800)},
                {one, shadow_offset, shadow, cell<float>(data, 0x00cee07c),
                    globals.default_color_mask_00f8be48(), globals.default_color_00f8be38()},
                {cell<double>(data, 0x00d7a280), cell<double>(data, 0x00cec380),
                    cell<double>(data, 0x00cef1b8), cell<double>(data, 0x00d5c5c8)},
                {cell<double>(data, 0x00d7a358), cell<double>(data, 0x00d5c7a8),
                    cell<double>(data, 0x00d7a220), cell<double>(data, 0x00d06880),
                    cell<double>(data, 0x00cec380)},
                game_native_gui_text_process().text_lineage_00f8be28()}, *effects);
        fonts.bind_text_factory(text->factory());
        require(!fonts.registry().fonts().empty(), "native Text requires a loaded Font");
        const auto& font = *fonts.registry().fonts().front();
        require(font.native_resources && font.native_resources->native_image(),
            "native Text requires the selected image-shaped Font producer");
        properties = std::make_shared<GuiTable>();
        properties->named = {{"Font", GuiValue(font.descriptor.name)},
            {"Multiline", GuiValue(false)}, {"Visible", GuiValue(true)},
            {"Pos", GuiValue(vector_table({0.05, 0.10, 0.0}))},
            {"Size", GuiValue(vector_table({0.9, 0.08}))},
            {"Color", GuiValue(vector_table({1.0, 1.0, 1.0, 1.0}))}};
        auto layout = std::make_unique<GuiLayoutWidget>();
        layout->key = "NativeGui_Text";
        layout->type = GuiWidgetType::Text;
        layout->source = properties.get();
        layout->transform.authored_x = 0.05f;
        layout->visible = true;
        widget = &nodes->create_standalone_text(std::move(layout));
        auto* platform = *static_cast<void* const volatile*>(
            renderer.borrow_platform_publication_0109cf04().identity());
        require(platform, "native Text requires the existing platform publication");
        bind_widget_properties_00aaa710(*properties, widget->layout(), false,
            read<std::uint8_t>(platform, 0x0d) != 0, sse2);
        nodes->widgets().base_properties_bound(widget->layout());
        nodes->widgets().properties_bound(widget->layout(), *properties);
        nodes->widgets().loaded78(widget->layout());
        require(text->submit_source(*widget, "Battlestations Pacific - native GuiText", false) ==
            GameNativeTextStatus::complete, "native Text bring-up retained an incomplete glyph child");
    }
    static void retire_camera(void* context, NativeCameraReference& reference) noexcept {
        auto& app = *static_cast<Impl*>(context);
        app.renderer.actual_owners().unbind(app.camera_raw, reference);
        app.camera_raw = nullptr;
        app.camera_retired = true;
    }
    void make_camera() {
        auto& environment = renderer.camera_environment();
        camera_name_live = true;
        camera_name.assign_0041e870(raw.strings, "GuiCam_NativeText");
        camera_raw = environment.pool_0108ffb0.allocate_raw_slot_00b71770();
        require(camera_raw, "native GUI camera pool returned null");
        auto admission = renderer.actual_owners().prepare_binding(camera_raw);
        auto viewport = renderer.viewport_registry().admit(viewport_record);
        camera = std::make_unique<NativeCameraOwner>(camera_raw, NativeCameraPool::slot_bytes,
            environment);
        construct_native_camera_00b71a80(*camera, &camera_name, renderer.node_constants(),
            std::move(viewport));
        camera_reference = std::make_unique<NativeCameraReference>(*camera,
            NativeCameraCompanionDisposal{this, &retire_camera});
        renderer.actual_owners().publish(std::move(admission), *camera_reference);
        camera_name.release_to(raw.strings);
        camera_name_live = false;
        // The recovered GUI scene acquisition disables distance/height fog.
        auto* fog = allocate_system_fog_owner();
        set_system_fog_scalar_68_00b84d00(*fog, 0);
        set_system_fog_scalar_78_00b84d40(*fog, 0);
        set_system_fog_camera_owner_00b71940(camera->frame.fog_184, fog);
        release_system_fog_owner(*fog);
        set_camera_clear_flags_00b6fe10(camera->frame, 6);
        camera->storage.camera.clear_color_190 = 0;
    }
    void make_scene() {
        scene_raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x24, 0x24});
        require(scene_raw, "native GUI scene allocation returned null");
        scene_name_live = true;
        scene_name.assign_0041e870(raw.strings, "NativeGuiText");
        scene_frame.name_argument = static_cast<Word>(reinterpret_cast<std::uintptr_t>(&scene_name));
        construct_native_gui_scene_storage_00b724e0(scene_raw, scene_frame,
            scene_storage, scene_construction);
        scene_reference.emplace(scene_raw, scene_identity, scene_destroy_frame, scene_destruction);
        scene_name.release_to(raw.strings);
        scene_name_live = false;
        // The Text owns its primary Model. B6D890 adds no reference, so unlink
        // it before Text scalar deletion and before the outer scene's terminal.
        propagate_native_node_root_storage_00b6d890(
            &widget->model_reference()->model_owner().storage.node, scene_raw, roots);
        root_linked = true;
    }

    struct Pass final : GuiRenderOrderHost, GuiOrthoProjectionHost {
        Impl& app;
        GameNativeGuiQueue& commands;
        CameraMatrix projection;
        std::array<Word, 2> dimensions{};
        Pass(Impl& value, GameNativeGuiQueue& queue) : app(value), commands(queue) {}
        bool has_pre_draw_object() override { return false; }
        void pre_draw_virtual40() override { throw std::logic_error("standalone GUI has no pre-draw object"); }
        std::array<Word, 2> device_size_00f8d394_30() override {
            auto& environment = app.renderer.camera_environment().viewport;
            require(environment.renderer_00f8d394, "native GUI pass lost renderer");
            const auto size = environment.renderer_access.parameters_00b1ff60(*environment.renderer_00f8d394);
            return {size.width_0c, size.height_10};
        }
        GuiBackBufferSize back_buffer_size_00f8d394_virtual80() override {
            const auto size = device_size_00f8d394_30();
            return {static_cast<std::int32_t>(size[0]), static_cast<std::int32_t>(size[1])};
        }
        void set_camera_look_at_00b71490(const void* camera, const GuiCameraPlacement& p) override {
            require(camera == app.camera_raw, "native GUI pass requires its same camera");
            CameraMatrix view;
            build_camera_look_at_00b63f10(view, {p.eye[0], p.eye[1], p.eye[2]},
                {p.target[0], p.target[1], p.target[2]}, {p.up[0], p.up[1], p.up[2]},
                app.camera->environment.crt, app.renderer.node_constants().one_00d7a24c);
            set_native_camera_view_00b71490(*app.camera, view.data());
        }
        void set_camera_ortho_00aa2020(const void* camera, const GuiOrthoVolume& volume) override {
            set_gui_ortho_projection_00aa2020(camera, volume, *this);
        }
        const void* platform() const {
            auto* value = *static_cast<void* const volatile*>(
                app.renderer.borrow_platform_publication_0109cf04().identity());
            require(value, "native GUI pass lost platform"); return value;
        }
        bool platform_widescreen_0d() override { return read<std::uint8_t>(platform(), 0x0d) != 0; }
        float platform_aspect_10() override { return read<float>(platform(), 0x10); }
        void* ortho_off_center_00b62a10(float l, float r, float b, float t, float n, float f) override {
            cc13fx::ortho_off_center_lh(projection.data(), l, r, b, t, n, f);
            return projection.data();
        }
        void set_projection_00b6fd60(const void* camera, void* matrix) override {
            require(camera == app.camera_raw && matrix == projection.data(), "native GUI projection identity changed");
            set_camera_projection_00b6fd60(app.camera->projection, projection);
        }
        void* camera_viewport_00b6fde0(const void* camera, const std::array<Word, 2>& size) override {
            require(camera == app.camera_raw && app.camera->storage.camera.viewport_180,
                "native GUI camera has no constructed viewport");
            dimensions = size; return app.camera->storage.camera.viewport_180;
        }
        void viewport_set_dimensions_00b1f940(void* viewport) override {
            require(viewport == app.camera->storage.camera.viewport_180, "native GUI viewport changed");
            set_native_viewport_dimensions_00b1f940(*static_cast<NativeViewportOwner*>(viewport), dimensions.data());
        }
        void set_pass_label_0041e870(std::string_view value) override {
            require(!app.label_live && value == "X", "native GUI pass label cannot overlap");
            app.label_live = true; app.label.assign_0041e870(app.raw.strings, "X");
        }
        void* render_command_queue_004c11f0() override { return commands.queue_004c11f0(); }
        void* render_pass_context_00b0d0d0() override { return commands.pass_context_00b0d0d0(); }
        void submit_scene_00b1f4d0(void* queue, const void* scene, const void* camera, void* context) override {
            require(app.label_live, "native GUI submission lost its actual label");
            // The recovered queue entry consumes the header even on unwind.
            app.label_live = false;
            commands.submit_scene_00b1f4d0(queue, const_cast<void*>(scene),
                const_cast<void*>(camera), context, &app.label);
        }
    };
};

bool GameNativeGuiRenderApplication::text_requested() noexcept {
    static const bool requested = [] {
        char value[2]{};
        return GetEnvironmentVariableA("BSP_NATIVE_GUI_TEXT", value, sizeof value) == 1 && value[0] == '1';
    }();
    return requested;
}
GameNativeGuiRenderApplication::GameNativeGuiRenderApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameInputRuntime& input, GameFontHost& fonts,
    GameLocaleHost& locale, const bool& sse2)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, input, fonts, locale, sse2)) {}
GameNativeGuiRenderApplication::~GameNativeGuiRenderApplication() noexcept {
    if (impl_->phase != Impl::Phase::prepared && impl_->phase != Impl::Phase::closed) std::terminate();
}
void GameNativeGuiRenderApplication::prepare_text_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.renderer.shader_cache_context().actual_cache_0108d6ec,
        "native GUI Text preparation requires the mission's same open shader cache");
    p.phase = Impl::Phase::preparing;
    try {
        p.make_text(); p.make_camera(); p.make_scene(); p.phase = Impl::Phase::ready;
        p.log.notef("native GUI Text ready: text=%p model=%p scene=%p camera=%p glyph_buffers=HAL font=image",
            &p.widget->layout(), &p.widget->model_reference()->model_owner().storage.node, p.scene_raw, p.camera_raw);
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::submit_mission_frame(GameNativeGuiQueue& queue) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready, "native GUI frame requires ready Text owners");
    p.phase = Impl::Phase::submitting;
    try {
        Impl::Pass pass(p, queue);
        submit_gui_pass_00aa3e00(p.scene_raw, p.camera_raw, pass);
        ++p.frames; p.phase = Impl::Phase::ready;
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::retire_before_fonts() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired || p.phase == Impl::Phase::closed) return;
    if (p.phase == Impl::Phase::prepared) { p.phase = Impl::Phase::retired; return; }
    require(p.phase == Impl::Phase::ready, "native GUI retirement cannot replay interrupted work");
    p.phase = Impl::Phase::retiring;
    try {
        if (p.root_linked) {
            propagate_native_node_root_storage_00b6d890(
                &p.widget->model_reference()->model_owner().storage.node, nullptr, p.roots);
            p.root_linked = false;
        }
        p.text->retire_owned_text(p.nodes->take_standalone_for_retirement(*p.widget));
        p.widget = nullptr;
        release_native_render_actual_owner(p.renderer.actual_owners(), p.camera_raw);
        require(p.camera_retired, "native GUI camera remains referenced after command retirement");
        p.renderer.viewport_registry().forget_quiescent(p.viewport_record);
        release_native_render_actual_owner_observed(p.renderer.actual_owners(), p.scene_raw, p.scene_release);
        p.phase = Impl::Phase::retired;
    } catch (...) { p.fail(); throw; }
}
void GameNativeGuiRenderApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::retired, "native GUI graph must retire before shared drain");
    if (p.effects) p.effects->after_native_renderer_drain();
    if (p.samplers) p.samplers->after_native_renderer_drain();
    if (p.scene_reference)
        require(p.scene_reference->retired() && p.scene_destruction.complete && p.scene_destruction.freed,
            "native GUI scene did not complete its actual terminal");
    p.effects.reset(); p.text.reset(); p.samplers.reset();
    p.scene_reference.reset(); p.camera_reference.reset(); p.camera.reset();
    p.clip.reset(); p.style.reset(); p.nodes.reset(); p.properties.reset();
    p.phase = Impl::Phase::closed;
}
bool GameNativeGuiRenderApplication::ready() const noexcept { return impl_->phase == Impl::Phase::ready; }
bool GameNativeGuiRenderApplication::requires_process_retention() const noexcept {
    const auto& p = *impl_;
    return p.failure || p.phase == Impl::Phase::preparing || p.phase == Impl::Phase::submitting ||
        p.phase == Impl::Phase::retiring || (p.nodes && p.nodes->failure()) ||
        (p.text && p.text->failure()) || (p.effects && p.effects->failure());
}
std::uint64_t GameNativeGuiRenderApplication::submitted_frames() const noexcept { return impl_->frames; }
} // namespace bsp::game
