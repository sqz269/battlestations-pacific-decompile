#include "bsp/game_native_gui_page_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
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
#include "bsp/native_gui_page_reader_continuation.hpp"
#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_gui_page_script_continuation.hpp"
#include "bsp/native_gui_script_paths.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/object_handle_resolvers.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <cstddef>
#include <array>
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
    // AC6806's CURRENT Page+18 and reader+08. Page+18 (AC4C50) is P3a2.
    struct Properties final : NativeGuiPagePropertiesBindings {
        GameNativeReadOnlyData& data;
        explicit Properties(GameNativeReadOnlyData& value) : data(value) {}
        const volatile Word* current_page_table(void*, Word profile) override {
            require(profile == kPageProfile, "native GUI page properties admit only the AC6600 page profile");
            return table(data, profile, 0x80);
        }
        const volatile Word* current_reader_table(NativeLuaReaderStorage&, Word profile) override {
            require(profile == kReaderProfile, "native GUI page properties admit only the CE44FC reader");
            return table(data, profile, 0x10);
        }
        void call_page_properties_18(void*, Word, NativeLuaReaderStorage&) override {
            throw std::logic_error("native GUI Page+18 (AC4C50) is not composed before P3a2");
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
            if (target != 0x00b8eec0) {
                app.scene_domain.node.trees.dispatch.invoke_virtual18(target, actual);
                return;
            }
            for (auto& page : app.pages) {
                if (page->group_slot != actual || !page->reference || page->reference->retired()) continue;
                require(app.renderer.actual_owners().find(actual) == page->reference.get(),
                    "raw GUI Group release requires its same canonical owner");
                release_native_group_storage_00b8eec0(actual, page->group_release_frame,
                    app.group_trees, page->group_release);
                return;
            }
            throw std::logic_error("raw GUI Group current18 has no live page-root owner");
        }
        void invoke_virtual0(Word target, void* actual, RenderCommandReference& reference) override {
            for (auto& page : app.pages) {
                if (page->group_slot != actual || page->reference.get() != &reference ||
                    !page->reference || page->reference->retired()) continue;
                require(target == 0x00bd30e0 &&
                    static_cast<void*>(&reference.reference_count) == static_cast<unsigned char*>(actual) + 4 &&
                    reference.reference_count.load(std::memory_order_relaxed) == 0 &&
                    app.renderer.actual_owners().find(actual) == &reference,
                    "raw GUI Group terminal requires the same current zero-count owner");
                reference.release_zero_references();
                return;
            }
            app.scene_domain.node.trees.dispatch.invoke_virtual0(target, actual, reference);
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
                    app.scene_domain.node.scenes, invocation.acquired);
                return;
            }
            throw std::logic_error("native GUI page root has no child below its Group before P3b/P3c");
        }
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
        std::unique_ptr<NativeGroupStorageReference> reference;
        NativeNodeTreeRetirementFrame group_release_frame{};
        NativeGroupStorageReleaseAcquired group_release;
        std::array<RootAttachment, 2> attachments{};
        std::size_t attachment_count{};
        void* page{};
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
    GroupTrees group_dispatch;
    NativeNodeTreeRetirementContext group_trees;
    NativeNodeBaseDestructionContext group_node;
    NativeGroupStorageIdentityContext group_identity;
    Lookup lookup;
    Properties properties;
    Roots roots;
    NativeGuiScriptPathContext paths;
    // AC4C50 is P3a2: a missed lookup rejects before any native effect.
    bool properties_composed{};
    int layer_atexit{};
    // Every page record, including one an entered failure retains.
    std::vector<std::unique_ptr<Page>> pages;
    // Entered failures: retained, never destroyed (the host _Exits first).
    std::vector<std::unique_ptr<Construction>> retained_frames;
    bool active{};
    std::uint32_t failed_site{};
    std::exception_ptr failed;

    Impl(GameHostLog& output, GameNativeGuiManagerApplication& gui, GameNativeRendererApplication& app,
        GameVfsHost& vfs, GameNativeReadOnlyData& mapped,
        const ObjectHandleResolverSlots& resolvers, const bool& sse2_conversion)
        : log(output), manager(gui), renderer(app), data(mapped),
          installed_resolvers(resolvers), crt_sse2_conversion(sse2_conversion), raw(vfs.borrow_raw_services()),
          strings(vfs.raw_strings()), scene_domain(app.startup_scene_domain()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          group_dispatch(*this),
          group_trees{scene_domain.node.trees.point_lights, app.actual_owners(),
              scene_domain.node.trees.decrement_00ce2220, group_dispatch, scene_domain.node.trees.frames},
          group_node{scene_domain.node.scenes, group_trees, strings, scene_domain.node.world},
          group_identity{group_node, app.actual_owners(),
              game_native_resource_pool_process().group_pool_010902f4(), table(mapped, kGroupProfile, 0x58)},
          lookup(mapped), properties(mapped), roots(*this),
          paths{vfs.raw_strings(), literal(mapped, 0x00d5cb18, 11), literal(mapped, 0x00cfd2c8, 5)} {
        auto& node_names = scene_domain.node.strings;
        require(&node_names.actual_published_01090aa8 == &strings.actual_published_01090aa8 &&
            &node_names.actual_manager_publication_01090aa0 == &strings.actual_manager_publication_01090aa0 &&
            &scene_domain.node.scenes.owners == &app.actual_owners() &&
            &scene_domain.camera.node_constants.one_00d7a24c == &app.node_constants().one_00d7a24c,
            "native GUI pages require the same canonical node/string/table domains");
        layer_atexit = game_native_gui_layer_pool_process().initialize_once_00cd73d0();
    }
    void require_entry() const {
        require(!active && !failed, "native GUI page chain cannot reenter or replay interrupted work");
    }
    void fail(std::uint32_t site) noexcept { failed_site = site; failed = std::current_exception(); }
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
};

GameNativeGuiPageApplication::GameNativeGuiPageApplication(GameHostLog& log,
    GameNativeGuiManagerApplication& manager, GameNativeRendererApplication& renderer,
    GameVfsHost& files, GameNativeReadOnlyData& data,
    const ObjectHandleResolverSlots& resolvers, const bool& sse2_conversion)
    : impl_(std::make_unique<Impl>(log, manager, renderer, files, data, resolvers, sse2_conversion)) {
    log.notef("native GUI page chain bound: layer_pool=00f8bf50 atexit=%d group_pool=010902f4 "
        "properties=%s", impl_->layer_atexit, impl_->properties_composed ? "ac4c50" : "rejected");
}

GameNativeGuiPageApplication::~GameNativeGuiPageApplication() noexcept {
    if (requires_process_retention()) std::terminate();
}

bool GameNativeGuiPageApplication::requires_process_retention() const noexcept {
    return impl_->failed != nullptr || impl_->active;
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
        initialize_native_gui_page_00ac6600_prefix(record->page, name, record->group_slot,
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
