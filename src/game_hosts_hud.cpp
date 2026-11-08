// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages.
// See include/bsp/game_hosts_hud.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/cc13_left1.hpp"
#include "bsp/ship_ai_path_corridor.hpp"
#include "bsp/profile_seg2_cc13.hpp"
#include "bsp/native_camera_matrix_copy.hpp"
#include "bsp/oct18_tail_a.hpp"
#include "bsp/frontend_screens_cc13.hpp"
#include "bsp/frontend_seg20_cc13.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/game_hosts_powerups.hpp"
#include "bsp/ai_close_attack_tick.hpp"
#include "bsp/unit_damage.hpp"
#include "bsp/math.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/gui_aspect_extent.hpp"
#include "bsp/platform_window.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hint_passes.hpp"
#include "bsp/controlled_unit.hpp"
#include "bsp/interface_runtime_tail.hpp"
#include "bsp/hud_movie_camera.hpp"
#include "bsp/unit_instance_layout.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/seg10_cc13.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/hud_ship_screen.hpp"
#include "bsp/hud_warning_screen.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/hud_markers_runtime.hpp"
#include "bsp/camera_transform.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_text.hpp"            // 00ABB877, the counters' Text Align
#include "bsp/gui_widget.hpp"          // 00AA6750 / 00AA8240 arithmetic
#include "bsp/band_0050_cc13.hpp"      // 005292E0 / 005290D0 / 00529350
#include "bsp/hud_counters_screen.hpp" // 0052BAA0 / 00528C40 / 00528C50
#include "bsp/session_context_cc13.hpp"
#include "bsp/world_ocean.hpp"         // 004B6260
#include "bsp/ocean_height.hpp"
#include "bsp/ocean_wave_field.hpp"
#include "bsp/ocean_spectrum.hpp"
#include "bsp/ship_class_fields.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/gunnery_tables.hpp"      // kInstalledLosTargetHeightAdd (cc13-aimunit3)
#include "bsp/unit_gunnery_pass.hpp"   // kUnitGunneryKindShipBase (cc13-aimunit3)
#include "bsp/game_hosts_world.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_gunnery.hpp"
#include "bsp/frontier_wiring_cc13.hpp"
#include "bsp/game_hosts_session_messages.hpp"
#include "bsp/native_session_message_participant_owner.hpp"
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/cc13_front6.hpp"
#include "bsp/cc13_big1_null_host.hpp"
#include "bsp/hud_root_rows.hpp"
#include "bsp/local_player_unit_lists.hpp"
#include "bsp/fe5_host_unreached.hpp"     // goal 3 chain 233: the warning show's Fe5Host
#include "bsp/world_map_bounds.hpp"
#include "bsp/ingame_gui_cc13.hpp"
#include "bsp/engine_seg11_35_66_cc13.hpp"
#include "bsp/frontend_seg12_17_cc13.hpp"    // 0068CC70 leaf screens 005178B0, 00565D40
#include "bsp/observed_unit_screens.hpp"     // screen 23h's 00517820 / 00517860
#include "bsp/frontend_seg15_22_cc13.hpp"    // 0052B030, 005D3540
#include "bsp/frontend_seg19_21_cc13.hpp"    // 005CB0A0
#include "bsp/seg25_cc13.hpp"                // 0060FD90, 00611750
#include "bsp/cc13_front4.hpp"               // 006529E0
#include "bsp/cc13_front5.hpp"               // 00652A30, 00652A20, 00652A50, 006529C0, 006529B0
#include "bsp/view_seg14_cc13.hpp"           // 0051E950
#include "bsp/cc13_sweepA3.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/native_input_action_configuration.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/unit_rudder.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/segs_13_15_22_44_cc13.hpp"
#include "bsp/seg27_cc13.hpp"                // 006735D0
#include "bsp/frontend_screen_animation.hpp"  // 00683610, 005FFCB0's +9Ch scroller
#include "bsp/cc13_sweepB2.hpp"              // 005AAEF0
#include "bsp/cg_rebuild_b_cc13.hpp"         // 005BA7B0
#include "bsp/cg_rebuild_c_cc13.hpp"         // 006821C0
#include "bsp/pool_pool17_cc13.hpp"          // 004BA0D0, 006821C0's +30h slots
#include "bsp/pool_pool22_cc13.hpp"          // 00525A10, 00525A90's +C0h selection
#include "bsp/voice_slot_start.hpp"          // 00A7B0A0 for 00525A90's +E8h
#include "bsp/gameplay_effect_sound.hpp"     // 005B9AF0, 005BA7B0's sample append
#include "bsp/sound_sample_runtime.hpp"      // [00F8BBE8] for 005BA7B0
#include "bsp/singleton_lifetime.hpp"        // 005B9D40's 00BF6989 on the 33h samples
#include "bsp/cc13_sweepB6.hpp"              // 005FA0F0
#include "bsp/game_native_string_process.hpp"  // the leaf registers' string temporaries
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/cc13_left61.hpp"               // 0093CB10, 00538E30's scene member
#include "bsp/frontend_managers.hpp"
#include "bsp/simulation_gate.hpp"

#include "bsp/hud_screens.hpp"
#include "bsp/in_mission_interface_runtime.hpp"
#include "bsp/ingame_interface.hpp"
#include "bsp/plane_squadron_host.hpp"
#include "bsp/ship_captain_camera_draws_cc13.hpp"
#include "bsp/unit_kind_query.hpp"
// Goal 4 chain 249: the in-mission interface's calls into the live sound runtime.
#include "bsp/game_sound_runtime.hpp"
#include "bsp/sound_configuration.hpp"
#include "bsp/sound_streaming_cc13.hpp"
#include "bsp/sound_system_owner.hpp"
#include "bsp/sound_manager_levels.hpp"
#include "bsp/sound_class_ownership.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_pooled_string_substring.hpp"  // 00426060, the counters tick's copies
#include "bsp/native_session_message35.hpp"        // HUD message 23h (0052B620 / 0052AF50)
#include "bsp/engine_seg43_45_cc13.hpp"            // 00772330
#include "bsp/cc13_sweepB4.hpp"                    // 00529D70 / 0052A100 / 0052A470
#include "bsp/fmod_configuration_library.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <deque>
#include <functional>
#include <intrin.h>
#include <map>
#include <memory>
#include <new>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::cc13_fe1 {
// bsp.py show 005BCA70 --asm: the complete body is RET 4. Original ECX is
// [HUD manager+58h]; its sole stacked argument is the current text header.
// The new fastcall padding formal leaves that pointer on the stack, preserving
// RET4 without reading either borrowed object or inventing a display update.
__declspec(naked) void __fastcall hud_panel_begin_005bca70(
    void*, void*, const void*) noexcept {
    __asm { ret 4 }
}
} // namespace bsp::cc13_fe1

namespace bsp::game {
namespace {

// Only +ACh is read by 005B5D40. This is a view of screen 33h's retained
// GUI_narrative widget, not a second widget or a fabricated numeric handle.
// 005BB130 binds that field to MissionInfoTitle_Group on GUI_narrative.
struct NarrativeHideView {
    std::array<std::byte, 0xAC> unused;
    GuiLayoutWidget* text_group;
};
static_assert(offsetof(NarrativeHideView, text_group) == 0xAC);
static_assert(sizeof(GuiLayoutWidget*) == sizeof(std::uint32_t));

struct NarrativeQueryView {
    std::array<std::byte, 0xB0> unused;
    GuiLayoutWidget* text;
};
static_assert(offsetof(NarrativeQueryView, text) == 0xB0);

class NarrativeTextColourBinding final : public bsp::frontend_seg20_cc13::NarrativeTextQueryHost {
public:
    void widget_vslot54(void* widget, std::uint8_t (&out)[16]) override {
        // The same live RGBA that GameFrontendHost's widget setters/rendering
        // use. 005B5D20 drops this copy and never mutates that state.
        const auto& colour = static_cast<GuiLayoutWidget*>(widget)->color;
        static_assert(sizeof colour == sizeof out);
        std::memcpy(out, colour, sizeof out);
    }
};

class NarrativeVisibilityBinding final : public bsp::cc13_fe1::WidgetHost {
public:
    explicit NarrativeVisibilityBinding(GameFrontendHost& frontend) : frontend_(frontend) {}
    void set_shown(std::uint32_t widget, bool shown) override {
        frontend_.set_widget_visible(*reinterpret_cast<GuiLayoutWidget*>(widget), shown);
    }
    void set_enabled(std::uint32_t, bool) override { unsupported(); }
    std::int32_t type(std::uint32_t) override { unsupported(); }
    void set_scale(std::uint32_t, float) override { unsupported(); }
    void set_frame_highlight(std::uint32_t, bool, std::int32_t, float) override { unsupported(); }
    void set_toggle_highlight(std::uint32_t, bool) override { unsupported(); }
private:
    [[noreturn]] static void unsupported() {
        throw std::logic_error("Narrative hide uses only widget visibility");
    }
    GameFrontendHost& frontend_;
};

// 0068CC70 allocates 174h bytes for manager+40h / registry slot 44h.
// Keep actual bool objects at the native +4h/+5h offsets, so the screen-set
// recompute and pump use the very base 0068BF60 constructs, not a flag copy.
// The tail also owns chain 413's +1Ch/+CCh/+E4h interface/observer fields.
struct HudRootConstructionStorage {
    std::uint32_t vtable{};
    FrontEndScreen flags{};
    std::array<std::uint8_t, 0x174 - 6> tail{};
    std::uint8_t* data() { return reinterpret_cast<std::uint8_t*>(this); }
};
static_assert(offsetof(HudRootConstructionStorage, flags) == 4);
static_assert(offsetof(FrontEndScreen, wanted) == 0);
static_assert(offsetof(FrontEndScreen, active) == 1);
static_assert(offsetof(HudRootConstructionStorage, tail) == 6);
static_assert(sizeof(HudRootConstructionStorage) == 0x174);

class HudRootConstructionBinding final : public bsp::cc13_fe5::Fe5HostUnreached {
public:
    explicit HudRootConstructionBinding(HudRootConstructionStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(bsp::cc13_fe5::Raw screen) override {
        if (screen.p != screen_.data())
            throw std::logic_error("0068BF60 base belongs to a different HUD root");
        bsp::construct_front_end_screen_004f7180(screen_.flags);
        // bsp.py show 004f7180: the typed base represents just the two flags;
        // its vtable store is retained here before 0068BF60 replaces it.
        screen.at<std::uint32_t>(0) = 0x00CEAE54u;
    }
private:
    HudRootConstructionStorage& screen_;
};

// 0068CC70's manager+D4h / slot 5Ah allocation is 34h bytes. As with the
// root screen, the registry borrows the actual base constructed by the leaf;
// there is no second set of wanted/active flags. The page and Message_Text
// pointers at +1Ch/+20h borrow this menu owner's installed GUI tree.
struct HudSceneInitStorage {
    std::uint32_t vtable{};
    FrontEndScreen flags{};
    std::array<std::uint8_t, 0x34 - 6> tail{};
    std::uint8_t* data() { return reinterpret_cast<std::uint8_t*>(this); }
};
static_assert(offsetof(HudSceneInitStorage, flags) == 4);
static_assert(offsetof(HudSceneInitStorage, tail) == 6);
static_assert(sizeof(HudSceneInitStorage) == 0x34);
constexpr int kSceneInitRegistrySlot = 0x5A;

class HudSceneInitBinding final : public bsp::ingame_gui_cc13::HudSceneInitHost {
public:
    HudSceneInitBinding(HudSceneInitStorage& screen, GameMenuHost& menu)
        : screen_(screen), menu_(menu) {}

    void front_end_screen_construct_004f7180(bsp::ingame_gui_cc13::Raw screen) override {
        require_screen(screen);
        bsp::construct_front_end_screen_004f7180(screen_.flags);
        screen.at<std::uint32_t>(0) = 0x00CEAE54u;
    }
    void front_end_screen_register_004f71d0(bsp::ingame_gui_cc13::Raw screen) override {
        require_screen(screen);
        const HudScreenLayout* layout = hud_screen_layout_for_slot(kSceneInitRegistrySlot);
        if (layout == nullptr || !menu_.register_in_game_screen(kSceneInitRegistrySlot,
                "HUD5A", layout->register_virtual, layout->layout_virtual, &screen_.flags))
            throw std::runtime_error("00636F30 could not register scene-init screen 5Ah");
    }
    void* gui_load_page_00aa5840(std::string_view name) override {
        const std::string page_name(name);
        if (!menu_.attach_in_game_page(kSceneInitRegistrySlot, page_name))
            throw std::runtime_error("00636F30 could not load " + page_name);
        GuiLayoutPage* page = menu_.in_game_page(kSceneInitRegistrySlot, page_name);
        if (page == nullptr || !page->root)
            throw std::runtime_error("00636F30 scene-init page has no root");
        return page->root.get();
    }
    void* gui_find_child_00aa7e00(void* parent, std::string_view name) override {
        auto* child = bsp::find_descendant_by_name(*static_cast<GuiLayoutWidget*>(parent), name);
        if (child == nullptr)
            throw std::runtime_error("00636F30 scene-init page lacks " + std::string(name));
        return child;
    }
    void widget_set_visible_34(void* widget, bool visible) override {
        menu_.frontend().set_widget_visible(*static_cast<GuiLayoutWidget*>(widget), visible);
    }
    void widget_get_colour_54(void* widget, float rgba[4]) override {
        const auto& colour = static_cast<GuiLayoutWidget*>(widget)->color;
        std::memcpy(rgba, colour, sizeof(float) * 4);
    }
private:
    void require_screen(bsp::ingame_gui_cc13::Raw screen) const {
        if (screen.p != reinterpret_cast<const std::uint8_t*>(&screen_))
            throw std::logic_error("scene-init base belongs to a different screen");
    }
    HudSceneInitStorage& screen_;
    GameMenuHost& menu_;
};

// Goal 4 chain 249: [00F8BBD8] for the update's 00A7B710 and 004CD0F0's class pauses
// (GameHudHost::bind_sound_runtime), and the pool their temporary names use. Goal 5
// chain 635: also [00F8BBE8], the sample cache screen 33h's constructor fills from.
GameSoundRuntime* g_hud_sound_runtime = nullptr;
NativeStringStorage* g_hud_sound_strings = nullptr;

// 0068CC70's operator new(size) for a HUD screen whose leaf constructor is
// reconstructed and needs only the 004F7180 base (plus, for 00606470 and
// 00608E60, its own 00BF681B blocks and 0064A4F0). The block is this owner's
// for the HUD's lifetime, like the root and scene-init screens above. As for
// those two, the front-end base's two flag bytes at +4h/+5h are an actual
// FrontEndScreen, so the registry borrows the very base the leaf constructed
// (register_in_game_screen's constructed_flags) instead of a second copy.
// The block is zeroed; the native allocation is not, and nothing reads a
// byte the leaf did not store before the screen's own register runs.
class HudLeafScreenStorage {
public:
    explicit HudLeafScreenStorage(std::size_t size_bytes)
        : bytes_(std::make_unique<std::uint8_t[]>(size_bytes)), size_(size_bytes) {
        if (size_bytes < 6) throw std::logic_error("0068CC70 leaf screen smaller than its base");
        flags_ = ::new (static_cast<void*>(bytes_.get() + 4)) FrontEndScreen{};
    }
    std::uint8_t* data() const noexcept { return bytes_.get(); }
    FrontEndScreen& flags() const noexcept { return *flags_; }
    bool contains(const void* p, std::size_t bytes) const noexcept {
        const auto* b = static_cast<const std::uint8_t*>(p);
        return b >= data() && bytes <= size_ && b <= data() + (size_ - bytes);
    }
    // 00BF681B blocks the leaf constructor stores into its own fields (the
    // two 14h flash records of 00606470); the native destructor frees them.
    void* allocate_owned(std::size_t bytes) {
        owned_.push_back(std::make_unique<std::uint8_t[]>(bytes));
        return owned_.back().get();
    }
    // 00BF6989 on a block allocate_owned handed out (the counters screen's
    // pointer arrays grow through 00529700); _free(null) does nothing.
    void release_owned(void* block) {
        if (block == nullptr) return;
        for (auto it = owned_.begin(); it != owned_.end(); ++it) {
            if (it->get() == block) {
                owned_.erase(it);
                return;
            }
        }
        throw std::logic_error("00BF6989 frees a block this HUD screen does not own");
    }

private:
    std::unique_ptr<std::uint8_t[]> bytes_;
    std::size_t size_;
    FrontEndScreen* flags_{nullptr};
    std::vector<std::unique_ptr<std::uint8_t[]>> owned_;
};

// 004F7180 on a leaf's own block (bsp.py show 004f7180): the base vtable
// 00CEAE54, then both flag bytes cleared. The leaf replaces the vtable.
void construct_leaf_screen_base(HudLeafScreenStorage& screen, const void* p) {
    if (p != screen.data())
        throw std::logic_error("004F7180 base belongs to a different HUD screen");
    bsp::construct_front_end_screen_004f7180(screen.flags());
    std::memcpy(screen.data(), &bsp::kFrontEndScreenBaseVtable, sizeof(std::uint32_t));
}

// One adapter per narrow host the leaf constructors take; each has the single
// 004F7180 service.
class HudLeafGuiBaseBinding final : public bsp::ingame_gui_cc13::HudScreenBaseHost {
public:
    explicit HudLeafGuiBaseBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(bsp::ingame_gui_cc13::Raw screen) override {
        construct_leaf_screen_base(screen_, screen.p);
    }
private:
    HudLeafScreenStorage& screen_;
};
class HudLeafSeg14BaseBinding final : public bsp::cc13_seg14::FrontEndScreenBaseHooks {
public:
    explicit HudLeafSeg14BaseBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(void* screen) override {
        construct_leaf_screen_base(screen_, screen);
    }
private:
    HudLeafScreenStorage& screen_;
};
template <class Host>
class HudLeafVoidBaseBinding final : public Host {
public:
    explicit HudLeafVoidBaseBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void frontend_construct_004f7180(void* screen) override {
        construct_leaf_screen_base(screen_, screen);
    }
private:
    HudLeafScreenStorage& screen_;
};
// The Fe5Host leaves: 004F7180, 00608E60's 0064A4F0 on its +16Ch member and
// 00606470's two operator new(14h). Every other Fe5Host service stays the
// base's loud failure.
class HudLeafFe5Binding final : public bsp::cc13_fe5::Fe5HostUnreached {
public:
    explicit HudLeafFe5Binding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(bsp::cc13_fe5::Raw screen) override {
        construct_leaf_screen_base(screen_, screen.p);
    }
    void widget_quad_fade_construct_0064a4f0(bsp::cc13_fe5::Raw fade) override {
        if (!screen_.contains(fade.p, 0x24))
            throw std::logic_error("0064A4F0 fade is not a member of this HUD screen");
        bsp::ingame_gui_cc13::hud_widget_quad_fade_construct_0064a4f0(fade);
    }
    void* operator_new(std::size_t bytes) override { return screen_.allocate_owned(bytes); }
private:
    HudLeafScreenStorage& screen_;
};

// The STL head-node allocators (_Buyheadnode) the leaf constructors call for
// their embedded containers. Each row was read from its listing (bsp.py show
// <routine> --asm): operator new(size) 00BF681B, then a list head stores its
// own address at +0h/+4h, or a tree head clears +0h/+4h/+8h and stores 1 at
// isnil-1 (colour) and 0 at isnil. The constructor then links a tree head to
// itself and sets isnil, as the native _Tree constructor does.
struct HudHeadNodeShape {
    std::uint32_t routine;
    std::uint8_t size;
    std::uint8_t isnil;  // 0: a list head
};
constexpr HudHeadNodeShape kHudHeadNodeShapes[] = {
    {0x0063B110u, 0x14, 0x11}, {0x006593A0u, 0x14, 0}, {0x00659B90u, 0x24, 0x21},
    {0x00659B40u, 0x18, 0x15}, {0x00659480u, 0x0C, 0}, {0x00659550u, 0x0C, 0},
    {0x006595C0u, 0x10, 0}, {0x00659350u, 0x14, 0}, {0x00659630u, 0x0C, 0},
    {0x006596A0u, 0x1C, 0}, {0x0059FD60u, 0x0C, 0}, {0x004E79F0u, 0x14, 0},
    {0x006596C0u, 0x0C, 0}, {0x004870F0u, 0x0C, 0}, {0x004C1630u, 0x0C, 0},
    {0x00659760u, 0x1C, 0}, {0x00659EB0u, 0x18, 0x15}, {0x00659F40u, 0x20, 0x1D},
    {0x005828E0u, 0x18, 0x15}, {0x00659F90u, 0x18, 0x15}, {0x005BDD40u, 0x18, 0x15},
    {0x005359A0u, 0x18, 0x15}, {0x00535720u, 0x0C, 0}, {0x005356C0u, 0x0C, 0},
    {0x0053DA70u, 0x3C, 0}, {0x005CC2E0u, 0x18, 0x15}, {0x005CC330u, 0x24, 0x21},
    {0x00613AE0u, 0x1C, 0x19}, {0x00613B30u, 0x18, 0x15}, {0x00613860u, 0x0C, 0},
    {0x005FD1B0u, 0x1C, 0x19}, {0x00582280u, 0x0C, 0}, {0x004C3020u, 0x10, 0},
    {0x005A0220u, 0x14, 0x11}, {0x005A0270u, 0x18, 0x15},
    // 005B7550: PUSH 0Ch, CALL 00BF681B, then [EAX] = EAX and [EAX+4] = EAX.
    {0x005B7550u, 0x0C, 0},
};
void* buy_hud_head_node(HudLeafScreenStorage& screen, std::uint32_t routine) {
    for (const HudHeadNodeShape& shape : kHudHeadNodeShapes) {
        if (shape.routine != routine) continue;
        auto* const node = static_cast<std::uint8_t*>(screen.allocate_owned(shape.size));
        if (shape.isnil == 0) {
            std::memcpy(node, &node, sizeof(node));
            std::memcpy(node + 4, &node, sizeof(node));
        } else {
            std::memset(node, 0, 0x0C);
            node[shape.isnil - 1] = 1;
            node[shape.isnil] = 0;
        }
        return node;
    }
    throw std::logic_error("HUD leaf screen head-node routine not read from its listing");
}

// 00640C80's tree head is bought by its caller here (the reconstruction takes
// it as a parameter; natively 00640C80 calls 0063B110 itself at 00640D0D).
void construct_hud_markers_leaf(HudLeafScreenStorage& screen) {
    HudLeafGuiBaseBinding b(screen);
    bsp::ingame_gui_cc13::hud_markers_construct_00640c80({screen.data()}, b,
        buy_hud_head_node(screen, 0x0063B110u));
}

// 00538E30, 00540BA0 (with 005401F0) and 005CE610.
class HudLeafSeg15Binding final : public bsp::frontend_seg15_22_cc13::Seg15LeafConstructHost {
public:
    explicit HudLeafSeg15Binding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(bsp::ingame_gui_cc13::Raw screen) override {
        construct_leaf_screen_base(screen_, screen.p);
    }
    void* operator_new_00bf681b(std::size_t bytes) override { return screen_.allocate_owned(bytes); }
    void simple_3d_scene_construct_0093cb10(bsp::ingame_gui_cc13::Raw scene) override {
        if (!screen_.contains(scene.p, 0x28))
            throw std::logic_error("0093CB10 scene is not a member of this HUD screen");
        bsp::cc13_left61::simple_3d_scene_construct_0093cb10({scene.p});
    }
    void* tree_buy_head_node_005359a0() override { return buy_hud_head_node(screen_, 0x005359A0u); }
    void* list_buy_head_node(std::uint32_t routine) override { return buy_hud_head_node(screen_, routine); }
    void* list_buy_head_node_0053da70() override { return buy_hud_head_node(screen_, 0x0053DA70u); }
    void* tree_buy_head_node(std::uint32_t routine) override { return buy_hud_head_node(screen_, routine); }
private:
    HudLeafScreenStorage& screen_;
};
// 005C0810.
class HudLeafMinimapBinding final : public bsp::cc13_fe1::MinimapConstructHost {
public:
    explicit HudLeafMinimapBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void frontend_construct_004f7180(void* screen) override { construct_leaf_screen_base(screen_, screen); }
    void* tree_buy_head_node_005bdd40(void* tree) override {
        if (!screen_.contains(tree, 0x0C))
            throw std::logic_error("005BDD40 tree is not a member of this HUD screen");
        return buy_hud_head_node(screen_, 0x005BDD40u);
    }
private:
    HudLeafScreenStorage& screen_;
};
// 0061A480.
class HudLeafPowerupBinding final : public bsp::seg25_cc13::PowerupManagerConstructHost {
public:
    explicit HudLeafPowerupBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void frontend_construct_004f7180(void* screen) override { construct_leaf_screen_base(screen_, screen); }
    void* container_buy_head_node(std::uint32_t function, void* container) override {
        if (!screen_.contains(container, 0x0C))
            throw std::logic_error("0061A480 container is not a member of this HUD screen");
        return buy_hud_head_node(screen_, function);
    }
private:
    HudLeafScreenStorage& screen_;
};
// 006735D0.
class HudLeafSupportBinding final : public bsp::seg27_cc13::SupportScreenConstructHost {
public:
    explicit HudLeafSupportBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void front_end_screen_construct_004f7180(bsp::ingame_gui_cc13::Raw screen) override {
        construct_leaf_screen_base(screen_, screen.p);
    }
    void* container_buy_head_node(std::uint32_t routine, void* container) override {
        if (!screen_.contains(container, 0x0C))
            throw std::logic_error("006735D0 container is not a member of this HUD screen");
        return buy_hud_head_node(screen_, routine);
    }
private:
    HudLeafScreenStorage& screen_;
};

// 00683610 writes the scroller in place at the screen's own offset, so the
// typed scroller must have the native 5Ch layout (bsp.py show 00683610 --asm).
static_assert(sizeof(bsp::FrontEndScreenScroller) == 0x5C);
static_assert(offsetof(bsp::FrontEndScreenScroller, scroll_range) == 0x0C);
static_assert(offsetof(bsp::FrontEndScreenScroller, auto_scroll) == 0x20);
static_assert(offsetof(bsp::FrontEndScreenScroller, thumb_widget) == 0x28);
static_assert(offsetof(bsp::FrontEndScreenScroller, thumb_min_height) == 0x40);
static_assert(offsetof(bsp::FrontEndScreenScroller, dragging) == 0x48);
static_assert(offsetof(bsp::FrontEndScreenScroller, drag_anchor_cursor_x) == 0x4C);
static_assert(offsetof(bsp::FrontEndScreenScroller, drag_anchor_thumb_offset) == 0x54);
static_assert(offsetof(bsp::FrontEndScreenScroller, auto_scroll_overridden) == 0x58);
static_assert(offsetof(bsp::FrontEndScreenScroller, tint_widgets) == 0x5B);

// 005FFCB0.
class HudLeafPauseBinding final : public bsp::cc13_fe1::PauseConstructHost {
public:
    explicit HudLeafPauseBinding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void frontend_construct_004f7180(void* screen) override { construct_leaf_screen_base(screen_, screen); }
    void scroller_construct_00683610(void* scroller) override {
        if (!screen_.contains(scroller, sizeof(bsp::FrontEndScreenScroller)))
            throw std::logic_error("00683610 scroller is not a member of this HUD screen");
        // The object begins its life here, at +9Ch of the zeroed block. 00683610
        // leaves +4Ch/+50h as operator new returned them; here they stay zero.
        auto* const typed = ::new (scroller) bsp::FrontEndScreenScroller;
        bsp::construct_front_end_screen_scroller_00683610(*typed);
    }
    void* list_allocate_sentinel_00582280(void* list) override { return head(list, 0x00582280u); }
    void* tree_buy_head_node_005fd1b0(void* tree) override { return head(tree, 0x005FD1B0u); }
    void* string_list_allocate_sentinel_004c3020(void* list) override { return head(list, 0x004C3020u); }
private:
    void* head(void* container, std::uint32_t routine) {
        if (!screen_.contains(container, 0x0C))
            throw std::logic_error("005FFCB0 container is not a member of this HUD screen");
        return buy_hud_head_node(screen_, routine);
    }
    HudLeafScreenStorage& screen_;
};

// 005AAEF0.
class HudLeafSweepB2Binding final : public bsp::InGameInterfaceScreenConstructHost {
public:
    explicit HudLeafSweepB2Binding(HudLeafScreenStorage& screen) : screen_(screen) {}
    void frontend_screen_construct_004f7180(void* screen) override { construct_leaf_screen_base(screen_, screen); }
    void simple3d_scene_construct_0093cb10(void* scene) override {
        if (!screen_.contains(scene, 0x28))
            throw std::logic_error("0093CB10 scene is not a member of this HUD screen");
        bsp::cc13_left61::simple_3d_scene_construct_0093cb10({static_cast<std::uint8_t*>(scene)});
    }
    void* stl_buy_head_node_005a0220(void* c) override { return head(c, 0x005A0220u); }
    void* stl_buy_head_node_005356c0(void* c) override { return head(c, 0x005356C0u); }
    void* stl_buy_head_node_005359a0(void* c) override { return head(c, 0x005359A0u); }
    void* stl_buy_head_node_005a0270(void* c) override { return head(c, 0x005A0270u); }
private:
    void* head(void* container, std::uint32_t routine) {
        if (!screen_.contains(container, 0x0C))
            throw std::logic_error("005AAEF0 container is not a member of this HUD screen");
        return buy_hud_head_node(screen_, routine);
    }
    HudLeafScreenStorage& screen_;
};

// 005BA7B0, screen 33h (narrative, manager +A4h, E8h bytes; goal 5 chain 635).
// Its calls (bsp.py show 005ba7b0 --asm, src/cg_rebuild_b_cc13.cpp): the 004F7180
// base; the EH constructor iterator over the one 18h voice slot at +8h (ctor
// 005B7FA0, dtor 005B7FC0); 005B7550 on the list at +64h; then for each of the five
// warning radio banks a pooled NativeString temporary (0041DD40 resize, 00BF7680
// copy, 00419CC0/00BD1510 release), 00A83FD0 on [00F8BBE8] and 005B9AF0 onto the
// retained samples at +A0h, dropping the cache's temporary reference
// (InterlockedDecrement [00CE2220], vtable +0 at zero). The cache, the samples'
// final release and the pool are this process's sound runtime (g_hud_sound_runtime,
// the same [00F8BBE8] the message manager's 00704590 banks use) and the HUD's string
// pool. The constructor runs no other host service.
//
// Lifetime: the five references stay in +A0h for the HUD's life, as on the native
// screen. release_hud_narrative_leaf_samples drops them when the HUD dies, as the
// destructor 005B9D40 does (see there).
class HudLeafNarrativeBinding final : public bsp::cg_rebuild_b_cc13::CgRebuildBHost {
public:
    using NativeStringImage = bsp::cg_rebuild_b_cc13::NativeStringImage;
    using CheckedIterImage = bsp::cg_rebuild_b_cc13::CheckedIterImage;

    HudLeafNarrativeBinding(HudLeafScreenStorage& screen, GameSoundRuntime& sound,
        NativeStringStorage& strings)
        : screen_(screen), sound_(sound), strings_(strings) {}

    void* global_ptr(std::uint32_t address) override {
        // 005BA899: MOV ECX,[00F8BBE8], the current sample cache 00A83FD0 scans.
        if (address == 0x00F8BBE8u) return &sound_.samples().current_sample_cache_00f8bbe8();
        unused("an image global other than [00F8BBE8]");
    }
    void front_end_screen_construct_004f7180(void* screen) override {
        construct_leaf_screen_base(screen_, screen);
    }
    void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size,
        std::uint32_t count, std::uint32_t ctor, std::uint32_t dtor) override {
        // 005BA7E3..005BA7F5: PUSH 005B7FC0, PUSH 005B7FA0, PUSH 1, PUSH 18h, +8h.
        if (ctor != 0x005B7FA0u || dtor != 0x005B7FC0u || size != 0x18 || count != 1
            || !screen_.contains(first, size * count))
            throw std::logic_error("005BA7B0 iterated an unexpected voice slot array");
        // 005B7FA0 is noexcept, so the iterator's unwind (005B7FC0) never runs.
        for (std::uint32_t i = 0; i != count; ++i)
            bsp::cc13_fe1::record14_clear_005b7fa0(static_cast<std::uint8_t*>(first) + i * size);
    }
    void* list_buy_head_node_005b7550(void* list) override {
        if (!screen_.contains(list, 0x0C))
            throw std::logic_error("005B7550 list is not a member of this HUD screen");
        return buy_hud_head_node(screen_, 0x005B7550u);
    }
    void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
        std::int32_t preserve) override {
        bsp::resize_native_string_header_0041dd40(string, strings_, length, preserve != 0);
    }
    void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) override {
        std::memcpy(destination, source, count);
    }
    void* sized_pool_get_00419cc0() override { return &strings_; }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
        std::int32_t flag) override {
        // The inlined NativeString release: length + 1 bytes back with flag 1.
        if (pool != &strings_ || flag != 1)
            throw std::logic_error("005BA7B0 returned a block to an unexpected pool");
        strings_.release(static_cast<char*>(data), bytes);
    }
    void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out,
        const NativeStringImage* name) override {
        SoundSampleRuntime& samples = sound_.samples();
        SoundAuxiliaryTreeOwner& current = samples.current_sample_cache_00f8bbe8();
        if (cache != &current)
            throw std::logic_error("00A83FD0 called on a cache other than [00F8BBE8]");
        // NativeStringImage is the native {length, data} header NativeString owns.
        static_assert(sizeof(NativeString) == sizeof(NativeStringImage));
        return bsp::acquire_sound_sample_00a83fd0(current, *out,
            *reinterpret_cast<const NativeString*>(name), samples.sample_cache_context());
    }
    void effect_sound_samples_append_005b9af0(void* samples, void** slot) override {
        if (!screen_.contains(samples, 0x0C))
            throw std::logic_error("005B9AF0 samples are not a member of this HUD screen");
        bsp::append_effect_sound_sample_005b9af0(samples, slot, sound_.samples());
    }
    std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) override {
        return _InterlockedDecrement(reinterpret_cast<volatile long*>(value));
    }
    void refcounted_vtable0_release(void* object) override {
        // D5B074's vtable +0 (00A82E80): the sample's final release.
        sound_.samples().zero_references_slot_00(object);
    }

    // Nothing else is reached from 005BA7B0.
    void global_store_u32(std::uint32_t, std::uint32_t) override { unused("global_store_u32"); }
    const char* image_cstring_table_entry(std::uint32_t) override { unused("an image string table"); }
    void free_00bf65ac(void*) override { unused("00BF65AC"); }
    void free_00bf6989(void*) override { unused("00BF6989"); }
    void* operator_new_00bf681b(std::uint32_t) override { unused("00BF681B"); }
    void invalid_parameter_00bf6713() override { unused("00BF6713"); }
    void eh_vector_destructor_iterator_00bf7c6e(void*, std::uint32_t, std::uint32_t,
        std::uint32_t) override { unused("00BF7C6E"); }
    std::int32_t interlocked_increment_00ce221c(std::int32_t*) override { unused("[00CE221C]"); }
    void leave_critical_section_00ce2210(void*) override { unused("[00CE2210]"); }
    void delete_critical_section_00ce2214(void*) override { unused("[00CE2214]"); }
    void stl_tree_erase_range(std::uint32_t, void*, CheckedIterImage*, CheckedIterImage,
        CheckedIterImage) override { unused("an STL tree erase"); }
    void stl_tree_iterator_increment(std::uint32_t, CheckedIterImage*) override {
        unused("an STL tree iterator");
    }
    void object_vtable0_scalar_dtor(void*, std::uint32_t) override { unused("a scalar dtor"); }
    void front_end_screen_destruct_004f71a0(void*) override { unused("004F71A0"); }
    void* gui_manager_get_or_create_004c12b0() override { unused("004C12B0"); }
    void gui_manager_remove_and_destroy_page_00aa31f0(void*, void*) override { unused("00AA31F0"); }
    void node_list_clear_005234f0(void*) override { unused("005234F0"); }
    void callback_owner_destroy_00695870(void*) override { unused("00695870"); }
    void observer_endpoint_destruct_00695760(void*) override { unused("00695760"); }
    void container_init_00525a10(void*) override { unused("00525A10"); }
    std::uint32_t sound_manager_find_class_00a7acf0(void*, const NativeStringImage*) override {
        unused("00A7ACF0");
    }
    std::uint32_t sound_manager_find_type_00a7b0a0(void*, const NativeStringImage*) override {
        unused("00A7B0A0");
    }
    void owned_object_list_clear_005b7840(void*) override { unused("005B7840"); }
    void list_tidy_005b7570(void*) override { unused("005B7570"); }
    void list_clear_005b77f0(void*) override { unused("005B77F0"); }
    void voice_slot_stop_007026f0(void*) override { unused("007026F0"); }
    void effect_sound_samples_resize_005b8fa0(void*, std::uint32_t) override { unused("005B8FA0"); }
    void debug_text_list_destroy_rows_00543e00(void*) override { unused("00543E00"); }
    void member_teardown_00544a70(void*) override { unused("00544A70"); }
    void debug_text_list_destruct_00590250(void*) override { unused("00590250"); }
    void cheat_menu_destroy_members_0051f910(void*) override { unused("0051F910"); }
    void list_tidy_005822a0(void*) override { unused("005822A0"); }
    void text_listener_base_destroy_00a97140(void*) override { unused("00A97140"); }
    void main_menu_release_mission_preview_005830a0(void*) override { unused("005830A0"); }
    void native_string_range_destroy_00432050(void*, void*, void*, void*) override { unused("00432050"); }
    void native_string_vector_resize_00427110(void*, std::uint32_t) override { unused("00427110"); }
    void warning_manager_clear_waiting_queue_0096bf70(void*) override { unused("0096BF70"); }
    void member_teardown_0097a190(void*) override { unused("0097A190"); }

private:
    [[noreturn]] static void unused(const char* what) {
        throw std::logic_error(std::string("005BA7B0 does not reach ") + what);
    }
    HudLeafScreenStorage& screen_;
    GameSoundRuntime& sound_;
    NativeStringStorage& strings_;
};

// 0068CC70's leaf 33h needs the started sound runtime its samples come from; the
// image's cache exists from the sound startup on, before any mission is built.
void construct_hud_narrative_leaf(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    NativeStringStorage* const strings = g_hud_sound_strings;
    if (sound == nullptr || strings == nullptr || !sound->started())
        throw std::logic_error("005BA7B0 needs the started sound runtime that owns [00F8BBE8]");
    HudLeafNarrativeBinding b(screen, *sound, *strings);
    bsp::cg_rebuild_b_cc13::hud_narrative_screen_construct_005ba7b0(screen.data(), b);
}

// 005B9D40's sample release on the 33h block (005B9E02..005B9E19): 005B8FA0
// resizes the samples at +A0h to 0, dropping each reference (D5B074's vtable +0,
// 00A82E80, when one reaches zero), then 00BF6989 frees the data 0086E770's reserve
// allocated (singleton_lifetime_allocate, so singleton_lifetime_free here). The
// header is zeroed after, since the block outlives this call by a moment. The rest
// of 005B9D40 (005B7840, 007026F0, the three page removals, the NativeStrings, the
// +94h vector, the +54h/+64h lists, 005B7FC0, 004F71A0) is not run: the block's
// +20h/+24h/+28h page slots were never filled (005BB130 stays generic) and its
// lists and strings hold nothing the constructor did not leave empty.
// The sound runtime is still bound and started here: the menu host owning this HUD
// is deleted before the singleton drain retires the sample cache and the sound owner
// (game_hosts.cpp: application_shutdown's `delete menu_` at 4121 runs before
// destroy_singleton_lifetime_manager's drain at 4197, winmain_startup.cpp 231..233;
// on the exceptional path `delete menu_` at 2673 precedes the drain at 2691,
// bind_sound_runtime(nullptr) at 2744 and sound_.reset() at 2745).
void release_hud_narrative_leaf_samples(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    if (sound == nullptr || !sound->started())
        throw std::logic_error("005B9D40's sample release needs the sound runtime that owns [00F8BBE8]");
    std::uint8_t* const samples = screen.data() + 0xA0;
    bsp::resize_effect_sound_samples_005b8fa0(samples, 0, sound->samples());  // 005B9E11
    void* data = nullptr;
    std::memcpy(&data, samples, sizeof(data));
    bsp::singleton_lifetime_free(data);                                       // 005B9E19
    std::memset(samples, 0, 0x0C);
}

// Screen 50h (GUI_Warning, manager +C4h, 7Ch bytes, vtable 00CF7184): the calls
// 006821C0 makes (bsp.py show 006821c0 --asm; src/cg_rebuild_c_cc13.cpp). The
// 004F7180 base; the EH constructor iterator over the four sample slots at +30h
// (ctor 004BA0D0, dtor 004C3810) and the four tracked-sound slots at +40h (ctor
// 0054D440, dtor 00524180), both `MOV dword [ECX],0; RET` (bsp.py ghidra disasm);
// then, per name of the .data table 00E08CC4..00E08CD3, a pooled NativeString
// temporary (0041DD40, 00BF7680, 00419CC0/00BD1510), 00A83FD0 on [00F8BBE8] and
// the retain/release of the slot (InterlockedIncrement [00CE221C] / Decrement
// [00CE2220], vtable +0 at zero). The cache is the sound runtime's (the one screen
// 33h's 005BA7B0 fills from), the pool the HUD's string pool. Every other
// CgRebuildCHost service throws.
//
// 00E08CC4 is read only by 006821C0 (bsp.py ghidra xrefs 00e08cc4: three reads at
// 0068222F/0068224C/00682260, no writes), so its run-time words are the image's:
// 00CF714C, 00CF7120, 00CF70EC, 00CF714C (bsp.py ghidra bytes 00e08cc4 --length 16),
// whose texts are below (bsp.py ghidra bytes 00cf70ec --length 140). The three
// .fsb/.def pairs are in the installation (sound/environment).
constexpr const char kWarnSampleStall[] = "environment/environment_warning_stall.fsb";             // 00CF714C
constexpr const char kWarnSampleOxygen[] = "environment/environment_warning_oxigen.fsb";           // 00CF7120
constexpr const char kWarnSampleShallow[] = "environment/environment_warning_shallowwater.fsb";    // 00CF70EC

class HudLeafWarningBinding final : public bsp::cg_rebuild_c_cc13::CgRebuildCHost {
public:
    HudLeafWarningBinding(HudLeafScreenStorage& screen, GameSoundRuntime& sound,
        NativeStringStorage& strings)
        : screen_(screen), sound_(sound), strings_(strings) {
        name_table_[0] = dword_of(kWarnSampleStall);    // 00E08CC4
        name_table_[1] = dword_of(kWarnSampleOxygen);   // 00E08CC8
        name_table_[2] = dword_of(kWarnSampleShallow);  // 00E08CCC
        name_table_[3] = dword_of(kWarnSampleStall);    // 00E08CD0
        // 006821C0 loads ECX = [00F8BBE8], the current sample cache 00A83FD0 scans.
        cache_dword_ = dword_of(&sound_.samples().current_sample_cache_00f8bbe8());
    }

    std::uint32_t& global_dword(std::uint32_t address) override {
        if (address >= 0x00E08CC4u && address < 0x00E08CD4u && (address & 3u) == 0)
            return name_table_[(address - 0x00E08CC4u) / 4];
        if (address == 0x00F8BBE8u) return cache_dword_;
        unused("an image global other than 00E08CC4..00E08CD3 and [00F8BBE8]");
    }
    void front_end_screen_construct_004f7180(void* screen) override {
        construct_leaf_screen_base(screen_, screen);
    }
    void eh_vector_construct_00bf7cd1(void* first, std::uint32_t size, std::uint32_t count,
        std::uint32_t constructor, std::uint32_t destructor) override {
        // 006821F0: +30h, 004BA0D0/004C3810; 0068220A: +40h, 0054D440/00524180;
        // PUSH 4 (count), PUSH 4 (size) each. Both constructors are noexcept, so
        // the iterator's unwind never runs.
        const bool samples = constructor == 0x004BA0D0u && destructor == 0x004C3810u
            && first == screen_.data() + 0x30;
        const bool sounds = constructor == 0x0054D440u && destructor == 0x00524180u
            && first == screen_.data() + 0x40;
        if ((!samples && !sounds) || size != 4 || count != 4 || !screen_.contains(first, 0x10))
            throw std::logic_error("006821C0 iterated an unexpected slot array");
        for (std::uint32_t i = 0; i != count; ++i) {
            std::uint8_t* const slot = static_cast<std::uint8_t*>(first) + i * size;
            if (samples) bsp::pool_pool17_cc13::pointer_slot_construct_null_004ba0d0(slot);
            else bsp::cc13_fe5::pointer_slot_construct_null_0054d440({slot});
        }
    }
    void native_string_resize_0041dd40(void* header, std::uint32_t length,
        std::uint32_t preserve) override {
        bsp::resize_native_string_header_0041dd40(header, strings_, length, preserve != 0);
    }
    void memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) override {
        std::memcpy(dst, src, bytes);
    }
    void* sized_pool_00419cc0() override { return &strings_; }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
        std::uint32_t flag) override {
        // The inlined NativeString release: length + 1 bytes back with flag 1.
        if (pool != &strings_ || flag != 1)
            throw std::logic_error("006821C0 returned a block to an unexpected pool");
        strings_.release(static_cast<char*>(data), size);
    }
    void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out,
        const void* name) override {
        SoundSampleRuntime& samples = sound_.samples();
        SoundAuxiliaryTreeOwner& current = samples.current_sample_cache_00f8bbe8();
        if (cache != &current)
            throw std::logic_error("00A83FD0 called on a cache other than [00F8BBE8]");
        // The temporary is the native {length, data} header NativeString owns.
        return bsp::acquire_sound_sample_00a83fd0(current, *out,
            *static_cast<const NativeString*>(name), samples.sample_cache_context());
    }
    std::int32_t interlocked_increment_00ce221c(void* counter) override {
        return _InterlockedIncrement(static_cast<volatile long*>(counter));
    }
    std::int32_t interlocked_decrement_00ce2220(void* counter) override {
        return _InterlockedDecrement(static_cast<volatile long*>(counter));
    }
    void virtual_call_0(void* object, std::uint32_t slot_offset) override {
        // D5B074's vtable +0 (00A82E80): the sample's final release.
        if (slot_offset != 0) unused("a virtual other than a sample's +0");
        sound_.samples().zero_references_slot_00(object);
    }

    // Nothing else is reached from 006821C0.
    void eh_vector_destruct_00bf7c6e(void*, std::uint32_t, std::uint32_t, std::uint32_t) override {
        unused("00BF7C6E");
    }
    void eh_vector_copy_construct_00bf9a96(void*, const void*, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t) override { unused("00BF9A96"); }
    void free_00bf65ac(void*) override { unused("00BF65AC"); }
    void free_00bf6989(void*) override { unused("00BF6989"); }
    void virtual_call_1(void*, std::uint32_t, std::uint32_t) override { unused("a one-argument virtual"); }
    void sound_record_set_destruct_004c9870(void*) override { unused("004C9870"); }
    void scene_record_header_destruct_004c6520(void*) override { unused("004C6520"); }
    void pod_pointer_array_resize_00468050(void*, std::uint32_t) override { unused("00468050"); }
    void effect_handle_vector_resize_004cb160(void*, std::uint32_t) override { unused("004CB160"); }
    void native_string_vector_resize_00427110(void*, std::uint32_t) override { unused("00427110"); }
    void native_dword_vector_resize_0086aa10(void*, std::uint32_t) override { unused("0086AA10"); }
    void sensor_gui_range_list_resize_00807e60(void*, std::uint32_t) override { unused("00807E60"); }
    void raw_string_tree_erase_range_004d1a50(void*, void*, void*, void*, void*, void*) override {
        unused("004D1A50");
    }
    void xlive_system_shutdown_00a46ee0(void*) override { unused("00A46EE0"); }
    void stl_list_tidy_004c1f70(void*) override { unused("004C1F70"); }
    void xlive_session_terminate_xnet_00a4b020(void*) override { unused("00A4B020"); }
    void string_range_destroy_004ce520(void*, void*, void*, void*) override { unused("004CE520"); }
    void session_record_destroy_range_00571af0(void*, void*, void*, void*) override {
        unused("00571AF0");
    }
    void xlive_voice_destroy_00a493a0(void*) override { unused("00A493A0"); }
    void trivial_body_00a48f00(void*) override { unused("00A48F00"); }
    void xlive_host_qos_cancel_00a4b410(void*) override { unused("00A4B410"); }
    void xlive_matchmaking_reset_00a4b460(void*) override { unused("00A4B460"); }
    void stl_inst_00a45510(void*) override { unused("00A45510"); }
    void stl_list_clear_strings_00688780(void*) override { unused("00688780"); }
    void game_participant_pairs_destroy_storage_004c5860(void*) override { unused("004C5860"); }
    void xlive_stats_block_destruct_00994180(void*) override { unused("00994180"); }
    void front_end_screen_destruct_004f71a0(void*) override { unused("004F71A0"); }
    void* gui_manager_get_or_create_004c12b0() override { unused("004C12B0"); }
    void gui_manager_remove_and_destroy_page_00aa31f0(void*, std::uint32_t) override {
        unused("00AA31F0");
    }
    void observed_owner_base_destroy_0064a610(void*) override { unused("0064A610"); }
    void ref_counted_destroy_base_00bd30f0(void*) override { unused("00BD30F0"); }
    void callback_owner_destroy_00695870(void*) override { unused("00695870"); }
    void tick_subnode_base_cleanup_00875b30(void*) override { unused("00875B30"); }
    void bot_observer_holder_destruct_full_007b3f70(void*) override { unused("007B3F70"); }
    void ai_load_high_level_globals_00a335d0(void*) override { unused("00A335D0"); }

private:
    static std::uint32_t dword_of(const void* p) noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
    }
    [[noreturn]] static void unused(const char* what) {
        throw std::logic_error(std::string("006821C0 does not reach ") + what);
    }
    HudLeafScreenStorage& screen_;
    GameSoundRuntime& sound_;
    NativeStringStorage& strings_;
    std::uint32_t name_table_[4]{};
    std::uint32_t cache_dword_{0};
};

// 0068CC70's leaf 50h, like 33h, needs the started sound runtime whose sample
// cache [00F8BBE8] exists from the sound startup on, before any mission is built.
void construct_hud_warning_leaf(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    NativeStringStorage* const strings = g_hud_sound_strings;
    if (sound == nullptr || strings == nullptr || !sound->started())
        throw std::logic_error("006821C0 needs the started sound runtime that owns [00F8BBE8]");
    HudLeafWarningBinding b(screen, *sound, *strings);
    bsp::cg_rebuild_c_cc13::vt_cf7184_construct_006821c0(screen.data(), b);
}

// 00682030's slot release on the 50h block (00682079..006820DE, bsp.py show
// 00682030): for each i, a set +40h tracked sound would get vtable +8h(0) and be
// released, and the +30h sample is released (InterlockedDecrement [00CE2220] on
// +4h, vtable +0 at zero: 00A82E80, the sample's final release) and cleared. On
// this block +40h stays null: the warning show routines (warning_screen_show_alert,
// 00682800) run on a per-show WarningScreenImage, not on this block, so no tracked
// sound is ever stored here; a set slot throws rather than being dropped. The
// rest of 00682030 is not run: the page +70h was never filled (the 50h register
// 00682740 is not a kHudLeafScreenRegisters entry, so its page load stays the
// generic host path, hud_screens.cpp kPages50), so its 00AA31F0 removal on
// the native GUI manager has nothing to remove, and the two EH destructor
// iterators (00524180, 004C3810) do nothing on null slots. As for 33h, the sound
// runtime is still bound and started when the HUD dies (see
// release_hud_narrative_leaf_samples).
void release_hud_warning_leaf_samples(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    if (sound == nullptr || !sound->started())
        throw std::logic_error("00682030's sample release needs the sound runtime that owns [00F8BBE8]");
    for (std::uint32_t i = 0; i < 4; ++i) {
        void* late = nullptr;
        std::memcpy(&late, screen.data() + 0x40 + i * 4, sizeof(late));
        if (late != nullptr)
            throw std::logic_error("screen 50h holds a tracked sound this host never created");
        void* early = nullptr;
        std::memcpy(&early, screen.data() + 0x30 + i * 4, sizeof(early));
        if (early != nullptr) {                                                // 006820C2
            auto* const references = reinterpret_cast<volatile long*>(
                static_cast<std::uint8_t*>(early) + 4);
            if (_InterlockedDecrement(references) == 0)
                sound->samples().zero_references_slot_00(early);              // 006820D0
        }
        std::memset(screen.data() + 0x30 + i * 4, 0, sizeof(void*));           // 006820D8
    }
}

// Screen 29h (the unit pick, manager +CCh, F0h bytes, vtable 00CECCF8): the calls
// 00525A90 makes (bsp.py show 00525a90 --asm; src/cg_rebuild_b_cc13.cpp). The
// 004F7180 base; operator new(18h) for the +BCh block (00525B0A), owned by this
// leaf; 00525A10 on the +C0h selection (src/pool_pool22_cc13.cpp), whose 00524DC0
// erase runs on its just-zeroed vector, an empty range; the EH constructor iterator
// over the two +DCh sample slots (ctor 004BA0D0, dtor 004C3810); per name of the
// .data table 00E0865C..00E08663 the pooled NativeString temporary, 00A83FD0 on
// [00F8BBE8] and the slot retain/release; then 00A7ACF0("InGameGUI") into +E4h and
// 00A7B0A0("Normal") into +E8h on the sound manager [00F8BBD8]. The cache, the
// manager's level and configuration tables and the string pool are the sound
// runtime's and the HUD's, as for 33h and 50h. Every other service throws.
//
// 00E0865C has only the constructor's three accesses (bsp.py ghidra xrefs
// 00e0865c: 00525B56, 00525B60, 00525B66; no writes), so its words are the image's:
// 00CECC90 and 00CECC74 (bsp.py ghidra bytes 00e0865c --length 8; texts at
// 00cecc74). Both .fsb files are in the installation (sound/gui).
//
// The fields this constructor writes (+3Ch..+44h, +BCh..+E8h) are disjoint from
// the ones the host keeps in UnitPickScreenState (+4Ch, +50h, +54h, +B0h, +B4h,
// +B8h, +D8h, +ECh; hud_warning_screen.hpp), so the leaf block duplicates none
// of them.
constexpr const char kUnitPickSampleClick[] = "sound/gui/click_01.fsb";        // 00CECC90
constexpr const char kUnitPickSampleHover[] = "sound/gui/onmouse_01.fsb";      // 00CECC74

// 00525A10's two calls on the 29h block's +C0h selection.
class HudLeafUnitPickSelectionBinding final : public bsp::pool22_cc13::Pool22Host {
public:
    void invalid_parameter_noinfo_00bf6713() override {
        throw std::logic_error("00525A10's checked iterators found first > last");
    }
    void vector_erase_range_00524dc0(void* vector, void* ret, void* first_vector,
        void* first, void* last_vector, void* last) override {
        // 00525A37..00525A40 zeroed first/last/end before the call, so this is
        // erase(begin(), end()) on an empty vector: nothing moves or is destroyed,
        // and the result is the checked iterator {vector, first}.
        if (first_vector != vector || last_vector != vector || first != last || first != nullptr)
            throw std::logic_error("00524DC0 from 00525A10 on a non-empty selection vector");
        void* const result[2] = {vector, first};
        std::memcpy(ret, result, sizeof(result));
    }
};

// 005251C0's ECX is the very F0h screen 00525A90 constructed. Its +C4h
// vector is still the empty vector 00525A10 produced: the candidate collectors
// 00525F80/00526180 are not called by this host. Do not substitute a temporary
// screen or a separate selection vector. An unexpected populated vector needs
// its actual 18h observer-handle destructor before this adapter can erase it.
class HudLeafUnitPickResetBinding final : public bsp::cc13_sweepA::HudUnitPickResetHost {
public:
    HudLeafUnitPickResetBinding(HudLeafScreenStorage& screen, GameMenuHost& menu)
        : screen_(screen), menu_(menu) {}
    void invalid_parameter_noinfo_00bf6713() override {
        selection_.invalid_parameter_noinfo_00bf6713();
    }
    void vector_erase_range_00524dc0(bsp::cc13_sweepA::Raw vector, void* ret,
        void* first_vector, void* first, void* last_vector, void* last) override {
        if (vector.p != screen_.data() + 0xC4
            || first != vector.ptr(4) || last != vector.ptr(8))
            throw std::logic_error("005251C0 erased a range outside its retained selection");
        // 00524DC0: equal iterators skip both the 18h element move and the
        // virtual-destructor loop; return {first_vector, first}. This is the
        // same checked empty erase the selection constructor already uses.
        selection_.vector_erase_range_00524dc0(
            vector.p, ret, first_vector, first, last_vector, last);
    }
    void* input_manager_get_singleton_004bec00() override {
        return menu_.input_context_manager().current_owner();
    }
    void input_manager_set_context_level_00a933f0(void* manager, std::int32_t context,
        std::int32_t level) override {
        menu_.input_context_manager().set_level(manager,
            static_cast<std::uint32_t>(context), static_cast<std::uint32_t>(level));
    }
private:
    HudLeafScreenStorage& screen_;
    GameMenuHost& menu_;
    HudLeafUnitPickSelectionBinding selection_;
};

class HudLeafUnitPickBinding final : public bsp::cg_rebuild_b_cc13::CgRebuildBHost {
public:
    using NativeStringImage = bsp::cg_rebuild_b_cc13::NativeStringImage;
    using CheckedIterImage = bsp::cg_rebuild_b_cc13::CheckedIterImage;

    HudLeafUnitPickBinding(HudLeafScreenStorage& screen, GameSoundRuntime& sound,
        NativeStringStorage& strings)
        : screen_(screen), sound_(sound), strings_(strings) {}

    void* global_ptr(std::uint32_t address) override {
        // 00525BB1: ECX = [00F8BBE8], the sample cache; 00525CC2/00525D34: ECX =
        // [00F8BBD8], the sound manager.
        if (address == 0x00F8BBE8u) return &sound_.samples().current_sample_cache_00f8bbe8();
        if (address == 0x00F8BBD8u) return &sound_.owner();
        unused("an image global other than [00F8BBE8] and [00F8BBD8]");
    }
    const char* image_cstring_table_entry(std::uint32_t address) override {
        if (address == 0x00E0865Cu) return kUnitPickSampleClick;
        if (address == 0x00E08660u) return kUnitPickSampleHover;
        unused("an image string table entry outside 00E0865C..00E08663");
    }
    void front_end_screen_construct_004f7180(void* screen) override {
        construct_leaf_screen_base(screen_, screen);
    }
    void* operator_new_00bf681b(std::uint32_t bytes) override {
        // 00525B03: PUSH 18h. The block is the leaf's own until the HUD dies,
        // as 00524BA0 frees it with the screen.
        if (bytes != 0x18) unused("an operator new other than the 18h +BCh block");
        return screen_.allocate_owned(bytes);
    }
    void container_init_00525a10(void* container) override {
        if (container != screen_.data() + 0xC0 || !screen_.contains(container, 0x18))
            throw std::logic_error("00525A10 on a member other than this screen's +C0h");
        HudLeafUnitPickSelectionBinding selection;
        bsp::pool22_cc13::unit_pick_selection_construct_00525a10(container, selection);
    }
    void eh_vector_constructor_iterator_00bf7cd1(void* first, std::uint32_t size,
        std::uint32_t count, std::uint32_t ctor, std::uint32_t dtor) override {
        // 00525B37..00525B51: PUSH 004C3810, PUSH 004BA0D0, PUSH 2, PUSH 4, +DCh.
        if (ctor != 0x004BA0D0u || dtor != 0x004C3810u || size != 4 || count != 2
            || first != screen_.data() + 0xDC || !screen_.contains(first, size * count))
            throw std::logic_error("00525A90 iterated an unexpected slot array");
        // 004BA0D0 is noexcept, so the iterator's unwind (004C3810) never runs.
        for (std::uint32_t i = 0; i != count; ++i)
            bsp::pool_pool17_cc13::pointer_slot_construct_null_004ba0d0(
                static_cast<std::uint8_t*>(first) + i * size);
    }
    void native_string_resize_0041dd40(NativeStringImage* string, std::uint32_t length,
        std::int32_t preserve) override {
        bsp::resize_native_string_header_0041dd40(string, strings_, length, preserve != 0);
    }
    void crt_memcpy_00bf7680(void* destination, const void* source, std::uint32_t count) override {
        std::memcpy(destination, source, count);
    }
    void* sized_pool_get_00419cc0() override { return &strings_; }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes,
        std::int32_t flag) override {
        // The inlined NativeString release: length + 1 bytes back with flag 1.
        if (pool != &strings_ || flag != 1)
            throw std::logic_error("00525A90 returned a block to an unexpected pool");
        strings_.release(static_cast<char*>(data), bytes);
    }
    void** sound_sample_cache_acquire_00a83fd0(void* cache, void** out,
        const NativeStringImage* name) override {
        SoundSampleRuntime& samples = sound_.samples();
        SoundAuxiliaryTreeOwner& current = samples.current_sample_cache_00f8bbe8();
        if (cache != &current)
            throw std::logic_error("00A83FD0 called on a cache other than [00F8BBE8]");
        static_assert(sizeof(NativeString) == sizeof(NativeStringImage));
        return bsp::acquire_sound_sample_00a83fd0(current, *out,
            *reinterpret_cast<const NativeString*>(name), samples.sample_cache_context());
    }
    std::int32_t interlocked_increment_00ce221c(std::int32_t* value) override {
        return _InterlockedIncrement(reinterpret_cast<volatile long*>(value));
    }
    std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) override {
        return _InterlockedDecrement(reinterpret_cast<volatile long*>(value));
    }
    void refcounted_vtable0_release(void* object) override {
        // D5B074's vtable +0 (00A82E80): the sample's final release.
        sound_.samples().zero_references_slot_00(object);
    }
    std::uint32_t sound_manager_find_class_00a7acf0(void* manager,
        const NativeStringImage* name) override {
        SoundSystemOwner& owner = sound_.owner();
        if (manager != &owner) throw std::logic_error("00A7ACF0 on a manager other than [00F8BBD8]");
        // EAX, the signed class index (-1 on a miss), stored as is at +E4h.
        return static_cast<std::uint32_t>(bsp::find_sound_class_00a7acf0(owner.levels,
            *reinterpret_cast<const NativeString*>(name), sound_.fmod()));
    }
    std::uint32_t sound_manager_find_type_00a7b0a0(void* manager,
        const NativeStringImage* name) override {
        SoundSystemOwner& owner = sound_.owner();
        if (manager != &owner) throw std::logic_error("00A7B0A0 on a manager other than [00F8BBD8]");
        return static_cast<std::uint32_t>(bsp::find_sound_type_00a7b0a0(owner.configuration,
            *reinterpret_cast<const NativeString*>(name), sound_.fmod()));
    }

    // Nothing else is reached from 00525A90.
    void global_store_u32(std::uint32_t, std::uint32_t) override { unused("global_store_u32"); }
    void free_00bf65ac(void*) override { unused("00BF65AC"); }
    void free_00bf6989(void*) override { unused("00BF6989"); }
    void invalid_parameter_00bf6713() override { unused("00BF6713"); }
    void eh_vector_destructor_iterator_00bf7c6e(void*, std::uint32_t, std::uint32_t,
        std::uint32_t) override { unused("00BF7C6E"); }
    void leave_critical_section_00ce2210(void*) override { unused("[00CE2210]"); }
    void delete_critical_section_00ce2214(void*) override { unused("[00CE2214]"); }
    void stl_tree_erase_range(std::uint32_t, void*, CheckedIterImage*, CheckedIterImage,
        CheckedIterImage) override { unused("an STL tree erase"); }
    void stl_tree_iterator_increment(std::uint32_t, CheckedIterImage*) override {
        unused("an STL tree iterator");
    }
    void object_vtable0_scalar_dtor(void*, std::uint32_t) override { unused("a scalar dtor"); }
    void front_end_screen_destruct_004f71a0(void*) override { unused("004F71A0"); }
    void* gui_manager_get_or_create_004c12b0() override { unused("004C12B0"); }
    void gui_manager_remove_and_destroy_page_00aa31f0(void*, void*) override { unused("00AA31F0"); }
    void node_list_clear_005234f0(void*) override { unused("005234F0"); }
    void callback_owner_destroy_00695870(void*) override { unused("00695870"); }
    void observer_endpoint_destruct_00695760(void*) override { unused("00695760"); }
    void owned_object_list_clear_005b7840(void*) override { unused("005B7840"); }
    void list_tidy_005b7570(void*) override { unused("005B7570"); }
    void list_clear_005b77f0(void*) override { unused("005B77F0"); }
    void* list_buy_head_node_005b7550(void*) override { unused("005B7550"); }
    void voice_slot_stop_007026f0(void*) override { unused("007026F0"); }
    void effect_sound_samples_resize_005b8fa0(void*, std::uint32_t) override { unused("005B8FA0"); }
    void effect_sound_samples_append_005b9af0(void*, void**) override { unused("005B9AF0"); }
    void debug_text_list_destroy_rows_00543e00(void*) override { unused("00543E00"); }
    void member_teardown_00544a70(void*) override { unused("00544A70"); }
    void debug_text_list_destruct_00590250(void*) override { unused("00590250"); }
    void cheat_menu_destroy_members_0051f910(void*) override { unused("0051F910"); }
    void list_tidy_005822a0(void*) override { unused("005822A0"); }
    void text_listener_base_destroy_00a97140(void*) override { unused("00A97140"); }
    void main_menu_release_mission_preview_005830a0(void*) override { unused("005830A0"); }
    void native_string_range_destroy_00432050(void*, void*, void*, void*) override { unused("00432050"); }
    void native_string_vector_resize_00427110(void*, std::uint32_t) override { unused("00427110"); }
    void warning_manager_clear_waiting_queue_0096bf70(void*) override { unused("0096BF70"); }
    void member_teardown_0097a190(void*) override { unused("0097A190"); }

private:
    [[noreturn]] static void unused(const char* what) {
        throw std::logic_error(std::string("00525A90 does not reach ") + what);
    }
    HudLeafScreenStorage& screen_;
    GameSoundRuntime& sound_;
    NativeStringStorage& strings_;
};

// 0068CC70's leaf 29h, like 33h and 50h, needs the started sound runtime: the
// sample cache [00F8BBE8] and the manager [00F8BBD8] exist from the sound startup
// on, before any mission is built.
void construct_hud_unit_pick_leaf(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    NativeStringStorage* const strings = g_hud_sound_strings;
    if (sound == nullptr || strings == nullptr || !sound->started())
        throw std::logic_error("00525A90 needs the started sound runtime that owns [00F8BBE8]");
    HudLeafUnitPickBinding b(screen, *sound, *strings);
    bsp::cg_rebuild_b_cc13::vt_ceccf8_construct_00525a90(screen.data(), b);
}

// 00524BA0's sample release on the 29h block (00524C1D..00524C57, bsp.py show
// 00524ba0): each set +DCh/+E0h sample is released (InterlockedDecrement
// [00CE2220] on +4h, vtable +0 at zero: 00A82E80) and both are cleared. The rest
// of 00524BA0 has nothing to undo on this block: the +3Ch node list and the
// +C8h..+D0h vector stay empty (the host's unit-pick logic keeps its own state),
// the +BCh block's NativeString at +10h stays empty and the block itself is the
// leaf's owned allocation, freed with it; its two inline subobject bases (+8h,
// +18h) hold only the constructor's zeros. As for 33h, the sound runtime is still
// bound and started when the HUD dies (see release_hud_narrative_leaf_samples).
void release_hud_unit_pick_leaf_samples(HudLeafScreenStorage& screen) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    if (sound == nullptr || !sound->started())
        throw std::logic_error("00524BA0's sample release needs the sound runtime that owns [00F8BBE8]");
    for (std::uint32_t i = 0; i < 2; ++i) {
        void* sample = nullptr;
        std::memcpy(&sample, screen.data() + 0xDC + i * 4, sizeof(sample));
        if (sample != nullptr) {                                               // 00524C2A
            auto* const references = reinterpret_cast<volatile long*>(
                static_cast<std::uint8_t*>(sample) + 4);
            if (_InterlockedDecrement(references) == 0)
                sound->samples().zero_references_slot_00(sample);
        }
        std::memset(screen.data() + 0xDC + i * 4, 0, sizeof(void*));           // 00524C45
    }
}

const bsp::cc13_fe5::SmallHudScreenShape& small_hud_shape(std::uint32_t constructor) {
    for (const auto& shape : bsp::cc13_fe5::kSmallHudScreens)
        if (shape.construct == constructor) return shape;
    throw std::logic_error("no small HUD screen shape for this constructor");
}

// The leaf constructors 0068CC70 calls (kInGameHudScreens) that run here.
constexpr std::array<std::uint32_t, 33> kHudLeafScreenConstructors{{
    0x0068A8A0u, 0x00640C80u, 0x0051E950u, 0x005472F0u, 0x005AAEF0u, 0x005C0810u,
    0x00538E30u, 0x00608E60u, 0x00606470u, 0x0060CAE0u, 0x00650B00u,
    0x00650140u, 0x00540BA0u, 0x0054F8C0u, 0x005CE610u, 0x005CC120u,
    0x005CB0A0u, 0x005FFCB0u,
    0x005178B0u, 0x00604780u, 0x0060FD90u, 0x005D3540u, 0x00565D40u,
    0x00643DA0u, 0x006735D0u, 0x0061A480u, 0x0052B030u, 0x005EE440u,
    0x005F9440u, 0x0067BE30u, 0x005BA7B0u, 0x006821C0u, 0x00525A90u,
}};
bool is_hud_leaf_screen_constructor(std::uint32_t constructor) noexcept {
    return std::find(kHudLeafScreenConstructors.begin(), kHudLeafScreenConstructors.end(),
        constructor) != kHudLeafScreenConstructors.end();
}

// 0068CC70's `call constructor` on the block allocate_screen made.
void run_hud_leaf_screen_constructor(std::uint32_t constructor, HudLeafScreenStorage& screen) {
    std::uint8_t* const p = screen.data();
    const bsp::ingame_gui_cc13::Raw raw{p};
    switch (constructor) {
    case 0x0067BE30u: { HudLeafGuiBaseBinding b(screen); bsp::ingame_gui_cc13::follow_screen_construct_0067be30(raw, b); return; }
    case 0x00643DA0u: { HudLeafGuiBaseBinding b(screen); bsp::ingame_gui_cc13::spectator_construct_00643da0(raw, b); return; }
    case 0x00650140u: { HudLeafGuiBaseBinding b(screen); bsp::ingame_gui_cc13::submarine_construct_00650140(raw, b); return; }
    case 0x00650B00u: { HudLeafGuiBaseBinding b(screen); bsp::ingame_gui_cc13::periscope_construct_00650b00(raw, b); return; }
    case 0x0052B030u: { HudLeafGuiBaseBinding b(screen); bsp::frontend_seg15_22_cc13::hud_counters_construct_0052b030(raw, b); return; }
    case 0x005D3540u: { HudLeafGuiBaseBinding b(screen); bsp::frontend_seg15_22_cc13::pause_multi_construct_005d3540(raw, b); return; }
    case 0x00565D40u: { HudLeafGuiBaseBinding b(screen); bsp::cc13_fe3::hud_limbo_construct_00565d40(raw, b); return; }
    case 0x005178B0u: { HudLeafGuiBaseBinding b(screen); bsp::cc13_fe3::in_game_screen_23_construct_005178b0(raw, b); return; }
    case 0x0051E950u: { HudLeafSeg14BaseBinding b(screen); bsp::cc13_seg14::binoculars_screen_construct_0051e950(p, b); return; }
    case 0x005EE440u: { HudLeafVoidBaseBinding<bsp::cc13_fe1::FrontEndScreenConstructHost> b(screen); bsp::cc13_fe1::hud_objectives_construct_005ee440(p, b); return; }
    case 0x005F9440u: { HudLeafVoidBaseBinding<bsp::cc13_fe1::FrontEndScreenConstructHost> b(screen); bsp::cc13_fe1::hud_order_construct_005f9440(p, b); return; }
    case 0x00604780u: { HudLeafVoidBaseBinding<bsp::cc13_fe1::FrontEndScreenConstructHost> b(screen); bsp::cc13_fe1::hud_pause_cheat_construct_00604780(p, b); return; }
    case 0x005CB0A0u: { HudLeafVoidBaseBinding<bsp::frontend_seg19_21_cc13::FrontEndScreenConstructHost> b(screen); bsp::frontend_seg19_21_cc13::hud_engine_movie_screen_construct_005cb0a0(p, b); return; }
    case 0x0060FD90u: { HudLeafVoidBaseBinding<bsp::seg25_cc13::FrontEndScreenConstructHost> b(screen); bsp::seg25_cc13::hud_scoring_screen_construct_0060fd90(p, b); return; }
    case 0x005472F0u: { HudLeafFe5Binding b(screen); bsp::cc13_fe5::screen2e_construct_005472f0(raw, b); return; }
    case 0x0054F8C0u: { HudLeafFe5Binding b(screen); bsp::cc13_fe5::idle_camera_screen_construct_0054f8c0(raw, b); return; }
    case 0x00608E60u: { HudLeafFe5Binding b(screen); bsp::cc13_fe5::plane_screen_construct_00608e60(raw, b); return; }
    case 0x00606470u: { HudLeafFe5Binding b(screen); bsp::cc13_fe5::plane_effects_construct_00606470(raw, b); return; }
    case 0x0060CAE0u:
    case 0x0068A8A0u: { HudLeafFe5Binding b(screen); bsp::cc13_fe5::small_hud_screen_construct(small_hud_shape(constructor), raw, b); return; }
    case 0x00640C80u: construct_hud_markers_leaf(screen); return;
    case 0x00538E30u: { HudLeafSeg15Binding b(screen); bsp::frontend_seg15_22_cc13::hud_formation_construct_00538e30(raw, b); return; }
    case 0x00540BA0u: { HudLeafSeg15Binding b(screen); bsp::frontend_seg15_22_cc13::free_camera_construct_00540ba0(raw, b); return; }
    case 0x005CE610u: { HudLeafSeg15Binding b(screen); bsp::frontend_seg15_22_cc13::movie_camera_debug_construct_005ce610(raw, b); return; }
    case 0x005C0810u: { HudLeafMinimapBinding b(screen); bsp::cc13_fe1::hud_minimap_screen_construct_005c0810(p, b); return; }
    case 0x0061A480u: { HudLeafPowerupBinding b(screen); bsp::seg25_cc13::powerup_manager_construct_0061a480(p, b); return; }
    case 0x006735D0u: { HudLeafSupportBinding b(screen); bsp::seg27_cc13::support_manager_screen_construct_006735d0(raw, b); return; }
    case 0x005CC120u: { HudLeafVoidBaseBinding<bsp::frontend_seg19_21_cc13::FrontEndScreenConstructHost> b(screen); bsp::frontend_seg19_21_cc13::hud_movie_screen_construct_005cc120(p, b); return; }
    case 0x005FFCB0u: { HudLeafPauseBinding b(screen); bsp::cc13_fe1::hud_pause_screen_construct_005ffcb0(p, b); return; }
    case 0x005AAEF0u: { HudLeafSweepB2Binding b(screen); bsp::cc13_sweepB2_ingame_interface_screen_construct_005aaef0(p, b); return; }
    case 0x005BA7B0u: construct_hud_narrative_leaf(screen); return;
    case 0x006821C0u: construct_hud_warning_leaf(screen); return;
    case 0x00525A90u: construct_hud_unit_pick_leaf(screen); return;
    default: break;
    }
    throw std::logic_error("0068CC70 leaf constructor listed but not bound");
}

// The leaf register overrides (+10h, kHudScreenLayouts) that run on their
// blocks: each is 004F71D0 and at most one page load (bsp.py show <address>
// --asm; 0060D1C0, 00565EE0 and 0051ED60 build the name in a native-string temporary),
// and each page is the one kHudScreenLayouts lists for the slot. 0067BE80,
// 00650170 and 005F9FF0 also call the screen's +14h layout virtual
// (run_hud_leaf_screen_layout). 005CCE70 (38h) loads its page and two children
// and reads the platform object's widescreen byte. 00643E80 (43h) loads
// GUI_spectator into +8h and then stores +4h = 1 (00643F26), the wanted byte of
// the flags the registry borrows. 0052CBF0 (51h, goal 5 chain 662) loads
// GUI_counters into +8h and builds its two counter entries.
constexpr std::array<std::uint32_t, 14> kHudLeafScreenRegisters{{
    0x0067B3F0u, 0x005176A0u, 0x005D38A0u, 0x005EE010u, 0x00604890u, 0x0060D1C0u, 0x00565EE0u,
    0x0051ED60u, 0x0067BE80u, 0x00650170u, 0x005F9FF0u, 0x005CCE70u, 0x00643E80u, 0x0052CBF0u,
}};
bool is_hud_leaf_screen_register(std::uint32_t register_virtual) noexcept {
    return std::find(kHudLeafScreenRegisters.begin(), kHudLeafScreenRegisters.end(),
        register_virtual) != kHudLeafScreenRegisters.end();
}

// What a leaf register's 004F71D0 and 00AA5840 reach: the registry the
// menu owner keeps (registering this block's own base flags) and the page
// that registry loads for the slot. The page root is what 00AA5840 returns.
struct HudLeafRegisterContext {
    HudLeafScreenStorage& screen;
    GameMenuHost& menu;
    int slot;
    std::string name;
    std::uint32_t register_virtual;
    std::uint32_t layout_virtual;

    void register_base(const void* p) {
        if (p != screen.data())
            throw std::logic_error("004F71D0 base belongs to a different HUD screen");
        if (!menu.register_in_game_screen(slot, name, register_virtual, layout_virtual,
                &screen.flags()))
            throw std::logic_error("004F71D0 could not register a free HUD slot");
    }
    void* load_page(std::string_view page_name) {
        const std::string page(page_name);
        menu.attach_in_game_page(slot, page);
        GuiLayoutPage* loaded = menu.in_game_page(slot, page);
        return loaded != nullptr && loaded->root ? loaded->root.get() : nullptr;
    }
};
class HudLeafRegisterBinding final : public bsp::ingame_gui_cc13::HudScreenRegisterPageHost {
public:
    explicit HudLeafRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void front_end_screen_register_004f71d0(bsp::ingame_gui_cc13::Raw screen) override {
        context_.register_base(screen.p);
    }
    void* gui_load_page_00aa5840(std::string_view name) override { return context_.load_page(name); }
private:
    HudLeafRegisterContext& context_;
};
class HudLeafScreenRegisterBinding final : public bsp::cc13_fe1::ScreenRegisterPageHost {
public:
    explicit HudLeafScreenRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void frontend_register_004f71d0(void* screen) override { context_.register_base(screen); }
    std::uint32_t load_page_00aa5840(std::string_view name) override {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(context_.load_page(name)));
    }
private:
    HudLeafRegisterContext& context_;
};

// The register overrides' native-string temporaries, over this process's
// actual string pool: 0041DD40 and the 00419CC0/00BD1510 pair on the
// 01090AA8/01090AA0/01090AA4 cells game_native_string_process() publishes,
// as the menu host's 0090C5D0 binding does.
struct HudLeafStringPool {
    bsp::NativeStringRawPoolContext& strings{game_native_string_process().raw_context()};
    void resize(void* header, std::uint32_t length, bool preserve) {
        bsp::resize_native_string_header_0041dd40(header, strings, length, preserve);
    }
    void* pool() {
        return bsp::native_string_pool_get_or_create_00419cc0(strings.actual_published_01090aa8,
            strings.actual_manager_publication_01090aa0);
    }
    void give_back(void* pool, void* block, std::uint32_t size) {
        bsp::return_native_string_pool_00bd1510(static_cast<bsp::NativeStringPoolStorage*>(pool),
            block, size, strings.actual_small_returns_disabled_01090aa4);
    }
    // The eight-byte header 00AA5840 takes the name in: length +0h, data +4h.
    static std::string_view name(const void* header) {
        std::uint32_t length = 0;
        const char* data = nullptr;
        std::memcpy(&length, header, sizeof(length));
        std::memcpy(&data, static_cast<const std::uint8_t*>(header) + 4, sizeof(data));
        return data != nullptr ? std::string_view(data, length) : std::string_view();
    }
};
// 0060D1C0. Its 004C12B0 is the registry owner that loads the page.
class HudLeafPlaneSpawnRegisterBinding final : public bsp::seg25_cc13::PlaneSpawnRegisterHost {
public:
    explicit HudLeafPlaneSpawnRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void native_string_resize_0041dd40(bsp::seg25_cc13::NativeStr* s, std::uint32_t length,
        std::int32_t flag) override { pool_.resize(s, length, flag != 0); }
    void* pool_singleton_00419cc0() override { return pool_.pool(); }
    void pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size, std::int32_t) override {
        pool_.give_back(pool, block, size);  // the unused stack argument is 1
    }
    void frontend_register_004f71d0(void* screen) override { context_.register_base(screen); }
    void* gui_manager_004c12b0() override { return &context_.menu; }
    std::uint32_t gui_load_page_00aa5840(void* manager, const bsp::seg25_cc13::NativeStr* name,
        std::int32_t, std::int32_t) override {
        if (manager != &context_.menu)
            throw std::logic_error("0060D1C0 loaded its page through another GUI manager");
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(
            context_.load_page(HudLeafStringPool::name(name))));
    }
private:
    HudLeafRegisterContext& context_;
    HudLeafStringPool pool_;
};
// 00565EE0. Its 00BD1510 runs on the 00419CC0 pool, both inside this service.
class HudLeafLimboRegisterBinding final : public bsp::cc13_fe3::HudLimboRegisterHost {
public:
    explicit HudLeafLimboRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void front_end_screen_register_004f71d0(bsp::ingame_gui_cc13::Raw screen) override {
        context_.register_base(screen.p);
    }
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint32_t preserve) override {
        pool_.resize(string, length, preserve != 0);
    }
    void* gui_load_page_00aa5840(const void* name, std::uint32_t, std::uint32_t) override {
        return context_.load_page(HudLeafStringPool::name(name));
    }
    void sized_pool_return_00bd1510(void* block, std::uint32_t bytes, std::uint32_t) override {
        pool_.give_back(pool_.pool(), block, bytes);
    }
private:
    HudLeafRegisterContext& context_;
    HudLeafStringPool pool_;
};

// 0051ED60. Its name temporary is built in this process's actual string pool
// (game_native_string_process().strings(), passed as the body's storage); its
// 004C12B0 manager is the registry owner that loads the page.
class HudLeafBinocularsRegisterBinding final : public bsp::cc13_seg14::BinocularsRegisterHooks {
public:
    explicit HudLeafBinocularsRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void front_end_screen_register_004f71d0(void* screen) override { context_.register_base(screen); }
    void* gui_manager_get_or_create_004c12b0() override { return &context_.menu; }
    void* gui_manager_load_page_00aa5840(void* manager, void* name, std::uint32_t, std::uint32_t) override {
        if (manager != &context_.menu)
            throw std::logic_error("0051ED60 loaded its page through another GUI manager");
        return context_.load_page(HudLeafStringPool::name(name));
    }
private:
    HudLeafRegisterContext& context_;
};

// 005CCE70, screen 38h (goal 5 chain 635). Its three names are image strings
// (bsp.py ghidra bytes 00cf1930 --length 28: 00CF1930 "down_Icon", 00CF193C
// "up_Icon", 00CF1944 "GUI_movie") copied into native-string temporaries of the
// process's string pool; its 004C12B0 manager is the registry owner that loads the
// page; 00AA7E00(name, 1) searches the page's descendants. [0109CF04] is the
// platform object 00BE2960 publishes (GameStartupHost's native platform, whose view
// active_platform_state() returns): its +0h block is the window state and +0Dh
// the widescreen byte the body reads for +1Ch.
class HudLeafEngineMovieRegisterBinding final : public bsp::Cc13SweepB2EngineMovieRegisterHost {
public:
    explicit HudLeafEngineMovieRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void* image_global(std::uint32_t address) override {
        static constexpr char kDownIcon[] = "down_Icon";  // 00CF1930
        static constexpr char kUpIcon[] = "up_Icon";      // 00CF193C
        static constexpr char kGuiMovie[] = "GUI_movie";  // 00CF1944
        switch (address) {
        case 0x00CF1930u: return const_cast<char*>(kDownIcon);
        case 0x00CF193Cu: return const_cast<char*>(kUpIcon);
        case 0x00CF1944u: return const_cast<char*>(kGuiMovie);
        case 0x0109CF04u: {
            const Win32PlatformState* const platform = active_platform_state();
            if (platform == nullptr)
                throw std::logic_error("005CCE70 read [0109CF04] with no published platform object");
            static_assert(offsetof(NativePlatformObjectStorage, window) == 0);
            static_assert(offsetof(NativePlatformWindowFocusStorage, widescreen_0d) == 0x0D);
            platform_object_ = &platform->native_window_focus();
            return &platform_object_;
        }
        default: break;
        }
        throw std::logic_error("005CCE70 read an image global it does not use");
    }
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint8_t preserve) override {
        pool_.resize(string, length, preserve != 0);
    }
    void* crt_memcpy_00bf7680(void* dest, const void* source, std::uint32_t size) override {
        return std::memcpy(dest, source, size);
    }
    void* sized_pool_get_00419cc0() override { return pool_.pool(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes, std::int32_t) override {
        pool_.give_back(pool, data, bytes);  // the flag argument is 1
    }
    void frontend_screen_register_004f71d0(void* screen) override { context_.register_base(screen); }
    void* gui_manager_get_004c12b0() override { return &context_.menu; }
    void* gui_manager_load_page_00aa5840(void* manager, const void* name, std::int32_t,
        std::int32_t) override {
        if (manager != &context_.menu)
            throw std::logic_error("005CCE70 loaded its page through another GUI manager");
        return context_.load_page(HudLeafStringPool::name(name));
    }
    void* widget_find_child_00aa7e00(void* widget, void* name, std::int32_t) override {
        if (widget == nullptr) return nullptr;
        return bsp::find_descendant_by_name(*static_cast<GuiLayoutWidget*>(widget),
            HudLeafStringPool::name(name));
    }
private:
    HudLeafRegisterContext& context_;
    HudLeafStringPool pool_;
    const void* platform_object_{nullptr};
};

// Goal 5 chain 662: the HUD counters screen (51h), the countdown / unit-HP /
// score panel the mission scripts drive. Its register 0052CBF0 (bsp.py show
// 0052cbf0 --asm) is 004F71D0, GUI_counters (00CED190) through 00AA5840(name,
// 1, 0) into +8h, then two 0ACh blocks, each operator new 00BF55BE and memset
// 00BF79F0: +0Ch = 0052C1B0(page root, the twelve names), +10h = 0052C640(+0Ch);
// both leave their root widget hidden. The widgets are the page's own GuiLayoutWidgets, as for
// the other leaves; the blocks are owned by the screen block, which the native
// destructor 0052CDF0 frees them with.
GuiLayoutWidget& hud_counters_widget(void* widget, const char* routine) {
    if (widget == nullptr)
        throw std::logic_error(std::string(routine) + " reached a widget GUI_counters lacks");
    return *static_cast<GuiLayoutWidget*>(widget);
}

// 0052C1B0's services. Each name temporary is built in this process's actual
// string pool (0041E870, bsp.py show 0041e870: a constructor, length and data
// zeroed first) and returned to it; 00AA7E00 is the direct-child lookup.
// 00AA67F0 is LEA EAX,[ECX+0Ch] and 00AA6740 hands out +20h: the widget's Pos and
// Size (GuiWidgetTransform). Virtual +34h is 00AA8530, which tests only BL.
class HudCounterEntryConstructBinding final : public bsp::Cc13SweepB4CounterEntryHost {
public:
    explicit HudCounterEntryConstructBinding(GameFrontendHost& frontend) : frontend_(frontend) {}
    void* native_string_assign_0041e870(void* string, const char* text) override {
        return bsp::construct_native_string_header_0041e870(string, pool_.strings, text);
    }
    void* sized_pool_get_00419cc0() override { return pool_.pool(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes, std::int32_t) override {
        pool_.give_back(pool, data, bytes);  // the flag argument is 1
    }
    void* widget_find_child_00aa7e00(void* widget, void* name, std::int32_t) override {
        return bsp::find_child_by_name_00aa7e00(hud_counters_widget(widget, "00AA7E00"),
            HudLeafStringPool::name(name));
    }
    const float* gui_widget_get_local_position_00aa67f0(void* widget) override {
        return &hud_counters_widget(widget, "00AA67F0").transform.position.x;
    }
    const float* gui_widget_get_size_00aa6740(void* widget) override {
        return &hud_counters_widget(widget, "00AA6740").transform.size.width;
    }
    void widget_vcall_34_set_visible(void* widget, std::int32_t show) override {
        frontend_.set_widget_visible(hud_counters_widget(widget, "00AA8530"), (show & 0xFF) != 0);
    }
private:
    GameFrontendHost& frontend_;
    HudLeafStringPool pool_;
};

// 0052C640's services. 00AAB4C0(widget, null) copies the subtree under the
// source's parent (GameFrontendHost::clone_page_widget_subtree); virtual
// +28h hands out the widget's own name, an eight-byte {length, data} header that
// 00AA7E00 reads, which here points into the widget's key.
class HudCounterEntryCloneBinding final : public bsp::Cc13SweepBCounterCloneHost {
public:
    HudCounterEntryCloneBinding(GameFrontendHost& frontend, std::string page)
        : frontend_(frontend), page_(std::move(page)) {}
    void widget_vcall_34_set_visible(void* widget, std::int32_t show) override {
        frontend_.set_widget_visible(hud_counters_widget(widget, "00AA8530"), (show & 0xFF) != 0);
    }
    void* widget_vcall_28_name(void* widget) override {
        const GuiLayoutWidget& w = hud_counters_widget(widget, "widget +28h");
        std::array<std::uint32_t, 2> header{static_cast<std::uint32_t>(w.key.size()),
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(w.key.c_str()))};
        names_.push_back(header);
        return names_.back().data();
    }
    void* widget_find_child_00aa7e00(void* widget, void* name, std::int32_t) override {
        return bsp::find_child_by_name_00aa7e00(hud_counters_widget(widget, "00AA7E00"),
            HudLeafStringPool::name(name));
    }
    void* widget_clone_subtree_00aab4c0(void* widget, void* parent_clone) override {
        if (parent_clone != nullptr)
            throw std::logic_error("0052C640 passes 00AAB4C0 a null parent clone");
        return frontend_.clone_page_widget_subtree(page_, hud_counters_widget(widget, "00AAB4C0"));
    }
private:
    GameFrontendHost& frontend_;
    std::string page_;
    std::deque<std::array<std::uint32_t, 2>> names_;
};

class HudLeafCountersRegisterBinding final : public bsp::frontend_seg15_22_cc13::HudCountersRegisterHost {
public:
    explicit HudLeafCountersRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void front_end_screen_register_004f71d0(bsp::ingame_gui_cc13::Raw screen) override {
        context_.register_base(screen.p);
    }
    void* gui_load_page_00aa5840(std::string_view name) override {
        page_ = std::string(name);
        return context_.load_page(name);
    }
    bsp::ingame_gui_cc13::Raw counter_entry_construct_0052c1b0(void* page,
        const std::string_view (&names)[12]) override {
        const char* cnames[12];
        for (std::size_t i = 0; i < 12; ++i) cnames[i] = names[i].data();  // image literals
        void* const entry = context_.screen.allocate_owned(kCounterEntryBytes);  // zeroed
        HudCounterEntryConstructBinding b(context_.menu.frontend());
        bsp::cc13_sweepB4_hud_counter_entry_construct_0052c1b0(entry, page, cnames, b);
        return {static_cast<std::uint8_t*>(entry)};
    }
    bsp::ingame_gui_cc13::Raw counter_entry_clone_0052c640(bsp::ingame_gui_cc13::Raw source) override {
        if (source.p == nullptr)
            throw std::logic_error("0052C640 clones a counter entry 0052C1B0 did not build");
        void* const entry = context_.screen.allocate_owned(kCounterEntryBytes);  // zeroed
        HudCounterEntryCloneBinding b(context_.menu.frontend(), page_);
        bsp::cc13_sweepB_counter_entry_construct_clone_0052c640(entry, source.p, b);
        return {static_cast<std::uint8_t*>(entry)};
    }
private:
    static constexpr std::size_t kCounterEntryBytes = 0xAC;  // 0052CC93 / 0052CD33 PUSH 0ACh
    HudLeafRegisterContext& context_;
    std::string page_;
};

// The countdown path on the same block: 0052B9B0 (Lua Countdown, 008B18B8) and
// 0052AB90 (CountdownCancel, 008B1A92 / 008B1AA2), with 0052A860 and 00529A30 /
// 00529810 under them. What they reach of the image: the entry widgets (the
// page's GuiLayoutWidgets 0052C1B0 / 0052C640 stored), the counters block's own
// visible list (+20h, 00529900 growing it with 00BF55BE / 00BF6989 blocks the
// screen owns), [00E198C4] (this HUD's interface manager, as the image object:
// +N is the leaf block 0068CC70 built for manager offset N, kInGameHudScreens),
// and [0109CF04] (the platform object, +0Dh the widescreen byte).
struct HudCountersContext {
    GameFrontendHost& frontend;
    HudLeafScreenStorage& counters;
    std::array<std::uint8_t*, 0x40> manager_view{};      // [00E198C4]+0h..+FCh
    const void* platform_object{nullptr};                 // [0109CF04]
    std::set<const GuiLayoutWidget*>& visibility_recurses;  // widget +75h (00AA6820)
    // The HUD host's unimplemented-call log (GameHudHost::Impl::record).
    std::function<void(const char*, std::uint32_t)> record{};
    const bsp::SessionParticipantPools* score_participants{nullptr};
    const std::size_t* score_local_slot{nullptr};
};

// 00AA8530 (widget +34h): the low byte is the request; with the widget's +75h
// set, 00AA8450 hands every descendant the request as its own visibility
// (apply_requested inherited), otherwise only this widget's own flag moves.
void hud_counters_set_visible_00aa8530(HudCountersContext& c, GuiLayoutWidget& widget, bool visible) {
    if (c.visibility_recurses.count(&widget) == 0) {
        c.frontend.set_widget_visible(widget, visible);
        return;
    }
    std::vector<GuiLayoutWidget*> walk{&widget};
    while (!walk.empty()) {
        GuiLayoutWidget* const w = walk.back();
        walk.pop_back();
        c.frontend.set_widget_visible(*w, visible);
        for (const auto& child : w->children) {
            if (child) walk.push_back(child.get());
        }
    }
}

// 00AA8240: the local translation that puts the widget at `world`
// (local_position_for_resolved); +08h, the authored X, is not written.
void hud_counters_set_resolved_00aa8240(HudCountersContext& c, GuiLayoutWidget& widget, const float* world) {
    const GuiWidgetPoint local = local_position_for_resolved(widget.transform,
        GuiWidgetPoint{world[0], world[1], world[2]});
    const float authored_x = widget.transform.authored_x;
    c.frontend.set_widget_local_position(widget, local.x, local.y, local.z);
    widget.transform.authored_x = authored_x;
    c.frontend.refresh_widget_geometry(widget);
}

void hud_counters_resolved_00aa6750(const GuiLayoutWidget& widget, float* out) {
    const GuiWidgetPoint p = resolved_position(widget.transform);
    out[0] = p.x;
    out[1] = p.y;
    out[2] = p.z;
}

// 00529A30 / 00529810 and the four passes under the occlusion step.
class HudCountersLayoutBinding final : public bsp::frontend_seg15_22_cc13::HudCountersLayoutHost {
public:
    explicit HudCountersLayoutBinding(HudCountersContext& c) : c_(c) {}
    bool widget_is_visible_38(void* widget) override {
        return c_.frontend.widget_own_visible(hud_counters_widget(widget, "00A9E0D0"));
    }
    void widget_set_visible_34(void* widget, bool visible) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), visible);
    }
    const float* widget_local_position_00aa67f0(void* widget) override {
        return &hud_counters_widget(widget, "00AA67F0").transform.position.x;
    }
    void widget_resolved_position_00aa6750(void* widget, float out[3]) override {
        hud_counters_resolved_00aa6750(hud_counters_widget(widget, "00AA6750"), out);
    }
    void widget_set_resolved_position_00aa8240(void* widget, const float xyz[3]) override {
        hud_counters_set_resolved_00aa8240(c_, hud_counters_widget(widget, "00AA8240"), xyz);
    }
    const float* widget_size_00aa6740(void* widget) override {
        return &hud_counters_widget(widget, "00AA6740").transform.size.width;
    }
    bsp::ingame_gui_cc13::Raw global_object(std::uint32_t address) override {
        if (address == bsp::frontend_seg15_22_cc13::kInterfaceGlobal)
            return {reinterpret_cast<std::uint8_t*>(c_.manager_view.data())};
        if (address == bsp::frontend_seg15_22_cc13::kDisplayModeGlobal) {
            // Read only (+0Dh); the platform object is the published one.
            return {static_cast<std::uint8_t*>(const_cast<void*>(c_.platform_object))};
        }
        throw std::logic_error("the HUD counters read an image global they do not use");
    }
    bool support_manager_mode_006529e0(bsp::ingame_gui_cc13::Raw manager) override {
        // The support-manager screen 4Eh's own block (the interface update reads
        // the same bytes, see camera_screen_spectating).
        return bsp::cc13_front4::overlay_busy_006529e0({manager.p}) != 0;
    }
    void counters_restore_hidden_a7_005292e0(bsp::ingame_gui_cc13::Raw screen) override {
        run_flag_pass(screen, bsp::cc13_band0050::restore_hidden_a7_005292e0);
    }
    void counters_hide_all_a6_005290d0(bsp::ingame_gui_cc13::Raw screen) override {
        run_flag_pass(screen, bsp::cc13_band0050::hide_all_a6_005290d0);
    }
    void counters_restore_hidden_a6_00529350(bsp::ingame_gui_cc13::Raw screen) override {
        run_flag_pass(screen, bsp::cc13_band0050::restore_hidden_a6_00529350);
    }
private:
    // src/band_0050_cc13.cpp's passes take the visible list (+20h, count +24h) as
    // HudCounterEntryFlags: each entry's widget +38h and its bytes +A5h..+A8h.
    // What a pass changes goes back as the image does it: widget +34h, the bytes.
    void run_flag_pass(bsp::ingame_gui_cc13::Raw s,
        void (*pass)(std::vector<bsp::cc13_band0050::HudCounterEntryFlags>&)) {
        const std::int32_t count = s.at<std::int32_t>(0x24);
        auto* const* const list = static_cast<std::uint8_t* const*>(s.ptr(0x20));
        std::vector<bsp::cc13_band0050::HudCounterEntryFlags> flags(
            static_cast<std::size_t>(count > 0 ? count : 0));
        for (std::size_t i = 0; i < flags.size(); ++i) {
            const bsp::ingame_gui_cc13::Raw e{list[i]};
            flags[i].widget_visible = widget_is_visible_38(e.ptr(0x0));
            flags[i].hidden_a5 = e.at<std::uint8_t>(0xA5) != 0;
            flags[i].hidden_a6 = e.at<std::uint8_t>(0xA6) != 0;
            flags[i].hidden_a7 = e.at<std::uint8_t>(0xA7) != 0;
            flags[i].hidden_a8 = e.at<std::uint8_t>(0xA8) != 0;
        }
        std::vector<bsp::cc13_band0050::HudCounterEntryFlags> after = flags;
        pass(after);
        for (std::size_t i = 0; i < flags.size(); ++i) {
            const bsp::ingame_gui_cc13::Raw e{list[i]};
            if (after[i].widget_visible != flags[i].widget_visible)
                widget_set_visible_34(e.ptr(0x0), after[i].widget_visible);
            e.at<std::uint8_t>(0xA5) = after[i].hidden_a5 ? 1u : 0u;
            e.at<std::uint8_t>(0xA6) = after[i].hidden_a6 ? 1u : 0u;
            e.at<std::uint8_t>(0xA7) = after[i].hidden_a7 ? 1u : 0u;
            e.at<std::uint8_t>(0xA8) = after[i].hidden_a8 ? 1u : 0u;
        }
    }
    HudCountersContext& c_;
};

// 0052A860's services on the countdown entry's widgets.
class HudCounterCountdownLayoutBinding final : public bsp::Cc13SweepB4CounterCountdownLayoutHost {
public:
    explicit HudCounterCountdownLayoutBinding(HudCountersContext& c) : c_(c) {}
    void gui_widget_set_visibility_recursive_00aa6820(void* widget, std::int32_t show) override {
        // 00AA6820 stores the byte +75h (src/gui_widgets_cc13.cpp).
        const GuiLayoutWidget* const w = &hud_counters_widget(widget, "00AA6820");
        if ((show & 0xFF) != 0) c_.visibility_recurses.insert(w);
        else c_.visibility_recurses.erase(w);
    }
    void widget_vcall_34_set_visible(void* widget, std::int32_t show) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), (show & 0xFF) != 0);
    }
    void widget_vcall_58_set_pair(void* widget, const float* pair) override {
        c_.frontend.set_widget_size(hud_counters_widget(widget, "widget +58h"), pair[0], pair[1]);
    }
    void widget_vcall_48_set_pair(void* widget, const float* pair) override {
        c_.frontend.set_widget_scale(hud_counters_widget(widget, "00AA7950"), pair[0], pair[1]);
    }
    void gui_text_store_align_100(void* text, std::int32_t align) override {
        // The bridge lays a Text widget out with its authored Align (00ABB877);
        // GUI_counters authors Left (0) on both texts this body stores 0 into.
        const GuiLayoutWidget& w = hud_counters_widget(text, "Text +100h");
        const GuiValue* const authored = w.source != nullptr ? w.source->find("Align") : nullptr;
        const bsp::GuiTextAlign current = authored != nullptr && authored->kind() == GuiValue::Kind::String
            ? bsp::parse_gui_text_align_00abb877(authored->string()) : bsp::GuiTextAlign::Left;
        if (static_cast<std::int32_t>(current) != align)
            throw std::logic_error("0052A860 stores a Text +100h the authored Align does not carry");
    }
    float gui_text_measured_width_114(void* text) override {
        float width = 0.0f;
        if (!c_.frontend.widget_text_width(hud_counters_widget(text, "Text +114h"), width))
            throw std::logic_error("0052A860 reads Text +114h with no text layout to measure it");
        return width;
    }
    void gui_text_set_localised_source_00abaed0(void* widget, const void* text, std::int32_t arg) override {
        if (arg != 1) throw std::logic_error("0052A860 passes 00ABAED0 its localise flag 1");
        c_.frontend.set_widget_text_source(hud_counters_widget(widget, "00ABAED0"),
            std::string(HudLeafStringPool::name(text)));
    }
    const float* gui_widget_get_size_00aa6740(void* widget) override {
        return &hud_counters_widget(widget, "00AA6740").transform.size.width;
    }
    void gui_widget_set_local_position_00aa7dc0(void* widget, const float* position) override {
        GuiLayoutWidget& w = hud_counters_widget(widget, "00AA7DC0");
        c_.frontend.set_widget_local_position(w, position[0], position[1], position[2]);
        c_.frontend.refresh_widget_geometry(w);
    }
    float* widget_get_resolved_position_00aa6750(void* widget, float* out) override {
        hud_counters_resolved_00aa6750(hud_counters_widget(widget, "00AA6750"), out);
        return out;
    }
    void widget_set_resolved_position_00aa8240(void* widget, const float* position) override {
        hud_counters_set_resolved_00aa8240(c_, hud_counters_widget(widget, "00AA8240"), position);
    }
    void* sized_pool_get_00419cc0() override { return pool_.pool(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes, std::int32_t) override {
        pool_.give_back(pool, data, bytes);  // the flag argument is 1
    }
private:
    HudCountersContext& c_;
    HudLeafStringPool pool_;
};

class HudCountersShowBinding final : public bsp::frontend_seg15_22_cc13::HudCountersShowHost {
public:
    explicit HudCountersShowBinding(HudCountersContext& c) : c_(c) {}
    void* operator_new_00bf55be(std::size_t bytes) override { return c_.counters.allocate_owned(bytes); }
    void free_00bf6989(void* block) override { c_.counters.release_owned(block); }
    bool widget_is_visible_38(void* widget) override {
        return c_.frontend.widget_own_visible(hud_counters_widget(widget, "00A9E0D0"));
    }
    void widget_set_visible_34(void* widget, bool visible) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), visible);
    }
    void counter_layout_countdown_0052a860(bsp::ingame_gui_cc13::Raw entry, std::string_view text,
        float seconds) override {
        // The by-value native string 0052A860 releases, built in the process pool.
        const std::string copy(text);
        bsp::Cc13SweepBNativeString value{};
        bsp::construct_native_string_header_0041e870(&value, pool_.strings, copy.c_str());
        HudCounterCountdownLayoutBinding b(c_);
        bsp::cc13_sweepB4_hud_counter_layout_countdown_0052a860(entry.p, value, seconds, b);
    }
    void counters_relayout_00529a30(bsp::ingame_gui_cc13::Raw screen) override {
        HudCountersLayoutBinding b(c_);
        bsp::frontend_seg15_22_cc13::hud_counters_relayout_00529a30(screen, b);
    }
private:
    HudCountersContext& c_;
    HudLeafStringPool pool_;
};

// 00528B00's calls: +34h, and 00ABBE50(text, 1), whose case-insensitive gate on
// the cached source (+F4h, bsp.py show 00abbe50) returns before 00ABAED0 when
// the time string is unchanged.
class HudCounterFormatTimeBinding final : public bsp::frontend_seg15_22_cc13::HudCounterFormatTimeHost {
public:
    explicit HudCounterFormatTimeBinding(HudCountersContext& c) : c_(c) {}
    void widget_set_visible_34(void* widget, bool visible) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), visible);
    }
    void text_set_localised_00abbe50(void* text, std::string_view value) override {
        GuiLayoutWidget& w = hud_counters_widget(text, "00ABBE50");
        const std::string* const cached = c_.frontend.widget_text_source(w);
        if (cached != nullptr && cached->size() == value.size()
            && _strnicmp(cached->data(), value.data(), value.size()) == 0) {
            return;
        }
        c_.frontend.set_widget_text_source(w, std::string(value));
    }
private:
    HudCountersContext& c_;
};

// Goal 5 chain 667: the services of the score layouts 00529D70 / 0052A100 and
// the tick's text-pair layout 0052A470 on a score entry (a 0052C640 clone of
// the HP entry). The widgets are the page's GuiLayoutWidgets, as for the
// countdown; the Text fields the image stores directly (+100h, +0FCh) become
// the text bridge's layout override, and +114h is the bridge's measured width.
class HudCounterScoreLayoutBinding final : public bsp::Cc13SweepB4CounterScoreLayoutHost {
public:
    explicit HudCounterScoreLayoutBinding(HudCountersContext& c) : c_(c) {}
    void gui_widget_set_visibility_recursive_00aa6820(void* widget, std::int32_t show) override {
        // 00AA6820 stores the byte +75h (src/gui_widgets_cc13.cpp).
        const GuiLayoutWidget* const w = &hud_counters_widget(widget, "00AA6820");
        if ((show & 0xFF) != 0) c_.visibility_recurses.insert(w);
        else c_.visibility_recurses.erase(w);
    }
    void widget_vcall_34_set_visible(void* widget, std::int32_t show) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), (show & 0xFF) != 0);
    }
    void widget_vcall_58_set_pair(void* widget, const float* pair) override {
        c_.frontend.set_widget_size(hud_counters_widget(widget, "widget +58h"), pair[0], pair[1]);
    }
    void widget_vcall_48_set_pair(void* widget, const float* pair) override {
        c_.frontend.set_widget_scale(hud_counters_widget(widget, "00AA7950"), pair[0], pair[1]);
    }
    void widget_vcall_88(void* widget, std::int32_t, std::int32_t, float) override {
        // Virtual +88h on flag_A_Icon / flag_B_Icon, the icon state select
        // (00AB1710); the sprite bridge draws a widget's first authored state only.
        static_cast<void>(hud_counters_widget(widget, "00AB1710"));
        if (c_.record) c_.record("HudCounters::score_flag_icon_state", 0x00ab1710u);
    }
    void widget_set_local_x_00aa78d0(void* widget, float x) override {
        // 00AA78D0 (bsp.py show 00aa78d0): MOVSS the argument into +0Ch (the
        // local X), then 00AA7220; +08h, the authored X, is not written.
        GuiLayoutWidget& w = hud_counters_widget(widget, "00AA78D0");
        const float authored_x = w.transform.authored_x;
        c_.frontend.set_widget_local_position(w, x, w.transform.position.y, w.transform.position.z);
        w.transform.authored_x = authored_x;
    }
    void gui_text_store_align_100(void* text, std::int32_t align) override {
        if (align < 0 || align > 2) throw std::logic_error("Text +100h stored outside Left..Right");
        c_.frontend.set_widget_text_align(hud_counters_widget(text, "Text +100h"),
            static_cast<bsp::GuiTextAlign>(align));
    }
    void gui_text_store_multiline_fc(void* text, std::uint8_t multiline) override {
        c_.frontend.set_widget_text_multiline(hud_counters_widget(text, "Text +0FCh"), multiline != 0);
    }
    float gui_text_measured_width_114(void* text) override {
        float width = 0.0f;
        if (!c_.frontend.widget_text_width(hud_counters_widget(text, "Text +114h"), width))
            throw std::logic_error("a score layout reads Text +114h with no text layout to measure it");
        return width;
    }
    void gui_text_set_localised_source_00abaed0(void* widget, const void* text, std::int32_t arg) override {
        if (arg != 1) throw std::logic_error("the score layouts pass 00ABAED0 its localise flag 1");
        c_.frontend.set_widget_text_source(hud_counters_widget(widget, "00ABAED0"),
            std::string(HudLeafStringPool::name(text)));
    }
    void gui_text_set_localised_cstring_00abbe50(void* text, const char* value, std::int32_t arg) override {
        if (arg != 1) throw std::logic_error("0052A470 passes 00ABBE50 its localise flag 1");
        // 00ABBE50's case-insensitive gate on the cached source (+F4h), as the
        // countdown's HudCounterFormatTimeBinding models it.
        GuiLayoutWidget& w = hud_counters_widget(text, "00ABBE50");
        const std::string_view v(value);
        const std::string* const cached = c_.frontend.widget_text_source(w);
        if (cached != nullptr && cached->size() == v.size()
            && _strnicmp(cached->data(), v.data(), v.size()) == 0) {
            return;
        }
        c_.frontend.set_widget_text_source(w, std::string(v));
    }
    const float* gui_widget_get_size_00aa6740(void* widget) override {
        return &hud_counters_widget(widget, "00AA6740").transform.size.width;
    }
    void gui_widget_set_local_position_00aa7dc0(void* widget, const float* position) override {
        GuiLayoutWidget& w = hud_counters_widget(widget, "00AA7DC0");
        c_.frontend.set_widget_local_position(w, position[0], position[1], position[2]);
        c_.frontend.refresh_widget_geometry(w);
    }
    float* widget_get_resolved_position_00aa6750(void* widget, float* out) override {
        hud_counters_resolved_00aa6750(hud_counters_widget(widget, "00AA6750"), out);
        return out;
    }
    void widget_set_resolved_position_00aa8240(void* widget, const float* position) override {
        hud_counters_set_resolved_00aa8240(c_, hud_counters_widget(widget, "00AA8240"), position);
    }
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::int32_t flag) override {
        pool_.resize(string, length, flag != 0);
    }
    void crt_memcpy_00bf7680(void* dest, const void* source, std::uint32_t bytes) override {
        std::memcpy(dest, source, bytes);
    }
    void* sized_pool_get_00419cc0() override { return pool_.pool(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes, std::int32_t) override {
        pool_.give_back(pool, data, bytes);  // the flag argument is 1
    }
private:
    HudCountersContext& c_;
    HudLeafStringPool pool_;
};

// The by-value native string a layout takes and releases, built in the process
// pool from a 0052CF60 copy (00426060 there, from the same text).
bsp::Cc13SweepBNativeString hud_counter_string_argument(HudLeafStringPool& pool, std::string_view text) {
    const std::string copy(text);
    bsp::Cc13SweepBNativeString value{};
    bsp::construct_native_string_header_0041e870(&value, pool.strings, copy.c_str());
    return value;
}

// 0052B080's services. The countdown entry is mode 2 (0052A860 stores it); the
// HP entry stays mode 3 (0052C1B0), which ticks nothing; a score entry is mode
// 1 (00529D70 / 0052A100 store it), whose tick re-lays out its two texts through
// 0052A470 (goal 5 chain 667). Mode 0 is the unit-HP layout 0052BB10 stores,
// which no path here runs, so its services fail loudly rather than answer.
class HudCounterTickBinding final : public bsp::Cc13SweepB4CounterTickHost {
public:
    explicit HudCounterTickBinding(HudCountersContext& c) : c_(c) {}
    bool widget_vcall_38_is_visible(void* widget) override {
        return c_.frontend.widget_own_visible(hud_counters_widget(widget, "00A9E0D0"));
    }
    void widget_vcall_34_set_visible(void* widget, std::int32_t show) override {
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(widget, "00AA8530"), (show & 0xFF) != 0);
    }
    bool session_counts_mission_start_004b6260() override {
        // 004B6260 on [00E188A8]+1EF0h: session+F4h (game+1FE4h) and +29Ch
        // (game+218Ch), the session context the mission frame reads.
        const bsp::SessionContext& session = bsp::session_context();
        return bsp::session_counts_mission_004b6260(session.session_word_1fe4 != 0,
            session.skirmish_218c);
    }
    void hud_counter_format_time_00528b00(void* entry, float seconds) override {
        HudCounterFormatTimeBinding b(c_);
        bsp::frontend_seg15_22_cc13::hud_counter_format_time_00528b00(
            {static_cast<std::uint8_t*>(entry)}, seconds, b);
    }
    void native_string_copy_construct_00426060(void* dest, const void* source) override {
        bsp::copy_construct_native_string_header_00426060(dest, source, pool_.strings);
    }
    void hud_counter_layout_text_pair_0052a470(void* entry, bsp::Cc13SweepBNativeString a,
        bsp::Cc13SweepBNativeString b) override {
        HudCounterScoreLayoutBinding binding(c_);
        bsp::cc13_sweepB4_hud_counter_layout_text_pair_0052a470(entry, a, b, binding);
    }
    void counter_units_remove_at_0052add0(void*, std::int32_t) override { unbuilt("0052ADD0"); }
    bool object_vcall_5c_is_kind(void*, std::int32_t) override { unbuilt("+5Ch"); }
    double getter_38_00939f70(void*) override { unbuilt("00939F70"); }
    double getter_34_00939f80(void*) override { unbuilt("00939F80"); }
    double getter_30_00939fc0(void*) override { unbuilt("00939FC0"); }
    double getter_2c_00939fb0(void*) override { unbuilt("00939FB0"); }
    void hud_counter_update_bar_00528e90(void*, float, float) override { unbuilt("00528E90"); }
private:
    [[noreturn]] static void unbuilt(const char* routine) {
        throw std::logic_error(std::string("0052B080 reached ") + routine
            + " on a unit-HP counter entry, which no HUD path builds");
    }
    HudCountersContext& c_;
    HudLeafStringPool pool_;
};

// 0052BAA0 (+20h) and 00528C40 / 00528C50 (+18h / +1Ch) on the counters block.
class HudCountersUpdateBinding final : public bsp::HudCountersHost {
public:
    explicit HudCountersUpdateBinding(HudCountersContext& c) : c_(c) {}
    void widget8_virtual_34(std::int32_t arg) override {
        void* const page = bsp::ingame_gui_cc13::Raw{c_.counters.data()}.ptr(0x8);
        hud_counters_set_visible_00aa8530(c_, hud_counters_widget(page, "counters +8h"), (arg & 0xFF) != 0);
    }
    void counter_tick_0052b080(std::uintptr_t counter, float seconds) override {
        HudCounterTickBinding b(c_);
        bsp::cc13_sweepB4_hud_counter_tick_0052b080(reinterpret_cast<void*>(counter), seconds, b);
    }
    void finish_00529810() override {
        HudCountersLayoutBinding b(c_);
        bsp::frontend_seg15_22_cc13::hud_counters_update_occlusion_00529810({c_.counters.data()}, b);
    }
private:
    HudCountersContext& c_;
};

// Goal 5 chain 667: 0052CF60's services on the counters block, for the score
// forms (kind 1) the Lua DisplayScores / HideScoreDisplay messages carry. A
// score entry is a zeroed 0ACh block the screen owns holding a 0052C640 clone
// of the HP entry (the page's own widgets, copied under New_Group's parent);
// 0052C0C0 deletes that clone's root (+A4h is 1 on a clone) and releases the
// entry's strings in the process pool. The 18h-record array (+70h) and the
// unit registry are the unit-HP form's (kind 0, 0052BB10), which no message
// here carries, so those services fail loudly.
class HudCountersScoreBinding final : public bsp::frontend_seg15_22_cc13::HudCountersScoreHost {
public:
    explicit HudCountersScoreBinding(HudCountersContext& c) : c_(c), show_(c) {}
    void* operator_new_00bf55be(std::size_t bytes) override { return c_.counters.allocate_owned(bytes); }
    void free_00bf6989(void* block) override { c_.counters.release_owned(block); }
    bool widget_is_visible_38(void* widget) override { return show_.widget_is_visible_38(widget); }
    void widget_set_visible_34(void* widget, bool visible) override {
        show_.widget_set_visible_34(widget, visible);
    }
    void counter_layout_countdown_0052a860(bsp::ingame_gui_cc13::Raw entry, std::string_view text,
        float seconds) override {
        show_.counter_layout_countdown_0052a860(entry, text, seconds);
    }
    void counters_relayout_00529a30(bsp::ingame_gui_cc13::Raw screen) override {
        show_.counters_relayout_00529a30(screen);
    }
    void object_delete_04(void* object) override {
        // 0052C0C0 on a score entry: the clone root, vtable +4h(1).
        GuiLayoutWidget& root = hud_counters_widget(object, "widget +4h(1)");
        std::vector<const GuiLayoutWidget*> walk{&root};
        while (!walk.empty()) {
            const GuiLayoutWidget* const w = walk.back();
            walk.pop_back();
            c_.visibility_recurses.erase(w);
            for (const auto& child : w->children) {
                if (child) walk.push_back(child.get());
            }
        }
        c_.frontend.destroy_runtime_widget(root);
    }
    std::uint32_t object_virtual_arg(bsp::ingame_gui_cc13::Raw, std::uint16_t, std::uint32_t) override {
        unbuilt("an 18h record's slot 0");
    }
    void observer_handle_copy_00523480(bsp::ingame_gui_cc13::Raw, bsp::ingame_gui_cc13::Raw) override {
        unbuilt("00523480");
    }
    void native_string_release(bsp::ingame_gui_cc13::Raw string) override {
        bsp::destroy_native_string_header_0041dd20(string.p, pool_.strings);
    }
    std::string native_string_value(bsp::ingame_gui_cc13::Raw string) override {
        return std::string(HudLeafStringPool::name(string.p));
    }
    bsp::ingame_gui_cc13::Raw global_block(std::uint32_t) override { unbuilt("the unit registry"); }
    bsp::ingame_gui_cc13::Raw counter_entry_clone_0052c640(bsp::ingame_gui_cc13::Raw source) override {
        if (source.p == nullptr)
            throw std::logic_error("0052C850 clones an HP entry 0052CBF0 did not build");
        void* const entry = c_.counters.allocate_owned(kCounterEntryBytes);  // zeroed
        HudCounterEntryCloneBinding b(c_.frontend, kCountersPage);
        bsp::cc13_sweepB_counter_entry_construct_clone_0052c640(entry, source.p, b);
        return {static_cast<std::uint8_t*>(entry)};
    }
    void counter_layout_timer_00529d70(bsp::ingame_gui_cc13::Raw entry, std::string_view a,
        std::string_view b) override {
        HudCounterScoreLayoutBinding binding(c_);
        bsp::cc13_sweepB4_hud_counter_layout_timer_00529d70(entry.p,
            hud_counter_string_argument(pool_, a), hud_counter_string_argument(pool_, b), binding);
    }
    void counter_layout_value_0052a100(bsp::ingame_gui_cc13::Raw entry, std::string_view a,
        std::string_view b, std::uint32_t v0, std::uint32_t v1) override {
        HudCounterScoreLayoutBinding binding(c_);
        bsp::cc13_sweepB4_hud_counter_layout_value_0052a100(entry.p,
            hud_counter_string_argument(pool_, a), hud_counter_string_argument(pool_, b),
            static_cast<std::int32_t>(v0), static_cast<std::int32_t>(v1), binding);
    }
    void counter_layout_score_0052bb10(bsp::ingame_gui_cc13::Raw, std::uint32_t, std::string_view) override {
        unbuilt("0052BB10");
    }
private:
    static constexpr std::size_t kCounterEntryBytes = 0xAC;  // as 0052CBF0's two entries
    static constexpr const char* kCountersPage = "GUI_counters";  // 0052CBF0's page
    [[noreturn]] static void unbuilt(const char* routine) {
        throw std::logic_error(std::string("0052CF60 reached ") + routine
            + ", the unit-HP form's, which no HUD message here carries");
    }
    HudCountersContext& c_;
    HudCountersShowBinding show_;
    HudLeafStringPool pool_;
};

// 00772330's services with ECX = [00E188A8]+1EF0h. The record addressed to the
// local slot goes to 0052CF60 on [[00E198C4]+C8h]; this single-player process
// has no peers (every record's +50h is null), so nothing is sent or broadcast.
class HudScoreSessionBinding final : public bsp::cc13cam2::SessionSendHost {
public:
    explicit HudScoreSessionBinding(HudCountersContext& c) : c_(c) {}
    void broadcast_except_00784790(const bsp::cc13cam2::SessionMessageOut&) override { unreached("00784790"); }
    void flush_peer_buffers_007848f0() override { unreached("007848F0"); }
    void send_to_peer_00770b50(std::uintptr_t, std::uintptr_t) override { unreached("00770B50"); }
    void kick_peer_00784b10(std::uintptr_t, int) override { unreached("00784B10"); }
    void apply_counter_record_0052cf60(std::uintptr_t message) override {
        HudCountersScoreBinding binding(c_);
        bsp::frontend_seg15_22_cc13::hud_counters_apply_counter_record_0052cf60({c_.counters.data()},
            {reinterpret_cast<std::uint8_t*>(message)}, binding);
    }
    void broadcast_message_00784830(std::uintptr_t) override { unreached("00784830"); }
    void scalar_delete(std::uintptr_t) override { unreached("the scalar delete"); }
private:
    [[noreturn]] static void unreached(const char* routine) {
        throw std::logic_error(std::string("00772330 reached ") + routine
            + ", which needs a peer this single-player session does not have");
    }
    HudCountersContext& c_;
};

// Only codec/profile bindings live here. Message ownership comes from the
// mission's retained participant pools and current local-slot cell below.
// 00E17669 is the writer's empty fallback; CED098 has five native slots.
struct HudScoreMessageImage {
    const char fallback_00e17669[1]{};
    bsp::NativeSessionMessage35Profile profile{game_native_string_process().raw_context(), fallback_00e17669};
};
HudScoreMessageImage& hud_score_message_image() {
    static HudScoreMessageImage image;
    return image;
}

// 0052B620 / 0052B780 / 0052B900's one service: the 23h message on the stack
// (0052B64B 0075B430(23h), 0052B652 +4h = 1, 0052B656 vtable 00CED098, the
// strings, the two value dwords and the word array zeroed, both flags 0; then
// the sender's fields, the strings through 0041DD40(length, 1) and memcpy),
// 00772330 on [00E188A8]+1EF0h (0052B711) and 0052AF50 (0052B71F).
class HudScoreMessageBinding final : public bsp::frontend_seg15_22_cc13::HudMessage35SendHost {
public:
    explicit HudScoreMessageBinding(HudCountersContext& c) : c_(c) {}
    void session_send_message35_00772330(const bsp::frontend_seg15_22_cc13::HudMessage35& m) override {
        if (c_.score_participants == nullptr || c_.score_local_slot == nullptr)
            throw std::logic_error("HUD score message requires the mission participant binding");
        HudScoreMessageImage& image = hud_score_message_image();
        bsp::NativeStringRawPoolContext& strings = *image.profile.strings;
        bsp::NativeSessionMessageParticipantOwner selected(*c_.score_participants, *c_.score_local_slot);
        // The selected-participant view reads the canonical owner directly;
        // 0075B430 never reads this raw-game interface cell in that arm.
        void* volatile no_raw_game = nullptr;
        const bsp::NativeSessionMessageContext session{no_raw_game, &selected};
        bsp::NativeSessionMessage35 message;
        // bsp.py show 0052AEF0: the constructor establishes the three string
        // headers, word array, flags and subtype on this same delivered object.
        // Values +18/+1C/+38/+3C are supplied by send(), not defaulted here.
        bsp::construct_native_session_message35_0052aef0(&message, session, image.profile);
        // The image's EH frame 00C6B938 runs 0052AF50 on unwind too.
        try {
            send(m, message, strings);
        } catch (...) {
            bsp::destroy_native_session_message35_0052af50(&message, strings, image.profile);
            throw;
        }
        bsp::destroy_native_session_message35_0052af50(&message, strings, image.profile);
    }
private:
    void send(const bsp::frontend_seg15_22_cc13::HudMessage35& m, bsp::NativeSessionMessage35& message,
        bsp::NativeStringRawPoolContext& strings) {
        message.value_18 = m.value_18;
        message.value_1c = m.value_1c;
        message.subtype_20 = m.subtype_20;
        message.flag_24 = m.hide_24;
        message.flag_25 = m.pair_25;
        auto assign = [&strings](std::uint32_t* header, std::string_view text) {
            bsp::resize_native_string_header_0041dd40(header, strings,
                static_cast<std::uint32_t>(text.size()), true);
            if (!text.empty()) {
                char* data = nullptr;
                std::memcpy(&data, reinterpret_cast<std::uint8_t*>(header) + 4, sizeof(data));
                std::memcpy(data, text.data(), text.size());
            }
        };
        assign(&message.length_28, m.text_28);
        assign(&message.length_30, m.text_30);
        message.values_38[0] = m.values_38[0];
        message.values_38[1] = m.values_38[1];
        if (m.unit_count_40 != 0 || !m.text_4c.empty())
            throw std::logic_error("the HUD score senders carry no unit-HP words or text");
        std::array<bsp::cc13cam2::SessionPlayerRecord, 8> records{};
        // 0077236A indexes game+18CCh by +1Ch, the 3-bit slot the 23h writer
        // packs (005294F0); a script owner outside 0..7 would read past it.
        if (message.value_1c >= records.size())
            throw std::logic_error("a HUD 23h message addressed beyond the eight player slots");
        HudScoreSessionBinding session(c_);
        // ECX = [00E188A8]+1EF0h: +F4h is game+1FE4h; the local slot game+18ECh.
        bsp::cc13cam2::session_send_to_player_slot_00772330(bsp::session_context().session_word_1fe4,
            static_cast<std::int32_t>(message.value_1c), static_cast<int>(*c_.score_local_slot), records,
            reinterpret_cast<std::uintptr_t>(&message), session);
    }
    HudCountersContext& c_;
};

// 0052BAA0's view of the block: +0Ch, +10h and the +14h array (count +18h).
bsp::HudCountersState hud_counters_state(const HudLeafScreenStorage& counters) {
    const bsp::ingame_gui_cc13::Raw s{counters.data()};
    bsp::HudCountersState state;
    state.counter_0c = reinterpret_cast<std::uintptr_t>(s.ptr(0x0C));
    state.counter_10 = reinterpret_cast<std::uintptr_t>(s.ptr(0x10));
    const std::int32_t count = s.at<std::int32_t>(0x18);
    auto* const* const slots = static_cast<void* const*>(s.ptr(0x14));
    for (std::int32_t i = 0; i < count; ++i)
        state.counters_14.push_back(reinterpret_cast<std::uintptr_t>(slots[i]));
    return state;
}

// The layout virtuals (+14h) those registers call through the screen's own
// vtable, ECX = screen. kHudScreenLayouts lists each slot's layout virtual;
// the vtable the leaf constructor stored must hold it at +14h (bsp.py ghidra
// bytes: 00CF6D68+14h = 0067BEA0, 00CF61D4+14h = 00650B90).
struct HudLeafLayoutVirtual {
    std::uint32_t layout_virtual;
    std::uint32_t vtable;
};
constexpr std::array<HudLeafLayoutVirtual, 3> kHudLeafLayoutVirtuals{{
    {0x0067BEA0u, bsp::ingame_gui_cc13::kFollowScreenVtable},
    {0x00650B90u, bsp::ingame_gui_cc13::kSubmarineScreenVtable},
    {0x005FA0F0u, 0x00CF3B7Cu},  // order 3Bh (005F9440 stores it; 00CF3B90 = 005FA0F0)
}};
// 00650B90's page and children: its 00AA5840 is the registry owner's page
// load for the slot; 00AA7E00(name, 1) searches the page's descendants and
// returns null for a name the page lacks.
class HudLeafPageChildBinding final : public bsp::ingame_gui_cc13::HudScreenPageChildHost {
public:
    explicit HudLeafPageChildBinding(HudLeafRegisterContext& context) : context_(context) {}
    void* gui_load_page_00aa5840(std::string_view name) override { return context_.load_page(name); }
    void* gui_find_child_00aa7e00(void* parent, std::string_view name) override {
        if (parent == nullptr) return nullptr;
        return bsp::find_descendant_by_name(*static_cast<GuiLayoutWidget*>(parent), name);
    }
private:
    HudLeafRegisterContext& context_;
};
// 005FA0F0's twenty name temporaries live in this process's actual string
// pool, as the register overrides' names do; its 00AA7E00 searches the
// GUI_order page 005F9FF0 stored at +8h.
class HudLeafOrderLayoutBinding final : public bsp::Cc13SweepB6OrderLayoutHost {
public:
    void native_string_resize_0041dd40(void* string, std::uint32_t length, std::uint8_t preserve) override {
        pool_.resize(string, length, preserve != 0);
    }
    void* sized_pool_get_00419cc0() override { return pool_.pool(); }
    void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t bytes, std::int32_t) override {
        pool_.give_back(pool, data, bytes);  // the flag argument is 1
    }
    void* widget_find_child_00aa7e00(void* widget, void* name, std::int32_t) override {
        if (widget == nullptr) return nullptr;
        return bsp::find_descendant_by_name(*static_cast<GuiLayoutWidget*>(widget),
            HudLeafStringPool::name(name));
    }
private:
    HudLeafStringPool pool_;
};
void run_hud_leaf_screen_layout(HudLeafRegisterContext& context) {
    std::uint8_t* const p = context.screen.data();
    const bsp::ingame_gui_cc13::Raw raw{p};
    const auto entry = std::find_if(kHudLeafLayoutVirtuals.begin(), kHudLeafLayoutVirtuals.end(),
        [&](const HudLeafLayoutVirtual& e) { return e.layout_virtual == context.layout_virtual; });
    if (entry == kHudLeafLayoutVirtuals.end() || raw.at<std::uint32_t>(0) != entry->vtable)
        throw std::logic_error("HUD leaf +14h virtual does not match the screen's vtable");
    switch (context.layout_virtual) {
    case 0x0067BEA0u: bsp::ingame_gui_cc13::hud_follow_screen49_bind_layout_0067bea0(raw); return;
    case 0x00650B90u: { HudLeafPageChildBinding b(context); bsp::ingame_gui_cc13::submarine_load_layout_00650b90(raw, b); return; }
    case 0x005FA0F0u: { HudLeafOrderLayoutBinding b; bsp::cc13_sweepB6_hud_order_screen_bind_layout_005fa0f0(p, b); return; }
    default: break;
    }
    throw std::logic_error("HUD leaf layout virtual listed but not bound");
}
// 0067BE80 / 00650170: 004F71D0 on the block, then its +14h virtual.
class HudLeafLayoutRegisterBinding final : public bsp::ingame_gui_cc13::HudScreenRegisterLayoutHost {
public:
    explicit HudLeafLayoutRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void front_end_screen_register_004f71d0(bsp::ingame_gui_cc13::Raw screen) override {
        context_.register_base(screen.p);
    }
    void screen_virtual_14(bsp::ingame_gui_cc13::Raw screen) override {
        if (screen.p != context_.screen.data())
            throw std::logic_error("+14h virtual called on a different HUD screen");
        run_hud_leaf_screen_layout(context_);
    }
private:
    HudLeafRegisterContext& context_;
};
// 005F9FF0: 004F71D0, GUI_order into +8h, then its +14h virtual.
class HudLeafPageLayoutRegisterBinding final : public bsp::cc13_fe1::ScreenRegisterLayoutHost {
public:
    explicit HudLeafPageLayoutRegisterBinding(HudLeafRegisterContext& context) : context_(context) {}
    void frontend_register_004f71d0(void* screen) override { context_.register_base(screen); }
    std::uint32_t load_page_00aa5840(std::string_view name) override {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(context_.load_page(name)));
    }
    void screen_bind_layout(void* screen) override {
        if (screen != context_.screen.data())
            throw std::logic_error("+14h virtual called on a different HUD screen");
        run_hud_leaf_screen_layout(context_);
    }
private:
    HudLeafRegisterContext& context_;
};

void run_hud_leaf_screen_register(HudLeafRegisterContext& context) {
    std::uint8_t* const p = context.screen.data();
    const bsp::ingame_gui_cc13::Raw raw{p};
    switch (context.register_virtual) {
    case 0x0067B3F0u: { HudLeafRegisterBinding b(context); bsp::ingame_gui_cc13::hud_common_overlay_screen_register_0067b3f0(raw, b); return; }
    case 0x005176A0u: { HudLeafRegisterBinding b(context); bsp::cc13_fe3::in_game_screen_23_register_005176a0(raw, b); return; }
    case 0x005D38A0u: { HudLeafRegisterBinding b(context); bsp::frontend_seg15_22_cc13::pause_multi_register_005d38a0(raw, b); return; }
    case 0x005EE010u: { HudLeafScreenRegisterBinding b(context); bsp::cc13_fe1::hud_objectives_register_005ee010(p, b); return; }
    case 0x00604890u: { HudLeafScreenRegisterBinding b(context); bsp::cc13_fe1::hud_pause_cheat_register_00604890(p, b); return; }
    case 0x0060D1C0u: { HudLeafPlaneSpawnRegisterBinding b(context); bsp::seg25_cc13::hud_plane_spawn_screen_register_0060d1c0(p, b); return; }
    case 0x00565EE0u: { HudLeafLimboRegisterBinding b(context); bsp::cc13_fe3::hud_limbo_register_00565ee0(raw, b); return; }
    case 0x0051ED60u: { HudLeafBinocularsRegisterBinding b(context); bsp::cc13_seg14::binoculars_screen_register_0051ed60(p, b, game_native_string_process().strings()); return; }
    case 0x0067BE80u: { HudLeafLayoutRegisterBinding b(context); bsp::ingame_gui_cc13::follow_screen_register_0067be80(raw, b); return; }
    case 0x00650170u: { HudLeafLayoutRegisterBinding b(context); bsp::ingame_gui_cc13::submarine_register_00650170(raw, b); return; }
    case 0x005F9FF0u: { HudLeafPageLayoutRegisterBinding b(context); bsp::cc13_fe1::hud_order_register_005f9ff0(p, b); return; }
    case 0x005CCE70u: { HudLeafEngineMovieRegisterBinding b(context); bsp::cc13_sweepB2_hud_engine_movie_screen_register_005cce70(p, b); return; }
    case 0x00643E80u: { HudLeafRegisterBinding b(context); bsp::ingame_gui_cc13::spectator_register_00643e80(raw, b); return; }
    case 0x0052CBF0u: { HudLeafCountersRegisterBinding b(context); bsp::frontend_seg15_22_cc13::hud_counters_register_0052cbf0(raw, b); return; }
    default: break;
    }
    throw std::logic_error("0068CC70 leaf register listed but not bound");
}

// Goal 5 chain 646: the leaf screens' own +4h (manages its own visibility),
// +18h (enter), +1Ch (exit) and +20h (update) virtuals on their blocks, as
// 004F7620 and the pump 004F8830 call them through the screen's vtable. Read
// from the vtables (bsp.py ghidra bytes <vtable> --length 36):
// - 00CF5998 (43h): +4h 00643DD0 (MOV AL,1), +18h 00643F40, +1Ch 00643DE0 (RET),
//   +20h 00643DF0 (RET 4). Its register 00643E80 stores +4h = 1, so the screen is
//   wanted from registration on and the set recompute never clears it.
// - 00CF7A38 (27h): +18h 0067B400 and +1Ch 0067B410, both RET.
// - 00CF61D4 (48h): +18h 00650190 (RET) and +1Ch the base 004F75B0 (RET).
// - 00CF6D68 (49h): +18h 0067BEB0 (RET).
// 0 leaves the slot's existing path: 27h's and 49h's +20h (0067BB50, 0067BF00)
// run from their own hosts; 48h's +20h 00650450 reads the controlled
// submarine's image fields (unit+1200h..+127Ch), which no host row carries;
// 49h's +1Ch 0067BEC0 clears +8h, which FollowScreen49State holds for 0067BF00,
// so running it on the block would split that field.
struct HudLeafScreenVirtuals {
    int slot;
    std::uint32_t vtable;
    std::uint32_t self_managed_04;
    std::uint32_t enter_18;
    std::uint32_t exit_1c;
    std::uint32_t update_20;
};
// - 00CED0AC (51h, goal 5 chain 662): +4h 0052B070 (MOV AL,1), +18h 00528C40 and
//   +1Ch 00528C50 (the page root +8h's +34h(1) / (0)), +20h 0052BAA0 (the
//   counter ticks and 00529810). bsp.py ghidra bytes 00ced0ac --length 40.
// - 00CEC194 (23h, the airfield-view screen, block 2Ch from 005178B0; goal 5 chain
//   671): +4h is the base 004F7570, +18h 00517820, +1Ch 00517860, +20h 005178A0
//   (C2 04 00, a bare RET 4). bsp.py ghidra bytes 00cec194 --length 40.
// - 00CF3B7C (3Bh, the order screen, block 90h from 005F9440; goal 5 chain 671):
//   +1Ch 005F9480 (XOR AL,AL; +81h = +82h = AL; RET), the two bytes this HUD's
//   InGameInterface update reads on the block (screen_3b_block_a/b). Its +18h
//   005FAD70 reads the controlled unit's image fields (+738h's vtable +50h,
//   +284h) and its +20h 005F9DD0 the manager's +5Ch screen and the page
//   colours, so both keep the existing path (0). bsp.py ghidra bytes 00cf3b7c
//   --length 40.
constexpr std::array<HudLeafScreenVirtuals, 8> kHudLeafScreenVirtuals{{
    {0x26, 0x00CEC9B0u, 0, 0, 0x0051E9B0u, 0},
    {0x43, bsp::ingame_gui_cc13::kSpectatorScreenVtable, 0x00643DD0u, 0x00643F40u, 0x00643DE0u,
        0x00643DF0u},
    {0x27, 0x00CF7A38u, 0, 0x0067B400u, 0x0067B410u, 0},
    {0x48, bsp::ingame_gui_cc13::kSubmarineScreenVtable, 0, 0x00650190u, 0x004F75B0u, 0},
    {0x49, bsp::ingame_gui_cc13::kFollowScreenVtable, 0, 0x0067BEB0u, 0, 0},
    {0x51, bsp::frontend_seg15_22_cc13::kHudCountersVtable, 0x0052B070u, 0x00528C40u, 0x00528C50u,
        0x0052BAA0u},
    {0x23, 0x00CEC194u, 0, 0x00517820u, 0x00517860u, 0x005178A0u},
    {0x3B, 0x00CF3B7Cu, 0, 0, 0x005F9480u, 0},
}};
const HudLeafScreenVirtuals* hud_leaf_screen_virtuals(int slot) noexcept {
    for (const HudLeafScreenVirtuals& v : kHudLeafScreenVirtuals) {
        if (v.slot == slot) return &v;
    }
    return nullptr;
}

// 00643F40's services: 00AA7E00(name, 1) searches the page's descendants, and
// vtable +34h is the menu's widget visibility setter. The image calls +34h with
// no null test; a page without Spectator_Text is reported, not dereferenced.
class HudLeafChildVisibilityBinding final
    : public bsp::ingame_gui_cc13::HudScreenChildVisibilityHost {
public:
    HudLeafChildVisibilityBinding(GameMenuHost& menu, GameHostLog& log) : menu_(menu), log_(log) {}
    void* gui_find_child_00aa7e00(void* parent, std::string_view name) override {
        if (parent == nullptr) return nullptr;
        return bsp::find_descendant_by_name(*static_cast<GuiLayoutWidget*>(parent), name);
    }
    void widget_set_visible_34(void* widget, bool visible) override {
        if (widget == nullptr) {
            log_.notef("HUD leaf enter: widget vtable +34h on a child the page does not hold");
            return;
        }
        menu_.frontend().set_widget_visible(*static_cast<GuiLayoutWidget*>(widget), visible);
    }
private:
    GameMenuHost& menu_;
    GameHostLog& log_;
};

// Packet cc9_hud_ray_pick (docs/HUD_PICK_SEGMENT_QUERY.md). True: 00526A40's
// camera segment query 009043A0 runs over the gunnery host's units
// (GameGunneryHost::query_segment_units) and the hit branch 00526C74..00526DB2
// picks a hit of kind 6, 0Fh, 45h, 46h, 1Bh or 35h by ray. False: the query
// reports no hit and the branch is a record.
constexpr bool kHudPickSegmentQueryBound = true;

// Packet cc13-wire1, granted by cc9 2026-10-18. 0064AE20 (src/cc13_big1.cpp,
// __fastcall(ECX = screen 45h), RET) is what 0064D590 calls at 0064D5F9 when
// screen+5 is set, after it stores the unit at +184h. The image: Lua
// VehicleClass[class+70h].NoRepairGUI hides +40h (the GUI_ship_damage page)
// when it is boolean 1; +44h = clamp((unit+980h + 0.5) / 1.5, 0, 1); the three gauges'
// +18h bytes cleared; +128h = 0064AAD0(), +12Ch = 0, 0064A9F0(0); flags
// +105h (class+C8h > 0), +107h (00818300 != 0), +106h (!+105h and 0059CB90
// == 16h), +104h (submarine kind 8 with 00852350 != 0); widget vt34 / vt88
// on +54h..+74h; then 0064A960. The current host (OFF): the gauges switch
// records 0064AE20 and the flags keep the enter's clear values. ON runs the
// body through ShipScreenApplyUnitAdapter below, whose stand-ins are: the Lua
// lookups (no NoRepairGUI key is read, so +40h shows), class+C8h = 0,
// 00818300 / 00852350 / 0059CB90 = 0, and the widgets at +40h, +54h, +58h and
// +68h..+74h, which 0064C0F0's host name map does not bind (no-ops).
// ON with both sub-switches (cc9-lua69, 2026-10-18): USN01 all-OFF vs all-ON pair,
// gameplay identical (presentation only; SQUADRON_LAND_TASK 5ir).
inline constexpr bool kHudShipScreenApplyUnitBound = true;

// Packet cc13-wire2, granted by cc9 2026-10-18: two OFF sub-switches that close
// stand-ins of the adapter above. Neither changes anything while
// kHudShipScreenApplyUnitBound is OFF, and each is OFF on its own.
// - kHudShipScreenApplyUnitLuaBound: the class reads 0064AE20 makes through
//   GameMissionLuaHost. VehicleClass[class+70h].NoRepairGUI (0064AED4, key
//   00CF5C88) through read_vehicle_class_boolean(type_id, .., false), and
//   class+C8h, which 00960503 stores from Catapult.LaunchStock (IntegerOr 0,
//   vehicle_class_fields.hpp kLaunchStock), through read_vehicle_class_integer.
//   OFF: NoRepairGUI reads as not a boolean (+40h shown) and class+C8h = 0.
// - kHudShipScreenApplyUnitWidgetsBound: 0064C0F0's stores for the offsets
//   0064AE20 drives, read from the bytes: +40h is the GUI_ship_damage PAGE
//   (00AA5840 at 0064CD06, name 00CF5F4C, stored at 0064CD0B), bound to the
//   page root as game_hosts_mission.cpp ~1763 does; +54h ship_recon_Icon
//   (00CF6044, 0064C5D8), +58h ship_torpedo_Icon (00CF6030, 0064C648), +68h
//   ship_recon_num1_Icon (00CF60B0, 0064C3A5), +6Ch ship_recon_num2_Icon
//   (00CF6098, 0064C415), +70h ship_torpedo_num1_Icon (00CF6080, 0064C485),
//   +74h ship_torpedo_num2_Icon (00CF6068, 0064C4F5). OFF: they stay unbound.
// LABELLED (both ON): the Lua reads key VehicleClass by class+70h taken as the row's
// type_id, which is inferred, not traced.
inline constexpr bool kHudShipScreenApplyUnitLuaBound = true;
inline constexpr bool kHudShipScreenApplyUnitWidgetsBound = true;

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// The ocean the camera's probes sample: 0078CF20 over the same flat wave field
// the unit host runs (amplitude +24h 0.0f, no coverage regions;
// src/game_hosts_units.cpp OceanFieldBinding).
class CameraOceanField final : public bsp::OceanHeightHost {
public:
    float wave_height_0078c890(float x, float z) override {
        if (bsp::ocean_live_field_ready()) {  // packet cc9_physics1_sea
            const bsp::OceanLiveField& live = bsp::ocean_live_field_process();
            return bsp::ocean_wave_field_sample_0078c890(live.field,
                bsp::ocean_spectrum_grid(live.spectrum), x, z);
        }
        bsp::OceanWaveFieldState field;
        field.flat_f9 = false;
        field.inv_tile_b4 = static_cast<float>(1.0 / 100.0);
        field.amplitude_24 = 0.0f;
        const bsp::OceanWaveGridView zero_grid{};
        return bsp::ocean_wave_field_sample_0078c890(field, zero_grid, x, z);
    }
    float coverage_mask_00b9cf50(float x, float z) override {
        return bsp::ocean_live_field_coverage_mask(x, z);  // packet cc9_physics1_sea
    }
};

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameHudHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in) : log(log_in), menu(menu_in) {}
    // Before leaf_screens frees the blocks: screen 33h's retained samples go back
    // as 005B9D40 returns them (release_hud_narrative_leaf_samples), screen 50h's
    // four as 00682030 releases its +30h slots (release_hud_warning_leaf_samples),
    // and screen 29h's two as 00524BA0 does (release_hud_unit_pick_leaf_samples).
    ~Impl() {
        for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
            if (!leaf_screens[i]) continue;
            const std::uint32_t constructor = kInGameHudScreens[i].constructor;
            try {
                if (constructor == 0x005BA7B0u) {
                    release_hud_narrative_leaf_samples(*leaf_screens[i]);
                    log.implemented("HudNarrative::release_samples", "005b9e11");
                } else if (constructor == 0x006821C0u) {
                    release_hud_warning_leaf_samples(*leaf_screens[i]);
                    log.implemented("HudWarning::release_samples", "006820c2");
                } else if (constructor == 0x00525A90u) {
                    release_hud_unit_pick_leaf_samples(*leaf_screens[i]);
                    log.implemented("HudUnitPick::release_samples", "00524c2a");
                }
            } catch (const std::exception& error) {
                log.notef("HUD screen %02X samples not released: %s",
                    static_cast<unsigned>(kInGameHudScreens[i].registry_slot), error.what());
            }
        }
    }

    GameHostLog& log;
    GameMenuHost& menu;
    // Packet cc9_mission_camera: the ShipCaptain mover 0064DA40 creates for the
    // 25h arm's ship view, and what it reads off its unit.
    GameMissionLuaHost* lua{nullptr};
    bool camera_bound{false};
    std::size_t camera_unit{0};
    std::uint64_t camera_last_frame{~0ull};
    bsp::ShipCaptainCamera camera{};
    bsp::ShipCameraSettings camera_settings{};
    bsp::ShipClassCameraFields camera_class{};
    float camera_class_length{0.0f};
    // GlobalConfig+F8h, loaded independently of screen 46h's ship-view binding.
    bool camera_fov_read{false};
    double camera_fov_ship_degrees{0.0};
    const bool* cinematic_hidden_634{nullptr};
    const bsp::SessionParticipantPools* score_participants{nullptr};
    const std::size_t* score_local_slot{nullptr};
    GameMissionResultHost* map_usage_scoring{nullptr};
    const std::size_t* map_usage_local_slot{nullptr};
    // Screen 45h's pipe-sight FOV block (docs/MISSION_CAMERA.md section 9).
    bool camera_pipe_sight_read{false};
    bool camera_pipe_sight_enabled{false};
    float camera_pipe_sight_zoom_rate{0.0f};
    bsp::MissionCameraProjection camera_projection{};
    // Goal 5 chain 671: game+1A04h, the GlobalConfig FOV index the last 004DC940
    // stored (004DC947); -1 before the first call.
    std::int32_t view_fov_index_1a04{-1};
    // 004DC940 BSP_Game_SetViewFov(scale, index) for the callers the HUD runs (the
    // leaf screen virtuals below), over this camera's projection.
    void set_view_fov(float scale, std::int32_t index);
    void publish_ship_view_camera(const bsp::CameraMatrix16& world);
    // Packet cc9_ship_screen_update: screen 45h's fields and its widgets.
    bsp::ShipScreenState ship_screen{};
    std::size_t ship_screen_unit{0};
    bool ship_screen_widgets_bound{false};
    std::map<int, GuiLayoutWidget*> ship_screen_widgets;
    GuiLayoutWidget* ship_screen_widget(bsp::ShipScreenWidget widget);
    // 0064AE20 through the Big1Host adapter (kHudShipScreenApplyUnitBound).
    void apply_ship_screen_unit_0064ae20();
    void bind_ship_screen_unit_0064d590(std::size_t unit);
    // Packet cc9_screen_50h: screen 50h's fields and its widgets.
    bsp::WarningScreenState warning_screen{};
    bool warning_widgets_bound{false};
    std::map<int, GuiLayoutWidget*> warning_widgets;
    GuiLayoutWidget* warning_widget(bsp::WarningWidget widget);
    // Packet cc9_screen_49h.
    bsp::FollowScreen49State follow_screen{};
    // Packet cc13-fix24: the hints screen at HUD manager +A8h (vtable 00CEE290,
    // 7Ch bytes), read only by 0054D3C0 at +54h (seconds left). Register
    // 0054D900 stores +54h = 0.0f; ShowHint, Update and Dismiss have no host
    // path, so the host's copy keeps that value. No other field is modelled.
    alignas(4) std::uint8_t hints_screen[0x7C]{};
    // Packet cc9_screen_46h: screen 46h's +1Ch, stored by 0064DA40.
    bool ship_view_has_unit{false};
    std::size_t ship_view_unit{SIZE_MAX};
    // Packet cc13-releasegun (kHarnessReleaseGunMapBound): armed by the plan's
    // first `releasegun` line; map_up is screen 4Ch's +4h ([this+54h]+4h)
    // once 0068AC30 has published the level-2 set.
    bool release_gun_armed{false};
    bool release_gun_map_up{false};
    void back_out_one_overlay_level_0068b470();
    // Goal 2 chain 35 (kWireGoal2Chain35Bound): 0068AB80 over the menu host's
    // screen-set state, for the two collapse sites' ON arms.
    void collapse_in_game_overlays(bool clear_level2, bool run_extra_hook, const char* site);
    void log_map_state(const char* state);
    bsp::IntegratedControlsState ship_view_controls{};
    bsp::BinocularsState binoculars{};
    void reset_binoculars_0051e8e0();
    float binoculars_saved_near_plane{0.0f}; // process cell 00E18DA8, initially 00D7A218
    // Screen 44h, the HUD root (00649860). The enter 006488D0 zeroes +F4h;
    // the default state is that store, and the screen enters once per run.
    bsp::HudRootUpdateState hud_root{};
    // Chain 413: retained native-layout fields of this process's root screen.
    // 0068BF60 clears +E4h; 0068CC70 -> 00644220(0) seeds +1Ch. 00646040
    // produces +CCh/+E4h from actual applied requests. No synthetic unit is
    // stored here: +E4h is GameUnitsHost's stable canonical unit identity.
    alignas(4) std::array<std::uint8_t, 0xE8> root_interface_fields{};
    HudRootConstructionStorage constructed_root_screen{};
    HudSceneInitStorage constructed_scene_init_screen{};
    // 0068CC70's blocks for the leaf screens kHudLeafScreenConstructors lists,
    // by kInGameHudScreens index; empty for every other screen.
    std::array<std::unique_ptr<HudLeafScreenStorage>, kInGameHudScreenCount> leaf_screens{};
    // Widget +75h (00AA6820) of the counters' widgets, while it is set.
    std::set<const GuiLayoutWidget*> counters_visibility_recurses;
    void* root_saved_observer_identity{nullptr};
    bsp::HudRootPayloadKind root_saved_payload_kind{bsp::HudRootPayloadKind::Unit};
    bsp::ingame_gui_cc13::Raw root_interface_view() {
        return {constructed_root_screen.data()};
    }
    void set_root_closed_value(std::int32_t value) {
        root_interface_view().at<std::int32_t>(0x1C) = value;
        hud_root.closed_hud_flag = value != 0;
    }
    std::size_t root_identity_index(const void* identity) const {
        if (identity != nullptr && units != nullptr) {
            for (std::size_t i = 0; i < units->count(); ++i)
                if (units->unit_identity(i) == identity) return i;
        }
        return SIZE_MAX;
    }
    std::size_t first_squadron_member(std::size_t unit) const {
        for (const bsp::PlaneSquadronHostRecord& squadron : bsp::plane_squadron_registry().records())
            if (squadron.squadron_unit == unit) return squadron.flight_leader();
        return SIZE_MAX;
    }
    void save_root_interface(int id, bsp::HudRootPayload payload);
    bool release_destroyed_payload(bsp::HudRootPayload& payload) const;
    void refresh_root_saved_payload();
    bool hud_root_widgets_bound{false};
    GuiLayoutWidget* hud_root_closed_group{nullptr};
    GuiLayoutWidget* hud_root_closed_group_widget();
    // Goal 4 chain 291: 00648C20's root fields, +6Ch (cached unit) and +88h
    // in root_rows, +70h (the cached weapon-info source) in
    // root_weapon_source_70, and the row widgets 006463E0 binds (+34h..+84h
    // on GUI_unit, +3Ch on GUI_selector), resolved once by name.
    bsp::HudRootRowState root_rows{};
    std::string root_weapon_source_70;
    bool hud_root_row_widgets_bound{false};
    std::map<bsp::HudRowWidget, GuiLayoutWidget*> hud_root_row_widgets;
    std::map<bsp::HudRowWidget, bool> hud_root_row_shown;
    std::string root_weapon_text;
    bool root_weapon_text_applied{false};
    GuiLayoutWidget* hud_root_row_widget(bsp::HudRowWidget widget);
    unsigned long long root_row_updates{0};
    // Screen 29h, the unit pick (00527260, 00526A40).
    bsp::UnitPickScreenState unit_pick{};
    // Screen 2Eh, the weapon groups (00549260, 005484F0).
    bsp::HudWeaponGroupScreenState weapon_group{};
    // Packet cc9_player_gun_seat: the message 00954A10 last built.
    GunAimMessage79 pending_gun_message{};
    bool pending_gun_message_valid{false};
    // cc15-smA (kGunAimMessageCc15Bound): the 79h session copy, and what the
    // copy changed against the angles the OFF delivery derives from forward.
    bsp::HostSessionMessages session_messages;
    unsigned long long gun_aim_round_trips{0};
    unsigned long long gun_aim_changed[6]{};  // yaw, pitch, camera, bools, group, target
    double gun_aim_max_yaw_delta{0.0};
    double gun_aim_max_pitch_delta{0.0};
    bool lock_radius_read{false};
    std::vector<float> lock_radius;
    const std::vector<float>& lock_radius_multipliers();
    void bind_mission_camera_0064da40();
    void step_mission_camera(float seconds);
    bool camera_target_view(bsp::ShipCaptainTargetView& out);
    GameHudSummary summary;
    InGameInterfaceManager manager{};
    InMissionInterfaceUpdateState update_state{};
    // 004bca50's answer during the mission. The load resolves a single-player
    // campaign mission to 8, the same value the scene contents pass uses.
    int effective_game_mode{8};
    // Milestone 2j: 004c9ca0's loading element at [00e198c4]+D4h. The 1 arm
    // allocates 34h bytes at 004c9cd7 and constructs with 00636d90, which has no
    // reconstruction, so what this process holds is the object's identity and
    // the two bytes the routine itself reads and writes: +4h at 004c9d1f /
    // 004c9d82 and +5h at 004c9d72 / 004c9d85.
    bool loading_element_present{false};
    bool loading_element_4{false};
    bool loading_element_5{false};
    bool in_game_interface_applied{false};

    // Milestone 2k. The two screens whose update virtual is reconstructed, and
    // the controlled-unit payload the second interface request carries.
    std::unique_ptr<GameHudMinimapHost> minimap;
    std::unique_ptr<GameHudMarkersHost> markers;
    GameUnitsHost* units{nullptr};
    // The helm `aim unit` track's diagnostics (packet cc13-aimunit).
    unsigned long long aim_track_frames{0};
    bool aim_track_warned{false};
    unsigned aim_track_serial{0};
    bool unit_request_pending{false};
    bool unit_request_applied{false};
    int unit_interface_id{0};
    // Packet cc13-mapback: the manager's pending record as 004CC460 last wrote
    // it, +20h (the id) and +38h (the payload, a unit index; SIZE_MAX for
    // null). 006840F0 services the record without clearing it, so 0068AA40's
    // "already the pending payload" test reads the last push.
    // SUBSTITUTION, labelled (rev2 N1). The tracker assumes that every push
    // noted here is accepted and that the noted sites are the only writers.
    // Neither holds in general:
    //  - 004CC460 refuses when [00E19894] is set and 00683E90(id) is true
    //    (004CC478..004CC491), leaving +20h/+38h as they were.
    //  - +38h is a linked weak reference (unlink 006952A0 / link 00694A60,
    //    004CC4CC..004CC4E2), so a unit's destruction can clear it while this
    //    index stays stale. That is harmless here: a dead unit fails +5Ch.
    //  - Pushes the host only records are not noted, e.g. the 34h limbo
    //    pushes at 0064734B, 006485A0's empty-list arm and cc13-limbo's
    //    009595B8.
    int record_pending_id_20{0};
    std::size_t record_pending_payload_38{static_cast<std::size_t>(-1)};
    void note_pending_record_004cc460(int id, bool controlled_payload) {
        pending_explicit_payload = false;
        record_pending_id_20 = id;
        record_pending_payload_38 = controlled_payload && units != nullptr
            && units->controlled_bound() ? units->controlled_index()
            : static_cast<std::size_t>(-1);
    }
    // Packet cc9_set_selected_unit: 00647300's calls, acceptances, and the
    // 20h pushes it made (serviced through the same pending/applied pair).
    unsigned long long select_calls{0};
    // Packet cc9_controlled_unit_observer: the unit the root observer is on.
    std::size_t observed_unit{SIZE_MAX};
    unsigned long long observer_notices{0};
    unsigned long long observer_releases{0};
    // Packet cc9_squadron_observer_liveness: slot deaths the squadron outlived.
    unsigned long long observer_squadron_holds{0};
    bool squadron_hold_logged{false};
    void run_root_observer_notice();
    // Packet cc9_force_select_unit: the HUD root's unit vectors and cursor.
    bsp::HudRootUnitLists root_lists{};
    bsp::HudRootSelectionState root_selection{};
    unsigned long long root_rebuilds{0};
    unsigned long long force_calls{0};
    unsigned long long force_selects{0};
    void run_root_lists(bool force);
    void reset_unit_pick_selection(const char* site);
    unsigned long long select_accepted{0};
    unsigned long long select_pushes{0};
    bool pick_first_basis_logged{false};
    // Packet cc9_movie_interface_and_reseed: the movie screen 37h's +1Ch/+20h,
    // the 00E19894 lock as 005CD1A0 sees it (never raised in single player),
    // and a pending non-20h request (004CC460 writes the one pending record,
    // so a later push replaces an earlier one).
    bsp::HudMovieScreenState movie_screen{};
    bsp::FrontEndInterfaceLock movie_lock{};
    int pending_interface_id{0};
    // The 00647080 push carries a member, which can differ from the controlled
    // squadron. Keep that exact UnitRef until the existing service consumes it.
    bool pending_explicit_payload{false};
    bsp::HudRootPayload pending_payload{};
    unsigned long long movie_calls{0};
    unsigned long long movie_engages{0};
    unsigned long long movie_pushes{0};
    unsigned long long movie_reseeds{0};
    // Packet cc9_movie_camera_mover_bind: the movie camera (screen 37h's +1Ch)
    // while it is the installed mover.
    bsp::HudMovieCamera movie{};
    bool movie_active{false};
    unsigned long long movie_builds{0};
    unsigned long long movie_destroys{0};
    unsigned long long movie_keyframes{0};
    unsigned long long movie_publishes{0};
    unsigned long long movie_unsupported{0};
    unsigned long long movie_events_fired{0};   // cc9_lua49_movie_modifier
    unsigned long long movie_finish_due{0}, movie_finish_ran{0};   // cc13-finscr
    unsigned long long movie_step_draws{0};   // switch 4, 00798D07
    unsigned long long movie_last_pose_log{0};
    int pick_basis_source{0};   // 0 none, 1 ShipCaptain, 2 movie camera
    bool build_movie_camera_005cc170();
    void destroy_movie_camera(const char* why);
    void step_movie_camera(float seconds);
    void reseed_stream(int stream, std::uint32_t seed, std::uint32_t site);
    bsp::UnitSelectableInputs selectable_inputs_00645060(std::size_t unit);
    // Packet cc9_vtable124_liveness: unit->vtable[124h]() by class.
    bool unit_vtable_124(std::size_t unit, std::uint32_t site);
    // Packet cc9_squadron_slot_class: the class the selection tests see, and
    // their IsKindOf over it. A squadron's fused slot answers 18h when bound.
    int selection_class_id(std::size_t unit) const;
    bool selection_is_kind_of(std::size_t unit, int class_id) const;
    bool is_squadron_slot(std::size_t unit) const;
    std::vector<std::size_t> squadron_slots_seen;   // census, each once
    // The units the class map answers true for and the ship-only rule does not,
    // each counted once, with the site that first asked.
    std::vector<std::pair<std::size_t, std::uint32_t>> vtable124_map_only;
    unsigned long long vtable124_asks{0};
    unsigned long long vtable124_map_only_asks{0};

    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }
};

namespace {
// Host-side weak pair, using the same destroyed-unit delivery boundary as the
// existing root observer below. The native 00694A60/006952A0 graph is still
// recorded; this pair owns the semantic saved-reference release independently.
class RootSavedInterfaceObserver final : public bsp::ingame_gui_cc13::HudObserverReferenceHost {
public:
    explicit RootSavedInterfaceObserver(GameHudHost::Impl& owner) : owner_(owner) {}
    void observer_register_00694a60(void*, void* observed) override {
        owner_.root_saved_observer_identity = observed;
        owner_.record("HudRootInterface::register_saved_reference", 0x00694a60u);
    }
    void observer_unregister_006952a0(void*, void*) override {
        owner_.root_saved_observer_identity = nullptr;
        owner_.record("HudRootInterface::unregister_saved_reference", 0x006952a0u);
    }
private:
    GameHudHost::Impl& owner_;
};
}

void GameHudHost::Impl::reset_unit_pick_selection(const char* site) {
    // 00647059 (bsp.py show 00647040 --asm) loads [00E198C4]+CCh. Use
    // 0068CC70's retained allocation, including its actual +30h mode; no mode
    // or iterator value is synthesized at this call. The reset itself writes
    // +C0h = 0, +D4h = -1 and +30h = 0 (005251CF/00525203/00525240).
    const std::size_t index = bsp::in_game_hud_screen_index_at(0xCC);
    HudLeafScreenStorage* const screen = index < kInGameHudScreenCount
        ? leaf_screens[index].get() : nullptr;
    if (screen == nullptr) {
        record(site, 0x005251c0u); // HUD manager has not built screen 29h yet.
        return;
    }
    HudLeafUnitPickResetBinding binding(*screen, menu);
    bsp::cc13_sweepA::hud_screen_reset_selection_005251c0({screen->data()}, binding);
    done(site, 0x005251c0u);
}

void GameHudHost::Impl::save_root_interface(int id, bsp::HudRootPayload payload) {
    RootSavedInterfaceObserver observer(*this);
    void* identity = units != nullptr && payload.unit < units->count()
        ? const_cast<void*>(units->unit_identity(payload.unit)) : nullptr;
    const bool saves_payload = root_interface_view().at<std::int32_t>(0x1C) == 0;
    // bsp.py show 00646040: +1Ch suppresses both stores while temporarily
    // closed. The existing body owns the +CCh/+E4h update and pair replacement.
    bsp::ingame_gui_cc13::hud_root_set_interface_00646040(
        root_interface_view(), static_cast<std::uint32_t>(id), identity, observer);
    // The fused identity can be unchanged when the 20h squadron request
    // delegates to its first plane. Update meaning even without a pair relink,
    // but freeze it alongside +CCh/+E4h while the root is closed.
    if (saves_payload) root_saved_payload_kind = payload.kind;
    done("InGameInterface::hud_root_set_interface", 0x00646040u);
}

bool GameHudHost::Impl::release_destroyed_payload(bsp::HudRootPayload& payload) const {
    if (units == nullptr || payload.unit == SIZE_MAX) return false;
    bool destroyed = false;
    for (const auto& death : units->destroyed_units()) {
        if (death.first == payload.unit) { destroyed = true; break; }
    }
    bool survivors = false;
    // 0068ACA0 delegates a squadron's 20h request through +3D0h; 22h/23h
    // carry a PLANE. Only an actual squadron reference may outlive that plane.
    if constexpr (kSquadronObserverLivenessBound) {
        if (payload.kind == bsp::HudRootPayloadKind::Squadron) {
            for (const bsp::PlaneSquadronHostRecord& squadron : bsp::plane_squadron_registry().records()) {
                if (squadron.squadron_unit == payload.unit) {
                    survivors = squadron.live_count() > 0;
                    break;
                }
            }
        }
    }
    return payload.release_if_destroyed(destroyed, survivors);
}

void GameHudHost::Impl::refresh_root_saved_payload() {
    bsp::HudRootPayload payload{root_identity_index(root_saved_observer_identity),
        root_saved_payload_kind};
    if (!release_destroyed_payload(payload)) return;
    // observer_ref_released's +14h store on the member at root+D0h. Native
    // callback dispatch remains unbound; the actual death list delivers it at
    // the HUD update boundary, before any saved payload can be restored.
    void* old = root_interface_view().ptr(0xE4);
    root_interface_view().ptr(0xE4) = nullptr;
    RootSavedInterfaceObserver observer(*this);
    observer.observer_unregister_006952a0(root_interface_view().p + 0xD0, old);
    root_saved_payload_kind = bsp::HudRootPayloadKind::Unit;
}

// ---------------------------------------------------------------------------
// 0068aca0's host
// ---------------------------------------------------------------------------

namespace {

// The 20h arm's unit probe, 0068ae0b. Milestone 2h reached it only with a null
// payload, so none of these was called and all three recorded. Milestone 2k
// makes the second request carry the controlled unit, and the probe is then the
// recovered class test over the recovered chain: a ship answers IsKindOf(6), so
// the arm re-enters its own virtual +10h with 25h INTF_CAPTAIN.
class HudUnitQuery final : public InGameInterfaceUnitQuery {
public:
    explicit HudUnitQuery(GameHudHost::Impl& owner) : owner_(owner) {}
    void use_payload(bsp::HudRootPayload payload) { explicit_payload_ = true; payload_ = payload; }
    bsp::HudRootPayload payload() const {
        if (explicit_payload_) return payload_;
        const std::size_t unit = owner_.units != nullptr && owner_.units->controlled_bound()
            ? owner_.units->controlled_index() : SIZE_MAX;
        return {unit, unit != SIZE_MAX && owner_.is_squadron_slot(unit)
            ? bsp::HudRootPayloadKind::Squadron : bsp::HudRootPayloadKind::Unit};
    }
    bool unit_is_kind_of(int type_code) override {
        if (owner_.units != nullptr && owner_.units->controlled_bound()) {
            owner_.done("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
            return owner_.units->unit_is_kind_of(owner_.units->controlled_index(),
                type_code);
        }
        owner_.record("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
        return false;
    }
    bool plane_is_in_flight() override {
        // 0068AE84 007BB9A0(unit) (packet cc9_plane_in_flight_test).
        bsp::PilotCmdLocalInputGate gate{};
        const bool plane = owner_.units != nullptr && owner_.units->controlled_bound()
            && owner_.units->plane_local_input_gate_007bb9a0(
                owner_.units->controlled_index(), gate);
        const bool answer = plane && bsp::pilot_cmd_local_input_enabled_007bb9a0(gate);
        owner_.log.notef("InGameInterface 007BB9A0 for the 20h payload: plane=%d c0c=%d aa0=%.1f "
            "chA=%d/%.2f 00604a20=%d 5d=%d -> %d (bound=%d)", plane ? 1 : 0,
            gate.enable_byte_c0c ? 1 : 0, static_cast<double>(gate.gate_float_aa0),
            gate.gate_block_present ? 1 : 0, static_cast<double>(gate.gate_block_value),
            gate.global_block_00604a20 ? 1 : 0, gate.unit_suppressed ? 1 : 0, answer ? 1 : 0,
            kPlaneInFlightTestBound ? 1 : 0);
        if constexpr (kPlaneInFlightTestBound) {
            owner_.done("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
            return answer;
        } else {
            owner_.record("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
            return false;
        }
    }
    bool has_delegate_unit() override {
        owner_.record("InGameInterface::has_delegate_unit", 0x0068af18u);
        return false;
    }

private:
    GameHudHost::Impl& owner_;
    bool explicit_payload_{false};
    bsp::HudRootPayload payload_{};
};

class HudInterfaceBinding final : public InGameInterfaceHost {
public:
    HudInterfaceBinding(GameHudHost::Impl& owner, HudUnitQuery& query)
        : owner_(owner), query_(query) {}

    bool apply_base_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 00684600. The lock at 00e19894 is the front-end owner's and is clear
        // in this process, so the base accepts the request; the record keeps the
        // base body itself out of the concrete count.
        owner_.record("InGameInterface::apply_base_request", 0x00684600u);
        owner_.summary.applied_interface_id = interface_id;
        return true;
    }
    bool payload_unit_is_dead() override { return false; }
    void limbo_screen_take_unit() override {
        owner_.record("InGameInterface::limbo_screen_take_unit", 0x00565fb0u);
    }
    void hud_root_screen_set_interface(int interface_id, bool has_payload) override {
        bsp::HudRootPayload payload = has_payload ? query_.payload() : bsp::HudRootPayload{};
        if (has_payload && (interface_id == kInterfacePlane
            || interface_id == kInterfacePlaneBomber || interface_id == kInterfacePlaneSpawn)) {
            // Native 0068AF18 delegates through +3D0h before these plane
            // interfaces. Keep a previously explicit plane exact, including
            // one whose index happens to equal the fused squadron slot.
            payload = payload.plane_member(owner_.first_squadron_member(payload.unit));
            query_.use_payload(payload);
        }
        owner_.save_root_interface(interface_id, payload);
    }
    bool mission_overlay_present() override { return false; }
    void tick_mission_overlay() override {
        owner_.record("InGameInterface::tick_mission_overlay", 0x0042a930u);
    }
    void set_level1_screen_set(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_screen_set_004f8530(ids, count);
        owner_.done("InGameInterface::set_level1_screen_set", 0x004f8530u);
        owner_.summary.level1_screens = 0;
        owner_.summary.level1_screen_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;  // the varargs terminator
            owner_.summary.level1_screen_ids.push_back(ids[i]);
            ++owner_.summary.level1_screens;
        }
    }
    void set_level1_input_contexts(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_input_contexts_004d8a50(ids, count);
        owner_.done("InGameInterface::set_level1_input_contexts", 0x004d8a50u);
        owner_.summary.level1_contexts = 0;
        owner_.summary.level1_context_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;
            owner_.summary.level1_context_ids.push_back(ids[i]);
            ++owner_.summary.level1_contexts;
        }
    }
    void screen_receive_unit(std::uint16_t manager_offset) override {
        owner_.record("InGameInterface::screen_receive_unit", 0x0068ad07u);
        // The 25h arm's hand-off on interface+7Ch is 0064DA40, the ship view
        // that creates the ShipCaptain camera mover (docs/MISSION_CAMERA.md).
        if (kMissionCameraBound && manager_offset == 0x7C) owner_.bind_mission_camera_0064da40();
        // 0064DA40 stores the unit at screen 46h's +1Ch.
        if (manager_offset == 0x7C && owner_.units != nullptr && owner_.units->controlled_bound()) {
            owner_.ship_view_has_unit = true;
            owner_.ship_view_unit = owner_.units->controlled_index();
        }
        // The 25h arm's hand-off on interface+78h is 0064D590, which stores
        // the unit in screen 45h's +184h (0064D598..0064D5EC).
        if (manager_offset == 0x78 && owner_.units != nullptr && owner_.units->controlled_bound()) {
            const std::size_t unit = owner_.units->controlled_index();
            owner_.bind_ship_screen_unit_0064d590(unit);
        }
    }
    int hud_root_screen_query() override {
        // bsp.py show 00644230: MOV EAX,[00E188D8]; RET, ECX unused.
        // 0068ACA0's two bomb-view queries now read the live publication
        // maintained by 004C0890, rather than returning the old zero.
        if (owner_.units == nullptr) {
            owner_.record("InGameInterface::hud_root_screen_query_missing_units", 0x00644230u);
            return 0;
        }
        void* const unit = oct18_tail_a::hud_controlled_unit_00644230(
            owner_.units->controlled_unit_publication());
        owner_.done("InGameInterface::hud_root_screen_query", 0x00644230u);
        static_assert(sizeof(int) == sizeof(void*));
        // The existing interface transports Win32 identity words as int.
        return static_cast<int>(reinterpret_cast<std::uintptr_t>(unit));
    }
    void bomb_view_screen_bind(int value) override { static_cast<void>(value); }
    void bomb_view_screen_store(int value) override { static_cast<void>(value); }
    std::uint32_t unit_ambient_sound_source() override { return 0; }
    void stop_ambient_sound() override {
        owner_.record("InGameInterface::stop_ambient_sound", 0x0054d510u);
    }
    std::uint32_t start_ambient_sound(std::uint32_t source) override {
        static_cast<void>(source);
        owner_.record("InGameInterface::start_ambient_sound", 0x00a7acf0u);
        return 0;
    }
    void collapse_overlays(bool clear_level2, bool run_extra_hook) override {
        // 0068AD43: 0068AB80 collapse_in_game_overlays over the menu's screen sets; OFF keeps the record.
        owner_.collapse_in_game_overlays(clear_level2, run_extra_hook,
            "InGameInterface::collapse_overlays");
    }
    InGameInterfaceUnitQuery& unit_query() override { return query_; }
    bool is_multiplayer() override { return false; }
    void redispatch(int interface_id, bool has_payload) override {
        // 0068AFB4, `this->vtable[10h](newId, payload)`, which is 0068ACA0
        // itself. Milestone 2h recorded it because the null-payload arm chose no
        // id; with the controlled unit as the payload the 20h arm is a unit-kind
        // classifier and this re-entry is what publishes the ship HUD.
        owner_.done("InGameInterface::redispatch", 0x0068afb4u);
        if (depth_ >= 4) return;  // the native has no guard; a cycle would hang
        ++depth_;
        apply_in_game_interface_0068aca0(owner_.manager, *this, interface_id, has_payload);
        --depth_;
    }

private:
    GameHudHost::Impl& owner_;
    HudUnitQuery& query_;
    int depth_{0};
};

// ---------------------------------------------------------------------------
// 0068c1f0's host, the per-frame update
// ---------------------------------------------------------------------------
//
// Step 17 of the in-mission branch of 004e4a40 calls this at 004e5252. Almost
// everything it reads belongs to one of the 42 screen classes, to the camera at
// game+19FCh or to the controlled unit at 00e188d8, and this process owns none
// of those: every one of them is a record with its own address and a neutral
// answer, and the recovered control flow runs over them. The two exceptions are
// the level-3 vector and the input contexts, which are the same stacks the
// level-1 publish uses.
class HudUpdateBinding final : public InMissionInterfaceUpdateHost {
public:
    explicit HudUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    int effective_game_mode() override {
        owner_.done("InGameInterfaceUpdate::effective_game_mode", 0x004bca50u);
        return owner_.effective_game_mode;
    }
    int local_player_slot() override { return 0; }
    bool local_slot_flag_19() override { return false; }
    int local_slot_unit_id() override { return -1; }
    bool has_controlled_unit() override {
        // 00e188d8, written by BSP_Game_SetControlledUnit 004c0880; the units
        // host binds its controlled unit through 004c0890.
        if constexpr (kHudInterfaceControlledUnitBound) {
            owner_.done("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
            return owner_.units != nullptr && oct18_tail_a::hud_controlled_unit_00644230(
                owner_.units->controlled_unit_publication()) != nullptr;
        } else {
            owner_.record("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
            return false;
        }
    }
    bool input_action_pressed(int action) override {
        if (!kHudPresentationTopBound) {
            static_cast<void>(action);
            owner_.record("InGameInterfaceUpdate::input_action_pressed", 0x004c43c0u);
            return false;
        }
        // 004C43C0 through the menu host's action records, the same route the
        // application frame's 00737AE7 test takes (GameFrameHost). The only
        // record the executable drives is the press-start action 4Eh; every
        // other index is a record 00A92370 zeroes each frame and nothing starts,
        // which is the state of an action no device reports, so the rising-edge
        // test answers false for it.
        owner_.done("InGameInterfaceUpdate::input_action_pressed", 0x004c43c0u);
        return owner_.menu.input_action_pressed(action);
    }
    bool modifier_key_allows_back_out() override {
        // 0068C65A..0068C66E: [00F88A30] or GetKeyState(A4h, VK_LMENU) >= 0.
        // Packet cc13-releasegun: an armed plan answers "left Alt not held";
        // the harness has no keyboard (LABELLED).
        return kHarnessReleaseGunMapBound && owner_.release_gun_armed;
    }
    bool session_flag_19c4() override { return false; }
    bool overlay_c_extra_gate() override { return false; }

    // The "camera screen" of this interface is the support-manager screen 4Eh,
    // ECX = [manager+BCh] at every site below (0068C237..0068C9AF); 0068CC70
    // builds it on its leaf block through 006735D0. Its register 0066EA60 does
    // not run on that block (chain 629), but it only zeroes +0Ah, +30h and +66h
    // of what these read (bsp.py show 0066ea60), and the block starts zeroed,
    // so each answer is the block's own byte. +4h is the screen's FrontEndScreen
    // flag the menu registry sets. Before Init has built the block the answers
    // stay the neutral records they were.
    bool camera_screen_spectating() override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) return record_false("006529e0", 0x006529e0u);
        owner_.done("InGameInterfaceUpdate::support_screen_byte_04", 0x006529e0u);
        return bsp::cc13_front4::overlay_busy_006529e0({s}) != 0;
    }
    bool camera_screen_flag_09() override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) return record_false("00652a20", 0x00652a20u);
        owner_.done("InGameInterfaceUpdate::support_screen_byte_09", 0x00652a20u);
        return bsp::cc13_front5::byte_09_get_00652a20({s}) != 0;
    }
    bool camera_screen_flag_08() override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) return record_false("00652a30", 0x00652a30u);
        owner_.done("InGameInterfaceUpdate::support_screen_byte_08", 0x00652a30u);
        return bsp::cc13_front5::byte_08_get_00652a30({s}) != 0;
    }
    void camera_screen_set_flag_08(bool value) override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) {
            owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_08", 0x00652a50u);
            return;
        }
        owner_.done("InGameInterfaceUpdate::support_screen_set_byte_08", 0x00652a50u);
        bsp::cc13_front5::byte_08_set_00652a50({s}, value ? 1u : 0u);
    }
    // 0068C25B CMP byte [ECX+0Ah] and 0068C7D8 MOV byte [ECX+0Ah],1 are inline.
    bool camera_screen_flag_0a() override {
        const std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        return s != nullptr && s[0x0A] != 0;
    }
    void camera_screen_set_flag_0a(bool value) override {
        if (std::uint8_t* const s = leaf_block_at(kSupportScreenOffset)) s[0x0A] = value ? 1u : 0u;
    }
    // 0068C7C0..0068C7CC: [[00E198C4]+BCh]+30h, the same screen through the global.
    bool camera_screen_flag_30() override {
        const std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        return s != nullptr && s[0x30] != 0;
    }
    bool camera_screen_flag_66() override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) return record_false("006529c0", 0x006529c0u);
        owner_.done("InGameInterfaceUpdate::support_screen_byte_66", 0x006529c0u);
        return bsp::cc13_front5::byte_66_get_006529c0({s}) != 0;
    }
    void camera_screen_set_flag_66(bool value) override {
        std::uint8_t* const s = leaf_block_at(kSupportScreenOffset);
        if (s == nullptr) {
            owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_66", 0x006529b0u);
            return;
        }
        owner_.done("InGameInterfaceUpdate::support_screen_set_byte_66", 0x006529b0u);
        bsp::cc13_front5::byte_66_set_006529b0({s}, value ? 1u : 0u);
    }
    void camera_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::camera_screen_frame_step", 0x00673130u);
    }

    std::size_t level3_screen_count() override { return 0; }
    int level3_screen_id(std::size_t index) override { static_cast<void>(index); return 0; }
    void set_level3_screen_set(const int* ids, std::size_t count) override {
        static_cast<void>(ids);
        static_cast<void>(count);
        owner_.record("InGameInterfaceUpdate::set_level3_screen_set", 0x004f8670u);
    }
    void set_level3_input_contexts(const int* ids, std::size_t count) override {
        owner_.menu.publish_level3_input_contexts(ids, count);
        owner_.done("InGameInterfaceUpdate::set_level3_input_contexts", 0x004d8b70u);
    }

    // The spectate walk over [[game+19cch]+58h]. construct_world 004de610 is a
    // load record, so the world object that list hangs off does not exist.
    std::size_t spectate_unit_count() override {
        owner_.record("InGameInterfaceUpdate::spectate_unit_list", 0x004de610u);
        return 0;
    }
    bool unit_is_kind_of(std::size_t, int) override { return false; }
    bool unit_has_pilot(std::size_t) override { return false; }
    int unit_owner_id(std::size_t) override { return -1; }
    int unit_pilot_state(std::size_t) override { return 0; }
    bool unit_dead(std::size_t) override { return false; }
    int unit_player_slot(std::size_t) override { return -1; }
    int unit_team(std::size_t) override { return 0; }
    void pilot_detach(std::size_t) override {
        owner_.record("InGameInterfaceUpdate::pilot_detach", 0x007ee4e0u);
    }
    void unit_bind_player(std::size_t, int) override {
        owner_.record("InGameInterfaceUpdate::unit_bind_player", 0x00927cc0u);
    }
    void hud_root_set_spectated_unit(std::size_t, bool) override {
        owner_.record("InGameInterfaceUpdate::hud_root_set_spectated_unit", 0x00647300u);
    }

    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        owner_.record("InGameInterfaceUpdate::push_interface_request", 0x004cc460u);
    }
    void push_scene_request_for_unit() override {
        owner_.record("InGameInterfaceUpdate::push_scene_request_for_unit", 0x004b4b00u);
    }
    void toggle_tactical_overlay(std::uint32_t) override {
        owner_.record("InGameInterfaceUpdate::toggle_tactical_overlay", 0x0068c0b0u);
    }
    void exit_free_camera_overlay() override {
        owner_.record("InGameInterfaceUpdate::exit_free_camera_overlay", 0x0068b3f0u);
    }
    void back_out_one_overlay_level() override {
        if (kHarnessReleaseGunMapBound && owner_.release_gun_armed) {
            owner_.back_out_one_overlay_level_0068b470();
            return;
        }
        owner_.record("InGameInterfaceUpdate::back_out_one_overlay_level", 0x0068b470u);
    }
    void collapse_overlays(bool clear_level2, bool run_extra_hook) override {
        // 0068C6E7: 0068AB80 collapse_in_game_overlays over the menu's screen sets; OFF keeps the record.
        owner_.collapse_in_game_overlays(clear_level2, run_extra_hook,
            "InGameInterfaceUpdate::collapse_overlays");
    }
    void toggle_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_movie_camera", 0x0068a160u);
    }
    void toggle_new_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_new_movie_camera", 0x0068a1f0u);
    }
    bool query_scene_unit(SceneRequestUnitFlags& flags, bool& is_pending_payload) override {
        flags = SceneRequestUnitFlags{};
        is_pending_payload = false;
        owner_.record("InGameInterfaceUpdate::query_scene_unit", 0x004b4b00u);
        return false;
    }

    void hud_root_set_field_1c(std::uint32_t value) override {
        owner_.set_root_closed_value(static_cast<std::int32_t>(value));
    }
    // The free-camera screen 2Bh, ECX = [manager+90h], on its 00540BA0 leaf
    // block: 0068C40E tests its +4h (the FrontEndScreen's wanted flag) inline,
    // and 0068C440 / 0068C449 call 0053D290 with 0 or 1.
    bool screen_2b_wanted() override {
        const FrontEndScreen* const f = leaf_flags_at(kFreeCameraScreenOffset);
        return f != nullptr && f->wanted;
    }
    void screen_2b_set_mode(bool mode) override {
        std::uint8_t* const s = leaf_block_at(kFreeCameraScreenOffset);
        if (s == nullptr) {
            owner_.record("InGameInterfaceUpdate::screen_2b_set_mode", 0x0053d290u);
            return;
        }
        owner_.done("InGameInterfaceUpdate::free_camera_screen_set_28", 0x0053d290u);
        bsp::cc13_front5::dword_28_set_0053d290({s}, mode ? 1u : 0u);
    }
    bool screen_34_query() override {
        if constexpr (kHudHintsSecondsLeftBound) {
            // 0068C526..0068C533: ECX = [this+A8h]; CALL 0054D3C0; TEST EAX,EAX;
            // JNE. Non-zero for |+54h| >= 1.0 and for NaN or |+54h| >= 2^31
            // (80000000h), zero for +54h in (-1, 1).
            owner_.done("InGameInterfaceUpdate::screen_34_query", 0x0054d3c0u);
            // cc15-hint2: the screen 34h the hint tracker shows and times
            // (src/game_hint_passes.cpp); the zeroed host copy below only
            // answers before the tracker exists.
            std::int32_t seconds_left = 0;
            if (hints_seconds_left_0054d3c0_cc15(seconds_left)) return seconds_left != 0;
            const bsp::cc13_sweepA::Raw screen{owner_.hints_screen};
            return bsp::cc13_front6::hints_screen_seconds_left_0054d3c0(screen) != 0;
        } else {
            return record_false("0054d3c0", 0x0054d3c0u);
        }
    }
    // The formation screen 2Ah, [manager+5Ch], on its 00538E30 leaf block:
    // 0068C6D4..0068C6E3 tests its +4h and clears it with MOV byte [EAX+4],0,
    // the wanted flag the menu registry borrows, before 0068AB80(1, 1).
    bool screen_2a_wanted() override {
        const FrontEndScreen* const f = leaf_flags_at(kFormationScreenOffset);
        return f != nullptr && f->wanted;
    }
    void screen_2a_clear_wanted() override {
        if (FrontEndScreen* const f = leaf_flags_at(kFormationScreenOffset)) f->wanted = false;
    }
    // 0068C685..0068C698: ECX = [manager+64h], the order screen 3Bh that
    // 0068CC70 builds on its leaf block through 005F9440 (its register 005F9FF0
    // runs there too); CMP byte [EAX+81h] / [EAX+82h], inline.
    bool screen_3b_block_a() override {
        const std::uint8_t* const s = leaf_block_at(kOrderScreenOffset);
        return s != nullptr && s[0x81] != 0;
    }
    bool screen_3b_block_b() override {
        const std::uint8_t* const s = leaf_block_at(kOrderScreenOffset);
        return s != nullptr && s[0x82] != 0;
    }
    // 005FB080 reads the controlled unit's native fields (+3B0h on a squadron)
    // and the manager's +20h/+40h/+60h, none of which this process has as
    // native objects; it stays a record.
    void screen_3b_idle() override {
        owner_.record("InGameInterfaceUpdate::screen_3b_idle", 0x005fb080u);
    }
    bool screen_45_block_a() override { return false; }
    bool screen_45_block_b() override { return false; }
    // The "map screen" is the powerup-manager screen 4Fh, ECX = [manager+C0h]
    // (0068C784..0068C9C2), built on its leaf block by 0061A480. 0068C795 and
    // 0068C80B test its +4h inline: the FrontEndScreen flag the menu registry
    // sets when the screen is shown.
    bool map_screen_wanted() override {
        const FrontEndScreen* const f = leaf_flags_at(kPowerupScreenOffset);
        return f != nullptr && f->wanted;
    }
    // 00612DA0 starts on the widgets its register 00614170 loads (+B0h, +D4h,
    // +E4h..+ECh), and that register does not run on the leaf block (chain
    // 629: it takes a typed state), so the step stays a record.
    void map_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::map_screen_frame_step", 0x00612da0u);
    }
    // 0068C9C2..0068C9CE: 00611750 on the same block, +1F9h or +13Fh. The
    // skipped register only clears +1F9h (00614170 at 006142EC, BL = 0), which
    // the zeroed block already holds; the constructor stores +13Fh (0061A513).
    bool map_screen_busy() override {
        std::uint8_t* const s = leaf_block_at(kPowerupScreenOffset);
        if (s == nullptr) return record_false("00611750", 0x00611750u);
        owner_.done("InGameInterfaceUpdate::powerup_screen_scrolling", 0x00611750u);
        return bsp::seg25_cc13::powerup_manager_is_scrolling_00611750(s);
    }
    std::uint32_t map_screen_overlay_argument() override { return 0; }
    bool pause_screen_flag_05() override { return false; }
    bool pause_screen_flag_08() override { return false; }
    bool pause_screen_child_query() override { return false; }
    bool support_request_pending() override {
        return game_powerups::local_player_has_usable_powerup();
    }

    void input_manager_update(float seconds) override {
        static_cast<void>(seconds);
        // 004bec00 then 00a92c40. The mission frame already ticks the input
        // manager through its own step; this call is the manager's own.
        owner_.record("InGameInterfaceUpdate::input_manager_update", 0x004bec00u);
    }
    bool device_a_flag_0b() override { return false; }
    bool device_a_flag_10() override { return false; }
    bool device_b_flag_0b() override { return false; }
    bool device_b_flag_10() override { return false; }
    std::uint32_t controlled_unit_overlay_argument() override { return 0; }

    bool has_ambient_sound_instance() override { return false; }
    bool camera_present() override {
        // game+19fch, the in-mission camera. Nothing in this process builds one.
        owner_.record("InGameInterfaceUpdate::camera", 0x004e4a40u);
        return false;
    }
    void refresh_camera_transform() override {
        owner_.record("InGameInterfaceUpdate::refresh_camera_transform", 0x00b6db70u);
    }
    float camera_height() override { return 0.0f; }
    float camera_ground_x() override { return 0.0f; }
    void set_ambient_volume(float volume) override {
        static_cast<void>(volume);
        owner_.record("InGameInterfaceUpdate::set_ambient_volume", 0x00a79880u);
    }
    float water_height(float x, float y) override {
        static_cast<void>(x);
        static_cast<void>(y);
        owner_.record("InGameInterfaceUpdate::water_height", 0x0078cf20u);
        return 0.0f;
    }
    // 0068CB4A..0068CB6D. First the plane-effects screen 3Fh ([manager+6Ch],
    // the 00606470 leaf block): its +4h, then 00604F30 on it. 00604F30 answers
    // 0 while the block's plane +B8h is null, which is the constructor's store
    // (00606470) and stays so because the 0068ACA0 plane arm that binds one is
    // a record here; only a bound plane would reach the kind query, which
    // HudLeafFe5Binding leaves unreached. Then the periscope screen 47h
    // ([manager+84h], the 00650B00 leaf block): its +4h and CMP dword [+74h],2.
    bool cockpit_screen_wanted() override {
        const FrontEndScreen* const f = leaf_flags_at(kPlaneEffectsScreenOffset);
        return f != nullptr && f->wanted;
    }
    int cockpit_screen_mode() override {
        HudLeafScreenStorage* const leaf = leaf_at(kPlaneEffectsScreenOffset);
        if (leaf == nullptr) return record_false("00604f30", 0x00604f30u) ? 1 : 0;
        HudLeafFe5Binding binding(*leaf);
        owner_.done("InGameInterfaceUpdate::plane_effects_cockpit_view", 0x00604f30u);
        return static_cast<int>(
            bsp::cc13_fe5::plane_screen_cockpit_view_flag_00604f30({leaf->data()}, binding));
    }
    bool second_cockpit_screen_wanted() override {
        const FrontEndScreen* const f = leaf_flags_at(kPeriscopeScreenOffset);
        return f != nullptr && f->wanted;
    }
    int second_cockpit_screen_mode() override {
        const std::uint8_t* const s = leaf_block_at(kPeriscopeScreenOffset);
        if (s == nullptr) return 0;
        std::int32_t mode = 0;
        std::memcpy(&mode, s + 0x74, sizeof(mode));
        return mode;
    }
    bool controlled_unit_is_submarine() override {
        // 0068CC04: IsKindOf(8) on 00E188D8.
        if constexpr (kHudInterfaceControlledUnitBound) {
            return owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 8);
        } else {
            return false;
        }
    }
    void refresh_controlled_unit_pose() override {}
    float controlled_unit_height() override {
        // The unit's +100h, the world translation's y.
        if constexpr (kHudInterfaceControlledUnitBound) {
            float r[3], u[3], f[3], o[3];
            if (owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_pose(owner_.units->controlled_index(), r, u, f, o)) {
                return o[1];
            }
        }
        return 0.0f;
    }
    void set_audio_environment(std::string_view name) override {
        owner_.summary.audio_environment.assign(name.data(), name.size());
        // 0068CC3C..0068CC4F: PUSH the chosen literal, ECX = [00F8BBD8], CALL
        // 00A7B710 on every update. The body finds the listener by name
        // (00A7AE00), and only when that ordinal differs from +10Ch moves +10Ch
        // to +110h, stores the new one and selects it (00A7AE80, +104h/+108h),
        // on the listener table the sound Lua configuration (00A7FF80) built.
        // The image has no null test; a runtime that never started has no
        // configuration, so that case stays the record below.
        GameSoundRuntime* const sound = g_hud_sound_runtime;
        if (sound != nullptr && sound->started()) {
            SoundConfigurationState& configuration = sound->owner().configuration;
            const std::int32_t before = configuration.current_listener_10c;
            bsp::cc13_audio::set_sound_listener_environment_00a7b710(configuration,
                owner_.summary.audio_environment.c_str(), sound->fmod());
            ++owner_.summary.audio_environment_selects;
            if (configuration.current_listener_10c != before)
                ++owner_.summary.audio_environment_changes;
            owner_.done("InGameInterfaceUpdate::set_audio_environment", 0x00a7b710u);
            return;
        }
        owner_.record("InGameInterfaceUpdate::set_audio_environment", 0x00a7b710u);
    }
    void tick_profile_hints(int slot) override {
        static_cast<void>(slot);
        owner_.record("InGameInterfaceUpdate::tick_profile_hints", 0x004c1e90u);
    }

private:
    // The manager slots the reads above dereference (src/ingame_interface.cpp).
    static constexpr std::uint16_t kFormationScreenOffset = 0x5C;   // screen 2Ah
    static constexpr std::uint16_t kOrderScreenOffset = 0x64;       // screen 3Bh
    static constexpr std::uint16_t kPlaneEffectsScreenOffset = 0x6C;  // screen 3Fh
    static constexpr std::uint16_t kPeriscopeScreenOffset = 0x84;     // screen 47h
    static constexpr std::uint16_t kFreeCameraScreenOffset = 0x90;  // screen 2Bh
    static constexpr std::uint16_t kSupportScreenOffset = 0xBC;     // screen 4Eh
    static constexpr std::uint16_t kPowerupScreenOffset = 0xC0;     // screen 4Fh
    // The leaf block 0068CC70 built for [manager+offset], or null.
    HudLeafScreenStorage* leaf_at(std::uint16_t offset) const {
        const std::size_t index = bsp::in_game_hud_screen_index_at(offset);
        return index < kInGameHudScreenCount ? owner_.leaf_screens[index].get() : nullptr;
    }
    std::uint8_t* leaf_block_at(std::uint16_t offset) const {
        HudLeafScreenStorage* const leaf = leaf_at(offset);
        return leaf != nullptr ? leaf->data() : nullptr;
    }
    // Its +4h/+5h, the FrontEndScreen the leaf constructed and the registry borrows.
    FrontEndScreen* leaf_flags_at(std::uint16_t offset) const {
        HudLeafScreenStorage* const leaf = leaf_at(offset);
        return leaf != nullptr ? &leaf->flags() : nullptr;
    }
    bool record_false(const char* suffix, std::uint32_t address) {
        char method[80];
        std::snprintf(method, sizeof(method), "InGameInterfaceUpdate::query_%s", suffix);
        owner_.record(method, address);
        return false;
    }

    GameHudHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0068cc70's host, the manager's Init
// ---------------------------------------------------------------------------
//
// bsp::init_in_game_interface_0068cc70 drives this, so the order the 42 screens
// are built and registered in is the recovered one. The root (0068BF60), the
// scene-init screen (00636D90) and the kHudLeafScreenConstructors leaves are
// constructed on this owner's own blocks and the registry borrows each one's
// base flags; for every other leaf the allocation and the constructor are
// records and the registry record is the executable's own, the substitution
// milestone 2c makes for the seven main-menu screen classes. What register_screen then does is the
// recovered part of each +10h override, 004f71d0 plus the page the screen loads
// through 00aa5840 and the widget names it binds through 00aa7e00.
class HudInitBinding final : public InMissionInterfaceInitHost {
public:
    explicit HudInitBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    void load_texture_atlas(std::string_view stem) override {
        static_cast<void>(stem);
        // 00af0060 on the atlas manager at 00f8c26c. The sprite bridge already
        // merges every atlas the installation ships, so the load is the
        // bridge's and the native call stays a record.
        owner_.record("InGameInterface::load_atlas", 0x00af0060u);
    }
    void publish_manager_global() override {
        // 0068ccfe, the second write of 00e198c4. The object is this process's
        // own InGameInterfaceManager, so the publish is real.
        owner_.summary.manager_global_published = true;
        owner_.done("InGameInterface::publish_manager_global", 0x0068ccfeu);
    }
    void base_init() override {
        // 0068CD04 calls the inherited +4h virtual, 00683A90. Its entire
        // retail body is RET (bsp.py show 00683a90); it needs no object fields.
        bsp::front_end_manager_init_default_00683a90();
        owner_.done("InGameInterface::base_init", 0x00683a90u);
    }
    bool allocate_screen(std::size_t index, std::size_t size_bytes) override {
        if (index == 0) {
            if (size_bytes != sizeof(HudRootConstructionStorage))
                throw std::logic_error("0068CC70 HUD root allocation size changed");
            // This screen is inline in the HUD owner, alive before any
            // registry borrows it and until that owner is destroyed.
            owner_.done("InGameInterface::allocate_root_screen", 0x00bf681bu);
            return true;
        }
        if (index < kInGameHudScreenCount
            && kInGameHudScreens[index].registry_slot == kSceneInitRegistrySlot) {
            if (size_bytes != sizeof(HudSceneInitStorage))
                throw std::logic_error("0068CC70 scene-init allocation size changed");
            owner_.done("InGameInterface::allocate_scene_init_screen", 0x00bf681bu);
            return true;
        }
        if (index < kInGameHudScreenCount
            && is_hud_leaf_screen_constructor(kInGameHudScreens[index].constructor)) {
            if (owner_.leaf_screens[index])
                throw std::logic_error("0068CC70 allocated a HUD leaf screen twice");
            owner_.leaf_screens[index] = std::make_unique<HudLeafScreenStorage>(size_bytes);
            owner_.done("InGameInterface::allocate_leaf_screen", 0x00bf681bu);
            return true;
        }
        owner_.record("InGameInterface::allocate_screen", 0x00bf681bu);
        return true;
    }
    void construct_screen(std::size_t index, std::uint32_t constructor) override {
        if (index == 0 && constructor == 0x0068BF60u) {
            // bsp.py show 0068bf60: one base call, then native vtables,
            // empty vectors, observer state and literal flags. No engine
            // services or fixture-supplied inputs are needed on this path.
            HudRootConstructionBinding binding(owner_.constructed_root_screen);
            bsp::cc13_fe5::hud_root_screen_construct_0068bf60(
                {owner_.constructed_root_screen.data()}, binding);
            ++owner_.summary.screen_constructors;
            owner_.done("InGameInterface::root_screen_constructor", constructor);
        } else {
            if (index < kInGameHudScreenCount
                && kInGameHudScreens[index].registry_slot == kSceneInitRegistrySlot) {
                if (constructor != 0x00636D90u)
                    throw std::logic_error("0068CC70 scene-init constructor changed");
                // bsp.py show 00636d90: base construction, callback
                // owner zero stores and the two native vtable values.
                HudSceneInitBinding binding(owner_.constructed_scene_init_screen, owner_.menu);
                bsp::ingame_gui_cc13::scene_init_construct_00636d90(
                    {owner_.constructed_scene_init_screen.data()}, binding);
                ++owner_.summary.screen_constructors;
                owner_.done("InGameInterface::scene_init_constructor", constructor);
            } else if (index < kInGameHudScreenCount && owner_.leaf_screens[index]) {
                if (constructor != kInGameHudScreens[index].constructor)
                    throw std::logic_error("0068CC70 leaf constructor changed");
                run_hud_leaf_screen_constructor(constructor, *owner_.leaf_screens[index]);
                ++owner_.summary.screen_constructors;
                owner_.done("InGameInterface::leaf_screen_constructor", constructor);
            } else {
                if (constructor != 0) ++owner_.summary.screen_constructors;
                owner_.record("InGameInterface::screen_constructor", 0x0068cc70u);
            }
        }
    }
    void register_screen(std::size_t index) override {
        if (index >= kInGameHudScreenCount) return;
        const InGameHudScreenSlot& slot = kInGameHudScreens[index];
        const HudScreenLayout* layout = hud_screen_layout_for_slot(slot.registry_slot);
        GameHudScreenRecord record;
        record.registry_slot = slot.registry_slot;
        record.manager_offset = slot.offset;
        record.register_virtual = layout != nullptr ? layout->register_virtual : 0;
        record.layout_virtual = layout != nullptr ? layout->layout_virtual : 0;

        char name[32];
        std::snprintf(name, sizeof(name), "HUD%02X",
            static_cast<unsigned>(slot.registry_slot));
        // The leaf's own register override over its block, for the overrides
        // kHudLeafScreenRegisters lists. As for scene-init, the generic page
        // loader below must not also run on that screen.
        bool leaf_registered = false;
        const bool registered = [&]() {
            if (owner_.leaf_screens[index] && layout != nullptr
                && is_hud_leaf_screen_register(layout->register_virtual)) {
                if (owner_.menu.in_game_screen_registered(slot.registry_slot)) return false;
                HudLeafRegisterContext context{*owner_.leaf_screens[index], owner_.menu,
                    slot.registry_slot, name, layout->register_virtual, layout->layout_virtual};
                run_hud_leaf_screen_register(context);
                owner_.done("InGameInterface::leaf_screen_register", layout->register_virtual);
                leaf_registered = true;
                return true;
            }
            if (slot.registry_slot == kSceneInitRegistrySlot) {
                // bsp.py show 00636f30: register this exact base, load
                // FE_sceneinit, bind Message_Text, retain its four colour
                // words and hide the loaded page. The generic host page
                // loader below must not also run on this screen.
                HudSceneInitBinding binding(owner_.constructed_scene_init_screen, owner_.menu);
                bsp::ingame_gui_cc13::scene_init_register_00636f30(
                    {owner_.constructed_scene_init_screen.data()}, binding);
                owner_.done("InGameInterface::scene_init_register", 0x00636f30u);
                return true;
            }
            FrontEndScreen* const constructed = index == 0 ? &owner_.constructed_root_screen.flags
                : owner_.leaf_screens[index] ? &owner_.leaf_screens[index]->flags() : nullptr;
            return owner_.menu.register_in_game_screen(slot.registry_slot, name,
                record.register_virtual, record.layout_virtual, constructed);
        }();
        if (!registered) {
            owner_.log.notef("HUD screen slot %02X could not be registered",
                static_cast<unsigned>(slot.registry_slot));
            return;
        }
        owner_.done("InGameInterface::screen_register", 0x004f71d0u);
        ++owner_.summary.screens_built;

        if (layout != nullptr) {
            for (std::size_t p = 0; p < layout->page_count; ++p) {
                const std::string page = layout->pages[p];
                record.pages.push_back(page);
                ++owner_.summary.pages_requested;
                const bool loaded = [&]() {
                    if (slot.registry_slot == kSceneInitRegistrySlot || leaf_registered)
                        return owner_.menu.in_game_page(slot.registry_slot, page) != nullptr;
                    return owner_.menu.attach_in_game_page(slot.registry_slot, page);
                }();
                if (loaded) {
                    ++record.pages_loaded;
                    ++owner_.summary.pages_loaded;
                }
            }
            for (std::size_t w = 0; w < layout->widget_count; ++w) {
                const std::string widget = layout->widgets[w].name;
                ++record.widgets_requested;
                ++owner_.summary.widgets_requested;
                const bool bound = [&]() {
                    if (slot.registry_slot == kSceneInitRegistrySlot && widget == "Message_Text")
                        return bsp::ingame_gui_cc13::Raw{
                            owner_.constructed_scene_init_screen.data()}.ptr(0x20) != nullptr;
                    return owner_.menu.in_game_page_has_child(slot.registry_slot, widget);
                }();
                if (bound) {
                    ++record.widgets_bound;
                    ++owner_.summary.widgets_bound;
                }
            }
        }
        owner_.summary.screens.push_back(std::move(record));
    }
    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 0068d73a, 004cc460(20h, 0). The pending record it writes is the
        // front-end owner's; the id is what the executable applies.
        owner_.summary.pushed_interface_id = interface_id;
        owner_.note_pending_record_004cc460(interface_id, false);
        owner_.record("InGameInterface::push_interface_request", 0x004cc460u);
    }
    void hud_root_set_field_1c(std::uint32_t value) override {
        owner_.set_root_closed_value(static_cast<std::int32_t>(value));
    }

private:
    GameHudHost::Impl& owner_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameHudHost
// ---------------------------------------------------------------------------

GameHudHost::GameHudHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudHost::~GameHudHost() = default;

bool GameHudHost::show_mission_narrative_text(const std::string& source) {
    auto* const page = impl_->menu.in_game_page(0x33, "GUI_narrative");
    if (page == nullptr || !page->root) return false;
    auto* const group = bsp::find_descendant_by_name(*page->root, "MissionInfoTitle_Group");
    auto* const text = group != nullptr
        ? bsp::find_descendant_by_name(*group, "textbox_obj_Text") : nullptr;
    if (text == nullptr) return false;
    // 005BB130's +AC/+B0 lookups, then the visibility/source portion of
    // 005B7390. This does not claim its text/frame metrics or output timings.
    auto& frontend = impl_->menu.frontend();
    frontend.set_widget_visible(*group, true);
    frontend.set_widget_text_source(*text, source);
    return true;
}

bool GameHudHost::hide_mission_narrative_text() {
    auto* const page = impl_->menu.in_game_page(0x33, "GUI_narrative");
    if (page == nullptr || !page->root) return false;
    auto* const group = bsp::find_descendant_by_name(*page->root, "MissionInfoTitle_Group");
    if (group == nullptr) return false;
    const NarrativeHideView view{{}, group};
    NarrativeVisibilityBinding binding(impl_->menu.frontend());
    // bsp.py show 005B5D40: +ACh -> widget vtable+34h(0), no other
    // fields read. Native ABI is ECX=screen, RET; this is its source view.
    bsp::cc13_fe1::hud_narrative_hide_text_005b5d40(&view, binding);
    impl_->done("HudNarrative::hide", 0x005b5d40u);
    return true;
}

namespace {
// [00E198C4]+C8h, the counters block, once its register built both entries,
// with the image objects the countdown path reads (HudCountersContext).
std::optional<HudCountersContext> hud_counters_context(GameHudHost::Impl& impl) {
    constexpr std::uint16_t kCountersOffset = 0xC8;  // kInGameHudScreens: 51h
    const std::size_t index = bsp::in_game_hud_screen_index_at(kCountersOffset);
    HudLeafScreenStorage* const counters = index < kInGameHudScreenCount
        ? impl.leaf_screens[index].get() : nullptr;
    if (counters == nullptr) return std::nullopt;
    const bsp::ingame_gui_cc13::Raw s{counters->data()};
    if (s.ptr(0x0C) == nullptr || s.ptr(0x10) == nullptr) return std::nullopt;  // 0052CBF0 not run
    const Win32PlatformState* const platform = active_platform_state();
    if (platform == nullptr) return std::nullopt;
    // 0052A860 reads the Text widget's measured width (+114h), which only the
    // text bridge lays out; a run without a device opens none.
    if (!impl.menu.frontend().summary().text_bridge_open) return std::nullopt;
    HudCountersContext c{impl.menu.frontend(), *counters, {}, &platform->native_window_focus(),
        impl.counters_visibility_recurses};
    for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
        if (!impl.leaf_screens[i]) continue;
        const std::size_t slot = kInGameHudScreens[i].offset / 4u;
        if (slot >= c.manager_view.size())
            throw std::logic_error("kInGameHudScreens offset beyond the counters' manager view");
        c.manager_view[slot] = impl.leaf_screens[i]->data();
    }
    // The screens 00529A30 / 00529810 read: +54h, +58h, +60h, +BCh, +C0h.
    static constexpr std::array<std::uint16_t, 5> kRead{{0x54, 0x58, 0x60, 0xBC, 0xC0}};
    for (const std::uint16_t offset : kRead) {
        if (c.manager_view[offset / 4u] == nullptr) return std::nullopt;
    }
    c.record = [&impl](const char* method, std::uint32_t address) { impl.record(method, address); };
    c.score_participants = impl.score_participants;
    c.score_local_slot = impl.score_local_slot;
    return c;
}
}  // namespace

bool GameHudHost::show_mission_countdown(const std::string& text, float seconds) {
    std::optional<HudCountersContext> c = hud_counters_context(*impl_);
    if (!c) {
        impl_->record("HudCounters::show_countdown", 0x0052b9b0u);
        return false;
    }
    HudCountersShowBinding binding(*c);
    bsp::frontend_seg15_22_cc13::hud_counters_show_countdown_0052b9b0(
        {c->counters.data()}, text, seconds, binding);
    impl_->done("HudCounters::show_countdown", 0x0052b9b0u);
    return true;
}

bool GameHudHost::cancel_mission_countdown() {
    std::optional<HudCountersContext> c = hud_counters_context(*impl_);
    if (!c) {
        impl_->record("HudCounters::cancel_countdown", 0x0052ab90u);
        return false;
    }
    HudCountersShowBinding binding(*c);
    // 008B1A92 and 008B1AA2: two calls; the second finds the entry hidden.
    bsp::frontend_seg15_22_cc13::hud_counters_cancel_countdown_0052ab90({c->counters.data()}, binding);
    bsp::frontend_seg15_22_cc13::hud_counters_cancel_countdown_0052ab90({c->counters.data()}, binding);
    impl_->done("HudCounters::cancel_countdown", 0x0052ab90u);
    return true;
}

bool GameHudHost::display_score(std::uint32_t id, std::uint32_t owner, const std::string& a,
    const std::string& b, bool pair, std::uint32_t v0, std::uint32_t v1) {
    const std::uint32_t sender = pair ? 0x0052b780u : 0x0052b620u;
    std::optional<HudCountersContext> c = hud_counters_context(*impl_);
    if (!c) {
        impl_->record("HudCounters::display_score", sender);
        return false;
    }
    HudScoreMessageBinding binding(*c);
    if (pair) {
        bsp::frontend_seg15_22_cc13::hud_send_display_score_pair_0052b780(id, owner, a, b, v0, v1, binding);
    } else {
        bsp::frontend_seg15_22_cc13::hud_send_display_score_0052b620(id, owner, a, b, binding);
    }
    impl_->done("HudCounters::display_score", sender);
    return true;
}

bool GameHudHost::hide_score(std::uint32_t id, std::uint32_t owner) {
    std::optional<HudCountersContext> c = hud_counters_context(*impl_);
    if (!c) {
        impl_->record("HudCounters::hide_score", 0x0052b900u);
        return false;
    }
    HudScoreMessageBinding binding(*c);
    bsp::frontend_seg15_22_cc13::hud_send_hide_score_0052b900(id, owner, binding);
    impl_->done("HudCounters::hide_score", 0x0052b900u);
    return true;
}

namespace {
// [00E198C4]+58h, the minimap screen 35h's leaf block (005C0810).
std::uint8_t* hud_minimap_block(GameHudHost::Impl& impl) {
    const std::size_t index = bsp::in_game_hud_screen_index_at(0x58);
    return index < kInGameHudScreenCount && impl.leaf_screens[index]
        ? impl.leaf_screens[index]->data() : nullptr;
}
}  // namespace

bool GameHudHost::begin_countdown_panel(const std::string& text) {
    std::uint8_t* const panel = hud_minimap_block(*impl_);
    if (panel == nullptr) return false;
    // The native empty body never interprets its text pointer; borrow this
    // call's real text object rather than fabricate a native-string header.
    bsp::cc13_fe1::hud_panel_begin_005bca70(panel, nullptr, &text);
    impl_->done("HudCountdown::begin", 0x005bca70u);
    return true;
}

bool GameHudHost::set_countdown_panel(float fraction, float left) {
    std::uint8_t* const panel = hud_minimap_block(*impl_);
    if (panel == nullptr) {
        impl_->record("HudCountdown::update", 0x005bca80u);
        return false;
    }
    // The two stack floats are stored with MOVSS (bit copies).
    std::uint32_t value = 0;
    std::uint32_t extra = 0;
    std::memcpy(&value, &fraction, sizeof(value));
    std::memcpy(&extra, &left, sizeof(extra));
    bsp::cc13_fe1::hud_panel_set_pending_value_005bca80(panel, value, extra);
    impl_->done("HudCountdown::update", 0x005bca80u);
    return true;
}

bool GameHudHost::clear_countdown_panel() {
    std::uint8_t* const panel = hud_minimap_block(*impl_);
    if (panel == nullptr) {
        impl_->record("HudCountdown::end", 0x005bcab0u);
        return false;
    }
    bsp::cc13_fe1::hud_panel_clear_pending_005bcab0(panel);
    impl_->done("HudCountdown::end", 0x005bcab0u);
    return true;
}

bool GameHudHost::query_mission_narrative_text_colour() {
    auto* const page = impl_->menu.in_game_page(0x33, "GUI_narrative");
    if (page == nullptr || !page->root) return false;
    auto* const group = bsp::find_descendant_by_name(*page->root, "MissionInfoTitle_Group");
    auto* const text = group != nullptr
        ? bsp::find_descendant_by_name(*group, "textbox_obj_Text") : nullptr;
    if (text == nullptr) return false;
    NarrativeQueryView view{{}, text};
    NarrativeTextColourBinding binding;
    bsp::frontend_seg20_cc13::narrative_query_text_005b5d20(
        &view, binding);
    impl_->done("HudNarrative::query_text", 0x005b5d20u);
    return true;
}

void GameHudHost::bind_sound_runtime(GameSoundRuntime* runtime,
    NativeStringStorage* strings) noexcept {
    g_hud_sound_runtime = runtime;
    g_hud_sound_strings = strings;
}

bool GameHudHost::pause_cinematic_sound_classes_004cd0f0(bool hidden_634, bool others_paused) {
    GameSoundRuntime* const sound = g_hud_sound_runtime;
    NativeStringStorage* const strings = g_hud_sound_strings;
    if (sound == nullptr || strings == nullptr || !sound->started()) return false;
    SoundSystemOwner& owner = sound->owner();
    // 004CD1B7/004CD212/004CD26D/004CD2C1 build each name with 0041E870 and release it
    // after the pause (the 00419CC0/00BD1510 pair); InGameGUI takes +634h itself.
    struct ClassPause {
        const char* name;
        bool paused;
    };
    const ClassPause pauses[4] = {
        {"3DEffect", others_paused},   // 004CD1D6 / 004CD1DE
        {"InGameGUI", hidden_634},     // 004CD234 / 004CD23C
        {"Warnings", others_paused},   // 004CD288 / 004CD290
        {"Sonar", others_paused},      // 004CD2DC / 004CD2E4
    };
    struct TemporaryName {
        NativeStringStorage& storage;
        NativeString value;
        TemporaryName(NativeStringStorage& s, const char* text) : storage(s) {
            value.assign_0041e870(storage, text);
        }
        ~TemporaryName() { value.release_to(storage); }
    };
    for (const ClassPause& pause : pauses) {
        std::int32_t index;
        {
            const TemporaryName name(*strings, pause.name);
            index = find_sound_class_00a7acf0(owner.levels, name.value, sound->fmod());
        }
        if (index < 0) continue;  // see the header: the image indexes before the table
        bsp::cc13_audio::set_sound_class_paused_00a7f890(owner.classes, index,
            static_cast<std::uint8_t>(pause.paused ? 1 : 0));
    }
    return true;
}

const GameHudSummary& GameHudHost::summary() const noexcept { return impl_->summary; }

void GameHudHost::build_manager_0068a990() {
    Impl& impl = *impl_;
    if (impl.summary.manager_built) return;
    // 004e0452 allocates 108h bytes and 004e046c runs the constructor; 0068cc70
    // is the Init the caller runs next. Packet cc_hud_updates reconstructed
    // both, so the executable runs them rather than recording them: the object
    // is bsp::InGameInterfaceManager and the order the 42 screens are built and
    // registered in is 0068cc70's own.
    construct_in_game_interface_0068a990(impl.manager);
    impl.done("InGameInterface::construct_manager", 0x0068a990u);

    HudInitBinding init(impl);
    init_in_game_interface_0068cc70(impl.manager, init);
    impl.done("InGameInterface::manager_init", 0x0068cc70u);

    impl.summary.manager_built = true;
    // manager+3Ch, the byte 00684700 sets and 006840FE and the frame's step 17
    // test. The manager's vtable slot +08h is 00684700 itself, so the native
    // raises it through the vtable; nothing in the reconstructed load reaches
    // that call, and Ghidra reports only three direct callers of 00684700, none
    // of them this manager. The executable sets the byte so the request Init
    // pushed can be serviced and step 17 can run, and says so. The other half
    // of 00684700, deactivating every other registered manager, has already
    // happened: the load destroyed the main-menu manager through 00686c90.
    impl.manager.base.active = true;
    impl.record("InGameInterface::activate_manager", 0x00684700u);
    impl.log.notef("in-mission HUD manager: %zu of %zu screens registered through "
        "bsp::init_in_game_interface_0068cc70 (%zu leaf constructors called), %zu of "
        "%zu pages loaded, %zu of %zu named widgets bound; INTF_SCENE3D (%02Xh) pushed by "
        "Init at 0068d73a", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.screen_constructors,
        impl.summary.pages_loaded, impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.pushed_interface_id));
}

void GameHudHost::apply_pending_interface_0068aca0() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    // A pending member payload is also a weak reference (+38h in the
    // native request record). A member that died after the push must not
    // be restored from its still-allocated host slot on the next frame.
    if (impl.pending_explicit_payload && impl.release_destroyed_payload(impl.pending_payload))
        impl.record_pending_payload_38 = SIZE_MAX;
    bool has_payload = false;
    const std::size_t controlled = impl.units != nullptr && impl.units->controlled_bound()
        ? impl.units->controlled_index() : SIZE_MAX;
    bsp::HudRootPayload payload{controlled, bsp::HudRootPayloadKind::Unit};
    if (controlled != SIZE_MAX && impl.is_squadron_slot(controlled))
        payload.kind = bsp::HudRootPayloadKind::Squadron;
    int requested = kInterfaceScene3d;
    if (!impl.summary.interface_applied) {
        has_payload = false;
    } else if (impl.pending_interface_id != 0) {
        // Packet cc9_movie_interface_and_reseed: a non-20h push (005CD1CE's 2Ch
        // with [00E188D8]), serviced through the same 006840f0 / 00684600 path.
        requested = impl.pending_interface_id;
        impl.pending_interface_id = 0;
        if (impl.pending_explicit_payload) {
            payload = impl.pending_payload;
            impl.pending_explicit_payload = false;
            has_payload = impl.units != nullptr && payload.unit < impl.units->count();
        } else {
            has_payload = impl.units != nullptr && impl.units->controlled_bound();
        }
    } else if (impl.unit_request_pending && !impl.unit_request_applied) {
        // Milestone 2k: the second request, the one that carries the controlled
        // unit. Serviced through the same 006840f0 / 00684600 path.
        has_payload = true;
        impl.unit_request_applied = true;
    } else {
        return;
    }
    // 006840f0 services the pending record and 00684600 hands the id to the
    // manager's own virtual +10h.
    impl.record("InGameInterface::service_pending_request", 0x006840f0u);
    HudUnitQuery query(impl);
    // 00647080 emits 22h/23h with the actual first member. Those arms do
    // not run the 20h classifier; preserve their identity for 00646040.
    query.use_payload(has_payload ? payload : bsp::HudRootPayload{});
    HudInterfaceBinding binding(impl, query);
    const bool accepted = apply_in_game_interface_0068aca0(impl.manager, binding,
        requested, has_payload);
    impl.done("InGameInterface::apply_pending_interface", 0x0068aca0u);
    if (!has_payload) impl.summary.interface_applied = accepted;
    if (has_payload) impl.unit_interface_id = impl.summary.applied_interface_id;

    // Which of the level-1 screens hold a page, for the run's own report.
    impl.summary.level1_pages.clear();
    for (GameHudScreenRecord& record : impl.summary.screens) record.in_level1_set = false;
    for (const int id : impl.summary.level1_screen_ids) {
        for (GameHudScreenRecord& record : impl.summary.screens) {
            if (record.registry_slot != id) continue;
            record.in_level1_set = true;
            for (const std::string& page : record.pages) {
                impl.summary.level1_pages.push_back(page);
            }
        }
    }

    std::string screens;
    for (const int id : impl.summary.level1_screen_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        screens += text;
    }
    std::string contexts;
    for (const int id : impl.summary.level1_context_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        contexts += text;
    }
    std::string pages;
    for (const std::string& page : impl.summary.level1_pages) {
        if (!pages.empty()) pages += ' ';
        pages += page;
    }
    const char* requested_label;
    requested_label = requested == kInterfaceScene3d ? "INTF_SCENE3D"
            : requested == kInterfacePlane ? "INTF_PLANE"
            : requested == kInterfacePlaneBomber ? "INTF_PLANEBOMBER"
            : "request 2Ch (movie camera new)";
    impl.log.notef("in-mission level-1 set for %s (%s payload, single player) "
        "applied as %02Xh: screens %s| contexts %s| pages %s",
        requested_label,
        has_payload ? "controlled unit" : "null",
        static_cast<unsigned>(impl.summary.applied_interface_id), screens.c_str(),
        contexts.c_str(), pages.c_str());
    if (has_payload && requested == kInterfaceScene3d) {
        impl.log.note("the 20h arm classified the controlled unit through the recovered "
            "IsKindOf chain and re-entered its own virtual +10h, which is what raises the "
            "world markers screen (4Dh) and keeps the minimap (35h): both have a "
            "reconstructed update virtual and the pump 004f8830 now calls them");
    }
}

bool GameHudHost::Impl::camera_target_view(bsp::ShipCaptainTargetView& out) {
    if (units == nullptr) return false;
    float right[3], up[3], forward[3], translation[3];
    if (!units->unit_pose(camera_unit, right, up, forward, translation)) return false;
    {
        namespace seg2 = bsp::profile_seg2_cc13;
        struct Copy final : seg2::MatrixCopyHost {
            void matrix_copy_004134f0(seg2::Raw destination, const seg2::Mat16& source) override {
                bsp::copy_native_camera_matrix_004134f0(destination.p, nullptr, source.m);
            }
        } copy;
        // bsp.py show 0042B680: assign invokes 004134F0 once and returns
        // its destination. Reuse that body for the live target-view copy.
        // The basis/translation are this controlled unit's current pose;
        // homogeneous 0/1 cells are the same affine layout as the old arm.
        // No copied diagnostic matrix: the destination is the camera input
        // consumed by bind_ship_captain_0064da40 / update_ship_captain_00432ed0.
        const seg2::Mat16 source{{right[0], right[1], right[2], 0.0f,
            up[0], up[1], up[2], 0.0f, forward[0], forward[1], forward[2], 0.0f,
            translation[0], translation[1], translation[2], 1.0f}};
        seg2::matrix_assign_0042b680(
            {reinterpret_cast<std::uint8_t*>(out.world.data())}, source, copy);
    }
    const GameUnitRow* row = units->unit_row(camera_unit);
    out.throttle = row != nullptr ? row->throttle : 0.0f;        // unit+980h
    out.rudder = row != nullptr ? row->ordered_rudder : 0.0f;    // unit+984h
    out.gate_5d = units->unit_flag_005d(camera_unit);
    out.min_height = camera_class.min_height;
    out.distance_front = camera_class.distance_front;
    out.distance_side = camera_class.distance_side;
    out.distance_vertical = camera_class.distance_vertical;
    out.length = camera_class_length;
    return true;
}

namespace {
// bsp::HudWeaponGroupLayoutHost over the two pages screen 2Eh registers.
class WeaponGroupLayoutBinding final : public bsp::HudWeaponGroupLayoutHost {
public:
    explicit WeaponGroupLayoutBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::uintptr_t page_58() override { return page("GUI_cross_ship"); }
    std::uintptr_t page_dc() override { return page("GUI_cross_gunstate"); }
    std::uintptr_t find_child(std::uintptr_t parent, const char* name) override {
        if (parent == 0) return 0;
        return reinterpret_cast<std::uintptr_t>(bsp::find_descendant_by_name(
            *reinterpret_cast<GuiLayoutWidget*>(parent), name));
    }
    void place_f2(std::uintptr_t, std::uintptr_t, int) override {
        owner_.record("HudWeaponGroupScreen::place_f2", 0x00546da2u);
    }
    void set_visible(std::uintptr_t widget, bool visible) override {
        if (widget == 0) return;
        owner_.menu.frontend().set_widget_visible(
            *reinterpret_cast<GuiLayoutWidget*>(widget), visible);
    }

private:
    std::uintptr_t page(const char* name) {
        GuiLayoutPage* p = owner_.menu.in_game_page(0x2e, name);
        return p != nullptr && p->root ? reinterpret_cast<std::uintptr_t>(p->root.get()) : 0;
    }
    GameHudHost::Impl& owner_;
};

// bsp::HudWeaponGroupScreenHost over this process's HUD (packet
// cc9_screen_2eh_group). Screen 2Eh's layout 00546A20 is not run, so its
// widgets are records; units are index + 1.
class WeaponGroupScreenBinding final : public bsp::HudWeaponGroupScreenHost {
public:
    explicit WeaponGroupScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    bool unit_is_kind_of(std::size_t unit, int kind) override {
        return unit != 0 && owner_.units != nullptr && owner_.units->unit_is_kind_of(unit - 1, kind);
    }
    bool local_player_role(std::size_t unit, int role) override {
        // 00927F30(unit, role): the holder at unit+1ACh is the local player's
        // slot 0, as for screen 46h (section 21).
        std::int32_t holder = -1;
        return unit != 0 && owner_.units != nullptr
            && owner_.units->unit_current_role_slot(unit - 1, role, holder) && holder == 0;
    }
    bool group_available_009542b0(std::size_t unit, int group) override {
        if constexpr (kHudGunnerRoleTakeBound) {
            return group_available_bound(unit, group);
        } else {
            // SUBSTITUTION: "not available". The image answers from the
            // unit's permission word (+194h/+198h/+19Ch/+1A4h against
            // PLAYER_ANY or the local slot) and its gun list at unit+48h, and
            // a yes makes the bind's cycle take that group's gunner roles
            // (00545410 -> 0077C470). Section 27.
            static_cast<void>(unit);
            static_cast<void>(group);
            owner_.record("HudWeaponGroupScreen::group_available", 0x009542b0u);
            return false;
        }
    }
    void role_request_0077c470(std::size_t unit, std::uint32_t mask, bool take) override {
        if constexpr (kHudGunnerRoleTakeBound) {
            if (unit != 0 && owner_.units != nullptr) {
                owner_.units->role_request_0077c470(unit - 1, mask, take);
                owner_.done("HudWeaponGroupScreen::role_request", 0x0077c470u);
                return;
            }
        }
        static_cast<void>(unit);
        static_cast<void>(mask);
        static_cast<void>(take);
        owner_.record("HudWeaponGroupScreen::role_request", 0x0077c470u);
    }
    // 009542B0(unit, group) for groups 2..5 (section 31).
    bool group_available_bound(std::size_t unit, int group) {
        if (unit == 0 || owner_.units == nullptr) return false;
        const std::size_t index = unit - 1;
        // 0059BBD0(unit, role, slot): the permission word is 9 or the local
        // slot game+18ECh (0 here).
        std::uint32_t permission_mask = 0;
        permission_mask = owner_.units->unit_role_permission_mask(index, 0);
        auto open = [&](int role) {
            return (permission_mask & (1u << role)) != 0;
        };
        bool permitted = false;
        switch (group) {
        case 2: permitted = open(3) || open(2); break;   // +194h, else 0059BBD0(2)
        case 3: permitted = open(4); break;              // +198h
        case 4: permitted = open(5); break;              // +19Ch
        case 5: permitted = open(7); break;              // +1A4h
        default: return false;
        }
        owner_.done("HudWeaponGroupScreen::group_available", 0x009542b0u);
        if (!permitted) return false;
        if (group != 5 && owner_.units->unit_is_kind_of(index, 8)) {
            // A kind-8 unit's depth tests (the pose y against 00CE3D50, and
            // BSP_Submarine_IsShallowEnoughToEngage for group 4): not held.
            owner_.record("HudWeaponGroupScreen::submarine_depth", 0x009542b0u);
            return false;
        }
        const GameGunneryHost* gunnery = owner_.units->gunnery();
        if (gunnery == nullptr) {
            owner_.record("HudWeaponGroupScreen::gun_list", 0x009542b0u);
            return false;
        }
        // The kind-20h children on unit+48h; the weapon Function is
        // [gun+3F4h]+80h, which the gunnery host keeps as the category.
        for (const GameGunRow& gun : gunnery->guns()) {
            if (gun.unit_index != index) continue;
            const int f = gun.category;
            switch (group) {
            case 2: if (f == 1 || f == 5 || f == 6) return true; break;   // 00728A90
            case 3: if (f == 2 || f == 3 || f == 4 || f == 6) return true; break;  // 005459B0
            case 4: if (f == 7) return true; break;
            case 5: if (f == 8 || f == 9) return true; break;
            default: break;
            }
        }
        return false;
    }
    std::vector<std::size_t> unit_records_778(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::unit_records_778", 0x00549337u);
        return {};
    }
    void clear_child_flags_005470c0(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::clear_child_flags", 0x005470e7u);
    }
    void reset_widgets_005464e0() override {
        owner_.record("HudWeaponGroupScreen::reset_widgets", 0x005464e0u);
    }
    bool player_unit_is_lst_rocket() override {
        owner_.record("HudWeaponGroupScreen::player_unit_name", 0x005493f9u);
        return false;
    }
    void lvlaa_hint_00548360() override {
        // 00548378: [00E188D8] must be kind 18h (a plane squadron); a ship
        // ends the routine here whatever the applied byte +5h holds.
        if (owner_.units != nullptr && owner_.units->controlled_bound()
            && !owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 0x18)) {
            return;
        }
        owner_.record("HudWeaponGroupScreen::lvlaa_hint", 0x00548360u);
    }
    bool player_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    void one_shot_108_0054856b() override {
        owner_.record("HudWeaponGroupScreen::one_shot_108", 0x0054856bu);
    }
    void one_shot_109_0054861b(float pitch_10c, float yaw_110) override {
        static_cast<void>(pitch_10c);
        static_cast<void>(yaw_110);
        owner_.record("HudWeaponGroupScreen::one_shot_109", 0x0054861bu);
    }
    void hide_widgets_005452f0() override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            bsp::HudWeaponGroupScreenState& s = layout();
            WeaponGroupLayoutBinding gui(owner_);
            for (auto& row : s.cells) {
                for (std::uintptr_t cell : row) gui.set_visible(cell, false);   // 005452FA
            }
            gui.set_visible(s.disable_c0, false);                     // 0054531A..
            gui.set_visible(s.aim_c4, false);
            gui.set_visible(s.aim_c8, false);
            gui.set_visible(s.aim_cc, false);
            owner_.done("HudWeaponGroupScreen::hide_widgets", 0x005452f0u);
        } else {
            owner_.record("HudWeaponGroupScreen::hide_widgets", 0x005452f0u);
        }
    }
    // 00546A20 has run at registration in the image; here it runs once,
    // before the first widget use (labelled).
    bsp::HudWeaponGroupScreenState& layout() {
        bsp::HudWeaponGroupScreenState& s = owner_.weapon_group;
        if (!s.layout_done) {
            WeaponGroupLayoutBinding gui(owner_);
            bsp::weapon_group_layout_00546a20(s, gui);
            owner_.done("HudWeaponGroupScreen::layout", 0x00546a20u);
        }
        return s;
    }
    void group_sight_00548300(int group) override {
        static_cast<void>(group);
        owner_.record("HudWeaponGroupScreen::group_sight", 0x00548300u);
    }
    void rocket_widgets_00548716() override {
        owner_.record("HudWeaponGroupScreen::rocket_widgets", 0x00548716u);
    }
    void widget_98_005486c7(bool flag_d5) override {
        static_cast<void>(flag_d5);
        owner_.record("HudWeaponGroupScreen::widget_98", 0x005486c7u);
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool input_held(int action) override { return owner_.menu.input_action_held(action); }
    bool input_axis_active_004d9480(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_axis_active", 0x004d9480u);
        return false;
    }
    float input_axis_value_004c5070(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_axis_value", 0x004c5070u);
        return 0.0f;
    }
    bool input_in_set_00547250(int action) override {
        static_cast<void>(action);
        owner_.record("HudWeaponGroupScreen::input_in_set", 0x00547250u);
        return false;
    }
    float clock_f876a4() override {
        owner_.record("HudWeaponGroupScreen::clock", 0x00548808u);
        return 0.0f;
    }
    bool lock_branch_00548b06(bsp::HudWeaponGroupScreenState&) override {
        owner_.record("HudWeaponGroupScreen::lock_branch", 0x00548b06u);
        return false;
    }
    bool torpedo_branch_00548cb6(bsp::HudWeaponGroupScreenState&) override {
        owner_.record("HudWeaponGroupScreen::torpedo_branch", 0x00548cb6u);
        return false;
    }
    std::size_t hud_target_00548856() override {
        if constexpr (kHudWeaponGroupTargetBound && kHudUnitPickScreenBound) {
            // The manager's +CCh is screen 29h; its +4Ch is the unit pick.
            owner_.done("HudWeaponGroupScreen::hud_target", 0x00548856u);
            return owner_.unit_pick.pick_4c;
        } else {
            owner_.record("HudWeaponGroupScreen::hud_target", 0x00548856u);
            return 0;
        }
    }
    float gunner_distance_00548ea9(std::size_t unit, std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548EA9..00548ECC: 00427EB0 on both (pose +FCh), 00427C90,
            // then 00427E30, the squared length.
            float r[3], u[3], f[3], up[3], tp[3];
            if (unit == 0 || target == 0 || owner_.units == nullptr
                || !owner_.units->unit_pose(unit - 1, r, u, f, up)
                || !owner_.units->unit_pose(target - 1, r, u, f, tp)) {
                return 0.0f;
            }
            const float d[3] = {up[0] - tp[0], up[1] - tp[1], up[2] - tp[2]};
            owner_.done("HudWeaponGroupScreen::gunner_distance", 0x00548ea9u);
            return d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
        } else {
            owner_.record("HudWeaponGroupScreen::gunner_distance", 0x00548ea9u);
            return 0.0f;
        }
    }
    float gunner_range_49c(std::size_t unit) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548ED0: [unit+49Ch], which 00953B80 writes.
            GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (unit == 0 || gunnery == nullptr) return 0.0f;
            owner_.done("HudWeaponGroupScreen::gunner_range", 0x00548ed0u);
            return gunnery->unit_gun_range_sq_049c(unit - 1);
        } else {
            owner_.record("HudWeaponGroupScreen::gunner_range", 0x00548ed0u);
            return 0.0f;
        }
    }
    bool target_virtual_10c(std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // vtable[10Ch]: 0074E1A0 on the plane classes, the
            // cached speed [unit+B1Ch] (007B8E60) above 3.0 (00CE3854,
            // FCOMIP/JBE); 0042BB40 (false) on every ship, submarine and the
            // dummy kamikaze. The cache is the magnitude 007D8020 stores of the
            // flight controller's world velocity.
            if (target == 0 || owner_.units == nullptr
                || !owner_.units->unit_is_kind_of(target - 1, bsp::kUnitGunneryKindPlaneBase)) {
                return false;
            }
            float v[3];
            if (!owner_.units->unit_plane_world_velocity(target - 1, v)) return false;
            const float speed = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            owner_.done("HudWeaponGroupScreen::target_virtual_10c", 0x0074e1a0u);
            return speed > 3.0f;
        } else {
            owner_.record("HudWeaponGroupScreen::target_virtual_10c", 0x00548923u);
            return false;
        }
    }
    int relation_00803ce0(std::size_t, std::size_t) override {
        owner_.record("HudWeaponGroupScreen::relation", 0x00803ce0u);
        return 0;
    }
    bool same_side_54(std::size_t target, std::size_t first) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548913..0054891F: [target+54h] against the screen unit's +54h.
            if (target == 0 || first == 0 || owner_.units == nullptr) return true;
            owner_.done("HudWeaponGroupScreen::same_side", 0x00548919u);
            return owner_.units->unit_side_0054(target - 1)
                == owner_.units->unit_side_0054(first - 1);
        } else {
            owner_.record("HudWeaponGroupScreen::same_side", 0x00548919u);
            return true;
        }
    }
    std::uint16_t target_id_174(std::size_t target) override {
        if constexpr (kPlayerGunSeatTargetInterceptBound) {
            // 00548934 MOVZX [target+174h]. SUBSTITUTION: the host has no
            // entity handle table, so the id is the 1-based unit index that
            // apply_gun_aim_message resolves in place of 00521E30.
            owner_.done("HudWeaponGroupScreen::target_id", 0x00548934u);
            return static_cast<std::uint16_t>(target);
        } else {
            owner_.record("HudWeaponGroupScreen::target_id", 0x00548934u);
            return 0;
        }
    }
    void target_debug_00548942(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::target_debug", 0x00548942u);
    }
    bool row_widget_present(int row, int column) override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            owner_.done("HudWeaponGroupScreen::row_widget", 0x005488d5u);
            return row >= 0 && row < 5 && column >= 0 && column < 5
                && layout().cells[row][column] != 0;
        } else {
            owner_.record("HudWeaponGroupScreen::row_widget", 0x005488d5u);
            return false;
        }
    }
    void show_row_widget(int row, int column, bool shown_d4) override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            if (row >= 0 && row < 5 && column >= 0 && column < 5) {
                WeaponGroupLayoutBinding gui(owner_);
                gui.set_visible(layout().cells[row][column], shown_d4);
            }
            owner_.done("HudWeaponGroupScreen::show_row_widget", 0x005488f9u);
        } else {
            owner_.record("HudWeaponGroupScreen::show_row_widget", 0x005488f9u);
        }
    }
    void build_fire_message_00954a10(int group, bool pressed, bool held, bool has_target,
                                     std::uint16_t target_id) override {
        if constexpr (kPlayerGunSeatBound) {
            // 00548839..005489DA: the mover's world matrix (0042D7E0) forward
            // row and position (00427EB0); the Operator node carries the
            // mover's published pose.
            bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
            if (node.ready) {
                if ((node.state.transform.valid_flags & 2u) == 0) {
                    bsp::refresh_camera_world_00b6db70(node.state.transform);
                }
                const bsp::CameraMatrix& world = node.state.transform.world;
                GunAimMessage79& m = owner_.pending_gun_message;
                m.group = group;
                for (int i = 0; i < 3; ++i) {
                    m.forward[i] = world[8 + i];
                    m.camera[i] = world[12 + i];
                }
                m.held_34 = held;
                m.pressed_35 = pressed;
                m.has_target_36 = has_target;
                m.target_38 = target_id;
                owner_.pending_gun_message_valid = true;
                owner_.done("HudWeaponGroupScreen::fire_message", 0x00954a10u);
                return;
            }
            owner_.pending_gun_message_valid = false;
        }
        static_cast<void>(group);
        static_cast<void>(pressed);
        static_cast<void>(held);
        static_cast<void>(has_target);
        static_cast<void>(target_id);
        owner_.record("HudWeaponGroupScreen::fire_message", 0x00954a10u);
    }
    // cc15-smA: count what the 79h copy changed, field by field, against the
    // posted value and the angles the OFF delivery derives from `forward`.
    void note_gun_aim_round_trip(const GunAimMessage79& m, const GunAimMessage79& q) {
        ++owner_.gun_aim_round_trips;
        const float fy = std::max(-1.0f, std::min(1.0f, m.forward[1]));
        const float pitch = std::max(-1.57f, std::min(1.57f, std::asin(fy)));
        const float yaw = std::atan2(m.forward[0], m.forward[2]);
        const double dy = std::fabs(static_cast<double>(q.yaw_2c) - yaw);
        const double dp = std::fabs(static_cast<double>(q.pitch_30) - pitch);
        if (q.yaw_2c != yaw) ++owner_.gun_aim_changed[0];
        if (q.pitch_30 != pitch) ++owner_.gun_aim_changed[1];
        if (std::memcmp(q.camera, m.camera, sizeof q.camera) != 0) ++owner_.gun_aim_changed[2];
        if (q.held_34 != m.held_34 || q.pressed_35 != m.pressed_35
            || q.has_target_36 != m.has_target_36) {
            ++owner_.gun_aim_changed[3];
        }
        if (q.group != m.group) ++owner_.gun_aim_changed[4];
        if (q.target_38 != m.target_38) ++owner_.gun_aim_changed[5];
        if (dy > owner_.gun_aim_max_yaw_delta) owner_.gun_aim_max_yaw_delta = dy;
        if (dp > owner_.gun_aim_max_pitch_delta) owner_.gun_aim_max_pitch_delta = dp;
    }
    void route_fire_message_0077c2a0(std::size_t unit) override {
        if constexpr (kPlayerGunSeatBound) {
            GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (unit != 0 && gunnery != nullptr && owner_.pending_gun_message_valid) {
                if constexpr (bsp::kGunAimMessageCc15Bound) {
                    // 00548A1F: 0077C2A0(flags 3) takes the local bit, and
                    // 0076E520's copy is what 00959C20 reads.
                    const GunAimMessage79& m = owner_.pending_gun_message;
                    const GunAimMessage79 queued = owner_.session_messages.queue_gun_aim_79(m);
                    note_gun_aim_round_trip(m, queued);
                    gunnery->apply_gun_aim_message_00959c20(unit - 1, queued);
                } else {
                    gunnery->apply_gun_aim_message_00959c20(unit - 1, owner_.pending_gun_message);
                }
                owner_.done("HudWeaponGroupScreen::route_fire_message", 0x0077c2a0u);
                return;
            }
        }
        static_cast<void>(unit);
        owner_.record("HudWeaponGroupScreen::route_fire_message", 0x0077c2a0u);
    }
    bool camera_busy_00518f30() override {
        owner_.record("HudWeaponGroupScreen::camera_busy", 0x00518f30u);
        return false;
    }
    bool depth_charge_child_00549061(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::depth_charge_child", 0x00549061u);
        return false;
    }
    void hold_camera_005490a6(std::size_t) override {
        owner_.record("HudWeaponGroupScreen::hold_camera", 0x005490a6u);
    }
    int input_mode_f8a0c4() override {
        owner_.record("HudWeaponGroupScreen::input_mode", 0x0054917du);
        return 0;
    }
    void show_widget_c0_005491c2() override {
        if constexpr (kHudWeaponGroupLayoutBound) {
            WeaponGroupLayoutBinding gui(owner_);
            gui.set_visible(layout().disable_c0, true);
            owner_.done("HudWeaponGroupScreen::widget_c0", 0x005491c2u);
        } else {
            owner_.record("HudWeaponGroupScreen::widget_c0", 0x005491c2u);
        }
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::Impl::bind_mission_camera_0064da40() {
    if (units == nullptr || !units->controlled_bound()) return;
    const std::size_t index = units->controlled_index();
    const GameUnitRow* row = units->unit_row(index);
    // 0064DA64: screen 2Eh's 005470C0 before anything else.
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(*this);
        bsp::weapon_group_screen_release_005470c0(weapon_group, group_host);
    }
    // 0064DA69..0064DA77: with +1Ch set, 0077C470(+1Ch, 1FEh, 0) releases the
    // previous unit's roles 1..8 before 0064DAA3 stores the new one (the caller
    // stores ship_view_unit after this returns). Packet cc13-relgaps gap 3. The
    // image's +1Ch is cleared by its observer when that entity is deleted; host
    // slots are never deleted in a mission and ship_view_has_unit lives until the
    // mission reset, so the guard is the slot bound, as at the 0064A490 exit.
    if constexpr (kShipViewPreviousUnitReleaseCc13Bound) {
        if (ship_view_has_unit && ship_view_unit < units->count()) {
            units->role_request_0077c470(ship_view_unit, 0x1FEu, false);
            done("HudShipView::release_previous_roles", 0x0077c470u);
        }
    }
    // The settings block (00424C40, ShipGlobals["ShipCamera"]) and the class
    // camera keys (00831E0D) come from the live Lua state.
    bool settings_read = false;
    bool class_read = false;
    bsp::ShipClassCameraInputs inputs{};
    inputs.captain_camera_global = 10.0f;   // 00CE38B8
    if (lua != nullptr) {
        settings_read = lua->read_ship_camera_settings_0083b5e0(camera_settings);
        if (row != nullptr) class_read = lua->read_ship_class_camera_00831e0d(row->type_id, inputs);
    }
    if (!settings_read) record("MissionCamera::ship_camera_settings", 0x0083b5e0u);
    if (!class_read) record("MissionCamera::class_camera_keys", 0x00831e0du);
    if (lua != nullptr) {
        camera_pipe_sight_read = lua->read_pipe_sight_params_0083b5e0(
            camera_pipe_sight_enabled, camera_pipe_sight_zoom_rate);
    }
    if (!camera_pipe_sight_read) record("MissionCamera::pipe_sight_params", 0x0083b5e0u);
    camera_class = bsp::ship_class_camera_00831e0d(inputs);
    camera_class_length = inputs.base_length;
    // Packet cc9_movie_camera_mover_bind: 004BC410 at 0064DC9C installs the
    // ShipCaptain and destroys the outgoing mover through 00926D90; when that
    // is the movie camera, its observer clears screen 37h's +1Ch.
    if (movie_active) destroy_movie_camera("0064DA40 installed the ShipCaptain (004BC410 at 0064DC9C)");
    if (!camera_bound) {
        bsp::construct_ship_captain_0064b650(camera);
        if constexpr (bsp::kShipCaptainCameraDrawsCc13Bound) {
            // The three 00BD2F10 phase draws of 00432750 (stream 0, ECX = 0).
            GameGunneryHost* draw_host = units != nullptr ? units->gunnery() : nullptr;
            if (draw_host != nullptr) {
                // SHARED-STREAM MOVER: the host's stream-0 generator, in draw order.
                // FFFFFFh is the index no unit has, as the movie-camera draw below uses,
                // so under BSP_GUNNERY_RNG_STREAMS=1 no unit's keyed draws move.
                float draws[3];
                for (float& d : draws) {
                    d = draw_host->death_mode_draw_00bd2f10(0, 0xFFFFFFu, 0.0f,
                        bsp::kShipCaptainPhaseHigh00ce3d9c);
                }
                bsp::seed_ship_captain_phases_00432938(camera, draws);
                done("MissionCamera::phase_draws", 0x00432938u);
            } else {
                for (int i = 0; i < 3; ++i) record("MissionCamera::phase_draw", 0x00bd2f10u);
            }
        } else {
        // The three 00BD2F10 phase draws of 00432750, not taken (see
        // bsp::construct_ship_captain_0064b650).
        for (int i = 0; i < 3; ++i) record("MissionCamera::phase_draw", 0x00bd2f10u);
        }
    }
    camera_unit = index;
    bsp::ShipCaptainTargetView view{};
    camera_target_view(view);
    bsp::bind_ship_captain_0064da40(camera, view, camera_settings);
    camera_bound = true;
    // 0064DCC4: 00549260(screen 2Eh, 46h's +1Ch the unit, 46h's +20h the mover).
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(*this);
        bsp::weapon_group_screen_bind_00549260(weapon_group, group_host, index + 1, camera_bound);
        done("HudWeaponGroupScreen::bind", 0x00549260u);
    }
    done("MissionCamera::bind_ship_view", 0x0064da40u);
    log.notef("mission camera: ShipCaptain mover bound to \"%s\" (0064da40): ShipCamera "
        "ZoomOffset %.3f LengthMult %.3f angles [%.1f, %.1f] deg; class CameraDistance front "
        "%.1f side %.1f vertical %.1f, CameraMinHeight %.1f, Length %.1f; yaw %.4f pitch %.4f",
        row != nullptr ? row->name.c_str() : "?",
        static_cast<double>(camera_settings.zoom_offset),
        static_cast<double>(camera_settings.length_mult),
        static_cast<double>(camera_settings.min_angle_deg),
        static_cast<double>(camera_settings.max_angle_deg),
        static_cast<double>(camera_class.distance_front),
        static_cast<double>(camera_class.distance_side),
        static_cast<double>(camera_class.distance_vertical),
        static_cast<double>(camera_class.min_height),
        static_cast<double>(camera_class_length),
        static_cast<double>(camera.yaw_384), static_cast<double>(camera.pitch_388));
}

namespace {
class CameraOcean final : public bsp::MissionCameraOcean {
public:
    explicit CameraOcean(GameHostLog& log) : log_(log) {}
    bool present() override { return true; }   // [game+19F0h], the ocean owner
    float water_height(float x, float z) override;
private:
    GameHostLog& log_;
};
}  // namespace

void GameHudHost::Impl::step_mission_camera(float seconds) {
    if (!kMissionCameraBound) return;
    if (!movie_active && !camera_bound) return;
    // The mover is a world entity ticked by the world update before the
    // interface runs. SUBSTITUTION: this host ticks it once per in-game
    // interface frame, at the first screen update of that frame, with the
    // screen's own delta.
    if (camera_last_frame == summary.update_frames) return;
    camera_last_frame = summary.update_frames;
    if (movie_active) {
        // Packet cc9_movie_camera_mover_bind: the installed mover is the movie
        // camera (004BC410 at 005CC2A9 replaced the ShipCaptain).
        step_movie_camera(seconds);
        return;
    }
    bsp::ShipCaptainTargetView view{};
    if (!camera_target_view(view)) return;
    CameraOcean ocean(log);
    bsp::CameraMatrix16 world{};
    const bool published =
        bsp::update_ship_captain_00432ed0(camera, view, camera_settings, ocean, seconds, world);
    done("MissionCamera::update", 0x00432ed0u);
    // 0042F4DC, once per probe: the ray against the target's collision.
    for (int i = 0; i < 5; ++i) record("MissionCamera::collision_ray", 0x0098b370u);
    if (!published) return;
    done("MissionCamera::publish_pose", 0x004329d0u);
    publish_ship_view_camera(world);
}

namespace {
// Review repair, chain 671. These are field views, not a second 004DC940 body:
// bsp.py show 004dc940 --asm, 004DC947..004DC9D9. +634h is borrowed from the
// mission frame; +614h/+61Ch/+1FE4h come from the process's live SessionContext.
// [00E198C4]+A8h is screen 34h and its +20h is hints_pause_Group (0054D980),
// whose +38h reads its own visibility. The hint tracker owns the active model.
class HudViewFovBinding final : public bsp::cc13_seg10::ViewFovHost {
    using Object = bsp::cc13_seg10::Object;
    template<class T> static T& at(Object p, std::uint32_t offset) {
        return bsp::cc13_seg10::native_at<T>(p, offset);
    }
public:
    explicit HudViewFovBinding(GameHudHost::Impl& owner) : owner_(owner) {
        const bsp::SessionContext& session = bsp::session_context();
        at<std::int32_t>(game_.data(), 0x614) = session.game_mode_614;
        at<std::uint8_t>(game_.data(), 0x61C) = session.forced_61c;
        at<std::uint8_t>(game_.data(), 0x634) =
            owner_.cinematic_hidden_634 != nullptr && *owner_.cinematic_hidden_634;
        at<std::int32_t>(game_.data(), 0x1FE4) = session.session_word_1fe4;
        at<std::int32_t>(game_.data(), 0x1A04) = owner_.view_fov_index_1a04;
        at<Object>(game_.data(), 0x19FC) = reinterpret_cast<Object>(&owner_.camera_projection);
        at<Object>(manager_.data(), 0xA8) = hints_.data();
        // The virtual receiver is resolved by vcall_38_bool to the actual hint
        // model's +20h handle, or the loaded frontend widget before that model exists.
        at<Object>(hints_.data(), 0x20) = hints_.data();
    }
    Object game_00e188a8() override { return game_.data(); }
    Object mission_00e198c4() override {
        return owner_.summary.manager_global_published ? manager_.data() : nullptr;
    }
    bool vcall_38_bool(Object object) override {
        if (object != hints_.data()) throw std::logic_error("004DC940 queried another hints group");
        GameHintTrackerSlot* slot = GameHintTrackerSlot::process_slot();
        GameHintTracker* tracker = slot != nullptr ? slot->built() : nullptr;
        if (tracker != nullptr) {
            GameHintsScreen& screen = tracker->hints_screen_34();
            return screen.widget_shown_38(screen.state().pause_group_20);
        }
        GuiLayoutPage* page = owner_.menu.in_game_page(0x34, "GUI_hints");
        GuiLayoutWidget* group = page != nullptr
            ? owner_.menu.frontend().find_page_child(*page, "hints_pause_Group") : nullptr;
        if (group == nullptr) {
            owner_.record("MissionCamera::missing_hints_pause_group", 0x004dc9a1u);
            return false;
        }
        return owner_.menu.frontend().widget_own_visible(*group);
    }
    Object global_config_00432650() override {
        // 0087EBE2..0087EC2F initializes GlobalConfig+F8h from Globals.FOVs.Ship.
        // The Lua reader loads globals.lua if needed; no ship-view bind is required.
        if (!owner_.camera_fov_read && owner_.lua != nullptr)
            owner_.camera_fov_read = owner_.lua->read_global_fov_ship_0087d7b0(
                owner_.camera_fov_ship_degrees);
        if (!owner_.camera_fov_read)
            throw std::runtime_error("004DC940 requires the live Globals.FOVs.Ship setting");
        if (at<std::int32_t>(game_.data(), 0x1A04) != 1)
            throw std::logic_error("HUD FOV caller requested an unbound GlobalConfig index");
        at<float>(config_.data(), 0xF8) = bsp::global_config_fov_0087ec0f(
            owner_.camera_fov_ship_degrees, bsp::kFovDivisor00f889b4);
        return config_.data();
    }
    float fov_scale_00f889b4() override { return bsp::kFovDivisor00f889b4; }
    void camera_set_fov_00b6fbb0(Object camera, float fov) override {
        if (camera != reinterpret_cast<Object>(&owner_.camera_projection))
            throw std::logic_error("004DC940 set another Operator camera");
        bsp::set_mission_camera_fov(owner_.camera_projection, fov);
        owner_.done("MissionCamera::view_fov", 0x004dc940u);
    }
    ~HudViewFovBinding() override {
        // Also retain 004DC947's unconditional index store when a gate rejects.
        owner_.view_fov_index_1a04 = at<std::int32_t>(game_.data(), 0x1A04);
    }
private:
    GameHudHost::Impl& owner_;
    alignas(4) std::array<std::uint8_t, 0x1FE8> game_{};
    alignas(4) std::array<std::uint8_t, 0xAC> manager_{};
    alignas(4) std::array<std::uint8_t, 0x24> hints_{};
    alignas(4) std::array<std::uint8_t, 0xFC> config_{};
};
}  // namespace

void GameHudHost::Impl::set_view_fov(float scale, std::int32_t index) {
    HudViewFovBinding binding(*this);
    bsp::cc13_seg10::game_set_view_fov_004dc940(binding, binding.game_00e188a8(), scale, index);
}

void GameHudHost::Impl::publish_ship_view_camera(const bsp::CameraMatrix16& world) {
    // Screen 45h's 0064DD30 pipe-sight FOV is the unraised default. A raised
    // screen 26h has already set the same Operator FOV via 0051E6E0/004DC940.
    // Existing substitution: [00E197F4] remains zero without the unhosted
    // 0064DFDE gun-fire adds, so the pipe-sight spring has not moved it.
    const bsp::MissionCameraFovFrame frame{
        kMissionFovBound && camera_pipe_sight_read && camera_pipe_sight_enabled,
        binoculars.mode_30 == 1, camera_pipe_sight_zoom_rate, 0.0f};
    HudViewFovBinding binding(*this);
    bsp::publish_ship_view_mission_camera(world, camera_projection, frame, binding);
}

namespace {
float CameraOcean::water_height(float x, float z) {
    CameraOceanField field;
    log_.implemented("MissionCamera::ocean_height", "0078cf20");
    return bsp::ocean_water_height_0078cf20(x, z, field);
}
}  // namespace

namespace {
// The HUD whose world is attached, for hud_set_selected_unit_00647300. The
// image's route is the global manager [00E198C4] and its +40h (slot 44h).
GameHudHost*& attached_hud_for_selection() noexcept {
    static GameHudHost* hud = nullptr;
    return hud;
}
}  // namespace

bool hud_set_selected_unit_00647300(std::size_t unit, bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr) return false;
    return hud->set_selected_unit_00647300(unit);
}

bool hud_force_select_unit_006485a0() {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return false;
    hud->force_select_unit_006485a0();
    return true;
}

bool hud_arm_release_gun_map_toggle(bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr || !kHarnessReleaseGunMapBound) return false;
    hud->arm_release_gun_map_toggle();
    return true;
}

void GameHudHost::arm_release_gun_map_toggle() noexcept {
    if (kHarnessReleaseGunMapBound) impl_->release_gun_armed = true;
}

bool GameHudHost::release_gun_armed() const noexcept {
    return kHarnessReleaseGunMapBound && impl_->release_gun_armed;
}

// Packet cc13-releasegun, for cc9's plans: one line per map state change (log
// only, no switch), naming the ship-view unit whose roles 0064A490 releases.
void GameHudHost::Impl::log_map_state(const char* state) {
    const std::size_t unit = ship_view_unit;
    const GameUnitRow* row = units != nullptr && unit < units->count()
        ? units->unit_row(unit) : nullptr;
    log.notef("map %s t=%llu (hud update frame) unit=%s", state, summary.update_frames,
        row != nullptr ? row->name.c_str() : "-");
}

// Packet cc13-releasegun: 0068B470, reached from 0068C6B1 on the C1h edge
// (IC_MAP_TOGGLE in this installation's Inputs.lua).
void GameHudHost::Impl::back_out_one_overlay_level_0068b470() {
    // 0068B4A8..0068B4C6: the level-3 collapse runs only with 00E18D1C non-empty; this
    // host never fills level 3 (level3_screen_count answers 0).
    reset_binoculars_0051e8e0();                              // 0068B4CE
    if (release_gun_map_up) {
        // Packet cc13-mapback. 0068B4D3..0068B4DA: [this+54h]+4h set (the map
        // is up) takes the close arm. HARNESS INPUT, labelled: the C1h press is
        // a plan's `input 0xC1` line standing in for the player's map toggle.
        // 0068B4E7/0068B4E9: 004F85D0(0) clears the level-2 screen set.
        static const int kNone[] = {0};
        menu.publish_level2_screen_set_004f85d0(kNone, 1);
        done("InGameInterface::clear_level2_screen_set", 0x0068b4e9u);
        // 0068B4EE..0068B4F7: 004D8AE0(game, 0) clears the level-2 contexts.
        menu.publish_level2_input_contexts_004d8ae0(kNone, 1);
        done("InGameInterface::clear_level2_input_contexts", 0x0068b4f7u);
        release_gun_map_up = false;
        // 46h is no longer occluded, so 0064D610 runs 0064B870 again.
        if (units != nullptr) units->set_helm_screen_occluded_by_map(false);
        // 0068B4FF..0068B507: 0068AA40(this, 004B4B00()), the controlled
        // vehicle. Its gates (0068AA44..0068AA7A): not the pending payload
        // +38h, present, +5Ch set with +5Dh/+60h/+5Eh clear (the units host
        // answers the four together), pending id +20h not a camera mode; then
        // 0068AA7F 004CC460(20h, unit).
        const bool present = units != nullptr && units->controlled_bound();
        const std::size_t unit = present ? units->controlled_index() : 0;
        const bool is_pending_payload = present && unit == record_pending_payload_38;
        bsp::SceneRequestUnitFlags flags{};
        if (present) {
            const bool alive = units->unit_alive_and_visible(unit);
            flags.flag_5c = alive;
        }
        const bool push = bsp::scene_request_accepts_unit_0068aa40(present,
            is_pending_payload, flags, record_pending_id_20);
        if (push) {
            unit_request_pending = true;
            unit_request_applied = false;
            pending_interface_id = 0;
            note_pending_record_004cc460(kInterfaceScene3d, true);
        }
        done("InGameInterface::request_scene_for_unit", 0x0068aa40u);
        log_map_state("close");
        log.notef("releasegun: C1h closed the map (0068B4E9..0068B507, harness input): "
            "overlay level 2 -> 1, level 2 {} contexts {}; 0068AA40 %s",
            push ? "pushed 20h with the controlled unit"
                 : !present ? "skipped (no controlled unit)"
                 : is_pending_payload ? "skipped (the unit is already the pending payload +38h)"
                 : !flags.flag_5c ? "skipped (+5Ch/+5Dh/+60h/+5Eh)"
                 : "skipped (pending id +20h is a camera mode)");
        return;
    }
    // 0068AC30. 00E08880 is EnableMap, 1 in single player
    // (docs/MISSION_LOBBY_SETTINGS.md), so the sound-queue arm is not taken.
    // 0068AC50: 0068AB30 pushes 20h only on interface 31h/32h; the captain's
    // is 25h.
    record("InGameInterface::leave_unit_request_level", 0x0068ab30u);
    // 0068AC63 / 0068AC73: HUD root +1Ch to +ECh, [00E188D8] to +F0h. The HUD
    // root object is not this host's.
    record("InGameInterface::hud_root_copy_selection", 0x0068ac63u);
    // 0068AC79 / 0068AC8E, both pushed right to left and zero-terminated.
    static const int kLevel2Screens[] = {0x4C, 0x39, 0x44, 0x29, 0x49};
    static const int kLevel2Contexts[] = {0x0E, 0x14, 0x0C, 0x11};
    menu.publish_level2_screen_set_004f85d0(kLevel2Screens, 5);
    done("InGameInterface::set_level2_screen_set", 0x004f85d0u);
    menu.publish_level2_input_contexts_004d8ae0(kLevel2Contexts, 4);
    done("InGameInterface::set_level2_input_contexts", 0x004d8ae0u);
    release_gun_map_up = true;
    // Packet cc13-helmmapgate: 46h, whose update 0064D610 runs 0064B870, is now
    // occluded. The units host counts the helm calls this skips, and skips them
    // only under kHelmPrepareMapUpGateCc13Bound.
    if (units != nullptr) units->set_helm_screen_occluded_by_map(true);
    done("InGameInterface::restore_hud_screen_set", 0x0068ac30u);
    log_map_state("open");
    log.note("releasegun: C1h opened the map: level 2 {4Ch 39h 44h 29h 49h}, contexts "
        "{0Eh 14h 0Ch 11h}; 4Ch's +8h (005A8EF0) occludes level 1");
}

// Goal 2 chain 35. 0068AB80 BSP_InGameInterface_CollapseOverlays, __thiscall(this,
// char clear_level2, char run_extra_hook), RET 8, over this host's real inputs:
//  * 0068AB86..0068AB97, the level-3 vector 00E18D18 (begin 00E18D1C, end
//    00E18D20): the menu host's screen-set stack, level 3. Only the menu host
//    writes that stack (levels 1, 2 and 4), so its level 3 is empty here.
//  * 0068AB9D, [[00E198C4]+BCh]+30h, read only on the level-3 arm: this host
//    keeps no camera-screen object; HudUpdateBinding answers the same byte false
//    to the update body and drops its +0Ah store, and this takes that answer.
//  * 0068ABEF..0068ABF9, [this+54h]+4h: registry slot 4Ch's requested byte,
//    FrontEndScreen::wanted as the menu's 004F7620 recompute leaves it.
// Each collapse is followed by 0068AA40(004B4B00()), modelled as the C1h close
// models it (back_out_one_overlay_level_0068b470). The level-2 clear also ends
// the host's two mirrors of the map being up (release_gun_map_up, the helm
// occlusion), which that close keeps in step with the same clear. The extra
// hook resets the same retained binocular state as the C1h close (0051E8E0).
void GameHudHost::Impl::collapse_in_game_overlays(bool clear_level2, bool run_extra_hook,
    const char* site) {
    bsp::InGameInterfaceCollapseState state;
    state.level3_non_empty = menu.screen_set_level_size(3) != 0;
    state.camera_screen_flag_30 = false;
    state.screen_4c_wanted = menu.in_game_screen_wanted(0x4c);
    bsp::collapse_in_game_overlays_0068ab80(state, clear_level2, run_extra_hook);
    const auto request_scene_for_controlled_unit = [this]() {
        // 0068AA40(004B4B00()): the gates of back_out_one_overlay_level_0068b470.
        const bool present = units != nullptr && units->controlled_bound();
        const std::size_t unit = present ? units->controlled_index() : 0;
        const bool is_pending_payload = present && unit == record_pending_payload_38;
        bsp::SceneRequestUnitFlags flags{};
        if (present) flags.flag_5c = units->unit_alive_and_visible(unit);
        const bool push = bsp::scene_request_accepts_unit_0068aa40(present,
            is_pending_payload, flags, record_pending_id_20);
        if (push) {
            unit_request_pending = true;
            unit_request_applied = false;
            pending_interface_id = 0;
            note_pending_record_004cc460(kInterfaceScene3d, true);
        }
        done("InGameInterface::request_scene_for_unit", 0x0068aa40u);
    };
    if (state.level3_cleared) {
        // 0068ABAD 004F8670(0), 0068ABB7 004D8B70(game, 0).
        menu.clear_level3_screen_and_context_sets();
        request_scene_for_controlled_unit();
    }
    if (state.extra_hook_ran) reset_binoculars_0051e8e0();
    if (state.level2_cleared) {
        // 0068AC00 004F85D0(0), 0068AC0A 004D8AE0(game, 0).
        static const int kNone[] = {0};
        menu.publish_level2_screen_set_004f85d0(kNone, 1);
        menu.publish_level2_input_contexts_004d8ae0(kNone, 1);
        release_gun_map_up = false;
        if (units != nullptr) units->set_helm_screen_occluded_by_map(false);
        request_scene_for_controlled_unit();
        log.notef("collapse: %s cleared overlay level 2 (0068AB80, screen 4Ch wanted)", site);
    }
    done(site, 0x0068ab80u);
}

// 0064A490, screen 46h's +1Ch (vtable 00CF7978): 005470C0 with ECX =
// [[00E198C4]+50h], then, with +1Ch (the unit 0064DA40 stored) set,
// 0077C470(+1Ch, 1FEh, 0), session message 4Bh with take 0.
void GameHudHost::exit_ship_view_screen_0064a490() {
    Impl& impl = *impl_;
    if constexpr (kHudWeaponGroupScreenBound) {
        WeaponGroupScreenBinding group_host(impl);
        bsp::weapon_group_screen_release_005470c0(impl.weapon_group, group_host);
        impl.done("HudWeaponGroupScreen::release", 0x005470c0u);
    } else {
        impl.record("HudWeaponGroupScreen::release", 0x005470c0u);
    }
    const bool roles = impl.ship_view_has_unit && impl.units != nullptr
        && impl.ship_view_unit < impl.units->count();
    if (roles) {
        impl.units->role_request_0077c470(impl.ship_view_unit, 0x1FEu, false);
        impl.done("HudShipView::release_roles", 0x0077c470u);
    }
    impl.done("HudShipView::exit", 0x0064a490u);
    impl.log.notef("releasegun: screen 46h exit 0064A490 released the weapon group%s",
        roles ? " and roles 1..8 (1FEh)" : "; no ship-view unit");
}

void GameHudHost::update_map_screen_005b45c0(float seconds) {
    Impl& impl = *impl_;
    // 005B4673..005B46B3, every update while the map is up: [00E188D8]'s
    // vtable[13Ch]() unit, its role-1 holder +1B0h (+1ACh[1]); not 8 (005B468D)
    // and not AI-held by 00927F10 (005B4695) sends 0077C470(unit, 2, 0).
    // Stand-ins: "not AI-held" is holder == local slot 0, the units host's
    // labelled 00927F10 substitution (game_hosts_units.cpp:13466); vtable[13Ch]()
    // returning the unit itself is checked for a ship only, and the controlled
    // index (read live on each call) stands for it.
    std::int32_t holder = 8;
    const bool release = impl.units != nullptr && impl.units->controlled_bound()
        && impl.units->unit_current_role_slot(impl.units->controlled_index(), 1, holder)
        && holder != 8 && holder == 0;
    if (release) {
        impl.units->role_request_0077c470(impl.units->controlled_index(), 0x2u, false);
        impl.done("HudMapScreen::release_helm", 0x0077c470u);
    }
    if (impl.map_usage_scoring != nullptr) {
        // The screen pump supplies this update's raw delta. Borrow the live
        // local-slot cell; add_map_usage retains the native session-mode gate.
        impl.map_usage_scoring->add_map_usage(*impl.map_usage_local_slot, seconds);
    }
    // The rest of 005B45C0's map view and input work remains unimplemented.
    impl.record("HudMapScreen::update", 0x005b45c0u);
}

void GameHudHost::bind_map_usage_scoring(GameMissionResultHost& scoring,
    const std::size_t& local_slot) noexcept {
    impl_->map_usage_scoring = &scoring;
    impl_->map_usage_local_slot = &local_slot;
}

void GameHudHost::unbind_map_usage_scoring(GameMissionResultHost& scoring,
    const std::size_t& local_slot) noexcept {
    if (impl_->map_usage_scoring == &scoring && impl_->map_usage_local_slot == &local_slot) {
        impl_->map_usage_scoring = nullptr;
        impl_->map_usage_local_slot = nullptr;
    }
}

void GameHudHost::bind_view_fov_cinematic_state(const bool* hidden_634) noexcept {
    impl_->cinematic_hidden_634 = hidden_634;
}

void GameHudHost::unbind_view_fov_cinematic_state(const bool* hidden_634) noexcept {
    if (impl_->cinematic_hidden_634 == hidden_634) impl_->cinematic_hidden_634 = nullptr;
}

void GameHudHost::bind_score_message_participants(const bsp::SessionParticipantPools& participants,
    const std::size_t& local_slot) noexcept {
    impl_->score_participants = &participants;
    impl_->score_local_slot = &local_slot;
}

void GameHudHost::unbind_score_message_participants(const bsp::SessionParticipantPools& participants,
    const std::size_t& local_slot) noexcept {
    if (impl_->score_participants == &participants && impl_->score_local_slot == &local_slot) {
        impl_->score_participants = nullptr;
        impl_->score_local_slot = nullptr;
    }
}

void GameHudHost::attach_world_2k(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& impl = *impl_;
    impl.units = &units;
    impl.lua = &lua;
    impl.camera_fov_read = false;
    if (!impl.minimap) impl.minimap = std::make_unique<GameHudMinimapHost>(impl.log, impl.menu);
    if (!impl.markers) impl.markers = std::make_unique<GameHudMarkersHost>(impl.log, impl.menu);
    impl.minimap->attach_world(units, lua);
    impl.markers->attach_world(units);
    attached_hud_for_selection() = this;
}

void GameHudHost::detach_world_2k() noexcept {
    Impl& impl = *impl_;
    if (attached_hud_for_selection() == this) attached_hud_for_selection() = nullptr;
    impl.units = nullptr;
    impl.lua = nullptr;
    impl.camera_fov_read = false;
    impl.camera_bound = false;
    impl.camera_last_frame = ~0ull;
    impl.ship_screen = bsp::ShipScreenState{};
    impl.ship_screen_widgets.clear();
    impl.ship_screen_widgets_bound = false;
    impl.warning_screen = bsp::WarningScreenState{};
    impl.warning_widgets.clear();
    impl.warning_widgets_bound = false;
    impl.follow_screen = bsp::FollowScreen49State{};
    impl.weapon_group = bsp::HudWeaponGroupScreenState{};
    impl.ship_view_has_unit = false;
    impl.ship_view_controls = bsp::IntegratedControlsState{};
    impl.binoculars = bsp::BinocularsState{};
    bsp::clear_mission_camera();
    impl.unit_request_pending = false;
    impl.unit_request_applied = false;
    impl.unit_interface_id = 0;
    impl.movie_screen = bsp::HudMovieScreenState{};
    impl.pending_interface_id = 0;
    // detach_world releases the mission's saved interface, not the
    // screen itself. Keep its constructed vtables/containers and the
    // registry's live flag bytes across this world-owner boundary.
    impl.root_interface_view().at<std::int32_t>(0x1C) = 0;
    impl.root_interface_view().at<std::uint32_t>(0xCC) = 0;
    impl.root_interface_view().ptr(0xE4) = nullptr;
    impl.root_saved_observer_identity = nullptr;
    impl.root_saved_payload_kind = bsp::HudRootPayloadKind::Unit;
    impl.set_root_closed_value(0);
    impl.pending_explicit_payload = false;
    impl.pending_payload = {};
    impl.record_pending_id_20 = 0;
    impl.record_pending_payload_38 = static_cast<std::size_t>(-1);
    impl.movie = bsp::HudMovieCamera{};
    impl.movie_active = false;
    // The child hosts borrow units and Lua. Their default destructors release
    // source caches without invoking their borrowed owners.
    impl.markers.reset();
    impl.minimap.reset();
}

void GameHudHost::request_scene_interface_for_unit_004cc460() {
    Impl& impl = *impl_;
    if (impl.units == nullptr || !impl.units->controlled_bound()) return;
    if (impl.unit_request_pending) return;
    // The load creates the units on its `load_scene_contents` row and builds the
    // HUD manager on a later row, so the request outlives the push: it is
    // serviced by the first 004c40f0 pass that finds the manager built, right
    // after the null-payload request Init pushed. In the game the pusher is the
    // HUD root, which by definition already has a manager; that ordering is the
    // executable's and is the only part of this step that is.
    // 004cc460(20h, unit). The native pushers are the HUD root's own 00649860,
    // 006485a0 and 00647300, all of which read a HUD root object this process
    // does not own, so the push is recorded and the executable makes it.
    impl.record("InGameInterface::push_interface_request", 0x004cc460u);
    impl.unit_request_pending = true;
    impl.note_pending_record_004cc460(kInterfaceScene3d, true);
    impl.log.note("interface request 20h pushed with the controlled unit as its payload: "
        "milestone 2h applied the null-payload request Init pushes at 0068d73a, whose arm "
        "publishes only 29h 49h 44h 35h; the same arm with a unit is a classifier");
}

// 00645060's inputs for one unit, teamIndex [game+18ECh] = 0 (single player)
// and allowSpectate 1 (00647313 PUSH 1).
bsp::UnitSelectableInputs GameHudHost::Impl::selectable_inputs_00645060(std::size_t unit) {
    bsp::UnitSelectableInputs in{};
    // 00645091..006450B3: +5Ch set, +5Dh/+60h/+5Eh clear. The units host
    // answers the four together (its 0043F080 test, the same four bytes).
    const bool alive = units->unit_alive_and_visible(unit)
        || bsp::plane_squadron_live_fused_slot(unit) != nullptr;   // CC13 item A, OFF
    in.alive_5c = alive;
    // 006450B9..006450BF: [unit+54h] == [[game+18CCh]+28h]. SUBSTITUTION
    // (labelled, as game_hosts_ai.cpp does for the same read): the local
    // player's side record is party 0, and unit+54h is the authored party.
    in.team_matches_owner =
        units->unit_side_0054(unit) == bsp::game::local_player_party();  // section 125
    // Packet cc9_squadron_slot_class: the unit's own class test, which for a
    // squadron's fused slot is PlaneSquadronGen's (18h) when bound.
    in.is_kind_2 = selection_is_kind_of(unit, bsp::kUnitTraitSelectableBase);
    in.is_kind_0f = selection_is_kind_of(unit, 0x0f);
    in.is_kind_2a = selection_is_kind_of(unit, bsp::kUnitTraitRejectedA);
    in.is_kind_46 = selection_is_kind_of(unit, bsp::kUnitTraitRejectedB);
    in.is_kind_45 = selection_is_kind_of(unit, bsp::kUnitTraitRejectedC);
    if (is_squadron_slot(unit)) {
        bool seen = false;
        for (const std::size_t u : squadron_slots_seen) seen = seen || u == unit;
        if (!seen) squadron_slots_seen.push_back(unit);
    }
    // 00645110 vtable[124h]() (packet cc9_vtable124_liveness).
    in.vtable_124_allows = unit_vtable_124(unit, 0x00645110u);
    // 00645121 00927C50(unit, 0): any role word 9 or 0.
    {
        // 00927CC0 returns the same predicates as a bitmask. Consume it for
        // this any-role gate; role zero's raw word still supplies spectate_kind.
        std::int32_t word = 0;
        if (units->unit_role_permission(unit, 0, word)) in.spectate_kind = word;
        in.team_query_00927c50 = units->unit_role_permission_mask(unit, 0) != 0;
    }
    // 0064507A: in single player game+2194h is clear, so BL = 0 and the
    // spectator door is shut. SUBSTITUTION (labelled): +2194h is not modelled;
    // [team+19h] is unread and left false.
    in.spectate_allowed = false;
    in.team_is_local = false;
    return in;
}

// Packet cc9_squadron_slot_class. A slot is a squadron's when a registry
// record names it as its squadron_unit (the fused wing 0).
bool GameHudHost::Impl::is_squadron_slot(std::size_t unit) const {
    for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
        if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit) return true;
    }
    return false;
}

int GameHudHost::Impl::selection_class_id(std::size_t unit) const {
    if constexpr (kSquadronSlotClassBound) {
        if (is_squadron_slot(unit)) return 0x18;         // PlaneSquadronGen
    }
    return units->unit_class_id(unit);
}

bool GameHudHost::Impl::selection_is_kind_of(std::size_t unit, int class_id) const {
    if (unit >= units->count()) return false;
    return bsp::unit_is_kind_of(selection_class_id(unit), class_id);
}

// unit->vtable[124h]() by class (include/bsp/game_hosts_hud.hpp,
// kUnitVtable124MapBound). The four bytes are 0043F080's, which the units
// host answers as unit_alive_and_visible.
bool GameHudHost::Impl::unit_vtable_124(std::size_t unit, std::uint32_t site) {
    ++vtable124_asks;
    const bool alive = units->unit_alive_and_visible(unit);
    // The rule before this packet: the ship vtable only.
    const bool ship_rule = units->unit_is_kind_of(unit, 6) && alive;
    bool map = false;
    if (selection_is_kind_of(unit, 0x1c)) {
        // 006F5920 MCommandBuilding: +790h is clear in single player (its one
        // setter 006F292D needs effective game mode 2 or 3, and 004BCA50
        // reports 8), so the +70h == 58h arm is not reached: the four bytes.
        map = alive;
    } else if (selection_is_kind_of(unit, 0x1b)) {
        map = false;                                   // 00745A50 MLandFort
    } else if (selection_is_kind_of(unit, 5)) {
        map = alive;                                   // 006D1EF0
    } else if (selection_is_kind_of(unit, 0x18)) {
        // 007EE670 PlaneSquadronGen. +3B0h is only ever cleared (007F2DD3,
        // 007EFB69), so it reads 0. SUBSTITUTION (labelled): +361h, set by
        // 007F31A0 when the leader leaves the map with survivors, is not
        // modelled by this host and reads clear. Then the four bytes of the
        // slot-0 plane +3D0h (007EE685), false when there is none.
        const bsp::PlaneSquadronHostRecord* squadron = nullptr;
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == unit) { squadron = &r; break; }
        }
        const std::size_t leader = squadron != nullptr ? squadron->flight_leader()
                                                       : bsp::kPlaneSquadronNoUnit;
        map = leader != bsp::kPlaneSquadronNoUnit && leader < units->count()
            && units->unit_alive_and_visible(leader);
    } else {
        map = false;                                   // 00927800, the class-02 base
    }
    if (map && !ship_rule) {
        ++vtable124_map_only_asks;
        bool seen = false;
        for (const auto& entry : vtable124_map_only) {
            if (entry.first == unit) { seen = true; break; }
        }
        if (!seen) vtable124_map_only.emplace_back(unit, site);
    }
    if constexpr (kUnitVtable124MapBound) {
        done("Unit::vtable_124", site);
        return map;
    } else {
        if (!units->unit_is_kind_of(unit, 6)) {
            record("SetSelectedUnit::unit_vtable_124", 0x00645110u);
        }
        return ship_rule;
    }
}

namespace {
// 00645600's host over the units host. The unit-side broadcasts and the
// observer pair are records: no host state carries them.
class SelectControlledUnitBinding final : public bsp::SelectControlledUnitHost {
public:
    SelectControlledUnitBinding(GameHudHost::Impl& owner, std::size_t candidate, bool selectable)
        : owner_(owner), candidate_(candidate), selectable_(selectable) {}
    bool current_is_kind_6() override {
        return owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 6);
    }
    void release_unit_parts() override {
        owner_.record("SetControlledUnit::release_unit_parts", 0x00817380u);
    }
    void release_unit_nodes() override {
        // 0064562D 0080E290(current). The caller (00645603..0064562D) reaches here only
        // with a bound kind-6 controlled unit, which is still the outgoing one: the
        // setter (004C0890) runs after. kHelmReleaseGunsToRestCc13Bound.
        GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
        if (bsp::kHelmReleaseGunsToRestCc13Bound && gunnery != nullptr) {
            gunnery->aim_unit_turning_guns_to_rest_0080e290(owner_.units->controlled_index());
            owner_.done("SetControlledUnit::release_unit_nodes", 0x0080e290u);
        } else {
            owner_.record("SetControlledUnit::release_unit_nodes", 0x0080e290u);
        }
    }
    void clear_hud_slot() override {
        owner_.record("SetControlledUnit::clear_hud_root_1c", 0x00645637u);
    }
    bool candidate_is_selectable() override { return selectable_; }
    bool candidate_is_kind_1() override {
        return owner_.selection_is_kind_of(candidate_, 1);   // 0064565A
    }
    bool candidate_vtable_124() override {
        // 0064565F (packet cc9_vtable124_liveness).
        return owner_.unit_vtable_124(candidate_, 0x0064565fu);
    }
    void unregister_observer() override {
        if constexpr (kControlledUnitObserverBound) {
            // 00645699 006952A0(controlled, root+8h).
            owner_.observed_unit = SIZE_MAX;
            owner_.done("SetControlledUnit::unregister_observer", 0x006952a0u);
        } else {
            owner_.record("SetControlledUnit::unregister_observer", 0x006952a0u);
        }
    }
    void register_observer() override {
        if constexpr (kControlledUnitObserverBound) {
            // 006456D3 00694A60([00E188D8], root+8h), after 004C0890 bound it.
            owner_.observed_unit = owner_.units->controlled_bound()
                ? owner_.units->controlled_index() : SIZE_MAX;
            owner_.done("SetControlledUnit::register_observer", 0x00694a60u);
        } else {
            owner_.record("SetControlledUnit::register_observer", 0x00694a60u);
        }
    }
    void set_unit_controlled_audio(bool) override {
        owner_.record("SetControlledUnit::controlled_audio", 0x00954990u);
    }
    void set_controlled_unit(bool candidate_present) override {
        if (!candidate_present) {
            // 004C0890(null): not reached from 00647300, whose 00645060 already
            // accepted the same unit with the same arguments.
            owner_.record("SetControlledUnit::clear_controlled_unit", 0x004c0890u);
            return;
        }
        owner_.units->set_controlled_unit_004c0890(candidate_);
    }
    bool controlled_unit_present() override { return owner_.units->controlled_bound(); }
    bool controlled_is_candidate() override {
        return owner_.units->controlled_bound()
            && owner_.units->controlled_index() == candidate_;
    }
    bool controlled_is_kind_5() override {
        return owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), 5);
    }

private:
    GameHudHost::Impl& owner_;
    std::size_t candidate_;
    bool selectable_;
};
}  // namespace

namespace {
// 005CD1A0's callees over this host. Switch 1 binds the 2Ch push; the reseed
// (switch 2) and the camera (switch 3) stay records here.
class MovieInterfaceBinding final : public bsp::MovieInterfaceHost {
public:
    explicit MovieInterfaceBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    void seed_random_stream(int stream, std::uint32_t seed) override {
        // 005CD1B0 / 005CD1BC.
        owner_.reseed_stream(stream, seed, 0x005cd1b0u);
    }
    void push_interface_request(int interface_id, const void*) override {
        // 005CD1CE 004CC460(2Ch, [00E188D8]); the pending record is replaced.
        owner_.pending_interface_id = interface_id;
        owner_.note_pending_record_004cc460(interface_id, true);
        ++owner_.movie_pushes;
        owner_.done("MovieInterface::push_interface_2ch", 0x005cd1ceu);
    }
    void set_input_context_level(int, int) override {
        // 005CD1F6, only behind the multiplayer lock gate: not reached here.
        owner_.record("MovieInterface::input_context_level", 0x00a933f0u);
    }
    std::uint32_t ensure_movie_camera(std::uint32_t existing) override {
        if constexpr (kMovieMoverBound) {
            // 005CC170: only while +1Ch is null (005CC192).
            if (existing != 0) return existing;
            return owner_.build_movie_camera_005cc170() ? 1u : 0u;
        } else {
            owner_.record("MovieInterface::ensure_movie_camera", 0x005cc170u);
            return existing;
        }
    }
    void send_unit_session_message(int) override {
        // 005CD217..005CD224 0077C470([00E188D8], 1FFh, 0): release roles 0..8 of
        // the unit just tested at 005CD20E. Packet cc13-relgaps gap 1. The unit is
        // the controlled index read live here, the same read the caller's 00927F30
        // test made this call; the engage runs once per movie-screen instance.
        if constexpr (kMovieEngageRoleReleaseCc13Bound) {
            if (owner_.units != nullptr && owner_.units->controlled_bound()) {
                owner_.units->role_request_0077c470(owner_.units->controlled_index(), 0x1FFu, false);
                owner_.done("MovieInterface::session_message_1ff", 0x0077c470u);
            } else {
                owner_.record("MovieInterface::session_message_1ff", 0x0077c470u);
            }
        } else {
            owner_.record("MovieInterface::session_message_1ff", 0x0077c470u);
        }
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::movie_screen_camera_005cd240() {
    Impl& impl = *impl_;
    ++impl.movie_calls;
    const bool first = !impl.movie_screen.engaged;
    bsp::MovieInterfaceEngageInputs inputs{};
    inputs.game_present = true;              // 005CD1D3
    inputs.local_view_mode_active = false;   // game+1FE4h, single player
    inputs.player_unit_present = impl.units != nullptr && impl.units->controlled_bound();
    if (inputs.player_unit_present) {
        // 005CD20E 00927F30(unit, 0): the unit's role 0 is held by slot 0.
        std::int32_t holder = -1;
        inputs.unit_is_local_player_role =
            impl.units->unit_current_role_slot(impl.units->controlled_index(), 0, holder)
            && holder == 0;
    }
    MovieInterfaceBinding binding(impl);
    bsp::movie_screen_camera_005cd240(impl.movie_screen, impl.movie_lock, inputs, nullptr,
        binding);
    impl.done("MovieScreen::get_camera_005cd240", 0x005cd240u);
    if (first) {
        ++impl.movie_engages;
        impl.log.notef("movie interface 005CD1A0 engaged on the first MovCamNew call (hud update "
            "frame %llu): 004CC460(2Ch, controlled unit) pushed; the next 006840f0 applies "
            "{37h}", impl.summary.update_frames);
    }
}

namespace {
// The keyframes' parent queries over the units host.
class MovieParentBinding final : public bsp::MovieCameraParentHost {
public:
    explicit MovieParentBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool destroyed_5e(std::size_t unit) override {
        if constexpr (kMovieParentKilledByteBound) {
            // Switch 5 (packet cc9_movie_camera_keys): 00799D70's CMP byte
            // [entity+5Eh], 0 alone, from the units host's scene-node flags.
            // A unit the host cannot resolve keeps the OFF answer (no parent).
            bsp::SceneNodeFlags flags;
            if (owner_.units != nullptr && owner_.units->unit_scene_node_flags(unit, flags)) {
                return flags.destroyed;
            }
            return true;
        } else {
            // SUBSTITUTION: 00799D70 reads +5Eh alone; the units host answers the
            // four bytes of 0043F080 together.
            return owner_.units == nullptr || !owner_.units->unit_alive_and_visible(unit);
        }
    }
    bool is_kind(std::size_t unit, int kind) override {
        return owner_.units != nullptr && owner_.units->unit_is_kind_of(unit, kind);
    }
    std::size_t squadron_leader_3d0(std::size_t unit) override {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != unit) continue;
            for (const std::size_t u : r.member_units) {
                if (u != bsp::kPlaneSquadronNoUnit) return u;
            }
        }
        unsupported("MovieKeyframe::squadron_leader", 0x00799d9bu);
        return bsp::kMovieNoParent;
    }
    bool world(std::size_t unit, std::array<float, 16>& m) override {
        float right[3], up[3], forward[3], translation[3];
        if (owner_.units == nullptr
            || !owner_.units->unit_pose(unit, right, up, forward, translation)) {
            m = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            return false;
        }
        m = {right[0], right[1], right[2], 0.0f, up[0], up[1], up[2], 0.0f,
            forward[0], forward[1], forward[2], 0.0f,
            translation[0], translation[1], translation[2], 1.0f};
        return true;
    }
    float class_height_a8(std::size_t unit) override {
        // [[unit+538h]+A8h] is the class `Height` (docs/GUN_MOUNT_POSITIONS.md),
        // read from the installed VehicleClass row.
        const GameUnitRow* row = owner_.units != nullptr ? owner_.units->unit_row(unit) : nullptr;
        if (row == nullptr || owner_.lua == nullptr) return 0.0f;
        // Goal 4 chain 391: class +A8h as the descriptor load 00960230 read it
        // (009603D6, a plain number; nil reads 0). Without a loaded
        // descriptor (load unbound) the direct read stands in.
        const bsp::VehicleClassBaseFields* fields =
            owner_.lua->loaded_vehicle_class_base_fields(row->type_id, "hud_class_height_a8");
        return fields != nullptr ? fields->height
            : owner_.lua->read_vehicle_class_number(row->type_id, "Height", 0.0f);
    }
    bool ground_height_00903860(const std::array<float, 3>& point, float& out) override {
        // Switch 6 (packet cc9_movie_camera_keys): the scene-contents host's
        // Landscape query, the same 00903860 the image calls at 00795B62.
        return world_ground_height_00903860(point.data(), out);
    }
    std::array<float, 3> plane_velocity_810(std::size_t) override {
        // SUBSTITUTION: plane+810h..818h has no traced writer
        // (docs/PLANE_ADVANCE_POSE.md); zero.
        return {0.0f, 0.0f, 0.0f};
    }
    void unsupported(const char* what, std::uint32_t address) override {
        ++owner_.movie_unsupported;
        owner_.record(what, address);
    }
    void run_script_00887e50(const std::string& function) override {
        // Packet cc9_lua49_movie_modifier: a fired keyframe event's `function`
        // through the script-orders host's 00887E50 (`_G[name]()`, no arguments).
        // LABELLED: that entry point also counts the call among its blackout
        // callbacks.
        GameScriptOrdersHost* orders = owner_.lua != nullptr ? owner_.lua->script_orders() : nullptr;
        owner_.log.notef("movie keyframe event fires \"%s\" at hud update frame %llu (00791540 -> "
            "00887E50)", function.c_str(), owner_.summary.update_frames);
        if (orders == nullptr) {
            unsupported("MovieKeyframeEvent::run_script", 0x00887e50u);
            return;
        }
        static_cast<bsp::MissionBlackoutHost&>(*orders).mission_lua_call_named_00887e50(function);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

void GameHudHost::Impl::reseed_stream(int stream, std::uint32_t seed, std::uint32_t site) {
    // Switch 2: stream 1 reaches the gunnery host's shared stand-in (a
    // labelled substitution there); stream 0 has no host generator.
    GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
    if (kMovieReseedBound && gunnery != nullptr
        && gunnery->reseed_shared_stream_00bd2fd0(stream, seed)) {
        ++movie_reseeds;
        done(site == 0x005cd1b0u ? "MovieInterface::seed_random_stream_1"
                                 : "MovieCamera::seed_random_stream_1", site);
        return;
    }
    record(site == 0x005cd1b0u ? "MovieInterface::seed_random_stream"
                               : "MovieCamera::seed_random_stream", 0x00bd2fd0u);
}

bool GameHudHost::Impl::build_movie_camera_005cc170() {
    // 005CC19F operator new(570h), 0079D020, then its 00BD2FD0(1, 123).
    bsp::movie_camera_construct_0079d020(movie);
    reseed_stream(1, 123u, 0x0079d1abu);
    record("MovieCamera::place_in_world", 0x00923870u);
    // 005CC27C..005CC29A: 00B6DB70 on the node, then 007A0860 with its world.
    std::array<float, 16> seed{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
    if (node.ready) {
        for (std::size_t i = 0; i < 16; ++i) seed[i] = node.state.transform.world[i];
    }
    bsp::movie_camera_seed_007a0860(movie, seed);
    // 005CC2A9 004BC410: the outgoing mover is destroyed (00926D90).
    const bool replaced = camera_bound;
    camera_bound = false;
    movie_active = true;
    ++movie_builds;
    record("MovieCamera::observer_pair", 0x00694a60u);
    done("MovieCamera::build_005cc170", 0x005cc170u);
    log.notef("movie camera 005CC170 built and installed as the mover (004BC410 at 005CC2A9, "
        "hud update frame %llu); it replaced %s; seed position (%.1f,%.1f,%.1f)",
        summary.update_frames, replaced ? "the ShipCaptain, destroyed" : "no mover",
        static_cast<double>(seed[12]), static_cast<double>(seed[13]),
        static_cast<double>(seed[14]));
    return true;
}

void GameHudHost::Impl::destroy_movie_camera(const char* why) {
    movie_active = false;
    movie_screen.camera = 0;                            // the 00694A60 observer
    ++movie_destroys;
    // 0079A2FB, the destructor: 00BD2FD0(1, wall-clock milliseconds). Not
    // reproducible (docs/RANDOM_STREAMS.md), so a record.
    record("MovieCamera::destructor_reseed_wall_clock", 0x0079a2fbu);
    log.notef("movie camera destroyed at hud update frame %llu: %s (clock %.2f s, %zu camera "
        "and %zu look-at keyframes)", summary.update_frames, why,
        static_cast<double>(movie.clock_3b0), movie.camera_498.keys.size(),
        movie.target_414.keys.size());
    movie = bsp::HudMovieCamera{};
}

void GameHudHost::Impl::step_movie_camera(float seconds) {
    // SUBSTITUTION: the tick element's wave-3 slot 00798C80 runs once per
    // fixed step; this host runs one 0.05 s fixed step per frame, so it is
    // called once per frame here, before the frame's 0079A3B0.
    const bool latched_before = movie.running_391;   // 00798C8A CMP byte [ESI+221h]
    if (bsp::movie_camera_fixed_step_00798c80(movie, 0.05f)) {
        reseed_stream(1, 12345u, 0x00798cadu);          // 00798CAD
        reseed_stream(0, 54321u, 0x00798cb9u);          // 00798CB9
    }
    if constexpr (kMovieStepDrawBound) {
        // Switch 4 (packet cc9_movie_camera_keys): the else arm runs on every
        // step after the latch step. 00798D07 is 00BD2F10(ECX = 1, 0.0,
        // [00D046A8] = 65535.0). SUBSTITUTION, labelled: the draw goes through
        // the gunnery host's death-mode entry point with stream 1, which is the
        // shared stand-in generator on the default path. Under
        // BSP_GUNNERY_RNG_STREAMS=1 its key is (death_mode, FFFFFFh), an index
        // no unit has, so no other draw reads it. A dedicated entry point waits
        // on src/game_hosts_gunnery.cpp's lease.
        GameGunneryHost* gunnery = units != nullptr ? units->gunnery() : nullptr;
        if (latched_before && gunnery != nullptr) {
            const float draw = gunnery->death_mode_draw_00bd2f10(1, 0xFFFFFFu, 0.0f, 65535.0f);
            bsp::movie_camera_store_step_draw_00798cd9(movie, 0.05f, draw);
            ++movie_step_draws;
            done("MovieCamera::fixed_step_draw", 0x00798d07u);
        } else if (latched_before) {
            record("MovieCamera::fixed_step_draw", 0x00798d07u);
        }
    } else {
        // 00798D07: one stream-1 draw per step into a 16-entry ring. SUBSTITUTION:
        // not taken (the ring feeds nothing this host reads).
        record("MovieCamera::fixed_step_draw", 0x00798d07u);
    }
    MovieParentBinding parents(*this);
    if constexpr (bsp::kMovieKeyframeModifierBound) {
        // 00798D38..00798D59, the step's tail: 00797ED0 over both tracks. An
        // event's script may destroy this camera; the update below then finds
        // empty tracks.
        movie_events_fired += static_cast<unsigned long long>(
            bsp::movie_camera_key_ticks_00797ed0(movie, 0.05f, parents));
        done("MovieCamera::key_ticks_00797ed0", 0x00797ed0u);
        if (!movie_active) return;
    }
    std::array<float, 16> world{};
    float fov = 0.0f;
    if (!bsp::movie_camera_update_0079a3b0(movie, parents, seconds, world, fov)) {
        record("MovieCamera::update_empty_track", 0x0079bbd5u);
        return;
    }
    done("MovieCamera::update", 0x0079a3b0u);
    // Packet cc13-finscr: the finishscripts 007912E0 reached in this update.
    std::string finish_note;
    const std::vector<std::string> due = std::move(movie.finish_due);
    movie.finish_due.clear();
    for (const std::string& name : due) {
        ++movie_finish_due;
        finish_note += " finishscript=" + name;
        if constexpr (kMovieCameraFinishScriptCc13Bound) {
            finish_note += " ran=1";
        } else {
            finish_note += " would_run=1";
        }
    }
    if constexpr (kMovieCameraFinishScriptCc13Bound) {
        // 007912E0 00887E50(ECX = [[00E188A8]+1A08h], 0, &name, 0, 0, -1): `_G[name]()`
        // through the script-orders host's named runner, as the keyframe events.
        // SUBSTITUTION, labelled: run after 00798130's loops instead of inside the
        // step, in the same order, because the script may destroy this camera. If
        // it destroys and rebuilds the camera here, this frame still publishes the
        // old camera's world and fov (computed above).
        GameScriptOrdersHost* orders = lua != nullptr ? lua->script_orders() : nullptr;
        for (const std::string& name : due) {
            if (orders == nullptr) {
                parents.unsupported("MovieKeyframe::finishscript", 0x00887e50u);
            } else {
                log.notef("movie keyframe finishscript runs \"%s\" at hud update frame %llu "
                    "(007912E0 -> 00887E50, packet cc13-finscr)", name.c_str(), summary.update_frames);
                static_cast<bsp::MissionBlackoutHost&>(*orders).mission_lua_call_named_00887e50(name);
                ++movie_finish_ran;
                done("MovieKeyframe::finishscript", 0x007912e0u);
            }
        }
    }
    if (!movie_active) {
        // A finishscript destroyed the camera; its pose is not published.
        log.notef("movie camera pose skipped at hud update frame %llu:%s", summary.update_frames,
            finish_note.c_str());
        return;
    }
    bsp::CameraMatrix16 matrix{};
    for (std::size_t i = 0; i < 16; ++i) matrix[i] = world[i];
    bsp::MissionCameraProjection projection = camera_projection;
    projection.fov = fov;                                // 0079B833 00B6FBB0
    bsp::publish_mission_camera(matrix, projection);
    ++movie_publishes;
    // A frame with a due finishscript also prints, off the 40-frame cadence
    // (which it does not reset), with the cc13-finscr note at the end.
    const bool cadence = movie_publishes == 1 || summary.update_frames >= movie_last_pose_log + 40;
    if (cadence || !finish_note.empty()) {
        if (cadence) movie_last_pose_log = summary.update_frames;
        log.notef("movie camera pose at hud update frame %llu: clock %.2f s, position "
            "(%.1f,%.1f,%.1f), look-at (%.1f,%.1f,%.1f), forward (%.4f,%.4f,%.4f), fov %.4f%s",
            summary.update_frames, static_cast<double>(movie.clock_3b0),
            static_cast<double>(world[12]), static_cast<double>(world[13]),
            static_cast<double>(world[14]), static_cast<double>(movie.look_384[0]),
            static_cast<double>(movie.look_384[1]), static_cast<double>(movie.look_384[2]),
            static_cast<double>(world[8]), static_cast<double>(world[9]),
            static_cast<double>(world[10]), static_cast<double>(fov), finish_note.c_str());
    }
}

int GameHudHost::movie_add_position_007a44d0(const bsp::MovieKeyframeInput& in) {
    Impl& impl = *impl_;
    // 008B7AE6: the camera 005CD240 returned; null skips the store.
    if (!impl.movie_active || impl.movie_screen.camera == 0) {
        impl.record("MovieCamera::add_position_no_camera", 0x008b7ae8u);
        return 0;
    }
    MovieParentBinding parents(impl);
    const int added = bsp::movie_camera_add_position_007a42c0(impl.movie, in, parents);
    impl.movie_keyframes += static_cast<unsigned long long>(added);
    impl.done("MovieCamera::add_position_007a44d0", 0x007a44d0u);
    return added;
}

int hud_movie_add_position_007a44d0(const bsp::MovieKeyframeInput& in) {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return -1;
    return hud->movie_add_position_007a44d0(in);
}

void GameHudHost::update_movie_screen_005cbaf0(float seconds) {
    Impl& impl = *impl_;
    impl.record("MovieScreen::black_bars_update", 0x005cbaf0u);
    impl.step_mission_camera(seconds);
}

bool hud_movie_screen_camera_005cd240() {
    GameHudHost* hud = attached_hud_for_selection();
    if (hud == nullptr) return false;
    hud->movie_screen_camera_005cd240();
    return true;
}

bool hud_unit_selectable_00645060(std::size_t unit, bool& reached) {
    GameHudHost* hud = attached_hud_for_selection();
    reached = hud != nullptr;
    if (hud == nullptr) return false;
    return hud->unit_selectable_00645060(unit);
}

bool GameHudHost::unit_selectable_00645060(std::size_t unit) {
    Impl& impl = *impl_;
    if (impl.units == nullptr || unit >= impl.units->count()) return false;   // 00645082
    impl.done("HelmOrder::selectable_00645060", 0x00645060u);
    return bsp::unit_is_selectable_00645060(impl.selectable_inputs_00645060(unit));
}

bool GameHudHost::set_selected_unit_00647300(std::size_t unit) {
    Impl& impl = *impl_;
    ++impl.select_calls;
    if (impl.units == nullptr || unit >= impl.units->count()) {
        // 00645082: a null unit is rejected by 00645060.
        impl.log.notef("SetSelectedUnit 00647300: unit index %zu has no created unit; "
            "00645060 rejects a null unit", unit);
        return false;
    }
    const GameUnitRow* row = impl.units->unit_row(unit);
    const std::string name = row != nullptr ? row->name : std::string("?");
    const bsp::UnitSelectableInputs in = impl.selectable_inputs_00645060(unit);
    const bool accepted = bsp::unit_is_selectable_00645060(in);        // 00647317
    impl.done("SetSelectedUnit::selectable_00645060", 0x00645060u);
    if (!accepted) {                                                  // 0064731E
        impl.log.notef("SetSelectedUnit 00647300: \"%s\" rejected by 00645060 (alive=%d "
            "party0=%d kind2=%d kind0F=%d kind2A=%d kind46=%d kind45=%d vt124=%d role=%d)",
            name.c_str(), in.alive_5c ? 1 : 0, in.team_matches_owner ? 1 : 0,
            in.is_kind_2 ? 1 : 0, in.is_kind_0f ? 1 : 0, in.is_kind_2a ? 1 : 0,
            in.is_kind_46 ? 1 : 0, in.is_kind_45 ? 1 : 0, in.vtable_124_allows ? 1 : 0,
            in.team_query_00927c50 ? 1 : 0);
        return false;
    }
    ++impl.select_accepted;
    const std::string before = impl.units->controlled_bound()
        ? impl.units->unit_row(impl.units->controlled_index())->name : std::string("none");
    SelectControlledUnitBinding binding(impl, unit, accepted);
    const bsp::SelectControlledUnitResult result =
        bsp::select_controlled_unit_00645600(true, binding);          // 00647323
    impl.done("SetSelectedUnit::set_controlled_unit_00645600", 0x00645600u);
    if (!impl.units->controlled_bound()) {                            // 00647328
        // 00647331..0064734B: the manager idle, push 34h INTF_LIMBO. Not
        // reachable after an accepted unit (the setter bound it).
        impl.record("SetSelectedUnit::push_limbo_34h", 0x0064734bu);
        return true;
    }
    // 00647059: reset the retained screen at [manager+CCh] before pushing.
    impl.reset_unit_pick_selection("SetSelectedUnit::screen_reset_005251c0");
    // 0064705E..00647077: 004CC460(20h, [00E188D8]->vtable[140h]()). For a
    // ship the slot is 0047F320, `MOV EAX,ECX`: the unit itself, which is the
    // payload the host's classifier already reads (the controlled unit).
    impl.unit_request_pending = true;
    impl.unit_request_applied = false;
    impl.pending_interface_id = 0;   // 004CC460 overwrites the one pending record
    impl.note_pending_record_004cc460(kInterfaceScene3d, true);
    ++impl.select_pushes;
    impl.done("SetSelectedUnit::push_interface_20h", 0x00647077u);
    impl.log.notef("SetSelectedUnit 00647300: \"%s\" accepted by 00645060; 00645600 moved "
        "00e188d8 from \"%s\" to \"%s\" (released_previous=%d refreshed_in_place=%d); "
        "00647040 pushed interface 20h with the unit, serviced by the next 006840f0",
        name.c_str(), before.c_str(), name.c_str(), result.released_previous ? 1 : 0,
        result.refreshed_in_place ? 1 : 0);
    return true;
}

namespace {
// Packet cc9_force_select_unit: bsp::HudRootUnitListsHost over the units host.
// Handles are unit index + 1; 0 is null.
class HudRootListsBinding final : public bsp::HudRootUnitListsHost {
public:
    explicit HudRootListsBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::uint32_t controlled_unit() override {
        return owner_.units->controlled_bound()
            ? static_cast<std::uint32_t>(owner_.units->controlled_index() + 1) : 0u;
    }
    const std::vector<std::uint32_t>& local_unit_list_1970() override {
        // game+1974h is the head of game+1970h, which the world host's last
        // 004C3CB0 body built (walk 0 minus ordnance, UnitRef = index + 1).
        list_.clear();
        const bsp::LocalPlayerUnitLists* lists = game_local_player_unit_lists();
        if (lists == nullptr) {
            owner_.record("HudRootLists::list_1970", 0x00648369u);
            return list_;
        }
        for (const bsp::UnitRef ref : (*lists)[static_cast<std::size_t>(
                 bsp::LocalPlayerUnitList::kWalk0Rest)]) {
            list_.push_back(ref);
        }
        return list_;
    }
    bool unit_selectable_00645060(std::uint32_t unit) override {
        if (unit == 0 || unit > owner_.units->count()) return false;
        return bsp::unit_is_selectable_00645060(owner_.selectable_inputs_00645060(unit - 1));
    }
    bool unit_is_group_leader_00778890(std::uint32_t unit) override {
        const std::int32_t group = group_of(unit);
        // bsp.py show 00778890: a null +284h is false; otherwise compare
        // the current +14h leader with this unit. Keep the HUD's existing
        // one-based identities, including zero for a missing group.
        return bsp::ship_ai_unit_group_leads_00778890(unit,
            group < 0 ? 0u : static_cast<std::uint32_t>(group) + 1u,
            group < 0 ? 0u : static_cast<std::uint32_t>(
                owner_.units->formation_leader_0014(group) + 1u));
    }
    bool unit_is_group_member_007788b0(std::uint32_t unit) override {
        return unit != 0 && owner_.units->unit_is_formation_follower_007788b0(unit - 1);
    }
    std::uint32_t group_leader_007788d0(std::uint32_t unit) override {
        const std::int32_t group = group_of(unit);
        if (group < 0) return 0;
        const std::size_t leader = owner_.units->formation_leader_0014(group);
        return leader == SIZE_MAX ? 0u : static_cast<std::uint32_t>(leader + 1);
    }
    bool role_word_188_open(std::uint32_t unit) override {
        // [unit+188h] == 9 or == [game+18ECh]. SUBSTITUTION (labelled, as
        // selectable_inputs_00645060 takes it): the local player index is 0.
        std::int32_t word = 0;
        if (unit == 0 || !owner_.units->unit_role_permission(unit - 1, 0, word)) return false;
        return word == 9 || word == 0;
    }
    int group_member_count_4f8(std::uint32_t leader) override {
        const std::int32_t group = group_of(leader);
        return group < 0 ? 0 : owner_.units->formation_member_count(group);
    }
    std::uint32_t group_member_at_0070d060(std::uint32_t leader, int slot) override {
        const std::int32_t group = group_of(leader);
        if (group < 0) return 0;
        const std::size_t member = owner_.units->formation_member_unit(group, slot);
        return member == SIZE_MAX ? 0u : static_cast<std::uint32_t>(member + 1);
    }
    void set_controlled_unit_00645600(std::uint32_t unit) override {
        const std::size_t index = unit != 0 ? unit - 1 : 0;
        const bool selectable = unit != 0 && unit_selectable_00645060(unit);
        SelectControlledUnitBinding binding(owner_, index, selectable);
        bsp::select_controlled_unit_00645600(unit != 0, binding);
        owner_.done("ForceSelectUnit::set_controlled_unit_00645600", 0x00645600u);
    }
    bool multiplayer_1fe4() override { return false; }
    // SUBSTITUTION (labelled): the manager's +4h/+20h/+1Ch/+38h are not read
    // here. They matter only on an empty +8Ch vector, where the answers
    // (applied 0, pending -1) take neither push; that arm is a record.
    int manager_applied_04() override {
        owner_.record("ForceSelectUnit::empty_list_manager_state", 0x006485d7u);
        return 0;
    }
    int manager_pending_20() override { return -1; }
    bool manager_1c_equals_38() override { return false; }
    void push_interface_request_004cc460(int) override {
        owner_.record("ForceSelectUnit::push_limbo", 0x004cc460u);
    }
    void push_controlled_unit_interface_00647040() override {
        // The same 00647040 as SetSelectedUnit's tail: reset, then push.
        owner_.reset_unit_pick_selection("ForceSelectUnit::screen_reset_005251c0");
        owner_.unit_request_pending = true;
        owner_.unit_request_applied = false;
        owner_.pending_interface_id = 0;
        owner_.note_pending_record_004cc460(kInterfaceScene3d, true);
        ++owner_.force_selects;
        owner_.done("ForceSelectUnit::push_interface_20h", 0x00647077u);
    }

private:
    std::int32_t group_of(std::uint32_t unit) const {
        return unit == 0 ? -1 : owner_.units->unit_formation_group_0284(unit - 1);
    }
    GameHudHost::Impl& owner_;
    std::vector<std::uint32_t> list_;
};
}  // namespace

void GameHudHost::Impl::run_root_lists(bool force) {
    HudRootListsBinding binding(*this);
    const std::string before = units->controlled_bound()
        ? units->unit_row(units->controlled_index())->name : std::string("none");
    if (force) {
        bsp::hud_root_force_select_unit_006485a0(root_lists, binding);
    } else {
        bsp::hud_root_rebuild_unit_lists_00648290(root_lists, binding);
    }
    ++root_rebuilds;
    const std::uint32_t at = bsp::hud_root_cursor_unit_00644a60(root_lists);
    const GameUnitRow* at_row = at != 0 ? units->unit_row(at - 1) : nullptr;
    const std::string after = units->controlled_bound()
        ? units->unit_row(units->controlled_index())->name : std::string("none");
    log.notef("hud root unit lists %s (hud update frame %llu): +8Ch %zu units, +9Ch %zu units, "
        "cursor (%d,%d) at \"%s\"; controlled \"%s\" -> \"%s\"",
        force ? "006485A0 ForceSelectUnit" : "00648290 rebuild at the 44h enter",
        summary.update_frames, root_lists.primary_8c.size(), root_lists.secondary_9c.size(),
        root_lists.cursor.secondary ? 1 : 0,
        root_lists.cursor.index == 0xffffu ? -1 : static_cast<int>(root_lists.cursor.index),
        at_row != nullptr ? at_row->name.c_str() : "none", before.c_str(), after.c_str());
    // Packet cc9_vtable124_liveness: the +8Ch members in order, with class ids.
    std::string members;
    for (const std::uint32_t id : root_lists.primary_8c) {
        const GameUnitRow* row = id != 0 && id <= units->count() ? units->unit_row(id - 1) : nullptr;
        char cls[16];
        std::snprintf(cls, sizeof(cls), "/%02X", id != 0 ? units->unit_class_id(id - 1) & 0xff : 0);
        if (!members.empty()) members += ' ';
        members += row != nullptr ? row->name : std::string("?");
        members += cls;
    }
    log.notef("  +8Ch members: %s", members.empty() ? "(none)" : members.c_str());
}

void GameHudHost::enter_hud_root_screen_006488d0() {
    Impl& impl = *impl_;
    if (impl.units == nullptr) {
        impl.record("HudRootScreen::enter", 0x006488d0u);
        return;
    }
    // 006488D0..00648923: the three widget +4Ch(0.0f) calls, game mode into
    // +24h and +F4h = 0 are not performed (records, as before this packet);
    // 00648913 00648290 is.
    impl.record("HudRootScreen::enter_widgets_mode_counter", 0x006488d0u);
    impl.run_root_lists(false);
    impl.done("HudRootScreen::rebuild_unit_lists_00648290", 0x00648913u);
}

void GameHudHost::force_select_unit_006485a0() {
    Impl& impl = *impl_;
    ++impl.force_calls;
    if (impl.units == nullptr) {
        impl.record("ForceSelectUnit::no_units", 0x006485a0u);
        return;
    }
    impl.run_root_lists(true);
    impl.done("ForceSelectUnit::force_select_006485a0", 0x006485a0u);
}

void GameHudHost::update_minimap_screen_005c0f20(float seconds) {
    Impl& impl = *impl_;
    impl.step_mission_camera(seconds);
    if (!impl.minimap) {
        impl.record("HudMinimap::update", 0x005c0f20u);
        return;
    }
    impl.minimap->update_005c0f20(seconds);
}

GuiLayoutWidget* GameHudHost::Impl::ship_screen_widget(bsp::ShipScreenWidget widget) {
    if (!ship_screen_widgets_bound) {
        ship_screen_widgets_bound = true;
        // 0064C0F0's lookups: the offset each named widget is stored at.
        static const std::pair<int, const char*> kNames[] = {
            {0x48, "ship_stick_Icon"}, {0xF8, "ship_relation_Icon"},
            {0x18C, "VillanasFelso_Icon"}, {0x190, "VillanasAlso_Icon"},
            {0x194, "VillanasBal_Icon"}, {0x198, "VillanasJobb_Icon"},
            // Parts 3 and 4 (packet cc9_ship_screen_parts34).
            {0x50, "ship_dir_Icon"}, {0xC4, "Icon_3_Icon"}, {0xD4, "Icon_5_Icon"},
            {0xD8, "Hl_1_Icon"}, {0xDC, "Hl_2_Icon"}, {0xE0, "Hl_3_Icon"},
            {0xE4, "Hl_4_Icon"}, {0xE8, "circle_1_Section"}, {0xEC, "circle_2_Section"},
            {0xF0, "circle_3_Section"}, {0xF4, "circle_4_Section"}};
        static const char* const kPages[] = {
            "GUI_ship", "GUI_repair", "GUI_ship_effects", "GUI_ship_damage"};
        for (const auto& entry : kNames) {
            GuiLayoutWidget* found = nullptr;
            for (const char* page_name : kPages) {
                GuiLayoutPage* page = menu.in_game_page(0x45, page_name);
                if (page == nullptr || !page->root) continue;
                found = bsp::find_descendant_by_name(*page->root, entry.second);
                if (found != nullptr) break;
            }
            ship_screen_widgets[entry.first] = found;
        }
        if constexpr (kHudShipScreenApplyUnitWidgetsBound) {
            // Packet cc13-wire2: the widgets 0064AE20 drives (addresses at the switch).
            static const std::pair<int, const char*> kApplyUnitNames[] = {
                {0x54, "ship_recon_Icon"}, {0x58, "ship_torpedo_Icon"},
                {0x68, "ship_recon_num1_Icon"}, {0x6C, "ship_recon_num2_Icon"},
                {0x70, "ship_torpedo_num1_Icon"}, {0x74, "ship_torpedo_num2_Icon"}};
            for (const auto& entry : kApplyUnitNames) {
                GuiLayoutWidget* found = nullptr;
                for (const char* page_name : kPages) {
                    GuiLayoutPage* page = menu.in_game_page(0x45, page_name);
                    if (page == nullptr || !page->root) continue;
                    found = bsp::find_descendant_by_name(*page->root, entry.second);
                    if (found != nullptr) break;
                }
                ship_screen_widgets[entry.first] = found;
            }
            // +40h holds the page itself (00AA5840's result), not a descendant.
            GuiLayoutPage* damage = menu.in_game_page(0x45, "GUI_ship_damage");
            ship_screen_widgets[0x40] =
                damage != nullptr && damage->root ? damage->root.get() : nullptr;
        }
        // 0064C0F0 stores the speed icons at +60/+64. The other two
        // pairs (+68..+74) are already bound by the apply-unit arm.
        GuiLayoutPage* page = menu.in_game_page(0x45, "GUI_ship");
        if (page != nullptr && page->root) {
            ship_screen_widgets[0x60] =
                bsp::find_descendant_by_name(*page->root, "ship_speed_num1_Icon");
            ship_screen_widgets[0x64] =
                bsp::find_descendant_by_name(*page->root, "ship_speed_num2_Icon");
        }
    }
    auto it = ship_screen_widgets.find(static_cast<int>(widget));
    return it != ship_screen_widgets.end() ? it->second : nullptr;
}

void GameHudHost::Impl::bind_ship_screen_unit_0064d590(std::size_t unit) {
    class Binding final : public bsp::ingame_gui_cc13::ShipScreenUnitHost {
    public:
        Binding(Impl& owner, std::size_t next) : owner_(owner), next_(next) {}
        const void* observed_unit() override {
            return owner_.ship_screen.has_unit
                ? owner_.units->unit_identity(owner_.ship_screen_unit) : nullptr;
        }
        void mark_unit_changed() override { owner_.ship_screen.unit_changed_188 = true; }
        void delete_timed_entry(std::size_t widget, int entry) override {
            // 0064D5A9..0064D5C3: +E8/+EC/+F0/+F4 are the four damage
            // circles on the ship damage page.
            // The bridge owns GuiLayoutWidget trees, not retained widget
            // companions, and never creates their timed entries. There is no
            // entry to retire on this path; keep the native lower call a record.
            static_cast<void>(widget);
            static_cast<void>(entry);
            owner_.record("HudShipScreen::cancel_unit_timed_entry", 0x00aa8b80u);
        }
        void observe_unit(const void* value) override {
            if (observed_unit() == value) return;
            // Preserve the host's one +184h projection (has_unit/index). The
            // native 00694A60/006952A0 graph and release callback for screen
            // +170h are still unbound; do not call them implemented merely
            // because the semantic reference changes.
            if (owner_.ship_screen.has_unit)
                owner_.record("HudShipScreen::unregister_unit_observer", 0x006952a0u);
            owner_.ship_screen.has_unit = value != nullptr;
            owner_.ship_screen_unit = next_;
            if (value != nullptr)
                owner_.record("HudShipScreen::register_unit_observer", 0x00694a60u);
        }
        bool screen_active() override { return owner_.ship_screen.active_05; }
        void apply_unit() override {
            // Same actual 0064AE20 adapter/gates as the previous host path.
            if (kHudShipScreenGaugesBound) {
                if constexpr (kHudShipScreenApplyUnitBound) {
                    owner_.apply_ship_screen_unit_0064ae20();
                } else {
                    owner_.record("HudShipScreen::apply_unit_0064ae20", 0x0064ae20u);
                }
            }
        }
    private:
        Impl& owner_;
        std::size_t next_;
    } binding(*this, unit);
    // The incoming identity and the previous identity both belong to the
    // existing live unit registry; no raw-layout scratch unit is fabricated.
    bsp::ingame_gui_cc13::ship_screen_set_unit_0064d590(
        binding, units->unit_identity(unit));
    done("HudShipScreen::set_unit", 0x0064d590u);
}

namespace {
// bsp::ShipScreenHost over this process's HUD.
class ShipScreenBinding final : public bsp::ShipScreenHost {
public:
    explicit ShipScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    void set_visible(bsp::ShipScreenWidget widget, bool visible) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    void set_rotation(bsp::ShipScreenWidget widget, float radians) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*w, radians);
    }
    float rotation(bsp::ShipScreenWidget widget) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        return w != nullptr ? w->transform.rotate : 0.0f;
    }
    void set_alpha(bsp::ShipScreenWidget widget, float alpha) override {
        // Virtual +4Ch (00AA6980) writes the alpha lane of the Color property.
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, w->color[0], w->color[1], w->color[2], alpha);
    }
    void select_state(bsp::ShipScreenWidget widget, int state, int zero, float one) override {
        static_cast<void>(widget);
        static_cast<void>(state);
        static_cast<void>(zero);
        static_cast<void>(one);
        // The icon state select, 00AB1710 (gui_icon.hpp); the bridge draws
        // the first authored state only.
        owner_.record("HudShipScreen::relation_select_state", 0x00ab1710u);
    }
    bool unit_is_kind_of(int class_id) override {
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.ship_screen_unit, class_id);
    }
    bool unit_in_formation() override {
        return owner_.units != nullptr
            && owner_.units->unit_formation_group_0284(owner_.ship_screen_unit) >= 0;
    }
    bool unit_leads_formation() override {
        if (owner_.units == nullptr) return false;
        const std::int32_t group = owner_.units->unit_formation_group_0284(owner_.ship_screen_unit);
        return owner_.units->formation_leader_0014(group) == owner_.ship_screen_unit;
    }
    float unit_throttle() override {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        return row != nullptr ? row->throttle : 0.0f;              // unit+980h
    }
    bool flash_view_mode_is_1() override {
        // SUBSTITUTION: [[00E198C4]+4Ch]+30h is on an interface object this
        // process does not build; the enable reads clear.
        owner_.record("HudShipScreen::flash_view_mode", 0x0064e31au);
        return false;
    }
    void relation_slide_block() override {
        owner_.record("HudShipScreen::relation_slide", 0x0064dd6du);
    }
    void pipe_sight_block(float dt) override {
        static_cast<void>(dt);
        // With kMissionFovBound the fov this block sets through 004DC940 is
        // applied by the mission camera's tick (docs/MISSION_CAMERA.md 9).
        owner_.record("HudShipScreen::pipe_sight_block", 0x0064de92u);
    }
    bool controlled_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    bool controlled_is_kind_of(int class_id) override {
        return controlled_present()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool controlled_is_local_player() override {
        // 00927F30(unit, 0): the controlled unit is the local player's ship.
        return controlled_present();
    }
    bool controlled_class_repair() override {
        // 0064E5E8..0064E5FB: [unit+538h]+D0h, the VehicleClass `Repair` byte
        // (00962E16), as the gunnery host keeps it (goal 5 chain 661).
        const GameGunneryHost* gunnery = ship_gunnery();
        return gunnery != nullptr && controlled_present()
            && gunnery->unit_class_repair_d0(owner_.units->controlled_index());
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool input_held(int action) override { return owner_.menu.input_action_held(action); }
    bool input_released(int action) override { return owner_.menu.input_action_released(action); }
    void turn_to_camera_order() override {
        owner_.record("HudShipScreen::turn_to_camera_order", 0x0077c2a0u);
    }
    void turn_to_camera_release() override {
        owner_.record("HudShipScreen::turn_to_camera_release", 0x0064e5c3u);
    }
    bool turn_timer_expired_009539e0() override {
        owner_.record("HudShipScreen::turn_timer_expired", 0x009539e0u);
        return false;
    }
    void repair_menu_open(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::repair_menu", 0x0064e62cu);
    }
    void repair_order_route() override {
        owner_.record("HudShipScreen::repair_order_route", 0x0077c2a0u);
    }
    void warning_pulse(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::warning_pulse", 0x0064f3c2u);
    }
    void repair_mode_panel(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::repair_mode_panel", 0x0064f4a3u);
    }
    bool other_screen_gate() override {
        // SUBSTITUTION: +156h is the host's (clear); [[00E198C4]+64h]+81h and
        // +82h sit on an interface object this process does not build and
        // read clear, so the gate passes.
        return true;
    }
    void other_screen_00545360() override {
        // 00545360 on [00E198C4]+50h: that screen's +D4h = 1, +1Ch = 0.
        if constexpr (kHudWeaponGroupLayoutBound) {
            // +1Ch is the screen's own byte, not modelled (labelled).
            owner_.weapon_group.flag_d4 = true;
            owner_.done("HudShipScreen::other_screen_00545360", 0x00545360u);
        } else {
            owner_.record("HudShipScreen::other_screen_00545360", 0x00545360u);
        }
    }
    bool controls_bound() override { return kHudShipScreenControlsBound; }
    void remainder_from_0064e415(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064e415u);
    }
    void remainder_from_0064f665(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064f665u);
    }
    // Part 3, packet cc9_ship_screen_parts34.
    bool damage_bound() override { return kHudShipScreenDamageBound; }
    void color(bsp::ShipScreenWidget widget, float out[4]) override {
        // Virtual +54h copies the Color property.
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        for (int i = 0; i < 4; ++i) out[i] = w != nullptr ? w->color[i] : 0.0f;
    }
    void set_color(bsp::ShipScreenWidget widget, const float rgba[4]) override {
        GuiLayoutWidget* w = owner_.ship_screen_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, rgba[0], rgba[1], rgba[2], rgba[3]);
    }
    bsp::ShipRepairTaskTerms controlled_repair_task() override {
        // Goal 5 chain 661: the repair task at unit+A20h of the controlled
        // ship as the gunnery host's damage control keeps it: priority +24h
        // (0064F6BD), water +34h over +40h (0064FA7E), fire +38h over +3Ch
        // (0064FB38), and 0093A3F0 on its +18h failure records (0064F9D2).
        bsp::ShipRepairTaskTerms terms{};
        const GameGunneryHost* gunnery = ship_gunnery();
        if (gunnery != nullptr && controlled_present()) {
            gunnery->ship_repair_task_terms(owner_.units->controlled_index(), terms);
        }
        return terms;
    }
    float settings_engine_jam_seconds() override {
        // 0064F8DA..0064F9C3 over GameSettings+3E4h, the ShipGlobals.Failures
        // rows the gunnery host loads (goal 5 chain 661).
        const GameGunneryHost* gunnery = ship_gunnery();
        return gunnery != nullptr ? gunnery->failure_descriptor_engine_jam_seconds() : 0.0f;
    }
    float settings_4c4() override {
        // Recorded with controlled_float_125c, the submarine arm's other term.
        return 0.0f;
    }
    float controlled_float_125c() override {
        // SUBSTITUTION: unit+125Ch and GameSettings+4C4h, the submarine
        // circle's terms, are not modelled.
        owner_.record("HudShipScreen::submarine_circle_terms", 0x0064f7ebu);
        return 0.0f;
    }
    int controlled_device_count() override {
        // Goal 5 chain 661: 0064FBEC..0064FC7A walks the unit+48h devices; the
        // gunnery host gives the controlled ship's gun rows (kind 20h) with
        // their Function and health, read once per damage-panel pass.
        devices_.clear();
        const GameGunneryHost* gunnery = ship_gunnery();
        if (gunnery != nullptr && controlled_present()) {
            devices_ = gunnery->ship_device_terms(owner_.units->controlled_index());
        }
        return static_cast<int>(devices_.size());
    }
    bsp::ShipDeviceTerms controlled_device(int index) override {
        if (index < 0 || static_cast<std::size_t>(index) >= devices_.size()) return {};
        return devices_[static_cast<std::size_t>(index)];
    }
    void circle_progress(bsp::ShipScreenWidget widget, float ratio, bool snap) override {
        static_cast<void>(widget);
        static_cast<void>(ratio);
        // 00ABE6E0 on a Section, or 00AA8B00(2)'s timed entry: the bridge
        // draws neither.
        if (snap) {
            owner_.record("HudShipScreen::circle_quad", 0x00abe6e0u);
        } else {
            owner_.record("HudShipScreen::circle_progress", 0x00aa8b00u);
        }
    }
    // Part 4.
    bool gauges_bound() override { return kHudShipScreenGaugesBound; }
    void remainder_from_0064fd24(float dt) override {
        static_cast<void>(dt);
        owner_.record("HudShipScreen::update_remainder", 0x0064fd24u);
    }
    float unit_ordered_rudder() override {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        return row != nullptr ? row->ordered_rudder : 0.0f;       // unit+984h
    }
    void dir_clock_step(bsp::ShipScreenState& screen) override {
        static_cast<void>(screen);
        // SUBSTITUTION: the step runs only when the platform clock (01090AB0
        // +14h, milliseconds) passes the static 00E19808 + 21h; that clock is
        // not reachable from the HUD host. +124h and +12Ch have no reader in
        // the screen's code (a disp32 scan of 124h finds only the enter and
        // this block), so the step is recorded and not applied.
        owner_.record("HudShipScreen::dir_clock_step", 0x0064fdd4u);
    }
    float unit_forward_speed() override {
        // 0092D730 on [unit+1018h], reconstructed.
        return owner_.units != nullptr
            ? owner_.units->unit_forward_speed_0092d730(owner_.ship_screen_unit) : 0.0f;
    }
    int unit_int_638() override {
        owner_.record("HudShipScreen::gauge_source_638", 0x0064fff9u);
        return 0;
    }
    float unit_float_1124() override {
        owner_.record("HudShipScreen::gauge_source_class_790", 0x00650019u);
        return 0.0f;
    }
    int unit_class_int_790() override { return 0; }
    int unit_device_count_00852300() override {
        owner_.record("HudShipScreen::gauge_source_00852300", 0x00852300u);
        return 0;
    }
    int unit_torpedo_stock_00815850() override {
        owner_.record("HudShipScreen::gauge_source_00815850", 0x00815850u);
        return 0;
    }
    void gauge_digit(int gauge, int digit, float magnitude) override {
        // The speed magnitude comes from 0092D730 on the controlled
        // unit. Recon/torpedo gauges still have recorded source hooks
        // above, so do not feed their substituted zeros to the body.
        if (gauge != 0) {
            owner_.record("HudShipScreen::gauge_digit_uv", 0x0043aba0u);
            return;
        }
        // 0064B429 constructs the speed pair; 0043DFB9..DFCE
        // appends its SECOND argument first. Thus digit 0 (units) is
        // num2 (+64), digit 1 (tens) is num1 (+60).
        if (digit < 0 || digit >= 2) {
            throw std::logic_error("HUD ship digit outside native gauge pair");
        }
        const auto slot = static_cast<bsp::ShipScreenWidget>(0x64 - 4 * digit);
        GuiLayoutWidget* widget = owner_.ship_screen_widget(slot);
        if (widget != nullptr && owner_.menu.frontend().roll_widget_digit(*widget, digit, magnitude)) {
            owner_.done("HudShipScreen::gauge_digit_uv", 0x0043aba0u);
        } else {
            // A missing authored widget/loaded atlas is still a missing
            // resource, rather than an invented icon rectangle.
            owner_.record("HudShipScreen::gauge_digit_uv", 0x0043aba0u);
        }
    }

private:
    const GameGunneryHost* ship_gunnery() const {
        return owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
    }

    GameHudHost::Impl& owner_;
    // The damage panel's device list for this pass (controlled_device_count).
    std::vector<bsp::ShipDeviceTerms> devices_;
};
}  // namespace

void GameHudHost::update_ship_screen_0064dd30(float seconds, bool active) {
    Impl& impl = *impl_;
    impl.ship_screen.active_05 = active;
    ShipScreenBinding binding(impl);
    bsp::ship_screen_update_0064dd30(impl.ship_screen, binding, seconds);
    impl.done("HudShipScreen::update", 0x0064dd30u);
}

namespace {
// The Big1Host adapter for 0064AE20 (kHudShipScreenApplyUnitBound). The body
// reads and writes the native layout, so the adapter lays out a scratch
// screen, unit and class record, runs the body over them and copies the
// written fields back into GameHudHost::Impl::ship_screen. Host state in:
// unit+980h = the row's throttle, class+70h = the row's type_id, screen+104h
// = gauge_sub_104 (the body leaves it unwritten off the submarine arm), and
// vtable[5Ch](8) = unit_is_kind_of. Every other input is a stand-in, labelled
// at its hook.
class ShipScreenApplyUnitAdapter final : public bsp::cc13_big1::Big1NullHost {
public:
    ShipScreenApplyUnitAdapter(GameHudHost::Impl& owner, bsp::ShipScreenHost& binding)
        : owner_(owner), binding_(binding) {}

    void run() {
        const GameUnitRow* row =
            owner_.units != nullptr ? owner_.units->unit_row(owner_.ship_screen_unit) : nullptr;
        put<float>(unit_, 0x980, row != nullptr ? row->throttle : 0.0f);
        put<std::int32_t>(class_, 0x70, row != nullptr ? row->type_id : -1);
        if constexpr (kHudShipScreenApplyUnitLuaBound) {
            // class+C8h (the SETG at 0064AFA3) is Catapult.LaunchStock as 00960503
            // stored it at class load; NoRepairGUI is the 0064AED4 lookup. Read once
            // here, served by the Lua hooks below. Uncertain: whether anything
            // rewrites class+C8h after the load is not established.
            if (row != nullptr && owner_.lua != nullptr) {
                GameMissionLuaHost& lua = *owner_.lua;
                {
                    // Goal 4 chain 391: class +C8h from the descriptor load
                    // 00960230 (00960503; 0 when Catapult is not a table). Without
                    // a loaded descriptor (load unbound) the direct read stands in.
                    const bsp::VehicleClassBaseFields* fields =
                        lua.loaded_vehicle_class_base_fields(row->type_id, "0064afa3");
                    put<std::int32_t>(class_, 0xC8, fields != nullptr ? fields->launch_stock
                        : lua.read_vehicle_class_integer(row->type_id, "Catapult",
                                                         "LaunchStock", 0));
                }
                no_repair_gui_ =
                    lua.read_vehicle_class_boolean(row->type_id, "NoRepairGUI", false) ? 1 : 0;
            }
        }
        // OFF (or no row / Lua): STAND-IN class+C8h = 0.
        put<void*>(unit_, 0x538, class_);
        put<void*>(screen_, 0x184, unit_);
        for (int offset : {0x40, 0x54, 0x58, 0x68, 0x6C, 0x70, 0x74}) {
            put<void*>(screen_, offset, widget_tags_ + offset);
        }
        for (int i = 0; i < 3; ++i) put<void*>(screen_, 0x78 + 4 * i, gauges_[i]);
        bsp::ShipScreenState& state = owner_.ship_screen;
        screen_[0x104] = state.gauge_sub_104 ? 1 : 0;

        bsp::cc13_big1::hud_ship_screen45_apply_unit_0064ae20(screen_, *this);

        state.stick_44 = get<float>(screen_, 0x44);
        state.dir_target_128 = get<float>(screen_, 0x128);
        state.dir_rate_12c = get<float>(screen_, 0x12C);
        state.gauge_sub_104 = screen_[0x104] != 0;
        state.gauge_recon_105 = screen_[0x105] != 0;
        state.gauge_class_106 = screen_[0x106] != 0;
        state.gauge_torpedo_107 = screen_[0x107] != 0;
        for (int i = 0; i < 3; ++i) state.gauges[i].primed_18 = gauges_[i][0x18] != 0;
        owner_.done("HudShipScreen::apply_unit_0064ae20", 0x0064ae20u);
    }

    void unreached(const char* hook, std::uint32_t address) override {
        static_cast<void>(hook);
        owner_.record("HudShipScreen::apply_unit_unexpected_hook", address);
    }
    // STAND-IN: the game singleton is only the base of 0064AE4D's +1A0Ch owner,
    // which the stand-in Lua hooks below never read.
    void* game_00e188a8() override { return game_; }
    // The Lua object hooks. The table walk itself (globals, VehicleClass, the
    // row, NoRepairGUI) is served by the one read_vehicle_class_boolean in run()
    // under kHudShipScreenApplyUnitLuaBound; these only carry its answer. The
    // body tests is_boolean && get_boolean == 1, and the reader returns false
    // unless the key is boolean true, so answering is_boolean = get_boolean =
    // the read value takes the same arm. OFF: STAND-IN, the key reads as not a
    // boolean and +40h is shown (0064AEE2 false arm).
    void* lua_state_owner_get_globals_00b67980(void*, void* out) override { return out; }
    void* lua_object_get_by_name_00b67800(void*, void* out, const char*) override { return out; }
    void* lua_object_get_by_index_00b67720(void*, void* out, std::int32_t) override { return out; }
    void lua_object_construct_00b65f50(void*) override {}
    void lua_object_assign_00b67690(void*, void*) override {}
    void lua_object_destruct_00b67700(void*) override {}
    bool lua_object_is_boolean_00b66000(void*) override {
        if constexpr (kHudShipScreenApplyUnitLuaBound) {
            return no_repair_gui_ != 0;
        } else {
            owner_.record("HudShipScreen::apply_unit_no_repair_gui", 0x0064aee2u);
            return false;
        }
    }
    std::uint8_t lua_object_get_boolean_00b66250(void*) override {
        return no_repair_gui_ != 0 ? 1 : 0;
    }
    // vtable +34h (show) through the ship screen binding. With
    // kHudShipScreenApplyUnitWidgetsBound the name map holds +40h (the damage
    // page root), +54h, +58h and +68h..+74h; OFF they reach no widget.
    void widget_vtable34(void* widget, std::uint8_t arg) override {
        binding_.set_visible(widget_at(widget), arg != 0);
    }
    // vtable +88h, only ever on +54h (0064B023 frame +106h, 0064B0C5 frame 2).
    // Recorded under this body's own name, not the relation icon's 00AB1710
    // record that ShipScreenBinding::select_state writes: the bridge draws a
    // widget's first authored state only.
    void widget_vtable88(void* widget, std::uint32_t frame, std::int32_t arg,
                         float value) override {
        static_cast<void>(widget);
        static_cast<void>(frame);
        static_cast<void>(arg);
        static_cast<void>(value);
        owner_.record("HudShipScreen::apply_unit_recon_frame", 0x0064b023u);
    }
    float hud_ship_screen45_update_dir_icon_0064aad0(void*) override {
        return bsp::ship_screen_dir_0064aad0(binding_);
    }
    void hud_ship_screen45_update_throttle_indicator_0064a9f0(void*, float value) override {
        bsp::ship_screen_stick_0064a9f0(binding_, value);
    }
    void hud_ship_screen45_update_formation_icon_0064a960(void*) override {
        bsp::ship_screen_relation_icon_0064a960(owner_.ship_screen, binding_);
    }
    bool unit_has_flag_vtable5c(void*, std::int32_t flag) override {
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.ship_screen_unit, flag);
    }
    // STAND-IN: 00818300 sums +448h over the unit's kind-7 gun children,
    // 00852350 counts its bomb-platform children and 0059CB90 reads the HUD
    // icon from the class; the host builds none of them, so each answers 0.
    std::int32_t unit_torpedo_barrel_count_00818300(void*) override {
        owner_.record("HudShipScreen::apply_unit_torpedo_barrels", 0x00818300u);
        return 0;
    }
    std::int32_t unit_count_bomb_platform_children_00852350(void*) override {
        owner_.record("HudShipScreen::apply_unit_bomb_platforms", 0x00852350u);
        return 0;
    }
    std::uint32_t unit_hud_icon_index_0059cb90(void*, void*) override {
        owner_.record("HudShipScreen::apply_unit_hud_icon", 0x0059cb90u);
        return 0;
    }

private:
    template <typename T> static void put(std::uint8_t* base, int offset, T value) {
        std::memcpy(base + offset, &value, sizeof(T));
    }
    template <typename T> static T get(const std::uint8_t* base, int offset) {
        T value;
        std::memcpy(&value, base + offset, sizeof(T));
        return value;
    }
    bsp::ShipScreenWidget widget_at(void* widget) const {
        const auto offset = static_cast<std::uint8_t*>(widget) - widget_tags_;
        return static_cast<bsp::ShipScreenWidget>(offset);
    }

    GameHudHost::Impl& owner_;
    bsp::ShipScreenHost& binding_;
    std::uint32_t no_repair_gui_{0};  // a dword so the alignas(4) arrays need no padding
    alignas(4) std::uint8_t screen_[0x1B0]{};
    alignas(4) std::uint8_t unit_[0x988]{};
    alignas(4) std::uint8_t class_[0xD0]{};
    alignas(4) std::uint8_t gauges_[3][0x1C]{};
    std::uint8_t widget_tags_[0x80]{};
    alignas(4) std::uint8_t game_[0x1A10]{};
};
}  // namespace

void GameHudHost::Impl::apply_ship_screen_unit_0064ae20() {
    ShipScreenBinding binding(*this);
    ShipScreenApplyUnitAdapter adapter(*this, binding);
    adapter.run();
}

GuiLayoutWidget* GameHudHost::Impl::warning_widget(bsp::WarningWidget widget) {
    if (!warning_widgets_bound) {
        warning_widgets_bound = true;
        // 006823C0's lookups on GUI_Warning.
        static const std::pair<int, const char*> kNames[] = {
            {0x50, "warning_text"}, {0x54, "warning_1_Icon"}, {0x58, "warning_2_Icon"},
            {0x68, "first_Group"}};
        GuiLayoutPage* page = menu.in_game_page(0x50, "GUI_Warning");
        for (const auto& entry : kNames) {
            warning_widgets[entry.first] = page != nullptr && page->root
                ? bsp::find_descendant_by_name(*page->root, entry.second) : nullptr;
        }
    }
    auto it = warning_widgets.find(static_cast<int>(widget));
    return it != warning_widgets.end() ? it->second : nullptr;
}

namespace {
// Goal 3 chain 233 (kWireGoal3Chain233Bound). GGame's world box as 00681F40
// reads it through ECX = [00E188A8]: +711Ch the north-west corner (minimum x,
// y, maximum z) and +7128h the south-east corner (maximum x, y, minimum z), the
// layout world_map_bounds.hpp and GameMissionLuaHost::world_border_bounds
// document. 00681F40 reads nothing else of GGame, so only the box is filled.
struct GameWorldBoxImage {
    std::uint8_t before_711c[0x711C];
    float north_west[3];  // +711Ch..+7124h
    float south_east[3];  // +7128h..+7130h
};
static_assert(offsetof(GameWorldBoxImage, north_west) == 0x711C);
static_assert(offsetof(GameWorldBoxImage, south_east) == 0x7128);
static_assert(sizeof(GameWorldBoxImage) == 0x7134);

// Goal 3 chain 233. Screen 50h (GUI_Warning, vtable 00CF7184, 7Ch bytes) as the
// show routines 00682CB0/00682D60/00682E10/00682ED0 (warning_screen_show_alert)
// and their 00682800 read it: offsets from frontend_seg16_24_28_cc13.cpp kWarn*
// and the constructor 006821C0 (cg_rebuild_c_cc13.cpp). A per-show view: the
// host's WarningScreenState and its warning widgets stay the one copy, and the
// phase the body advances is written back after the call.
struct WarningScreenImage {
    std::uint8_t head_00[0x18];
    float phase[4];             // +18h..+24h, kWarnPhase
    std::uint8_t active[4];     // +28h..+2Bh
    std::uint8_t latch_2c;      // +2Ch, kWarnLatch
    std::uint8_t pad_2d[3];
    void* sample[4];            // +30h..+3Ch, kWarnSample: 006821C0's 00A83FD0 samples
    void* sound[4];             // +40h..+4Ch, kWarnSound: the tracked sounds 00682800 makes
    void* text;                 // +50h, kWarnText (warning_text)
    void* icon1;                // +54h, kWarnIcon1 (warning_1_Icon)
    void* icon2;                // +58h, kWarnIcon2 (warning_2_Icon)
    void* field_5c;             // +5Ch
    void* second_group;         // +60h
    void* third_group;          // +64h
    void* first_group;          // +68h, kWarnFirstGroup (first_Group)
    void* field_6c;             // +6Ch
    void* page;                 // +70h, kWarnPage
    std::uint32_t last_unit;    // +74h
    float dt;                   // +78h, kWarnDt
};
static_assert(offsetof(WarningScreenImage, phase) == 0x18);
static_assert(offsetof(WarningScreenImage, sample) == 0x30);
static_assert(offsetof(WarningScreenImage, sound) == 0x40);
static_assert(offsetof(WarningScreenImage, text) == 0x50);
static_assert(offsetof(WarningScreenImage, icon1) == 0x54);
static_assert(offsetof(WarningScreenImage, icon2) == 0x58);
static_assert(offsetof(WarningScreenImage, first_group) == 0x68);
static_assert(offsetof(WarningScreenImage, dt) == 0x78);
static_assert(sizeof(WarningScreenImage) == 0x7C);

// Goal 3 chain 233. The Fe5Host services warning_screen_show_alert and its
// 00682800 reach, over this process's HUD. Widget pointers are the HUD's own
// GuiLayoutWidget objects (GameHudHost::Impl::warning_widget, 006823C0's
// lookups on GUI_Warning). Every other Fe5Host service stays the base's loud
// failure (fe5_host_unreached.hpp): award_tracker_trigger_hint_00690fd0 is the
// stall show's (alert 0) only, and this host never raises a stall
// (plane_stall_007c6e10 answers false).
class WarningAlertFe5Binding final : public bsp::cc13_fe5::Fe5HostUnreached {
public:
    explicit WarningAlertFe5Binding(GameHudHost::Impl& owner) : owner_(owner) {}
    // Vtable +34h, the call WarningScreenBinding::set_visible makes; a widget
    // the page did not supply stays untouched, as there.
    void widget_set_visible_34(void* widget, bool visible) override {
        if (widget == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*static_cast<GuiLayoutWidget*>(widget), visible);
    }
    // Vtable +4Ch: the alpha over the widget's own colour, as set_alpha.
    void widget_set_alpha_4c(void* widget, float alpha) override {
        if (widget == nullptr) return;
        GuiLayoutWidget& w = *static_cast<GuiLayoutWidget*>(widget);
        owner_.menu.frontend().set_widget_color(w, w.color[0], w.color[1], w.color[2], alpha);
    }
    // 00ABBE50(text, key, 1): the key becomes the Text widget's localised
    // source through 00ABAED0, the host's set_widget_text_source. (00ABBE50's
    // case-insensitive same-source gate only skips a relayout of the same text.)
    void text_set_localised_key_00abbe50(void* text, const char* key) override {
        if (text == nullptr) return;
        owner_.menu.frontend().set_widget_text_source(*static_cast<GuiLayoutWidget*>(text),
            key != nullptr ? std::string(key) : std::string());
    }
    // 00682800. [00E1988C], the in-game GUI sound class, has no owner in this
    // host. The samples at +30h are loaded by screen 50h's constructor 006821C0
    // (00A83FD0 on the sample cache [00F8BBE8]), which now runs on the 50h leaf
    // block (construct_hud_warning_leaf) and keeps them there for the HUD's life.
    // This per-show image still carries null sample slots: without the class and
    // a home for the +40h tracked sounds across shows (the image is rebuilt per
    // call), no tracked sound is made (the host's alerts have never had sound).
    void* ingame_gui_sound_class_00e1988c() override {
        owner_.record("HudWarningScreen::gui_sound_class", 0x00e1988cu);
        return nullptr;
    }
    void* sound_create_tracked_normal(void* sample, void* sound_class) override {
        static_cast<void>(sound_class);
        if (sample != nullptr) bsp::cc13_fe5::fe5_unreached("sound_create_tracked_normal");
        owner_.record("HudWarningScreen::sound_create", 0x00682800u);
        return nullptr;
    }

private:
    GameHudHost::Impl& owner_;
};

// bsp::WarningScreenHost over this process's HUD.
class WarningScreenBinding final : public bsp::WarningScreenHost {
public:
    explicit WarningScreenBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    bool controlled_flag_5d() override {
        return owner_.units->unit_flag_005d(owner_.units->controlled_index());
    }
    bool controlled_is_kind_of(int class_id) override {
        return owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool plane_stall_007c6e10() override {
        // SUBSTITUTION: 007C6E10 on the plane's [unit+3D0h] is unread here;
        // reached only when the controlled unit is a plane.
        owner_.record("HudWarningScreen::plane_stall", 0x007c6e10u);
        return false;
    }
    bool ship_shallow_1011() override {
        // SUBSTITUTION: unit+1011h is the latched kind-8 physics contact
        // (the grounding edge 008255B0 rotates from +1010h,
        // docs/UNIT_INSTANCE_UPDATE.md); this host has no terrain contacts,
        // so the latch reads clear.
        owner_.record("HudWarningScreen::contact_latch_1011", 0x006830a5u);
        return false;
    }
    float submarine_127c() override {
        // SUBSTITUTION: unit+127Ch (the submarine's oxygen) is not modelled;
        // reached only for a submarine.
        owner_.record("HudWarningScreen::submarine_oxygen", 0x006830e6u);
        return 1.0f;
    }
    bool submarine_below_00852860() override {
        owner_.record("HudWarningScreen::submarine_below", 0x00852860u);
        return false;
    }
    bool pose_current_c8() override {
        // The host's pose is always current, so 00414DB0 has nothing to refresh.
        return true;
    }
    void refresh_pose_00414db0() override {}
    bool near_world_edge_00681f40() override {
        // 00683140..00683158, 00681F40(ECX = [00E188A8], &unit+FCh, 0.0f): ON runs
        // game_near_world_edge_00681f40 over the host's border box; OFF keeps the
        // substitution's false.
        // GGame+711Ch..+7130h is the box GameMissionLuaHost keeps from the
        // avoid-zone load (select_world_map_bounds_004d5ede, set before any
        // script runs). Before that load there is no box, and no mission
        // frame runs this update then.
        const bsp::WorldMapBounds* bounds =
            owner_.lua != nullptr ? owner_.lua->world_border_bounds() : nullptr;
        if (bounds == nullptr) {
            owner_.record("HudWarningScreen::world_edge", 0x00681f40u);
            return false;
        }
        auto game = std::make_unique<GameWorldBoxImage>();
        for (int i = 0; i < 3; ++i) {
            game->north_west[i] = bounds->north_west[static_cast<std::size_t>(i)];
            game->south_east[i] = bounds->south_east[static_cast<std::size_t>(i)];
        }
        // &[00E188D8]+FCh, the controlled unit's world translation (its pose
        // is current here, pose_current_c8).
        float position[3];
        owner_.units->unit_position_00fc(owner_.units->controlled_index(),
            position[0], position[1], position[2]);
        owner_.done("HudWarningScreen::world_edge", 0x00681f40u);
        return bsp::cc13_fe5::game_near_world_edge_00681f40(
            bsp::cc13_fe5::Raw{reinterpret_cast<std::uint8_t*>(game.get())}, position, 0.0f);
    }
    void set_visible(bsp::WarningWidget widget, bool visible) override {
        GuiLayoutWidget* w = owner_.warning_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    void set_alpha(bsp::WarningWidget widget, float alpha) override {
        GuiLayoutWidget* w = owner_.warning_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, w->color[0], w->color[1], w->color[2], alpha);
    }
    void sound_stop(int alert, bool flag) override {
        static_cast<void>(alert);
        static_cast<void>(flag);
        owner_.record("HudWarningScreen::sound_stop", 0x006831b5u);
    }
    bool sound_finished(int alert) override {
        static_cast<void>(alert);
        owner_.record("HudWarningScreen::sound_finished", 0x0068321du);
        return false;
    }
    void sound_release(int alert) override {
        static_cast<void>(alert);
        owner_.record("HudWarningScreen::sound_release", 0x0068322fu);
    }
    void show_alert(bsp::WarningScreenState& screen, int alert) override {
        static const std::uint32_t kShow[4] = {0x00682ed0u, 0x00682cb0u, 0x00682d60u,
                                               0x00682e10u};
        // 00683282's jump table to 00682ED0/00682CB0/00682D60/00682E10: ON runs
        // warning_screen_show_alert (phase += dt, first_Group shown, the alert's
        // key, the pulsed alpha on the three widgets, 00682800) over the HUD's
        // warning widgets; OFF keeps the record.
        const std::size_t i = static_cast<std::size_t>(alert & 3);
        WarningScreenImage image{};
        for (std::size_t k = 0; k < 4; ++k) {
            image.phase[k] = screen.phase[k];
            // A non-null +40h slot is a tracked sound 00682800 made; this host
            // has made none (its samples are null), so a set flag has no handle.
            if (screen.sound[k]) bsp::cc13_fe5::fe5_unreached("warning sound handle");
        }
        image.dt = screen.dt_78;
        image.text = owner_.warning_widget(bsp::WarningWidget::Text);
        image.icon1 = owner_.warning_widget(bsp::WarningWidget::Icon1);
        image.icon2 = owner_.warning_widget(bsp::WarningWidget::Icon2);
        image.first_group = owner_.warning_widget(bsp::WarningWidget::FirstGroup);
        WarningAlertFe5Binding fe5(owner_);
        bsp::cc13_fe5::warning_screen_show_alert(
            bsp::cc13_fe5::Raw{reinterpret_cast<std::uint8_t*>(&image)}, alert & 3, fe5);
        screen.phase[i] = image.phase[i];
        screen.sound[i] = image.sound[i] != nullptr;
        owner_.done("HudWarningScreen::show_alert", kShow[i]);
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

namespace {
// bsp::FollowScreen49Host over this process's HUD.
class FollowScreen49Binding final : public bsp::FollowScreen49Host {
public:
    explicit FollowScreen49Binding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool screen_29h_applied() override { return owner_.menu.in_game_screen_applied(0x29); }
    std::size_t screen_29h_unit() override {
        // Screen 29h's +4Ch, stored by its update (005272E5, from 00526A40).
        if (kHudUnitPickScreenBound) return owner_.unit_pick.pick_4c;
        // SUBSTITUTION with 29h unbound: it reads null.
        owner_.record("HudFollowScreen::screen_29h_unit", 0x0067bf44u);
        return 0;
    }
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    std::size_t controlled_target_00927880() override {
        // SUBSTITUTION: 00927880 goes through the unit's vtable +114h and
        // that object's +18h, neither read; it answers none.
        owner_.record("HudFollowScreen::controlled_target", 0x00927880u);
        return 0;
    }
    bool is_kind_of(std::size_t unit, int class_id) override {
        return owner_.units != nullptr && unit != 0
            && owner_.units->unit_is_kind_of(unit - 1, class_id);
    }
    bool alive_and_visible(std::size_t unit) override {
        return owner_.units != nullptr && unit != 0
            && owner_.units->unit_alive_and_visible(unit - 1);
    }
    bool leader_3d0_alive(std::size_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudFollowScreen::leader_3d0", 0x0067bfafu);
        return false;
    }

private:
    GameHudHost::Impl& owner_;
};
}  // namespace

namespace {
// bsp::ShipViewScreen46Host over this process's HUD.
class ShipViewScreen46Binding final : public bsp::ShipViewScreen46Host {
public:
    ShipViewScreen46Binding(GameHudHost::Impl& owner, bool wanted)
        : owner_(owner), wanted_(wanted) {}
    bool wanted_04() override { return wanted_; }
    bool has_unit_1c() override { return owner_.ship_view_has_unit; }
    void view_input_0064a400(float dt) override;
    void integrated_controls_0064b870(float dt) override;
    bool screen_2eh_present() override {
        // [00E198C4]+50h is slot 2Eh's screen, which the registry holds.
        return owner_.menu.in_game_screen_registered(0x2e);
    }
    void screen_2eh_005484f0() override {
        owner_.menu.note_screen_2eh_ran();   // the helm `input` line's lost-edge report
        if constexpr (kHudWeaponGroupScreenBound) {
            WeaponGroupScreenBinding group_host(owner_);
            bsp::weapon_group_screen_update_005484f0(owner_.weapon_group, group_host);
            owner_.done("HudWeaponGroupScreen::update", 0x005484f0u);
        } else {
            owner_.record("HudShipView::screen_2eh_005484f0", 0x005484f0u);
        }
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    bool unit_virtual_234() override {
        owner_.record("HudShipView::unit_virtual_234", 0x0064d697u);
        return false;
    }
    // 00927F30(unit, 0): the controlled unit is the local player's ship.
    bool unit_local_player() override { return true; }
    bool unit_is_kind_of(int class_id) override {
        return owner_.units != nullptr && owner_.units->controlled_bound()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), class_id);
    }
    bool unit_00812960() override {
        owner_.record("HudShipView::unit_00812960", 0x00812960u);
        return false;
    }
    void order_route_0077d600() override {
        owner_.record("HudShipView::order_route_0077d600", 0x0077d600u);
    }
    void order_route_0077c2a0() override {
        owner_.record("HudShipView::order_route_0077c2a0", 0x0077c2a0u);
    }

private:
    GameHudHost::Impl& owner_;
    bool wanted_;
};
}  // namespace

namespace {
// bsp::IntegratedControlsHost over this process's HUD.
class IntegratedControlsBinding final : public bsp::IntegratedControlsHost {
public:
    explicit IntegratedControlsBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_byte_6c8() override {
        // The unit constructor stores 1 at 0095CE29 and a disp32 scan finds no
        // other byte writer of +6C8h, so the gate reads set.
        return true;
    }
    bsp::IntegratedControlsInputs inputs() override {
        // SUBSTITUTION: the input manager (004BEC00) axes +1BE4h/+1BB4h, the
        // two binding queries on +1B90h and the byte +1B91h are not modelled;
        // no in-mission input is driven, so they read zero and clear.
        owner_.record("HudShipView::control_inputs", 0x004bec00u);
        return {};
    }
    bool unit_1130_clear() override {
        owner_.record("HudShipView::unit_1130", 0x0064b97du);
        return true;
    }
    bool local_player_role(int role) override {
        if (kHudShipViewRoleTableBound) {
            // 00927F30(unit, role): the role's holder at unit+1ACh is the
            // local player's slot, game+18ECh, 0 in this single-player host.
            // The units host owns that table (the 27h take 0067BB50 and the
            // BSP_PLAYER_HELM transfer write it). With role 1 held the branch
            // below reaches issue_order, which is a record: the helm option's
            // own 00816A40 issue stays the only one.
            std::int32_t holder = -1;
            return owner_.units != nullptr && owner_.units->controlled_bound()
                && owner_.units->unit_current_role_slot(owner_.units->controlled_index(),
                                                        role, holder)
                && holder == 0;
        }
        // SUBSTITUTION: role 0 held, role 1 not, until an input-started
        // transfer (docs/CONTROLLED_UNIT_HELM.md section 6).
        owner_.record("HudShipView::local_player_role", 0x00927f30u);
        return role == 0;
    }
    void role_transfer_0077c470(int mask, int take) override {
        static_cast<void>(mask);
        static_cast<void>(take);
        owner_.record("HudShipView::role_transfer", 0x0077c470u);
    }
    float unit_ordered_rudder() override {
        const GameUnitRow* row = controlled_row();
        return row != nullptr ? row->ordered_rudder : 0.0f;
    }
    float unit_throttle() override {
        const GameUnitRow* row = controlled_row();
        return row != nullptr ? row->throttle : 0.0f;
    }
    void issue_order(float thrust, float turn) override {
        static_cast<void>(thrust);
        static_cast<void>(turn);
        owner_.record("HudShipView::issue_order", 0x00816a40u);
    }
    bool game_19c4() override {
        owner_.record("HudShipView::game_19c4", 0x0064bb2cu);
        return false;
    }

private:
    const GameUnitRow* controlled_row() const {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return nullptr;
        return owner_.units->unit_row(owner_.units->controlled_index());
    }
    GameHudHost::Impl& owner_;
};

// bsp::ShipViewInputHost over this process's HUD and the mission camera's
// mover.
class ShipViewInputBinding final : public bsp::ShipViewInputHost,
    public bsp::cc13_seg14::BinocularsToggleHooks,
    public bsp::cc13_seg14::BinocularsResetHooks,
    public bsp::cc13_sweepA3::BinocularsAxesHost,
    public bsp::segs_13_15_22_44_cc13::BinocularsFovHost {
public:
    ShipViewInputBinding(GameHudHost::Impl& owner, float dt = 0.0f) : owner_(owner), dt_(dt) {}
    bsp::BinocularsViewTerms view_terms() override {
        // The raise axis is the application's actual input-action bank. The
        // look axes/config retain the existing helm-aim source binding below.
        owner_.record("HudShipView::view_input_terms", 0x0051f061u);
        bsp::BinocularsViewTerms terms{};
        terms.config_04 = 1.0f;
        terms.raise_axis_15e4 = binoculars_input_bank().at<float>(0x15e4);
        helm_aim_axes(terms);
        return terms;
    }
    // Packet cc13-helmaim: the helm `aim` line drives the look axes the way a
    // mouse move does. Each axis is the one that makes 0051F050's step land the
    // mover on the target: yaw += step * -1.0 * zoom * axis * config (0051F159)
    // and pitch += step * 0.5 * zoom * axis * config, wrapped, then clamped into
    // [+3ECh, +3F0h] (0051E650). 0051F050 clamps each axis into [-50, 50], so a
    // long turn takes more than one frame. The target clears once both angles are
    // there, and the mover keeps them: no reader decays +384h / +388h.
    void helm_aim_axes(bsp::BinocularsViewTerms& terms) {
        float yaw = 0.0f;
        float pitch = 0.0f;
        const bool tracking = helm_aim_track_target(yaw, pitch);
        if (!tracking && !injected_view_aim(yaw, pitch)) return;
        if (!owner_.camera_bound) return;
        const float low = owner_.camera.min_pitch_3ec;
        const float high = owner_.camera.max_pitch_3f0;
        if (low > pitch) pitch = low;
        else if (pitch > high) pitch = high;
        const auto wrap = [](double a) {
            constexpr double kPi = 3.141592653589793;
            while (a > kPi) a -= 2.0 * kPi;
            while (a <= -kPi) a += 2.0 * kPi;
            return a;
        };
        const double dyaw = wrap(static_cast<double>(yaw) - owner_.camera.yaw_384);
        const double dpitch = wrap(static_cast<double>(pitch) - owner_.camera.pitch_388);
        constexpr double kReached = 1.0e-4;
        if (tracking) {
            // `aim unit` never completes; it steers again next frame.
            if (std::fabs(dyaw) < kReached && std::fabs(dpitch) < kReached) return;
        } else if (std::fabs(dyaw) < kReached && std::fabs(dpitch) < kReached) {
            clear_injected_view_aim();
            owner_.log.notef("helm aim reached: mover yaw %.4f pitch %.4f rad (+384h / +388h)",
                static_cast<double>(owner_.camera.yaw_384),
                static_cast<double>(owner_.camera.pitch_388));
            return;
        }
        const double step = static_cast<double>(dt_) > 0.5 ? 0.5 : static_cast<double>(dt_);
        const double zoom = owner_.binoculars.zoom_38;
        if (!(step > 0.0) || !(zoom > 0.0)) return;
        terms.yaw_axis_1584 = static_cast<float>(dyaw / (step * -1.0 * zoom * terms.config_04));
        terms.pitch_axis_15b4 = static_cast<float>(dpitch / (step * 0.5 * zoom * terms.config_04));
    }
    // Packet cc13-aimunit: the `aim unit` track. The look is steered in closed
    // loop on the camera's published view (the forward row and position 00954A10
    // reads for message 79h, i.e. the view ray after the orbit position, zoom and
    // shake, which 009578C3 tests), so it needs no model of +384h's frame:
    // 00432ED0 builds the forward row as rot_y(-yaw) then rot_x(-pitch) on the
    // mover's basis, so a heading error e (atan2(x, z) convention) is removed by
    // yaw -= e, and a pitch error by pitch += e. The auto pitch aims at the unit's
    // origin (unit+0FCh). Returns false (no steering) when there is no track or
    // nothing to steer from. A row that becomes unusable is reported once per
    // loss (the look keeps its last yaw) and its return is reported too.
    bool helm_aim_track_target(float& yaw, float& pitch) {
        std::size_t unit = 0;
        bool has_pitch = false;
        float fixed_pitch = 0.0f;
        if (!injected_view_aim_track(unit, has_pitch, fixed_pitch)) return false;
        const unsigned serial = injected_view_aim_track_serial();
        if (serial != owner_.aim_track_serial) {
            // A new `aim unit` line: its own loss report and its own first frames.
            owner_.aim_track_serial = serial;
            owner_.aim_track_warned = false;
            owner_.aim_track_frames = 0;
        }
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!owner_.camera_bound || !node.ready) return false;
        if (owner_.units == nullptr || unit >= owner_.units->count()
            || !owner_.units->unit_alive_and_visible(unit)) {
            if (!owner_.aim_track_warned) {
                owner_.aim_track_warned = true;
                owner_.log.notef("helm aim unit: row %zu is gone or hidden; the look keeps its "
                    "last yaw", unit);
            }
            return false;
        }
        if (owner_.aim_track_warned) {
            owner_.aim_track_warned = false;
            owner_.log.notef("helm aim unit: row %zu is back; tracking resumes", unit);
        }
        // As the 79h build reads it (00548839..005489DA).
        if ((node.state.transform.valid_flags & 2u) == 0) {
            bsp::refresh_camera_world_00b6db70(node.state.transform);
        }
        const bsp::CameraMatrix& world = node.state.transform.world;
        float tx = 0.0f, ty = 0.0f, tz = 0.0f;
        owner_.units->unit_position_00fc(unit, tx, ty, tz);
        // The auto pitch's aim point. A ship (kind 6) is aimed at the image's
        // line-of-sight target point, 00864D90: the pose +0FCh raised at
        // 00864E17 by GlobalConfig+90h, LOSTargetHeightAdd (5.0 on this
        // installation), plus the class Height (+A8h) from the IsKindOf(5) arm at
        // 00864E25. The image applies +90h to every target and gates the Height
        // on IsKindOf(5); restricting the point to kind 6 is this harness's
        // choice. UNCERTAIN: when class+50h is non-empty the image's height term
        // is max(+90h, (bbox+38h - +2Ch) * +98h) through 00415550; that mesh arm
        // is not modelled here. The seat ray's hit must lie above y = 0
        // (00957891: COMISS hit.y, [00D7A218] 0.0f; JBE drops it), and aiming at
        // the origin (the waterline) put every hull hit at or below it.
        // Other units keep their origin.
        double aim_y = ty;
        if (owner_.units->unit_is_kind_of(unit, bsp::kUnitGunneryKindShipBase)) {
            float length = 0.0f, width = 0.0f, height = 0.0f;
            owner_.units->unit_class_extents(unit, length, width, height);
            aim_y = static_cast<double>(ty) + height + bsp::kInstalledLosTargetHeightAdd;
            // HARNESS clamp, not in the image: a ship origin below 0 (heave,
            // sinking) never pulls the aim point below y = 5.0.
            if (aim_y < bsp::kInstalledLosTargetHeightAdd) aim_y = bsp::kInstalledLosTargetHeightAdd;
        }
        const double dx = static_cast<double>(tx) - world[12];
        const double dy = aim_y - world[13];
        const double dz = static_cast<double>(tz) - world[14];
        // Reuse 00401CF0 on the live camera-to-target planar delta. Zero Y
        // is this caller's existing horizontal projection, before its pitch
        // calculation below. The squared length now honors the main thread's
        // x87 PC24 setting; no original 00401CF0 call at this site is claimed.
        const double flat = std::sqrt(bsp::length_squared_00401cf0({dx, 0.0, dz}));
        if (!(flat > 1.0e-3)) return false;
        // Goal 5 chain 730: reuse the existing direction-error body on the
        // published camera forward row and this live target's aim direction.
        // bsp.py show 00605630 --asm, 00605630..006057DA: normalize both
        // vectors, wrap target-current yaw and negate target-current pitch.
        // Its existing 00794750 delegate has the same arithmetic (see
        // cc13_left1.cpp); the helm's pitch convention therefore subtracts
        // that output too. This replaces the hand-written atan2/asin path.
        // The aim-unit input/target-height policy above remains a host policy;
        // no original plane-HUD receiver or camera ABI is implied by reuse.
        const float forward[3]{world[8], world[9], world[10]};
        const float direction[3]{static_cast<float>(dx), static_cast<float>(dy),
            static_cast<float>(dz)};
        float e_heading, e_pitch;
        bsp::cc13_left1::direction_angle_deltas_00605630(
            &e_heading, &e_pitch, forward, direction);
        yaw = static_cast<float>(static_cast<double>(owner_.camera.yaw_384) - e_heading);
        if (has_pitch) {
            pitch = fixed_pitch;
        } else {
            pitch = static_cast<float>(static_cast<double>(owner_.camera.pitch_388) - e_pitch);
        }
        // Every one of a track's first 10 frames (a 2-frame oscillation shows),
        // then every 200th.
        const unsigned long long frame = owner_.aim_track_frames++;
        if (frame < 10 || frame % 200 == 0) {
            owner_.log.notef("helm aim unit: row %zu heading error %.4f rad, target pitch %.4f "
                "rad, range %.0f", unit, static_cast<double>(e_heading),
                static_cast<double>(pitch), flat);
        }
        return true;
    }
    bsp::cc13_sweepA::Raw binoculars_input_bank() override {
        void* manager = owner_.menu.input_context_manager().current_owner();
        const auto count = read<std::uint32_t>(manager, 8);
        if (count <= 0xe0u) throw std::logic_error("binoculars requires prepared native action bank");
        return {static_cast<std::uint8_t*>(read<void*>(manager, 4))};
    }
    bool input_pressed(int action) override {
        const auto bank = binoculars_input_bank();
        const auto* record = bank.field(static_cast<std::size_t>(action) * 0x30);
        bsp::InputActionRecord state;
        state.current_down = read<std::uint8_t>(record, 0x28) != 0;
        state.current_hold = read<float>(record, 0x24);
        state.previous_down = read<std::uint8_t>(record, 0x20) != 0;
        state.previous_hold = read<float>(record, 0x1c);
        return bsp::action_pressed_this_frame_004c43c0(state)
            || owner_.menu.input_action_pressed(action);
    }
    void raise_toggle_0051e7e0(bsp::BinocularsState& screen) override {
        // 0051EF6B..EFA6: the controlled unit gates precede the CALL. The
        // ship view has no kind-46h weapon-view owner; that arm stays recorded.
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return;
        const auto unit = owner_.units->controlled_index();
        if (owner_.units->unit_is_kind_of(unit, 0x18)) return;
        if (owner_.units->unit_is_kind_of(unit, 0x46)) {
            owner_.record("HudShipView::weapon_view_raise_gate", 0x0051ef90u);
            return;
        }
        bsp::cc13_seg14::binoculars_toggle_zoom_0051e7e0(&screen,
            owner_.units->controlled_unit_publication(), owner_.binoculars_saved_near_plane, *this);
        owner_.done("HudShipView::binoculars_raise_toggle", 0x0051e7e0u);
    }
    void raised_view(bsp::BinocularsState& screen, float dt) override {
        // 0051EFD1..EFDF: ECX=&+38h, EDX=&+28h; the three byte
        // arguments are the screen's retained +2Ch/+2Dh/+2Eh, not frame flags.
        bsp::cc13_sweepA3::hud_binoculars_step_input_axes_0051eaa0(
            &screen.zoom_38, &screen.zoom_limit_28, screen.zoom_divisor_3c, dt,
            &screen.zoom_active_2c, &screen.zoom_rising_2d, &screen.zoom_falling_2e, 0, *this);
        binoculars_apply_fov_0051e6e0(&screen);
        set_model_visible(true);
        // The renderer's lens-flare owner has no source implementation here.
        owner_.record("HudShipView::binoculars_lens_update", 0x00452bd0u);
        owner_.done("HudShipView::binoculars_raised_view", 0x0051efb7u);
    }
    bool object_is_kind_virtual_5c(void* object, std::uint32_t kind) override {
        return owner_.units != nullptr && owner_.units->controlled_bound()
            && object == owner_.units->controlled_unit_publication()
            && owner_.units->unit_is_kind_of(owner_.units->controlled_index(), kind);
    }
    void* binoculars_camera(void* mover) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars received another mover");
        return owner_.camera_bound ? &owner_.camera_projection : nullptr;
    }
    float binoculars_near_plane(void* camera) override {
        if (camera != &owner_.camera_projection) throw std::logic_error("binoculars received another camera");
        return owner_.camera_projection.near_plane;
    }
    void camera_set_near_plane_00b6fbf0(void* camera, float value) override {
        if (camera != &owner_.camera_projection) throw std::logic_error("binoculars set another camera");
        owner_.camera_projection.near_plane = value;
        bsp::update_mission_camera_projection(owner_.camera_projection);
    }
    void binoculars_apply_fov_0051e6e0(void* raw_screen) override {
        bsp::segs_13_15_22_44_cc13::binoculars_apply_fov_0051e6e0(
            {static_cast<std::uint8_t*>(raw_screen)}, *this);
    }
    void binoculars_set_view_fov(float scale, std::int32_t index) override {
        owner_.set_view_fov(scale, index);
    }
    void binoculars_set_mover_zoom(void* mover, bool enabled) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars zoomed another mover");
        owner_.camera.zoom_enabled_3a0 = enabled;
    }
    float& binoculars_mover_pitch(void* mover) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars pitched another mover");
        return owner_.camera.pitch_388;
    }
    float binoculars_mover_min_pitch(void* mover) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars read another mover limit");
        return owner_.camera.min_pitch_3ec;
    }
    float binoculars_mover_max_pitch(void* mover) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars read another mover limit");
        return owner_.camera.max_pitch_3f0;
    }
    void unit_virtual_dc(void* mover, float delta) override {
        if (mover != &owner_.camera) throw std::logic_error("binoculars turned another mover");
        // ShipCaptain's +DCh = 0051E620: 00438AA0 on yaw+384h.
        owner_.camera.yaw_384 = bsp::wrapped_angle_add_00438aa0(owner_.camera.yaw_384, delta);
    }
    void unit_broadcast_controlled_part_flag_00817380(void*, std::uint32_t) override {
        // Same unresolved provider as SelectControlledUnitBinding: the host
        // has no unit+FFCh effect-group list. Do not invent an empty list.
        owner_.record("HudShipView::binoculars_controlled_part_flag", 0x00817380u);
    }
    bool input_action_has_device_query_00a92090(void* action) override {
        auto& devices = owner_.menu.input_context_manager().configuration.bindings.devices;
        return bsp::query_native_input_action_relative_00a92090(action, devices) != 0;
    }
    bool binoculars_zoom_pressed() override { return input_pressed(0xe0); }
    void math_step_towards_0042ac60(float* value, float target, float rate) override {
        *value = bsp::unit_step_towards_0042ac60(*value, target, rate);
    }
    void binoculars_reset_overlay() override {
        // 0051EC65..EC92: use the actual screen at root+84h and its +20h
        // widget. These allocations belong to the HUD registry/page loader.
        const auto index = bsp::in_game_hud_screen_index_at(0x84);
        auto* leaf = index < kInGameHudScreenCount ? owner_.leaf_screens[index].get() : nullptr;
        if (leaf == nullptr) return;
        leaf->data()[0x1c] = 1;
        auto* widget = read<GuiLayoutWidget*>(leaf->data(), 0x20);
        if (widget != nullptr) owner_.menu.frontend().set_widget_visible(*widget, false);
    }
    void set_model_visible(bool visible) override {
        // Screen 26h's +20h, the Tavcso_Model widget of GUI_binoculars.
        auto* w = static_cast<GuiLayoutWidget*>(binoculars_model_widget(&owner_.binoculars));
        if (w != nullptr) owner_.menu.frontend().set_widget_visible(*w, visible);
    }
    bool binoculars_screen_applied(void* screen) override {
        if (screen != &owner_.binoculars) throw std::logic_error("binoculars reset another screen");
        return owner_.menu.in_game_screen_applied(0x26);
    }
    void* binoculars_model_widget(void* screen) override {
        if (screen != &owner_.binoculars) throw std::logic_error("binoculars read another model");
        GuiLayoutPage* page = owner_.menu.in_game_page(0x26, "GUI_binoculars");
        return page != nullptr && page->root
            ? bsp::find_descendant_by_name(*page->root, "Tavcso_Model") : nullptr;
    }
    void widget_show_virtual_34(void* widget, std::uint32_t flag) override {
        if (widget != nullptr)
            owner_.menu.frontend().set_widget_visible(*static_cast<GuiLayoutWidget*>(widget), flag != 0);
    }
    void lens_flare_set_enabled_00452b80(void* lens, std::uint32_t flag) override {
        if (lens != reinterpret_cast<void*>(0x00e081a0u) || flag != 0)
            throw std::logic_error("binoculars reset requested another lens operation");
        lens_effect_off_00452b80();
    }
    void lens_effect_off_00452b80() override {
        // 00B0D020 on the renderer object [00F8D39C] is not modelled.
        owner_.record("HudShipView::binoculars_lens_off", 0x00452b80u);
    }
    void* mover_object() override { return owner_.camera_bound ? &owner_.camera : nullptr; }
    float mover_yaw() override { return owner_.camera.yaw_384; }
    void set_mover_yaw(float yaw) override { owner_.camera.yaw_384 = yaw; }
    float mover_pitch() override { return owner_.camera.pitch_388; }
    void set_mover_pitch(float pitch) override { owner_.camera.pitch_388 = pitch; }
    float mover_min_pitch() override { return owner_.camera.min_pitch_3ec; }
    float mover_max_pitch() override { return owner_.camera.max_pitch_3f0; }
    void view_arm(bsp::BinocularsState& screen, float dt) override {
        static_cast<void>(screen);
        static_cast<void>(dt);
        owner_.record("HudShipView::binoculars_view_arm", 0x0051f1c0u);
    }
    void screen_2eh_005454b0(float a, float b, float c) override {
        static_cast<void>(a);
        static_cast<void>(b);
        static_cast<void>(c);
        // Screen 2Eh's +48h..+50h and screen 4Dh's +44h/+48h have no reader
        // in this host.
        owner_.record("HudShipView::screen_2eh_005454b0", 0x005454b0u);
    }
    bool ship_view_mover_20() override { return owner_.camera_bound; }

private:
    template<class T> static T read(const void* object, std::size_t offset) {
        T value;
        std::memcpy(&value, static_cast<const std::uint8_t*>(object) + offset, sizeof value);
        return value;
    }
    GameHudHost::Impl& owner_;
    float dt_;
};

void ShipViewScreen46Binding::view_input_0064a400(float dt) {
    if (!kHudShipViewInputBound) {
        // Screen 26h's 0051F330 and screen 2Eh's 005454B0, not bound.
        owner_.record("HudShipView::view_input", 0x0064a400u);
        return;
    }
    ShipViewInputBinding binding(owner_, dt);
    bsp::ship_view_input_0064a400(owner_.binoculars, binding, dt);
    owner_.done("HudShipView::view_input_0064a400", 0x0064a400u);
}

void ShipViewScreen46Binding::integrated_controls_0064b870(float dt) {
    if (!kHudShipViewControlsBound) {
        // BSP_HudUnitOrder_UpdateIntegratedControls, not bound.
        owner_.record("HudShipView::integrated_controls", 0x0064b870u);
        return;
    }
    IntegratedControlsBinding binding(owner_);
    bsp::integrated_controls_0064b870(owner_.ship_view_controls, binding, dt);
    owner_.done("HudShipView::integrated_controls_0064b870", 0x0064b870u);
}
}  // namespace

void GameHudHost::Impl::reset_binoculars_0051e8e0() {
    // 0051E8E0 has no frame/input arguments: this binding only reads the
    // retained screen, registry visibility and the existing camera projection.
    ShipViewInputBinding binding(*this);
    bsp::cc13_seg14::binoculars_reset_0051e8e0(&binoculars, binding);
    done("InGameInterface::binoculars_reset", 0x0051e8e0u);
}

void GameHudHost::update_ship_view_screen_0064d610(float seconds, bool wanted) {
    Impl& impl = *impl_;
    ShipViewScreen46Binding binding(impl, wanted);
    bsp::ship_view_update_0064d610(binding, seconds);
    impl.done("HudShipView::update", 0x0064d610u);
}

// 0067BB50, slot 20h of vtable 00CF7A38 (HUD page 27h). The pump only calls
// it while the page's +4h and +5h are set, so the routine's +4h test at
// 0067BBE0 holds on this path. The units host owns the take.
void GameHudHost::update_role_screen_0067bb50() {
    Impl& impl = *impl_;
    if (impl.units == nullptr) {
        impl.log.unimplemented("HudRoleScreen::update_no_units", "0067bb50");
        return;
    }
    impl.units->role_screen_update_0067bb50();
    impl.done("HudRoleScreen::update", 0x0067bb50u);
}

void GameHudHost::enter_weapon_group_screen_005494c0() {
    Impl& impl = *impl_;
    WeaponGroupScreenBinding host(impl);
    WeaponGroupLayoutBinding gui(impl);
    bsp::weapon_group_screen_enter_005494c0(impl.weapon_group, host, gui);
    impl.done("HudWeaponGroupScreen::bind", 0x00549260u);
    impl.done("HudWeaponGroupScreen::enter", 0x005494c0u);
}

void GameHudHost::exit_weapon_group_screen_005470a0() {
    Impl& impl = *impl_;
    WeaponGroupScreenBinding host(impl);
    WeaponGroupLayoutBinding gui(impl);
    bsp::weapon_group_screen_exit_005470a0(impl.weapon_group, host, gui);
    impl.done("HudWeaponGroupScreen::exit", 0x005470a0u);
}

void GameHudHost::update_follow_screen_0067bf00() {
    Impl& impl = *impl_;
    FollowScreen49Binding binding(impl);
    bsp::follow_screen_update_0067bf00(impl.follow_screen, binding);
    impl.done("HudFollowScreen::update", 0x0067bf00u);
}

void GameHudHost::update_warning_screen_00683020(float seconds) {
    Impl& impl = *impl_;
    WarningScreenBinding binding(impl);
    bsp::warning_screen_update_00683020(impl.warning_screen, binding, seconds);
    impl.done("HudWarningScreen::update", 0x00683020u);
}

namespace {
// 00644DB0's callback surface over the same lists/cursor used by 00648290
// and 006485A0. Handles remain unit index + 1, with zero representing null.
class HudRootSelectionBinding final : public bsp::HudRootSelectionHost {
public:
    explicit HudRootSelectionBinding(GameHudHost::Impl& owner)
        : owner_(owner), lists_(owner) {}
    bool unit_is_group_leader(std::uint32_t unit) override {
        return lists_.unit_is_group_leader_00778890(unit);
    }
    bool unit_is_group_member(std::uint32_t unit) override {
        return lists_.unit_is_group_member_007788b0(unit);
    }
    std::uint32_t group_leader(std::uint32_t unit) override {
        return lists_.group_leader_007788d0(unit);
    }
    bool input_action_pressed(int action) override {
        return owner_.menu.input_action_pressed(action);
    }
    bool base_screen_active() override {
        // 0068A140 reads the manager's last pending id at +20h, including
        // after application. Preserve that owner here, not the applied id
        // in summary (bsp.py show 0068a140).
        return bsp::is_base_interface_0068a140(owner_.record_pending_id_20);
    }
    bool panel_54_active() override {
        // Manager+54h is registry slot 4Ch; +5h is its applied byte.
        return owner_.menu.in_game_screen_applied(0x4c);
    }
    std::uint32_t controlled_unit() override { return lists_.controlled_unit(); }
    bool unit_virtual_124(std::uint32_t unit) override {
        return unit != 0 && owner_.unit_vtable_124(unit - 1, 0x00644e7eu);
    }
    std::uint32_t selected_unit() override {
        return bsp::hud_root_cursor_unit_00644a60(owner_.root_lists);
    }

private:
    GameHudHost::Impl& owner_;
    HudRootListsBinding lists_;
};

// Goal 4 chain 291: bsp::HudRootRowsHost over this process's HUD, for 00648C20
// (hud_root_update_rows), which 00649860 calls every frame. Units are index + 1
// as in HudRootListsBinding. The text, alpha and visibility writes reach the
// frontend; the icon state select (virtual +88h), the scale pair (virtual +48h)
// and the WeaponInfo animation (00AA8B00 / 00AC2F20) have no bridge operation,
// so they are records, as on the ship screen. Each input this host cannot
// source is a labelled record; the ones that only feed a recorded write are
// noted as such. docs/HUD_ROOT_UNIT_ROWS.md.
class HudRootRowsBinding final : public bsp::HudRootRowsHost {
public:
    explicit HudRootRowsBinding(GameHudHost::Impl& owner) : owner_(owner), lists_(owner) {}

    void rebuild_unit_lists() override {
        // 00648C2C: 00648290 on the root, as the enter 006488D0 runs it, but
        // without run_root_lists's per-call log lines (this is every frame).
        bsp::hud_root_rebuild_unit_lists_00648290(owner_.root_lists, lists_);
        ++owner_.root_rebuilds;
    }
    std::uint32_t controlled_unit() override { return lists_.controlled_unit(); }
    std::uint32_t selected_unit() override {
        return bsp::hud_root_cursor_unit_00644a60(owner_.root_lists);
    }
    bool panel_byte(std::uint32_t manager_offset, std::uint32_t byte_offset) override {
        // [00E198C4]+N is the screen 0068CC70 stores at manager offset N; its
        // registry slot comes from kInGameHudScreens (src/ingame_interface.cpp):
        // +50h is slot 2Eh, +54h slot 4Ch, +6Ch slot 3Fh, +74h slot 41h. +5h is
        // the applied byte, +4h the requested byte 004F7620 writes.
        const std::size_t at = bsp::in_game_hud_screen_index_at(
            static_cast<std::uint16_t>(manager_offset));
        if (at >= bsp::kInGameHudScreenCount) return false;
        const int slot = bsp::kInGameHudScreens[at].registry_slot;
        if (byte_offset == 5) return owner_.menu.in_game_screen_applied(slot);
        if (byte_offset == 4) return owner_.menu.in_game_screen_wanted(slot);
        // 00648D0F: [manager+6Ch]+BDh, read only after +5h of the same screen
        // (slot 3Fh) was found set. SUBSTITUTION, labelled: this host keeps no
        // slot-3Fh screen object, so the byte reads clear.
        owner_.record("HudRootRows::panel_6c_bd", 0x00648d0fu);
        return false;
    }
    std::uint32_t panel_74_override_unit() override {
        // 00648C66..00648C82: [manager+74h] (slot 41h) applied, then its +20h
        // object's +9D4h. SUBSTITUTION, labelled: the slot-41h screen's +20h
        // object is not modelled; while the screen is not applied the image
        // takes no override either.
        if (!owner_.menu.in_game_screen_applied(0x41)) return 0u;
        owner_.record("HudRootRows::panel_74_override_unit", 0x00648c74u);
        return 0u;
    }
    int panel_50_weapon_mode() override {
        // 00649017: [[manager+50h]+44h] through the 0..5 jump table, screen
        // 2Eh's selected weapon group, which 005494C0 / 005484F0 / 005470A0
        // keep in this host's weapon_group state.
        return owner_.weapon_group.group_44;
    }
    bool unit_is_kind(std::uint32_t unit, int kind) override {
        return unit != 0 && owner_.units != nullptr && unit <= owner_.units->count()
            && owner_.units->unit_is_kind_of(unit - 1, kind);
    }
    std::uint32_t first_member(std::uint32_t unit) override {
        // [unit+3D0h], read on kind-18h (squadron) units only: the squadron's
        // flight leader, which the list-24 node answers (unit count: none).
        if (unit == 0 || owner_.units == nullptr || unit > owner_.units->count()) return 0u;
        const std::size_t leader = owner_.units->squadron_list_24_leader(unit - 1);
        return leader < owner_.units->count() ? static_cast<std::uint32_t>(leader + 1) : 0u;
    }
    // Reached only behind panel_byte(6Ch, 5) && panel_byte(6Ch, BDh).
    bool weapon_probe(std::uint32_t, bsp::HudWeaponProbe) override {
        owner_.record("HudRootRows::weapon_probe", 0x007b9320u);
        return false;
    }
    std::int32_t weapon_count(std::uint32_t, bsp::HudWeaponCount kind) override {
        // Payload / Rocket behind the 6Ch gate above; SubmarineTorpedo behind
        // weapon mode 4, which panel_50_weapon_mode never answers here.
        owner_.record("HudRootRows::weapon_count",
            kind == bsp::HudWeaponCount::SubmarineTorpedo ? 0x00815850u : 0x007c1db0u);
        return 0;
    }
    bool unit_machinegun_byte(std::uint32_t unit) override {
        // unit+C24h PilotFires (007CD930's store).
        return unit != 0 && owner_.units != nullptr && unit <= owner_.units->count()
            && owner_.units->plane_pilot_fires_0c24(unit - 1);
    }
    bool weapon_source_differs(const bsp::HudWeaponSource& source) override {
        // 00449AF0 over root+70h and the new source: emptiness first, then a
        // case-insensitive compare.
        const std::string text = source_text(source);
        const std::string& cached = owner_.root_weapon_source_70;
        if (text.empty() != cached.empty()) return true;
        if (text.size() != cached.size()) return true;
        for (std::size_t i = 0; i < text.size(); ++i) {
            if (lower(text[i]) != lower(cached[i])) return true;
        }
        return false;
    }
    void set_weapon_source(const bsp::HudWeaponSource& source, bool) override {
        // 00ABAED0(WeaponInfo_Text, source, 1). The frontend re-lays out a
        // text on every set, so an unchanged source is not re-submitted (the
        // widget already holds it).
        GuiLayoutWidget* w = owner_.hud_root_row_widget(bsp::HudRowWidget::WeaponInfo);
        if (w == nullptr) return;
        std::string text = source_text(source);
        if (owner_.root_weapon_text_applied && owner_.root_weapon_text == text) return;
        owner_.root_weapon_text = text;
        owner_.root_weapon_text_applied = true;
        owner_.menu.frontend().set_widget_text_source(*w, std::move(text));
    }
    void cache_weapon_source(const bsp::HudWeaponSource& source) override {
        owner_.root_weapon_source_70 = source_text(source);   // root+70h
    }
    void widget_scalar(bsp::HudRowWidget widget, float value) override {
        // Virtual +4Ch (00AA6980): the alpha lane of the Color property.
        GuiLayoutWidget* w = owner_.hud_root_row_widget(widget);
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_color(*w, w->color[0], w->color[1], w->color[2], value);
    }
    void stop_weapon_animation(int slot) override {
        if (auto* widget = owner_.hud_root_row_widget(bsp::HudRowWidget::WeaponInfo))
            owner_.menu.frontend().stop_widget_alpha_animation(*widget, slot);
    }
    std::uint32_t weapon_animation(int slot) override {
        auto* widget = owner_.hud_root_row_widget(bsp::HudRowWidget::WeaponInfo);
        if (!widget) return 0u;
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(
            owner_.menu.frontend().widget_alpha_animation(*widget, slot)));
    }
    void animation_interval(std::uint32_t animation, float target, float rate) override {
        owner_.menu.frontend().set_widget_alpha_interval(
            reinterpret_cast<bsp::GuiTimedEntryStorage*>(static_cast<std::uintptr_t>(animation)), target, rate);
    }
    void animation_field_4(std::uint32_t animation, float value) override {
        owner_.menu.frontend().set_widget_alpha_delay(
            reinterpret_cast<bsp::GuiTimedEntryStorage*>(static_cast<std::uintptr_t>(animation)), value);
    }
    bool special_command_state(std::uint32_t unit, int role) override {
        // 00927F30(unit, role): the holder at unit+1ACh is the local player's
        // slot 0, as WeaponGroupScreenBinding::local_player_role answers it.
        std::int32_t holder = -1;
        return unit != 0 && owner_.units != nullptr && unit <= owner_.units->count()
            && owner_.units->unit_current_role_slot(unit - 1, role, holder) && holder == 0;
    }
    std::uint32_t command_descriptor(std::uint32_t unit) override {
        // Virtual +114h (the weapon director) then 0071BE40 on it; 0 without one.
        if (unit == 0 || owner_.units == nullptr || unit > owner_.units->count()) return 0u;
        return owner_.units->director_current_command_0071be40(unit - 1);
    }
    void widget_state(bsp::HudRowWidget widget, std::uint32_t, int, float) override {
        // Virtual +88h, the icon state select (00AB1710); the bridge draws a
        // widget's first authored state only.
        owner_.record(widget == bsp::HudRowWidget::Command
            ? "HudRootRows::command_icon_state" : "HudRootRows::payload_icon_state", 0x00ab1710u);
    }
    const char* unit_name(std::uint32_t) override {
        // Virtual +14h, 0081C0A0: +160h when +15Ch is set, else [+538h]+54h.
        // SUBSTITUTION, labelled: neither string reaches this host, so the
        // name is not sourced and unit_name_source below writes nothing.
        owner_.record("HudRootRows::unit_display_name", 0x0081c0a0u);
        return "";
    }
    void unit_name_source(const char*, float, bool) override {
        owner_.record("HudRootRows::unit_name_source", 0x00abb000u);
    }
    int controlled_unit_country(std::uint32_t unit) override {
        return unit != 0 && owner_.units != nullptr && unit <= owner_.units->count()
            ? owner_.units->unit_side_0054(unit - 1) : 0;   // unit+54h
    }
    void widget_shown(bsp::HudRowWidget widget, bool shown) override {
        // Virtual +34h. The frontend rebuilds its quads on every call, so a
        // widget already applied with the same value is left alone.
        GuiLayoutWidget* w = owner_.hud_root_row_widget(widget);
        if (w == nullptr) return;
        const auto it = owner_.hud_root_row_shown.find(widget);
        if (it != owner_.hud_root_row_shown.end() && it->second == shown && w->visible == shown) return;
        owner_.hud_root_row_shown[widget] = shown;
        owner_.menu.frontend().set_widget_visible(*w, shown);
    }
    // The payload frame and the health widths only reach widget_state /
    // widget_width, which are records here.
    std::uint32_t unit_class(std::uint32_t unit) override { return unit; }
    std::uint32_t class_payload_icon(std::uint32_t) override {
        owner_.record("HudRootRows::payload_icon_frame", 0x00653200u);
        return 0u;
    }
    long double kind_45_health(std::uint32_t unit) override {
        // 006D2560, the airfield's health fraction. LABELLED: answered by the
        // 00923BE0 projection below; it only feeds the recorded width.
        owner_.record("HudRootRows::airfield_health", 0x006d2560u);
        return unit_health(unit);
    }
    long double unit_health(std::uint32_t unit) override {
        // 00923BE0: the +5Dh arm, else the unit's +110h provider and clamp.
        const GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
        if (unit == 0 || gunnery == nullptr) return 0.0L;
        const std::vector<GameGunneryUnitRow>& rows = gunnery->unit_rows();
        if (unit - 1 >= rows.size()) return 0.0L;
        const GameGunneryUnitRow& row = rows[unit - 1];
        const bool torn_down = gunnery->unit_dead(unit - 1);
        const bool fraction_available = row.max_health > 0.0f;
        bsp::UnitHealth health;
        health.current_health = row.health;
        health.max_health = row.max_health;
        // bsp.py show 00876260 --asm: FLD +370h / FDIV +36Ch / FSTP / FLD.
        // Adapt the live gunnery pair to the existing numeric body, preserving
        // its ambient x87 precision and float spill. 00923BE4 skips the provider
        // on torn-down units; the unavailable-model fallback remains the host's.
        const float fraction = !torn_down && fraction_available
            ? bsp::health_fraction_00876260(health) : 0.0f;
        return bsp::ai_unit_health_00923be0(torn_down, fraction, fraction_available);
    }
    void widget_width(bsp::HudRowWidget widget, float, float) override {
        // Virtual +48h, the scale pair; no bridge operation.
        owner_.record(widget == bsp::HudRowWidget::HpDamage
            ? "HudRootRows::hp_damage_width" : "HudRootRows::hp_healthy_width", 0x006496a6u);
    }
    float health_metric(std::uint32_t, std::uint32_t address) override {
        // 00939F70 / F80 / FC0 / FB0 read the repair task at unit+A20h
        // (+38h, +34h, +30h, +2Ch). SUBSTITUTION, labelled: the gunnery host
        // keeps that task privately; 0 takes the plain-health arm, and the
        // value only feeds the recorded width.
        owner_.record("HudRootRows::repair_task_metric", address);
        return 0.0f;
    }
    int unit_field_a44(std::uint32_t) override {
        owner_.record("HudRootRows::repair_priority_a44", 0x00649615u);
        return 0;
    }
    float tuning_float(std::uint32_t) override {
        owner_.record("HudRootRows::tuning_float", 0x00424c40u);
        return 1.0f;
    }
    long double powerup_multiplier(int, std::uint32_t) override {
        // 00470440: FLD1 unless the flag byte 00E0C978 is set with a modifier
        // owner; the power-up manager [00F88C30] is not built in this host.
        return 1.0L;
    }
    float unit_float_36c(std::uint32_t unit) override {
        const GameGunneryHost* gunnery = owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
        if (unit == 0 || gunnery == nullptr) return 0.0f;
        const std::vector<GameGunneryUnitRow>& rows = gunnery->unit_rows();
        return unit - 1 < rows.size() ? rows[unit - 1].max_health : 0.0f;   // unit+36Ch
    }

private:
    static char lower(char c) {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
    }
    // The 00648CBD..006491C5 recipe: a counted source is "." + decimal count
    // + "x |" + key (004263B0 / 004260B0 / 004261A0); otherwise the key.
    static std::string source_text(const bsp::HudWeaponSource& source) {
        std::string key = source.key != nullptr ? source.key : "";
        if (!source.counted) return key;
        return "." + std::to_string(source.count) + "x |" + key;
    }
    GameHudHost::Impl& owner_;
    HudRootListsBinding lists_;
};

// 00647080's typed UnitRefs are unit index+1, as in the root's other adapters.
// The first member is the registry's compacted live +3D0h array, not an
// invented copy of the controlled squadron. Unresolved containers are refused
// before the native body's required-member branch is entered.
class HudClosedRowsBinding final : public bsp::HudClosedRowsHost {
public:
    HudClosedRowsBinding(GameHudHost::Impl& owner, bsp::HudRootPayloadKind pushed_kind)
        : owner_(owner), pushed_kind_(pushed_kind) {}
    std::uint32_t controlled_unit() override {
        return owner_.units != nullptr && owner_.units->controlled_bound()
            ? static_cast<std::uint32_t>(owner_.units->controlled_index() + 1) : 0u;
    }
    std::uint32_t first_member(std::uint32_t unit) override {
        if (unit == 0) return 0;
        const std::size_t first = owner_.first_squadron_member(unit - 1);
        return owner_.units != nullptr && first < owner_.units->count()
            ? static_cast<std::uint32_t>(first + 1) : 0u;
    }
    bool unit_is_kind(std::uint32_t unit, int kind) override {
        return unit != 0 && owner_.units != nullptr && unit <= owner_.units->count()
            && owner_.units->unit_is_kind_of(unit - 1, kind);
    }
    bool unit_flag_379(std::uint32_t unit) override {
        return unit != 0 && owner_.lua != nullptr
            && owner_.lua->squadron_force_gunner_mode_379(unit - 1);
    }
    int unit_owner_1a8(std::uint32_t unit) override {
        std::int32_t role = 0;
        // +188h + 8*4 = +1A8h is the gunner role's permission, NOT party.
        if (unit == 0 || owner_.units == nullptr
            || !owner_.units->unit_role_permission(unit - 1, 8, role))
            throw std::logic_error("HUD closed rows lack the controlled unit's role 8");
        return role;
    }
    int local_team() override {
        // Despite this interface's historical name the native read is the
        // local PLAYER SLOT at game+18ECh, not the scene party. Single-player
        // mission entry owns slot 0 (entry_state.local_slot_index, 004DA7E0).
        return 0;
    }
    void push_interface_request(int id, std::uint32_t payload) override {
        const std::size_t unit = payload != 0 ? static_cast<std::size_t>(payload - 1) : SIZE_MAX;
        owner_.pending_interface_id = id;
        owner_.note_pending_record_004cc460(id, false);
        owner_.pending_explicit_payload = true;
        owner_.pending_payload = {unit, pushed_kind_};
        owner_.record_pending_payload_38 = unit;
        owner_.done("HudRootScreen::push_closed_rows_interface", 0x004cc460u);
    }
private:
    GameHudHost::Impl& owner_;
    bsp::HudRootPayloadKind pushed_kind_;
};

// bsp::HudRootUpdateHost over this process's HUD, for 00649860. Inputs the
// owner cannot source remain records; chain 413 binds the closed-HUD path.
class HudRootUpdateBinding final : public bsp::HudRootUpdateHost {
public:
    explicit HudRootUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    bool game_hud_suppressed() override {
        // SUBSTITUTION: game+61Fh (004D95F0, the pause menu's "set paused") and
        // game+620h (004D94F0, the tutorial hints' pause) are not modelled.
        // The host never pauses, so both read clear.
        // cc15-hint2-gate: 0064987D CMP byte [EAX+620h],0 / JNZ 00649FB3 skips
        // the whole update while a critical hint is shown.
        owner_.record("HudRootScreen::game_pause_bytes", 0x00649863u);
        if constexpr (kHintPauseHudGateCc15Bound) {
            return game_hint_pause_620_cc15();
        } else {
            return false;
        }
    }
    bool platform_row_inset() override {
        // SUBSTITUTION: platform 0109CF04+0Dh (the widescreen byte) does not
        // reach the HUD host (docs/HUD_PRESENTATION_TOP.md row 2). It only
        // places the first power-up row, and no row is placed here.
        owner_.record("HudRootScreen::platform_row_inset", 0x006499d9u);
        return false;
    }
    float mission_clock() override {
        return powerup_entries_.clock;
    }
    bool controlled_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    // The clone vectors +104h and +114h remain empty: widget cloning below is
    // still a record. Chain 360 does evaluate the real item/unit applicability.
    void release_icon_clones() override {}
    void release_circle_clones() override {}
    void collect_powerups() override {
        game_powerups::collect_hud_powerups(powerup_entries_);
        owner_.done("HudRootScreen::collect_powerups", 0x008e9af0u);
    }
    std::size_t untimed_entry_count() override {
        return powerup_entries_.untimed.size();
    }
    std::size_t timed_entry_count() override {
        return powerup_entries_.timed.size();
    }
    float timed_entry_deadline(std::size_t index) override {
        return powerup_entries_.deadlines.at(index);
    }
    bool untimed_entry_applies(std::size_t index) override {
        if (!controlled_unit_present()) return false;
        return game_powerups::hud_powerup_applies_to_unit(*powerup_entries_.untimed.at(index),
            *owner_.units, owner_.units->controlled_index());
    }
    bool timed_entry_applies(std::size_t index) override {
        if (!controlled_unit_present()) return false;
        return game_powerups::hud_powerup_applies_to_unit(*powerup_entries_.timed.at(index),
            *owner_.units, owner_.units->controlled_index());
    }
    std::uint32_t clone_icon_template() override {
        owner_.record("HudRootScreen::clone_template", 0x00aab4c0u);
        return 0;
    }
    std::uint32_t clone_circle_template() override {
        owner_.record("HudRootScreen::clone_template", 0x00aab4c0u);
        return 0;
    }
    float icon_template_height() override { return 0.0f; }
    void widget_set_resolved_position(std::uint32_t, const bsp::HudGuiPoint&) override {}
    int local_team_index() override {
        if (!controlled_unit_present()) return 0;
        const GameUnitRow* row = owner_.units->unit_row(owner_.units->controlled_index());
        return row != nullptr && row->party >= 0 && row->party <= 7 ? row->party : 0;
    }
    void icon_add_state_from_entry(std::uint32_t, std::size_t, bool) override {}
    void widget_select_state(std::uint32_t) override {}
    void circle_set_fill(std::uint32_t, float) override {}
    float timed_entry_duration(std::size_t) override { return 1.0f; }

    void advance_ticker() override {
        owner_.record("HudRootScreen::advance_ticker", 0x00648060u);
    }
    // +78h's only writer is 0076CFD0, the receiver of session message 4Dh,
    // which Lua MW_MultiSelectUnit (008AB3D0) sends. The host keeps that
    // native unimplemented and USN04 never calls it, so the handle stays 0 and
    // none of the following is reached.
    bool pending_unit_selectable(std::uint16_t) override {
        owner_.record("HudRootScreen::pending_unit_handle", 0x0064a018u);
        return false;
    }
    bool pending_unit_allowed() override { return false; }
    void commit_pending_unit() override {
        // 00645600 sets the controlled unit: gameplay state, not performed.
        owner_.record("HudRootScreen::commit_pending_unit", 0x00645600u);
    }
    bool interface_manager_idle() override { return false; }
    void notify_camera_and_input() override {}
    int controlled_unit_scene_payload() override { return 0; }
    void push_interface_request(int, int) override {
        owner_.record("HudRootScreen::push_interface_request", 0x004cc460u);
    }

    void widget_set_shown(std::uint32_t widget, bool shown) override {
        if (widget != 0x40u) {
            owner_.record("HudRootScreen::clone_set_shown", 0x00649aafu);
            return;
        }
        // 0064A108: +40h is ClosedUnitHUD_Group (006463E0 stores it at
        // 00646AAE, found under Units_Group on GUI_selector), shown through
        // virtual +34h(1).
        GuiLayoutWidget* w = owner_.hud_root_closed_group_widget();
        if (w == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*w, shown);
    }
    std::uint32_t resolve_weapon_info(bool& needs_fallback) override {
        // 00644CC0 calls hud_root_find_unit; OFF keeps the neutral selection tuple.
        HudRootSelectionBinding binding(owner_);
        const auto& lists = owner_.root_lists;
        const bsp::HudUnitListView primary{lists.primary_8c.data(),
            static_cast<std::uint32_t>(lists.primary_8c.size())};
        const bsp::HudUnitListView secondary{lists.secondary_9c.data(),
            static_cast<std::uint32_t>(lists.secondary_9c.size())};
        const auto controlled = owner_.units != nullptr ? binding.controlled_unit() : 0u;
        const auto selection = bsp::hud_root_find_unit(primary, secondary, controlled, binding);
        needs_fallback = selection.index == 0xffffu;
        return (static_cast<std::uint32_t>(selection.index) << 16)
            | (selection.secondary ? 1u : 0u);
    }
    std::uint32_t resolve_weapon_info_fallback() override {
        // 00644C20 calls hud_root_find_primary_unit; OFF keeps the neutral fallback.
        HudRootSelectionBinding binding(owner_);
        const auto& lists = owner_.root_lists;
        const bsp::HudUnitListView primary{lists.primary_8c.data(),
            static_cast<std::uint32_t>(lists.primary_8c.size())};
        const auto controlled = owner_.units != nullptr ? binding.controlled_unit() : 0u;
        const auto selection = bsp::hud_root_find_primary_unit(primary, controlled, binding);
        return (static_cast<std::uint32_t>(selection.index) << 16)
            | (selection.secondary ? 1u : 0u);
    }
    void update_unit_rows(float) override {
        // 00644DB0 calls hud_root_poll_selection; OFF keeps the selection-poll record.
        if (owner_.units == nullptr) return;
        HudRootSelectionBinding binding(owner_);
        auto& state = owner_.root_selection;
        auto& lists = owner_.root_lists;
        const auto incoming = owner_.hud_root.weapon_info_field;
        state.selection = {(incoming & 0xffu) != 0,
            static_cast<std::uint16_t>(incoming >> 16)};
        // The caller just wrote root+C2h. selected_unit() must see that
        // same tuple if the poll converts a secondary selection to primary.
        lists.cursor = state.selection;
        state.pending = lists.force_pending_c0;
        const bsp::HudUnitListView primary{lists.primary_8c.data(),
            static_cast<std::uint32_t>(lists.primary_8c.size())};
        const bsp::HudUnitListView secondary{lists.secondary_9c.data(),
            static_cast<std::uint32_t>(lists.secondary_9c.size())};
        bsp::hud_root_poll_selection(state, primary, secondary, binding);
        lists.cursor = state.selection;
        lists.force_pending_c0 = state.pending;
        owner_.hud_root.weapon_info_field = (incoming & 0x0000ff00u)
            | (static_cast<std::uint32_t>(state.selection.index) << 16)
            | (state.selection.secondary ? 1u : 0u);
        owner_.done("HudRootScreen::selection_poll", 0x00644db0u);
    }
    void update_medals() override {
        // 00648C20, the unit rows (name, flag, payload, health, command).
        if (owner_.units == nullptr) {
            owner_.record("HudRootScreen::unit_rows", 0x00648c20u);
            return;
        }
        HudRootRowsBinding binding(owner_);
        bsp::hud_root_update_rows(owner_.root_rows, binding);
        ++owner_.root_row_updates;
        owner_.done("HudRootScreen::unit_rows", 0x00648c20u);
    }

    bool controlled_unit_is_kind(int kind) override {
        return owner_.selection_is_kind_of(owner_.units->controlled_index(), kind);
    }
    bool game_blocks_toggle() override {
        // SUBSTITUTION: game+19C4h is not modelled (as for screen 46h); read clear.
        owner_.record("HudRootScreen::game_19c4", 0x0064a179u);
        return false;
    }
    bool input_action_pressed(int action) override {
        return owner_.menu.input_action_pressed(action);
    }
    bool controlled_unit_flag_379() override {
        return owner_.lua != nullptr
            && owner_.lua->squadron_force_gunner_mode_379(owner_.units->controlled_index());
    }
    void toggle_closed_hud() override {
        owner_.refresh_root_saved_payload();
        const auto root = owner_.root_interface_view();
        const std::size_t saved = owner_.root_identity_index(root.ptr(0xE4));
        bsp::HudClosedRowsState state{
            root.at<std::int32_t>(0x1C), root.at<std::int32_t>(0xCC),
            saved != SIZE_MAX ? static_cast<std::uint32_t>(saved + 1) : 0u};
        // Closing pushes the current first PLANE; reopening preserves the
        // saved payload's original meaning rather than reclassifying its
        // index as a squadron after that plane's death.
        HudClosedRowsBinding binding(owner_, state.closed == 1
            ? owner_.root_saved_payload_kind : bsp::HudRootPayloadKind::Plane);
        if (state.closed == 0 && (state.interface_id == 0x22 || state.interface_id == 0x23)
            && binding.first_member(binding.controlled_unit()) == 0) {
            owner_.record("HudRootScreen::unresolved_closed_rows_member", 0x00647080u);
            return;
        }
        // bsp.py show 00647080: exactly-zero closes, exactly-one restores
        // the saved interface/payload; all other state values are inert.
        bsp::hud_root_toggle_closed_00647080(state, binding);
        owner_.set_root_closed_value(state.closed);
        owner_.done("HudRootScreen::toggle_closed_hud", 0x00647080u);
    }
    // +20h is the screen's own byte; nothing in this host sets it, and the
    // tail then stops at game+1FE4h, which is zero in single player.
    bool game_has_group_manager() override { return false; }
    int controlled_unit_team() override { return local_team_index(); }
    void rebuild_group_rows() override {
        owner_.record("HudRootScreen::rebuild_group_rows", 0x006485a0u);
    }

private:
    game_powerups::HudPowerupEntries powerup_entries_;
    GameHudHost::Impl& owner_;
};
}  // namespace

GuiLayoutWidget* GameHudHost::Impl::hud_root_closed_group_widget() {
    if (!hud_root_widgets_bound) {
        hud_root_widgets_bound = true;
        GuiLayoutPage* page = menu.in_game_page(0x44, "GUI_selector");
        hud_root_closed_group = page != nullptr && page->root
            ? bsp::find_descendant_by_name(*page->root, "ClosedUnitHUD_Group") : nullptr;
    }
    return hud_root_closed_group;
}

// The row widgets 006463E0 stores: +34h..+84h found under GUI_unit, +3Ch under
// GUI_selector (the 006463E0 layout in src/ingame_gui_cc13.cpp).
GuiLayoutWidget* GameHudHost::Impl::hud_root_row_widget(bsp::HudRowWidget widget) {
    if (!hud_root_row_widgets_bound) {
        hud_root_row_widgets_bound = true;
        GuiLayoutPage* unit_page = menu.in_game_page(0x44, "GUI_unit");
        GuiLayoutPage* selector_page = menu.in_game_page(0x44, "GUI_selector");
        const auto find = [&](GuiLayoutPage* page, bsp::HudRowWidget key, const char* name) {
            hud_root_row_widgets[key] = page != nullptr && page->root
                ? bsp::find_descendant_by_name(*page->root, name) : nullptr;
        };
        find(unit_page, bsp::HudRowWidget::UnitName, "unit_name_Text");
        find(unit_page, bsp::HudRowWidget::FlagJP, "FlagJP_Icon");
        find(unit_page, bsp::HudRowWidget::FlagUS, "FlagUS_Icon");
        find(unit_page, bsp::HudRowWidget::Payload, "unit_payload_Icon");
        find(unit_page, bsp::HudRowWidget::HpDamage, "unit_HP_damage_Icon");
        find(unit_page, bsp::HudRowWidget::HpHealthy, "unit_HP_healthy_Icon");
        find(unit_page, bsp::HudRowWidget::Command, "unit_command_Icon");
        find(selector_page, bsp::HudRowWidget::WeaponInfo, "WeaponInfo_Text");
    }
    const auto it = hud_root_row_widgets.find(widget);
    return it != hud_root_row_widgets.end() ? it->second : nullptr;
}

void GameHudHost::advance_gui_alpha_animations(float seconds) {
    impl_->menu.frontend().advance_widget_alpha_animations(seconds);
}

void GameHudHost::update_hud_root_screen_00649860(float seconds) {
    Impl& impl = *impl_;
    // 00649FD7..00649FF5: with the ticker timer expired the award queue at
    // +B0h..+B4h is read. SUBSTITUTION: 00648AB0, which
    // BSP_MissionScoring_GrantAward (0090EDE0) calls to queue an award
    // message, is not reached from this host's award grants, so the queue
    // reads empty and the timer is never reloaded.
    if (impl.hud_root.ticker_timer <= 0.0f) {
        impl.record("HudRootScreen::award_ticker_queue", 0x00649fe0u);
    }
    impl.hud_root.ticker_queue_count = 0;
    impl.hud_root.selector_widget = 0x40u;  // ClosedUnitHUD_Group's field
    HudRootUpdateBinding binding(impl);
    bsp::hud_root_screen_update(impl.hud_root, binding, seconds);
    impl.done("HudRootScreen::update", 0x00649860u);
}

namespace {
// bsp::UnitPickHost over this process's HUD, for screen 29h (00527260 and
// 00526A40). Units are index + 1. The lock branches, which send orders, move
// roles or take control, are records that perform nothing; each is reached
// only after an input flag.
// Packet cc9_pick_squadron_members (docs/PICK_SQUADRON_MEMBERS.md). ON answers
// 00526A40's squadron arm from bsp::plane_squadron_registry():
//  * 00526F33 IsKindOf(18h) on a LIST ENTRY is true when the entry is a
//    registry squadron_unit. SUBSTITUTION: this host fuses the 414h container
//    with its wing-0 leader (docs/PLANE_SQUADRON_HOST.md section 1), so the
//    leader stands for the container here; every other kind test is the unit's.
//  * +3CCh (00526FBE) is the record's live_count(), +3D0h[k] (00526E58,
//    00526FCF) its k-th live member, index + 1. The image's array holds only
//    planes that exist; the host keeps a no-unit slot for a wing that never
//    became a unit, so the k-th LIVE member is the image's slot k.
// OFF keeps the entry as the one plane it is and records squadron_members.
// ON by the verdict: the USN04 and E2 pairs are identical (docs section 4). The
// arm is unexercised there, because no squadron is in either list.
constexpr bool kPickSquadronMembersBound = true;

class UnitPickBinding final : public bsp::UnitPickHost {
public:
    explicit UnitPickBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    float lock_zoom_modifier_5c() override {
        // Reached only below full zoom; the binoculars zoom stays 1.0 here.
        owner_.record("UnitPickScreen::lock_zoom_modifier", 0x00526a8eu);
        return 0.8f;  // 0087DDF7's default, 3F4CCCCDh
    }
    float interpolate_clamped_00419010(float, float) override {
        owner_.record("UnitPickScreen::interpolate_clamped", 0x00419010u);
        return 1.0f;
    }
    bool game_19c4() override {
        // SUBSTITUTION: game+19C4h is not modelled (as for screens 44h and 46h).
        // The first query of an update is 00526A40's (00526B05); the later
        // ones are 00527260's own (005273C5 onwards).
        const bool pick = game_19c4_queries_++ == 0;
        owner_.record(pick ? "UnitPickScreen::game_19c4_pick" : "UnitPickScreen::game_19c4",
                      pick ? 0x00526b05u : 0x005273c5u);
        return false;
    }
    std::size_t spectated_unit_005a1310() override {
        owner_.record("UnitPickScreen::spectated_unit", 0x005a1310u);
        return 0;
    }
    std::size_t firing_unit_004b4b00() override {
        // 004B4B00: the controlled unit when it is kind 5, its [+3D0h] when it
        // is kind 18h, else none.
        const std::size_t c = controlled_unit();
        if (c == 0) return 0;
        if (is_kind_of(c, 5)) return c;
        if (is_kind_of(c, 0x18)) {
            owner_.record("UnitPickScreen::squadron_first_member", 0x004b4b35u);
            return 0;
        }
        return 0;
    }
    bool is_kind_of(std::size_t unit, int class_id) override {
        if (unit == 0) return false;
        if (kPickSquadronMembersBound && class_id == 0x18 && squadron_of(unit) != nullptr) {
            owner_.done("UnitPickScreen::squadron_entry", 0x00526f33u);
            return true;
        }
        return owner_.units->unit_is_kind_of(unit - 1, class_id);
    }
    // The registry record whose squadron_unit is `unit` (index + 1), or null.
    const bsp::PlaneSquadronHostRecord* squadron_of(std::size_t unit) const {
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit != bsp::kPlaneSquadronNoUnit && r.squadron_unit == unit - 1) {
                return &r;
            }
        }
        return nullptr;
    }
    int unit_slot_1b4(std::size_t) override {
        owner_.record("UnitPickScreen::unit_slot_1b4", 0x00526b36u);
        return -1;
    }
    bool slot_auto_engage_00927f10(int) override {
        owner_.record("UnitPickScreen::slot_auto_engage", 0x00927f10u);
        return false;
    }
    void camera_basis(float position[3], float forward[3]) override {
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!(kMissionCameraBound && node.ready)) {
            owner_.record("UnitPickScreen::camera_basis", 0x00526b71u);
            for (int i = 0; i < 3; ++i) position[i] = forward[i] = 0.0f;
            return;
        }
        // 00526B77/00526BBA: 00B6DB70 when the world-valid bit 2 is clear,
        // then +110h..+118h (row 2) and +120h..+128h (row 3) of the world.
        if ((node.state.transform.valid_flags & 2u) == 0) {
            bsp::refresh_camera_world_00b6db70(node.state.transform);
        }
        const bsp::CameraMatrix& world = node.state.transform.world;
        for (int i = 0; i < 3; ++i) {
            forward[i] = world[8 + i];
            position[i] = world[12 + i];
        }
        const int source = owner_.movie_active ? 2 : 1;
        if (source != owner_.pick_basis_source) {
            owner_.pick_basis_source = source;
            owner_.pick_first_basis_logged = true;
            owner_.log.notef("unit pick: camera basis now from the %s at hud "
                "update frame %llu: from=(%.1f,%.1f,%.1f) forward=(%.4f,%.4f,%.4f)",
                source == 2 ? "movie camera" : "published mover",
                owner_.summary.update_frames, static_cast<double>(position[0]),
                static_cast<double>(position[1]), static_cast<double>(position[2]),
                static_cast<double>(forward[0]), static_cast<double>(forward[1]),
                static_cast<double>(forward[2]));
        }
    }
    bool ray_pick_009043a0(const float* from, const float* to, std::size_t ignore,
                           std::size_t& hit) override {
        hit = 0;
        if constexpr (kHudPickSegmentQueryBound) {
            // Packet cc9_hud_ray_pick (docs/HUD_PICK_SEGMENT_QUERY.md sections
            // 1-2): 009043A0 with kind filter 0 (00526BCD) and the firing unit
            // as `ignore`, whose vtable +B0h is its own collision root here.
            // 0098ADD0 runs over the gunnery host's units (SegmentBinding):
            // SUBSTITUTION for the world grid [game+19CCh]+84h, which is not
            // built in this process.
            const GameGunneryHost* gunnery =
                owner_.units != nullptr ? owner_.units->gunnery() : nullptr;
            if (gunnery == nullptr) {
                owner_.record("UnitPickScreen::segment_query", 0x009043a0u);
                return false;
            }
            float point[3];
            const bool any = gunnery->query_segment_units(from, to, ignore, hit, point);
            owner_.done("UnitPickScreen::segment_query", 0x009043a0u);
            if (any) owner_.done("UnitPickScreen::segment_query_hit", 0x00526c62u);
            return any;
        } else {
            // SUBSTITUTION: the spatial index ([00E188A8]+19CCh, 0098ADD0) is not
            // built, so the segment query reports no hit.
            static_cast<void>(from);
            static_cast<void>(to);
            static_cast<void>(ignore);
            owner_.record("UnitPickScreen::segment_query", 0x009043a0u);
            return false;
        }
    }
    std::size_t ray_hit_branch(bsp::UnitPickScreenState& screen, std::size_t hit) override {
        if constexpr (kHudPickSegmentQueryBound) {
            // 00526C74..00526DB2.
            if (is_kind_of(hit, 6)) {
                // 00526C80..00526D15: the section label at +54h and the two
                // points from record+38h, which no host narrowphase writes;
                // +54h keeps 00CE43EC "" (section_named_54 false).
                owner_.record("UnitPickScreen::hit_section", 0x00526c8cu);
            }
            // 00526D1A..00526D78: these kinds are the pick themselves.
            for (const int kind : {6, 0x0f, 0x45, 0x46, 0x1b, 0x35}) {
                if (!is_kind_of(hit, kind)) continue;
                const bool own = owner_.units != nullptr && controlled_unit() != 0
                    && owner_.units->unit_side_0054(hit - 1)
                        == owner_.units->unit_side_0054(controlled_unit() - 1);
                owner_.done(own ? "UnitPickScreen::ray_pick_own" : "UnitPickScreen::ray_pick_other",
                    0x00526d9cu);
                return hit;
            }
            // 00526D80: kind 1Eh, +50h = the hit and the pick is 00923810(hit, 1).
            if (is_kind_of(hit, 0x1e)) {
                screen.exclude_hit_50 = hit;
                owner_.record("UnitPickScreen::part_owner_00923810", 0x00923810u);
                return 0;
            }
            owner_.done("UnitPickScreen::ray_hit_other_kind", 0x00526dafu);
            return 0;
        } else {
            static_cast<void>(screen);
            static_cast<void>(hit);
            owner_.record("UnitPickScreen::ray_hit_branch", 0x00526c74u);
            return 0;
        }
    }
    bool game_1fe4() override { return false; }  // single player: no session
    int game_difficulty_6ac() override { return game_effective_difficulty_6ac(); }
    float lock_radius_multiplier(int index) override {
        const std::vector<float>& m = owner_.lock_radius_multipliers();
        if (index < 0 || static_cast<std::size_t>(index) >= m.size()) {
            // No config vector: the image would fault in 00BF6713. Radius 0
            // picks nothing.
            owner_.record("UnitPickScreen::lock_radius_multiplier", 0x00526dd7u);
            return 0.0f;
        }
        return m[static_cast<std::size_t>(index)];
    }
    std::size_t list_size(bsp::UnitPickList list) override {
        if constexpr (kReconUnitListSourcesBound) {
            // Packet cc9_recon_call_sites: 00526E01 reads the head at
            // game+1974h, the list game+1970h (walk 0's everything-not-
            // ordnance), and 00526FF1 the head at game+19BCh, the list
            // game+19B8h (walk 1's 35h and objective-set units, then the
            // merge of ships, squadrons, airfields, shipyards and forts).
            // 004C3CB0 built both once after the load; the tests after the
            // list are 00526A40's own (bsp::unit_pick_00526a40).
            team_.clear();
            const bsp::LocalPlayerUnitLists* lists = game_local_player_unit_lists();
            const bool team = list == bsp::UnitPickList::TeamUnits;
            const char* name = team ? "UnitPickScreen::list_1970" : "UnitPickScreen::list_19b8";
            const std::uint32_t site = team ? 0x00526e01u : 0x00526ff1u;
            if (lists == nullptr) {
                owner_.record(name, site);
                return 0;
            }
            const auto& source = (*lists)[static_cast<std::size_t>(team
                ? bsp::LocalPlayerUnitList::kWalk0Rest : bsp::LocalPlayerUnitList::kMerged)];
            for (const bsp::UnitRef ref : source) team_.push_back(static_cast<std::size_t>(ref));
            owner_.done(name, site);
            report_list_census(team, team_);
            return team_.size();
        }
        if (list == bsp::UnitPickList::Kind35) {
            // SUBSTITUTION: game+19BCh, 004C3CB0's second-walk list of kind-35h
            // and grey-arrow units (team record +DE8h), is not built: empty.
            owner_.record("UnitPickScreen::kind35_list", 0x004c3e99u);
            return 0;
        }
        // SUBSTITUTION: game+1974h is 004C3CB0's first walk over the local team
        // record's +DDCh list, which this process does not fill. The stand-in
        // walks the created units with that walk's own filter: the local
        // party, +5Ch set with +5Dh/+60h/+5Eh clear, and not kind 2Ah.
        owner_.record("UnitPickScreen::team_unit_list", 0x004c3cb0u);
        team_.clear();
        const std::size_t c = controlled_unit();
        if (c == 0) return 0;
        const GameUnitRow* self = owner_.units->unit_row(c - 1);
        if (self == nullptr) return 0;
        const std::size_t count = owner_.units->units().size();
        for (std::size_t i = 0; i < count; ++i) {
            const GameUnitRow* row = owner_.units->unit_row(i);
            if (row == nullptr || row->party != self->party) continue;
            if (!owner_.units->unit_alive_and_visible(i)) continue;
            if (owner_.units->unit_is_kind_of(i, 0x2a)) continue;
            team_.push_back(i + 1);
        }
        return team_.size();
    }
    std::size_t list_unit(bsp::UnitPickList, std::size_t i) override {
        return i < team_.size() ? team_[i] : 0;
    }
    int member_count_3cc(std::size_t squadron) override {
        if constexpr (kPickSquadronMembersBound) {
            // 00526FBE CMP EBP,[EDI+3CCh].
            const bsp::PlaneSquadronHostRecord* r = squadron_of(squadron);
            return r != nullptr ? r->live_count() : 0;
        } else {
            static_cast<void>(squadron);
            return 0;
        }
    }
    std::size_t member_3d0(std::size_t squadron, int k) override {
        if constexpr (kPickSquadronMembersBound) {
            // 00526E58 [EDI+3D0h] for k = 0, 00526FCF [ECX] for the next slots.
            const bsp::PlaneSquadronHostRecord* r = squadron_of(squadron);
            if (r != nullptr && k >= 0) {
                int live = 0;
                for (const std::size_t u : r->member_units) {
                    if (u == bsp::kPlaneSquadronNoUnit) continue;
                    if (live++ == k) {
                        owner_.done("UnitPickScreen::squadron_members", 0x00526e58u);
                        return u + 1;
                    }
                }
            }
            owner_.done("UnitPickScreen::squadron_member_empty", 0x00526e5eu);
            return 0;
        } else {
            // SUBSTITUTION: squadron members (+3D0h, count +3CCh) are not exposed.
            static_cast<void>(squadron);
            static_cast<void>(k);
            owner_.record("UnitPickScreen::squadron_members", 0x00526e58u);
            return 0;
        }
    }
    bool grey_arrow_contains_008ddf90(std::size_t unit) override {
        if constexpr (kHudGreyArrowSetBound) {
            // 008DDF90 itself (bsp::objective_set_contains_008ddf90) on the local
            // slot's set (game+18ECh is 0 here): the size test 008DDF93, the kind
            // arms 008DDFA9 / 008DDFB8 and the 008DDF00 lookup feed the body.
            const std::vector<std::size_t> set =
                bsp::game::game_objective_sets().units_in_slot(0);
            bsp::ObjectiveSetQuery query;
            query.set_size = set.size();                                     // [this+20h]
            if (unit != 0 && owner_.units != nullptr) {
                const std::size_t subject = unit - 1;
                query.unit_is_vehicle = owner_.units->unit_is_kind_of(subject, 5);   // 008DDFA9
                query.unit_is_squadron = !query.unit_is_vehicle
                    && owner_.units->unit_is_kind_of(subject, 0x18);                 // 008DDFB8
                if (query.unit_is_vehicle) {                                         // 008DDF00
                    query.subject_in_set =
                        std::find(set.begin(), set.end(), subject) != set.end();
                } else if (query.unit_is_squadron) {
                    // unit+3D0h, a squadron's plane: not exposed here.
                    owner_.record("UnitPickScreen::grey_arrow_squadron", 0x008ddfc2u);
                }
            }
            owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
            return bsp::objective_set_contains_008ddf90(query);
        } else if constexpr (kHudGreyArrowSetBound) {
            // 008DDF90 on the local slot's set (game+18ECh is 0 here). The
            // set's unit tree (+18h, size +20h) is what the objective table
            // holds per slot; the Lua host fills it (Objectives_AddUnit).
            const std::vector<std::size_t> set =
                bsp::game::game_objective_sets().units_in_slot(0);
            if (set.empty() || unit == 0 || owner_.units == nullptr) {  // 008DDF93
                owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
                return false;
            }
            std::size_t subject = unit - 1;
            if (!owner_.units->unit_is_kind_of(subject, 5)) {           // 008DDFA9
                if (owner_.units->unit_is_kind_of(subject, 0x18)) {     // 008DDFB8
                    // unit+3D0h, a squadron's plane: not exposed here.
                    owner_.record("UnitPickScreen::grey_arrow_squadron", 0x008ddfc2u);
                }
                owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
                return false;
            }
            owner_.done("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
            for (const std::size_t member : set) {                      // 008DDF00
                if (member == subject) return true;
            }
            return false;
        }
        // SUBSTITUTION: no entity set is built at game+21A4h
        // (docs/AI_SQUADRON_SERVED.md), so nothing is a grey-arrow member.
        owner_.record("UnitPickScreen::grey_arrow_set", 0x008ddf90u);
        return false;
    }
    bool flag_5d(std::size_t unit) override {
        return owner_.units->unit_flag_005d(unit - 1);
    }
    void unit_position(std::size_t unit, float out[3]) override {
        const GameUnitRow* row = owner_.units->unit_row(unit - 1);
        for (int i = 0; i < 3; ++i) out[i] = row != nullptr ? row->position[i] : 0.0f;
    }
    void intercept_point_00901c20(std::size_t, std::size_t, float out[3]) override {
        owner_.record("UnitPickScreen::gunbot_intercept", 0x00901c20u);
        for (int i = 0; i < 3; ++i) out[i] = 0.0f;
    }
    unsigned project_0043a660(const float point[3], float& x, float& y) override {
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (!(kMissionCameraBound && node.ready)) {
            owner_.record("UnitPickScreen::project", 0x0043a660u);
            return 0;
        }
        // 0043A697..0043A6AF: 00B70490 then 00B62D10, then the mode-1 map.
        const bsp::CameraMatrix& vp = bsp::get_camera_view_projection_00b70490(node.state);
        const float source[4] = {point[0], point[1], point[2], 1.0f};
        float clip[4];
        bsp::transform_native_vector4_00b62d10(source, clip, vp.data());
        // Mode 1 multiplies y by the GUI extent height (00AA1FE0()+4h).
        // OFF: SUBSTITUTION of 1.0, the value its 4/3 law gives
        // (docs/HUD_PRESENTATION_TOP.md row 2), as a record.
        float extent_height = 1.0f;
        const Win32PlatformState* platform =
            kHudGuiExtentBound ? active_platform_state() : nullptr;
        if (platform != nullptr) {
            extent_height = bsp::gui_aspect_extent_00aa1fe0(platform->widescreen,
                                                            platform->active_aspect).height;
            owner_.done("UnitPickScreen::gui_extent", 0x00aa1fe0u);
        } else {
            owner_.record("UnitPickScreen::gui_extent", 0x00aa1fe0u);
        }
        const bsp::HudMarkerScreenProjection p = bsp::camera_project_world_to_screen_0043a660(
            clip[0], clip[1], clip[2], clip[3], bsp::HudMarkerProjectMode::ViewportAspect, true,
            extent_height, 1.0f);
        x = p.screen_x;
        y = p.screen_y;
        return p.clip_mask;
    }
    bool ray_hit_filter_005220c0(std::size_t) override {
        owner_.record("UnitPickScreen::ray_hit_filter", 0x005220c0u);
        return true;
    }
    bool alive_and_visible(std::size_t unit) override {
        return owner_.units->unit_alive_and_visible(unit - 1);
    }

    float binoculars_zoom() override { return owner_.binoculars.zoom_38; }  // 26h+38h
    std::size_t controlled_unit() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        return owner_.units->controlled_index() + 1;
    }
    std::size_t owner_140(std::size_t) override {
        // Unit vtable +140h (the scene payload 0064A0EB also asks for), unread.
        owner_.record("UnitPickScreen::owner_140", 0x0052731cu);
        return 0;
    }
    bool team_record_19() override {
        // [game+18CCh+slot*4]+19h, the local player record's byte: unread.
        owner_.record("UnitPickScreen::team_record_19", 0x0052733cu);
        return false;
    }
    bool byte_e0e350() override {
        owner_.record("UnitPickScreen::byte_e0e350", 0x005273cfu);
        return false;
    }
    int interface_id() override { return owner_.summary.applied_interface_id; }
    bool action_byte(int, int) override {
        // The action records' device state ([record+2Ch]+0Bh/+10h): no device
        // drives an in-mission action here. Recorded once per update.
        return false;
    }
    bool input_pressed(int action) override { return owner_.menu.input_action_pressed(action); }
    void reset_c0_00525170() override {
        owner_.record("UnitPickScreen::reset_c0", 0x00525170u);
    }
    bool lock_branch(bsp::UnitPickScreenState&, std::uint32_t address) override {
        owner_.record("UnitPickScreen::lock_branch", address);
        return true;
    }
    void tail_sound_00a7e490() override {
        owner_.record("UnitPickScreen::tail_sound", 0x00a7e490u);
    }

private:
    GameHudHost::Impl& owner_;
    // Packet cc9_pick_squadron_members (docs/PICK_SQUADRON_MEMBERS.md section 1):
    // the class census of each pick list. 00526F33 expands an entry that answers
    // IsKindOf(18h) into its +3D0h members. The line prints when a list's shape
    // changes, so a mission logs a few lines, not one per update.
    void report_list_census(bool team, const std::vector<std::size_t>& entries) {
        struct Shape {
            std::size_t size{static_cast<std::size_t>(-1)};
            std::size_t kind_18{0};
            std::size_t kind_5{0};
            std::size_t squadron_units{0};
            std::size_t members{0};
        };
        static Shape last[2];
        Shape now;
        now.size = entries.size();
        std::map<int, std::size_t> classes;
        const bsp::PlaneSquadronRegistry& registry = bsp::plane_squadron_registry();
        for (const std::size_t unit : entries) {
            if (unit == 0) {
                ++classes[-1];
                continue;
            }
            ++classes[owner_.units->unit_class_id(unit - 1)];
            if (owner_.units->unit_is_kind_of(unit - 1, 0x18)) ++now.kind_18;
            if (owner_.units->unit_is_kind_of(unit - 1, 5)) ++now.kind_5;
            if (registry.find_by_member_unit(unit - 1) != nullptr) ++now.members;
            for (const bsp::PlaneSquadronHostRecord& r : registry.records()) {
                if (r.squadron_unit == unit - 1) {
                    ++now.squadron_units;
                    break;
                }
            }
        }
        Shape& prev = last[team ? 0 : 1];
        if (now.size == prev.size && now.kind_18 == prev.kind_18 && now.kind_5 == prev.kind_5
            && now.squadron_units == prev.squadron_units && now.members == prev.members) {
            return;
        }
        prev = now;
        std::string histogram;
        for (const auto& [class_id, count] : classes) {
            char item[32];
            std::snprintf(item, sizeof item, " %Xh:%zu", static_cast<unsigned>(class_id), count);
            histogram += item;
        }
        owner_.log.notef("pick list census: list=%s entries=%zu kind18=%zu kind5=%zu "
            "squadron_units=%zu squadron_members=%zu registry_squadrons=%zu classes:%s",
            team ? "1970" : "19b8", now.size, now.kind_18, now.kind_5, now.squadron_units,
            now.members, registry.size(), histogram.c_str());
    }

    std::vector<std::size_t> team_;
    int game_19c4_queries_{0};
};
}  // namespace

const std::vector<float>& GameHudHost::Impl::lock_radius_multipliers() {
    if (!lock_radius_read) {
        lock_radius_read = true;
        if (lua == nullptr || !lua->read_lock_radius_multipliers_0087dc85(lock_radius)) {
            lock_radius.clear();
        }
    }
    return lock_radius;
}

void GameHudHost::update_unit_pick_screen_00527260(float seconds) {
    Impl& impl = *impl_;
    // 00527419..005275D5 read the action records' device bytes directly
    // (CEh, CFh, D0h's neighbours D1h/D3h/D4h/D5h); one record per update.
    impl.record("UnitPickScreen::input_record_bytes", 0x00527419u);
    UnitPickBinding binding(impl);
    bsp::unit_pick_screen_update_00527260(impl.unit_pick, binding, seconds);
    impl.done("UnitPickScreen::update", 0x00527260u);
}

namespace {
// The block kHudLeafScreenVirtuals' slot was constructed on, null when 0068CC70
// built none. The vtable its constructor stored must be the one listed.
HudLeafScreenStorage* hud_leaf_block(GameHudHost::Impl& impl, const HudLeafScreenVirtuals& v) {
    const std::size_t index = bsp::in_game_hud_screen_index_for_slot(v.slot);
    if (index >= kInGameHudScreenCount || !impl.leaf_screens[index]) return nullptr;
    HudLeafScreenStorage& block = *impl.leaf_screens[index];
    std::uint32_t vtable = 0;
    std::memcpy(&vtable, block.data(), sizeof(vtable));
    if (vtable != v.vtable)
        throw std::logic_error("HUD leaf screen virtuals listed for a different vtable");
    return &block;
}
// Goal 5 chain 671. Screen 23h's +18h 00517820 / +1Ch 00517860 (bsp.py ghidra
// disasm 00517820) on its 2Ch block: the state is the block's +1Ch unit and +28h
// word, read at the call (neither body stores into the screen). 00517820 runs
// 004DC940(1.0, 1) on [00E188A8] (the HUD's view fov service), then, only with a
// unit at +1Ch, the observer pair on screen+8; 00517860 sends 0077C470(+1Ch, 1FEh,
// 0) only with a unit. +1Ch is stored by the constructor 005178B0 (0) and by
// 005178F0 BSP_InGameScreen23_BindUnit, which nothing in this process calls, so
// the unit arms are unreached and throw; so do the bomber screen 25h's services.
class HudLeafObservedUnitBinding final : public bsp::ObservedUnitScreenHost {
public:
    explicit HudLeafObservedUnitBinding(GameHudHost::Impl& owner) : owner_(owner) {}
    bool observer_registered_00694af0(std::uintptr_t) override { unreached("00694AF0"); }
    void observer_register_00694a60(std::uintptr_t) override { unreached("00694A60"); }
    void unit_send_0077c470(std::uintptr_t, std::int32_t, std::int32_t) override {
        unreached("0077C470");
    }
    std::uintptr_t unit_child_9d4(std::uintptr_t) override { unreached("unit+9D4h"); }
    void game_set_view_fov_004dc940(float fov, std::int32_t arg) override {
        owner_.set_view_fov(fov, arg);
    }
    void weapon_group_bind_unit_00549260(std::uintptr_t, std::uintptr_t) override {
        unreached("00549260");
    }
    void weapon_group_release_005470c0() override { unreached("005470C0"); }
    void screen26_call_0051e8e0() override { owner_.reset_binoculars_0051e8e0(); }

    static bsp::ObservedUnitScreenState state_of(const HudLeafScreenStorage& block) {
        if (!block.contains(block.data() + 0x28, sizeof(std::uint32_t)))
            throw std::logic_error("screen 23h block shorter than its +28h word");
        std::uint32_t unit = 0;
        std::uint32_t word = 0;
        std::memcpy(&unit, block.data() + 0x1C, sizeof(unit));
        std::memcpy(&word, block.data() + 0x28, sizeof(word));
        bsp::ObservedUnitScreenState state;
        state.unit_1c = unit;
        state.field_28 = word;
        return state;
    }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("screen 23h virtual reached ") + site
            + " with a +1Ch unit no host call stores");
    }
    GameHudHost::Impl& owner_;
};

}  // namespace

bool GameHudHost::leaf_screen_self_managed(int slot, bool& self_managed) {
    const HudLeafScreenVirtuals* v = hud_leaf_screen_virtuals(slot);
    if (v == nullptr || v->self_managed_04 == 0) return false;
    HudLeafScreenStorage* block = hud_leaf_block(*impl_, *v);
    if (block == nullptr) return false;
    switch (v->self_managed_04) {
    case 0x00643DD0u:
        self_managed = bsp::ingame_gui_cc13::hud_spectator_screen_is_self_managed_00643dd0(
            {block->data()});
        return true;
    case 0x0052B070u:
        self_managed = bsp::frontend_seg15_22_cc13::hud_counters_screen_is_self_managed_0052b070(
            {block->data()});
        return true;
    default: break;
    }
    throw std::logic_error("HUD leaf +4h virtual listed but not bound");
}

bool GameHudHost::enter_leaf_screen(int slot) {
    Impl& impl = *impl_;
    const HudLeafScreenVirtuals* v = hud_leaf_screen_virtuals(slot);
    if (v == nullptr || v->enter_18 == 0) return false;
    HudLeafScreenStorage* block = hud_leaf_block(impl, *v);
    if (block == nullptr) return false;
    const bsp::ingame_gui_cc13::Raw raw{block->data()};
    switch (v->enter_18) {
    case 0x00643F40u: {
        HudLeafChildVisibilityBinding binding(impl.menu, impl.log);
        bsp::ingame_gui_cc13::spectator_enter_00643f40(raw, binding);
        break;
    }
    case 0x0067B400u: bsp::ingame_gui_cc13::hud_common_overlay_screen_enter_0067b400(raw); break;
    case 0x00650190u: bsp::ingame_gui_cc13::hud_submarine_screen_enter_00650190(raw); break;
    case 0x0067BEB0u: bsp::ingame_gui_cc13::hud_follow_screen49_enter_0067beb0(raw); break;
    case 0x00528C40u: {
        std::optional<HudCountersContext> c = hud_counters_context(impl);
        if (!c) return false;
        HudCountersUpdateBinding binding(*c);
        bsp::hud_counters_enter_00528c40(binding);
        break;
    }
    case 0x00517820u: {
        bsp::ObservedUnitScreenState state = HudLeafObservedUnitBinding::state_of(*block);
        HudLeafObservedUnitBinding binding(impl);
        bsp::in_game_screen23_enter_00517820(state, binding);
        break;
    }
    default: throw std::logic_error("HUD leaf +18h virtual listed but not bound");
    }
    impl.done("HudLeafScreen::enter", v->enter_18);
    return true;
}

bool GameHudHost::exit_leaf_screen(int slot) {
    Impl& impl = *impl_;
    const HudLeafScreenVirtuals* v = hud_leaf_screen_virtuals(slot);
    if (v == nullptr || v->exit_1c == 0) return false;
    HudLeafScreenStorage* block = hud_leaf_block(impl, *v);
    if (block == nullptr) return false;
    const bsp::ingame_gui_cc13::Raw raw{block->data()};
    switch (v->exit_1c) {
    case 0x0051E9B0u: {
        ShipViewInputBinding binding(impl);
        bsp::cc13_seg14::binoculars_screen_exit_0051e9b0(&impl.binoculars, binding);
        break;
    }
    case 0x00643DE0u: bsp::ingame_gui_cc13::hud_spectator_screen_exit_00643de0(raw); break;
    case 0x0067B410u: bsp::ingame_gui_cc13::hud_common_overlay_screen_exit_0067b410(raw); break;
    case 0x004F75B0u: bsp::engine_seg11_35_66_cc13::base_screen_exit_004f75b0(); break;
    case 0x00528C50u: {
        std::optional<HudCountersContext> c = hud_counters_context(impl);
        if (!c) return false;
        HudCountersUpdateBinding binding(*c);
        bsp::hud_counters_exit_00528c50(binding);
        break;
    }
    case 0x00517860u: {
        bsp::ObservedUnitScreenState state = HudLeafObservedUnitBinding::state_of(*block);
        HudLeafObservedUnitBinding binding(impl);
        bsp::in_game_screen23_exit_00517860(state, binding);
        break;
    }
    case 0x005F9480u: bsp::cc13_fe1::hud_order_screen_exit_005f9480(block->data()); break;
    default: throw std::logic_error("HUD leaf +1Ch virtual listed but not bound");
    }
    impl.done("HudLeafScreen::exit", v->exit_1c);
    return true;
}

bool GameHudHost::update_leaf_screen(int slot, float seconds) {
    Impl& impl = *impl_;
    const HudLeafScreenVirtuals* v = hud_leaf_screen_virtuals(slot);
    if (v == nullptr || v->update_20 == 0) return false;
    HudLeafScreenStorage* block = hud_leaf_block(impl, *v);
    if (block == nullptr) return false;
    switch (v->update_20) {
    // RET 4: the pump's seconds argument is not read.
    case 0x00643DF0u:
        bsp::ingame_gui_cc13::hud_spectator_screen_update_00643df0({block->data()});
        break;
    // __thiscall(screen, float seconds), RET 4: the pump's seconds.
    case 0x0052BAA0u: {
        std::optional<HudCountersContext> c = hud_counters_context(impl);
        if (!c) return false;
        HudCountersUpdateBinding binding(*c);
        bsp::hud_counters_update_0052baa0(hud_counters_state(*block), seconds, binding);
        break;
    }
    // C2 04 00: RET 4, the pump's seconds unread.
    case 0x005178A0u:
        bsp::cc13_fe3::in_game_screen23_update_005178a0({block->data()});
        break;
    default: throw std::logic_error("HUD leaf +20h virtual listed but not bound");
    }
    impl.done("HudLeafScreen::update", v->update_20);
    return true;
}

void GameHudHost::update_markers_screen_006435d0(float seconds) {
    Impl& impl = *impl_;
    impl.step_mission_camera(seconds);
    if (!impl.markers) {
        impl.record("HudMarkers::update", 0x006435d0u);
        return;
    }
    impl.markers->update_006435d0(seconds);
}

void GameHudHost::apply_in_game_interface_004c9ca0(bool loading) {
    Impl& host = *impl_;
    // 004c9cc0 tests the one stack argument and branches on it, so this is two
    // routines sharing a frame. The load calls it with 1 at 004e1873 and the
    // mission-state entry with 0 at 004da746.
    if (loading) {
        // 004c9ccd..004c9d68, the arm that puts the loading element up. The
        // element is allocated at 004c9cd7 and constructed by 00636d90, neither
        // of which is reconstructed, so both are records; the two stores this
        // process can make are the slot at manager+D4h and the byte at +4h.
        if (!host.loading_element_present) {
            host.record("MissionLoad::allocate_loading_element", 0x00bf681bu);
            host.record("MissionLoad::construct_loading_element", 0x00636d90u);
            host.loading_element_present = true;
            // 004c9d11, the element's own vtable +10h right after the store.
            host.record("MissionLoad::loading_element_init", 0x004c9d11u);
        }
        host.loading_element_4 = true;   // 004c9d1f
        host.done("MissionLoad::apply_in_game_interface", 0x004c9ca0u);
        host.log.notef("in-game interface applied with 1: the loading element at "
            "[00e198c4]+D4h is up and its +4h byte is set; the single-player branch at "
            "004c9d23 ends the routine, because game+1FE4h is zero here");
        return;
    }

    // 004c9d6b..004c9ea2, the arm the mission-state entry takes.
    if (host.loading_element_present) {
        if (host.loading_element_5) {
            // 004c9d7e, the element's exit virtual +1Ch, only when +5h is set.
            host.record("MissionEntry::loading_element_exit", 0x004c9d7eu);
        }
        host.loading_element_4 = false;  // 004c9d82
        host.loading_element_5 = false;  // 004c9d85
        // 004c9d88, 004f83b0 on the element: the cleared bytes are committed
        // into the page tree. No page is attached to this element, so the walk
        // reaches nothing and the call is recorded rather than claimed.
        host.record("MissionEntry::commit_loading_element", 0x004f83b0u);
        // 004c9daa, the element's deleting destructor through vtable +0Ch with
        // 1, then the slot is nulled at 004c9dac.
        host.record("MissionEntry::destroy_loading_element", 0x004c9daau);
        host.loading_element_present = false;
    }
    // 004c9dae..004c9dcf: the name at 00ce765c is assigned into a pooled string
    // and handed to the atlas manager at 00f8c26c. The sprite bridge owns every
    // atlas in this process, so the release is a record.
    host.record("MissionEntry::release_front_end_atlas", 0x00aefa30u);
    // 004c9dfb: PUSH 1; PUSH 3; CALL 004c1ac0; MOV ECX,EAX; CALL 00518250. The
    // two pushes are 00518250's arguments (set 3, commit 1) and 004c1ac0 takes
    // none: it is the lazy getter for the 164h front-end frame object at
    // 00e18d80. This is the committing call, and it is what releases the front
    // end's own layouts.
    host.done("MissionEntry::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, true);
    // 004c9e1d: 007f8d60(game+650h, game+2198h) and the latch at game+1EE0h,
    // which is the engine-movie flag the frame reads. The comparison is over the
    // player profile's own record set, which this process does not own.
    host.record("MissionEntry::engine_movie_check", 0x007f8d60u);
    // 004c9e32: the single-player tail. game+1FE4h is zero in this process, so
    // the branch at 004c9e38 jumps over the input-context push 004bec00 /
    // 00a933f0, the controlled-unit registry walk 008053c0 / 008073c0, the unit
    // list rebuild 004c3cb0 and the interface refresh 006485a0. Recording them
    // would claim call sites this run does not reach, so it does not.
    host.in_game_interface_applied = true;
    host.done("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
    host.log.notef("in-game interface applied with 0: the loading element is torn down and "
        "00518250(3, commit=1) at 004c9e06 releases every other set's layouts, so FE_frame "
        "and FE_frame_title come off the screen and set 3's GUI_pause pair is acquired "
        "hidden");
}

void GameHudHost::select_front_end_layout_00518250() {
    // The load's own row at 004e04e4: PUSH EBX (0); PUSH 3; CALL 004c1ac0; MOV
    // ECX,EAX; CALL 00518250. Milestone 2i read the two pushes as 004c1ac0's
    // arguments; they are 00518250's (set 3, commit 0). 004c1ac0 is
    // BSP_FrontEndFrame_GetOrCreate, the lazy getter for the 164h front-end
    // frame object at 00e18d80, and it takes none: it only supplies ECX. Set 3
    // is the pause pair GUI_pause / GUI_pause_title
    // (bsp/title_init.hpp, FrontEndFrameSet::Pause).
    impl_->done("MissionLoad::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, false);
}

void GameHudHost::release_main_menu_manager_00686c90() {
    impl_->menu.destroy_main_menu_manager_00686c90();
    impl_->log.note("the main-menu manager was destroyed: 00687300 -> 00686c90 -> 00683aa0, "
        "whose tail publishes the empty level-4 screen set (004f8710) and the empty level-4 "
        "input contexts (004d8c00), so the front-end pages come down");
}

void GameHudHost::update_interface_only_004c40f0(float raw_delta) {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.menu.pump_interface_only_004c40f0(raw_delta);
    ++impl.summary.pump_frames;
}

void GameHudHost::report() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.log.notef("summary mission hud screens=%zu/%zu pages=%zu/%zu widgets=%zu/%zu "
        "interface=%02Xh level1_screens=%zu level1_contexts=%zu pump_frames=%llu "
        "update_frames=%llu audio_environment=%s", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.pages_loaded,
        impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.applied_interface_id),
        impl.summary.level1_screens, impl.summary.level1_contexts,
        impl.summary.pump_frames, impl.summary.update_frames,
        impl.summary.audio_environment.empty() ? "(none)"
                                               : impl.summary.audio_environment.c_str());
    impl.log.notef("summary mission hud listener environment (goal 4 chain 249, 00A7B710): "
        "selects=%llu changes=%llu", impl.summary.audio_environment_selects,
        impl.summary.audio_environment_changes);
    impl.log.notef("summary mission hud gun aim message 79h (cc15-smA, 00954A10 / 0075FFF0 / "
        "0075FF60 / 007600D0): bound=%d round_trips=%llu changed yaw=%llu pitch=%llu camera=%llu "
        "bools=%llu group=%llu target=%llu max_delta yaw=%.9g pitch=%.9g",
        bsp::kGunAimMessageCc15Bound ? 1 : 0, impl.gun_aim_round_trips, impl.gun_aim_changed[0],
        impl.gun_aim_changed[1], impl.gun_aim_changed[2], impl.gun_aim_changed[3],
        impl.gun_aim_changed[4], impl.gun_aim_changed[5], impl.gun_aim_max_yaw_delta,
        impl.gun_aim_max_pitch_delta);
    impl.log.notef("summary mission hud ship screen gauge product round (0043B370, sw1 reach): "
        "bound=%d ticks=%llu differs=%llu", bsp::ship_screen_gauge_reach().bound ? 1 : 0,
        bsp::ship_screen_gauge_reach().ticks, bsp::ship_screen_gauge_reach().differs);
    impl.log.notef("summary mission hud set selected unit (packet cc9_set_selected_unit, "
        "008AB260 / 00647300): bound=%d calls=%llu accepted=%llu pushes_20h=%llu",
        kSetSelectedUnitBound ? 1 : 0, impl.select_calls, impl.select_accepted,
        impl.select_pushes);
    {
        std::string only;
        for (const auto& entry : impl.vtable124_map_only) {
            const GameUnitRow* row = impl.units != nullptr ? impl.units->unit_row(entry.first) : nullptr;
            char tail[48];
            std::snprintf(tail, sizeof(tail), "/%02X@%08X",
                impl.units != nullptr ? impl.units->unit_class_id(entry.first) & 0xff : 0,
                static_cast<unsigned>(entry.second));
            if (!only.empty()) only += ' ';
            only += row != nullptr ? row->name : std::string("?");
            only += tail;
        }
        std::string slots;
        for (const std::size_t u : impl.squadron_slots_seen) {
            const GameUnitRow* row = impl.units != nullptr ? impl.units->unit_row(u) : nullptr;
            char tail[24];
            std::snprintf(tail, sizeof(tail), "/%02X->%02X",
                impl.units != nullptr ? impl.units->unit_class_id(u) & 0xff : 0,
                impl.selection_class_id(u) & 0xff);
            if (!slots.empty()) slots += ' ';
            slots += row != nullptr ? row->name : std::string("?");
            slots += tail;
        }
        impl.log.notef("summary mission hud squadron slot class (packet cc9_squadron_slot_class, "
            "00645060 / 00645600): bound=%d squadron slots asked=%zu: %s",
            kSquadronSlotClassBound ? 1 : 0, impl.squadron_slots_seen.size(),
            slots.empty() ? "(none)" : slots.c_str());
        impl.log.notef("summary mission hud unit vtable124 map (packet cc9_vtable124_liveness, "
            "00645110 / 0064565F): bound=%d asks=%llu map_only_asks=%llu map_only_units=%zu: %s",
            kUnitVtable124MapBound ? 1 : 0, impl.vtable124_asks, impl.vtable124_map_only_asks,
            impl.vtable124_map_only.size(), only.empty() ? "(none)" : only.c_str());
    }
    impl.log.notef("summary mission hud movie interface (packet cc9_movie_interface_and_reseed, "
        "005CD240 / 005CD1A0): bound=%d calls=%llu engages=%llu pushes_2ch=%llu reseed_bound=%d "
        "stream1_reseeds=%llu",
        kMovieInterfacePushBound ? 1 : 0, impl.movie_calls, impl.movie_engages,
        impl.movie_pushes, kMovieReseedBound ? 1 : 0, impl.movie_reseeds);
    impl.log.notef("summary mission hud movie camera (packet cc9_movie_camera_mover_bind, "
        "005CC170 / 0079A3B0): bound=%d builds=%llu destroys=%llu keyframes=%llu "
        "publishes=%llu unsupported=%llu active_at_end=%d", kMovieMoverBound ? 1 : 0,
        impl.movie_builds, impl.movie_destroys, impl.movie_keyframes, impl.movie_publishes,
        impl.movie_unsupported, impl.movie_active ? 1 : 0);
    impl.log.notef("summary mission hud movie keyframe modifiers (packet cc9_lua49_movie_modifier, "
        "00795CB7 / 00797ED0): bound=%d events_fired=%llu", bsp::kMovieKeyframeModifierBound ? 1 : 0,
        impl.movie_events_fired);
    impl.log.notef("summary mission hud movie camera keys (packet cc9_movie_camera_keys): "
        "step_draw_bound=%d step_draws=%llu parent_killed_byte_bound=%d "
        "terrain_avoid_bound=%d finishscript_bound=%d finishscript_due=%llu "
        "finishscript_ran=%llu", kMovieStepDrawBound ? 1 : 0, impl.movie_step_draws,
        kMovieParentKilledByteBound ? 1 : 0, kMovieTerrainAvoidBound ? 1 : 0,
        kMovieCameraFinishScriptCc13Bound ? 1 : 0, impl.movie_finish_due,
        impl.movie_finish_ran);
    impl.log.notef("summary mission hud force select unit (packet cc9_force_select_unit, "
        "008AAF30 / 006485A0 / 00648290): bound=%d calls=%llu selects=%llu rebuilds=%llu "
        "primary_8c=%zu secondary_9c=%zu", kForceSelectUnitBound ? 1 : 0, impl.force_calls,
        impl.force_selects, impl.root_rebuilds, impl.root_lists.primary_8c.size(),
        impl.root_lists.secondary_9c.size());
    impl.log.notef("summary mission hud root observer (packet cc9_controlled_unit_observer, "
        "00644A20): bound=%d notices=%llu releases=%llu", kControlledUnitObserverBound ? 1 : 0,
        impl.observer_notices, impl.observer_releases);
    impl.log.notef("summary mission hud squadron observer liveness bound=%d holds=%llu (007F3970 "
        "+3CCh -> 00926D90, packet cc9_squadron_observer_liveness)",
        kSquadronObserverLivenessBound ? 1 : 0, impl.observer_squadron_holds);
    // Milestone 2j. `minimap_islandmap_Icon` of GUI_minimap names the texture
    // `error.tga` with the material `minimap_terrain.mshd`, and milestone 2h
    // read that as the page's own authored texture. It is, and the material is
    // not an ordinary single-texture one: the installed
    // shaderfx/gui/minimap_terrain.shfx declares two samplers, `RadarMap` at
    // texture register 0 with clamped addressing and `FadeBorder` at index 1,
    // and names no texture file at all. Sampler 0 is the widget's own texture
    // slot, which is why `error.tga` reaches it and why the icon already takes
    // exactly the texture path every other HUD texture takes (the bridge's VFS
    // loader standing in for the renderer's vtable +64h, 00aa5e60). What the
    // running game puts there instead is the mission's own radar map, which the
    // minimap screen 005bec50 binds after taking the widget by name at 005bed21
    // through 00aa7e00, and which the renderer owner produces. There is no
    // second texture for this process to load.
    impl.record("HudMinimap::radar_map_texture", 0x00aa5e60u);
    if (impl.minimap) impl.minimap->report();
    if (impl.markers) impl.markers->report();
    impl.log.note("minimap island-map icon: minimap_terrain.mshd is the two-sampler GUI "
        "shader effect shaderfx/gui/minimap_terrain.shfx (RadarMap at register 0, "
        "FadeBorder at index 1) and names no texture, so error.tga is sampler 0's authored "
        "value and the icon is already on the same texture path as every other HUD texture; "
        "what replaces it is the renderer owner's radar map");
}

bool GameHudHost::manager_active() const noexcept {
    return impl_->summary.manager_built && impl_->manager.base.active;
}

void GameHudHost::Impl::run_root_observer_notice() {
    // The destruction notice. SUBSTITUTION (labelled): the image delivers it
    // in the on-killed dispatch (009274DE..00927510, 00696330 -> 00693550 ->
    // observer->vtable[4h]); here the units host's destroyed-unit record
    // (a sunk gunnery row) stands for it, read once per interface update.
    if (observed_unit == SIZE_MAX || units == nullptr) return;
    bool destroyed = false;
    for (const auto& death : units->destroyed_units()) {
        if (death.first == observed_unit) { destroyed = true; break; }
    }
    if constexpr (kSquadronObserverLivenessBound) {
        // Packet cc9_squadron_observer_liveness. For a squadron's fused slot
        // the observed entity is the squadron, which 007F3970 kills (00926D90)
        // only when +3CCh reaches 0. SUBSTITUTION (labelled): the host reads
        // the registry record's live count at the interface update; the image
        // kills the squadron inside the last plane's kill flush.
        for (const bsp::PlaneSquadronHostRecord& r : bsp::plane_squadron_registry().records()) {
            if (r.squadron_unit == bsp::kPlaneSquadronNoUnit || r.squadron_unit != observed_unit) {
                continue;
            }
            const bool squadron_gone = r.live_count() <= 0;
            if (destroyed && !squadron_gone && !squadron_hold_logged) {
                ++observer_squadron_holds;
                squadron_hold_logged = true;
                log.notef("hud root observer 00644A20: the fused slot died but squadron \"%s\" "
                    "keeps %d live plane(s) (+3CCh), so no notice (packet "
                    "cc9_squadron_observer_liveness)", r.name.c_str(), r.live_count());
            }
            // Only once the fused slot's own plane has died: a record whose
            // members never became units reads 0 while the squadron is alive.
            destroyed = destroyed && squadron_gone;
            break;
        }
    }
    if (!destroyed) return;
    squadron_hold_logged = false;
    ++observer_notices;
    const std::size_t unit = observed_unit;
    observed_unit = SIZE_MAX;   // the entity's observer list goes with it
    // 00644A20..00644A50: only when the entity is [00E188D8].
    if (!units->controlled_bound() || units->controlled_index() != unit) {
        done("HudRootObserver::destruction_notice_other", 0x00644a2du);
        return;
    }
    set_root_closed_value(0);
    units->clear_controlled_unit_004c0890();                  // 00644A38
    root_lists.cursor = bsp::HudUnitSelection{false, 0xffffu};   // 00644A3D/00644A44
    ++observer_releases;
    done("HudRootObserver::destruction_notice_00644a20", 0x00644a20u);
    const GameUnitRow* row = units->unit_row(unit);
    log.notef("hud root observer 00644A20: controlled unit \"%s\" destroyed; 004C0890(null) "
        "and cursor (0,-1) at hud update frame %llu", row != nullptr ? row->name.c_str() : "?",
        summary.update_frames);
}

void GameHudHost::update_in_game_interface_0068c1f0() {
    Impl& impl = *impl_;
    impl.refresh_root_saved_payload();
    if constexpr (kControlledUnitObserverBound) impl.run_root_observer_notice();
    if (!manager_active()) return;
    HudUpdateBinding binding(impl);
    bsp::update_in_game_interface_0068c1f0(impl.manager, impl.update_state, binding);
    impl.done("InGameInterface::update", 0x0068c1f0u);
    ++impl.summary.update_frames;
}

}  // namespace bsp::game
