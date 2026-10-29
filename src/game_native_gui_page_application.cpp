#include "bsp/game_native_gui_page_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/game_native_dyn_process.hpp"
#include "bsp/game_native_entity_registry_process.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_group_owner.hpp"
#include "bsp/native_group_pool.hpp"
#include "bsp/native_group_storage_lifetime.hpp"
#include "bsp/native_gui_layer_storage.hpp"
#include "bsp/native_gui_page_construction.hpp"
#include "bsp/native_gui_page_properties_continuation.hpp"
#include "bsp/native_gui_page_properties.hpp"
#include "bsp/native_gui_layer_lifetime.hpp"
#include "bsp/native_gui_media_focus_lifetime.hpp"
#include "bsp/native_gui_widget_lifetime.hpp"
#include "bsp/native_gui_widget_position.hpp"
#include "bsp/native_gui_widget_property_interaction.hpp"
#include "bsp/native_gui_widget_visibility.hpp"
#include "bsp/native_lua_reader_fields.hpp"
#include "bsp/native_lua_reader_queries.hpp"
#include "bsp/native_node_raw_transform.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include "bsp/native_window_focus_dispatch.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/native_gui_page_reader_continuation.hpp"
#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_gui_page_script_continuation.hpp"
#include "bsp/native_gui_script_paths.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/object_handle_resolvers.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_platform_load_messages.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <cstddef>
#include <array>
#include <cstdio>
#include <cstring>
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>

namespace bsp::game {
namespace {
using Word = std::uint32_t;
constexpr Word kPageProfile = 0x00d5be38;   // AC6600's page vtable
constexpr Word kReaderProfile = 0x00ce44fc; // 4425C0's Lua reader vtable
constexpr Word kGroupProfile = 0x00d634f8;  // B8F5E0's Group vtable

void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
const volatile Word* table(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const volatile Word*>(data.data_at(address, bytes));
}
const volatile Word& word(GameNativeReadOnlyData& data, Word address) {
    return *table(data, address, 4);
}
const char* literal(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const char*>(data.data_at(address, bytes));
}
template<class T> const volatile T& scalar(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
void* at(void* p, Word offset = 0) noexcept { return static_cast<unsigned char*>(p) + offset; }
Word read(const void* p, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(static_cast<const unsigned char*>(p) + offset);
}
void write(void* p, Word offset, Word value) noexcept { *static_cast<volatile Word*>(at(p, offset)) = value; }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
[[noreturn]] void reject(const char* slot, Word target) {
    char message[160];
    std::snprintf(message, sizeof message, "childless GUI Page does not admit %s target/type %08x", slot, target);
    throw std::logic_error(message);
}
} // namespace

struct GameNativeGuiPageApplication::Impl {
    // AA3140's CURRENT Page table. Every registered page comes from AC6600, so
    // only D5BE38 is admitted; its +7C is AC3FA0 (the page name at +100).
    struct Lookup final : NativeGuiPageLookupBindings {
        GameNativeReadOnlyData& data;
        explicit Lookup(GameNativeReadOnlyData& value) : data(value) {}
        const volatile Word* current_page_table(void*, Word profile) override {
            require(profile == kPageProfile, "native GUI page lookup admits only the AC6600 page profile");
            return table(data, profile, 0x80);
        }
        const void* call_page_name_7c(void*, Word) override {
            throw std::logic_error("native GUI page lookup has no Page+7C other than AC3FA0");
        }
    };
    // AC6806's CURRENT Page+18 and reader+08. The admitted Page+18 path is
    // the exact childless AC4C50 composition, not a generic child factory.
    struct Properties final : NativeGuiPagePropertiesBindings {
        Impl& app;
        explicit Properties(Impl& value) : app(value) {}
        const volatile Word* current_page_table(void*, Word profile) override {
            require(profile == kPageProfile, "native GUI page properties admit only the AC6600 page profile");
            return table(app.data, profile, 0x80);
        }
        const volatile Word* current_reader_table(NativeLuaReaderStorage&, Word profile) override {
            require(profile == kReaderProfile, "native GUI page properties admit only the CE44FC reader");
            return table(app.data, profile, 0x1c);
        }
        void call_page_properties_18(void* page, Word target, NativeLuaReaderStorage& reader) override {
            if (target != 0x00ac4c50) reject("Page+18", target);
            app.read_properties(page, reader);
        }
    };
    // The raw Group's distinct current18 is B8EEC0. Its canonical zero terminal
    // may still hold an actual3Ch scene resource; its genuine raw destructor
    // closes that field. Never route it through a legacy SceneResource* owner.
    struct GroupTrees final : NativeNodeTreeRetirementDispatch {
        Impl& app;
        explicit GroupTrees(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            if (profile == kGroupProfile) return table(app.data, profile, 0x58);
            return app.scene_domain.node.trees.dispatch.resolve_profile(profile);
        }
        void invoke_virtual18(Word target, void* actual) override {
            for (auto& page : app.pages) {
                if (page->group_slot == actual && page->reference && !page->reference->retired()) {
                    require(target == 0x00b8eec0 &&
                        app.renderer.actual_owners().find(actual) == page->reference.get(),
                        "raw GUI Group release requires its genuine current18 and same canonical owner");
                    release_native_group_storage_00b8eec0(actual, page->group_release_frame,
                        app.group_trees, page->group_release);
                    return;
                }
                if (!page->properties) continue;
                auto& scene = page->properties->acquired.scene;
                auto& frames = page->properties->scenes;
                if (scene.camera_result == actual && scene.camera_reference && !scene.camera_reference->retired()) {
                    require(target == 0x00b6f310 &&
                        app.renderer.actual_owners().find(actual) == &*scene.camera_reference,
                        "GUI camera release requires its genuine current18 and same canonical owner");
                    release_native_node_tree_00b6f310(actual, frames.camera_release_frame,
                        app.group_trees, frames.camera_release);
                    return;
                }
                if (scene.directional_result == actual && scene.directional_reference &&
                    !scene.directional_reference->retired()) {
                    require(target == 0x00b6f310 &&
                        app.renderer.actual_owners().find(actual) == &*scene.directional_reference,
                        "GUI sun release requires its genuine current18 and same canonical owner");
                    release_native_node_tree_00b6f310(actual, frames.directional_release_frame,
                        app.group_trees, frames.directional_release);
                    return;
                }
            }
            reject("Page tree current18 outside its admitted live families", target);
        }
        void invoke_virtual0(Word target, void* actual, RenderCommandReference& reference) override {
            auto release = [&](auto* expected) {
                require(target == 0x00bd30e0 && expected == &reference &&
                    static_cast<void*>(&reference.reference_count) == static_cast<unsigned char*>(actual) + 4 &&
                    reference.reference_count.load(std::memory_order_relaxed) == 0 &&
                    app.renderer.actual_owners().find(actual) == &reference,
                    "Page tree terminal requires its same current zero-count canonical family owner");
                // The family's genuine CURRENT04 removes its actual scene.
                // In particular, do not preclear170 or impose A1's null guard.
                reference.release_zero_references();
            };
            for (auto& page : app.pages) {
                if (page->group_slot == actual && page->reference && !page->reference->retired()) {
                    release(page->reference.get()); return;
                }
                if (!page->properties) continue;
                auto& scene = page->properties->acquired.scene;
                if (scene.camera_result == actual && scene.camera_reference && !scene.camera_reference->retired()) {
                    release(&*scene.camera_reference); return;
                }
                if (scene.directional_result == actual && scene.directional_reference &&
                    !scene.directional_reference->retired()) {
                    release(&*scene.directional_reference); return;
                }
            }
            // In particular, retained130 may be a NON-node family. Never apply
            // A1's node170-null admission to an unknown retained owner.
            reject("raw Group current0 outside its admitted family", target);
        }
    };
    // B6D890 from AC4C50 and AC6806 on the SAME raw page-root Group. Each
    // normal page construction has two distinct prepared attachment frames.
    struct Roots final : NativeNodeSceneChildDispatch {
        Impl& app;
        explicit Roots(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            require(profile == kGroupProfile, "native GUI page root admits only its raw Group profile");
            return app.group_identity.actual_profile_00d634f8;
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            require(target == 0x00b6ed80 && !recursion,
                "native GUI page root requires its genuine B6ED80 attachment with recurse0");
            for (auto& page : app.pages) {
                if (!page->reference || page->group_slot != actual || page->reference->retired()) continue;
                require(app.renderer.actual_owners().find(actual) == page->reference.get(),
                    "native GUI page root lost its same canonical raw Group");
                require(page->attachment_count < page->attachments.size(),
                    "raw GUI page root exceeded its two prepared constructor attachments");
                auto& invocation = page->attachments[page->attachment_count++];
                invocation.frame.scene_argument = requested;
                invocation.frame.recursion_argument = recursion;
                set_native_node_scene_storage_00b6ed80(actual, invocation.frame,
                    app.group_scenes, invocation.acquired);
                return;
            }
            throw std::logic_error("native GUI page root has no child below its Group before P3b/P3c");
        }
    };
    struct SceneFrames {
        // Explicit initialized application preimages. These disjoint backing
        // arrays are Source caller storage, not recovered private stack bytes.
        std::array<volatile Word, 32> locals{};
        std::array<volatile Word, 3> arguments{};
        NativeGuiCameraStoreFindScratch find{};
        NativeGuiCameraStoreCreateFrame create{};
        NativeGuiSceneConstructFrame outer_construct{};
        std::array<NativeNodeTreeRetirementAcquired, 256> root_calls{};
        NativeGuiSceneDestroyFrame outer_destroy{root_calls.data(), 256, 0};
        std::array<volatile Word, 6> camera_locals{};
        std::array<volatile Word, 2> camera_arguments{};
        std::array<volatile Word, 39> pose_locals{};
        std::array<volatile Word, 13> look_at_locals{};
        std::array<volatile Word, 4> look_at_arguments{};
        NativeNodeBaseDestructionFrame camera_cleanup{};
        NativeCameraStorageConstructFrame camera_construct{camera_locals.data(), camera_arguments.data(),
            {pose_locals.data(), {look_at_locals.data(), look_at_arguments.data()},
                &look_at_arguments[3], &look_at_arguments[3]}, camera_cleanup};
        NativeCameraStorageDestroyFrame camera_destroy{};
        NativeNodeTreeRetirementFrame camera_release_frame{};
        NativeNodeTreeRetirementAcquired camera_release;
        NativeSceneResourceConstructFrame resource_construct{};
        NativeLightLifetimeFrame directional_destroy{};
        NativeNodeTreeRetirementFrame directional_release_frame{};
        NativeNodeTreeRetirementAcquired directional_release;
        std::array<volatile Word, 32> directional_world{};
        NativeGuiSceneAcquisitionFrame frame{locals.data(), arguments.data(), find, create,
            outer_construct, outer_destroy, camera_construct, camera_destroy, resource_construct,
            directional_destroy, {directional_world.data()}};
    };
    struct PropertyInvocation {
        SceneFrames scenes;
        alignas(4) std::byte locals[0x30]{};
        NativeGuiWidgetPropertyScratch widget{};
        NativeGuiWidgetTransformScratch transform{};
        GuiWidgetBounds bounds;
        NativeLuaReaderFieldScratch field{};
        NativeLuaFieldResolverFrame resolver;
        NativeLuaReaderQueryScratch keys{};
        NativeGuiPageReadPropertiesAcquired acquired;
        NativeLuaReaderStorage* reader{}; // borrowed only through the synchronous call
        Word rejected_widget_type{};
    };
    struct RootAttachment {
        NativeNodeSceneAttachmentFrame frame{};
        NativeNodeSceneAttachmentAcquired acquired{};
    };
    // The page's raw Group is owned through page+4C. All raw lifetime and
    // attachment backing survives until its native owner has actually retired.
    struct Page {
        Impl* app{};
        void* group_slot{};
        NativeGroupStorageLifetimeFrame group_lifetime_frame{};
        NativeGroupStorageLifetimeAcquired group_lifetime;
        std::unique_ptr<PropertyInvocation> properties;
        std::unique_ptr<NativeGroupStorageReference> reference;
        NativeNodeTreeRetirementFrame group_release_frame{};
        NativeGroupStorageReleaseAcquired group_release;
        NativeNodeTreeRetirementAcquired group_unlink;
        std::array<RootAttachment, 2> attachments{};
        std::size_t attachment_count{};
        void* page{};
        bool scene_released{}, native_retired{};
    };
    struct Construction {
        NativeString suffix, path;
        bool suffix_live{}, path_live{};
        NativeVfsNameResolutionAcquired resolution;
        NativeGuiPageConstructionAcquired prefix;
        NativeGuiPageScriptContinuation scripts;
        NativeGuiPageReaderContinuation reader;
        NativeGuiPagePropertiesContinuation properties;
    };

    struct SceneTypes final : NativeSceneRegistrationDerivedTypes {
        Impl& app;
        explicit SceneTypes(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            if (profile != kGroupProfile) reject("scene type profile", profile);
            return app.group_identity.actual_profile_00d634f8;
        }
        std::uint8_t invoke_type_0c(Word target, void* actual, Word token) override {
            if (target != 0x00b8f650) reject("scene type+0C", target);
            bool found = false;
            for (auto& page : app.pages)
                if (page->group_slot == actual && page->reference && !page->reference->retired() &&
                    app.renderer.actual_owners().find(actual) == page->reference.get()) found = true;
            require(found, "scene Group predicate requires its same live canonical raw root");
            return static_cast<std::uint8_t>(app.group_types.is_type_00b8f650(token));
        }
    };
    struct Fields final : NativeGuiWidgetPropertyDispatch {
        Impl& app;
        explicit Fields(Impl& value) : app(value) {}
        void call_virtual0c(Word target, void* visitor, void* key, void* field, void* fallback) override {
            if (target != 0x00bd68d0) reject("reader+0C", target);
            auto* const invocation = app.current_properties;
            require(invocation && invocation->reader == visitor && read(visitor) == kReaderProfile,
                "Page field call requires its same retained raw reader/invocation");
            NativeLuaFieldResolverAdapter::Activation activation(app.resolvers, invocation->resolver);
            read_native_lua_field_or_default_00bd68d0(*invocation->reader, read(key), read(key, 4),
                field, fallback, invocation->field, app.field_values);
        }
    };
    // The admitted page has no children/timed entries/parent. Unsupported
    // families reject; no successful child deletion, type answer or list
    // progress is fabricated. Preflight those branches before scalar teardown.
    struct Widgets final : NativeGuiWidgetLifetimeBindings, NativeGuiWidgetVisibilityBindings,
        NativeGuiWidgetPropertyWidgetDispatch, NativeGuiWidgetTransformDispatch {
        Impl& app;
        explicit Widgets(Impl& value) : app(value) {}
        const volatile Word* current_table(void* actual, Word profile) override {
            (void)app.live_page(actual);
            if (profile != kPageProfile && profile != 0x00d5c130) reject("widget profile", profile);
            return table(app.data, profile, 0x80);
        }
        Word call_type_5c(Word target, void* actual) override {
            if (target != 0x00a9e110) reject("widget type+5C", target);
            return read_native_gui_widget_type_00a9e110(actual);
        }
        void call_visible_34(Word target, void* actual, std::uint8_t requested) override {
            if (target != 0x00aa8530) reject("property Visible+34", target);
            set_native_gui_widget_visible_00aa8530(actual, requested, app.visibility);
        }
        std::uint8_t call_visibility_38(void* actual, Word target) override {
            if (target != 0x00aa38e0) reject("visibility+38", target);
            return *static_cast<const volatile std::uint8_t*>(at(actual, 0xf5));
        }
        void call_visibility_changed_3c(void* actual, Word target, std::uint8_t visible) override {
            if (target != 0x00a9e100) reject("visibility changed+3C", target);
            native_gui_widget_visibility_changed_00a9e100(actual, nullptr, visible);
        }
        void call_virtual38(Word target, void* actual, const void* matrix) override {
            if (target != 0x00b6db10) reject("root transform+38", target);
            bool found = false;
            for (auto& page : app.pages)
                if (page->group_slot == actual && page->reference && !page->reference->retired() &&
                    app.renderer.actual_owners().find(actual) == page->reference.get()) found = true;
            require(found, "Page transform requires its same actual raw Group root");
            set_raw_local_matrix_00b6db10(actual, app.transform_profiles, matrix);
        }
        bool call_class_query_0c(void* actual, Word target, Word descriptor) override {
            (void)app.live_page(actual);
            if (target != 0x00aa38b0) reject("Layer class+0C", target);
            return gui_widgets_cc13::native_gui_layer_is_kind_of_00aa38b0(
                game_native_gui_text_process().layer_lineage_00f8bf88(), descriptor);
        }
        void call_child_scene_20(void*, Word target) override { reject("child scene+20", target); }
        void call_child_delete_04(void*, Word target, Word) override { reject("child delete+04", target); }
        void call_timed_delete_00(void*, Word target, Word) override { reject("timed delete+00", target); }
        void remove_all_00a9bd50(void*, const void*) override {
            reject("parent-list removal outside childless/null-parent admission", 0x00a9bd50);
        }
    };
    struct Layers final : NativeGuiLayerLifetimeBindings {
        Impl& app;
        explicit Layers(Impl& value) : app(value) {}
        void* manager_004c12b0() override { return app.warm_manager(); }
        void remove_camera_store_00aa4b30(void* actual, void* store) override {
            require(actual == app.warm_manager(), "Page store removal requires its same warm manager");
            remove_native_gui_camera_store_00aa4b30(actual, store, app.remove_literals);
        }
        const volatile Word* current_scene_table(void*, Word profile) override {
            if (profile != 0x00d62d48) reject("outer scene profile", profile);
            return table(app.data, profile, 8);
        }
        void call_scene_zero_00(void* scene, Word target) override { app.retire_scene(scene, target); }
    };
    struct PageLifetime final : NativeGuiPageLifetimeHost {
        Impl& app;
        explicit PageLifetime(Impl& value) : app(value) {}
        void call_current_page_virtual_20(void* actual) override { app.release_page_scene(actual); }
        void call_current_page_virtual_04(void* actual, Word flags) override { app.delete_page(actual, flags); }
    };

    GameHostLog& log;
    GameNativeGuiManagerApplication& manager;
    GameNativeRendererApplication& renderer;
    GameNativeReadOnlyData& data;
    const ObjectHandleResolverSlots& installed_resolvers;
    const bool& crt_sse2_conversion;
    GameNativeVfsRawServices raw;
    NativeStringRawPoolContext& strings;
    GameNativeRendererSceneDomain scene_domain;
    LightTypeBootstrap node_types;
    NativeGroupTypes group_types;
    SceneTypes scene_types;
    NativeSceneRegistrationGateContext group_gates;
    NativeNodeSceneAttachmentContext group_scenes;
    GroupTrees group_dispatch;
    NativeNodeTreeRetirementContext group_trees;
    NativeNodeBaseDestructionContext group_node;
    NativeGroupStorageIdentityContext group_identity;
    NativeGuiSceneStorageContext outer_storage;
    NativeGuiSceneIdentityContext outer_identity;
    NativeCameraStorageLifetimeContext camera_lifetime;
    NativeCameraStorageConstructContext camera;
    NativeLightLifetimeContext directional_lifetime;
    NativeDirectionalLightIdentityContext directional_identity;
    GameNativeGuiTextState& globals;
    NativeNodeMappedRawTransformProfiles transform_profiles;
    NativeLuaFieldResolverAdapter resolvers;
    NativeLuaFieldValueBindings field_values;
    NativeGuiCameraStoreMapBindings stores;
    GuiGroupBoundsCrtAccess radius;
    NativeGuiSceneAcquisitionContext scenes;
    NativeGuiWidgetPropertyLiterals property_literals;
    NativeGuiWidgetTypeLiteralViews class_literals;
    NativeGuiWidgetTypeKeyContext key_types;
    Fields fields;
    Widgets widgets;
    NativeGuiWidgetPropertyBindings widget_properties;
    NativeGuiWidgetVisibilityContext visibility;
    NativeGuiWidgetTransformBindings transform;
    NativeGuiWidgetLifetimeContext widget_lifetime;
    NativeGuiCameraStoreRemoveLiterals remove_literals;
    Layers layers;
    NativeGuiLayerLifetimeContext layer_lifetime;
    PageLifetime page_lifetime;
    Lookup lookup;
    Properties properties;
    Roots roots;
    NativeGuiScriptPathContext paths;
    // Published only after every admitted childless provider is prepared.
    bool properties_composed{};
    int layer_atexit{};
    // Every page record, including one an entered failure retains.
    std::vector<std::unique_ptr<Page>> pages;
    // Entered failures: retained, never destroyed (the host _Exits first).
    std::vector<std::unique_ptr<Construction>> retained_frames;
    std::unique_ptr<GameNativeGuiManagerApplication::PageLifetimeBinding> manager_binding;
    PropertyInvocation* current_properties{};
    bool scene_hooks_bound{}, retiring_page{};
    bool active{};
    std::uint32_t failed_site{};
    std::exception_ptr failed;

    Impl(GameHostLog& output, GameNativeGuiManagerApplication& gui, GameNativeRendererApplication& app,
        GameVfsHost& vfs, GameNativeReadOnlyData& mapped,
        const ObjectHandleResolverSlots& slots, const bool& sse2_conversion)
        : log(output), manager(gui), renderer(app), data(mapped),
          installed_resolvers(slots), crt_sse2_conversion(sse2_conversion), raw(vfs.borrow_raw_services()),
          strings(vfs.raw_strings()), scene_domain(app.startup_scene_domain()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          scene_types(*this),
          group_gates{scene_domain.node.scenes.gates.registry, scene_domain.node.scenes.gates.light_types,
              scene_domain.node.scenes.gates.camera_types, scene_domain.node.scenes.gates.actual_node_profile_00d62c88,
              scene_domain.node.scenes.gates.actual_camera_profile_00d62cf0,
              scene_domain.node.scenes.gates.actual_light_profile_00d62f58,
              scene_domain.node.scenes.gates.actual_directional_profile_00d62fb0, &scene_types},
          group_scenes{group_gates, app.actual_owners(), scene_domain.node.scenes.increment_00ce221c,
              scene_domain.node.scenes.decrement_00ce2220, scene_domain.node.scenes.children},
          group_dispatch(*this),
          group_trees{scene_domain.node.trees.point_lights, app.actual_owners(),
              scene_domain.node.trees.decrement_00ce2220, group_dispatch, scene_domain.node.trees.frames},
          group_node{group_scenes, group_trees, strings, scene_domain.node.world},
          group_identity{group_node, app.actual_owners(),
              game_native_resource_pool_process().group_pool_010902f4(), table(mapped, kGroupProfile, 0x58)},
          outer_storage{scene_domain.outer.storage.weak, strings, app.actual_owners(),
              scene_domain.outer.storage.decrement_00ce2220, scene_domain.outer.storage.actual_one_00d7a24c,
              &group_trees},
          outer_identity{outer_storage, scene_domain.outer.scene_profile_00d62d48},
          camera_lifetime{group_node, scene_domain.camera.lifetime.camera_pool_0108ffb0,
              scene_domain.camera.lifetime.increment_00ce221c, scene_domain.camera.lifetime.decrement_00ce2220,
              scene_domain.camera.lifetime.camera_profile_00d62cf0,
              scene_domain.camera.lifetime.viewport_profile_00d5e5f8,
              scene_domain.camera.lifetime.fog_profile_00d63180, scene_domain.camera.lifetime.retained},
          camera{scene_domain.camera.node_constants, scene_domain.camera.viewport, scene_domain.camera.pose_dispatch,
              scene_domain.camera.look_at, camera_lifetime, scene_domain.camera.hundred_00ce3d08,
              scene_domain.camera.scalar_00ce7d20, scene_domain.camera.scalar_00d0c5f8,
              scene_domain.camera.scalar_00d5bd98, scene_domain.camera.scalar_00ce77fc,
              scene_domain.camera.scalar_00ce3c88},
          directional_lifetime{group_node, scene_domain.directional.lifetime.shadow_zero},
          directional_identity{directional_lifetime, app.actual_owners(),
              scene_domain.directional.pool_01090154, scene_domain.directional.actual_profile_00d62fb0},
          globals(game_native_gui_text_state()), transform_profiles(mapped),
          resolvers(installed_resolvers, game_native_entity_registry_process().handle_tables(), literal(mapped, 0x00cfad08, 4)),
          field_values{strings, crt_sse2_conversion, this->resolvers.number_cell(), this->resolvers.table_cell()},
          stores{word(mapped, 0x00d7a2f0), word(mapped, 0x00ce3804), word(mapped, 0x00d7a24c),
              globals.widget_empty_00f8bc60()},
          radius{&application_camera_axes_crt(),
              &game_native_dyn_process(application_camera_axes_crt()).crt_conversion_0109eea4()},
          scenes{gui.context(), stores, outer_identity, scene_domain.scene_fields, scene_domain.resource,
              camera, directional_identity, scene_domain.fog_constants, scene_domain.fog,
              radius, scalar<double>(mapped, 0x00d7a308), word(mapped, 0x00ce7820),
              literal(mapped, 0x00d5cb5c, 8), literal(mapped, 0x00d5cb50, 10), literal(mapped, 0x00d5cb3c, 20)},
          property_literals{{literal(mapped, 0x00cf168c, 4), literal(mapped, 0x00cff278, 5),
              literal(mapped, 0x00d5c228, 6), literal(mapped, 0x00ce60ac, 6), literal(mapped, 0x00d5c220, 7),
              literal(mapped, 0x00ce93f8, 6), literal(mapped, 0x00d5c214, 9), literal(mapped, 0x00d5c208, 10),
              literal(mapped, 0x00d5c1fc, 12), literal(mapped, 0x00d5c1ec, 16)},
              literal(mapped, 0x00ce3a0c, 1), literal(mapped, 0x00ce92e4, 5), literal(mapped, 0x00ce92dc, 6)},
          class_literals{literal(mapped, 0x00ce8ec8, 5), literal(mapped, 0x00cec778, 5),
              literal(mapped, 0x00d05680, 6), literal(mapped, 0x00d5be04, 8), literal(mapped, 0x00d5bdfc, 5),
              literal(mapped, 0x00d5bdf0, 10), literal(mapped, 0x00d5bde8, 5), literal(mapped, 0x00d5bde0, 6),
              literal(mapped, 0x00cebb64, 6), literal(mapped, 0x00d5bdd8, 8), literal(mapped, 0x00d5bdcc, 9),
              literal(mapped, 0x00d5bdc4, 6), literal(mapped, 0x00cea3fc, 6), literal(mapped, 0x00d5bdbc, 7),
              literal(mapped, 0x00d5bdb4, 8), literal(mapped, 0x00d5bdac, 8), literal(mapped, 0x00d5bda0, 9)},
          key_types{globals.widget_empty_00f8bc60(), literal(mapped, 0x00ce7890, 2), class_literals, &_stricmp},
          fields(*this), widgets(*this),
          widget_properties{property_literals, scalar<float>(mapped, 0x00d7a24c),
              globals.widget_white_guard_00f8bcf0(), globals.widget_white_color_00f8bce0(),
              globals.widget_black_guard_00f8bcdc(), globals.widget_black_color_00f8bccc(),
              *static_cast<void* const volatile*>(app.borrow_platform_publication_0109cf04().identity()),
              scalar<double>(mapped, 0x00d5c118), strings, fields},
          visibility{scalar<float>(mapped, 0x00d7a218), widgets},
          transform{scalar<float>(mapped, 0x00d7a24c), scalar<float>(mapped, 0x00d7a208),
              globals.widget_y_scale_00e12fc4(), widgets},
          widget_lifetime{app.camera_environment().nodes, table(mapped, 0x00d5c130, 0x24),
              game_native_gui_text_process().element_lineage_00f8bc88(),
              game_native_gui_text_process().text_lineage_00f8be28()[0], widgets},
          remove_literals{literal(mapped, 0x00d5bf80, 1), literal(mapped, 0x00d5bf54, 1),
              globals.widget_empty_00f8bc60()}, layers(*this),
          layer_lifetime{widget_lifetime, strings, layers, scene_domain.node.scenes.decrement_00ce2220},
          page_lifetime(*this), lookup(mapped), properties(*this), roots(*this),
          paths{vfs.raw_strings(), literal(mapped, 0x00d5cb18, 11), literal(mapped, 0x00cfd2c8, 5)} {
        auto& node_names = scene_domain.node.strings;
        require(&node_names.actual_published_01090aa8 == &strings.actual_published_01090aa8 &&
            &node_names.actual_manager_publication_01090aa0 == &strings.actual_manager_publication_01090aa0 &&
            &scene_domain.node.scenes.owners == &app.actual_owners() &&
            &scene_domain.camera.node_constants.one_00d7a24c == &app.node_constants().one_00d7a24c,
            "native GUI pages require the same canonical node/string/table domains");
        layer_atexit = game_native_gui_layer_pool_process().initialize_once_00cd73d0();
        // This distinct CRT lineage was not installed by the P3a1 prefix.
        // The existing owner rejects uninitialized aliases and retargeting.
        game_native_gui_text_process().initialize_layer_type_once_00cd73f0(
            vfs.native_owners().types(), vfs.native_types().light_types());
    }
    void require_entry() const {
        require(!active && !failed && !retiring_page && !current_properties && !resolvers.requires_process_retention(),
            "native GUI page chain cannot reenter or replay interrupted work");
    }
    void fail(std::uint32_t site) noexcept {
        if (!failed) { failed_site = site; failed = std::current_exception(); }
    }
    void release(NativeString& header, bool& live) {
        live = false;
        destroy_native_string_header_0041dd20(&header, strings);
    }
    // B8F450 + B8F5E0(name), then the canonical companion for B6D890.
    void create_group(Page& page, const void* name) {
        page.group_slot = allocate_native_group_slot_00b8f450();                    // 00AA58FF
        require(page.group_slot != nullptr, "native GUI page Group pool returned null");
        construct_native_group_00b8f5e0(page.group_slot, NativeGroupPool::slot_bytes,
            name, strings, renderer.node_constants());                                    // 00AA5914
        page.reference = std::make_unique<NativeGroupStorageReference>(page.group_slot,
            group_identity, page.group_lifetime_frame, page.group_lifetime);
    }
    Page& live_page(void*);
    void* warm_manager();
    void read_properties(void*, NativeLuaReaderStorage&);
    void read_childless_keys(PropertyInvocation&, void*, NativeLuaReaderStorage&);
    void retire_scene(void*, Word);
    void release_page_scene(void*);
    void delete_page(void*, Word);
    void bind_providers();
    ~Impl() noexcept;
};

GameNativeGuiPageApplication::Impl::Page& GameNativeGuiPageApplication::Impl::live_page(void* actual) {
    for (auto& page : pages)
        if (page->page == actual && !page->native_retired) return *page;
    throw std::logic_error("native Page entry has no same live application allocation");
}

void* GameNativeGuiPageApplication::Impl::warm_manager() {
    require(manager.context().actual_gui_00f8bc5c != nullptr,
        "native Page lifetime requires its already-published warm manager");
    return manager.get_004c12b0();
}

void GameNativeGuiPageApplication::Impl::read_properties(void* actual, NativeLuaReaderStorage& reader) {
    auto& page = live_page(actual);
    require(properties_composed && !current_properties && !page.properties && read(actual, 0x60) == 1,
        "childless AC4C50 requires a fresh actual type1 Page and retained invocation");
    page.properties = std::make_unique<PropertyInvocation>();
    auto& f = *page.properties;
    auto& a = f.acquired;
    f.reader = &reader;
    current_properties = &f;
    a.phase = NativeGuiPageReadPropertiesAcquired::Phase::entered;
    auto* const base = f.locals;
    auto* const field = at(base, 8);
    auto* const fallback = at(base, 0x10);
    try {
        // Exact recovered AC4C50 ordering and live pair addresses. This
        // application specializes only the later child suffix, not these calls.
        write(fallback, 4, 0); write(fallback, 0, 1);
        write(field, 4, bits(at(base, 0x20)));
        write(base, 4, bits(literal(data, 0x00d5cb00, 9))); write(field, 0, 1);
        const Word priority_target = table(data, read(&reader), 0x10)[3]; // BEFORE key-tag store.
        write(base, 0, 0);
        a.active_call_site = 0x00ac4c8b;
        fields.call_virtual0c(priority_target, &reader, base, field, fallback);
        const Word priority = read(base, 0x20);
        write(actual, 0xfc, priority); write(actual, 0x118, priority); a.priority_published = true;
        write(fallback, 0, 2); write(fallback, 4, 0);
        write(field, 0, 2); write(field, 4, bits(at(actual, 0x11c)));
        write(base, 0, 0); write(base, 4, bits(literal(data, 0x00d5caf4, 12)));
        a.active_call_site = 0x00ac4cdd;
        fields.call_virtual0c(table(data, read(&reader), 0x10)[3], &reader, base, field, fallback);
        a.render_order_read = true;
        a.loaded_profile = read(actual);
        a.loaded_target = table(data, a.loaded_profile, 0x7c)[0x78 / 4];
        if (a.loaded_target != 0x00ac59a0) reject("Page scene+78", a.loaded_target);
        a.active_call_site = 0x00ac4ce6;
        acquire_native_gui_scene_storage_warm_manager_00ac59a0(actual, f.scenes.frame, scenes, a.scene);
        a.scene_returned = true;
        a.requested_scene = pointer(read(actual, 0xec)); // AFTER current78, BEFORE current4C.
        a.root_node = pointer(read(actual, 0x4c));
        a.active_call_site = 0x00ac4cf2;
        propagate_native_node_root_storage_00b6d890(a.root_node, a.requested_scene,
            scene_domain.node.scenes.children);
        a.root_returned = true;
        write(base, 0x24, 0); write(base, 0x28, 0); write(base, 0x2c, 0);
        a.active_call_site = 0x00ac4d13;
        set_native_gui_widget_local_position_00aa7dc0(actual, at(base, 0x24),
            f.transform, transform, f.bounds, scalar<double>(data, 0x00d7a280));
        a.position_returned = true;
        NativeGuiWidgetPropertyInteractionContext interaction{widget_properties,
            literal(data, 0x00d5c1e4, 8), literal(data, 0x00d5c1d8, 11), literal(data, 0x00d5c1cc, 9),
            widgets, visibility, f.transform, transform};
        a.active_call_site = 0x00ac4d1b;
        read_native_gui_widget_properties_through_transform_00aaa710(actual, &reader, f.widget, interaction);
        read_childless_keys(f, actual, reader);
        a.properties_returned = true;
        a.phase = NativeGuiPageReadPropertiesAcquired::Phase::complete;
        f.reader = nullptr;
        current_properties = nullptr;
    } catch (...) {
        a.phase = NativeGuiPageReadPropertiesAcquired::Phase::failed;
        fail(a.active_call_site);
        throw; // Current invocation and all reached native owners remain retained.
    }
}

void GameNativeGuiPageApplication::Impl::read_childless_keys(
    PropertyInvocation& f, void* actual, NativeLuaReaderStorage& reader) {
    (void)actual; // No admitted child branch reaches AA6560 or a Model factory.
    auto& a = f.acquired.children;
    require(a.phase == NativeGuiWidgetPropertyChildrenAcquired::Phase::fresh,
        "childless AAACD4 requires fresh persistent diagnostics");
    a.phase = NativeGuiWidgetPropertyChildrenAcquired::Phase::entered;
    auto* const base = f.widget.bytes;
    auto* const block = at(base, 0x4c);
    auto* const count = at(base, 0x81c);
    auto* const name = at(base, 0x38);
    auto site = [&](Word value) { a.active_call_site = value; f.acquired.active_call_site = value; };
    auto release_name = [&] {
        a.name_live = false; a.name_cleanup_entered = true;
        site(0x00aaae85);
        destroy_native_string_header_0041dd20(name, strings);
    };
    try {
        for (Word index = 0; index != 250; ++index) write(block, index * 8, 0xffffffffu);
        const Word target = table(data, read(&reader), 0x1c)[0x18 / 4]; // BEFORE count store.
        write(count, 0, 0);
        site(0x00aaad01);
        if (target != 0x00bd5f50) reject("reader key enumeration+18", target);
        // Begin the trivial borrowed output view without clearing its native
        // preimages. Actual BD5F50 enumerates the real reader in native order.
        auto* const output = ::new (block) NativeLuaReaderKeys;
        native_lua_reader_enumerate_keys_00bd5f50(reader, *output, f.keys, crt_sse2_conversion);
        write(base, 0x2c, 0);
        if (static_cast<std::int32_t>(read(count)) > 0) {
            write(base, 0x34, bits(block));
            do {
                void* const entry = pointer(read(base, 0x34));
                if (read(entry) == 0) {
                    const char* const source = static_cast<const char*>(pointer(read(entry, 4)));
                    write(name, 0, 0); write(name, 4, 0);
                    Word length = 0;
                    while (source[length] != '\0') ++length;
                    site(0x00aaad4a);
                    resize_native_string_header_0041dd40(name, strings, length, true);
                    if (void* destination = pointer(read(name, 4)))
                        std::memcpy(destination, source, static_cast<std::size_t>(read(name) + 1u));
                    a.name_live = true; a.name_cleanup_entered = false;
                    site(0x00aaad78);
                    const Word type = native_gui_widget_type_for_key_00aa2490(name, key_types);
                    if (type) {
                        f.rejected_widget_type = type;
                        site(0x00aaad89);
                        reject("AA6560 widget in childless Page", type);
                    }
                    release_name(); // Exact type0 skip still constructs/maps/returns the name.
                }
                const Word next_index = read(base, 0x2c) + 1u;
                const Word next_entry = read(base, 0x34) + 8u;
                write(base, 0x2c, next_index); write(base, 0x34, next_entry);
            } while (static_cast<std::int32_t>(read(base, 0x2c)) < static_cast<std::int32_t>(read(count)));
        }
        a.phase = NativeGuiWidgetPropertyChildrenAcquired::Phase::complete;
    } catch (...) {
        const Word child_failed_site = a.active_call_site;
        a.phase = NativeGuiWidgetPropertyChildrenAcquired::Phase::failed;
        if (a.name_live) {
            try { release_name(); }
            catch (...) { std::terminate(); }
        }
        f.acquired.active_call_site = child_failed_site;
        throw;
    }
}

void GameNativeGuiPageApplication::Impl::retire_scene(void* actual, Word target) {
    if (target != 0x00bd30e0) reject("outer scene current0", target);
    for (auto& page : pages) {
        if (!page->properties) continue;
        auto& scene = page->properties->acquired.scene;
        if (scene.scene_result != actual || !scene.scene_reference || scene.scene_reference->retired()) continue;
        auto& reference = *scene.scene_reference;
        require(renderer.actual_owners().find(actual) == &reference &&
            static_cast<void*>(&reference.reference_count) == static_cast<unsigned char*>(actual) + 4 &&
            reference.reference_count.load(std::memory_order_relaxed) == 0,
            "outer scene zero requires its same observed-zero canonical owner");
        reference.release_zero_references();
        return;
    }
    throw std::logic_error("Page scene terminal has no same live raw scene companion");
}

void GameNativeGuiPageApplication::Impl::release_page_scene(void* actual) {
    require_entry();
    retiring_page = true;
    try {
        auto& page = live_page(actual);
        require(read(actual) == kPageProfile && table(data, kPageProfile, 0x24)[8] == 0x00aa8320,
            "Page retirement requires its genuine current20 AA8320");
        const Word head = read(actual, 0x68);
        require(head && read(actual, 0x6c) == 0 && read(pointer(head)) == head && read(pointer(head), 4) == head,
            "Page scene retirement admits only the actual empty child ring");
        // Child walk is genuinely empty, then the CURRENT class-cell address
        // is captured BEFORE the descriptor getter, exactly as AA8320 does.
        const auto* const class_cell = table(data, read(actual), 0x10) + 3;
        const Word descriptor = native_gui_text_descriptor_00ab6a30(
            game_native_gui_text_process().text_lineage_00f8be28()[0]);
        const Word target = *class_cell;
        const bool is_text = widgets.call_class_query_0c(actual, target, descriptor);
        if (is_text) reject("Text glyph family/extent outside actual124h Layer", 0x00ab73b0);
        if (void* const root = pointer(read(actual, 0x4c))) {
            require(root == page.group_slot && page.reference && !page.reference->retired() &&
                renderer.actual_owners().find(root) == page.reference.get() && read(root, 0x130) == 0,
                "childless Page release requires its same raw root and admits no retained130 family");
            // B6DFA0/B8EEC0/B6F310 own the SINGLE creator release. The Group
            // terminal may run INSIDE this call while page+4C is still set.
            unlink_native_node_tree_00b6dfa0(root, group_trees, page.group_unlink);
            write(actual, 0x4c, 0);
        }
        page.scene_released = true;
        retiring_page = false;
    } catch (...) { fail(0x00aa8320); throw; }
}

void GameNativeGuiPageApplication::Impl::delete_page(void* actual, Word flags) {
    require_entry();
    retiring_page = true;
    try {
        auto& page = live_page(actual);
        // The manager/AA31F0 calls are flags1. The raw Group's +04 has no such
        // page-field guard; only this later PAGE operation requires +4C cleared.
        const Word head = read(actual, 0x68);
        require(flags == 1 && page.scene_released && read(actual, 0x4c) == 0 &&
            head && read(actual, 0x6c) == 0 && read(pointer(head)) == head && read(pointer(head), 4) == head &&
            read(actual, 0x70) == 0 && read(actual, 0x88) == 0 && read(actual, 0x8c) == 0 && read(actual, 0x90) == 0,
            "Page deleting04 requires released scene, empty children/timed storage and null parent");
        require(read(actual) == kPageProfile && table(data, kPageProfile, 8)[1] == 0x00aa38f0,
            "Page deletion requires its genuine current04 AA38F0");
        // AA9730 stamps the base profile before its second AA8320. A real Text
        // match would enter an unadmitted glyph extent; never replace it with false.
        const Word text = native_gui_text_descriptor_00ab6a30(game_native_gui_text_process().text_lineage_00f8be28()[0]);
        if (native_gui_widget_is_kind_of_00a9e070(text, game_native_gui_text_process().element_lineage_00f8bc88()))
            reject("base Text glyph family/extent outside actual124h Layer", 0x00ab73b0);
        (void)scalar_delete_native_gui_layer_00aa38f0(actual, flags, layer_lifetime);
        page.native_retired = true; // Metadata only; never reread returned pool storage.
        retiring_page = false;
    } catch (...) { fail(0x00aa38f0); throw; }
}

void GameNativeGuiPageApplication::Impl::bind_providers() {
    require(!properties_composed && !scene_hooks_bound && !manager_binding && pages.empty(),
        "childless Page providers bind once before a native page exists");
    (void)warm_manager();
    renderer.bind_startup_scene_child_delegate(&roots); // Throws before publication on conflict.
    scene_hooks_bound = true;
    // The unique_ptr-owned complete Impl restores the child metadata hook if
    // token allocation rejects. Its derived gate is private, never published
    // into the frozen A1 context. No throwing construction/logging follows.
    manager_binding = manager.bind_page_lifetime_host(page_lifetime);
    properties_composed = true;
}

GameNativeGuiPageApplication::Impl::~Impl() noexcept {
    if (active || failed || retiring_page || current_properties || resolvers.requires_process_retention())
        std::terminate();
    for (const auto& page : pages) {
        if (!page->native_retired || (page->reference && !page->reference->retired())) std::terminate();
        if (page->properties) {
            const auto& scene = page->properties->acquired.scene;
            auto retired = [](const auto& reference) { return !reference || reference->retired(); };
            if (!retired(scene.scene_reference) || !retired(scene.camera_reference) ||
                !retired(scene.resource_reference) || !retired(scene.directional_reference) ||
                !retired(scene.resource_construct.ambient_reference)) std::terminate();
        }
    }
    if (scene_hooks_bound) {
        try { renderer.bind_startup_scene_child_delegate(nullptr); }
        catch (...) { std::terminate(); }
    }
}

GameNativeGuiPageApplication::GameNativeGuiPageApplication(GameHostLog& log,
    GameNativeGuiManagerApplication& manager, GameNativeRendererApplication& renderer,
    GameVfsHost& files, GameNativeReadOnlyData& data,
    const ObjectHandleResolverSlots& resolvers, const bool& sse2_conversion)
    : impl_(std::make_unique<Impl>(log, manager, renderer, files, data, resolvers, sse2_conversion)) {
    log.notef("native GUI childless Page chain prepared: layer_pool=00f8bf50 atexit=%d group_pool=010902f4 "
        "properties=ac4c50-childless", impl_->layer_atexit);
    impl_->bind_providers(); // Last throwing step: complete Impl owns rollback/retention.
}

GameNativeGuiPageApplication::~GameNativeGuiPageApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}

bool GameNativeGuiPageApplication::requires_process_retention() const noexcept {
    return impl_->failed != nullptr || impl_->active || impl_->retiring_page ||
        impl_->current_properties || impl_->resolvers.requires_process_retention();
}
std::uint32_t GameNativeGuiPageApplication::failure_site() const noexcept { return impl_->failed_site; }

void* GameNativeGuiPageApplication::load_page_00aa5840(void* actual_manager,
    const void* name, std::int32_t screen_flag, std::uint8_t add_reference) {
    auto& p = *impl_;
    p.require_entry();
    require(actual_manager && actual_manager == p.manager.get_004c12b0() && name,
        "native GUI page load requires the published 00F8BC5C manager and a name");
    void* page = find_native_gui_page_00aa3140(actual_manager, name, p.lookup);  // 00AA5862
    if (page) {
        if (add_reference)                                                         // 00AA5985
            game_native_renderer_scalar_process().increment_iat_00ce221c()(
                reinterpret_cast<volatile long*>(static_cast<unsigned char*>(page) + 4));
        return page;
    }
    require(p.properties_composed, "native GUI page construction waits for AC4C50 (P3a2)");

    // Publish retained metadata before the first native name/pool/Lua effect.
    // Failure transport below must not allocate while preserving live frames.
    p.retained_frames.push_back(std::make_unique<Impl::Construction>());
    auto& retained = p.retained_frames.back();
    auto& f = *retained;
    p.active = true;
    Impl::Page* record{};
    std::uint32_t site = 0x00aa587a;
    try {
        construct_native_string_header_0041e870(&f.suffix, p.strings, literal(p.data, 0x00d0e67c, 6));
        f.suffix_live = true;
        site = 0x00aa588f;
        concatenate_native_string_headers_004261a0(name, &f.path, &f.suffix, p.strings);
        f.path_live = true;
        site = 0x00aa5894;
        p.release(f.suffix, f.suffix_live);
        site = 0x00aa58c3;
        const bool model = resolve_native_vfs_existing_name_00bdf4c0(p.raw.actual_vfs_publication_0109ceec,
            &f.path, p.raw.name_resolution, f.resolution);
        if (model) {
            // 004C1400 / B80D70 / factory+8 and manager+30 are not composed.
            p.release(f.path, f.path_live);
            p.active = false;
            throw std::logic_error("native GUI page .mmod model route is not composed");
        }
        site = 0x00aa58ff;
        // Listed before B8F450 so AC6806's B6D890 finds its Group.
        p.pages.push_back(std::make_unique<Impl::Page>());
        record = p.pages.back().get();
        record->app = &p;
        p.create_group(*record, name);
        site = 0x00aa5929;
        record->page = allocate_native_gui_layer_slot_00ac51a0();
        require(record->page != nullptr, "native GUI layer pool returned null");
        site = 0x00ac6600;
        NativeGuiPageConstructionContext prefix_context{p.raw.strings, word(p.data, 0x00d7a2f0),
            word(p.data, 0x00ce3804), p.renderer.node_constants().one_00d7a24c};
        initialize_native_gui_page_00ac6600_prefix(record->page, name,
            static_cast<NativeNodeStorage*>(record->group_slot),
            static_cast<std::uint32_t>(screen_flag), prefix_context, f.prefix);
        site = 0x00ac6714;
        auto& lua = p.renderer.lua_service_binding();
        NativeGuiPageScriptContext scripts{p.raw.strings, p.paths, lua, literal(p.data, 0x00d5cb64, 8)};
        continue_native_gui_page_scripts_00ac6714(f.prefix, f.scripts, scripts);
        site = 0x00ac67cc;
        NativeGuiPageReaderContext reader{lua, literal(p.data, 0x00d5cb24, 10), table(p.data, kReaderProfile, 0x10)};
        continue_native_gui_page_reader_00ac67cc(f.prefix, f.scripts, f.reader, reader);
        site = 0x00ac6806;
        NativeGuiPagePropertiesContext tail{p.properties, p.roots};
        continue_native_gui_page_properties_00ac6806(f.prefix, f.reader, f.properties, tail);
        require(f.prefix.phase == NativeGuiPageConstructionAcquired::Phase::constructed,
            "native GUI page AC6600 did not reach its constructed tail");
        page = record->page;
        site = 0x00aa5957;
        register_native_gui_page_00aa52a0(actual_manager, page);
        site = 0x00aa595c;
        p.release(f.path, f.path_live);
        retained.reset();
        p.active = false;
        p.log.notef("native GUI page loaded: page=%p group=%p manager=%p flag=%d",
            page, record->group_slot, actual_manager, screen_flag);
        return page;
    } catch (...) {
        if (!p.active) throw; // model-route rejection consumed both headers
        p.fail(site);
        p.active = false;
        throw;
    }
}
} // namespace bsp::game
