#include "bsp/game_native_gui_page_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_model_domain.hpp"
#include "bsp/model_type_bootstrap.hpp"
#include "bsp/native_gui_group_factory.hpp"
#include "bsp/native_gui_group_identity.hpp"
#include "bsp/native_gui_widget_load_hooks.hpp"
#include "bsp/native_node_parenting_storage.hpp"
#include "bsp/cc13_gui1.hpp"
#include "bsp/singleton_lifetime.hpp"
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
#include <deque>
#include <list>
#include <limits>
#include <optional>

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
    std::snprintf(message, sizeof message, "native GUI Page has no supplied %s target/type %08x", slot, target);
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
    // the whole AC4C50 composition with the same per-invocation raw child context.
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
    struct TreeCall {
        NativeNodeTreeRetirementFrame frame{};
        NativeNodeTreeRetirementAcquired acquired;
        NativeGroupStorageReleaseAcquired group;
        void* actual{};
        Word target{};
        std::exception_ptr exception;
        bool late_primary{};
    };
    struct SceneCall {
        NativeNodeSceneAttachmentFrame frame{}; NativeNodeSceneAttachmentAcquired acquired;
        void* actual{}; Word target{}; std::exception_ptr exception;
    };
    struct AttachmentCall {
        NativeNodeAttachmentFrame frame{}; NativeNodeParentingAcquired acquired;
        void* actual{}; Word target{}; std::exception_ptr exception;
    };
    struct ParentNullCall { NativeNodeParentNullFrame frame{}; NativeNodeParentNullAcquired acquired; };
    struct ScalarCall { volatile Word flags{}; bool entered{}, complete{}; };
    struct NativeCalls {
        std::deque<TreeCall> trees;
        std::deque<SceneCall> scenes;
        std::deque<AttachmentCall> attachments;
        std::deque<ParentNullCall> parents;
        std::deque<ScalarCall> scalars;
        std::size_t tree_used{}, scene_used{}, attachment_used{}, parent_used{}, scalar_used{};
        bool failed{};
        void prepare(std::size_t nodes) {
            // This Source graph's callback edges are the finite default-Group
            // producer, postorder AA8320, raw tree/scene walkers and scalars.
            // Each node has separate call homes, including cleanup/repeated
            // scene passes. Capacity grows with actual producer records.
            require(nodes <= (std::numeric_limits<std::size_t>::max() - 8) / 8,
                "Page call graph metadata extent overflow");
            const auto target = nodes * 8 + 8;
            while (trees.size() < target) trees.emplace_back();
            while (scenes.size() < target) scenes.emplace_back();
            while (attachments.size() < target) attachments.emplace_back();
            while (parents.size() < target) parents.emplace_back();
            while (scalars.size() < target) scalars.emplace_back();
        }
        template<class T> T& take(std::deque<T>& slots, std::size_t& used) {
            require(used < slots.size(), "Page native call lacks prepared graph backing");
            return slots[used++]; // monotonic; completed/failed frames are never reset
        }
        TreeCall& tree() { return take(trees, tree_used); }
        SceneCall& scene() { return take(scenes, scene_used); }
        AttachmentCall& attachment() { return take(attachments, attachment_used); }
        ParentNullCall& parent() { return take(parents, parent_used); }
        ScalarCall& scalar() { return take(scalars, scalar_used); }
    };
    // The raw Group's distinct current18 is B8EEC0. Its canonical zero terminal
    // may still hold an actual3Ch scene resource; its genuine raw destructor
    // closes that field. Never route it through a legacy SceneResource* owner.
    struct GroupTrees final : NativeNodeTreeRetirementDispatch, NativeNodeTreeRetirementFrames {
        Impl& app;
        explicit GroupTrees(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            return app.roots.resolve_profile(profile);
        }
        NativeNodeTreeRetirementInvocation recursive_unregister(void* actual) override {
            app.authenticate_node(actual);
            auto& call = app.calls().tree(); call.actual = actual; call.target = 0x00b6d850;
            return {call.frame, call.acquired};
        }
        void invoke_virtual18(Word target, void* actual) override {
            app.authenticate_node(actual);
            auto& call = app.calls().tree(); call.actual = actual; call.target = target;
            try {
                if (auto* model = app.find_model(actual)) {
                    model->release_raw_virtual18_00b6f310(target, call.frame, call.acquired);
                    return; // companion may have died; only retained diagnostics survive
                }
                if (app.find_root(actual)) {
                    require(target == 0x00b8eec0, "raw Page root requires genuine current18 B8EEC0");
                    release_native_group_storage_00b8eec0(actual, call.frame, app.group_trees, call.group);
                    return;
                }
                require(target == 0x00b6f310, "Page camera/light requires genuine current18 B6F310");
                release_native_node_tree_00b6f310(actual, call.frame, app.group_trees, call.acquired);
            } catch (...) {
                call.exception = std::current_exception(); app.calls().failed = true;
                const auto stopped = call.group.started ? call.group.active_call_site : call.acquired.active_call_site;
                app.fail(stopped ? stopped : call.target);
                throw;
            }
        }
        void invoke_virtual0(Word target, void* actual, RenderCommandReference& reference) override {
            app.authenticate_node(actual);
            require(target == 0x00bd30e0 &&
                app.renderer.actual_owners().find(actual) == &reference &&
                static_cast<void*>(&reference.reference_count) == at(actual, 4) &&
                reference.reference_count.load(std::memory_order_relaxed) == 0,
                "Page tree terminal requires SAME observed-zero canonical actual+04 owner");
            reference.release_zero_references();
        }
    };
    // B6D890 from AC4C50 and AC6806 on the SAME raw page-root Group. Each
    // normal page construction has two distinct prepared attachment frames.
    struct Roots final : NativeNodeSceneChildDispatch {
        Impl& app;
        explicit Roots(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            if (profile == kGroupProfile) return app.group_identity.actual_profile_00d634f8;
            if (profile == 0x00d62de8) return app.models->raw_access().models.vtable_00d62de8;
            if (profile == 0x00d62c88) return app.models->raw_access().models.vtable_00d62c88;
            if (profile == 0x00d62cf0 || profile == 0x00d62fb0 || profile == 0x00d62f58)
                return table(app.data, profile, 0x58);
            reject("raw Page node profile", profile);
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            app.authenticate_node(actual);
            auto& call = app.calls().scene(); call.actual = actual; call.target = target;
            call.frame.scene_argument = requested;
            call.frame.recursion_argument = recursion;
            try {
                if (auto* model = app.find_model(actual)) {
                    if (target == 0x00b6ee10) {
                        model->remove_raw_scene_virtual54_00b6ee10(target, call.frame, call.acquired);
                        return;
                    }
                    require(target == 0x00b6ed80, "Page Model requires genuine current50 B6ED80");
                    set_native_node_scene_storage_00b6ed80(actual, call.frame, app.group_scenes, call.acquired);
                    return;
                }
                const auto profile = read(actual);
                if (profile == kGroupProfile || profile == 0x00d62c88 || profile == 0x00d62cf0) {
                    if (target == 0x00b6ed80)
                        set_native_node_scene_storage_00b6ed80(actual, call.frame, app.group_scenes, call.acquired);
                    else if (target == 0x00b6ee10)
                        remove_native_node_scene_storage_00b6ee10(actual, call.frame, app.group_scenes, call.acquired);
                    else reject("Page node current50/54", target);
                    return;
                }
                if (target == 0x00b7c020)
                    attach_native_light_scene_storage_00b7c020(actual, call.frame, app.group_scenes, call.acquired);
                else if (target == 0x00b7bd60)
                    remove_native_light_scene_storage_00b7bd60(actual, call.frame, app.group_scenes, call.acquired);
                else reject("Page light current50/54", target);
            } catch (...) {
                call.exception = std::current_exception(); app.calls().failed = true;
                app.fail(call.acquired.active_call_site ? call.acquired.active_call_site : call.target);
                throw;
            }
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
        std::vector<NativeNodeTreeRetirementAcquired> root_calls;
        NativeGuiSceneDestroyFrame outer_destroy{};
        void prepare(std::size_t nodes) {
            require(!outer_destroy.used && nodes <= std::numeric_limits<Word>::max(),
                "Page scene root-call preparation must precede destruction");
            if (root_calls.size() < nodes) root_calls.resize(nodes);
            outer_destroy.root_calls = root_calls.data();
            outer_destroy.capacity = static_cast<Word>(root_calls.size());
        }
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
        NativeSceneResourceConstructFrame resource_construct{};
        NativeLightLifetimeFrame directional_destroy{};
        std::array<volatile Word, 32> directional_world{};
        NativeGuiSceneAcquisitionFrame frame{locals.data(), arguments.data(), find, create,
            outer_construct, outer_destroy, camera_construct, camera_destroy, resource_construct,
            directional_destroy, {directional_world.data()}};
    };
    struct PropertyInvocation {
        SceneFrames scenes;
        NativeGuiWidgetPropertyScratch widget{};
        NativeGuiWidgetTransformScratch transform{};
        GuiWidgetBounds bounds;
        NativeLuaReaderFieldScratch field{};
        NativeLuaFieldResolverFrame resolver;
        NativeLuaReaderQueryScratch keys{};
        NativeGuiPageReadPropertiesFrame page_frame{{}, scenes.frame, widget, bounds};
        NativeGuiPageReadPropertiesAcquired acquired;
        NativeLuaObjectStorage enter{};
        NativeNodeParentingFrame parent_frame{};
        NativeNodeSetParentAcquired parent_acquired;
        NativeNodeParentNullFrame detach_frame{};
        NativeNodeParentNullAcquired detach_acquired;
        NativeCalls native_calls;
        std::optional<NativeGuiWidgetPropertyInteractionContext> interaction_context;
        std::optional<NativeGuiWidgetPropertyChildrenRawContext> children_context;
        std::optional<NativeGuiPageReadPropertiesRawContext> page_context;
        NativeCalls* suspended_calls{};
        NativeLuaReaderStorage* reader{};
        PropertyInvocation* suspended{};
        void* actual_widget{};
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
        NativeCalls construction_calls;
        std::vector<std::unique_ptr<NativeCalls>> lifetime_calls;
        void* page{};
        bool scene_released{}, native_retired{};
    };
    struct WidgetRecord {
        void* actual{};
        bool constructor_returned{}, native_retired{};
        std::unique_ptr<NativeGuiGroupReference> reference;
        std::unique_ptr<PropertyInvocation> properties;
        GuiWidgetBounds loaded_bounds;
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

    struct SceneTypes final : NativeSceneRegistrationDerivedTypes, NativeNodeParentingDispatch {
        Impl& app;
        explicit SceneTypes(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override { return app.roots.resolve_profile(profile); }
        std::uint8_t invoke_type_0c(Word target, void* actual, Word token) override {
            app.authenticate_node(actual);
            if (target == 0x00b8f650 && app.find_root(actual))
                return static_cast<std::uint8_t>(app.group_types.is_type_00b8f650(token));
            if (target == 0x006ef860 && app.find_model(actual))
                return static_cast<std::uint8_t>(app.models->raw_access().models.types.is_type_006ef860(token));
            if (target == 0x00b6f570 && read(actual) == 0x00d62c88)
                return static_cast<std::uint8_t>(app.node_types.node_is_type_00b6f570(token));
            reject("raw Page node type+0C", target);
        }
        void invoke_attachment_1c(Word target, void* actual, Word requested) override {
            app.authenticate_node(actual);
            auto& call = app.calls().attachment(); call.actual = actual; call.target = target;
            call.frame.attachment_argument = requested;
            try {
                if (target == 0x00b6d7b0 && app.find_model(actual))
                    set_native_node_attachment_00b6d7b0(actual, call.frame, *this, call.acquired);
                else if (target == 0x00b8f4f0 && app.find_root(actual))
                    set_native_group_attachment_00b8f4f0(actual, call.frame, call.acquired);
                else reject("raw Page node attachment+1C", target);
            } catch (...) {
                call.exception = std::current_exception(); app.calls().failed = true;
                app.fail(call.acquired.active_call_site ? call.acquired.active_call_site : call.target); throw;
            }
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
    struct ChildCalls final : NativeGuiWidgetPropertyChildrenCalls, NativeGuiWidgetLoadedDispatch {
        Impl& app;
        explicit ChildCalls(Impl& value) : app(value) {}
        void enumerate_keys_18(void* visitor, Word target, void* block) override {
            if (target != 0x00bd5f50) reject("reader key enumeration+18", target);
            auto& f = app.reader_invocation(visitor);
            auto* output = ::new (block) NativeLuaReaderKeys;
            native_lua_reader_enumerate_keys_00bd5f50(*f.reader, *output, f.keys, app.crt_sse2_conversion);
        }
        void* create_widget_00aa6560(Word type, const void* source) override {
            require(app.current_properties, "Group factory requires its retained parent reader");
            if (type != 2 || source) reject("AA6560 factory outside default GuiGroup", type);
            // The record exists before AA12F0; constructor-return custody is
            // recorded before canonical allocation/bind can fail.
            app.widget_records.push_back(std::make_unique<WidgetRecord>());
            auto& record = *app.widget_records.back();
            record.actual = create_native_gui_group_00aa12f0(NativeGuiGroupDefaultInputs{
                app.renderer.node_constants().one_00d7a24c});
            if (!record.actual) return nullptr;
            record.constructor_returned = true;
            record.reference = std::make_unique<NativeGuiGroupReference>(record.actual, app.widget_identity);
            for (auto* f = app.current_properties; f; f = f->suspended) {
                app.prepare_calls(f->native_calls);
                if (f->suspended_calls) app.prepare_calls(*f->suspended_calls);
            }
            return record.actual;
        }
        void widget_created_74(void* widget, Word target) override {
            (void)app.live_widget(widget);
            if (target != 0x00a9ac00) reject("Group current74", target);
            native_gui_widget_base_post_construct_00a9ac00(widget, nullptr);
        }
        void enter_key_04(void* visitor, Word target, Word tag, Word payload) override {
            if (target != 0x00bd8e20) reject("reader enter+04", target);
            auto& f = app.reader_invocation(visitor);
            f.reader->enter_key_00bd8e20(f.enter, tag, payload);
        }
        void read_widget_18(void* widget, Word target, void* visitor) override {
            if (target != 0x00ac6fd0) reject("Group current18", target);
            (void)app.reader_invocation(visitor);
            app.read_group_properties(widget, *static_cast<NativeLuaReaderStorage*>(visitor));
        }
        void leave_key_08(void* visitor, Word target) override {
            if (target != 0x00bd7a20) reject("reader leave+08", target);
            app.reader_invocation(visitor).reader->pop_back_00bd7130();
        }
        void widget_loaded_78(void* widget, Word target) override {
            auto& record = app.live_widget(widget);
            if (target != 0x00aa7170) reject("Group current78", target);
            native_gui_widget_loaded_00aa7170(widget, *this, record.loaded_bounds,
                scalar<double>(app.data, 0x00d7a280));
        }
        void call_active_60(Word target, void*, std::uint8_t) override { reject("Group current60", target); }
    };
    struct RawTimedEntries {
        Impl& app;
        enum class Phase { allocated, alpha, section, base, destroying, consumed, failed };
        struct Entry { void* widget{}; void* actual{}; Phase phase{Phase::allocated}; };
        struct Backing { void* header{}; void* actual{}; bool acquired{}, consumed{}; };
        std::list<Entry> entries;
        std::list<Backing> backing;
        struct Invocation final : cc13_gui1::GuiTimedEntryHooks {
            RawTimedEntries& service;
            void* const widget;
            const std::int32_t index;
            Entry* pending{};
            bool entered{}, complete{}, failed{};
            Invocation(RawTimedEntries& s, void* w, std::int32_t i) : service(s), widget(w), index(i) {}
            void timed_entry_array_resize_00aa77b0(void* header, std::int32_t count) override {
                require(header == at(widget, 0x88), "timed producer requires SAME actual widget header");
                resize_native_gui_timed_pointers_00aa77b0(header, count, service.app.widgets);
            }
            void* operator_new_00bf681b(Word bytes) override {
                require(bytes == 0x14 && !pending, "timed producer requires one exact14h allocation");
                service.entries.emplace_back();
                pending = &service.entries.back(); pending->widget = widget;
                pending->actual = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
                if (pending->actual) ::new (pending->actual) NativeGuiTimedEntryStorage;
                return pending->actual;
            }
            void* timed_entry_construct_00ac2ed0(void* block, void* actual_widget) override {
                require(pending && pending->actual == block && actual_widget == widget,
                    "timed constructor requires its SAME captured allocation/widget");
                return construct_native_gui_timed_entry_alpha_00ac2ed0(block, actual_widget,
                    scalar<float>(service.app.data, 0x00d7a24c));
            }
            void free_failed_constructor_00bf65ac(void* captured) noexcept override {
                if (!pending || pending->actual != captured || pending->phase != Phase::allocated)
                    std::terminate();
                pending->phase = Phase::consumed;
                singleton_lifetime_free(captured);
            }
            void timed_entry_construction_returned(void* actual) noexcept override {
                if (!pending || pending->actual != actual || pending->phase != Phase::allocated)
                    std::terminate();
                pending->phase = index == 2 ? Phase::section : Phase::alpha;
            }
        };
        std::list<Invocation> invocations;
        explicit RawTimedEntries(Impl& value) : app(value) {}
        Entry* find(void* actual) noexcept {
            for (auto& entry : entries)
                if (entry.actual == actual && entry.phase != Phase::consumed) return &entry;
            return nullptr;
        }
        const volatile Word* current_table(void* actual, Word profile) {
            auto* entry = find(actual);
            if (!entry) return nullptr;
            app.authenticate_widget(entry->widget);
            const Word expected = entry->phase == Phase::alpha ? 0x00d5ca74 :
                entry->phase == Phase::section ? 0x00d5d210 : entry->phase == Phase::base ? 0x00d5d204 : 0;
            require(expected && profile == expected, "raw timed lookup requires its captured genuine live family");
            return table(app.data, profile, 4);
        }
        static void free_entry(void* context, void* actual) noexcept {
            auto& entry = *static_cast<Entry*>(context);
            if (entry.actual != actual || entry.phase != Phase::destroying) std::terminate();
            entry.phase = Phase::consumed; // consume provenance immediately BEFORE actual free
            singleton_lifetime_free(actual);
        }
        void scalar_delete(void* actual, Word target, Word flags) {
            auto* entry = find(actual);
            require(entry != nullptr, "timed scalar requires independent SAME-allocation custody");
            app.authenticate_widget(entry->widget);
            const Phase family = entry->phase;
            require((family == Phase::alpha && target == 0x00ac2fe0) ||
                (family == Phase::section && target == 0x00ad3a40) ||
                (family == Phase::base && target == 0x00ad3a60), "raw timed scalar target/family mismatch");
            entry->phase = Phase::destroying;
            try {
                if (family == Phase::alpha)
                    scalar_delete_native_gui_timed_entry_alpha_00ac2fe0(actual, flags, entry, &free_entry);
                else if (family == Phase::section)
                    scalar_delete_native_gui_timed_entry_section_00ad3a40(actual, flags, entry, &free_entry);
                else scalar_delete_native_gui_timed_entry_base_00ad3a60(actual, flags, entry, &free_entry);
                if (entry->phase != Phase::consumed) entry->phase = Phase::base;
            } catch (...) { entry->phase = Phase::failed; app.fail(target); throw; }
        }
        void* allocate_backing(void* header, Word bytes) {
            app.authenticate_widget(static_cast<unsigned char*>(header) - 0x88);
            backing.emplace_back();
            auto& record = backing.back(); record.header = header;
            record.actual = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
            record.acquired = true;
            return record.actual;
        }
        void free_backing(void* header, void* actual) {
            app.authenticate_widget(static_cast<unsigned char*>(header) - 0x88);
            if (!actual) { singleton_lifetime_free(nullptr); return; }
            for (auto& record : backing) {
                if (record.header == header && record.actual == actual && record.acquired && !record.consumed) {
                    record.consumed = true;
                    singleton_lifetime_free(actual);
                    return;
                }
            }
            throw std::logic_error("raw timed backing free lacks SAME-header/allocation custody");
        }
        void* get_or_create(void* widget, std::int32_t index) {
            app.authenticate_widget(widget);
            invocations.emplace_back(*this, widget, index);
            auto& invocation = invocations.back(); invocation.entered = true;
            try {
                void* result = cc13_gui1::gui_timed_entry_get_or_create_00aa8b00(invocation, widget, index);
                invocation.complete = true; return result;
            } catch (...) { invocation.failed = true; app.fail(0x00aa8b00); throw; }
        }
        bool empty() const noexcept {
            for (const auto& entry : entries) if (entry.actual && entry.phase != Phase::consumed) return false;
            for (const auto& record : backing) if (record.acquired && record.actual && !record.consumed) return false;
            for (const auto& invocation : invocations) if (invocation.failed || (invocation.entered && !invocation.complete)) return false;
            return true;
        }
    };
    // Concrete Page and default GuiGroup families share whole native lifetime
    // schedules. Raw nodes and timed allocations retain separate real custody;
    // neither becomes another counted widget/model companion.
    struct Widgets final : NativeGuiWidgetLifetimeBindings, NativeGuiWidgetVisibilityBindings,
        NativeGuiWidgetPropertyWidgetDispatch, NativeGuiWidgetTransformDispatch,
        NativeGuiWidgetRawNodeLifetimeBindings, NativeGuiTimedStorageCalls {
        Impl& app;
        explicit Widgets(Impl& value) : app(value) {}
        const volatile Word* current_table(void* actual, Word profile) override {
            if (const auto* timed = app.timed.current_table(actual, profile)) return timed;
            app.authenticate_widget(actual);
            if (app.is_page_widget(actual)) {
                if (profile != kPageProfile && profile != 0x00d5c130) reject("Page widget profile", profile);
            } else if (profile != 0x00d5cb80 && profile != 0x00d5c130) reject("Group widget profile", profile);
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
            app.authenticate_node(actual);
            require(app.find_root(actual) || app.find_model(actual),
                "widget transform requires its SAME raw renderer root or canonical Model");
            set_raw_local_matrix_00b6db10(actual, app.transform_profiles, matrix);
        }
        bool call_class_query_0c(void* actual, Word target, Word descriptor) override {
            app.authenticate_widget(actual);
            if (app.is_page_widget(actual) && target == 0x00aa38b0)
                return gui_widgets_cc13::native_gui_layer_is_kind_of_00aa38b0(
                    game_native_gui_text_process().layer_lineage_00f8bf88(), descriptor);
            if (!app.is_page_widget(actual) && target == 0x00ac6f70)
                return native_gui_group_is_kind_of_00ac6f70(descriptor,
                    game_native_gui_text_process().group_lineage_00f8bfd8());
            reject("Page/Group class+0C", target);
        }
        void call_child_scene_20(void*, Word target) override { reject("child scene+20", target); }
        void call_child_delete_04(void* actual, Word target, Word flags) override {
            auto& record = app.live_widget(actual);
            if (target != 0x00ac73e0) reject("Group child delete+04", target);
            auto& call = app.calls().scalar(); call.flags = flags; call.entered = true;
            record.reference->delete_scalar_00ac73e0(call.flags);
            call.complete = true;
            record.native_retired = true; // no read of returned native slot
        }
        void call_timed_delete_00(void* actual, Word target, Word flags) override {
            app.timed.scalar_delete(actual, target, flags);
        }
        void remove_all_00a9bd50(void* actual_list, const void* cell) override {
            remove_native_gui_widget_list_value_00a9bd50(
                *static_cast<NativeEffectDeletionListStorage*>(actual_list), cell);
        }
        void begin_raw_widget_scene(void* actual) override {
            app.authenticate_widget(actual); (void)app.calls();
        }
        void finish_raw_widget_scene() override {}
        void release_raw_widget_node(void* actual, bool late) override {
            app.authenticate_node(actual);
            auto& call = app.calls().tree(); call.late_primary = late;
            call.actual = actual; call.target = 0x00b6dfa0;
            try { unlink_native_node_tree_00b6dfa0(actual, app.group_trees, call.acquired); }
            catch (...) { call.exception = std::current_exception(); throw; }
        }
        void detach_raw_widget_parent(void* actual, void* parent) override {
            app.authenticate_widget(actual); app.authenticate_widget(parent);
            auto& call = app.calls().parent();
            detach_native_gui_widget_child_00aa83a0(parent, actual, call.frame, app.group_node, call.acquired);
        }
        void raw_widget_lifetime_failed() noexcept override {
            if (app.current_calls) app.current_calls->failed = true;
            app.fail(0x00aa9730);
        }
        void* allocate_timed_pointer_slots_00bf681b(void* header, Word bytes) override {
            return app.timed.allocate_backing(header, bytes);
        }
        void free_timed_pointer_slots_00bf65ac(void* header, void* backing) override {
            app.timed.free_backing(header, backing);
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
    Roots roots;
    SceneTypes scene_types;
    NativeSceneRegistrationGateContext group_gates;
    NativeNodeSceneAttachmentContext group_scenes;
    GroupTrees group_dispatch;
    NativeNodeTreeRetirementContext group_trees;
    NativeNodeBaseDestructionContext group_node;
    NativeGroupStorageIdentityContext group_identity;
    NativeNodeParentingContext parenting;
    NativeModelStorageContext model_storage;
    std::unique_ptr<GameNativeModelClient> models;
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
    ChildCalls child_calls;
    RawTimedEntries timed;
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
    NativeGuiScriptPathContext paths;
    // Published only after the actual raw caller/provider domain is prepared.
    bool properties_composed{};
    int layer_atexit{};
    // Every page record, including one an entered failure retains.
    std::vector<std::unique_ptr<Page>> pages;
    std::vector<std::unique_ptr<WidgetRecord>> widget_records;
    NativeGuiGroupIdentityContext widget_identity;
    // Entered failures: retained, never destroyed (the host _Exits first).
    std::vector<std::unique_ptr<Construction>> retained_frames;
    std::unique_ptr<GameNativeGuiManagerApplication::PageLifetimeBinding> manager_binding;
    PropertyInvocation* current_properties{};
    NativeCalls* current_calls{};
    bool scene_hooks_bound{}, retiring_page{};
    bool active{};
    std::uint32_t failed_site{};
    std::exception_ptr failed;

    Impl(GameHostLog& output, GameNativeGuiManagerApplication& gui, GameNativeRendererApplication& app,
        GameNativeModelDomain& model_domain, GameVfsHost& vfs, GameNativeReadOnlyData& mapped,
        const ObjectHandleResolverSlots& slots, const bool& sse2_conversion)
        : log(output), manager(gui), renderer(app), data(mapped),
          installed_resolvers(slots), crt_sse2_conversion(sse2_conversion), raw(vfs.borrow_raw_services()),
          strings(vfs.raw_strings()), scene_domain(app.startup_scene_domain()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          roots(*this), scene_types(*this),
          group_gates{scene_domain.node.scenes.gates.registry, scene_domain.node.scenes.gates.light_types,
              scene_domain.node.scenes.gates.camera_types, scene_domain.node.scenes.gates.actual_node_profile_00d62c88,
              scene_domain.node.scenes.gates.actual_camera_profile_00d62cf0,
              scene_domain.node.scenes.gates.actual_light_profile_00d62f58,
              scene_domain.node.scenes.gates.actual_directional_profile_00d62fb0, &scene_types},
          group_scenes{group_gates, app.actual_owners(), scene_domain.node.scenes.increment_00ce221c,
              scene_domain.node.scenes.decrement_00ce2220, roots},
          group_dispatch(*this),
          group_trees{app.camera_environment().nodes.point_lights, app.actual_owners(),
              scene_domain.node.trees.decrement_00ce2220, group_dispatch, group_dispatch},
          group_node{group_scenes, group_trees, strings, scene_domain.node.world},
          group_identity{group_node, app.actual_owners(),
              game_native_resource_pool_process().group_pool_010902f4(), table(mapped, kGroupProfile, 0x58)},
          parenting{group_node, scene_types, group_types.storage().group_0109032c.own_id},
          model_storage{group_node, game_native_resource_pool_process().model_pool_01090054(),
              app.node_constants(), word(mapped, 0x00d7a208)},
          models(model_domain.make_page_client(model_storage)),
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
          fields(*this), child_calls(*this), timed(*this), widgets(*this),
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
          page_lifetime(*this), lookup(mapped), properties(*this),
          paths{vfs.raw_strings(), literal(mapped, 0x00d5cb18, 11), literal(mapped, 0x00cfd2c8, 5)},
          widget_identity{app.actual_owners(), widget_lifetime, table(mapped, 0x00d5cb80, 0x80)} {
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
    void read_group_properties(void*, NativeLuaReaderStorage&);
    NativeCalls& calls() { require(current_calls != nullptr, "Page raw call has no retained invocation backing"); return *current_calls; }
    void prepare_calls(NativeCalls&);
    Page* find_root(void*);
    WidgetRecord& live_widget(void*);
    NativeModelReference* find_model(void*);
    void authenticate_node(void*);
    bool is_page_widget(void*) const noexcept;
    void authenticate_widget(void*);
    PropertyInvocation& reader_invocation(void*);
    NativeGuiWidgetPropertyInteractionContext interaction(PropertyInvocation&);
    NativeGuiWidgetPropertyChildrenRawContext children(PropertyInvocation&);
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

bool GameNativeGuiPageApplication::Impl::is_page_widget(void* actual) const noexcept {
    for (const auto& page : pages) if (page->page == actual && !page->native_retired) return true;
    return false;
}
GameNativeGuiPageApplication::Impl::WidgetRecord& GameNativeGuiPageApplication::Impl::live_widget(void* actual) {
    for (auto& record : widget_records) {
        if (record->actual != actual || !record->constructor_returned || record->native_retired) continue;
        require(record->reference && !record->reference->retired() &&
            renderer.actual_owners().find(actual) == record->reference.get(),
            "GuiGroup requires its SAME canonical live actual+04 owner");
        return *record;
    }
    throw std::logic_error("raw GuiGroup has no same retained completed allocation");
}
void GameNativeGuiPageApplication::Impl::authenticate_widget(void* actual) {
    if (is_page_widget(actual)) return;
    (void)live_widget(actual);
}
GameNativeGuiPageApplication::Impl::Page* GameNativeGuiPageApplication::Impl::find_root(void* actual) {
    for (auto& page : pages)
        if (page->group_slot == actual && page->reference && !page->reference->retired() &&
            renderer.actual_owners().find(actual) == page->reference.get()) return page.get();
    return nullptr;
}
NativeModelReference* GameNativeGuiPageApplication::Impl::find_model(void* actual) {
    auto* reference = dynamic_cast<NativeModelReference*>(renderer.actual_owners().find(actual));
    if (!reference) return nullptr;
    auto& owner = reference->model_owner();
    require(&owner.storage.node == actual && &owner.environment == &models->raw_access().models &&
        owner.raw_lifetime && &owner.raw_lifetime->storage == &model_storage &&
        !owner.raw_lifetime->host.typed_storage_ended,
        "Page raw Model requires the SAME client/immutable lifetime/canonical owner");
    return reference;
}
void GameNativeGuiPageApplication::Impl::authenticate_node(void* actual) {
    if (find_root(actual) || find_model(actual)) return;
    for (auto& page : pages) {
        if (!page->properties) continue;
        auto& scene = page->properties->acquired.scene;
        auto admitted = [&](void* identity, auto& reference) {
            return identity == actual && reference && !reference->retired() &&
                renderer.actual_owners().find(actual) == &*reference;
        };
        if (admitted(scene.camera_result, scene.camera_reference) ||
            admitted(scene.directional_result, scene.directional_reference)) return;
    }
    throw std::logic_error("raw Page node has no authenticated reached family/extent");
}
void GameNativeGuiPageApplication::Impl::prepare_calls(NativeCalls& backing) {
    require(pages.size() <= (std::numeric_limits<std::size_t>::max() - widget_records.size() - 1) / 4,
        "Page producer graph extent overflow");
    backing.prepare(pages.size() * 4 + widget_records.size() + 1);
}
GameNativeGuiPageApplication::Impl::PropertyInvocation&
GameNativeGuiPageApplication::Impl::reader_invocation(void* visitor) {
    require(current_properties && current_properties->reader == visitor &&
        current_calls == &current_properties->native_calls && read(visitor) == kReaderProfile,
        "raw reader callback requires its SAME active retained recursion record");
    return *current_properties;
}
NativeGuiWidgetPropertyInteractionContext GameNativeGuiPageApplication::Impl::interaction(PropertyInvocation& f) {
    return {widget_properties, literal(data, 0x00d5c1e4, 8), literal(data, 0x00d5c1d8, 11),
        literal(data, 0x00d5c1cc, 9), widgets, visibility, f.transform, transform};
}
NativeGuiWidgetPropertyChildrenRawContext GameNativeGuiPageApplication::Impl::children(PropertyInvocation& f) {
    return {strings, key_types, models->raw_access(), renderer.node_constants(), parenting,
        f.parent_frame, f.parent_acquired, f.detach_frame, f.detach_acquired, child_calls};
}
void GameNativeGuiPageApplication::Impl::read_properties(void* actual, NativeLuaReaderStorage& reader) {
    auto& page = live_page(actual);
    require(properties_composed && !current_properties && !page.properties && read(actual, 0x60) == 1,
        "AC4C50 requires a fresh actual type1 Page and retained invocation");
    page.properties = std::make_unique<PropertyInvocation>();
    auto& f = *page.properties;
    prepare_calls(f.native_calls);
    f.scenes.prepare(pages.size() * 4 + widget_records.size() + 1);
    f.interaction_context.emplace(interaction(f));
    f.children_context.emplace(children(f));
    f.page_context.emplace(NativeGuiPageReadPropertiesRawContext{data, scenes,
        *f.interaction_context, *f.children_context, scalar<double>(data, 0x00d7a280)});
    f.reader = &reader; f.actual_widget = actual; f.suspended_calls = current_calls;
    current_properties = &f; current_calls = &f.native_calls;
    try {
        read_native_gui_page_properties_00ac4c50(actual, &reader, f.page_frame, *f.page_context, f.acquired);
        f.reader = nullptr;
        current_properties = nullptr; current_calls = f.suspended_calls;
    } catch (...) {
        f.native_calls.failed = true;
        fail(f.acquired.children.phase == NativeGuiWidgetPropertyChildrenAcquired::Phase::failed ?
            f.acquired.children.active_call_site : f.acquired.active_call_site);
        throw;
    }
}
void GameNativeGuiPageApplication::Impl::read_group_properties(void* actual, NativeLuaReaderStorage& reader) {
    auto& record = live_widget(actual);
    require(properties_composed && current_properties && current_properties->reader == &reader &&
        !record.properties && read(actual) == 0x00d5cb80,
        "AC6FD0 requires a fresh retained GuiGroup invocation on SAME suspended reader");
    record.properties = std::make_unique<PropertyInvocation>();
    auto& f = *record.properties;
    prepare_calls(f.native_calls);
    f.interaction_context.emplace(interaction(f)); f.children_context.emplace(children(f));
    f.reader = &reader; f.actual_widget = actual;
    f.suspended = current_properties; f.suspended_calls = current_calls;
    current_properties = &f; current_calls = &f.native_calls;
    f.acquired.phase = NativeGuiPageReadPropertiesAcquired::Phase::entered;
    f.acquired.active_call_site = 0x00ac6fd0;
    try {
        read_native_gui_widget_properties_and_children_00aaa710(actual, &reader, f.widget,
            *f.interaction_context, *f.children_context, f.acquired.children);
        f.acquired.properties_returned = true;
        f.acquired.phase = NativeGuiPageReadPropertiesAcquired::Phase::complete;
        f.reader = nullptr;
        current_properties = f.suspended; current_calls = f.suspended_calls;
    } catch (...) {
        f.acquired.phase = NativeGuiPageReadPropertiesAcquired::Phase::failed;
        f.native_calls.failed = true;
        fail(f.acquired.children.active_call_site ? f.acquired.children.active_call_site : f.acquired.active_call_site);
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
    NativeCalls* backing{};
    retiring_page = true;
    try {
        auto& page = live_page(actual);
        page.lifetime_calls.push_back(std::make_unique<NativeCalls>());
        backing = page.lifetime_calls.back().get();
        prepare_calls(*backing);
        if (page.properties) page.properties->scenes.prepare(pages.size() * 4 + widget_records.size() + 1);
        current_calls = backing;
        require(read(actual) == kPageProfile && table(data, kPageProfile, 0x24)[8] == 0x00aa8320,
            "Page retirement requires genuine current20 AA8320");
        release_native_gui_widget_scene_00aa8320(actual, widget_lifetime);
        page.scene_released = true;
        current_calls = nullptr; retiring_page = false;
    } catch (...) {
        if (backing) backing->failed = true;
        fail(0x00aa8320); throw;
    }
}
void GameNativeGuiPageApplication::Impl::delete_page(void* actual, Word flags) {
    require_entry();
    NativeCalls* backing{};
    retiring_page = true;
    try {
        auto& page = live_page(actual);
        page.lifetime_calls.push_back(std::make_unique<NativeCalls>());
        backing = page.lifetime_calls.back().get();
        prepare_calls(*backing);
        if (page.properties) page.properties->scenes.prepare(pages.size() * 4 + widget_records.size() + 1);
        auto& scalar = backing->scalar(); scalar.flags = flags;
        current_calls = backing;
        require(read(actual) == kPageProfile && table(data, kPageProfile, 8)[1] == 0x00aa38f0,
            "Page scalar requires genuine current04 AA38F0");
        scalar.entered = true;
        scalar_delete_native_gui_layer_00aa38f0(actual, scalar.flags, layer_lifetime);
        scalar.complete = true;
        page.native_retired = true; // metadata only after possible pool return
        current_calls = nullptr; retiring_page = false;
    } catch (...) {
        if (backing) backing->failed = true;
        fail(0x00aa38f0); throw;
    }
}

void GameNativeGuiPageApplication::Impl::bind_providers() {
    require(!properties_composed && !scene_hooks_bound && !manager_binding && pages.empty(),
        "Page providers bind once before a native page exists");
    (void)warm_manager();
    require(dynamic_cast<NativeGuiWidgetRawNodeLifetimeBindings*>(&widget_lifetime.bindings) ==
        static_cast<NativeGuiWidgetRawNodeLifetimeBindings*>(&widgets) &&
        dynamic_cast<NativeGuiTimedStorageCalls*>(&widget_lifetime.bindings) ==
        static_cast<NativeGuiTimedStorageCalls*>(&widgets),
        "raw Page admission requires its SAME node and timed custody service");
    renderer.bind_startup_scene_child_delegate(&roots); // Throws before publication on conflict.
    scene_hooks_bound = true;
    // The unique_ptr-owned complete Impl restores the child metadata hook if
    // token allocation rejects. Its derived gate is private, never published
    // into the frozen A1 context. No throwing construction/logging follows.
    manager_binding = manager.bind_page_lifetime_host(page_lifetime);
    properties_composed = true;
}

GameNativeGuiPageApplication::Impl::~Impl() noexcept {
    if (active || failed || retiring_page || current_properties || current_calls ||
        resolvers.requires_process_retention() || !timed.empty() || !models->empty()) std::terminate();
    for (const auto& widget : widget_records)
        if (widget->actual && (!widget->native_retired || !widget->reference || !widget->reference->retired()))
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
    GameNativeModelDomain& models, GameVfsHost& files, GameNativeReadOnlyData& data,
    const ObjectHandleResolverSlots& resolvers, const bool& sse2_conversion)
    : impl_(std::make_unique<Impl>(log, manager, renderer, models, files, data, resolvers, sse2_conversion)) {
    log.notef("native GUI Page chain prepared: layer_pool=00f8bf50 atexit=%d group_pool=010902f4 "
        "properties=ac4c50 raw_type2=default-group guard=off", impl_->layer_atexit);
    impl_->bind_providers(); // Last throwing step: complete Impl owns rollback/retention.
}

GameNativeGuiPageApplication::~GameNativeGuiPageApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}

bool GameNativeGuiPageApplication::requires_process_retention() const noexcept {
    return impl_->failed != nullptr || impl_->active || impl_->retiring_page ||
        impl_->current_properties || impl_->current_calls || impl_->resolvers.requires_process_retention() ||
        impl_->models->requires_process_retention();
}
std::uint32_t GameNativeGuiPageApplication::failure_site() const noexcept { return impl_->failed_site; }
void GameNativeGuiPageApplication::after_native_renderer_drain() {
    require(!requires_process_retention() && impl_->timed.empty(),
        "Page raw graph retains entered failures or independent timed custody after drain");
    for (const auto& page : impl_->pages)
        require(page->native_retired && (!page->reference || page->reference->retired()),
            "Page raw Page/root ownership survived native drain");
    for (const auto& widget : impl_->widget_records)
        require(!widget->actual || (widget->native_retired && widget->reference && widget->reference->retired()),
            "Page raw GuiGroup ownership survived native drain");
    impl_->models->after_native_renderer_drain();
}

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
        p.prepare_calls(record->construction_calls);
        p.current_calls = &record->construction_calls;
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
        p.current_calls = nullptr;
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
