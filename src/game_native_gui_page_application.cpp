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
#include "bsp/native_gui_layer_storage.hpp"
#include "bsp/native_gui_page_construction.hpp"
#include "bsp/native_gui_page_properties_continuation.hpp"
#include "bsp/native_gui_page_reader_continuation.hpp"
#include "bsp/native_gui_page_registry.hpp"
#include "bsp/native_gui_page_script_continuation.hpp"
#include "bsp/native_gui_script_paths.hpp"
#include "bsp/native_node_scene_attachment.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include <cstddef>
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
    // B6D890 from AC6806 on the page's raw Group root, null scene, recurse 0.
    struct Roots final : NativeNodeSceneChildDispatch {
        Impl& app;
        explicit Roots(Impl& value) : app(value) {}
        const volatile Word* resolve_profile(Word profile) const override {
            require(profile == kGroupProfile, "native GUI page root admits only its raw Group profile");
            return app.environment.vtable_00d634f8;
        }
        void invoke_child(Word target, void* actual, Word requested, Word recursion) override {
            require(target == 0x00b6ed80 && !requested && !recursion,
                "native GUI page root requires the genuine null-lighting B6ED80 attachment");
            for (auto& page : app.pages) {
                if (!page->reference || &page->group->storage.node != actual) continue;
                auto& reference = *page->reference;
                require(page->group->phase == NativeGroupOwner::Phase::live &&
                    app.renderer.actual_owners().find(actual) == &reference,
                    "native GUI page root lost its same canonical Group");
                auto& attachment = reference.scene_attachment();
                require(attachment.attach_scene && !attachment.scene,
                    "native GUI page Group cannot substitute a logical lighting owner");
                attachment.attach_scene(app.environment.nodes.scenes, attachment, nullptr, false);
                return;
            }
            throw std::logic_error("native GUI page root has no child below its Group before P3b/P3c");
        }
    };
    // The page's raw Group (manager-owned through page+4C) and its canonical
    // companion. Construction frames live only until AC6806 returns.
    struct Page {
        Impl* app{};
        void* group_slot{};
        std::unique_ptr<NativeGroupOwner> group;
        std::unique_ptr<NativeGroupReference> reference;
        NativeRenderActualOwnerRegistry::BindingAdmission admission;
        bool registered{}, retired{};
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
    GameNativeVfsRawServices raw;
    NativeStringRawPoolContext& strings;
    LightTypeBootstrap node_types;
    NativeGroupTypes group_types;
    NativeGroupEnvironment environment;
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
        GameVfsHost& vfs, GameNativeReadOnlyData& mapped)
        : log(output), manager(gui), renderer(app), data(mapped), raw(vfs.borrow_raw_services()),
          strings(vfs.raw_strings()),
          node_types(vfs.native_owners().types(), vfs.native_types().light_types()),
          group_types(vfs.native_owners().types(), node_types, vfs.native_types().group_types()),
          environment{game_native_resource_pool_process().group_pool_010902f4(),
              app.camera_environment().nodes, group_types,
              {app.node_constants().one_00d7a24c, app.node_constants().positive_bound_00ce4970},
              table(mapped, kGroupProfile, 0x58), table(mapped, 0x00d62c88, 0x58), &app.node_constants()},
          lookup(mapped), properties(mapped), roots(*this),
          paths{vfs.raw_strings(), literal(mapped, 0x00d5cb18, 11), literal(mapped, 0x00cfd2c8, 5)} {
        auto& node_names = environment.nodes.require_raw_name_pool();
        require(&node_names.actual_published_01090aa8 == &strings.actual_published_01090aa8 &&
            &node_names.actual_manager_publication_01090aa0 == &strings.actual_manager_publication_01090aa0 &&
            environment.vtable_00d62c88 == app.camera_environment().vtable_00d62c88,
            "native GUI pages require the same canonical node/string/table domains");
        layer_atexit = game_native_gui_layer_pool_process().initialize_once_00cd73d0();
    }
    void require_entry() const {
        require(!active && !failed, "native GUI page chain cannot reenter or replay interrupted work");
    }
    void fail(std::uint32_t site) noexcept { failed_site = site; failed = std::current_exception(); }
    static void retire(void* context, NativeGroupReference& same_reference) noexcept {
        auto& app = *static_cast<Impl*>(context);
        for (auto& page : app.pages) {
            if (page->reference.get() != &same_reference) continue;
            if (!page->registered || page->group_slot != &same_reference.group_owner().storage.node)
                std::terminate();
            app.renderer.actual_owners().unbind(page->group_slot, same_reference);
            page->registered = false;
            page->retired = true;
            return;
        }
        std::terminate();
    }
    void release(NativeString& header, bool& live) {
        live = false;
        destroy_native_string_header_0041dd20(&header, strings);
    }
    // B8F450 + B8F5E0(name), then the canonical companion for B6D890.
    void create_group(Page& page, const void* name) {
        page.group_slot = allocate_native_group_slot_00b8f450();                    // 00AA58FF
        require(page.group_slot != nullptr, "native GUI page Group pool returned null");
        page.admission = renderer.actual_owners().prepare_binding(page.group_slot);
        page.group = std::make_unique<NativeGroupOwner>(page.group_slot, NativeGroupPool::slot_bytes, environment);
        construct_native_group_00b8f5e0(*page.group, name, *environment.raw_node_constants);  // 00AA5914
        page.reference = std::make_unique<NativeGroupReference>(*page.group,
            NativeGroupCompanionDisposal{this, &Impl::retire});
        renderer.actual_owners().publish(std::move(page.admission), *page.reference);
        page.registered = true;
    }
};

GameNativeGuiPageApplication::GameNativeGuiPageApplication(GameHostLog& log,
    GameNativeGuiManagerApplication& manager, GameNativeRendererApplication& renderer,
    GameVfsHost& files, GameNativeReadOnlyData& data)
    : impl_(std::make_unique<Impl>(log, manager, renderer, files, data)) {
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

    p.active = true;
    auto frames = std::make_unique<Impl::Construction>();
    auto& f = *frames;
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
        initialize_native_gui_page_00ac6600_prefix(record->page, name, &record->group->storage.node,
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
        frames.reset();
        p.active = false;
        p.log.notef("native GUI page loaded: page=%p group=%p manager=%p flag=%d",
            page, record->group_slot, actual_manager, screen_flag);
        return page;
    } catch (...) {
        if (!p.active) throw; // model-route rejection consumed both headers
        p.fail(site);
        p.retained_frames.push_back(std::move(frames));
        p.active = false;
        throw;
    }
}
} // namespace bsp::game
