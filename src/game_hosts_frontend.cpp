// bsp_game.exe milestone 2b: phase 7 of application initialize and the title GUI pages.
// See include/bsp/game_hosts_frontend.hpp for the address list and the evidence.
#include "bsp/game_hosts_frontend.hpp"

#include "bsp/d3d9_texture.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_text.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_page_script.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/texture_atlas.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_mounts.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// D3DX_DEFAULT, kept local because the D3DX headers stay private to the implementations
// that need them (bsp/d3d9_texture.hpp forward-declares the image-info tag).
constexpr UINT kD3dxDefault = 0xFFFFFFFFu;

// D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG. Declared locally for the same reason.
constexpr DWORD kD3dxImageFormatPng = 3;
using SaveSurfaceToFile = HRESULT(WINAPI*)(LPCSTR, DWORD, IDirect3DSurface9*,
    const PALETTEENTRY*, const RECT*);

// The same two D3DX9 entry points the font resources use, imported once for the sprite
// bridge. This is executable plumbing, not a native routine.
class D3dxImageImports {
public:
    D3dxImageImports() {
        module_ = LoadLibraryExW(L"d3dx9_40.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
        if (!module_) return;
        FARPROC address = GetProcAddress(module_, "D3DXGetImageInfoFromFileInMemory");
        std::memcpy(&read_info, &address, sizeof(read_info));
        address = GetProcAddress(module_, "D3DXCreateTextureFromFileInMemoryEx");
        std::memcpy(&create, &address, sizeof(create));
        address = GetProcAddress(module_, "D3DXSaveSurfaceToFileA");
        std::memcpy(&save_surface, &address, sizeof(save_surface));
    }
    ~D3dxImageImports() { if (module_) FreeLibrary(module_); }
    D3dxImageImports(const D3dxImageImports&) = delete;
    D3dxImageImports& operator=(const D3dxImageImports&) = delete;
    bool usable() const noexcept { return read_info != nullptr && create != nullptr; }
    ReadImageInfoFromMemory read_info{};
    CreateTextureFromMemory create{};
    SaveSurfaceToFile save_surface{};

private:
    HMODULE module_{};
};

// 00AA2490's suffix table gives every type but Screen, which no key produces.
std::string type_name(GuiWidgetType type) {
    if (type == GuiWidgetType::Screen) return "Screen";
    if (const GuiWidgetClass* entry = gui_widget_class(type)) return std::string(entry->suffix);
    return "None";
}

// Authored `States` entries carry the texture name. The array form is what every shipped
// page uses; the named form is accepted because the snapshot preserves both.
std::string first_state_texture(const GuiTable& table) {
    const GuiValue* states = table.find("States");
    if (states == nullptr || !states->is_table()) return {};
    const GuiTable* list = states->table();
    for (const GuiValue& entry : list->array) {
        if (!entry.is_table()) continue;
        if (const GuiValue* texture = entry.table()->find("Texture")) {
            if (texture->kind() == GuiValue::Kind::String) return texture->string();
        }
    }
    for (const auto& entry : list->named) {
        if (!entry.second.is_table()) continue;
        if (const GuiValue* texture = entry.second.table()->find("Texture")) {
            if (texture->kind() == GuiValue::Kind::String) return texture->string();
        }
    }
    return {};
}

std::string string_key(const GuiTable& table, const char* key) {
    const GuiValue* value = table.find(key);
    if (value != nullptr && value->kind() == GuiValue::Kind::String) return value->string();
    return {};
}

// resolved_position (00AA6750) over the loader's own parent links: the transform's +70h
// chain is the scene widget's, and a diagnostic tree does not build one.
GuiWidgetPoint resolved_point(const GuiLayoutWidget& widget) {
    if (widget.parent == nullptr) return widget.transform.position;
    const GuiWidgetPoint parent_point = resolved_point(*widget.parent);
    const GuiWidgetSize offset = pivot_offset(widget.parent->transform);
    GuiWidgetPoint point;
    point.x = (parent_point.x + widget.transform.position.x) - offset.width;
    point.y = (parent_point.y + widget.transform.position.y) - offset.height;
    point.z = parent_point.z + widget.transform.position.z;
    return point;
}

// Atlas item names are full virtual paths; page textures are relative and may use the
// authored backslash separator.
std::string atlas_key(const std::string& authored) {
    std::string path = "interface/textures/";
    for (char character : authored) path.push_back(character == '\\' ? '/' : character);
    return path;
}

struct BridgeVertex {
    float x, y, z, rhw;
    D3DCOLOR color;
    float u, v;
};

struct BridgeQuad {
    IDirect3DTexture9* texture{};
    float z{0.0f};
    BridgeVertex vertices[6]{};
};

// Milestone 2d: one quad of a Text widget's glyph run carries the widget's authored
// colour, so the diffuse lane is no longer a constant white for every quad.
D3DCOLOR bridge_color(const float (&color)[4]) noexcept {
    const auto lane = [](float value) -> int {
        const float scaled = value * 255.0f;
        if (!(scaled > 0.0f)) return 0;
        if (scaled >= 255.0f) return 255;
        return static_cast<int>(scaled + 0.5f);
    };
    return D3DCOLOR_ARGB(lane(color[3]), lane(color[0]), lane(color[1]), lane(color[2]));
}

}  // namespace

struct GameFrontendHost::Impl {
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    GameFontHost& fonts;
    IDirect3DDevice9& device;
    bool widescreen{};
    // 0109EEA4, the CRT's double-to-int conversion selector. Queried rather than assumed:
    // the same processor feature the CRT tests decides it.
    bool sse2_conversion{IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != FALSE};

    GuiPageRegistry registry;
    std::unique_ptr<VfsGuiPageScripts> page_scripts;
    std::vector<GameGuiResourceRecord> resources;
    std::vector<GamePageRecord> page_records;
    std::vector<GameWidgetRecord> widget_records;
    // Parallel to widget_records: the loaded widget each row describes. The pages live in
    // `registry` for the whole run, so these stay valid.
    std::vector<const GuiLayoutWidget*> widget_nodes;
    // The widgets 00aa5e20's virtual +34h calls reached, so a false there can be told from
    // the projected constructor default. Milestone 2c: the page roots that a front-end
    // screen's 004f83b0 commit reached land here too, which is what takes those pages out
    // of the bridge's substitute rule.
    std::set<const void*> visibility_applied;
    // Pages a front-end screen owns (milestone 2c).
    std::set<const GuiLayoutPage*> screen_pages;
    GameFrontendSummary summary;

    // The 0x88-byte GUI manager of 00aa5d70, kept as the dword slots 00aa5e20 writes.
    struct ManagerState {
        void* fields[0x88 / 4]{};
        bool constructed{false};
        bool ready_flag{true};
    } manager;
    std::set<const void*> page_handles;  // which create_gui_resource results are pages

    // The sprite bridge.
    D3dxImageImports imports;
    std::vector<IDirect3DTexture9*> owned_textures;
    std::map<std::string, IDirect3DTexture9*> loose_textures;
    TextureAtlasParseResult atlas;
    IDirect3DTexture9* atlas_texture{};
    UINT atlas_width{};
    UINT atlas_height{};
    std::vector<BridgeQuad> quads;
    // Milestone 2d, the text half of the bridge. The runs are built once per widget,
    // because a visibility change or a newly loaded page does not change a layout;
    // build_quads then turns the cached glyph quads into bridge quads every rebuild.
    std::unique_ptr<GameTextHost> text_host;
    std::map<const GuiLayoutWidget*, GameTextRun> text_runs;
    // Milestone 2e: the run-time 00ABAED0 sources a screen pushed into a Text
    // widget, which replace the page's authored DefaultText on the next build.
    std::map<const GuiLayoutWidget*, std::string> text_sources;
    unsigned back_buffer_width{};
    unsigned back_buffer_height{};
    bool quads_built{false};
    std::size_t last_logged_quads{static_cast<std::size_t>(-1)};
    std::size_t node_counter{0};

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        GameFontHost& fonts_in, IDirect3DDevice9& device_in, bool widescreen_in)
        : log(log_in), vfs(vfs_in), scripts(scripts_in), fonts(fonts_in), device(device_in),
          widescreen(widescreen_in) {}

    ~Impl() {
        for (IDirect3DTexture9* texture : owned_textures) {
            if (texture) texture->Release();
        }
    }

    // 00bdf4c0 then 00bdf310, the same read the font resources use.
    bool read(const std::string& requested, std::shared_ptr<MemoryStream>& stream,
        std::string& error) {
        VfsProviderManager* manager_ptr = vfs.manager();
        if (manager_ptr == nullptr) { error = "no mounted VFS"; return false; }
        std::string resolved = requested;
        if (!resolve_existing_resource_00bdf4c0_fragment(manager_ptr->context(),
                vfs.search_registrations(), resolved)) {
            error = "cannot resolve " + requested;
            return false;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(manager_ptr->context(),
            resolved, 2);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            error = "cannot read " + resolved + ": " + opened.error;
            return false;
        }
        stream = std::move(opened.stream);
        return true;
    }

    IDirect3DTexture9* create_texture(const std::string& path) {
        if (!imports.usable()) return nullptr;
        auto cached = loose_textures.find(path);
        if (cached != loose_textures.end()) return cached->second;
        std::shared_ptr<MemoryStream> stream;
        std::string error;
        IDirect3DTexture9* texture = nullptr;
        if (read(path, stream, error)) {
            const UINT size = static_cast<UINT>(stream->size_00bef600());
            const HRESULT result = imports.create(&device, stream->data_00bef610(), size,
                kD3dxDefault, kD3dxDefault, kD3dxDefault, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
                kD3dxDefault, kD3dxDefault, 0, nullptr, nullptr, &texture);
            if (FAILED(result)) texture = nullptr;
        }
        loose_textures.emplace(path, texture);
        if (texture) owned_textures.push_back(texture);
        return texture;
    }

    void record_widgets(const std::string& page, const GuiLayoutWidget& widget, int depth,
        const std::string& parent_key, bool runtime = false) {
        GameWidgetRecord record;
        record.runtime = runtime;
        record.page = page;
        record.key = widget.key;
        record.type = type_name(widget.type);
        record.parent = parent_key;
        record.depth = depth;
        const GuiWidgetPoint point = resolved_point(widget);
        record.x = point.x;
        record.y = point.y;
        record.z = point.z;
        record.width = widget.transform.size.width;
        record.height = widget.transform.size.height;
        record.pivot_x = widget.transform.pivot_x;
        record.pivot_y = widget.transform.pivot_y;
        record.visible = widget.visible;
        if (widget.source != nullptr) {
            record.visible_authored = widget.source->find("Visible") != nullptr;
            if (widget.type == GuiWidgetType::Icon) {
                // 00AB3310, the Icon's own authored reader: the States list with its
                // per-state texture and the ShaderName that selects its material.
                const GuiIconAuthoredPage authored = read_gui_icon_authored_page_00ab3310(
                    *widget.source, widget.transform, sse2_conversion);
                if (!authored.states.empty()) record.texture = authored.states.front().texture;
                record.material = authored.shader_name;
            } else {
                record.texture = first_state_texture(*widget.source);
                record.material = string_key(*widget.source, "ShaderName");
            }
            if (record.material.empty()) record.material = string_key(*widget.source, "Font");
        }
        if (runtime) {
            // A run-time clone is one of dozens per frame, and it is not one of
            // the page's authored widgets, so it neither logs a tree line nor
            // counts in the authored totals the earlier milestones report.
            widget_records.push_back(std::move(record));
            widget_nodes.push_back(&widget);
            for (const auto& child : widget.children) {
                if (child) record_widgets(page, *child, depth + 1, widget.key, true);
            }
            return;
        }
        if (!record.texture.empty()) ++summary.widgets_with_texture;
        log.notef("  widget %*s%-28s type=%-9s pos=(%.4f,%.4f,%.1f) size=(%.4f,%.4f) "
            "visible=%d texture=%s material=%s", depth * 2, "", record.key.c_str(),
            record.type.c_str(), static_cast<double>(record.x), static_cast<double>(record.y),
            static_cast<double>(record.z), static_cast<double>(record.width),
            static_cast<double>(record.height), record.visible ? 1 : 0,
            record.texture.empty() ? "-" : record.texture.c_str(),
            record.material.empty() ? "-" : record.material.c_str());
        widget_records.push_back(std::move(record));
        widget_nodes.push_back(&widget);
        for (const auto& child : widget.children) {
            if (child) record_widgets(page, *child, depth + 1, widget.key);
        }
    }

    // What the sprite bridge shows. A widget the native resource list explicitly hid, or
    // one the page authored as hidden, stays hidden, and so does its whole subtree. A
    // widget with neither is drawn: the projected default is false and the true value a
    // running game would see comes from the screen activate 004f83b0.
    //
    // Milestone 2c: for a page a front-end screen owns, that push now happens for real.
    // The page root is in visibility_applied, so the first arm of the walk answers from
    // the byte the screen published and the substitute rule never reaches it. The rule
    // still stands in for the pages no screen owns: the five 00aa5e20 resource entries
    // and the two frame layouts 00518250 selects.
    bool bridge_visible(const GuiLayoutWidget* widget) const {
        for (const GuiLayoutWidget* node = widget; node != nullptr; node = node->parent) {
            if (visibility_applied.count(node) != 0) {
                if (!node->visible) return false;
                continue;
            }
            if (node->source != nullptr && node->source->find("Visible") != nullptr
                && !node->visible) {
                return false;
            }
        }
        return true;
    }

    GuiLayoutPage* load_page(GuiLayoutHost& host, const std::string& name, bool log_tree);
    void load_atlases();
    void build_quads();
    // Milestone 2d: the glyph quads of one visible Text widget, appended to `quads`.
    void append_text_quads(GameWidgetRecord& record, const GuiLayoutWidget& node,
        float screen_w, float screen_h);
};

namespace {

// GuiLayoutHost for the page loader 00aa5840 / 00aaa710. The Lua half comes from
// GuiPageScriptLayoutHost, which is the reconstruction of 00ac6600's script side; the
// scene-graph half (00b74eb0, 00b75030, 00b6e680) has no reconstruction and is recorded
// as unimplemented with a diagnostic node id.
class GameGuiLayoutHost final : public GuiPageScriptLayoutHost {
public:
    GameGuiLayoutHost(GameFrontendHost::Impl& owner, VfsGuiPageScripts& scripts)
        : GuiPageScriptLayoutHost(scripts), owner_(owner) {}

    const bool& crt_sse2_conversion() const override { return owner_.sse2_conversion; }

    bool vfs_name_exists(const std::string& path) override {
        owner_.log.implemented("GuiLayoutHost::vfs_name_exists", "00aa58c3");
        return owner_.vfs.exists(path);
    }

    bool instantiate_page_model(const std::string& path) override {
        owner_.log.unimplemented("GuiLayoutHost::instantiate_page_model", "00aa58cc");
        (void)path;
        return false;
    }

    // Milestone 2d correction. 00AA6720 is BSP_GuiWidget_SetSceneNode, four instructions
    // that store a node at widget+4Ch and clear the low two bits of node+138h; it creates
    // nothing. What creates a plain page root is 00AA5840's own branch, which builds the
    // 18Ch cGroup of 00B8F5E0 and registers it. The host method therefore stands for the
    // creation and the bind together, and cites the bind, which is the call the loader
    // makes with the result.
    std::uint32_t create_page_root_node(const std::string& name, bool model_backed,
        std::uint8_t screen_flag) override {
        (void)name;
        (void)model_backed;
        (void)screen_flag;
        owner_.log.unimplemented("GuiLayoutHost::bind_page_root_scene_node", "00aa6720");
        return next_node();
    }

    // 00AA6640 BSP_GuiWidget_CreateWithSceneNode is the pair the loader runs per child:
    // 00B74EB0 takes a canonical 188h slot out of the model pool at 01090054 and 00B75030
    // constructs the 184h generated model in it. Both are recorded, because both are real
    // allocations this process does not perform.
    std::uint32_t create_widget_node(const std::string& key) override {
        (void)key;
        owner_.log.unimplemented("GuiLayoutHost::allocate_model_slot", "00b74eb0");
        owner_.log.unimplemented("GuiLayoutHost::create_widget_node", "00b75030");
        return next_node();
    }

    void set_node_parent(std::uint32_t child, std::uint32_t parent) override {
        (void)child;
        (void)parent;
        owner_.log.unimplemented("GuiLayoutHost::set_node_parent", "00b6e680");
    }

    bool widescreen_enabled() override {
        owner_.log.implemented("GuiLayoutHost::widescreen_enabled", "00aa8750");
        return owner_.widescreen;
    }

    void on_widget_constructed(GuiLayoutWidget& widget) override {
        (void)widget;
        owner_.log.unimplemented("GuiLayoutHost::widget_vtable_74", "00aaade8");
    }
    void on_widget_loaded(GuiLayoutWidget& widget) override {
        (void)widget;
        owner_.log.unimplemented("GuiLayoutHost::widget_vtable_78", "00aaae5b");
    }
    // 00AAA710 is the base half; its caller is the leaf class's own reader, and three of
    // the four types the title and menu pages use have one. Milestone 2b recorded a single
    // `derived_property_reader [00aaa710]` for all 29 widgets, which said less than it
    // could: the record now names the routine that stands behind each type.
    void on_widget_properties_bound(GuiLayoutWidget& widget, const GuiTable& table) override {
        if (widget.type == GuiWidgetType::Icon) {
            // 00AB3310, the Icon's authored States and ShaderName reader. The sprite
            // bridge already consumed its result; this puts the call at its own site.
            static_cast<void>(read_gui_icon_authored_page_00ab3310(table, widget.transform,
                owner_.sse2_conversion));
            owner_.log.implemented("GuiIcon::read_properties", "00ab3310");
            return;
        }
        if (widget.type == GuiWidgetType::Text) {
            // 00ABB630 needs the font registry and the locale tables, which the text
            // bridge owns; it runs and is recorded there, on the first draw.
            return;
        }
        if (widget.type == GuiWidgetType::FrameBox) {
            // 00AD08E0 is reconstructed (bsp/gui_framebox.hpp) but takes
            // GuiFrameBoxTextureServices, the native renderer's texture acquire and
            // release pair. This process owns no such reference, so the reader is not run.
            owner_.log.unimplemented("GuiFrameBox::read_properties", "00ad08e0");
            return;
        }
        owner_.log.unimplemented("GuiLayoutHost::derived_property_reader", "00aaa710");
    }

protected:
    // Milestone 2d correction. 00B6D890 is BSP_Node_PropagateRootRegistration, the
    // complete recursive root propagation that registers or unregisters a subtree against
    // a requested scene root; it is not a list clear. 00AC6825 calls it with the page's
    // own root. There is no scene node here, so nothing is propagated and the site is
    // recorded under its own name.
    void clear_page_root_list_00b6d890(GuiLayoutPage& page) override {
        (void)page;
        owner_.log.unimplemented("GuiLayoutHost::propagate_root_registration", "00b6d890");
    }

private:
    std::uint32_t next_node() noexcept {
        // Never zero: 00AA7E00 skips a child whose node pointer is null.
        return static_cast<std::uint32_t>(++owner_.node_counter);
    }
    GameFrontendHost::Impl& owner_;
};

// GuiStartupHost for 0073bae0.
class GameGuiStartupHost final : public GuiStartupHost {
public:
    GameGuiStartupHost(GameFrontendHost::Impl& owner, GameGuiLayoutHost& layout,
        std::string language_font_path)
        : owner_(owner), layout_(layout), language_font_path_(std::move(language_font_path)) {}

    std::string language_font_path() override {
        owner_.log.implemented("GuiStartupHost::language_font_path", "008d4890");
        return language_font_path_;
    }

    FontRegistry& font_registry() override {
        owner_.log.implemented("GuiStartupHost::font_registry", "007371d0");
        return owner_.fonts.registry().registry();
    }

    bool load_font_descriptors(FontRegistry& registry, std::string_view root,
        std::string_view descriptor, std::string_view language_path) override {
        (void)registry;
        owner_.fonts.load_descriptors_00ac3910(std::string(root), std::string(descriptor),
            std::string(language_path));
        owner_.log.implemented("GuiStartupHost::load_font_descriptors", "00ac3910");
        return true;
    }

    void preload_fallback_glyph_table() override {
        owner_.fonts.preload_fingerprint_payload_00be9760();
        owner_.log.implemented("GuiStartupHost::preload_fallback_glyph_table", "0053bc00");
    }

    void* gui_manager() override {
        // 004c12b0, the double-checked singleton over DAT_00f8bc5c, constructed by
        // 00aa5d70. The lifetime manager's critical section is not modelled; the
        // once-only construction it protects is.
        if (!owner_.manager.constructed) {
            owner_.manager.constructed = true;
            owner_.summary.gui_manager_created = true;
            owner_.log.implemented("GuiStartupHost::gui_manager_get_or_create", "004c12b0");
        }
        return &owner_.manager;
    }

    void* create_gui_resource(void* manager, void* parent,
        const GuiManagerResource& entry) override {
        (void)manager;
        GameGuiResourceRecord record;
        record.name = std::string(entry.name);
        record.offset = entry.offset;
        void* result = nullptr;
        switch (entry.kind) {
        case GuiResourceKind::Texture: {
            record.kind = "texture";
            // The renderer's own texture entry point is *(00f8d394) virtual +64h, which
            // belongs to the renderer owner. The bridge loads the file through the same
            // mounted VFS instead, and a name the VFS rejects stays null exactly as the
            // native guard leaves it.
            IDirect3DTexture9* texture = owner_.create_texture(record.name);
            result = texture;
            record.detail = texture != nullptr ? "loaded by the sprite bridge"
                                               : "VFS rejected the name";
            owner_.log.unimplemented("GuiManagerResources::renderer_load_texture", "00aa5e60");
            break;
        }
        case GuiResourceKind::Group: {
            record.kind = "group";
            GuiLayoutPage* page = owner_.load_page(layout_, record.name, true);
            result = page;
            if (page != nullptr) owner_.page_handles.insert(page);
            record.detail = page != nullptr ? "page loaded through 00aa5840"
                                            : "page load failed";
            owner_.log.implemented("GuiManagerResources::load_page", "00aa5840");
            break;
        }
        case GuiResourceKind::Child: {
            record.kind = "child";
            auto* page = static_cast<GuiLayoutPage*>(parent);
            GuiLayoutWidget* child = nullptr;
            if (page != nullptr && page->root) {
                child = find_child_by_name_00aa7e00(*page->root, record.name);
            }
            result = child;
            record.detail = child != nullptr ? "direct child of the group's root"
                                             : "no such direct child";
            owner_.log.implemented("GuiManagerResources::find_child", "00aa7e00");
            break;
        }
        }
        record.acquired = result != nullptr;
        if (record.acquired) ++owner_.summary.gui_resources_acquired;
        owner_.log.notef("gui resource %-38s %-7s %-8s offset=%s (%s)", record.name.c_str(),
            record.kind.c_str(), record.acquired ? "acquired" : "null",
            entry.offset == kGuiResourceNotStored ? "not stored" : "manager", record.detail.c_str());
        owner_.resources.push_back(std::move(record));
        return result;
    }

    void store_gui_resource(void* manager, std::uint16_t offset, void* value) override {
        auto* state = static_cast<GameFrontendHost::Impl::ManagerState*>(manager);
        if (state != nullptr && offset < 0x88) state->fields[offset / 4] = value;
        ++owner_.summary.gui_resource_stores;
    }

    void set_gui_resource_visibility(void* resource, bool visible) override {
        // The native call is the object's virtual +34h. For a page that is the screen's
        // own visibility byte; for a child it is the widget's. Both reach the projected
        // `visible` field here; the scene-node half of the native setter does not exist.
        if (resource == nullptr) return;
        if (owner_.page_handles.count(resource) != 0) {
            auto* page = static_cast<GuiLayoutPage*>(resource);
            if (page->root) {
                page->root->visible = visible;
                owner_.visibility_applied.insert(page->root.get());
            }
        } else {
            auto* widget = static_cast<GuiLayoutWidget*>(resource);
            widget->visible = visible;
            owner_.visibility_applied.insert(widget);
        }
        owner_.log.implemented("GuiManagerResources::set_visibility", "00aa5faf");
    }

    void clear_gui_manager_ready_flag(void* manager) override {
        auto* state = static_cast<GameFrontendHost::Impl::ManagerState*>(manager);
        if (state != nullptr) state->ready_flag = false;
        owner_.log.implemented("GuiManagerResources::clear_ready_flag", "00aa6305");
    }

private:
    GameFrontendHost::Impl& owner_;
    GameGuiLayoutHost& layout_;
    std::string language_font_path_;
};

}  // namespace

GuiLayoutPage* GameFrontendHost::Impl::load_page(GuiLayoutHost& host, const std::string& name,
    bool log_tree) {
    GamePageRecord record;
    record.name = name;
    ++summary.pages_requested;
    GuiLayoutPage* page = nullptr;
    try {
        page = load_gui_page_00aa5840(registry, host, name, 1, false);
    } catch (const std::exception& error) {
        record.error = error.what();
    }
    if (page != nullptr) {
        record.loaded = true;
        record.priority = page->priority;
        record.model_backed = page->model_backed;
        ++summary.pages_loaded;
        log.notef("gui page %-18s priority=%d model_backed=%d script=%d", name.c_str(),
            page->priority, page->model_backed ? 1 : 0, page->script_evaluated ? 1 : 0);
        if (log_tree && page->root) {
            const std::size_t before = widget_records.size();
            record_widgets(name, *page->root, 0, std::string());
            record.widgets = widget_records.size() - before;
            summary.widgets += record.widgets;
        }
    } else {
        log.notef("gui page %-18s FAILED: %s", name.c_str(),
            record.error.empty() ? "no page" : record.error.c_str());
    }
    page_records.push_back(std::move(record));
    return page;
}

GameFrontendHost::GameFrontendHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
    GameFontHost& fonts, IDirect3DDevice9& device, bool widescreen)
    : impl_(std::make_unique<Impl>(log, vfs, scripts, fonts, device, widescreen)) {}
GameFrontendHost::~GameFrontendHost() = default;

void GameFrontendHost::run_font_and_gui_startup_0073bae0(const std::string& language_font_path) {
    Impl& host = *impl_;
    host.page_scripts = std::make_unique<VfsGuiPageScripts>(host.scripts.files(),
        host.scripts.runtime(), host.scripts.globals());
    GameGuiLayoutHost layout(host, *host.page_scripts);
    GameGuiStartupHost startup(host, layout, language_font_path);
    const GuiStartupResult result = run_gui_startup(startup);
    host.summary.gui_startup_ran = true;
    host.summary.font_descriptors_loaded = result.font_descriptors_loaded;
    for (const auto& font : host.fonts.registry().fonts()) {
        host.log.notef("font registered %-12s data=%-16s gfx=%-16s alpha=%-12s scale=%.4f "
            "upper=%d resources=%d", font->descriptor.name.c_str(),
            font->descriptor.data_file.c_str(), font->descriptor.gfx_file.c_str(),
            font->descriptor.alpha_texture.c_str(),
            static_cast<double>(font->descriptor.scale_ratio),
            font->descriptor.uppercase_only ? 1 : 0, font->resources ? 1 : 0);
    }
    host.log.notef("gui startup language_font_path=%s fonts=%zu resources=%zu stores=%zu "
        "manager=%p", result.language_font_path.empty() ? "(none)"
            : result.language_font_path.c_str(), host.fonts.registry().fonts().size(),
        result.resources_created, result.stores, result.gui_manager);
    host.log.implemented("Phase 7 fonts_and_gui_startup", "0073bae0");
}

void GameFrontendHost::load_title_pages(const std::vector<std::string>& names) {
    Impl& host = *impl_;
    if (!host.page_scripts) {
        throw std::logic_error("Title pages require the GUI startup phase");
    }
    GameGuiLayoutHost layout(host, *host.page_scripts);
    for (const std::string& name : names) {
        host.load_page(layout, name, true);
        // The same loader 00518250 and the title screen's activate reach; the screens that
        // would call it are packet cc_frontend_states.
        host.log.implemented("TitlePages::load_page", "00aa5840");
    }
}

void GameFrontendHost::open_sprite_bridge(unsigned back_buffer_width,
    unsigned back_buffer_height) {
    Impl& host = *impl_;
    host.back_buffer_width = back_buffer_width;
    host.back_buffer_height = back_buffer_height;
    if (!host.imports.usable()) {
        host.log.note("sprite bridge unavailable: d3dx9_40 image imports missing");
        return;
    }
    host.load_atlases();
    host.summary.bridge_open = true;
}

void GameFrontendHost::open_text_bridge(LocaleTables& locale) {
    Impl& host = *impl_;
    if (host.text_host) return;
    host.text_host = std::make_unique<GameTextHost>(host.log, host.fonts, locale);
    host.summary.text_bridge_open = true;
    host.quads_built = false;
    host.log.notef("text bridge open: %zu fonts, %zu locale keys, vertical_scale=%.4f "
        "(00e12fd4)", host.fonts.registry().fonts().size(), locale.size(),
        static_cast<double>(host.text_host->summary().vertical_scale));
}

void GameFrontendHost::Impl::load_atlases() {
    // 00aeeaf0 over the installed atlas descriptors. GGame::OnInitTitle loads exactly one,
    // `interface/textures/allbutingame.ats`, but the page textures of the title layouts are
    // spread over the whole installed set, and the reconstructed startup does not populate
    // the VFS suffix list (manager +48h/+4Ch) that would turn that one name into the DXT
    // variant a machine actually ships. Enumerating the variants, and merging every atlas
    // into one item list, are bridge decisions; the native manager keeps them separate.
    static const char* const bases[] = {"allbutingame", "common", "game", "menu"};
    static const char* const variants[] = {"", "_dxt1", "_dxt5", "_dxt1_1", "_dxt1_2",
        "_dxt5_1", "_dxt5_2", "_dxt5_3"};
    std::size_t descriptors = 0;
    for (const char* base : bases) {
        for (const char* variant : variants) {
            const std::string path = std::string("interface/textures/") + base + variant
                + ".ats";
            std::shared_ptr<MemoryStream> stream;
            std::string error;
            if (!read(path, stream, error)) continue;
            const std::string text(reinterpret_cast<const char*>(stream->data_00bef610()),
                static_cast<std::size_t>(stream->size_00bef600()));
            TextureAtlasParseResult parsed = parse_texture_atlas_00aeeaf0(text, path,
                [this](std::string_view name, std::uint32_t flags) -> void* {
                    (void)flags;
                    return create_texture(std::string(name));
                });
            if (parsed.items.empty()) {
                log.notef("atlas %s rejected: %s", path.c_str(), parsed.detail.c_str());
                continue;
            }
            ++descriptors;
            if (atlas.items.empty()) {
                atlas_texture = static_cast<IDirect3DTexture9*>(parsed.texture);
                if (atlas_texture != nullptr) {
                    D3DSURFACE_DESC description{};
                    if (SUCCEEDED(atlas_texture->GetLevelDesc(0, &description))) {
                        atlas_width = description.Width;
                        atlas_height = description.Height;
                    }
                }
                summary.bridge_atlas = path;
            }
            log.notef("atlas %-44s items=%3zu texture=%s", path.c_str(), parsed.items.size(),
                parsed.texture_path.c_str());
            atlas.items.insert(atlas.items.end(),
                std::make_move_iterator(parsed.items.begin()),
                std::make_move_iterator(parsed.items.end()));
        }
    }
    summary.bridge_atlas_items = atlas.items.size();
    log.notef("sprite bridge atlases=%zu items=%zu", descriptors, atlas.items.size());
}

// Milestone 2d. The reconstructed text path runs once per widget and its result is
// cached: the layout depends on the authored table, the font and the widget size, none
// of which a visibility push or a page load changes. What this adds on top of the
// reconstruction is only the placement of the text context's normalised space at the
// widget's own resolved corner, and the fixed-function draw, both of which are the
// bridge's and are labelled as such.
void GameFrontendHost::Impl::append_text_quads(GameWidgetRecord& record,
    const GuiLayoutWidget& node, float screen_w, float screen_h) {
    if (!text_host) return;
    auto cached = text_runs.find(&node);
    if (cached == text_runs.end()) {
        // The widget's own pivot is a fraction of its own size, exactly as for a sprite:
        // this is the corner the text context's origin sits on.
        const float origin_x = record.x - record.pivot_x * record.width;
        const float origin_y = record.y - record.pivot_y * record.height;
        GameTextRun run;
        const auto source = text_sources.find(&node);
        text_host->build_run(record.page, node, origin_x, origin_y, run,
            source != text_sources.end() ? &source->second : nullptr);
        cached = text_runs.emplace(&node, std::move(run)).first;
        summary.text_widgets = text_host->summary().text_widgets;
        summary.text_runs = text_host->summary().runs_built;
        summary.text_glyphs = text_host->summary().glyph_quads;
    }
    const GameTextRun& run = cached->second;
    record.text = run.resolved_ascii;
    record.text_glyphs = run.quads.size();
    if (run.sheet == nullptr || run.quads.empty()) return;
    // The colour is read live rather than out of the cached run: 00ab6b50 re-applies the
    // widget's +50h on every set, and the press-start screen's update pulses exactly that
    // field on its prompt through the element vtable +50h, 62 times in a 120 frame run.
    const D3DCOLOR color = bridge_color(node.color);
    for (const GameTextQuad& glyph : run.quads) {
        BridgeQuad quad;
        quad.texture = run.sheet;
        quad.z = record.z;
        const float left = glyph.left * screen_w;
        const float top = glyph.top * screen_h;
        const float right = glyph.right * screen_w;
        const float bottom = glyph.bottom * screen_h;
        const BridgeVertex top_left{left, top, 0.0f, 1.0f, color, glyph.u1, glyph.v1};
        const BridgeVertex top_right{right, top, 0.0f, 1.0f, color, glyph.u2, glyph.v1};
        const BridgeVertex bottom_left{left, bottom, 0.0f, 1.0f, color, glyph.u1, glyph.v2};
        const BridgeVertex bottom_right{right, bottom, 0.0f, 1.0f, color, glyph.u2,
            glyph.v2};
        quad.vertices[0] = top_left;
        quad.vertices[1] = top_right;
        quad.vertices[2] = bottom_left;
        quad.vertices[3] = top_right;
        quad.vertices[4] = bottom_right;
        quad.vertices[5] = bottom_left;
        quads.push_back(quad);
        ++summary.text_quads;
    }
    record.drawn = true;
}

void GameFrontendHost::Impl::build_quads() {
    quads_built = true;
    ++summary.bridge_rebuilds;
    summary.text_quads = 0;
    quads.clear();
    for (GameWidgetRecord& record : widget_records) record.drawn = false;
    if (back_buffer_width == 0 || back_buffer_height == 0) return;
    const float screen_w = static_cast<float>(back_buffer_width);
    const float screen_h = static_cast<float>(back_buffer_height);
    // The widget records are in page order and, inside a page, in tree order. The bridge
    // draws them back to front by the authored Z, which is the only ordering key it takes
    // from the data; the native render order (RenderOrder / geOrder) is another owner's.
    std::vector<std::size_t> order;
    for (std::size_t index = 0; index < widget_records.size(); ++index) order.push_back(index);
    std::stable_sort(order.begin(), order.end(), [this](std::size_t left, std::size_t right) {
        // Milestone 2k: the executable's own run-time clones are drawn after
        // every authored quad. The authored Z is not the native draw order at
        // all -- `GUI_minimap` puts its island map at Z -5 and its frame, glass
        // and direction wedge at -22, -23 and -24, so a Z-only order buries the
        // unit icons under the frame, while the page's own `geOrder` puts
        // unit_marker_Group (0) in front of every one of them. geOrder is not a
        // draw order either (`sidemarker_Group` gives HP_Icon 5 and HP_BG_Icon 3,
        // which is front-to-back), and the native render order is another
        // owner's, so the bridge keeps milestone 2b's Z rule for authored
        // widgets and adds one rule of its own for the widgets it created.
        const bool left_runtime = widget_records[left].runtime;
        const bool right_runtime = widget_records[right].runtime;
        if (left_runtime != right_runtime) return !left_runtime;
        return widget_records[left].z > widget_records[right].z;
    });
    for (std::size_t index : order) {
        GameWidgetRecord& record = widget_records[index];
        const GuiLayoutWidget* node = widget_nodes[index];
        // 00aa5e20's visibility calls run after the pages load, so the record's own flag is
        // refreshed from the widget before the bridge reads it.
        if (node != nullptr) record.visible = node->visible;
        // Milestone 2k: a run-time clone is moved every mission frame by the
        // minimap icon pass or the marker pass, so its cached transform is stale
        // by construction and is re-read here. An authored widget keeps the
        // values the page load resolved, exactly as before.
        if (node != nullptr && record.runtime) {
            const GuiWidgetPoint point = resolved_point(*node);
            record.x = point.x;
            record.y = point.y;
            record.z = point.z;
            record.width = node->transform.size.width;
            record.height = node->transform.size.height;
            record.pivot_x = node->transform.pivot_x;
            record.pivot_y = node->transform.pivot_y;
        }
        if (!bridge_visible(node)) continue;
        // Milestone 2d: a Text widget carries a font, not a texture, so it takes the
        // reconstructed text path instead of the atlas lookup below.
        if (node != nullptr && node->type == GuiWidgetType::Text) {
            // A run-time clone's Text run caches the origin it was laid out at,
            // and the pass that owns the clone moves it every frame, so the
            // cached run is dropped rather than reused.
            if (record.runtime) text_runs.erase(node);
            append_text_quads(record, *node, screen_w, screen_h);
            continue;
        }
        if (record.texture.empty()) continue;
        IDirect3DTexture9* texture = nullptr;
        float u1 = 0.0f, v1 = 0.0f, u2 = 1.0f, v2 = 1.0f;
        const std::string key = atlas_key(record.texture);
        if (const TextureAtlasItem* item = find_texture_atlas_item_00aefb20(atlas.items,
                key.c_str())) {
            texture = static_cast<IDirect3DTexture9*>(item->texture);
            u1 = item->uv[0];
            v1 = item->uv[1];
            u2 = item->uv[2];
            v2 = item->uv[3];
        } else {
            texture = create_texture(key);
        }
        if (texture == nullptr) {
            if (summary.bridge_rebuilds == 1) {
                log.notef("sprite bridge miss %-22s %s (no atlas item, no loose file)",
                    record.key.c_str(), key.c_str());
            }
            continue;
        }
        float width = record.width;
        float height = record.height;
        if (width <= 0.0f || height <= 0.0f) {
            // The authored native size: the texture's own extent over the 960x720 design
            // resolution, multiplied by the atlas sub-rectangle (docs/GUI_ICON_WIDGET.md's
            // native-size rule, which the FrameBox reader deliberately does not apply).
            UINT texture_width = atlas_width;
            UINT texture_height = atlas_height;
            if (texture != atlas_texture) {
                D3DSURFACE_DESC description{};
                if (SUCCEEDED(texture->GetLevelDesc(0, &description))) {
                    texture_width = description.Width;
                    texture_height = description.Height;
                }
            }
            width = (u2 - u1) * static_cast<float>(texture_width) / 960.0f;
            height = (v2 - v1) * static_cast<float>(texture_height) / 720.0f;
        }
        if (width <= 0.0f || height <= 0.0f) continue;
        // A widget's own pivot is a fraction of its own size, subtracted from its resolved
        // position (00AA6750's pivot_offset, applied to the widget itself here).
        const float left = (record.x - record.pivot_x * width) * screen_w;
        const float top = (record.y - record.pivot_y * height) * screen_h;
        const float right = left + width * screen_w;
        const float bottom = top + height * screen_h;
        BridgeQuad quad;
        quad.texture = texture;
        quad.z = record.z;
        // Milestone 2k: a run-time clone carries its template's authored Color
        // (the six minimap groups differ only by it) and the rotation the icon
        // pass wrote. An authored widget keeps milestone 2b's constant white and
        // its axis-aligned quad, so no earlier capture moves. Honouring the
        // authored Color and Rotate on every quad is bridge work this milestone
        // does not do; see the follow-ups.
        const D3DCOLOR color = (node != nullptr && record.runtime)
            ? bridge_color(node->color) : D3DCOLOR_ARGB(255, 255, 255, 255);
        BridgeVertex top_left{left, top, 0.0f, 1.0f, color, u1, v1};
        BridgeVertex top_right{right, top, 0.0f, 1.0f, color, u2, v1};
        BridgeVertex bottom_left{left, bottom, 0.0f, 1.0f, color, u1, v2};
        BridgeVertex bottom_right{right, bottom, 0.0f, 1.0f, color, u2, v2};
        if (node != nullptr && record.runtime && node->transform.rotate != 0.0f) {
            // The authored Rotate reaches the native Z rotation builder 00AA7250
            // negated (bsp/gui_widget.hpp), and the GUI y axis points down, so a
            // widget whose texture points along +x turns toward its own heading
            // at -rotate. The rotation is about the widget's own pivot, which is
            // where the resolved position sits.
            const float pivot_x = record.x * screen_w;
            const float pivot_y = record.y * screen_h;
            const float angle = -node->transform.rotate;
            const float cosine = std::cos(angle);
            const float sine = std::sin(angle);
            const auto spin = [&](BridgeVertex& vertex) {
                const float dx = vertex.x - pivot_x;
                const float dy = vertex.y - pivot_y;
                vertex.x = pivot_x + dx * cosine - dy * sine;
                vertex.y = pivot_y + dx * sine + dy * cosine;
            };
            spin(top_left);
            spin(top_right);
            spin(bottom_left);
            spin(bottom_right);
        }
        quad.vertices[0] = top_left;
        quad.vertices[1] = top_right;
        quad.vertices[2] = bottom_left;
        quad.vertices[3] = top_right;
        quad.vertices[4] = bottom_right;
        quad.vertices[5] = bottom_left;
        quads.push_back(quad);
        record.drawn = true;
    }
    summary.bridge_quads = quads.size();
    summary.bridge_textures = 0;
    for (const auto& entry : loose_textures) {
        if (entry.second != nullptr) ++summary.bridge_textures;
    }
    // The quad list is rebuilt whenever a screen publishes a visibility byte or a page
    // loads, so only a changed result is worth a line.
    if (last_logged_quads != quads.size()) {
        last_logged_quads = quads.size();
        log.notef("sprite bridge quads=%zu (text_glyph_quads=%zu) textures=%zu/%zu "
            "atlas_items=%zu rebuild=%zu", quads.size(), summary.text_quads,
            summary.bridge_textures, loose_textures.size(), atlas.items.size(),
            summary.bridge_rebuilds);
    }
}

void GameFrontendHost::draw_bridge(IDirect3DDevice9& device) {
    Impl& host = *impl_;
    if (!host.summary.bridge_open) return;
    if (!host.quads_built) host.build_quads();
    if (host.quads.empty()) return;
    ++host.summary.bridge_frames;
    device.SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
    device.SetRenderState(D3DRS_LIGHTING, FALSE);
    device.SetRenderState(D3DRS_ZENABLE, FALSE);
    device.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device.SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    device.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    device.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    device.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    device.SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    device.SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    device.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    for (const BridgeQuad& quad : host.quads) {
        device.SetTexture(0, quad.texture);
        device.DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quad.vertices, sizeof(BridgeVertex));
    }
    device.SetTexture(0, nullptr);
}

// ---------------------------------------------------------------------------
// Milestone 2c: pages a front-end screen owns
// ---------------------------------------------------------------------------

GuiLayoutPage* GameFrontendHost::load_screen_page(const std::string& name) {
    Impl& host = *impl_;
    if (!host.page_scripts) {
        throw std::logic_error("A screen page requires the GUI startup phase");
    }
    GameGuiLayoutHost layout(host, *host.page_scripts);
    const std::size_t before = host.widget_records.size();
    GuiLayoutPage* page = host.load_page(layout, name, true);
    if (page == nullptr) return nullptr;
    host.screen_pages.insert(page);
    for (std::size_t index = before; index < host.widget_records.size(); ++index) {
        host.widget_records[index].screen_owned = true;
    }
    host.summary.screen_owned_pages = host.screen_pages.size();
    host.quads_built = false;
    return page;
}

void GameFrontendHost::release_screen_page(GuiLayoutPage* page) {
    Impl& host = *impl_;
    // 00aa31f0 on the GUI manager. GuiPageRegistry owns every page for the whole
    // run, so the release is recorded and the page stays loaded; nothing here
    // models the native reference count at page+4h.
    host.log.unimplemented("GuiManagerResources::release_page", "00aa31f0");
    if (page != nullptr) host.screen_pages.erase(page);
}

GuiLayoutWidget* GameFrontendHost::find_page_child(GuiLayoutPage& page,
    const std::string& name) {
    Impl& host = *impl_;
    host.log.implemented("GuiManagerResources::find_child", "00aa7e00");
    if (!page.root) return nullptr;
    return find_child_by_name_00aa7e00(*page.root, name);
}

void GameFrontendHost::commit_page_visibility(GuiLayoutPage& page, bool visible) {
    Impl& host = *impl_;
    if (!page.root) return;
    page.root->visible = visible;
    host.visibility_applied.insert(page.root.get());
    ++host.summary.visibility_pushes;
    host.quads_built = false;
}

void GameFrontendHost::set_widget_visible(GuiLayoutWidget& widget, bool visible) {
    Impl& host = *impl_;
    widget.visible = visible;
    host.visibility_applied.insert(&widget);
    host.quads_built = false;
}

void GameFrontendHost::set_widget_color(GuiLayoutWidget& widget, float r, float g, float b,
    float a) {
    // Milestone 2d: a changed colour now changes what is drawn, because a Text widget's
    // glyph quads carry it in their diffuse lane, so the bridge has to rebuild. Before
    // the text half every quad was a constant white and this store reached nothing.
    const bool changed = widget.color[0] != r || widget.color[1] != g
        || widget.color[2] != b || widget.color[3] != a;
    widget.color[0] = r;
    widget.color[1] = g;
    widget.color[2] = b;
    widget.color[3] = a;
    if (changed) impl_->quads_built = false;
}

void GameFrontendHost::set_widget_text_source(GuiLayoutWidget& widget, std::string source) {
    Impl& host = *impl_;
    host.text_sources[&widget] = std::move(source);
    host.text_runs.erase(&widget);
    host.quads_built = false;
}

void GameFrontendHost::invalidate_bridge() { impl_->quads_built = false; }

// ---------------------------------------------------------------------------
// Milestone 2k: run-time clones of a page's own authored template
// ---------------------------------------------------------------------------

namespace {

// A deep copy of one authored widget and its subtree. `source` keeps pointing at
// the page's evaluated table, which outlives every clone because the registry
// owns the page for the whole run. The scene-node id is not copied: a clone is
// not one of the 00b75030 nodes the loader asked the host for.
std::unique_ptr<GuiLayoutWidget> clone_widget_tree(const GuiLayoutWidget& source,
    const std::string& key) {
    auto clone = std::make_unique<GuiLayoutWidget>();
    clone->key = key;
    clone->type = source.type;
    clone->transform = source.transform;
    clone->transform.parent = nullptr;
    clone->source = source.source;
    clone->parent = nullptr;
    clone->node_id = 0;
    for (int lane = 0; lane < 4; ++lane) {
        clone->color[lane] = source.color[lane];
        clone->low_color[lane] = source.low_color[lane];
        clone->high_color[lane] = source.high_color[lane];
    }
    clone->blend_factor = source.blend_factor;
    clone->visible = source.visible;
    for (const auto& child : source.children) {
        if (!child) continue;
        std::unique_ptr<GuiLayoutWidget> copy = clone_widget_tree(*child, child->key);
        copy->parent = clone.get();
        copy->transform.parent = &clone->transform;
        clone->children.push_back(std::move(copy));
    }
    return clone;
}

void show_runtime_subtree(GameFrontendHost::Impl& host, GuiLayoutWidget& widget) {
    widget.visible = true;
    host.visibility_applied.insert(&widget);
    for (const auto& child : widget.children) {
        if (child) show_runtime_subtree(host, *child);
    }
}

}  // namespace

GuiLayoutWidget* GameFrontendHost::clone_runtime_widget(const std::string& page,
    const GuiLayoutWidget& source, GuiLayoutWidget& parent, const std::string& key) {
    Impl& host = *impl_;
    std::unique_ptr<GuiLayoutWidget> clone = clone_widget_tree(source, key);
    clone->parent = &parent;
    clone->transform.parent = &parent.transform;
    GuiLayoutWidget* borrowed = clone.get();
    parent.children.push_back(std::move(clone));
    // The clone and its subtree are drawn, so the bridge's substitute visibility
    // rule must not hide them: the template the six minimap groups carry has no
    // "Visible" key at all, and the marker template is authored with the
    // lower-case `visible` the property reader never looks at.
    show_runtime_subtree(host, *borrowed);
    host.record_widgets(page, *borrowed, 0, parent.key, true);
    host.quads_built = false;
    return borrowed;
}

void GameFrontendHost::set_widget_local_position(GuiLayoutWidget& widget, float x, float y,
    float z) {
    Impl& host = *impl_;
    widget.transform.position.x = x;
    widget.transform.position.y = y;
    widget.transform.position.z = z;
    widget.transform.authored_x = x;
    host.quads_built = false;
}

void GameFrontendHost::set_widget_rotation(GuiLayoutWidget& widget, float radians) {
    Impl& host = *impl_;
    if (widget.transform.rotate == radians) return;
    widget.transform.rotate = radians;
    host.quads_built = false;
}

std::size_t GameFrontendHost::runtime_widgets_drawn() const noexcept {
    std::size_t drawn = 0;
    for (const GameWidgetRecord& record : impl_->widget_records) {
        if (record.runtime && record.drawn) ++drawn;
    }
    return drawn;
}

bool GameFrontendHost::save_back_buffer(IDirect3DDevice9& device, const std::string& path) {
    Impl& host = *impl_;
    if (host.imports.save_surface == nullptr) {
        host.log.note("screenshot unavailable: d3dx9_40!D3DXSaveSurfaceToFileA missing");
        return false;
    }
    IDirect3DSurface9* surface = nullptr;
    HRESULT result = device.GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface);
    if (FAILED(result) || surface == nullptr) {
        host.log.notef("screenshot failed: GetBackBuffer hr=0x%08lx",
            static_cast<unsigned long>(result));
        return false;
    }
    result = host.imports.save_surface(path.c_str(), kD3dxImageFormatPng, surface, nullptr,
        nullptr);
    surface->Release();
    if (FAILED(result)) {
        host.log.notef("screenshot failed: D3DXSaveSurfaceToFileA hr=0x%08lx",
            static_cast<unsigned long>(result));
        return false;
    }
    host.log.notef("screenshot written %s (%ux%u)", path.c_str(), host.back_buffer_width,
        host.back_buffer_height);
    return true;
}

const std::vector<GameGuiResourceRecord>& GameFrontendHost::gui_resources() const noexcept {
    return impl_->resources;
}
const std::vector<GamePageRecord>& GameFrontendHost::pages() const noexcept {
    return impl_->page_records;
}
const std::vector<GameWidgetRecord>& GameFrontendHost::widgets() const noexcept {
    return impl_->widget_records;
}
const GameFrontendSummary& GameFrontendHost::summary() const noexcept { return impl_->summary; }

}  // namespace bsp::game
