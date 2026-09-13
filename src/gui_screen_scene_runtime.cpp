#include "bsp/gui_screen_scene_runtime.hpp"
#include "bsp/camera_configuration.hpp"
#include "bsp/lighting_configuration_apply.hpp"
#include "bsp/system_fog_owner.hpp"
#include <cstring>
#include <exception>
#include <stdexcept>
#include <unordered_map>

namespace bsp {
namespace {
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
// Native strings own their actual sized-pool buffers, including temporaries.
class ScreenString final {
public:
    ScreenString(SizedStoragePool& pool, std::string_view text) : storage_(pool) {
        const std::string terminated(text);
        value.assign_0041e870(storage_, terminated.c_str());
    }
    ~ScreenString() { destroy_native_string_header_0041dd20(&value, storage_); }
    NativeString value;
private:
    PooledStringStorage storage_;
};
LightingDirectionalFields fields(DirectionalLightOwner& owner) noexcept {
    auto& tail = owner.light;
    return {tail.diffuse_184, tail.specular_194, tail.base_diffuse_1a4,
        tail.diffuse_mode3_1b4, tail.base_specular_1c4, tail.diffuse_scale_1d8,
        tail.specular_scale_1dc, tail.direction_1e0};
}
void release_lights(ConcreteSystemSceneResource& lights) noexcept {
    if (lights.references.fetch_sub(1) == 1) lights.destroy_on_zero(lights);
}
}

struct GuiScreenSceneRuntime::Shared {
    struct StoreOwners { NativeCameraReference* camera; NativeGuiSceneOwner* scene; };
    // Association bookkeeping only. Values borrow already-existing companions;
    // fields, visible counts and render order are never synchronized here.
    std::unordered_map<GuiCameraStore*, StoreOwners> owners;
    GuiScreenSceneEnvironment environment;
    explicit Shared(GuiScreenSceneEnvironment supplied) : environment(supplied) {
        require(&environment.scenes.nodes == &environment.cameras.nodes,
            "GUI scene and camera must use the same native node runtime");
        environment.scenes.nodes.require_semantic_name_pool();
        const auto* table = environment.directional_vtable_00d62fb0;
        require(table && table[0] == 0x00bd30e0u && table[1] == 0x00b7c820u &&
            table[0x18 / 4] == 0x00b6f310u && table[0x34 / 4] == 0x00b6e870u &&
            table[0x40 / 4] == 0x00b6dbe0u && table[0x54 / 4] == 0x00b7bd60u,
            "GUI light requires the actual D62FB0 lifetime and transform profile");
    }
    StoreOwners& resolve(GuiCameraStore* store) {
        const auto found = owners.find(store);
        require(found != owners.end(), "GUI store has no canonical owner association");
        require_gui_store_scene(*store, *found->second.scene);
        require_gui_store_camera(*store, *found->second.camera);
        return found->second;
    }
    void bind(GuiCameraStore& store, NativeCameraReference& camera, NativeGuiSceneOwner& scene) {
        require_gui_store_scene(store, scene);
        require_gui_store_camera(store, camera);
        bool present = false;
        for (const auto& entry : environment.stores.entries())
            if (entry.second == &store) present = true;
        require(present, "GUI owner association requires the supplied canonical map entry");
        const auto result = owners.emplace(&store, StoreOwners{&camera, &scene});
        require(result.second || (result.first->second.camera == &camera &&
            result.first->second.scene == &scene), "GUI store owner association changed identity");
    }
    struct Layer {
        std::shared_ptr<Shared> runtime;
        GuiWidgetOwner& widget;
        NativeGuiSceneOwner* scene{}; // companion for the actual scene_ec only
        bool acquisition_started{}; // distinguishes constructor-unwritten F4
        bool store_created{}; // host failure recovery, not a native field/ref
        bool retired{};
        Layer(std::shared_ptr<Shared> shared, GuiWidgetOwner& owner)
            : runtime(std::move(shared)), widget(owner) {}

        void acquire(GuiWidgetOwner& owner, GuiScreenLayerState& state) {
            require(&owner == &widget && !retired, "GUI acquire requires its live original widget");
            auto& shared = *runtime;
            auto& env = shared.environment;
            state.store_f0 = env.stores.find_00aa3280(state.key_108);
            acquisition_started = true;
            store_created = false;
            if (state.share_scene_120 && state.store_f0) {
                // Native publishes F4/EC, increments actual scene+04, RETURNS.
                auto& found = shared.resolve(state.store_f0);
                state.owns_store_f4 = false;
                state.scene_ec = state.store_f0->scene;
                scene = found.scene;
                retain_render_command_reference(*scene);
                return;
            }
            state.owns_store_f4 = true;
            {
                ScreenString name(env.scenes.nodes.require_semantic_name_pool(), state.name_100);
                scene = allocate_native_gui_scene_00b724e0(env.scenes, name.value);
                state.scene_ec = &scene->storage;
            }
            NativeCameraReference* camera;
            {
                ScreenString name(env.scenes.nodes.require_semantic_name_pool(), "GuiCam_" + state.name_100);
                camera = allocate_native_gui_camera(env.cameras, name.value);
            }
            state.store_f0 = create_gui_camera_store_00aa5070(env.stores, *camera, *scene, state.key_108);
            store_created = true;
            shared.bind(*state.store_f0, *camera, *scene);
            ConcreteSystemSceneResource* lights;
            {
                ScreenString name(env.scenes.nodes.require_semantic_name_pool(), "GuiLights");
                lights = allocate_system_scene_resource_00b83c50(env.scenes.nodes.require_semantic_name_pool(),
                    name.value, env.directional_lights);
            }
            set_native_gui_scene_lighting_00b723f0(*scene, lights);
            release_lights(*lights); // native drops the allocation's initial ref
            attach_native_gui_scene_node_00b6d890(*scene, *camera);
            auto& camera_owner = camera->camera_owner();
            set_camera_clear_flags_00b6fe10(camera_owner.frame, 7);
            camera_owner.storage.camera.clear_color_190 = 0; // B6FE50
            auto* viewport = allocate_native_viewport_owner(env.cameras.viewport);
            set_native_camera_viewport_00b71990(camera_owner, viewport);
            release_native_viewport_owner(*viewport);
            auto* fog = allocate_system_fog_owner();
            set_system_fog_scalar_68_00b84d00(*fog, 0);
            set_system_fog_scalar_78_00b84d40(*fog, 0);
            set_system_fog_camera_owner_00b71940(SystemFogSlotRef(camera_owner.storage.camera.fog_184), fog);
            release_system_fog_owner(*fog);
            GuiScreenDirectionalAllocation directional = [&] {
                ScreenString name(env.scenes.nodes.require_semantic_name_pool(), "GuiDirectionalLight");
                return env.native.allocate_directional_light_00b7c6b0(name.value);
            }();
            auto& light = directional.owner;
            auto* reference = dynamic_cast<RenderCommandReference*>(&directional.lifetime);
            require(&light.runtime == &env.scenes.nodes &&
                &directional.lifetime.transform() == &light.node.transform &&
                &directional.lifetime.scene_attachment() == &light.node.scene_attachment &&
                &env.scenes.nodes.attachments.resolve(light.node.transform) == &directional.lifetime &&
                reference && &reference->reference_count == &light.node.storage.references_04 &&
                light.node.storage.vtable_00 == 0x00d62fb0u,
                "GUI directional allocation must bind the same actual light and native count");
            CameraMatrix identity{};
            const auto one = env.one_00d7a24c;
            for (std::size_t i = 0; i != 4; ++i)
                std::memcpy(&identity[i * 4 + i], &one, sizeof(one));
            // Actual current+34 B6E870, with current+40 B6DBE0.
            set_transform_world_matrix_00b6e870(light.node.transform, identity,
                notify_camera_world_changed_00b6dbe0);
            light.light.direction_1e0 = {0, 0, env.one_00d7a24c};
            set_lighting_base_diffuse_004b62e0(fields(light), {0, 0, 0, env.one_00d7a24c});
            attach_native_gui_scene_node_00b6d890(*scene, directional.lifetime);
            // Native retains the GuiLights local and reloads its actual +10.
            require(lights->environment_10 != nullptr, "GuiLights has no actual ambient owner");
            const auto half = env.half_00ce3800;
            set_lighting_ambient_00b7af20(lighting_ambient_fields(*lights->environment_10),
                {half, half, half, env.one_00d7a24c});
            // Reload store+18 separately for both calls, as AC5EA0/AC5EC6.
            set_camera_clear_flags_00b6fe10(shared.resolve(state.store_f0).camera->camera_owner().frame,
                static_cast<std::uint32_t>(state.key_108.flags) | 6u);
            shared.resolve(state.store_f0).camera->camera_owner().storage.camera.clear_color_190 = 0;
            if (state.applied_priority_fc != state.key_108.priority) {
                state.applied_priority_fc = state.key_108.priority;
                env.native.register_page_00aa52a0(owner, state);
            }
            if (!owner.layout().transform.bounds_enabled) {
                const auto center = env.half_00ce3800;
                const auto radius = env.native.root_radius_00ac5f00(env.squared_radius_00d7a308);
                std::uint32_t radius_bits;
                std::memcpy(&radius_bits, &radius, sizeof(radius_bits));
                auto* root = owner.node_binding(); // native loads +4C after CRT calls
                require(root != nullptr, "GUI screen bounds require its actual root node");
                env.native.set_root_bounds_00b8e6c0(*root, {center, center, 0, radius_bits});
            }
        }
        void bind_node(NativeNodeBinding& node, const void* requested_scene) {
            auto& nodes = runtime->environment.scenes.nodes;
            if (!requested_scene) {
                propagate_native_node_root_00b6d890(nodes, node.transform, nullptr);
                return;
            }
            require(scene && requested_scene == &scene->storage,
                "GUI binding requires this layer's actual acquired scene");
            attach_native_gui_scene_node_00b6d890(*scene, nodes.attachments.resolve(node.transform));
        }
        void release(GuiWidgetOwner& owner, GuiScreenLayerState& state) noexcept {
            if (&owner != &widget || retired) std::terminate();
            // AA31F0 performs the tree's current+20 before deleting the layer.
            // This is solely derived AC5480; it neither walks nor retains nodes.
            if (acquisition_started) {
                if (state.owns_store_f4 && store_created) {
                    // A throwing host construction may leave F0 as the find
                    // result. Only this layer's published allocation is owned.
                    // Successful native AC5480 always takes this branch.
                    runtime->owners.erase(state.store_f0);
                    remove_gui_camera_store_00aa4b30(runtime->environment.stores, state.store_f0);
                }
                if (state.scene_ec) {
                    if (!scene || state.scene_ec != &scene->storage) std::terminate();
                    release_native_gui_scene_00b72250(*scene);
                    state.scene_ec = nullptr; // native clear AFTER terminal callback
                }
            }
            scene = nullptr;
            // std::string is this semantic state's name owner. Native returns
            // its buffer before AA9730; no second NativeString field is kept.
            std::string().swap(state.name_100);
            retired = true;
        }
    };
};
GuiScreenSceneRuntime::GuiScreenSceneRuntime(GuiScreenSceneEnvironment environment)
    : shared_(std::make_shared<Shared>(environment)) {}
GuiScreenLayerServices GuiScreenSceneRuntime::make_services(GuiWidgetOwner& owner) {
    auto layer = std::make_shared<Shared::Layer>(shared_, owner);
    return {
        [layer](GuiWidgetOwner& widget, GuiScreenLayerState& state) { layer->acquire(widget, state); },
        [layer](NativeNodeBinding& node, const void* scene) { layer->bind_node(node, scene); },
        [layer] { return layer->runtime->environment.native.visibility_hook_installed_00f8bf4c(); },
        [layer](std::string_view name, bool visible) {
            layer->runtime->environment.native.visibility_hook_00f8bf4c(name, visible);
        },
        [layer](GuiWidgetOwner& widget, GuiScreenLayerState& state) noexcept { layer->release(widget, state); }
    };
}
void GuiScreenSceneRuntime::bind_existing_store(GuiCameraStore& store,
    NativeCameraReference& camera, NativeGuiSceneOwner& scene) {
    shared_->bind(store, camera, scene);
}
NativeCameraReference& GuiScreenSceneRuntime::camera_for_store(GuiCameraStore& store) {
    return *shared_->resolve(&store).camera;
}
NativeGuiSceneOwner& GuiScreenSceneRuntime::scene_for_store(GuiCameraStore& store) {
    return *shared_->resolve(&store).scene;
}
} // namespace bsp
