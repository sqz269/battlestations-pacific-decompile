// bsp_game.exe milestone 2b: phase 7 of application initialize and the title GUI pages.
// See include/bsp/game_hosts_frontend.hpp for the address list and the evidence.
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/frontend_scroller_bridge.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/cc13_sweepA2.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/cc13_gui1.hpp"

#include "bsp/game_native_gui_resource_application.hpp"
#include "bsp/game_native_gui_manager_application.hpp"
#include "bsp/game_native_gui_page_application.hpp"
#include "bsp/native_string.hpp"

#include "bsp/blocking_screen.hpp"
#include "bsp/d3d9_texture.hpp"
#include "bsp/fingerprint_payload.hpp"
#include "bsp/font_registry_startup.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_fonts.hpp"
#include "bsp/game_hosts_text.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_icon_runtime.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_manager_cc13.hpp"
#include "bsp/gui_page_script.hpp"
#include "bsp/gui_startup.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/gui_timed_entry_types.hpp"
#include "bsp/engine_seg49_64_78_cc13.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/texture_atlas.hpp"
#include "bsp/vfs_candidates.hpp"
#include "bsp/vfs_mounts.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdlib>
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

// Borrow the SAME 88h manager constructed by 00AA5D70, never a copied flag.
// 00AA5E20 produces +7C/+80 at 00AA6251/00AA62C8 and clears +84 at
// 00AA6305. 00AA2110's only stores are the one-byte +84 shown latch.
struct GuiSafeZoneManagerImage {
    std::byte prefix_00[0x7c];
    void* four_by_three_7c;
    void* sixteen_by_nine_80;
    gui_manager_cc13::GuiSafeZoneState state_84;
    std::byte tail_85[3];
};
static_assert(sizeof(gui_manager_cc13::GuiSafeZoneState) == 1);
static_assert(offsetof(GuiSafeZoneManagerImage, four_by_three_7c) == 0x7c);
static_assert(offsetof(GuiSafeZoneManagerImage, sixteen_by_nine_80) == 0x80);
static_assert(offsetof(GuiSafeZoneManagerImage, state_84) == 0x84);
static_assert(sizeof(GuiSafeZoneManagerImage) == 0x88);

// D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG. Declared locally for the same reason.
constexpr DWORD kD3dxImageFormatPng = 3;
using SaveSurfaceToFile = HRESULT(WINAPI*)(LPCSTR, DWORD, IDirect3DSurface9*,
    const PALETTEENTRY*, const RECT*);

// The binary's scalar constants, shared by this bridge's constructor/update
// domain. bsp.py ghidra bytes: D7A24C = 3F800000, D7A208 = 80000000 (negative
// zero). These are not widget state defaults.
const volatile float kTimedOne = 1.0f;
const volatile float kTimedZero = -0.0f;
struct BridgeAlphaEntryDelete {
    void operator()(GuiTimedEntryStorage* entry) const noexcept {
        if (!entry) return;
        try {
            delete_gui_timed_entry_profile(*entry, 1u, &singleton_lifetime_free);
        } catch (...) {
            std::terminate();
        }
    }
};
using BridgeAlphaEntry = std::unique_ptr<GuiTimedEntryStorage, BridgeAlphaEntryDelete>;

// AD3A80's indices 0/1 use the bridge's real borrowed layout and the same
// AC2ED0 constructor/CRT domain as the retained alpha owner. This adapter
// supplies operations only; the already-created owner slot adopts the result.
class BridgeAlphaEntryFactory final : public cc13_gui1::GuiTimedEntryHooks {
public:
    void timed_entry_array_resize_00aa77b0(void*, std::int32_t) override {
        throw std::logic_error("Alpha factory has no native timed-entry array");
    }
    void* operator_new_00bf681b(std::uint32_t bytes) override {
        return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    }
    void* timed_entry_construct_00ac2ed0(void* block, void* widget) override {
        auto* entry = ::new (block) GuiTimedEntryStorage;
        return &construct_gui_timed_entry_alpha_00ac2ed0(
            *entry, *static_cast<GuiLayoutWidget*>(widget), kTimedOne);
    }
    void free_failed_constructor_00bf65ac(void* block) noexcept override {
        singleton_lifetime_free(block);
    }
    void timed_entry_construction_returned(void* entry) noexcept override {
        completed_ = static_cast<GuiTimedEntryStorage*>(entry);
    }
    GuiTimedEntryStorage* completed() const noexcept { return completed_; }
private:
    GuiTimedEntryStorage* completed_{};
};

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

}  // namespace

struct GameFrontendHost::Impl {
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    GameFontHost& fonts;
    IDirect3DDevice9& device;
    const bool& widescreen; // SAME platform+0Dh, kept alive by GameStartupHost
    // SAME Source capability owned by GameStartupHost through shared drain.
    // No second hardware query/value or Original0109EEA4 state is installed.
    const bool& sse2_conversion;

    GuiPageRegistry registry;
    std::unique_ptr<VfsGuiPageScripts> page_scripts;
    std::vector<GameGuiResourceRecord> resources;
    std::vector<GamePageRecord> page_records;
    std::vector<GameWidgetRecord> widget_records;
    // Parallel to widget_records: the loaded widget each row describes. The pages live in
    // `registry` for the whole run, so these stay valid.
    std::vector<const GuiLayoutWidget*> widget_nodes;
    // 0043ABA0's CPU-owned vertex UV plane, keyed by the same live page
    // widgets as widget_nodes. Registry pages outlive this map's borrowers.
    using DigitUvs = std::array<std::array<float, 2>, 4>;
    std::map<const GuiLayoutWidget*, DigitUvs> digit_uvs;
    // Render output of 00AA7930 -> 00AA7220, over the same live widgets as
    // widget_nodes. Pages and their runtime clones outlive these borrowers.
    // Retain the input bits so an independently edited widget cannot consume
    // an earlier publication. This is derived draw state, not another widget.
    struct PublishedRotation {
        std::uint32_t input_bits;
        float angle;
    };
    std::map<const GuiLayoutWidget*, PublishedRotation> published_rotations;
    struct RotationPublication final : GuiWidgetTransformHost {
        Impl& owner;
        GuiLayoutWidget& node;
        RotationPublication(Impl& host, GuiLayoutWidget& widget) : owner(host), node(widget) {}
        bool widescreen_enabled() override { return owner.widescreen; }
        void publish_local_transform(GuiWidgetTransform& widget,
            const GuiWidgetLocalTransform& transform) override {
            if (&widget != &node.transform)
                throw std::logic_error("GUI rotation must publish its same live widget");
            PublishedRotation output;
            std::memcpy(&output.input_bits, &widget.rotate, sizeof(output.input_bits));
            std::memcpy(&output.angle, &transform.rotation_z, sizeof(output.angle));
            // The sprite bridge applies its rotation while building quads,
            // rather than owning the native widget+4Ch scene-node matrix.
            // Deliver the reconstructed rotation to that actual draw path.
            owner.published_rotations.insert_or_assign(&node, output);
            owner.quads_built = false;
        }
        void publish_local_bounds(GuiWidgetTransform&, const GuiWidgetBounds&) override {
            throw std::logic_error("GUI rotation does not refresh bounds");
        }
        void notify_layout_changed(GuiWidgetTransform&) override {
            throw std::logic_error("GUI rotation does not notify layout");
        }
        void on_layout_finished(GuiWidgetTransform&) override {
            throw std::logic_error("GUI rotation does not finish layout");
        }
    };
    float published_rotation_angle(const GuiLayoutWidget& node) const {
        std::uint32_t input_bits;
        std::memcpy(&input_bits, &node.transform.rotate, sizeof(input_bits));
        const auto found = published_rotations.find(&node);
        if (found != published_rotations.end() && found->second.input_bits == input_bits)
            return found->second.angle;
        // Authored widgets and direct layout edits have no setter publication.
        return -node.transform.rotate;
    }
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
        GuiManagerPointerState pointer{};  // the byte at +48h 00aa0e00 stores
        bool constructed{false};
        gui_manager_cc13::GuiSafeZoneState safe_zone;
    } manager;
    GameNativeGuiManagerApplication* native_manager{};  // kGuiManagerCc15Bound only
    std::set<const void*> page_handles;  // which create_gui_resource results are pages
    // Sole owner of the two returned native texture references. ManagerState
    // stores raw borrowers; bridge COM textures remain a separate draw path.
    std::unique_ptr<GameNativeGuiResourceApplication> native_gui_resources;
    struct RawResourceCall {
        RawResourceCall() {} // Ordinary lookup homes are written only at the call epoch.
        NativeString name;
        volatile std::uint32_t lookup_homes[2];
        void* manager{};void* parent{};void* result{};void* visibility_receiver{};
        bool name_live{},name_entered{},load_entered{},load_returned{},cleanup_entered{};
        bool visibility_entered{},visibility_returned{};
        bool store_entered[2]{};
        std::size_t stores{};
    };
    struct RawPointerCall {
        enum class Kind { getter, enabled, visible, cursor, safe_zone };
        Kind kind;bool requested{};
        void* manager{};void* pointer{};
        bool entered{},getter_returned{},enabled_store_entered{},enabled_stored{},visibility_entered{},normal_returned{};
        std::uint32_t site{};std::exception_ptr failure;
        explicit RawPointerCall(Kind k,bool value):kind(k),requested(value) {}
    };
    struct RawTransport {
        enum class Phase { bound, starting, complete, failed };
        GameNativeGuiPageApplication& pages;
        GameNativeGuiManagerApplication& manager;
        GameNativeGuiResourceApplication& textures;
        NativeStringRawPoolContext& strings;
        Phase phase{Phase::bound};std::uint32_t site{};std::exception_ptr failure;
        std::array<RawResourceCall,10> calls;
        std::vector<std::unique_ptr<RawPointerCall>> pointer_calls;
        bool pointer_active{};
        bool finish_entered{},finish_returned{};
        void* actual_manager{};void* group{};RawResourceCall* deferred{};
        RawTransport(GameNativeGuiPageApplication& p,GameNativeGuiManagerApplication& m,
            GameNativeGuiResourceApplication& t,NativeStringRawPoolContext& s)
            :pages(p),manager(m),textures(t),strings(s) {}
    };
    std::unique_ptr<RawTransport> raw_transport;
    GuiStartupResult run_raw_startup(GameFrontendHost&,const std::string&);
    void* run_raw_pointer(GameFrontendHost&,RawPointerCall::Kind,bool);

    // The sprite bridge.
    D3dxImageImports imports;
    std::vector<IDirect3DTexture9*> owned_textures;
    std::map<std::string, IDirect3DTexture9*> loose_textures;
    TextureAtlasParseResult atlas;
    IDirect3DTexture9* atlas_texture{};
    UINT atlas_width{};
    UINT atlas_height{};
    std::vector<BridgeQuad> quads;
    FrontendScrollerBridge scroller_bridge;
    // Milestone 2d, the text half of the bridge. The runs are built once per widget,
    // because a visibility change or a newly loaded page does not change a layout;
    // build_quads then turns the cached glyph quads into bridge quads every rebuild.
    std::unique_ptr<GameTextHost> text_host;
    std::map<const GuiLayoutWidget*, GameTextRun> text_runs;
    // Milestone 2e: the run-time 00ABAED0 sources a screen pushed into a Text
    // widget, which replace the page's authored DefaultText on the next build.
    std::map<const GuiLayoutWidget*, std::string> text_sources;
    // Goal 5 chain 667: the +100h / +0FCh stores a screen made ahead of them.
    std::map<const GuiLayoutWidget*, GameTextLayoutOverride> text_layouts;
    // The existing layout is the sole widget. Each timed allocation borrows it;
    // no raw widget/native node or second color projection is manufactured.
    std::map<const GuiLayoutWidget*, std::map<int, BridgeAlphaEntry>> alpha_entries;
    void require_alpha_widget(const GuiLayoutWidget& widget) const {
        if (std::find(widget_nodes.begin(), widget_nodes.end(), &widget) == widget_nodes.end())
            throw std::logic_error("Timed alpha requires a live widget owned by this bridge");
    }
    void require_alpha_entry(const GuiTimedEntryStorage* entry) const {
        for (const auto& widget : alpha_entries) {
            for (const auto& slot : widget.second) {
                if (slot.second.get() == entry) {
                    if (entry->widget_08 != widget.first)
                        throw std::logic_error("Timed alpha changed its borrowed widget identity");
                    require_alpha_widget(*widget.first);
                    return;
                }
            }
        }
        throw std::logic_error("Timed alpha handle is not live in this bridge");
    }
    const GameTextLayoutOverride* text_layout(const GuiLayoutWidget& widget) const {
        const auto found = text_layouts.find(&widget);
        return found != text_layouts.end() ? &found->second : nullptr;
    }
    unsigned back_buffer_width{};
    unsigned back_buffer_height{};
    bool quads_built{false};
    std::size_t last_logged_quads{static_cast<std::size_t>(-1)};
    std::size_t node_counter{0};

    // 00AA7D40 -> 00AA7220 publishes the live widget's local translation.
    // Page owners and their runtime clones outlive these borrowed keys.
    struct PublishedLocalZ {
        std::uint32_t input_bits;
        float translation;
    };
    std::map<const GuiWidgetTransform*, PublishedLocalZ> published_local_z;
    struct LocalPositionPublication final : GuiWidgetTransformHost {
        Impl& owner;
        GameFrontendHost& frontend;
        GuiLayoutWidget& receiver;
        LocalPositionPublication(Impl& host, GameFrontendHost& bridge, GuiLayoutWidget& widget)
            : owner(host), frontend(bridge), receiver(widget) {}
        bool widescreen_enabled() override { return owner.widescreen; }
        void publish_local_transform(GuiWidgetTransform& widget,
            const GuiWidgetLocalTransform& transform) override {
            if (&widget != &receiver.transform)
                throw std::logic_error("GUI local Z must publish its live receiver");
            PublishedLocalZ output;
            std::memcpy(&output.input_bits, &widget.position.z, sizeof(output.input_bits));
            std::memcpy(&output.translation, &transform.translation.z, sizeof(output.translation));
            owner.published_local_z.insert_or_assign(&widget, output);
            owner.quads_built = false;
        }
        void publish_local_bounds(GuiWidgetTransform& widget, const GuiWidgetBounds&) override {
            if (&widget != &receiver.transform)
                throw std::logic_error("GUI bounds must publish their live receiver");
            // This sprite bridge derives its rectangle from the same retained
            // widget's size/pivot instead of a native +4Ch scene-node volume.
            // Refresh the actual draw records (including the moved subtree),
            // not a second, unconsumed bounds owner.
            frontend.refresh_widget_geometry(receiver);
        }
        void notify_layout_changed(GuiWidgetTransform&) override {
            throw std::logic_error("00AA7D40 does not notify layout");
        }
        void on_layout_finished(GuiWidgetTransform&) override {
            throw std::logic_error("00AA7D40 does not finish layout");
        }
    };
    float published_resolved_z(const GuiWidgetTransform& widget) const {
        std::uint32_t input_bits;
        std::memcpy(&input_bits, &widget.position.z, sizeof(input_bits));
        const auto found = published_local_z.find(&widget);
        if (found == published_local_z.end() || found->second.input_bits != input_bits)
            return resolved_position(widget).z;
        const float local = found->second.translation;
        if (widget.parent == nullptr) return local;
        // Same live parent and Z order as 00AA67C0/00AA6798..00AA67BD.
        // Consume 00AA7220's published lane in the bridge's resolved depth;
        // each float operation rounds to PC24, including the zero bias.
        using namespace bsp::cc13x87;
        return static_cast<float>(x87_sub24(
            x87_add24(resolved_position(*widget.parent).z, local), kGuiResolvedZBias));
    }

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        GameFontHost& fonts_in, IDirect3DDevice9& device_in, const bool& widescreen_in,
        const bool& sse2_conversion_in)
        : log(log_in), vfs(vfs_in), scripts(scripts_in), fonts(fonts_in), device(device_in),
          widescreen(widescreen_in), sse2_conversion(sse2_conversion_in) {}

    ~Impl() {
        for (IDirect3DTexture9* texture : owned_textures) {
            if (texture) texture->Release();
        }
    }

    // 00bdf4c0 then 00bdf310, the same read the font resources use.
    bool read(const std::string& requested, std::shared_ptr<MemoryStream>& stream,
        std::string& error) {
        auto* manager_ptr = vfs.ready() ? &vfs.context() : nullptr;
        if (manager_ptr == nullptr) { error = "no mounted VFS"; return false; }
        std::string resolved = requested;
        if (!resolve_existing_resource_00bdf4c0_fragment(*manager_ptr,
                vfs.search_registrations(), resolved)) {
            error = "cannot resolve " + requested;
            return false;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(*manager_ptr,
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

    GuiLayoutPage* load_page(GuiLayoutHost& host, const std::string& name, bool log_tree,
        std::int32_t screen_flag = 1);
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

// 004c12b0, the double-checked singleton over DAT_00f8bc5c, constructed by
// 00aa5d70. The lifetime manager's critical section is not modelled; the
// once-only construction it protects is. Startup (0073bae0) and every later
// caller share this one get-or-create.
void* gui_manager_get_or_create_004c12b0(GameFrontendHost::Impl& owner) {
    if (!owner.manager.constructed) {
        owner.manager.constructed = true;
        owner.summary.gui_manager_created = true;
        owner.log.implemented("GuiStartupHost::gui_manager_get_or_create", "004c12b0");
    }
    return &owner.manager;
}

// A manager resource's virtual +34h. For a page that is the screen's own
// visibility byte; for a child it is the widget's. Both reach the projected
// `visible` field here; the scene-node half of the native setter does not exist.
void apply_gui_resource_visibility(GameFrontendHost::Impl& owner, void* resource,
    bool visible) {
    if (resource == nullptr) return;
    if (owner.page_handles.count(resource) != 0) {
        auto* page = static_cast<GuiLayoutPage*>(resource);
        if (page->root) {
            page->root->visible = visible;
            owner.visibility_applied.insert(page->root.get());
        }
    } else {
        auto* widget = static_cast<GuiLayoutWidget*>(resource);
        widget->visible = visible;
        owner.visibility_applied.insert(widget);
    }
}

// The manager's +50h pointer slot, which both 00aa0e00 and 00aa0e50 reach.
struct GuiManagerPointerSlot final : GuiPointerHost {
    GameFrontendHost::Impl& owner;
    explicit GuiManagerPointerSlot(GameFrontendHost::Impl& o) : owner(o) {}
    void set_front_end_pointer_visible(bool visible) override {
        apply_gui_resource_visibility(owner, owner.manager.fields[0x50 / 4], visible);
        owner.quads_built = false;
    }
};

// 00AA0E20 calls only current+34h(0). Use the same visibility provider as
// 00AA0E00/50, on the captured current cursor; the body then reloads the chosen
// +54h/+58h slot after that call. These are the manager's actual owning-domain
// slots, not a temporary cursor selection copied out before the callback.
struct GuiManagerCursorSwitch final : NativeGuiAnimatedValueCalls {
    GameFrontendHost::Impl& owner;
    GameFrontendHost::Impl::RawPointerCall* history;
    explicit GuiManagerCursorSwitch(GameFrontendHost::Impl& o,
        GameFrontendHost::Impl::RawPointerCall* h = nullptr) : owner(o), history(h) {}
    void stop_vcall_34(void* current, std::uint32_t flag) override {
        if (!current) throw std::logic_error("cursor switch requires the actual current widget");
        if (history) {
            history->pointer = current;
            history->visibility_entered = true;
            owner.raw_transport->pages.set_widget_visibility_00aa8530(current, static_cast<std::uint8_t>(flag));
        } else {
            apply_gui_resource_visibility(owner, current, flag != 0);
            owner.quads_built = false;
        }
    }
    void set_target_vcall_88(void*, std::uint32_t, std::uint32_t, float) override {
        throw std::logic_error("cursor switch does not call virtual88");
    }
    void notify_04(std::uint32_t, void*, std::uint8_t) override {
        throw std::logic_error("cursor switch does not call manager+4");
    }
};

// AA2110's current +34 calls use the very same Page visibility graph as the
// AA5E20 resource producer. Reload each frame slot at its call boundary;
// hiding 169 may change the 43 slot before the second call (00AA2165).
struct GuiManagerSafeZone final : gui_manager_cc13::GuiSafeZoneHost {
    GameFrontendHost::Impl& owner;
    GameFrontendHost::Impl::RawPointerCall* call;
    explicit GuiManagerSafeZone(GameFrontendHost::Impl& host,
        GameFrontendHost::Impl::RawPointerCall* history = nullptr) : owner(host), call(history) {}
    bool platform_widescreen() override { return owner.widescreen; }
    void set_frame_visible(gui_manager_cc13::GuiSafeZoneFrame frame, bool visible) override {
        const bool wide = frame == gui_manager_cc13::GuiSafeZoneFrame::SixteenByNine;
        if (call) {
            auto& image = *static_cast<GuiSafeZoneManagerImage*>(call->manager);
            call->pointer = wide ? image.sixteen_by_nine_80 : image.four_by_three_7c;
            call->site = visible ? 0x00aa2140 : (wide ? 0x00aa2163 : 0x00aa216f);
            call->visibility_entered = true;
            owner.raw_transport->pages.set_widget_visibility_00aa8530(call->pointer, visible ? 1 : 0);
        } else {
            apply_gui_resource_visibility(owner, owner.manager.fields[(wide ? 0x80 : 0x7c) / 4], visible);
            owner.quads_built = false;
        }
    }
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
        if constexpr (kGuiManagerCc15Bound) {
            if (owner_.native_manager) owner_.native_manager->get_004c12b0(); // 0073BC19
        }
        return gui_manager_get_or_create_004c12b0(owner_);
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
            if (owner_.native_gui_resources) {
                if (manager != &owner_.manager) {
                    throw std::logic_error("Native GUI resources require the existing frontend manager");
                }
                result = owner_.native_gui_resources->load_manager_texture_00aa5e20(
                    entry.offset, entry.name);
                const auto& operation = owner_.native_gui_resources->operation(
                    owner_.native_gui_resources->operation_count() - 1);
                record.detail = !operation.resolved ? "actual VFS rejected the name"
                    : result ? "actual renderer64/B319B0 texture reference"
                             : "actual renderer64/B319B0 returned null";
            } else {
                // Default-OFF host route through the same mounted VFS.
                IDirect3DTexture9* texture = owner_.create_texture(record.name);
                result = texture;
                record.detail = texture != nullptr ? "loaded by the sprite bridge"
                                                   : "VFS rejected the name";
                owner_.log.unimplemented("GuiManagerResources::renderer_load_texture", "00aa5e60");
            }
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
        if (resource == nullptr) return;
        apply_gui_resource_visibility(owner_, resource, visible);
        owner_.log.implemented("GuiManagerResources::set_visibility", "00aa5faf");
    }

    void clear_gui_manager_ready_flag(void* manager) override {
        auto* state = static_cast<GameFrontendHost::Impl::ManagerState*>(manager);
        if (state != nullptr) state->safe_zone.shown = false;
        owner_.log.implemented("GuiManagerResources::clear_ready_flag", "00aa6305");
    }

private:
    GameFrontendHost::Impl& owner_;
    GameGuiLayoutHost& layout_;
    std::string language_font_path_;
};

}  // namespace

GuiStartupResult GameFrontendHost::Impl::run_raw_startup(GameFrontendHost& frontend,
    const std::string& language_path) {
    auto& raw=*raw_transport;
    if(raw.phase!=RawTransport::Phase::bound || raw.pointer_active)
        throw std::logic_error("actual GUI startup is once-only and serialized");
    GuiStartupResult result;raw.phase=RawTransport::Phase::starting;
    try {
        // SAME existing font/preload operations; no semantic page/layout path.
        raw.site=0x0073bafe;result.language_font_path=language_path;
        log.implemented("GuiStartupHost::language_font_path","008d4890");
        raw.site=0x0073bb7e;(void)fonts.registry().registry();
        log.implemented("GuiStartupHost::font_registry","007371d0");
        fonts.load_descriptors_00ac3910(std::string(kFontRootPrefix),std::string(kFontDescriptorPath),language_path);
        result.font_descriptors_loaded=true;
        log.implemented("GuiStartupHost::load_font_descriptors","00ac3910");
        raw.site=0x0073bc0d;fonts.preload_fingerprint_payload_00be9760();
        log.implemented("GuiStartupHost::preload_fallback_glyph_table","0053bc00");
        fonts.bind_page_text_owner(raw.pages); // SAME producer, before the first actual Page load
        raw.site=0x0073bc19;
        raw.actual_manager=raw.manager.begin_initial_resources(frontend);
        result.gui_manager=raw.actual_manager;summary.gui_manager_created=true;
        static constexpr std::uint32_t name_sites[]={0,0,0x00aa5f4a,0x00aa5fc3,0x00aa6033,
            0x00aa60b6,0x00aa612d,0x00aa61a4,0x00aa621b,0x00aa6292};
        static constexpr std::uint32_t load_sites[]={0,0,0x00aa5f7c,0x00aa5ff5,0x00aa6065,
            0x00aa60e8,0x00aa615e,0x00aa61d5,0x00aa624c,0x00aa62c3};
        static constexpr std::uint32_t visibility_sites[]={0,0,0x00aa5fb1,0x00aa60a4,0x00aa6099,
            0x00aa611b,0x00aa6192,0x00aa6209,0x00aa6280,0x00aa62fd};
        static constexpr std::uint32_t store_sites[]={0,0,0x00aa5f87,0x00aa5ffa,0x00aa6070,
            0,0x00aa6163,0x00aa61da,0x00aa6251,0x00aa62c8};
        static constexpr std::uint32_t cleanup_sites[]={0,0,0x00aa5f9b,0x00aa6017,0x00aa6084,
            0x00aa6106,0x00aa617d,0x00aa61f4,0x00aa626b,0x00aa62e5};
        for(std::size_t i=0;i<kGuiManagerResources.size();++i) {
            const auto& entry=kGuiManagerResources[i];auto& call=raw.calls[i];
            call.manager=raw.actual_manager;
            GameGuiResourceRecord observed;observed.name=std::string(entry.name);observed.offset=entry.offset;
            if(entry.kind==GuiResourceKind::Texture) {
                observed.kind="texture";call.load_entered=true;raw.site=i==0?0x00aa5ea2:0x00aa5ef0;
                call.result=raw.textures.load_initial_texture_into_actual_manager(raw.manager,
                    raw.actual_manager,entry.offset,entry.name);
                call.load_returned=true;
                const auto& texture=raw.textures.operation(raw.textures.operation_count()-1);
                if(texture.manager_publication_entered) { ++result.stores;++summary.gui_resource_stores; }
                observed.detail=!texture.resolved?"actual VFS rejected; no +28 store":
                    texture.result?"returned actual credit transferred to manager":"accepted null published to actual manager";
            } else {
                raw.site=name_sites[i];call.name_entered=true;call.name_live=true;
                call.name.assign_0041e870(raw.strings,entry.name.data());
                raw.site=load_sites[i];call.load_entered=true;
                if(entry.kind==GuiResourceKind::Group) {
                    observed.kind="group";
                    call.result=raw.pages.load_page_00aa5840(raw.actual_manager,&call.name,1,0);
                    raw.group=call.result; // Actual returned Widget; Highlight is held only here.
                } else {
                    observed.kind="child";
                    // Original Mouse children reload manager+4C AFTER name
                    // construction. Highlight children use captured returned EDI.
                    call.parent=i==3 || i==4?
                        GameNativeGuiManagerApplication::current_resource_slot(raw.actual_manager,0x4c):raw.group;
                    call.lookup_homes[1]=1; // Genuine ordinary HOME; callee never reads it.
                    call.lookup_homes[0]=reinterpret_cast<std::uint32_t>(&call.name);
                    call.result=raw.pages.find_child_00aa7e00(call.parent,call.lookup_homes);
                }
                call.load_returned=true;
                if(entry.offset!=kGuiResourceNotStored) {
                    raw.site=store_sites[i];call.store_entered[0]=true;
                    raw.manager.store_initial_widget(frontend,raw.actual_manager,entry.offset,call.result);
                    ++call.stores;++result.stores;++summary.gui_resource_stores;
                }
                if(entry.second_offset!=kGuiResourceNotStored) {
                    raw.site=0x00aa5ffd;call.store_entered[1]=true;
                    raw.manager.store_initial_widget(frontend,raw.actual_manager,entry.second_offset,call.result);
                    ++call.stores;++result.stores;++summary.gui_resource_stores;
                }
                // Normal cleanup follows actual stores and precedes visibility.
                raw.site=cleanup_sites[i];call.cleanup_entered=true;
                destroy_native_string_header_0041dd20(&call.name,raw.strings);call.name_live=false;
                if(entry.sets_visibility && !entry.visibility_deferred) {
                    call.visibility_receiver=entry.offset==kGuiResourceNotStored?call.result:
                        GameNativeGuiManagerApplication::current_resource_slot(raw.actual_manager,entry.offset);
                    raw.site=visibility_sites[i];call.visibility_entered=true;
                    raw.pages.set_widget_visibility_00aa8530(call.visibility_receiver,entry.visibility?1:0);
                    call.visibility_returned=true;
                }
                if(raw.deferred) {
                    auto& deferred=*raw.deferred;
                    deferred.visibility_receiver=GameNativeGuiManagerApplication::current_resource_slot(raw.actual_manager,0x50);
                    raw.site=0x00aa60a4;deferred.visibility_entered=true;
                    raw.pages.set_widget_visibility_00aa8530(deferred.visibility_receiver,0);
                    deferred.visibility_returned=true;raw.deferred=nullptr;
                }
                if(entry.visibility_deferred) raw.deferred=&call;
                observed.detail="actual Page-owned load/lookup/current34; raw graph borrow";
            }
            observed.acquired=call.result!=nullptr;
            if(observed.acquired) { ++result.resources_created;++summary.gui_resources_acquired; }
            resources.push_back(std::move(observed)); // Observations confer no birth/retain authority.
        }
        raw.site=0x00aa6305;raw.finish_entered=true;
        raw.manager.finish_initial_resources(frontend,raw.actual_manager);raw.finish_returned=true;
        raw.phase=RawTransport::Phase::complete;return result;
    } catch(...) {
        raw.failure=std::current_exception();raw.phase=RawTransport::Phase::failed;
        raw.manager.fail_frontend_transport(raw.failure);
        throw; // Retain partial slots/credits/names/frames; never load_page.error.what/continue.
    }
}

void* GameFrontendHost::Impl::run_raw_pointer(GameFrontendHost& frontend,RawPointerCall::Kind kind,bool value) {
    auto& raw=*raw_transport;
    if(raw.pointer_active || raw.phase==RawTransport::Phase::starting || raw.phase==RawTransport::Phase::failed ||
        (kind!=RawPointerCall::Kind::getter && raw.phase!=RawTransport::Phase::complete))
        throw std::logic_error("actual GUI pointer operation is serialized against retained transport history");
    auto record=std::make_unique<RawPointerCall>(kind,value);auto& call=*record;
    raw.pointer_calls.push_back(std::move(record));raw.pointer_active=true;
    try {
        call.entered=true;call.site=0x004c12b0;call.manager=raw.manager.get_004c12b0();call.getter_returned=true;
        if(kind!=RawPointerCall::Kind::getter) {
            raw.manager.admit_pointer_operation(frontend,call.manager); // BEFORE any pointer Native effect.
            if(kind==RawPointerCall::Kind::cursor) {
                call.site=0x00aa0e2d;
                GuiManagerCursorSwitch binding(*this,&call);
                restart_native_gui_animated_value_00aa0e20(call.manager,value?1:0,binding);
            } else if(kind==RawPointerCall::Kind::safe_zone) {
                GuiManagerSafeZone binding(*this, &call);
                auto& image = *static_cast<GuiSafeZoneManagerImage*>(call.manager);
                gui_manager_cc13::set_safe_zone_visible_00aa2110(image.state_84, binding, value);
            } else {
                if(kind==RawPointerCall::Kind::enabled) {
                    call.enabled_store_entered=true;
                    call.site=0x00aa0e06;GameNativeGuiManagerApplication::store_enabled_48(call.manager,value?1:0);
                    call.enabled_stored=true;
                }
                if(kind==RawPointerCall::Kind::visible || !value) {
                    call.site=kind==RawPointerCall::Kind::visible?0x00aa0e50:0x00aa0e0b;
                    call.pointer=GameNativeGuiManagerApplication::current_resource_slot(call.manager,0x50);
                    call.visibility_entered=true;
                    raw.pages.set_widget_visibility_00aa8530(call.pointer,kind==RawPointerCall::Kind::visible && value?1:0);
                }
            }
        }
        call.normal_returned=true;raw.pointer_active=false;return call.manager;
    } catch(...) {
        call.failure=raw.failure=std::current_exception();raw.phase=RawTransport::Phase::failed;
        raw.manager.fail_frontend_transport(raw.failure);throw;
    }
}

GuiLayoutPage* GameFrontendHost::Impl::load_page(GuiLayoutHost& host, const std::string& name,
    bool log_tree, std::int32_t screen_flag) {
    GamePageRecord record;
    record.name = name;
    ++summary.pages_requested;
    GuiLayoutPage* page = nullptr;
    try {
        page = load_gui_page_00aa5840(registry, host, name, screen_flag, false);
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

const bool& GameFrontendHost::crt_sse2_conversion() const noexcept {
    return impl_->sse2_conversion;
}

GameFrontendHost::GameFrontendHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
    GameFontHost& fonts, IDirect3DDevice9& device, const bool& widescreen, const bool& gui_sse2_conversion)
    : impl_(std::make_unique<Impl>(log, vfs, scripts, fonts, device, widescreen,
          gui_sse2_conversion)) {}
GameFrontendHost::~GameFrontendHost() = default;

void GameFrontendHost::bind_native_gui_resources(GameNativeRendererApplication& renderer) {
    if (!GameNativeGuiResourceApplication::requested()) return;
    auto& host = *impl_;
    if (host.native_gui_resources || host.summary.gui_startup_ran || host.manager.constructed ||
        host.manager.fields[0x28 / 4] || host.manager.fields[0x2c / 4]) {
        throw std::logic_error("Native GUI resource binding requires the empty initial frontend manager");
    }
    host.native_gui_resources = std::make_unique<GameNativeGuiResourceApplication>(
        host.log, renderer, host.vfs);
}

void GameFrontendHost::bind_native_gui_manager(GameNativeGuiManagerApplication& manager) {
    auto& host = *impl_;
    if (host.native_manager || host.summary.gui_startup_ran || host.manager.constructed)
        throw std::logic_error("Native GUI manager binding requires the empty initial frontend manager");
    host.native_manager = &manager;
}
void GameFrontendHost::bind_native_gui_pages(GameNativeGuiPageApplication& pages,
    GameNativeGuiManagerApplication& manager) {
    auto& host=*impl_;
    if(host.raw_transport || host.summary.gui_startup_ran || host.manager.constructed ||
        host.native_manager!=&manager || !host.native_gui_resources ||
        host.native_gui_resources->operation_count()!=0)
        throw std::logic_error("actual Page transport binds once before frontend startup with the same actual manager/resources");
    // Nonmoving metadata is owned BEFORE cross-binding. Even a failed bind
    // leaves the entered tuple retained and cannot silently select legacy.
    host.raw_transport=std::make_unique<Impl::RawTransport>(pages,manager,*host.native_gui_resources,
        host.vfs.raw_strings());
    try {
        host.native_gui_resources->require_actual_manager_domain(manager);
        manager.bind_frontend_transport(*this,pages,*host.native_gui_resources,host.vfs.raw_strings());
    }
    catch(...) {
        host.raw_transport->failure=std::current_exception();
        host.raw_transport->phase=Impl::RawTransport::Phase::failed;throw;
    }
}
void GameFrontendHost::retire_native_gui_resources() {
    auto& host = *impl_;
    if (!host.native_gui_resources) return;
    if (host.native_gui_resources->failure() || (host.raw_transport &&
        (host.raw_transport->phase==Impl::RawTransport::Phase::starting ||
         host.raw_transport->phase==Impl::RawTransport::Phase::failed || host.raw_transport->pointer_active))) {
        throw std::logic_error("Failed native GUI resources require process retention without manager mutation");
    }
    // End both projected manager borrowers before any terminal can reenter.
    if(!host.raw_transport) {
        host.manager.fields[0x28 / 4] = nullptr;
        host.manager.fields[0x2c / 4] = nullptr;
    }
    host.native_gui_resources->retire_textures();
}
bool GameFrontendHost::native_gui_resources_require_process_retention() const noexcept {
    if(impl_->raw_transport && (impl_->raw_transport->phase==Impl::RawTransport::Phase::starting ||
        impl_->raw_transport->phase==Impl::RawTransport::Phase::failed || impl_->raw_transport->pointer_active)) return true;
    return impl_->native_gui_resources && impl_->native_gui_resources->requires_process_retention();
}

void GameFrontendHost::run_font_and_gui_startup_0073bae0(const std::string& language_font_path) {
    Impl& host = *impl_;
    try {
    GuiStartupResult result;
    if(host.raw_transport) {
        result=host.run_raw_startup(*this,language_font_path);
    } else {
        host.page_scripts = std::make_unique<VfsGuiPageScripts>(host.scripts.files(),
            host.scripts.runtime(), host.scripts.globals());
        GameGuiLayoutHost layout(host, *host.page_scripts);
        GameGuiStartupHost startup(host, layout, language_font_path);
        result=run_gui_startup(startup);
    }
    host.summary.gui_startup_ran = true;
    host.summary.font_descriptors_loaded = result.font_descriptors_loaded;
    for (const auto& font : host.fonts.registry().fonts()) {
        host.log.notef("font registered %-12s data=%-16s gfx=%-16s alpha=%-12s scale=%.4f "
            "upper=%d resources=%d", font->descriptor.name.c_str(),
            font->descriptor.data_file.c_str(), font->descriptor.gfx_file.c_str(),
            font->descriptor.alpha_texture.c_str(),
            static_cast<double>(font->descriptor.scale_ratio),
            font->descriptor.uppercase_only ? 1 : 0,
            font->resources || (font->native_resources && font->native_resources->live()) ? 1 : 0);
    }
    host.log.notef("gui startup language_font_path=%s fonts=%zu resources=%zu stores=%zu "
        "manager=%p", result.language_font_path.empty() ? "(none)"
            : result.language_font_path.c_str(), host.fonts.registry().fonts().size(),
        result.resources_created, result.stores, result.gui_manager);
    host.log.implemented("Phase 7 fonts_and_gui_startup", "0073bae0");
    } catch(...) {
        if(host.raw_transport) {
            host.raw_transport->failure=std::current_exception();
            host.raw_transport->phase=Impl::RawTransport::Phase::failed;
            host.raw_transport->manager.fail_frontend_transport(host.raw_transport->failure);
        }
        throw;
    }
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
            source != text_sources.end() ? &source->second : nullptr, text_layout(node));
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
        // by construction and is re-read here. Other authored widgets keep
        // the page-load values except the explicitly bound scroller controls.
        if (node != nullptr && record.runtime) {
            const GuiWidgetPoint point = resolved_point(*node);
            record.x = point.x;
            record.y = point.y;
            record.z = published_resolved_z(node->transform);
            record.width = node->transform.size.width;
            record.height = node->transform.size.height;
            record.pivot_x = node->transform.pivot_x;
            record.pivot_y = node->transform.pivot_y;
        }
        // 006834A0 changes authored controls too. Refresh the exact
        // records whose dimensions feed the emitted quad corners below.
        scroller_bridge.refresh(record, node);
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
        // pass wrote. Other authored widgets keep milestone 2b's constant white and
        // its axis-aligned quad, so no earlier capture moves. Honouring the
        // authored Color and Rotate on every quad is bridge work this milestone
        // does not do; see the follow-ups.
        const D3DCOLOR color = [&] {
            return scroller_bridge.color(record, node);
        }();
        BridgeVertex top_left{left, top, 0.0f, 1.0f, color, u1, v1};
        BridgeVertex top_right{right, top, 0.0f, 1.0f, color, u2, v1};
        BridgeVertex bottom_left{left, bottom, 0.0f, 1.0f, color, u1, v2};
        BridgeVertex bottom_right{right, bottom, 0.0f, 1.0f, color, u2, v2};
        {
            // Keep the authored dimensions above: the digit routine changes
            // the sampled strip, not the geometry's width/height.
            const auto found = digit_uvs.find(node);
            if (found != digit_uvs.end()) {
                const auto& uv = found->second;
                top_left.u = uv[0][0]; top_left.v = uv[0][1];
                top_right.u = uv[1][0]; top_right.v = uv[1][1];
                bottom_left.u = uv[2][0]; bottom_left.v = uv[2][1];
                bottom_right.u = uv[3][0]; bottom_right.v = uv[3][1];
            }
        }
        if (node != nullptr && record.runtime && node->transform.rotate != 0.0f) {
            // The authored Rotate reaches the native Z rotation builder 00AA7250
            // negated (bsp/gui_widget.hpp), and the GUI y axis points down, so a
            // widget whose texture points along +x turns toward its own heading
            // at -rotate. The rotation is about the widget's own pivot, which is
            // where the resolved position sits.
            const float pivot_x = record.x * screen_w;
            const float pivot_y = record.y * screen_h;
            const float angle = [&] {
                return published_rotation_angle(*node);
            }();
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

GuiLayoutPage* GameFrontendHost::load_screen_page(const std::string& name, std::int32_t screen_flag) {
    Impl& host = *impl_;
    if (!host.page_scripts) {
        throw std::logic_error("A screen page requires the GUI startup phase");
    }
    GameGuiLayoutHost layout(host, *host.page_scripts);
    const std::size_t before = host.widget_records.size();
    GuiLayoutPage* page;
    page = host.load_page(layout, name, true, screen_flag);
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

GuiLayoutPage* GameFrontendHost::find_page_00aa3140(std::string_view name) noexcept {
    // The registry 00aa5840 registers every page into (Impl::load_page) is the
    // manager's +14h vector; GuiPageRegistry::find_00aa3140 is its scan.
    return impl_->registry.find_00aa3140(name);
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

bool GameFrontendHost::widget_own_visible(const GuiLayoutWidget& widget) const {
    // bridge_visible's test for one node, without its walk up the parents.
    if (impl_->visibility_applied.count(&widget) != 0) return widget.visible;
    if (widget.source != nullptr && widget.source->find("Visible") != nullptr) return widget.visible;
    return true;
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

GuiTimedEntryStorage* GameFrontendHost::widget_alpha_animation(GuiLayoutWidget& widget, int slot) {
    Impl& host = *impl_;
    host.require_alpha_widget(widget);
    // 00AD3A80: indices 0/1 construct an AC2ED0 alpha entry; index 2 is a
    // Section entry, outside this explicitly alpha-only bridge owner.
    if (slot < 0 || slot > 1)
        throw std::logic_error("Bridge alpha animation requires native index 0 or 1");
    auto& entry = host.alpha_entries[&widget][slot];
    if (!entry) {
        // bsp.py show 00AD3A80: indices 0/1 allocate 14h, call AC2ED0,
        // free the captured block on constructor failure and return the entry.
        // Replace the former inline producer; subsequent updates/deletion still
        // consume this sole entry through the existing bridge owner.
        BridgeAlphaEntryFactory factory;
        auto* created = static_cast<GuiTimedEntryStorage*>(
            cc13_gui1::gui_timed_entry_create_by_index_00ad3a80(factory, slot, &widget));
        if (!created) throw std::bad_alloc();
        if (factory.completed() != created) std::terminate();
        entry.reset(created);
    }
    return entry.get();
}

void GameFrontendHost::stop_widget_alpha_animation(GuiLayoutWidget& widget, int slot) {
    Impl& host = *impl_;
    host.require_alpha_widget(widget);
    const auto found = host.alpha_entries.find(&widget);
    if (found == host.alpha_entries.end()) return;
    // AA8B80: delete the existing slot's entry, then clear the pointer. Erasing
    // this owner slot performs the same profile reset/free once.
    found->second.erase(slot);
    if (found->second.empty()) host.alpha_entries.erase(found);
}

void GameFrontendHost::set_widget_alpha_interval(GuiTimedEntryStorage* entry, float target, float rate) {
    if (!entry) return; // The HUD may have no loaded WeaponInfo widget.
    impl_->require_alpha_entry(entry);
    std::uint32_t target_bits, rate_bits;
    std::memcpy(&target_bits, &target, sizeof(target_bits));
    std::memcpy(&rate_bits, &rate, sizeof(rate_bits));
    cc13_seg49_64_78::GuiHudTickerRange view{entry->target_0c, entry->rate_10};
    // bsp.py show AC2F20: MOV [this+C],arg1; MOV [this+10],arg2, RET8.
    cc13_seg49_64_78::hud_ticker_set_range_00ac2f20(view, target_bits, rate_bits);
}

void GameFrontendHost::set_widget_alpha_delay(GuiTimedEntryStorage* entry, float seconds) {
    if (!entry) return;
    impl_->require_alpha_entry(entry);
    entry->remaining_04 = seconds; // 00649245: the caller's 1.0f delay store.
}

void GameFrontendHost::advance_widget_alpha_animations(float seconds) {
    Impl& host = *impl_;
    struct AlphaCalls final : GuiTimedEntryAlphaCalls {
        GameFrontendHost& frontend;
        GuiLayoutWidget& widget;
        AlphaCalls(GameFrontendHost& f, GuiLayoutWidget& w) : frontend(f), widget(w) {}
        void require_widget(const GuiLayoutWidget& current) const {
            if (&current != &widget)
                throw std::logic_error("Bridge alpha dispatch changed its live widget");
        }
        const float* read_color54(GuiLayoutWidget& current, float (&)[4]) override {
            require_widget(current);
            return &current.color[0];
        }
        void set_alpha4c(GuiLayoutWidget& current, float alpha) override {
            require_widget(current);
            frontend.set_widget_color(current, current.color[0], current.color[1], current.color[2], alpha);
        }
    };
    const GuiTimedEntryConstants constants{kTimedOne, kTimedZero};
    for (auto widget = host.alpha_entries.begin(); widget != host.alpha_entries.end();) {
        host.require_alpha_widget(*widget->first);
        auto& slots = widget->second;
        for (auto slot = slots.begin(); slot != slots.end();) {
            // Like AA87B0's pointer walk, an empty slot has no current8.
            // It can remain empty if allocation escaped before publication.
            if (!slot->second) { ++slot; continue; }
            auto& entry = *slot->second;
            host.require_alpha_entry(&entry);
            AlphaCalls calls(*this, *entry.widget_08);
            // Same frame delta, retained +04 delay, current widget color and
            // AC2F20's actual +0C/+10 cells. AD39A0 owns the countdown and
            // current8 dispatch; AC2F40 owns all fade arithmetic/clamping.
            if (advance_gui_timed_entry_00ad39a0(entry, calls, seconds, entry.widget_08, constants))
                ++slot;
            else slot = slots.erase(slot);
        }
        if (slots.empty()) widget = host.alpha_entries.erase(widget);
        else ++widget;
    }
}

void GameFrontendHost::set_widget_text_source(GuiLayoutWidget& widget, std::string source) {
    Impl& host = *impl_;
    host.text_sources[&widget] = std::move(source);
    host.text_runs.erase(&widget);
    host.quads_built = false;
}

const std::string* GameFrontendHost::widget_text_source(const GuiLayoutWidget& widget) const {
    const auto found = impl_->text_sources.find(const_cast<GuiLayoutWidget*>(&widget));
    return found != impl_->text_sources.end() ? &found->second : nullptr;
}

void GameFrontendHost::set_widget_text_align(GuiLayoutWidget& widget, GuiTextAlign align) {
    if (widget.type != GuiWidgetType::Text)
        throw std::logic_error("Text +100h stored on a widget that is not a Text");
    Impl& host = *impl_;
    // An unchanged store leaves the cached run, which was built with it.
    std::optional<GuiTextAlign>& stored = host.text_layouts[&widget].align;
    if (stored == align) return;
    stored = align;
    host.text_runs.erase(&widget);
    host.quads_built = false;
}

void GameFrontendHost::set_widget_text_multiline(GuiLayoutWidget& widget, bool multiline) {
    if (widget.type != GuiWidgetType::Text)
        throw std::logic_error("Text +0FCh stored on a widget that is not a Text");
    Impl& host = *impl_;
    std::optional<bool>& stored = host.text_layouts[&widget].multiline;
    if (stored == multiline) return;
    stored = multiline;
    host.text_runs.erase(&widget);
    host.quads_built = false;
}

bool GameFrontendHost::widget_text_width(GuiLayoutWidget& widget, float& width) {
    // The run widget_text_height builds or finds; measured_width is the layout's
    // own result (src/game_hosts_text.cpp, from 00ABA270 / 00AB9FD0).
    float height = 0.0f;
    static_cast<void>(widget_text_height(widget, height));
    const auto cached = impl_->text_runs.find(&widget);
    if (cached == impl_->text_runs.end()) return false;
    width = cached->second.measured_width;
    return true;
}

void GameFrontendHost::set_widget_size(GuiLayoutWidget& widget, float width, float height) {
    widget.transform.size.width = width;   // 00AA7974..00AA797C, FLD/FSTP float
    widget.transform.size.height = height;
    if (widget.type == GuiWidgetType::Text) impl_->text_runs.erase(&widget);  // 00ABBF3F 00ABB1D0
    refresh_widget_geometry(widget);       // 00AA797F 00AA7220
}

void GameFrontendHost::set_widget_scale(GuiLayoutWidget& widget, float x, float y) {
    // GUI counter slot +48h reaches this with the actual retained layout and
    // the pair read by HudCountersLifecycleHost/HudCountersTableHost. Keep
    // this bridge's geometry publication as the recompose callback; no native
    // node identity is manufactured for the sprite bridge's layout object.
    struct ScaleView final : gui_widgets_cc13::GuiWidgetScaleView {
        ScaleView(GameFrontendHost& frontend, GuiLayoutWidget& widget)
            : GuiWidgetScaleView(widget.transform.scale_x, widget.transform.scale_y),
                frontend(frontend), widget(widget) {}
        void recompose_00aa7220() override { frontend.refresh_widget_geometry(widget); }
        GameFrontendHost& frontend;
        GuiLayoutWidget& widget;
    } view{*this, widget};
    const float scale[2] = {x, y};
    gui_widgets_cc13::gui_widget_set_scale_00aa7950(view, scale);
}

bool GameFrontendHost::widget_text_height(GuiLayoutWidget& widget, float& height) {
    Impl& host = *impl_;
    if (!host.text_host || widget.type != GuiWidgetType::Text) return false;
    auto cached = host.text_runs.find(&widget);
    if (cached == host.text_runs.end()) {
        // Reuse the render cache, including its current runtime source. The
        // next draw consumes this run rather than rebuilding the text path.
        const auto node = std::find(host.widget_nodes.begin(), host.widget_nodes.end(), &widget);
        if (node == host.widget_nodes.end()) return false;
        const auto index = static_cast<std::size_t>(node - host.widget_nodes.begin());
        const auto position = resolved_position(widget.transform);
        const auto& transform = widget.transform;
        GameTextRun run;
        const auto source = host.text_sources.find(&widget);
        host.text_host->build_run(host.widget_records[index].page, widget,
            position.x - transform.pivot_x * transform.size.width,
            position.y - transform.pivot_y * transform.size.height, run,
            source != host.text_sources.end() ? &source->second : nullptr,
            host.text_layout(widget));
        cached = host.text_runs.emplace(&widget, std::move(run)).first;
        host.summary.text_widgets = host.text_host->summary().text_widgets;
        host.summary.text_runs = host.text_host->summary().runs_built;
        host.summary.text_glyphs = host.text_host->summary().glyph_quads;
    }
    if (!cached->second.normalized_height) return false;
    height = *cached->second.normalized_height;
    return true;
}

void GameFrontendHost::bind_scroller_controls(const FrontEndScreenScroller& scroller) {
    impl_->scroller_bridge.bind(scroller);
    impl_->quads_built = false;
}

GuiWidgetPoint GameFrontendHost::scroller_cursor() {
    if (!impl_->native_manager)
        throw std::logic_error("front-end drag requires the application's actual GUI manager");
    const auto* manager = static_cast<const std::byte*>(impl_->native_manager->get_004c12b0());
    GuiWidgetPoint cursor;
    std::memcpy(&cursor.x, manager + 0x5c, sizeof cursor.x);
    std::memcpy(&cursor.y, manager + 0x60, sizeof cursor.y);
    return cursor;
}

namespace {
// A typed CPU vertex-stream view for the *existing* 0043ABA0 body. Input
// +0C..+18 is the loaded icon's atlas rectangle. The output buffer is the
// same four-vertex UV plane consumed by build_quads; no GPU/native object is
// claimed here. Lock/unlock enforce the owned CPU buffer's access interval.
class ShipDigitIconBinding final : public bsp::cc13_sweepA2::HudDigitIconHost {
public:
    using Uvs = std::array<std::array<float, 2>, 4>;
    ShipDigitIconBinding(GuiLayoutWidget& widget, const TextureAtlasItem& item)
        : widget_(widget) {
        std::copy(std::begin(item.uv), std::end(item.uv), icon_.uv);
        stream_.vertices = uv.data();
    }
    void crt_itoa_00bf9010(std::int32_t value, char* buffer, std::int32_t radix) override {
        // 0043ABA0 owns a 25-byte buffer and passes buffer+8 to _itoa.
        if (_itoa_s(value, buffer, 17, radix) != 0) {
            throw std::logic_error("HUD digit decimal conversion failed");
        }
    }
    void* vcall_widget_vertex_stream_a0(void* widget) override {
        require_widget(widget);
        return &stream_;
    }
    void* vcall_widget_icon_record_9c(void* widget, std::int32_t index) override {
        require_widget(widget);
        if (index != 0) throw std::logic_error("HUD digit icon state is not zero");
        return &icon_;
    }
    void vcall_vertex_stream_lock_10(void* stream, std::int32_t a,
        std::int32_t b, std::int32_t c) override {
        if (stream != &stream_ || locked_ || a != 0 || b != 0 || c != 0) {
            throw std::logic_error("HUD digit CPU stream lock mismatch");
        }
        locked_ = true;
    }
    void vcall_vertex_stream_unlock_14(void* stream) override {
        if (stream != &stream_ || !locked_) {
            throw std::logic_error("HUD digit CPU stream unlock mismatch");
        }
        locked_ = false;
    }
    Uvs uv{};

private:
    struct Icon {
        std::uint8_t unused[0x0c]{};
        float uv[4]{};
    } icon_;
    struct Stream {
        std::uint8_t unused_00[8]{};
        std::int32_t offset{};
        std::int32_t stride{sizeof(Uvs::value_type)};
        std::uint8_t unused_10[0x18]{};
        void* vertices{};
    } stream_;
    static_assert(offsetof(Icon, uv) == 0x0c);
    static_assert(offsetof(Stream, vertices) == 0x28);
    static_assert(offsetof(Stream, offset) == 8 && offsetof(Stream, stride) == 0x0c);
    void require_widget(void* widget) const {
        if (widget != &widget_) throw std::logic_error("HUD digit widget mismatch");
    }
    GuiLayoutWidget& widget_;
    bool locked_{};
};
} // namespace

bool GameFrontendHost::roll_widget_digit(GuiLayoutWidget& widget, int digit, float magnitude) {
    if (widget.source == nullptr || widget.type != GuiWidgetType::Icon) return false;
    const std::string texture = first_state_texture(*widget.source);
    if (texture.empty()) return false;
    const std::string key = atlas_key(texture);
    const TextureAtlasItem* item = find_texture_atlas_item_00aefb20(impl_->atlas.items, key.c_str());
    if (item == nullptr || item->texture == nullptr) return false;
    ShipDigitIconBinding binding(widget, *item);
    bsp::cc13_sweepA2::hud_digit_icon_roll_uv_0043aba0(digit, &widget, magnitude, binding);
    impl_->digit_uvs[&widget] = binding.uv;
    impl_->quads_built = false;
    return true;
}

void GameFrontendHost::invalidate_bridge() { impl_->quads_built = false; }

void* GameFrontendHost::gui_manager_004c12b0() {
    if(impl_->raw_transport) return impl_->run_raw_pointer(*this,Impl::RawPointerCall::Kind::getter,false);
    return gui_manager_get_or_create_004c12b0(*impl_);
}

// Callers reach both through 004c12b0, so the manager is constructed first.
void GameFrontendHost::gui_manager_set_enabled_00aa0e00(bool enabled) {
    if(impl_->raw_transport) {
        impl_->run_raw_pointer(*this,Impl::RawPointerCall::Kind::enabled,enabled);return;
    }
    gui_manager_get_or_create_004c12b0(*impl_);
    GuiManagerPointerSlot slot(*impl_);
    bsp::gui_manager_set_enabled_00aa0e00(impl_->manager.pointer, slot, enabled);
}

void GameFrontendHost::gui_manager_set_pointer_visible_00aa0e50(bool visible) {
    if(impl_->raw_transport) {
        impl_->run_raw_pointer(*this,Impl::RawPointerCall::Kind::visible,visible);return;
    }
    gui_manager_get_or_create_004c12b0(*impl_);
    GuiManagerPointerSlot slot(*impl_);
    bsp::gui_manager_set_pointer_visible_00aa0e50(slot, visible);
}

void GameFrontendHost::select_gui_cursor(bool frontend_cursor) {
    if (impl_->raw_transport) {
        impl_->run_raw_pointer(*this, Impl::RawPointerCall::Kind::cursor, frontend_cursor);
        return;
    }
    gui_manager_get_or_create_004c12b0(*impl_);
    GuiManagerCursorSwitch binding(*impl_);
    restart_native_gui_animated_value_00aa0e20(
        impl_->manager.fields, frontend_cursor ? 1 : 0, binding);
}

void GameFrontendHost::set_safe_zone_visible(bool visible) {
    if (!impl_->summary.gui_startup_ran)
        throw std::logic_error("safe-zone frames require completed 00AA5E20 resources");
    if (impl_->raw_transport) {
        impl_->run_raw_pointer(*this, Impl::RawPointerCall::Kind::safe_zone, visible);
    } else {
        gui_manager_get_or_create_004c12b0(*impl_);
        GuiManagerSafeZone binding(*impl_);
        gui_manager_cc13::set_safe_zone_visible_00aa2110(impl_->manager.safe_zone, binding, visible);
    }
    impl_->log.implemented("OnInitOnce::set_safe_zone_visible", "00aa2110");
}

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
    clone->transform.children.clear();
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
        clone->transform.children.push_back(&copy->transform);
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

// clone_widget_tree keeps the child order, so the two trees walk in step.
void mirror_applied_visibility(GameFrontendHost::Impl& host, const GuiLayoutWidget& source,
    const GuiLayoutWidget& clone) {
    if (host.visibility_applied.count(&source) != 0) host.visibility_applied.insert(&clone);
    std::size_t c = 0;
    for (const auto& child : source.children) {
        if (!child) continue;
        if (c >= clone.children.size()) break;
        mirror_applied_visibility(host, *child, *clone.children[c]);
        ++c;
    }
}

// 00AAB4C0's per-widget 00AA6560 builds each copy with its own scene node
// (00AA6640: 00B74EB0 + 00B75030), which this process does not allocate. The
// copies take the loader's diagnostic node ids instead, never zero, so a later
// 00AA7E00 finds them by name as it finds the page's own widgets.
void assign_clone_nodes(GameFrontendHost::Impl& host, GuiLayoutWidget& widget) {
    host.log.unimplemented("GuiLayoutHost::allocate_model_slot", "00b74eb0");
    host.log.unimplemented("GuiLayoutHost::create_widget_node", "00b75030");
    widget.node_id = static_cast<std::uint32_t>(++host.node_counter);
    for (const auto& child : widget.children) {
        if (child) assign_clone_nodes(host, *child);
    }
}

}  // namespace

GuiLayoutWidget* GameFrontendHost::clone_page_widget_subtree(const std::string& page,
    const GuiLayoutWidget& source) {
    Impl& host = *impl_;
    // 00AAB4C0 with a null parent_clone attaches the copy under the source's +70h
    // (bsp.py show 00aab4c0, src/cc13_gui1.cpp); a parentless source would leave
    // the copy detached, which no caller of this binding has.
    if (source.parent == nullptr)
        throw std::logic_error("00AAB4C0 clone of a widget with no parent");
    GuiLayoutWidget& parent = *source.parent;
    std::unique_ptr<GuiLayoutWidget> clone = clone_widget_tree(source, source.key);
    clone->parent = &parent;
    clone->transform.parent = &parent.transform;
    GuiLayoutWidget* borrowed = clone.get();
    parent.children.push_back(std::move(clone));
    assign_clone_nodes(host, *borrowed);
    parent.transform.children.push_back(&borrowed->transform);
    mirror_applied_visibility(host, source, *borrowed);
    host.record_widgets(page, *borrowed, 0, parent.key, true);
    host.quads_built = false;
    return borrowed;
}

void GameFrontendHost::reparent_runtime_widget(GuiLayoutWidget& widget, GuiLayoutWidget& parent) {
    Impl& host = *impl_;
    GuiLayoutWidget* const old_parent = widget.parent;
    if (old_parent == &parent) return;
    if (old_parent == nullptr)
        throw std::logic_error("00AAA5A0 clone has no owning parent");
    for (GuiLayoutWidget* ancestor = &parent; ancestor; ancestor = ancestor->parent) {
        if (ancestor == &widget) throw std::logic_error("00AAA5A0 would create a widget cycle");
    }
    const auto owned = std::find_if(old_parent->children.begin(), old_parent->children.end(),
        [&widget](const auto& child) { return child.get() == &widget; });
    if (owned == old_parent->children.end())
        throw std::logic_error("00AAA5A0 clone is not owned by its parent");
    // Reserve before detaching so allocation failure preserves ownership.
    parent.children.reserve(parent.children.size() + 1);
    parent.transform.children.reserve(parent.transform.children.size() + 1);
    auto clone = std::move(*owned);
    old_parent->children.erase(owned);
    auto& old_transforms = old_parent->transform.children;
    old_transforms.erase(std::remove(old_transforms.begin(), old_transforms.end(), &widget.transform),
        old_transforms.end());
    widget.parent = &parent;
    widget.transform.parent = &parent.transform;
    parent.children.push_back(std::move(clone));
    parent.transform.children.push_back(&widget.transform);
    for (std::size_t i = 0; i < host.widget_nodes.size(); ++i) {
        if (host.widget_nodes[i] == &widget) host.widget_records[i].parent = parent.key;
    }
    refresh_widget_geometry(widget);
    host.quads_built = false;
}

void GameFrontendHost::destroy_runtime_widget(GuiLayoutWidget& widget) {
    Impl& host = *impl_;
    GuiLayoutWidget* const parent = widget.parent;
    if (parent == nullptr)
        throw std::logic_error("widget +4h(1) on a run-time widget with no parent");
    const auto owned = std::find_if(parent->children.begin(), parent->children.end(),
        [&widget](const auto& child) { return child.get() == &widget; });
    if (owned == parent->children.end())
        throw std::logic_error("widget +4h(1) on a widget its parent does not own");
    std::set<const GuiLayoutWidget*> subtree;
    std::vector<const GuiLayoutWidget*> walk{&widget};
    while (!walk.empty()) {
        const GuiLayoutWidget* const w = walk.back();
        walk.pop_back();
        subtree.insert(w);
        for (const auto& child : w->children) {
            if (child) walk.push_back(child.get());
        }
    }
    // Every bridge row of the subtree must be a run-time clone's (record_widgets
    // marks clone_page_widget_subtree's rows), checked before anything moves.
    for (std::size_t i = 0; i < host.widget_nodes.size(); ++i) {
        if (subtree.count(host.widget_nodes[i]) != 0 && !host.widget_records[i].runtime)
            throw std::logic_error("widget +4h(1) on a page's authored widget");
    }
    std::vector<GameWidgetRecord> records;
    std::vector<const GuiLayoutWidget*> nodes;
    records.reserve(host.widget_records.size());
    nodes.reserve(host.widget_nodes.size());
    for (std::size_t i = 0; i < host.widget_nodes.size(); ++i) {
        if (subtree.count(host.widget_nodes[i]) != 0) continue;
        records.push_back(std::move(host.widget_records[i]));
        nodes.push_back(host.widget_nodes[i]);
    }
    host.widget_records = std::move(records);
    host.widget_nodes = std::move(nodes);
    for (const GuiLayoutWidget* const w : subtree) {
        host.alpha_entries.erase(w); // End borrowers before freeing the widget.
        host.digit_uvs.erase(w);
        host.published_rotations.erase(w);
        host.text_runs.erase(w);
        host.text_sources.erase(w);
        host.text_layouts.erase(w);
        host.visibility_applied.erase(w);
        host.published_local_z.erase(&w->transform);
    }
    // 00AA9730's detach (the 00AA83A0 list edit, src/gui_widget_detach.cpp): the
    // parent drops the child, whose sole owner then frees the subtree.
    auto& transforms = parent->transform.children;
    transforms.erase(std::remove(transforms.begin(), transforms.end(), &widget.transform),
        transforms.end());
    parent->children.erase(owned);
    host.quads_built = false;
}

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
    widget.transform.authored_x = x;
    // Goal 5 chain 801. bsp.py show 00AA7DC0 and 00AA7D40 --asm:
    // the local-position setter's tail is the same Z store, 00AA7220,
    // then 00AA70E0 as 00AA7D40, after X/Y have been supplied above.
    // Reuse that body over this live widget; its publication replaces the
    // former Z-only call, so the transform is recomposed exactly once.
    // The caller supplies the retained page/runtime widget and its actual XYZ.
    Impl::LocalPositionPublication publication(host, *this, widget);
    bsp::gui_widgets_cc13::gui_widget_set_local_z_and_bounds_00aa7d40(
        widget.transform, z, publication);
}

void GameFrontendHost::set_widget_rotation(GuiLayoutWidget& widget, float radians) {
    Impl& host = *impl_;
    if (widget.transform.rotate == radians) return;
    // 00AA7930 stores the actual requested angle then calls 00AA7220.
    // Keep the bridge's existing unchanged-angle gate; the publication
    // below replaces its old store/cache invalidation exactly once.
    Impl::RotationPublication publication(host, widget);
    bsp::gui_widgets_cc13::gui_widget_set_rotate_00aa7930(widget.transform,
        radians, publication);
}

void GameFrontendHost::refresh_widget_geometry(GuiLayoutWidget& widget) {
    Impl& host = *impl_;
    for (std::size_t i = 0; i < host.widget_nodes.size(); ++i) {
        const GuiLayoutWidget* node = host.widget_nodes[i];
        if (node == nullptr) continue;
        const GuiWidgetTransform* ancestor = &node->transform;
        while (ancestor && ancestor != &widget.transform) ancestor = ancestor->parent;
        if (ancestor == nullptr) continue;
        auto& record = host.widget_records.at(i);
        const auto point = resolved_point(*node);
        record.x = point.x;
        record.y = point.y;
        record.z = point.z;
        record.width = node->transform.size.width;
        record.height = node->transform.size.height;
        record.pivot_x = node->transform.pivot_x;
        record.pivot_y = node->transform.pivot_y;
        // Text runs cache their origin; a moved parent invalidates it too.
        host.text_runs.erase(node);
    }
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
