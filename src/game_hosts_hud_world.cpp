// bsp_game.exe milestone 2k: the two in-mission HUD screens that show the world.
// See include/bsp/game_hosts_hud_world.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hint_passes.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/game_hosts_world.hpp"

#include "bsp/camera_projection.hpp"
#include "bsp/frontend_menus.hpp"
#include "bsp/gui_aspect_extent.hpp"
#include "bsp/platform_window.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/hud_markers_runtime.hpp"
#include "bsp/hud_minimap.hpp"
#include "bsp/hud_screens.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/mission_camera.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/cc13_front6.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_hudgui_cc13.hpp"
#include "bsp/hud_heading_vtable50_cc13.hpp"  // packet cc13-hudhdg
#include "bsp/pool_pool02_cc13.hpp"
#include "bsp/pool_pool09_cc13.hpp"
#include "bsp/pool_pool13_cc13.hpp"
#include "bsp/game_hosts_ai.hpp"           // game_objective_sets, goal 4 chain 376
#include "bsp/local_player_unit_lists.hpp"  // 008DDF90, goal 4 chain 376
#include "bsp/cc13_seg41_land.hpp"          // 0074DDF0, the land vehicle's +B8h
#include "bsp/band_0085_cc13.hpp"           // 008DA130 / 008DA140
#include "bsp/band_0060_cc13.hpp"           // 00638B90, live marker margins
#include "bsp/engine_seg56_cc13.hpp"        // 008DACD0 / 008DAD30
#include "bsp/game_hosts_gunnery.hpp"       // the recon pass records
#include "bsp/game_hosts_hud.hpp"           // 00645060
#include "bsp/ingame_gui_cc13.hpp"          // 00645180
#include "bsp/objective_units.hpp"          // the +20h entry records
#include "bsp/plane_squadron_host.hpp"      // vtable +140h of a plane
#include "bsp/recon_sensor_pass.hpp"
#include "bsp/cc13_seg49.hpp"
#include "bsp/frontend_seg20_cc13.hpp"
#include "bsp/cc13_sweepB3.hpp"
#include "bsp/objectives_seg57_argon.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <cstdio>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// 004B4B00, __cdecl, no arguments, returns the unit or null: [00E188D8]
// through vtable +5Ch(5), else +5Ch(18h) -> [unit+3D0h]. Units are index + 1;
// `squadron_case` flags the kind-18h arm, whose +3D0h is not exposed here.
std::size_t camera_unit_004b4b00(const GameUnitsHost* units, bool& squadron_case) {
    squadron_case = false;
    if (units == nullptr || !units->controlled_bound()) return 0;
    const std::size_t index = units->controlled_index();
    if (units->unit_is_kind_of(index, 5)) return index + 1;
    if (units->unit_is_kind_of(index, 0x18)) squadron_case = true;
    return 0;
}

}  // namespace

namespace {

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// Packet cc9_hud_minimap_bind (docs/UNIMPLEMENTED_AUDIT.md). ON answers the
// minimap walk's vtable +B8h (005C1675) with the body the unit's class installs
// and runs 005BE110's map find as done; OFF keeps the records under 0043F080 and
// 005BE110 with the old constant true.
constexpr bool kHudMinimapUnitGateBound = true;

// The registry slots of the two screens, from docs/IN_GAME_INTERFACE_SCREEN_SETS.md.
constexpr int kMinimapScreenSlot = 0x35;
constexpr int kMarkersScreenSlot = 0x4D;

// 00639990 BSP_InGameHudMarkers_ResolveMarkerColourIndex, the non-objective
// part of the rule docs/HUD_MARKERS_RUNTIME.md states: 1 when [unit+54h] is 0,
// 0 when it is 1, 5 otherwise. The objective arm (2, 3 or 4 by 008DD240(unit)
// [+18h]) needs an objective container this process does not build.
int marker_colour_index_00639990(int team_id) noexcept {
    if (team_id == 0) return 1;
    if (team_id == 1) return 0;
    return 5;
}

// Goal 5 chain 717: the list formerly represented only by the prune record.
// bsp.py show 008E2B90 / 008E28F0: the world constructor's list is initially
// empty, with a real 1Ch self-linked sentinel. Retain that native layout and
// the same allocation domain through the HUD lifetime. This owner supplies no
// synthetic ping: 005B45C0's input and the session receive producer remain
// unbound, so single-player currently retains the constructor's empty state.
class MinimapPingList final : public bsp::objectives_seg57_argon::MapPingListHost {
    struct Node {
        Node* next;
        Node* previous;
        std::uint32_t party;
        std::uint32_t field_0c;
        float x;
        float y;
        float expiry;
    };
    struct Storage {
        std::uint32_t profile;
        std::uint32_t allocator; // constructor-unwritten, never consumed here
        Node* head;
        std::uint32_t count;
    } storage_;
    static_assert(sizeof(Node) == 0x1c && offsetof(Node, expiry) == 0x18);
    static_assert(sizeof(Storage) == 0x10 && offsetof(Storage, head) == 8);
    static_assert(offsetof(Storage, count) == 0x0c);

public:
    MinimapPingList() {
        bsp::objectives_seg57_argon::map_ping_list_construct_008e2b90(*this, &storage_);
    }
    ~MinimapPingList() override {
        auto* node = storage_.head->next;
        while (node != storage_.head) {
            auto* const next = node->next;
            bsp::singleton_lifetime_free(node);
            node = next;
        }
        bsp::singleton_lifetime_free(storage_.head);
    }
    MinimapPingList(const MinimapPingList&) = delete;
    MinimapPingList& operator=(const MinimapPingList&) = delete;

    void prune(bsp::MissionResultHost& clock_source) {
        clock_source_ = &clock_source;
        // 005C22DE..005C22EA calls the original expiry body on the same
        // retained list, after all objective entries and before pool trimming.
        bsp::objectives_seg57_argon::map_ping_list_prune_expired_008e2bf0(*this, &storage_);
        clock_source_ = nullptr;
    }

private:
    float& global_f32(std::uint32_t address) override {
        if (address != 0x00f876a4u || clock_source_ == nullptr)
            throw std::logic_error("Map ping expiry requires the live mission clock");
        // Re-read the same mission clock used by Lua, scoring and objectives
        // at every native read boundary; there is no local elapsed-time clock.
        clock_read_ = clock_source_->mission_clock();
        return clock_read_;
    }
    void* image_address(std::uint32_t address) override {
        if (address != 0x00d16168u)
            throw std::logic_error("Unexpected map ping list profile");
        // An opaque profile word, never dispatched as a host vtable.
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(address));
    }
    void* stl_list_buy_head_node_008e28f0() override {
        auto* const head = static_cast<Node*>(bsp::singleton_lifetime_allocate(
            {bsp::SingletonAllocationKind::object, sizeof(Node), sizeof(Node)}));
        if (head == nullptr) throw std::bad_alloc();
        head->next = head;
        head->previous = head;
        return head;
    }
    void free_00bf65ac(void* node) override { bsp::singleton_lifetime_free(node); }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("Invalid map ping list iterator");
    }
    bsp::MissionResultHost* clock_source_{nullptr};
    float clock_read_; // assigned from the live source before any read
};

}  // namespace

// ===========================================================================
// Slot 35h, the minimap
// ===========================================================================

struct GameHudMinimapHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in) : log(log_in), menu(menu_in) {}

    GameHostLog& log;
    GameMenuHost& menu;
    GameUnitsHost* units{nullptr};
    GameMissionLuaHost* lua{nullptr};
    GameHudMinimapSummary summary;
    MinimapPingList map_pings;

    GuiLayoutPage* page{nullptr};
    // The six `item_ship_Icon` templates at screen +6Ch..+7Ch, indexed by the
    // colour index 00639990 returns, and the parent at +F0h.
    GuiLayoutWidget* colour_templates[6]{};
    GuiLayoutWidget* marker_group{nullptr};   // unit_marker_Group, +F0h
    GuiLayoutWidget* compass{nullptr};        // +4Ch
    GuiLayoutWidget* island_map{nullptr};     // +50h
    GuiLayoutWidget* direction{nullptr};      // +48h

    // Chain 47 (kWireGoal2Chain47Bound): the screen fields 005BD590 reads, bound
    // where the image binds them, in 005BE240 at page load (bind_page_005bec50),
    // and left empty otherwise. +6Ch..+78h are minimap_units_white_Group's
    // item_ship/plane/sub/building_Icon (005BE397, 005BE406, 005BE475, 005BE4E4);
    // +80h, +90h and +C0h are the 16-byte colours virtual +54h returns for the
    // red, blue and grey groups' item_ship_Icon (005BE6A2, 005BE7BA, 005BE8D2).
    // The image keeps them for the screen's life; so does this Impl.
    GuiLayoutWidget* entry_templates[4]{};    // ship, plane, sub, building
    float entry_colours[3][4]{};              // red +80h, blue +90h, grey +C0h
    bool entry_colour_bound[3]{};
    bool logged_entry_body{false};

    // The objective markers of 005C1E00..005C22DE. 005BE240 also binds the white
    // group's item_marker_Icon into +7Ch (005BE553) and the colours the loop
    // hands virtual +50h: white +D0h (005BE58A, +6Ch's virtual +54h), yellow
    // +A0h (005BE9EA) and silver +B0h (005BEB02), each the group's item_ship_Icon.
    GuiLayoutWidget* objective_marker_template{nullptr};   // +7Ch
    float objective_colours[3][4]{};                       // yellow, silver, white
    bool objective_colour_bound[3]{};
    // 005C0930's five pools, screen +104h + k*10h with the used cursors
    // +154h + k*4h: k = 0 an entry without a unit (+7Ch), 1 IsKindOf(8) (+74h
    // sub), 2 IsKindOf(6) (+6Ch ship), 3 IsKindOf(0Fh) (+70h plane), 4 any other
    // unit (+78h building), the order 005C0930 and 005BCAC0 test.
    struct ObjectiveMarker {
        std::uint32_t vtable{};
        GuiLayoutWidget* widget{};
    };
    struct ObjectiveMarkerVector {
        std::uint32_t allocator{};
        void** first{};
        void** last{};
        void** end{};
    };
    struct ObjectiveMarkerScreen {
        alignas(4) std::uint8_t layout[0x104]{};
        ObjectiveMarkerVector vectors[5]{};
        std::int32_t cursors[5]{-1, -1, -1, -1, -1};
    } objective_marker_screen;
    static_assert(sizeof(ObjectiveMarker) == 8);
    static_assert(sizeof(ObjectiveMarkerVector) == 0x10);
    static_assert(offsetof(ObjectiveMarkerScreen, vectors) == 0x104);
    static_assert(offsetof(ObjectiveMarkerScreen, cursors) == 0x154);
    // Owned marker records; the native views above borrow these exact pools.
    std::vector<void*> objective_markers[5];
    bool logged_objective_marker{false};

    ~Impl() {
        // Widget subtrees remain page-owned at screen teardown, as do icons.
        // Surplus markers during a frame take 005C0BD0's deleting path below.
        for (auto& pool : objective_markers) {
            for (void* marker : pool) delete static_cast<ObjectiveMarker*>(marker);
        }
    }

    // The per-unit icon map at screen +F8h, as this process holds it: one
    // run-time clone of the colour template per unit index.
    std::map<std::size_t, GuiLayoutWidget*> icons;
    // Stable handles, so the reconstruction's `void*` unit is an index rather
    // than a pointer into the units host.
    struct UnitHandle { std::size_t index{0}; };
    std::vector<UnitHandle> handles;

    float icon_heading{0.0f};
    bool bound_attempted{false};
    bool range_attempted{false};
    bool logged_camera{false};
    bool logged_entry{false};

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

    std::size_t index_of(void* handle) const noexcept {
        auto* typed = static_cast<UnitHandle*>(handle);
        return typed != nullptr ? typed->index : 0;
    }
    void* handle_for(std::size_t index) noexcept {
        if (index >= handles.size()) return nullptr;
        return &handles[index];
    }

    void bind_page_005bec50();
    GuiLayoutWidget* create_icon(std::size_t index);
    // Chain 47: the 005BE240 binds above, and the icon 005BD590 builds.
    void bind_icon_entry_fields(GuiLayoutWidget& root);
    GuiLayoutWidget* create_icon_from_entry(void* unit);
    GuiLayoutWidget* acquire_objective_marker(void* unit);
    void hide_unused_objective_markers();
};

namespace {

// Goal 5 chain 672. bsp.py show 005C0930 / 005C0BD0 and the 005BCAC0
// listing: the five checked vectors/cursors are persistent screen state;
// [[00E198C4]+58h] supplies the same screen's bound templates and +F0h parent.
// This view publishes those live objects, never replacement templates or units.
class MinimapObjectiveMarkerBinding final
    : public bsp::frontend_seg20_cc13::MinimapMarkerHost,
      public bsp::frontend_seg20_cc13::MinimapMarkerDeletionHost,
      public bsp::MinimapMarkerRetirementHost {
public:
    explicit MinimapObjectiveMarkerBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {
        auto screen = bsp::frontend_seg20_cc13::raw(&owner_.objective_marker_screen);
        screen.ptr(0x6C) = owner_.entry_templates[0];
        screen.ptr(0x70) = owner_.entry_templates[1];
        screen.ptr(0x74) = owner_.entry_templates[2];
        screen.ptr(0x78) = owner_.entry_templates[3];
        screen.ptr(0x7C) = owner_.objective_marker_template;
        screen.ptr(0xF0) = owner_.marker_group;
        bsp::frontend_seg20_cc13::raw(interface_.data()).ptr(0x58) = screen.p;
        interface_pointer_ = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(interface_.data()));
        for (std::size_t k = 0; k < 5; ++k) publish(k);
    }
    std::uint32_t& global_u32(std::uint32_t address) override {
        if (address != 0x00E198C4) throw std::logic_error("minimap marker unrelated global");
        return interface_pointer_;
    }
    bool is_a_vslot5c(void* unit, std::int32_t kind) override {
        return owner_.units != nullptr && unit != nullptr
            && owner_.units->unit_is_kind_of(owner_.index_of(unit), kind);
    }
    void* operator_new_00bf681b(std::uint32_t size) override {
        if (size != sizeof(GameHudMinimapHost::Impl::ObjectiveMarker))
            throw std::logic_error("minimap marker allocation size");
        return new GameHudMinimapHost::Impl::ObjectiveMarker;
    }
    void* widget_clone_00aab4c0(void* widget, std::int32_t flags) override {
        if (widget == nullptr || owner_.page == nullptr || flags != 0)
            throw std::logic_error("minimap marker clone missing template/page");
        return owner_.menu.frontend().clone_page_widget_subtree(owner_.page->name,
            *static_cast<GuiLayoutWidget*>(widget));
    }
    void widget_add_child_00aaa5a0(void* parent, void* child, std::int32_t flags) override {
        if (parent == nullptr || child == nullptr || flags != 0)
            throw std::logic_error("minimap marker attach missing widget");
        owner_.menu.frontend().reparent_runtime_widget(*static_cast<GuiLayoutWidget*>(child),
            *static_cast<GuiLayoutWidget*>(parent));
    }
    void widget_vslot34(void* widget, std::int32_t value) override {
        owner_.menu.frontend().set_widget_visible(*static_cast<GuiLayoutWidget*>(widget), value != 0);
        ++owner_.summary.objective_markers_created;
        owner_.done("HudMinimap::construct_objective_marker", 0x005bcac0u);
    }
    void pointer_vector_push_back_005c0790(void* vector, void* const* value) override {
        const std::size_t k = pool_index(vector);
        owner_.objective_markers[k].push_back(*value);
        publish(k);
    }
    void** pointer_vector_at_005bcbd0(void* vector, std::uint32_t index) override {
        auto& pool = owner_.objective_markers[pool_index(vector)];
        if (index >= pool.size()) invalid_parameter_00bf6713();
        return &pool[index];
    }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("minimap marker vector bound");
    }
    void crt_invalid_parameter_noinfo_00bf6713() override { invalid_parameter_00bf6713(); }
    void object_vcall_00_delete(void* object, std::uint32_t flags) override {
        bsp::frontend_seg20_cc13::minimap_marker_scalar_delete_005bdb50(object, flags, *this);
    }
    void widget_delete_vslot04(void* widget, std::uint32_t flags) override {
        if (flags != 1) throw std::logic_error("minimap marker widget delete flags");
        owner_.menu.frontend().destroy_runtime_widget(*static_cast<GuiLayoutWidget*>(widget));
    }
    void free_marker_00bf65ac(void* marker) override {
        delete static_cast<GameHudMinimapHost::Impl::ObjectiveMarker*>(marker);
    }
    void dword_vector_resize_005c0390(void* vector, std::uint32_t size, std::uint32_t value) override {
        const std::size_t k = pool_index(vector);
        if (value != 0 || size > owner_.objective_markers[k].size())
            throw std::logic_error("minimap marker retirement must shrink");
        owner_.objective_markers[k].resize(size);
        publish(k);
    }
private:
    std::size_t pool_index(void* vector) const {
        for (std::size_t k = 0; k < 5; ++k) {
            if (vector == &owner_.objective_marker_screen.vectors[k]) return k;
        }
        throw std::logic_error("minimap marker unrelated vector");
    }
    void publish(std::size_t k) {
        auto& pool = owner_.objective_markers[k];
        auto& view = owner_.objective_marker_screen.vectors[k];
        view.first = pool.empty() ? nullptr : pool.data();
        view.last = pool.empty() ? nullptr : pool.data() + pool.size();
        view.end = view.last;
    }
    GameHudMinimapHost::Impl& owner_;
    alignas(4) std::array<std::uint8_t, 0x5C> interface_{};
    std::uint32_t interface_pointer_{};
};

// bsp::cc13_seg41::LandVehicleObjectiveSetHost for 0074DDF0 on the minimap's
// handle: game+18ECh is the local player slot (0 in single player, as for the
// markers' objective container below), and 008DDF00's find runs over that
// slot's +18h tree, GameObjectiveSets::units_in_slot (the members while
// kObjectiveSetHiddenMembershipBound), in the units host's index space.
class LandVehicleObjectiveSetBinding final : public bsp::cc13_seg41::LandVehicleObjectiveSetHost {
public:
    explicit LandVehicleObjectiveSetBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {}
    std::int32_t local_player_slot_18ec() override { return 0; }
    bool objective_set_find_008ddf00(std::int32_t slot, const void* vehicle) override {
        owner_.done("HudMinimap::objective_set_find", 0x008ddf00u);
        if (vehicle == nullptr || slot < 0
            || slot >= static_cast<std::int32_t>(GameObjectiveSets::kSlotCount)) {
            return false;
        }
        const std::size_t index = owner_.index_of(const_cast<void*>(vehicle));
        const std::vector<std::size_t> set = game_objective_sets().units_in_slot(slot);
        return std::find(set.begin(), set.end(), index) != set.end();
    }

private:
    GameHudMinimapHost::Impl& owner_;
};

// bsp::HudMinimapHost, the boundary of src/hud_minimap.cpp. One native call site
// per method, in the order 005C0F20 reaches them.
class MinimapBinding final : public bsp::HudMinimapHost {
public:
    explicit MinimapBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {}

    void prepare_frame() override {
        // 005C0F3F -> 005BD420, the screen's own transform refresh. It belongs
        // to the slot 35h class, which is one of the 42 this process records.
        owner_.record("HudMinimap::prepare_frame", 0x005bd420u);
    }

    void* camera_unit() override {
        // 005C154E -> 004B4B00. There is no in-mission camera at game+19FCh in
        // this process, so the executable hands the pass the controlled unit and
        // records the call. This is the executable's decision, not a value the
        // routine returns.
        if constexpr (kHudCameraUnitBound) {
            bool squadron = false;
            const std::size_t unit = camera_unit_004b4b00(owner_.units, squadron);
            if (squadron) {
                owner_.record("HudMinimap::camera_unit_squadron_leader", 0x004b4b2au);
                return nullptr;
            }
            owner_.done("HudMinimap::camera_unit", 0x004b4b00u);
            return unit != 0 ? owner_.handle_for(unit - 1) : nullptr;
        } else {
            owner_.record("HudMinimap::camera_unit", 0x004b4b00u);
            if (owner_.units == nullptr || !owner_.units->controlled_bound()) return nullptr;
            return owner_.handle_for(owner_.units->controlled_index());
        }
    }

    float minimap_range() override {
        // 005C157E -> 00432650, then +6Ch. The global config singleton is not
        // built here; the value is the one 0087D7B0 would have written.
        global_config_read();
        return owner_.summary.minimap_range;
    }
    float visibility_range() override {
        // 005C158A -> 00432650, then +70h.
        global_config_read();
        return owner_.summary.visibility_range;
    }
    // 00432650 returns the global config object; its +6Ch/+70h are the two
    // Globals["Minimap"] values 0087D7B0 writes. When those were read from the
    // installed globals.lua, the host's two cells stand in for the object's two
    // fields and the getter plus field read is done; otherwise it is a record.
    void global_config_read() {
        if (kHudPresentationTopBound && owner_.summary.range_from_data) {
            owner_.done("HudMinimap::global_config", 0x00432650u);
        } else {
            owner_.record("HudMinimap::global_config", 0x00432650u);
        }
    }

    void refresh_pose(void* unit) override {
        // 005C15BB -> 00414DB0 when [unit+C8h] is clear. Every created instance
        // publishes its world pose on the motion step, so +C8h is set and the
        // refresh is the no-op branch.
        static_cast<void>(unit);
        owner_.done("HudMinimap::refresh_pose", 0x00414db0u);
    }

    void* displayed_self_unit(void* controlled_unit) override {
        // 005C15F2 -> 00927880 on 00E188D8. The routine has no reconstruction,
        // so it is a record; the walk uses its result only to skip one cull.
        static_cast<void>(controlled_unit);
        owner_.record("HudMinimap::displayed_self_unit", 0x00927880u);
        return nullptr;
    }

    bsp::HudMinimapWorldPoint world_position(void* unit) override {
        // 005C1687 -> 00427EB0, which returns unit+FCh, the world-matrix
        // translation row (docs/HUD_MINIMAP.md correction 3).
        owner_.done("HudMinimap::world_position", 0x00427eb0u);
        bsp::HudMinimapWorldPoint point{};
        if (owner_.units == nullptr) return point;
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!owner_.units->unit_pose(owner_.index_of(unit), right, up, forward, translation)) {
            return point;
        }
        point.x = translation[0];
        point.y = translation[1];
        point.z = translation[2];
        return point;
    }

    void* team_unit_list_head(int team_index) override {
        if constexpr (kReconUnitListSourcesBound) {
            // Packet cc9_recon_call_sites: 005C1610 loads the head of the
            // local slot's union triple (+E08h, head +E0Ch), which 008073C0
            // rebuilds every refresh: own, enemy, neutral and unknown in
            // that order. The tests after the list are 005C1628..005C1675's
            // (bsp hud_minimap walk). A node is an entry of `union_`.
            static_cast<void>(team_index);
            union_.clear();
            if (owner_.units == nullptr
                || !game_local_recon_triple(*owner_.units, 4, union_)) {
                owner_.record("HudMinimap::union_triple", 0x005c1610u);
                return nullptr;
            }
            owner_.done("HudMinimap::union_triple", 0x005c1610u);
            return union_.empty() ? nullptr : static_cast<void*>(union_.data());
        }
        // [[[00E188A8 + 18ECh*4 + 18CCh] + 30h] + E0Ch]. Nothing in this process
        // fills the local player's unit registry, which is the same stand-in
        // milestone 2i records for walk 0 of 004C3CB0: the executable hands the
        // walk the created units so the recovered filters run over real units.
        static_cast<void>(team_index);
        owner_.record("HudMinimap::team_unit_list", 0x004c3cb0u);
        if (owner_.handles.empty()) return nullptr;
        return &owner_.handles[0];
    }
    void* team_unit_list_next(void* node) override {
        if constexpr (kReconUnitListSourcesBound) {
            std::size_t* entry = static_cast<std::size_t*>(node) + 1;
            return entry < union_.data() + union_.size() ? static_cast<void*>(entry) : nullptr;
        }
        const std::size_t next = owner_.index_of(node) + 1;
        return owner_.handle_for(next);
    }
    void* team_unit_list_unit(void* node) override {
        if constexpr (kReconUnitListSourcesBound) {
            return owner_.handle_for(*static_cast<std::size_t*>(node));
        }
        return node;
    }

    bool unit_is_alive_and_visible(void* unit) override {
        // 005C1628..005C164A, the same four bytes 0043F080 tests.
        owner_.done("HudMinimap::unit_alive_and_visible", 0x0043f080u);
        return owner_.units != nullptr
            && owner_.units->unit_alive_and_visible(owner_.index_of(unit));
    }

    bool is_kind_of(void* unit, int class_id) override {
        // 005C165D, the unit vtable +5Ch, over the recovered class chain.
        owner_.done("HudMinimap::is_kind_of", 0x006fe530u);
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.index_of(unit), class_id);
    }

    bool unit_shows_on_minimap(void* unit) override {
        // 005C1675, the unit vtable +B8h with no arguments.
        if (!kHudMinimapUnitGateBound) {
            // Before the law: a record under the gate's address, answering true.
            static_cast<void>(unit);
            owner_.record("HudMinimap::unit_shows_on_minimap", 0x0043f080u);
            return true;
        }
        // The law: the body the unit's most-derived class installs at +B8h
        // (bsp::hud_minimap_unit_shows_body, surveyed over all 88 kind bodies).
        const int class_id = owner_.units != nullptr
            ? owner_.units->unit_class_id(owner_.index_of(unit)) : -1;
        switch (bsp::hud_minimap_unit_shows_body(class_id)) {
        case bsp::HudMinimapUnitShows::Always:
            // 005C1675 dispatches the current unit's +B8h slot. Evidence:
            // bsp.py show 006d1fa0: MOV AL,1 / RET; ECX is unused, so the
            // live minimap unit handle needs no native-layout adapter.
            owner_.done("HudMinimap::unit_shows_on_minimap", bsp::kHudMinimapShowsAlwaysBody);
            return bsp::pool_pool13_cc13::unit_shows_on_minimap_true_006d1fa0(
                static_cast<std::uint8_t*>(unit));
        case bsp::HudMinimapUnitShows::Never:
            {
                // The recovered most-derived vtables distinguish class35's
                // 007001E0 from the 006F57D0 shared by land forts/command
                // buildings. Both bodies are XOR AL,AL / RET and read no
                // object fields (bsp.py show 006f57d0 and 007001e0).
                const auto body = bsp::hud_minimap_unit_shows_address(class_id);
                owner_.done("HudMinimap::unit_shows_on_minimap(fort)", body);
                return body == bsp::kHudMinimapShowsClass35Body
                    ? bsp::cc13_pool09::class35_shows_on_minimap_007001e0()
                    : bsp::pool_pool02_cc13::land_fort_shows_on_minimap_006f57d0();
            }
        case bsp::HudMinimapUnitShows::PlayerQuery:
            {
                // MLandVehicle's +B8h is 0074DDF0 (00CFFE98): 008DDF00 on the local
                // player slot's objective set [[00E188A8]+[..+18ECh]*4+21A4h], the
                // find in its +18h unit tree. That tree is
                // GameObjectiveSets::members, filled by Objectives_AddUnit and
                // trimmed by Objectives_RemoveUnit / the status calls
                // (src/game_hosts_lua.cpp), so a land vehicle is drawn only while
                // it is a listed objective unit of slot 0 (game+18ECh in single
                // player).
                LandVehicleObjectiveSetBinding set(owner_);
                owner_.done("HudMinimap::unit_shows_on_minimap(land vehicle)",
                    bsp::kHudMinimapShowsLandVehicleBody);
                return bsp::cc13_seg41::land_vehicle_in_local_objective_set_0074ddf0(unit, set);
            }
        case bsp::HudMinimapUnitShows::NotAUnit:
            break;
        }
        // Unreachable in the image: the walk tested IsKindOf(5) at 005C165D first.
        owner_.record("HudMinimap::unit_shows_on_minimap(unclassified)", 0x005c1675u);
        return true;
    }

    void* find_icon_entry(void* unit) override {
        // 005C170A -> 005BE110 on the map at screen+F8h, keyed by the unit
        // pointer; 005BCC00 then compares the result with end(). 005BE110 is
        // MSVC's std::map::find (lower bound down the _Tree from [map+4]+4 with
        // the key at node+0Ch and the nil byte at node+15h, then end() unless
        // the key is not below the found node's). The host's icon map is the
        // stand-in for the container, keyed by unit index, which is injective
        // over the same units, so the same find is the law.
        if (kHudMinimapUnitGateBound) {
            owner_.done("HudMinimap::find_icon_entry", 0x005be110u);
        } else {
            owner_.record("HudMinimap::find_icon_entry", 0x005be110u);
        }
        auto found = owner_.icons.find(owner_.index_of(unit));
        return found != owner_.icons.end() ? found->second : nullptr;
    }
    void* create_icon_entry(void* unit) override {
        // 005C173A: 005BD590 builds the entry (white template by IsKindOf, colour by
        // unit+54h) via MinimapIconEntryBinding; OFF keeps the 00639990 group clone.
        owner_.record("HudMinimap::allocate_icon_entry", 0x00bf681bu);
        GuiLayoutWidget* icon = owner_.create_icon_from_entry(unit);
        owner_.record("HudMinimap::insert_icon_entry", 0x005c0700u);
        owner_.record("HudMinimap::attach_icon_entry", 0x00694a60u);
        return icon;
    }

    void renderer_basis(float& y_component, float& x_component) override {
        // 005C17A7 -> 00B6DB70 then [renderer+110h] and [renderer+118h]. The
        // renderer at game+19FCh is the renderer owner's and this process has
        // none, so the executable supplies the controlled unit's own forward row
        // and records the refresh. The heading is then atan2(fwd.x, fwd.z),
        // which is the convention the trajectory dump already prints.
        owner_.done("HudMinimap::atan2", 0x00bf701au);
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (kMissionCameraBound && node.ready) {
            // 005C17A7: 00B6DB70 on the Operator node, then its world +110h and
            // +118h, the forward row's x and z (docs/MISSION_CAMERA.md).
            bsp::refresh_camera_world_00b6db70(node.state.transform);
            owner_.done("HudMinimap::refresh_camera_world", 0x00b6db70u);
            y_component = node.state.transform.world[8];
            x_component = node.state.transform.world[10];
            return;
        }
        owner_.record("HudMinimap::refresh_renderer_basis", 0x00b6db70u);
        y_component = 0.0f;
        x_component = 1.0f;
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return;
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!owner_.units->unit_pose(owner_.units->controlled_index(), right, up, forward,
                translation)) {
            return;
        }
        y_component = forward[0];
        x_component = forward[2];
    }

    void set_icon_position(void* icon, const bsp::HudGuiPoint& position) override {
        // 005C1C99 -> 00AA7DC0 BSP_GuiWidget_SetLocalPositionAndBounds.
        owner_.done("HudMinimap::set_icon_position", 0x00aa7dc0u);
        auto* widget = static_cast<GuiLayoutWidget*>(icon);
        if (widget == nullptr) return;
        owner_.menu.frontend().set_widget_local_position(*widget, position.x, position.y,
            position.z);
    }

    void set_icon_rotation(void* icon, float radians) override {
        // 005C1CD0, the widget rotation virtual +44h.
        owner_.done("HudMinimap::set_icon_rotation", 0x00aa7d00u);
        auto* widget = static_cast<GuiLayoutWidget*>(icon);
        if (widget == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*widget, radians);
    }

    void set_map_layer_rotation(void* widget, float radians) override {
        // 005C1825 and 005C184F, the same virtual on the compass and the map.
        owner_.done("HudMinimap::set_map_layer_rotation", 0x00aa7d00u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*typed, radians);
    }

    void* map_layer(std::size_t slot) override {
        if (slot == bsp::kHudMinimapCompassSlot) return owner_.compass;
        if (slot == bsp::kHudMinimapIslandMapSlot) return owner_.island_map;
        if (slot == bsp::kHudMinimapDirIconSlot) return owner_.direction;
        return nullptr;
    }

private:
    GameHudMinimapHost::Impl& owner_;
    // Packet cc9_recon_call_sites: the union triple the walk is over.
    std::vector<std::size_t> union_;
};

// 00CF142C (00 00 10 C1), the depth 005C221F loads for an objective of kind 0.
constexpr float kMinimapObjectiveDepthKind0 = -9.0f;

// bsp::ingame_gui_cc13::UnitSelectHost for 00645180 on an objective entry's
// unit (005C1F0D). The unit word is the address of a host unit index: in
// single player the body only hands it to 00645060 (its +54h read is the
// session arm, game+1FE4h != 0, which this process never takes).
class ObjectiveOwnUnitBinding final : public bsp::ingame_gui_cc13::UnitSelectHost {
public:
    explicit ObjectiveOwnUnitBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {}
    bool unit_selectable_00645060(void* unit, std::int32_t slot, int flag) override {
        // 00645060(unit, slot, flag): BL takes the flag only when game+2194h is
        // set (0064506F..0064507E), and slot 0 is the local player's record, so
        // the call is GameHudHost::unit_selectable_00645060 (game_hosts_hud.cpp).
        static_cast<void>(slot);
        static_cast<void>(flag);
        bool reached = false;
        const bool selectable =
            hud_unit_selectable_00645060(*static_cast<const std::size_t*>(unit), reached);
        if (reached) {
            owner_.done("HudMinimap::objective_unit_selectable", 0x00645060u);
        } else {
            owner_.record("HudMinimap::objective_unit_selectable", 0x00645060u);
        }
        return selectable;
    }
    std::int32_t local_player_slot() override { return 0; }   // game+18ECh, single player
    std::int32_t local_player_record_28() override { return local_player_party(); }
    std::int32_t session_mode() override { return 0; }        // game+1FE4h, single player

private:
    GameHudMinimapHost::Impl& owner_;
};

// bsp::cc13_seg56::ObjectiveEntityHost for 008DAD30's vtable +C4h. Every unit
// class installs 0042BA00 there (00CFC494 and 00CFFEA4 both hold 0042BA00):
// 00414DB0 while +C8h is clear, then +FCh..+104h, the world translation the
// minimap's world_position reads through 00427EB0.
class ObjectiveEntityPositionBinding final : public bsp::cc13_seg56::ObjectiveEntityHost {
public:
    explicit ObjectiveEntityPositionBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {}
    bsp::cc13_seg56::Vec3 entity_vcall_c4_position(void* entity) override {
        bsp::cc13_seg56::Vec3 out{};
        const std::size_t index = *static_cast<const std::size_t*>(entity);
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (owner_.units == nullptr
            || !owner_.units->unit_pose(index, right, up, forward, translation)) {
            owner_.record("HudMinimap::objective_entity_position", 0x0042ba00u);
            return out;
        }
        owner_.done("HudMinimap::objective_entity_position", 0x0042ba00u);
        out.x = translation[0];
        out.y = translation[1];
        out.z = translation[2];
        return out;
    }

private:
    GameHudMinimapHost::Impl& owner_;
};

// 005C1E00..005C22DE, the objective markers, run after the unit icon pass and
// only when that pass ran (005C1567..005C1578 leave through 005C27C5 without a
// camera unit). The five 005C0930 cursors go to -1 (005C1E00..005C1E1B); the
// local player slot's objective list (008DA180 on [game+21A4h+[game+18ECh]*4])
// is walked, and each objective that 008DA140 shows and 008DA130 finds active
// walks its +20h entries. 008DACD0 must reveal the entry; an entry with a unit
// is skipped when 00645180 finds the unit's vtable +140h answer (the unit, or a
// plane's squadron) to be a local player unit; 008DAD30 gives the point.
// 005C1F26..005C2143 then repeat the unit icon's rim clamp and transform
// (hud_minimap_icon_position_005c1b62) at depth -9.0f (00CF142C) for an
// objective of kind 0, -8.0f (00CE3CC8) otherwise. The marker 005C0930 returns
// for the entry's unit takes virtual +88h(d2 > range*range, 0, 1.0f), +50h with
// white +D0h when the unit is the displayed self unit [ESP+5Ch] or a plane whose
// +9D4h squadron is that unit, else yellow +A0h for kind 0 and silver +B0h,
// 00AA7DC0 the position, and +44h pi/2 - (unit +50h heading + icon heading), 0
// without a unit. 008E2BF0 then prunes the ping list [game+21C8h] and 005C0BD0
// drops each pool's surplus.
//
// The entries are the +20h list the Objectives bindings keep
// (game_objective_entry_list, src/game_hosts_lua.cpp): 008DEE50's unit records
// and 008DEEB0's position records (unit word 0, revealed by 008DACD0's first
// test, pool 0, item_marker_Icon) in push order. Kind and state come from
// GameObjectiveSets, which the status bindings write. A dead unit stays listed:
// the set's observer detach 008DF1E0 is not built (the Lua host's
// ObjectiveListHost labels that), so its marker goes when the script completes
// or fails the objective or removes the unit.
void place_objective_markers_005c1e00(GameHudMinimapHost::Impl& host, MinimapBinding& binding) {
    void* camera = binding.camera_unit();
    if (camera == nullptr || host.units == nullptr) return;
    binding.refresh_pose(camera);
    const bsp::HudMinimapWorldPoint camera_pos = binding.world_position(camera);
    const float range = binding.minimap_range();
    // [ESP+5Ch], 00927880 on 00E188D8 at 005C15F2 (the walk's own record).
    void* const self_handle = binding.displayed_self_unit(nullptr);
    const std::size_t self = self_handle != nullptr ? host.index_of(self_handle)
                                                    : bsp::kPlaneSquadronNoUnit;

    for (std::int32_t& cursor : host.objective_marker_screen.cursors) cursor = -1;   // 005C1E00..005C1E1B

    const int party = local_player_party();   // [[game+18CCh+slot*4]+28h], the record row
    const GameGunneryHost* gunnery = host.units->gunnery();
    ObjectiveOwnUnitBinding own(host);
    ObjectiveEntityPositionBinding entity_position(host);
    std::size_t placed = 0;
    // One +20h entry: an 008DEE50 record names a unit, an 008DEEB0 record has a
    // null unit word and the point at +4h..+0Ch (005C1EDE..005C22A4).
    const auto place_entry = [&](const GameObjectiveSets::Objective& objective, bool has_unit,
                                 std::size_t unit, const bsp::ObjectiveWorldPoint& stored) {
        // 005C1EE7 008DACD0: true without a unit; otherwise entity+1E8h +
        // party*34h, +8h when the force byte +10h is set, else +4h, above 1. The
        // record is the recon pass's persistent one (kReconDetectionRecordsBound);
        // a unit no pass has laid out yet holds the zero record its constructor
        // wrote.
        bsp::cc13_seg56::ObjectiveRevealView reveal;
        reveal.has_entity = has_unit;
        reveal.local_side = 0;
        reveal.side_party_row = party;
        const bsp::ReconDetectionRecord* record = has_unit && gunnery != nullptr
            ? gunnery->recon_sensor_pass_state().record(party, unit) : nullptr;
        if (record != nullptr) {
            reveal.row_selector_byte = record->forced;
            reveal.row_value_plus4 = static_cast<std::int32_t>(record->level);
            reveal.row_value_plus8 = static_cast<std::int32_t>(record->forced_level);
        }
        const bool revealed =
            bsp::cc13_seg56::objective_entry_is_revealed_to_local_side_008dacd0(reveal);
        if (!revealed) {
            ++host.summary.objective_entries_hidden;
            return;
        }
        host.done("HudMinimap::objective_entry_revealed", 0x008dacd0u);
        // 005C1EF4..005C1F14, only with a unit: vtable +140h (0047F320, the unit
        // itself; a plane answers its squadron, the plane registry's record),
        // then 00645180 with ECX = [00E198C4]+40h, which the body does not read.
        const bsp::PlaneSquadronHostRecord* squadron =
            has_unit ? bsp::plane_squadron_registry().find_by_member_unit(unit) : nullptr;
        if (has_unit) {
            std::size_t owner = unit;
            if (squadron != nullptr && squadron->squadron_unit != bsp::kPlaneSquadronNoUnit) {
                owner = squadron->squadron_unit;
            }
            if (bsp::ingame_gui_cc13::unit_is_local_player_unit_00645180(
                    bsp::ingame_gui_cc13::Raw{reinterpret_cast<std::uint8_t*>(&owner)}, own)) {
                ++host.summary.objective_entries_own;
                return;
            }
            host.done("HudMinimap::objective_unit_is_local", 0x00645180u);
        }
        // 005C1F21 008DAD30 into [ESP+48h]: the unit's vtable +C4h, or the
        // record's stored point.
        std::size_t entity_word = unit;
        bsp::cc13_seg56::ObjectiveEntryView entry;
        entry.entity = has_unit ? &entity_word : nullptr;
        entry.stored_position = bsp::cc13_seg56::Vec3{stored.x, stored.y, stored.z};
        const bsp::cc13_seg56::Vec3 at =
            bsp::cc13_seg56::objective_entry_position_008dad30(entry, entity_position);
        host.done("HudMinimap::objective_entry_position", 0x008dad30u);
        const bsp::HudMinimapWorldPoint point{at.x, at.y, at.z};

        // 005C1F26..005C1F86: d2 from camera minus point, (x*x + y*y) + z*z,
        // against range*range ([ESP+58h], squared once at 005C15B1).
        const float dx = camera_pos.x - point.x;
        const float dy = camera_pos.y - point.y;
        const float dz = camera_pos.z - point.z;
        const float d2 = (dx * dx + dy * dy) + dz * dz;
        const bool outside = d2 > range * range;
        const float depth = objective.kind == 0 ? kMinimapObjectiveDepthKind0
                                                : bsp::kHudMinimapDepthCapture;
        const bsp::HudGuiPoint position = bsp::hud_minimap_icon_position_005c1b62(point,
            camera_pos, range, host.icon_heading, depth);

        // 005C2143 005C0930(screen, [entry]): the pool by the unit's kind, 0
        // without a unit.
        GuiLayoutWidget* marker = host.acquire_objective_marker(has_unit ? host.handle_for(unit) : nullptr);
        if (marker == nullptr) return;
        ++placed;
        // 005C214D..005C2176 virtual +88h(outside, 0, 1.0f), the icon's state.
        // The sprite bridge draws a widget's first authored state only.
        static_cast<void>(outside);
        host.record("HudMinimap::objective_marker_state", 0x005c2176u);
        // 005C2178..005C2213 virtual +50h.
        std::size_t colour = objective.kind == 0 ? 0 : 1;   // yellow +A0h, silver +B0h
        bool white = false;
        if (!has_unit) {
            white = false;                     // 005C217C..005C21A0
        } else if (self != bsp::kPlaneSquadronNoUnit && unit == self) {
            white = true;                      // 005C21A2..005C21AC, a non-null self
        } else if (host.units->unit_is_kind_of(unit, 0x0F)) {
            // 005C21AE..005C21C7: the plane's +9D4h squadron against self.
            // +9D4h is the registry's record of the plane (null without one); a
            // record without a unit slot is a native squadron this host cannot
            // name, which a null self never equals.
            white = squadron == nullptr
                ? self == bsp::kPlaneSquadronNoUnit
                : self != bsp::kPlaneSquadronNoUnit && squadron->squadron_unit == self;
        }
        if (white) colour = 2;   // white +D0h
        if (host.objective_colour_bound[colour]) {
            const float* rgba = host.objective_colours[colour];
            host.menu.frontend().set_widget_color(*marker, rgba[0], rgba[1], rgba[2], rgba[3]);
        } else {
            host.record("HudMinimap::objective_marker_colour_unbound", 0x005be240u);
        }
        // 005C2269 00AA7DC0.
        host.menu.frontend().set_widget_local_position(*marker, position.x, position.y,
            position.z);
        // 005C226E..005C22A4 virtual +44h: 0 without a unit (005C2297 FLDZ).
        float rotation = 0.0f;
        if (has_unit) {
            float unit_heading = 0.0f;
            if constexpr (bsp::kHudHeadingVtable50Cc13Bound) {
                unit_heading = host.units->unit_heading_vtable50(unit);   // 005C227E vtable +50h
            } else {
                const GameUnitRow* row = host.units->unit_row(unit);
                unit_heading = row != nullptr ? row->heading_degrees * 3.1415927f / 180.0f : 0.0f;
            }
            rotation = bsp::hud_minimap_icon_rotation_005c1ca1(unit_heading, host.icon_heading);
        }
        host.menu.frontend().set_widget_rotation(*marker, rotation);
    };

    // 008DA180([game+21A4h + 4*[game+18ECh]]), slot 0 in single player.
    const std::vector<GameObjectiveSets::Objective>& objectives = game_objective_sets().slots[0];
    for (std::size_t i = 0; i < objectives.size(); ++i) {
        const GameObjectiveSets::Objective& objective = objectives[i];
        if (!bsp::cc13_band0085::objective_is_shown_008da140(objective.kind, objective.state)) continue;
        if (!bsp::cc13_band0085::objective_is_active_008da130(objective.state)) continue;
        // The +20h list (005C1EA1..005C1EC5), as the Objectives bindings keep
        // it; an objective no binding pushed into has the unit projection only.
        if (const bsp::ObjectiveUnitList* list = game_objective_entry_list(0, i)) {
            for (const bsp::ObjectiveUnitEntry& e : list->entries) {
                if (e.unit != nullptr) {
                    place_entry(objective, true, game_objective_entry_unit(e.unit), {});
                } else {
                    place_entry(objective, false, 0, e.position);
                }
            }
        } else {
            for (const std::size_t unit : objective.units) place_entry(objective, true, unit, {});
        }
    }
    GameMissionResultHost* const result = host.lua != nullptr
        ? host.lua->mission_result() : nullptr;
    if (result == nullptr)
        throw std::logic_error("Minimap ping expiry requires the attached mission clock");
    host.map_pings.prune(*result);
    host.done("HudMinimap::map_ping_prune", 0x008e2bf0u);
    host.hide_unused_objective_markers();
    host.summary.objective_markers_placed = placed;
    if (placed > 0 && !host.logged_objective_marker) {
        host.logged_objective_marker = true;
        host.log.notef("minimap objective markers: %zu placed (slot 0, party %d; 008dacd0 "
            "hid %llu entries, 00645180 skipped %llu own units so far)", placed, party,
            host.summary.objective_entries_hidden, host.summary.objective_entries_own);
    }
}

}  // namespace

GuiLayoutWidget* GameHudMinimapHost::Impl::acquire_objective_marker(void* unit) {
    if (page == nullptr || marker_group == nullptr || objective_marker_template == nullptr
        || std::any_of(std::begin(entry_templates), std::end(entry_templates),
            [](const auto* widget) { return widget == nullptr; })) {
        record("HudMinimap::construct_objective_marker_missing_layout", 0x005bcac0u);
        return nullptr;
    }
    MinimapObjectiveMarkerBinding binding(*this);
    auto* marker = static_cast<ObjectiveMarker*>(
        bsp::frontend_seg20_cc13::minimap_acquire_marker_005c0930(
            &objective_marker_screen, unit, binding));
    done("HudMinimap::acquire_objective_marker", 0x005c0930u);
    return marker != nullptr ? marker->widget : nullptr;
}

void GameHudMinimapHost::Impl::hide_unused_objective_markers() {
    MinimapObjectiveMarkerBinding binding(*this);
    bsp::cc13_sweepB3_hud_minimap_hide_unused_markers_005c0bd0(&objective_marker_screen, binding);
    done("HudMinimap::release_unused_markers", 0x005c0bd0u);
}

void GameHudMinimapHost::Impl::bind_page_005bec50() {
    if (bound_attempted) return;
    bound_attempted = true;
    // 005BEC50 loads GUI_minimap and stores the page root at +1Ch; it and the
    // layout binder 005BE240 then take every widget by name through 00AA7E00.
    // The screen's own register virtual is one of the 42 records, so what runs
    // here is the widget bind, over the page the HUD manager's Init loaded.
    record("HudMinimap::screen_register", 0x005bec50u);
    page = menu.in_game_page(kMinimapScreenSlot, bsp::kHudMinimapPageName);
    if (page == nullptr || !page->root) {
        log.note("minimap: GUI_minimap is not loaded on registry slot 35h, so the unit "
            "icon pass has no page to place icons in");
        return;
    }
    summary.page_bound = true;
    GuiLayoutWidget& root = *page->root;
    compass = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapCompassIcon);
    island_map = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapIslandMapIcon);
    direction = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapDirIcon);
    marker_group = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapMarkerGroup);
    done("HudMinimap::find_child", 0x00aa7e00u);
    summary.marker_group_bound = marker_group != nullptr;
    for (std::size_t slot = 0; slot < 6; ++slot) {
        GuiLayoutWidget* group = bsp::find_child_by_name_00aa7e00(root,
            bsp::kHudMinimapUnitGroups[slot]);
        if (group == nullptr) continue;
        colour_templates[slot] = bsp::find_child_by_name_00aa7e00(*group,
            bsp::kHudMinimapUnitItemIcon);
        if (colour_templates[slot] != nullptr) ++summary.colour_templates;
    }
    summary.groups_bound = summary.colour_templates > 0;
    bind_icon_entry_fields(root);
    // The island map's own content. minimap_terrain.mshd is the two-sampler GUI
    // shader effect; `RadarMap` at texture register 0 is a render target the
    // renderer owner produces, and 005BED21 is where 005BEC50 takes the widget
    // that receives it. Nothing in this process can produce that target.
    record("HudMinimap::radar_map_target", 0x005bed21u);
    log.notef("minimap page bound: %zu of 6 colour templates, unit_marker_Group %s, "
        "compass %s, island map %s, direction wedge %s; the RadarMap render target "
        "minimap_terrain.mshd samples is the renderer owner's and stays a record, so the "
        "island map keeps the page's authored error.tga",
        summary.colour_templates, marker_group != nullptr ? "bound" : "missing",
        compass != nullptr ? "bound" : "missing",
        island_map != nullptr ? "bound" : "missing",
        direction != nullptr ? "bound" : "missing");
}

GuiLayoutWidget* GameHudMinimapHost::Impl::create_icon(std::size_t index) {
    if (page == nullptr || marker_group == nullptr || units == nullptr) return nullptr;
    const GameUnitRow* row = units->unit_row(index);
    if (row == nullptr) return nullptr;
    const int colour = marker_colour_index_00639990(row->party);
    record("HudMinimap::marker_colour_index", 0x00639990u);
    GuiLayoutWidget* templ = colour_templates[colour < 0 || colour > 5 ? 0 : colour];
    if (templ == nullptr) {
        for (GuiLayoutWidget* candidate : colour_templates) {
            if (candidate != nullptr) { templ = candidate; break; }
        }
    }
    if (templ == nullptr) return nullptr;
    char key[64];
    std::snprintf(key, sizeof(key), "item_ship_Icon_%zu", index);
    // The clone is parented to `unit_marker_Group`, screen +F0h, which
    // docs/HUD_MINIMAP.md calls the per-unit icon map's parent: its authored
    // position (0.865167, 0.164028) is the minimap centre, and the six colour
    // groups' positions are not, so an icon whose local position is the small
    // offset 005C1C61 computes only lands on the map under this parent. Which
    // group the native entry is parented to is the open question of packet
    // `hud_minimap_icon_entries`.
    GuiLayoutWidget* icon = menu.frontend().clone_runtime_widget(page->name, *templ,
        *marker_group, key);
    if (icon == nullptr) return nullptr;
    icons.emplace(index, icon);
    ++summary.icons_created;
    if (!logged_entry) {
        logged_entry = true;
        log.notef("minimap icons: the 0Ch-byte per-unit entry of 005bd590 / 005c0700 / "
            "00694a60 has no reconstruction, so the executable clones the page's own "
            "`item_ship_Icon` template (colour index %d of six, from 00639990's rule on "
            "party %d) under unit_marker_Group and drives the clone through the sprite "
            "bridge; the clone is an executable-side stand-in", colour, row->party);
    }
    return icon;
}

namespace {

// bsp::MinimapIconHost, the boundary of 005BD590 (src/frontend_menus.cpp), over
// this screen's Impl (chain 47). The unit is the walk's UnitHandle.
class MinimapIconEntryBinding final : public bsp::MinimapIconHost {
public:
    MinimapIconEntryBinding(GameHudMinimapHost::Impl& owner, std::size_t index)
        : owner_(owner), index_(index) {}

    // Virtual +5Ch, IsKindOf, by the class the unit's VehicleClass selected.
    bool unit_is_kind_5c(bsp::HudUnitRef unit, int class_id) override {
        return owner_.units->unit_is_kind_of(owner_.index_of(unit.native), class_id);
    }
    // unit+54h, the side: the row field the host already reads as [unit+54h]
    // for 00639990 (marker_colour_index_00639990 above).
    std::int32_t unit_side_54(bsp::HudUnitRef unit) override {
        const GameUnitRow* row = owner_.units->unit_row(owner_.index_of(unit.native));
        return row != nullptr ? row->party : -1;
    }
    // 00AAB4C0(template, parent 0). The host's clone takes its parent at creation,
    // so the clone is made under unit_marker_Group (+F0h), where 00AAA5A0 puts it.
    bsp::MainMenuWidgetRef clone_template_00aab4c0(bsp::MinimapField field) override {
        GuiLayoutWidget* templ = template_for(field);
        if (templ == nullptr || owner_.page == nullptr || owner_.marker_group == nullptr) {
            owner_.record("HudMinimap::entry_template_unbound", 0x00aab4c0u);
            return {};
        }
        char suffix[32];
        std::snprintf(suffix, sizeof(suffix), "_%zu", index_);
        GuiLayoutWidget* icon = owner_.menu.frontend().clone_runtime_widget(owner_.page->name,
            *templ, *owner_.marker_group, templ->key + suffix);
        owner_.done("HudMinimap::clone_template", 0x00aab4c0u);
        return bsp::MainMenuWidgetRef{icon};
    }
    // Virtual +50h (00AA6870 copies the RGBA to widget +50h) with the screen's
    // +80h, +90h or +C0h colour.
    void widget_set_colour_50(bsp::MainMenuWidgetRef widget,
        bsp::MinimapField colour_field) override {
        auto* icon = static_cast<GuiLayoutWidget*>(widget.native);
        const int slot = colour_slot(colour_field);
        if (icon == nullptr || slot < 0 || !owner_.entry_colour_bound[slot]) {
            owner_.record("HudMinimap::entry_colour_unbound", 0x005bd590u);
            return;
        }
        const float* colour = owner_.entry_colours[slot];
        owner_.menu.frontend().set_widget_color(*icon, colour[0], colour[1], colour[2],
            colour[3]);
    }
    void attach_to_unit_marker_group_00aaa5a0(bsp::MainMenuWidgetRef widget) override {
        // Already done by the clone above, whose parent is unit_marker_Group.
        if (widget.native != nullptr) {
            owner_.done("HudMinimap::attach_to_marker_group", 0x00aaa5a0u);
        }
    }
    void widget_set_visible_34(bsp::MainMenuWidgetRef widget, bool visible) override {
        auto* icon = static_cast<GuiLayoutWidget*>(widget.native);
        if (icon != nullptr) owner_.menu.frontend().set_widget_visible(*icon, visible);
    }

private:
    GuiLayoutWidget* template_for(bsp::MinimapField field) const noexcept {
        switch (field) {
        case bsp::MinimapField::ShipTemplate: return owner_.entry_templates[0];
        case bsp::MinimapField::PlaneTemplate: return owner_.entry_templates[1];
        case bsp::MinimapField::SubTemplate: return owner_.entry_templates[2];
        case bsp::MinimapField::BuildingTemplate: return owner_.entry_templates[3];
        default: return nullptr;
        }
    }
    static int colour_slot(bsp::MinimapField field) noexcept {
        switch (field) {
        case bsp::MinimapField::RedColour: return 0;
        case bsp::MinimapField::BlueColour: return 1;
        case bsp::MinimapField::GreyColour: return 2;
        default: return -1;
        }
    }

    GameHudMinimapHost::Impl& owner_;
    std::size_t index_;
};

}  // namespace

void GameHudMinimapHost::Impl::bind_icon_entry_fields(GuiLayoutWidget& root) {
    // 005BE240 (chain 47): minimap_units_white_Group's four unit templates into
    // +6Ch..+78h, then virtual +54h of the red, blue and grey groups' item_ship_Icon
    // copied 16 bytes each into +80h, +90h and +C0h. The colour groups' icons are
    // colour_templates[1..3], bound above by the same two names.
    static constexpr const char* kEntryTemplateNames[4] = {
        "item_ship_Icon", "item_plane_Icon", "item_sub_Icon", "item_building_Icon"};
    GuiLayoutWidget* white = bsp::find_child_by_name_00aa7e00(root,
        bsp::kHudMinimapUnitGroups[0]);
    std::size_t templates_bound = 0;
    if (white != nullptr) {
        for (std::size_t slot = 0; slot < 4; ++slot) {
            entry_templates[slot] = bsp::find_child_by_name_00aa7e00(*white,
                kEntryTemplateNames[slot]);
            if (entry_templates[slot] != nullptr) ++templates_bound;
        }
    }
    static constexpr std::size_t kColourGroups[3] = {1, 2, 3};  // red, blue, grey
    std::size_t colours_bound = 0;
    for (std::size_t slot = 0; slot < 3; ++slot) {
        const GuiLayoutWidget* icon = colour_templates[kColourGroups[slot]];
        if (icon == nullptr) continue;
        for (std::size_t lane = 0; lane < 4; ++lane) {
            entry_colours[slot][lane] = icon->color[lane];
        }
        entry_colour_bound[slot] = true;
        ++colours_bound;
    }
    // 005BE52F..005BE553: item_marker_Icon of the white group into +7Ch; then
    // the three colours the objective loop uses: +D0h from +6Ch's virtual +54h
    // (005BE579..005BE58A), +A0h and +B0h from the yellow and silver groups'
    // item_ship_Icon (005BE9EA, 005BEB02).
    if (white != nullptr) {
        objective_marker_template = bsp::find_child_by_name_00aa7e00(*white, "item_marker_Icon");
    }
    const GuiLayoutWidget* objective_colour_icons[3] = {
        colour_templates[4], colour_templates[5], entry_templates[0]};   // yellow, silver, white
    for (std::size_t slot = 0; slot < 3; ++slot) {
        const GuiLayoutWidget* icon = objective_colour_icons[slot];
        if (icon == nullptr) continue;
        for (std::size_t lane = 0; lane < 4; ++lane) {
            objective_colours[slot][lane] = icon->color[lane];
        }
        objective_colour_bound[slot] = true;
    }
    log.notef("minimap icon entry fields (005be240): %zu of 4 white-group templates, "
        "%zu of 3 side colours (red %.3f %.3f %.3f %.3f, blue %.3f %.3f %.3f %.3f, "
        "grey %.3f %.3f %.3f %.3f)", templates_bound, colours_bound,
        static_cast<double>(entry_colours[0][0]), static_cast<double>(entry_colours[0][1]),
        static_cast<double>(entry_colours[0][2]), static_cast<double>(entry_colours[0][3]),
        static_cast<double>(entry_colours[1][0]), static_cast<double>(entry_colours[1][1]),
        static_cast<double>(entry_colours[1][2]), static_cast<double>(entry_colours[1][3]),
        static_cast<double>(entry_colours[2][0]), static_cast<double>(entry_colours[2][1]),
        static_cast<double>(entry_colours[2][2]), static_cast<double>(entry_colours[2][3]));
}

GuiLayoutWidget* GameHudMinimapHost::Impl::create_icon_from_entry(void* unit) {
    if (page == nullptr || marker_group == nullptr || units == nullptr) return nullptr;
    const std::size_t index = index_of(unit);
    const GameUnitRow* row = units->unit_row(index);
    if (row == nullptr) return nullptr;
    MinimapIconEntryBinding binding(*this, index);
    const bsp::MinimapIconEntry entry =
        bsp::construct_minimap_icon_entry_005bd590(bsp::HudUnitRef{unit}, binding);
    done("HudMinimap::construct_icon_entry", 0x005bd590u);
    auto* icon = static_cast<GuiLayoutWidget*>(entry.widget.native);
    if (icon == nullptr) return nullptr;
    icons.emplace(index, icon);
    ++summary.icons_created;
    if (!logged_entry_body) {
        logged_entry_body = true;
        log.notef("minimap icons: 005bd590 builds each entry; the first is %s (party %d, "
            "kind sub %d ship %d plane %d) as a clone of %s under unit_marker_Group",
            row->name.c_str(), row->party, units->unit_is_kind_of(index, 0x08) ? 1 : 0,
            units->unit_is_kind_of(index, 0x06) ? 1 : 0,
            units->unit_is_kind_of(index, 0x0F) ? 1 : 0, icon->key.c_str());
    }
    return icon;
}

GameHudMinimapHost::GameHudMinimapHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudMinimapHost::~GameHudMinimapHost() = default;

const GameHudMinimapSummary& GameHudMinimapHost::summary() const noexcept {
    return impl_->summary;
}

void GameHudMinimapHost::attach_world(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& host = *impl_;
    host.units = &units;
    host.lua = &lua;
    host.handles.clear();
    host.handles.resize(units.count());
    for (std::size_t index = 0; index < host.handles.size(); ++index) {
        host.handles[index].index = index;
    }
    if (host.range_attempted) return;
    host.range_attempted = true;
    // 0087D7B0 writes the two radii into the global config object at +6Ch and
    // +70h from Globals["Minimap"]; the loader is not reconstructed, so only its
    // two reads are performed, against the installed globals.lua.
    float range = 0.0f;
    float visibility = 0.0f;
    if (lua.read_minimap_globals_0087d7b0(range, visibility)) {
        host.summary.minimap_range = range;
        host.summary.visibility_range = visibility;
        host.summary.range_from_data = true;
    } else {
        host.log.note("minimap range: Globals[\"Minimap\"] did not load, so the pass has no "
            "radius and places no icon; 005c1799 would divide 80.0 by zero");
    }
}

void GameHudMinimapHost::update_005c0f20(float seconds) {
    Impl& host = *impl_;
    static_cast<void>(seconds);
    host.bind_page_005bec50();
    if (!host.summary.page_bound || host.units == nullptr) {
        host.record("HudMinimap::update", 0x005c0f20u);
        return;
    }
    MinimapBinding binding(host);
    binding.prepare_frame();

    // The heading the whole frame shares, 005C1788..005C1812. The pass computes
    // it again internally; it is recomputed here because the icon rotation at
    // 005C1CA1 is outside what src/hud_minimap.cpp covers.
    float basis_y = 0.0f;
    float basis_x = 0.0f;
    binding.renderer_basis(basis_y, basis_x);
    const bsp::HudMinimapHeadings headings = bsp::hud_minimap_headings_005c1788(
        std::atan2(basis_y, basis_x), host.summary.minimap_range);
    host.icon_heading = headings.icon;
    host.summary.camera_heading = headings.map;

    const std::size_t before = host.icons.size();
    // The team index is [00E188A8+18ECh]; this process's local player is the
    // controlled unit's own party, which is 0 for the Allied side of USN02.
    int team = 0;
    if (host.units->controlled_bound()) {
        const GameUnitRow* row = host.units->unit_row(host.units->controlled_index());
        if (row != nullptr && row->party >= 0 && row->party <= 7) team = row->party;
    }
    const int placed = bsp::hud_minimap_place_unit_icons_005c154e(team, binding,
        bsp::kHudMinimapDepthSelf);
    host.done("HudMinimap::update", 0x005c0f20u);
    host.summary.icons_placed = static_cast<std::size_t>(placed < 0 ? 0 : placed);
    const std::size_t reachable = host.units->count() > 0 ? host.units->count() - 1 : 0;
    host.summary.units_culled = reachable > host.summary.icons_placed
        ? reachable - host.summary.icons_placed : 0;
    ++host.summary.frames;

    // 005C1CA1..005C1CD0: pi/2 minus the sum of the unit's own heading and the
    // icon heading, handed to the widget rotation virtual +44h. The pass in
    // src/hud_minimap.cpp places positions only, so the rotation runs here.
    for (const auto& entry : host.icons) {
        const GameUnitRow* row = host.units->unit_row(entry.first);
        if (row == nullptr || entry.second == nullptr) continue;
        float unit_heading = 0.0f;
        if constexpr (bsp::kHudHeadingVtable50Cc13Bound) {
            // 005C1CA6 CALL [vtable+50h] on the icon's unit.
            unit_heading = host.units->unit_heading_vtable50(entry.first);
        } else {
            unit_heading = row->heading_degrees * 3.1415927f / 180.0f;
        }
        binding.set_icon_rotation(entry.second,
            bsp::hud_minimap_icon_rotation_005c1ca1(unit_heading, headings.icon));
    }
    // 005C1E00..005C22DE, after the unit loop: the objective markers.
    place_objective_markers_005c1e00(host, binding);
    // 005C1872, the direction wedge: SetRotation(camera->vtable[C8h]() -
    // icon_heading + pi/2). The camera object is the renderer owner's, so the
    // heading it would return is recorded and the controlled unit's own is used.
    if (host.direction != nullptr) {
        bool squadron = false;
        const std::size_t camera = kHudMinimapDirectionWedgeBound
            ? camera_unit_004b4b00(host.units, squadron) : 0;
        float right[3], up[3], forward[3], origin[3];
        if (camera != 0 && host.units->unit_pose(camera - 1, right, up, forward, origin)) {
            // 0042BA40 (vtable +C8h): 00414DB0 refreshes the pose, then
            // FSTP float of atan2(+ECh, +F4h) and FCHS; 005C1864 FSUB the icon
            // heading, 005C186C FADD double pi/2 (00CF1438), 005C1874 FSTP.
            const float yaw = static_cast<float>(std::atan2(static_cast<double>(forward[0]),
                                                            static_cast<double>(forward[2])));
            float wedge = 0.0f;
            if constexpr (bsp::kPc24HudGuiCc13Bound) {
                // 005C1864 FSUB float and 005C186C FADD double each round at PC24.
                wedge = static_cast<float>(bsp::cc13x87::x87_add24(
                    bsp::cc13x87::x87_sub24(-yaw, headings.icon), 1.5707963267948966));
            } else {
                wedge = static_cast<float>(static_cast<double>(-yaw)
                    - static_cast<double>(headings.icon) + 1.5707963267948966);
            }
            host.done("HudMinimap::camera_heading_0042ba40", 0x0042ba40u);
            binding.set_map_layer_rotation(host.direction, wedge);
        } else {
            host.record("HudMinimap::camera_heading_virtual", 0x004b4b00u);
            binding.set_map_layer_rotation(host.direction,
                bsp::kHudMinimapHeadingBias - headings.icon);
        }
    }
    if (host.icons.size() != before && !host.logged_camera) {
        host.logged_camera = true;
        host.log.notef("minimap unit icons: %zu icon(s) created out of %zu created unit(s); "
            "the camera unit is the controlled unit (004b4b00 is a record, there is no "
            "camera at game+19FCh), so the player's own ship carries no dot and is the "
            "centre direction wedge, exactly as 005c1650 skips it",
            host.icons.size(), host.units->count());
    }
    // Where the icons actually land, once per run and again every 100 frames, so
    // the picture can be checked against the transform rather than trusted.
    if (host.summary.frames == 1 || host.summary.frames % 100 == 0) {
        const auto first = host.icons.begin();
        if (first != host.icons.end() && first->second != nullptr) {
            const GameUnitRow* row = host.units->unit_row(first->first);
            host.log.notef("  minimap frame %-4llu placed=%zu drawn=%zu  first=%s "
                "local=(%.5f, %.5f, %.1f) parent=%s", host.summary.frames,
                host.summary.icons_placed, host.menu.frontend().runtime_widgets_drawn(),
                row != nullptr ? row->name.c_str() : "?",
                static_cast<double>(first->second->transform.position.x),
                static_cast<double>(first->second->transform.position.y),
                static_cast<double>(first->second->transform.position.z),
                host.marker_group != nullptr ? host.marker_group->key.c_str() : "-");
        }
    }
}

void GameHudMinimapHost::report() {
    Impl& host = *impl_;
    if (!host.summary.page_bound) return;
    host.log.notef("summary mission minimap range=%.0f visibility=%.0f from_data=%d "
        "icons=%zu placed=%zu culled=%zu frames=%llu heading=%.4f rad",
        static_cast<double>(host.summary.minimap_range),
        static_cast<double>(host.summary.visibility_range),
        host.summary.range_from_data ? 1 : 0, host.summary.icons_created,
        host.summary.icons_placed, host.summary.units_culled, host.summary.frames,
        static_cast<double>(host.summary.camera_heading));
    host.log.notef("summary mission minimap objectives markers_placed=%zu markers_created=%zu "
        "entries_hidden=%llu own_units_skipped=%llu (005c1e00: 008dacd0 / 00645180 / 008dad30)",
        host.summary.objective_markers_placed, host.summary.objective_markers_created,
        host.summary.objective_entries_hidden, host.summary.objective_entries_own);
}

// ===========================================================================
// Slot 4Dh, the world markers
// ===========================================================================

struct GameHudMarkersHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in)
        : log(log_in), menu(menu_in),
          fallback_pose{bsp::PoseRefreshParentSlot(fallback_parent), fallback_local,
              fallback_valid, fallback_world, fallback_derived} {}

    GameHostLog& log;
    GameMenuHost& menu;
    GameUnitsHost* units{nullptr};
    GameHudMarkersSummary summary;

    GuiLayoutPage* page{nullptr};
    GuiLayoutWidget* sidemarker{nullptr};       // the authored template
    GuiLayoutWidget* type_icon{nullptr};        // its `type_Icon` child
    std::map<std::size_t, GuiLayoutWidget*> markers;
    // The four pools of 00640620, screen 4Dh +4Ch..+A0h. Pool A holds the
    // host's marker clones in acquisition order; B, C and D stay empty
    // because the host builds none of their entries (the target callout,
    // the group labels and the owned objects).
    std::vector<GuiLayoutWidget*> pool_a;      // first +50h, last +54h
    std::uint32_t pool_cursor_a{0};            // +7Ch
    std::uint32_t pool_cursor_b{0};            // +80h
    std::uint32_t pool_cursor_c{0};            // +84h
    std::uint32_t pool_cursor_d{0};            // +A0h
    std::size_t pool_b_size{0};
    std::size_t pool_c_size{0};
    std::size_t pool_d_size{0};
    void reset_marker_pools_00640620();

    bsp::HudMarkersUpdateState state{};
    bsp::HudMarkerClipRect clip{};
    bool wide_aspect{false};
    bool bound_attempted{false};
    bool logged_selection{false};
    bool logged_camera{false};
    // The marked set at screen +34h, as an index set for one frame.
    std::vector<std::size_t> marked;

    struct UnitHandle { std::size_t index{0}; };
    std::vector<UnitHandle> handles;

    // The pose the squad arm of 006435D0 would read. That arm never runs here:
    // its list is the world object's and the radius at unit+7C4h has no field.
    bsp::CameraMatrix fallback_local{};
    bsp::CameraMatrix fallback_world{};
    std::uint8_t fallback_valid{1};
    std::uint8_t fallback_derived{1};
    bsp::PoseRefreshView* fallback_parent{nullptr};
    bsp::PoseRefreshView fallback_pose;

    // Packet cc13-fix24: host copies of 0077A2F0's function-local statics,
    // zero like the image's .bss until the guard bits fill them.
    std::uint32_t health_colour_guard_f8718c{0};
    float health_colour_low_f8715c[4]{};
    float health_colour_mid_f8716c[4]{};
    float health_colour_high_f8717c[4]{};

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

    std::size_t index_of(void* handle) const noexcept {
        auto* typed = static_cast<UnitHandle*>(handle);
        return typed != nullptr ? typed->index : 0;
    }

    void bind_page_0063e280();
    void fit_stand_in_camera();
    const char* display_name_for_unit(std::size_t index, const bsp::HudMarkerFlags& flags);
    void set_marker_name(GuiLayoutWidget& marker, std::size_t index,
        const bsp::HudMarkerFlags& flags);
    void place_marker(std::size_t index, const bsp::HudMarkerScreenBounds& bounds,
        const bsp::HudMarkerFlags& flags);
};

namespace {

class MarkerReconNameBinding final : public bsp::cc13_seg49::MarkerDisplayNameHost {
public:
    explicit MarkerReconNameBinding(GameHudMarkersHost::Impl& owner) : owner_(owner) {}

    bool marked_unit_is_aircraft(void* marked_unit) override {
        return owner_.units->unit_is_kind_of(owner_.index_of(marked_unit), 0x0f);
    }
    bool marked_plane_is_flight_leader(void* marked_unit) override {
        // 007B8AD0 reads plane+9D8h == 0, the original spawn-index stamp.
        // It is not the current leader's position after member compaction.
        return bsp::plane_squadron_registry().spawn_index_of_unit(
            owner_.index_of(marked_unit)) == 0;
    }
    void* aircraft_squadron_9d4(void*) override {
        // The registry fuses squadron_unit with wing 0 and has no independent
        // squadron detection record/name virtual. Neither the marked wing nor
        // that fused slot can bind the native [plane+9D4h] naming subject.
        return nullptr;
    }
    void* unit_subject_140(void* marked_unit) override {
        // Supported non-aircraft units inherit 0047F320 at +140h: return self.
        // Bind that result explicitly; a missing class descriptor remains unbound.
        const auto* row = owner_.units->unit_row(owner_.index_of(marked_unit));
        return row != nullptr && row->class_row_found ? marked_unit : nullptr;
    }

    int display_name_detection_level(int party, void* unit) override {
        const auto* gunnery = owner_.units->gunnery();
        const auto* record = gunnery != nullptr
            ? gunnery->recon_sensor_pass_state().record(party, owner_.index_of(unit)) : nullptr;
        // A newly created unit's unlaid-out detection record is zero, just as
        // the objective reveal binding above handles it. Do not use level(),
        // whose permissive pre-pass fallback would reveal an unobserved name.
        return record != nullptr
            ? static_cast<int>(bsp::recon_record_effective_level(*record))
            : static_cast<int>(bsp::ReconDetectionRecord{}.level);
    }

    const char* unit_name_14(void* unit) override {
        // The host currently retains the default display-name producer:
        // [unit+538h]+54h is VehicleClass[Type].Name, loaded into this row.
        // The optional +15Ch/+160h custom-name owner is not implemented here.
        // Its absence is not the entity's scene/instance name (+10h).
        return owner_.units->unit_row(owner_.index_of(unit))->class_row_name.c_str();
    }

private:
    GameHudMarkersHost::Impl& owner_;
};

// bsp::HudMarkersRuntimeHost, the boundary of src/hud_markers_runtime.cpp.
// Packet cc13-fix24: the Front6Host the 0077A2F0 body needs. It reaches only
// global_address (00F8715C..00F8718C, mapped to the marker host's copies) and
// 00BEE140; every other callback belongs to another front6 body.
class HealthColourFront6Binding final : public bsp::cc13_front6::Front6Host {
public:
    explicit HealthColourFront6Binding(GameHudMarkersHost::Impl& owner) : owner_(owner) {}

    void* global_address(std::uint32_t address) override {
        switch (address) {
        case 0x00F8715Cu: return owner_.health_colour_low_f8715c;
        case 0x00F8716Cu: return owner_.health_colour_mid_f8716c;
        case 0x00F8717Cu: return owner_.health_colour_high_f8717c;
        case 0x00F8718Cu: return &owner_.health_colour_guard_f8718c;
        default: unreached("global_address");
        }
    }
    float* vector4_scale_00bee140(float* out, const float* scalar, const float* vec) override {
        bsp::engine_services_cc13::scale_vector4_00bee140(out, scalar, vec);
        return out;
    }

    void* global_pointer(std::uint32_t) override { unreached("global_pointer"); }
    void invalid_parameter_00bf6713() override { unreached("00BF6713"); }
    std::int32_t stricmp_00bf7fbf(const char*, const char*) override { unreached("00BF7FBF"); }
    void* sized_pool_00419cc0() override { unreached("00419CC0"); }
    void sized_pool_return_00bd1510(void*, void*, std::uint32_t, std::uint32_t) override {
        unreached("00BD1510");
    }
    bsp::cc13_front6::NativeStr* native_string_from_int_004260b0(
        bsp::cc13_front6::NativeStr*, std::int32_t) override {
        unreached("004260B0");
    }
    std::uint8_t* linked_list_nth_node_004be940(void*, std::int32_t) override {
        unreached("004BE940");
    }
    bsp::cc13_front6::MapIterator* map_find_00a03b20(void*, bsp::cc13_front6::MapIterator*,
                                                     const std::uint32_t*) override {
        unreached("00A03B20");
    }
    void archive_enter_04(void*, bsp::cc13_front6::TaggedValue) override { unreached("archive+04h"); }
    void archive_leave_08(void*) override { unreached("archive+08h"); }
    void archive_write_0c(void*, bsp::cc13_front6::TaggedValue,
                          bsp::cc13_front6::TaggedValue) override {
        unreached("archive+0Ch");
    }
    void game_entity_write_archive_00928450(void*, void*) override { unreached("00928450"); }
    void recon_detection_set_write_archive_00805020(void*, void*) override { unreached("00805020"); }
    void observer_member_write_ref_0070e890(void*, void*) override { unreached("0070E890"); }
    void* object_vslot_08(void*) override { unreached("vtable+08h"); }
    void* operator_new_00bf55be(std::uint32_t) override { unreached("00BF55BE"); }
    void free_00bf6989(void*) override { unreached("00BF6989"); }
    void effect_named_record14_copy_construct_0086cdb0(void*, const void*) override {
        unreached("0086CDB0");
    }

private:
    [[noreturn]] static void unreached(const char* site) {
        throw std::logic_error(std::string("HealthColourFront6Binding: ") + site
            + " is not reached by 0077A2F0");
    }

    GameHudMarkersHost::Impl& owner_;
};

class MarkerRuntimeBinding final : public bsp::HudMarkersRuntimeHost {
public:
    explicit MarkerRuntimeBinding(GameHudMarkersHost::Impl& owner) : owner_(owner) {}

    void gui_extent(float& width, float& height) override {
        // 00AA1FE0 over the platform object's +0Dh and +10h
        // (docs/GUI_EXTENT_INPUTS.md). OFF keeps the (1, 1) the unit-square
        // GUI space gives at 4:3, as a record.
        if constexpr (kHudGuiExtentBound) {
            if (const Win32PlatformState* p = active_platform_state()) {
                const GuiAspectExtent e = gui_aspect_extent_00aa1fe0(p->widescreen,
                                                                     p->active_aspect);
                owner_.done("HudMarkers::gui_extent", 0x00aa1fe0u);
                width = e.width;
                height = e.height;
                return;
            }
        }
        owner_.record("HudMarkers::gui_extent", 0x00aa1fe0u);
        width = 1.0f;
        height = 1.0f;
    }
    void* camera_unit() override {
        owner_.record("HudMarkers::camera_unit", 0x004b4b00u);
        return nullptr;
    }
    void* displayed_self_unit(void* controlled_unit) override {
        static_cast<void>(controlled_unit);
        owner_.record("HudMarkers::displayed_self_unit", 0x00927880u);
        return nullptr;
    }
    bool parts_object_flag_is_one(void* unit) override {
        // 006430E4 -> 0080E490 then the byte at +14h. The parts object belongs to
        // packet `unit_parts`; a created instance of this process has none, so
        // the gate answers the value that lets the unit through and says so.
        static_cast<void>(unit);
        owner_.record("HudMarkers::parts_object", 0x0080e490u);
        return false;
    }
    bool is_kind_of(void* unit, int class_id) override {
        owner_.done("HudMarkers::is_kind_of", 0x006fe530u);
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.index_of(unit), class_id);
    }
    bool is_alive_and_visible(void* unit) override {
        owner_.done("HudMarkers::is_alive_and_visible", 0x0043f080u);
        return owner_.units != nullptr
            && owner_.units->unit_alive_and_visible(owner_.index_of(unit));
    }
    bool already_marked(void* unit) override {
        // 0064311C -> 0063ABD0, the linear scan of the frame's vector at +34h.
        owner_.done("HudMarkers::already_marked", 0x0063abd0u);
        const std::size_t index = owner_.index_of(unit);
        for (std::size_t marked : owner_.marked) {
            if (marked == index) return true;
        }
        return false;
    }
    void mark_unit(void* unit) override {
        // 00643131 -> 004323D0, the push_back into the same vector.
        owner_.done("HudMarkers::mark_unit", 0x004323d0u);
        owner_.marked.push_back(owner_.index_of(unit));
    }
    bool objective_container_contains(void* unit) override {
        // 00643145..00643183: ECX = [game+21A4h + 4*[game+18ECh]], the local
        // player's objective set (game+18ECh is 0 in single player, as for the
        // unit pick screen's grey arrow), then 008DDF90(unit). The set's +18h
        // unit tree is GameObjectiveSets::units_in_slot, filled by the Lua
        // objective bindings (src/game_hosts_lua.cpp).
        const std::vector<std::size_t> set = game_objective_sets().units_in_slot(0);
        bsp::ObjectiveSetQuery query;
        query.set_size = set.size();                                         // [this+20h]
        if (owner_.units != nullptr) {
            const std::size_t subject = owner_.index_of(unit);
            query.unit_is_vehicle = owner_.units->unit_is_kind_of(subject, 5);   // 008DDFA9
            query.unit_is_squadron = !query.unit_is_vehicle
                && owner_.units->unit_is_kind_of(subject, 0x18);                 // 008DDFB8
            if (query.unit_is_vehicle) {                                         // 008DDF00
                query.subject_in_set = std::find(set.begin(), set.end(), subject) != set.end();
            } else if (query.unit_is_squadron) {
                // 008DDFC2 looks up the squadron's plane at +3D0h, which this
                // host does not expose (the grey arrow records the same gap).
                owner_.record("HudMarkers::objective_container_squadron", 0x008ddfc2u);
            }
        }
        owner_.done("HudMarkers::objective_container", 0x008ddf90u);
        return bsp::objective_set_contains_008ddf90(query);
    }
    int marker_colour_index(void* unit) override {
        owner_.record("HudMarkers::marker_colour_index", 0x00639990u);
        if (owner_.units == nullptr) return 5;
        const GameUnitRow* row = owner_.units->unit_row(owner_.index_of(unit));
        return marker_colour_index_00639990(row != nullptr ? row->party : -1);
    }
    void refresh_pose(void* unit) override {
        static_cast<void>(unit);
        owner_.done("HudMarkers::refresh_pose", 0x00414db0u);
    }
    void world_matrix_rows(void* unit, float right[3], float up[3], float forward[3],
        float translation[3]) override {
        owner_.done("HudMarkers::world_matrix_rows", 0x00427eb0u);
        for (int lane = 0; lane < 3; ++lane) {
            right[lane] = 0.0f;
            up[lane] = 0.0f;
            forward[lane] = 0.0f;
            translation[lane] = 0.0f;
        }
        if (owner_.units == nullptr) return;
        owner_.units->unit_pose(owner_.index_of(unit), right, up, forward, translation);
    }
    void class_record_extents(void* unit, float& forward, float& right, float& up) override {
        // [unit+538h] +A0h, +A4h and +A8h. Only +A0h and +A8h have a recovered
        // Lua key and both are zero on this installation's ships, so the eight
        // corners collapse onto the anchor and 0063AB75's collapse path runs.
        owner_.record("HudMarkers::class_record_extents", 0x0063a7c3u);
        forward = 0.0f;
        right = 0.0f;
        up = 0.0f;
        if (owner_.units == nullptr) return;
        owner_.units->unit_class_extents(owner_.index_of(unit), forward, right, up);
    }
    void project_clip_space(const float world[3], float out_clip[4]) override {
        // 0043A6A0 -> 00B70490 for the view-projection matrix and 0043A6AF ->
        // 00B62D10 for the transform. Both are the renderer's, and this process
        // has no camera, so **the projection is an executable-side stand-in**: a
        // fixed top-down orthographic camera over the mission's own unit bounds,
        // with +x to the right and +z up the screen.
        bsp::MissionCameraPublication& node = bsp::mission_camera_publication();
        if (kMissionCameraBound && node.ready) {
            // 0043A697..0043A6AF: 00B70490 on [game+19FCh], the Operator node the
            // ShipCaptain mover publishes into, then 00B62D10 with its matrix.
            const bsp::CameraMatrix& view_projection =
                bsp::get_camera_view_projection_00b70490(node.state);
            owner_.done("HudMarkers::view_projection_matrix", 0x00b70490u);
            const float source[4] = {world[0], world[1], world[2], 1.0f};
            bsp::transform_native_vector4_00b62d10(source, out_clip, view_projection.data());
            owner_.done("HudMarkers::transform_vec4", 0x00b62d10u);
            return;
        }
        owner_.record("HudMarkers::view_projection_matrix", 0x00b70490u);
        const float half = owner_.summary.camera_half_extent;
        const float cx = 0.5f * (owner_.summary.bounds_min[0] + owner_.summary.bounds_max[0]);
        const float cz = 0.5f * (owner_.summary.bounds_min[2] + owner_.summary.bounds_max[2]);
        if (!kHudPresentationTopBound) {
            owner_.record("HudMarkers::transform_vec4", 0x00b62d10u);
            out_clip[0] = half > 0.0f ? (world[0] - cx) / half : 0.0f;
            out_clip[1] = half > 0.0f ? (world[2] - cz) / half : 0.0f;
            out_clip[2] = 0.5f;
            out_clip[3] = 1.0f;
            return;
        }
        // SUBSTITUTION: the stand-in camera as the row-vector matrix 00B70490
        // would return (00B62D10 computes v * M, translation in row 3): x and z
        // map onto clip x and y about the bounds centre, depth 0.5, w 1.
        float matrix[16] = {};
        if (half > 0.0f) {
            const float inv = 1.0f / half;
            matrix[0] = inv;          // x -> clip x
            matrix[9] = inv;          // z -> clip y
            matrix[12] = -cx * inv;
            matrix[13] = -cz * inv;
        }
        matrix[14] = 0.5f;
        matrix[15] = 1.0f;
        const float source[4] = {world[0], world[1], world[2], 1.0f};
        // 0043A6AF -> 00B62D10, the recovered transform with its x87 schedule.
        bsp::transform_native_vector4_00b62d10(source, out_clip, matrix);
        owner_.done("HudMarkers::transform_vec4", 0x00b62d10u);
    }
    const char* unit_class_name(void* unit) override {
        // This legacy callback carries no marker eligibility flags. Only the
        // non-aircraft +140h arm can bind here; place_marker passes the flags.
        if (owner_.units == nullptr || owner_.units->unit_is_kind_of(owner_.index_of(unit), 0x0f)) {
            return nullptr;
        }
        return owner_.display_name_for_unit(owner_.index_of(unit), {});
    }
    float unit_health(void* unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::unit_health", 0x0077a2f0u);
        return 0.0f;
    }
    const float* unit_health_colour_0077a2f0(float* out, float health) override {
        if constexpr (kHudUnitHealthColourBound) {
            owner_.done("HudMarkers::unit_health_colour", 0x0077a2f0u);
            HealthColourFront6Binding statics(owner_);
            return bsp::cc13_front6::unit_health_colour_0077a2f0(out, health, statics);
        } else {
            static_cast<void>(health);
            owner_.record("HudMarkers::unit_health_colour", 0x0077a2f0u);
            for (int k = 0; k < 4; ++k) out[k] = 0.0f;
            return out;
        }
    }
    void* find_child(void* widget, const char* name) override {
        owner_.done("HudMarkers::find_child", 0x00aa7e00u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr || name == nullptr) return nullptr;
        return bsp::find_child_by_name_00aa7e00(*typed, name);
    }
    void set_visible(void* widget, bool visible) override {
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*typed, visible);
    }
    void set_local_position(void* widget, const bsp::HudGuiPoint& position) override {
        owner_.done("HudMarkers::set_local_position", 0x00aa7dc0u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_local_position(*typed, position.x, position.y,
            position.z);
    }
    void set_resolved_position(void* widget, const bsp::HudGuiPoint& position) override {
        static_cast<void>(widget);
        static_cast<void>(position);
        owner_.record("HudMarkers::set_resolved_position", 0x00aa8240u);
    }
    void set_local_xy(void* widget, float x, float y) override {
        static_cast<void>(widget);
        static_cast<void>(x);
        static_cast<void>(y);
        owner_.record("HudMarkers::set_local_xy", 0x00aa7d00u);
    }
    void set_localised_text(void* widget, const char* key) override {
        owner_.record("HudMarkers::set_localised_text", 0x00abaed0u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr || key == nullptr) return;
        owner_.menu.frontend().set_widget_text_source(*typed, key);
    }
    void widget_size(void* widget, float& width, float& height) override {
        owner_.record("HudMarkers::widget_size", 0x00aa6740u);
        width = 0.0f;
        height = 0.0f;
        static_cast<void>(widget);
    }
    void release_pool_tail(int pool, std::uint32_t cursor) override {
        static_cast<void>(pool);
        static_cast<void>(cursor);
        owner_.record("HudMarkers::release_pool_tail", 0x00640620u);
    }

private:
    GameHudMarkersHost::Impl& owner_;
};

// bsp::HudMarkersUpdateHost, the boundary of 006435D0 itself.
class MarkersUpdateBinding final : public bsp::HudMarkersUpdateHost {
public:
    MarkersUpdateBinding(GameHudMarkersHost::Impl& owner, MarkerRuntimeBinding& runtime)
        : owner_(owner), runtime_(runtime) {}

    int local_team_index() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        const GameUnitRow* row = owner_.units->unit_row(owner_.units->controlled_index());
        return row != nullptr && row->party >= 0 && row->party <= 7 ? row->party : 0;
    }
    bool game_hud_suppressed() override {
        // cc15-hint2-gate: 00643605 CMP byte [ECX+620h],0 returns early while a
        // critical hint is shown. game+61Fh (the pause menu) is not modelled.
        if constexpr (kHintPauseHudGateCc15Bound) {
            return game_hint_pause_620_cc15();
        } else {
            return false;
        }
    }
    void gui_extent(float& width, float& height) override {
        runtime_.gui_extent(width, height);
    }
    void publish_clip_rect(const bsp::HudMarkerClipRect& rect) override {
        // 00E197C4..00E197D8, one rectangle whose x bounds are stored twice
        // (docs/HUD_MARKERS_RUNTIME.md's correction).
        owner_.clip = rect;
        owner_.done("HudMarkers::publish_clip_rect", 0x0064361bu);
    }
    std::uint32_t camera_unit() override {
        if constexpr (kHudCameraUnitBound) {
            // The update tests the answer against zero only (00643685, and
            // the gates at hud_updates 307/378), so a non-zero token stands.
            bool squadron = false;
            const std::size_t unit = camera_unit_004b4b00(owner_.units, squadron);
            if (squadron) {
                owner_.record("HudMarkers::camera_unit_squadron_leader", 0x004b4b2au);
                return 0u;
            }
            owner_.done("HudMarkers::camera_unit", 0x004b4b00u);
            return unit != 0 ? 1u : 0u;
        } else {
            owner_.record("HudMarkers::camera_unit", 0x004b4b00u);
            // There is no camera; the controlled unit stands in, which is what makes
            // the target, group and reinforcement gates reachable at all.
            return owner_.units != nullptr && owner_.units->controlled_bound() ? 1u : 0u;
        }
    }
    void reset_marker_pool() override {
        if (kHudMarkerPoolsBound) {
            // 006436B9..006436C4: 006435D0 zeroes the pool-D cursor before the
            // opening call, so the opening call tears pool D down completely.
            owner_.pool_cursor_d = 0;
            owner_.reset_marker_pools_00640620();
            return;
        }
        // 00640620, the four pools. This process owns no pool entry, so the
        // release walks nothing and the routine is a record.
        owner_.record("HudMarkers::reset_marker_pool", 0x00640620u);
    }
    void clear_marker_set() override {
        owner_.done("HudMarkers::clear_marker_set", 0x0063bcd0u);
        owner_.marked.clear();
    }
    bool controlled_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    std::uint32_t controlled_unit() override { return 1u; }
    std::uint32_t self_marker_unit() override {
        owner_.record("HudMarkers::self_marker_unit", 0x00927880u);
        return 1u;
    }
    void refresh_screen_state() override {
        owner_.record("HudMarkers::refresh_screen_state", 0x006394b0u);
    }
    void refresh_marker_layout() override {
        owner_.record("HudMarkers::refresh_marker_layout", 0x0063b5e0u);
    }

    void add_marker(std::uint32_t unit, int a, int b, int kind) override {
        if (unit == 0 || owner_.units == nullptr || !owner_.units->controlled_bound()) return;
        const std::size_t index = owner_.units->controlled_index();
        if (index >= owner_.handles.size()) return;
        // 006430C0 through its reconstruction. `a` is the force-objective-colour
        // byte, `b` and `kind` the two kind arguments the caller supplies.
        bsp::HudMarkerFlags flags = bsp::hud_marker_flags_006430c0(
            &owner_.handles[index], &owner_.handles[index], nullptr, 0, 0, false, a != 0);
        bsp::HudMarkerScreenBounds bounds{};
        const bsp::HudMarkerBuilder builder = bsp::hud_markers_add_unit_marker_006430c0(
            &owner_.handles[index], flags, a != 0, b, kind, false, 0, 0, owner_.clip,
            owner_.wide_aspect, bsp::kHudMarkersEnterField88, runtime_, bounds);
        owner_.done("HudMarkers::add_unit_marker", 0x006430c0u);
        if (builder == bsp::HudMarkerBuilder::None) {
            ++owner_.summary.markers_rejected;
            return;
        }
        ++owner_.summary.markers_added;
        if (bounds.collapsed) ++owner_.summary.markers_collapsed;
        owner_.place_marker(index, bounds, flags);
    }
    void note_marker(std::uint32_t unit) override { static_cast<void>(unit); }

    std::uint32_t interface_target_unit() override {
        // [[00E198C4 + CCh] + 4Ch]. The interface manager's target is set by the
        // selector screen, which is one of the 42 records.
        owner_.record("HudMarkers::interface_target_unit", 0x0068aca0u);
        return 0;
    }
    int unit_team_id(std::uint32_t unit) override { static_cast<void>(unit); return 0; }
    int local_team_record_id() override {
        owner_.record("HudMarkers::local_team_record", 0x004c3cb0u);
        return 0;
    }
    bool unit_passes_target_filter(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::target_filter", 0x00804350u);
        return false;
    }
    bool target_is_selectable(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::target_selectable", 0x005220c0u);
        return false;
    }

    const void* squad_first_node() override {
        // [[00E188A8 + 19CCh] + 16Ch], a list on the world object 004DE610 would
        // build. The squad radius at unit+7C4h has no field either, and
        // hud_marker_within_radius rejects every distance against a zero radius,
        // so the squad arm would add nothing even with a list.
        owner_.record("HudMarkers::squad_list", 0x004de610u);
        return nullptr;
    }
    std::uint32_t squad_node_unit(const void* node) override {
        static_cast<void>(node);
        return 0;
    }
    const void* squad_next_node(const void* node) override {
        static_cast<void>(node);
        return nullptr;
    }
    bsp::PoseRefreshView& unit_pose(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::unit_pose", 0x00414db0u);
        return owner_.fallback_pose;
    }
    int squad_marker_radius(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::squad_marker_radius", 0x006437dbu);
        return 0;
    }

    bool pick_world_point(float screen_x, float screen_y, float& wx, float& wy,
        float& wz) override {
        // 0043A290, the crosshair unproject onto the plane y == 0. It needs the
        // same camera 0043A660 needs, and the stand-in is orthographic, so the
        // pick is the inverse of the same projection.
        owner_.record("HudMarkers::pick_crosshair_world_point", 0x0043a290u);
        static_cast<void>(screen_x);
        static_cast<void>(screen_y);
        wx = 0.0f;
        wy = 0.0f;
        wz = 0.0f;
        return false;
    }
    void publish_crosshair_point(bool valid, float wx, float wy, float wz) override {
        static_cast<void>(valid);
        static_cast<void>(wx);
        static_cast<void>(wy);
        static_cast<void>(wz);
        owner_.record("HudMarkers::publish_crosshair_point", 0x006438f7u);
    }
    void viewport_size(int& width, int& height) override {
        owner_.record("HudMarkers::viewport_descriptor", 0x00b6fde0u);
        width = 0;
        height = 0;
    }

    void sweep_objectives() override {
        owner_.record("HudMarkers::sweep_objectives", 0x00643360u);
    }
    void sweep_command_units() override {
        owner_.record("HudMarkers::sweep_command_units", 0x00642c20u);
    }
    void add_group_marker(std::uint32_t unit, int flag) override {
        static_cast<void>(unit);
        static_cast<void>(flag);
        owner_.record("HudMarkers::add_group_marker", 0x006434e0u);
    }
    std::uint32_t query_target_unit() override {
        owner_.record("HudMarkers::query_target_unit", 0x00523020u);
        return 0;
    }
    bool unit_is_group_leader(std::uint32_t unit) override {
        static_cast<void>(unit);
        return false;
    }
    std::size_t group_member_count(std::uint32_t unit) override {
        static_cast<void>(unit);
        return 0;
    }
    std::uint32_t group_member(std::uint32_t unit, std::size_t index) override {
        static_cast<void>(unit);
        static_cast<void>(index);
        return 0;
    }
    void flush_markers() override {
        // 00640D70 then 00640620: the target section callout and the closing
        // pool reset. The callout needs a target and a parts object.
        owner_.record("HudMarkers::target_section_callout", 0x00640d70u);
        if (kHudMarkerPoolsBound) {
            owner_.reset_marker_pools_00640620();   // 00643D41, the closing call
            return;
        }
        owner_.record("HudMarkers::reset_marker_pool", 0x00640620u);
    }
    float objective_detail_level() override {
        owner_.record("HudMarkers::objective_detail_level", 0x00f88a00u);
        return 0.0f;
    }
    void sweep_reinforcements() override {
        owner_.record("HudMarkers::sweep_reinforcements", 0x00643d1au);
    }

private:
    GameHudMarkersHost::Impl& owner_;
    MarkerRuntimeBinding& runtime_;
};

}  // namespace

void GameHudMarkersHost::Impl::bind_page_0063e280() {
    if (bound_attempted) return;
    bound_attempted = true;
    // 0063E280 BSP_HudMarkersScreen_Register loads GUI_markers and stores it at
    // the screen's +8Ch; the screen class is one of the 42 records, so what runs
    // here is the widget bind over the page the manager's Init loaded.
    record("HudMarkers::screen_register", 0x0063e280u);
    page = menu.in_game_page(kMarkersScreenSlot, "GUI_markers");
    if (page == nullptr || !page->root) {
        log.note("markers: GUI_markers is not loaded on registry slot 4Dh, so the marker "
            "pass has no page to place markers in");
        return;
    }
    summary.page_bound = true;
    sidemarker = bsp::find_child_by_name_00aa7e00(*page->root, "sidemarker_Group");
    done("HudMarkers::find_child", 0x00aa7e00u);
    if (sidemarker != nullptr) {
        type_icon = bsp::find_child_by_name_00aa7e00(*sidemarker, "type_Icon");
        summary.template_bound = type_icon != nullptr;
    }
    log.notef("markers page bound: sidemarker_Group %s, its type_Icon %s. The per-marker "
        "widget writer 0063d1e0 has no reconstruction, so the executable clones the "
        "authored sidemarker_Group and places the clone itself",
        sidemarker != nullptr ? "bound" : "missing",
        type_icon != nullptr ? "bound" : "missing");
}

void GameHudMarkersHost::Impl::fit_stand_in_camera() {
    if (summary.camera_fitted || units == nullptr || units->count() == 0) return;
    summary.camera_fitted = true;
    bool first = true;
    for (std::size_t index = 0; index < units->count(); ++index) {
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!units->unit_pose(index, right, up, forward, translation)) continue;
        for (int lane = 0; lane < 3; ++lane) {
            if (first || translation[lane] < summary.bounds_min[lane]) {
                summary.bounds_min[lane] = translation[lane];
            }
            if (first || translation[lane] > summary.bounds_max[lane]) {
                summary.bounds_max[lane] = translation[lane];
            }
        }
        first = false;
    }
    const float half_x = 0.5f * (summary.bounds_max[0] - summary.bounds_min[0]);
    const float half_z = 0.5f * (summary.bounds_max[2] - summary.bounds_min[2]);
    const float half = half_x > half_z ? half_x : half_z;
    // 1.2 is the stand-in's own margin, so a ship that leaves the start box
    // still projects inside the clip rectangle for a while. It is not a
    // recovered value; nothing about this camera is.
    summary.camera_half_extent = half > 0.0f ? half * 1.2f : 1.0f;
    log.notef("markers camera: **an executable-side stand-in**, a fixed top-down "
        "orthographic camera over the mission's own unit bounds x[%.0f, %.0f] z[%.0f, %.0f], "
        "half extent %.0f. 00b70490 and 00b62d10, the view-projection matrix and the "
        "transform 0043a660 needs, are records: this process builds no camera at "
        "game+19FCh, so nothing here is a claim about what the game projects",
        static_cast<double>(summary.bounds_min[0]), static_cast<double>(summary.bounds_max[0]),
        static_cast<double>(summary.bounds_min[2]), static_cast<double>(summary.bounds_max[2]),
        static_cast<double>(summary.camera_half_extent));
}

const char* GameHudMarkersHost::Impl::display_name_for_unit(std::size_t index,
    const bsp::HudMarkerFlags& flags) {
    if (units == nullptr || index >= handles.size()) return nullptr;
    MarkerReconNameBinding binding(*this);
    // 00642287..006422F5 chooses the naming subject before 00803DC0. The
    // resolved object supplies BOTH detection and +14h, never the marked wing.
    const char* name = bsp::cc13_seg49::marker_display_name(binding, local_player_party(),
        &handles[index], flags.is_self, flags.is_friendly, flags.is_target);
    if (name != nullptr) done("HudMarkers::unit_class_name", 0x00803dc0u);
    return name;
}

void GameHudMarkersHost::Impl::set_marker_name(GuiLayoutWidget& marker, std::size_t index,
    const bsp::HudMarkerFlags& flags) {
    auto* text = bsp::find_child_by_name_00aa7e00(marker, bsp::kHudMarkerWidgetUnitName);
    if (text == nullptr) return;
    const char* name = display_name_for_unit(index, flags);
    // An unsupported descriptor must not inherit another unit's label when
    // this widget comes from the shared marker pool.
    menu.frontend().set_widget_visible(*text, name != nullptr);
    if (name == nullptr) return;
    // 006422F0 -> 00803DC0; 0063D751 -> 00ABBE50. The latter is the
    // C-string overload of the LOCALIZED source setter (cc13_pkt642.cpp), so
    // the bridge can consume the returned class key or globals.unknown.
    // Refresh on pool reuse and recon changes, not just when a clone is made.
    const auto* previous = menu.frontend().widget_text_source(*text);
    if (previous == nullptr || *previous != name) {
        menu.frontend().set_widget_text_source(*text, name);
    }
    done("HudMarkers::find_child", 0x00aa7e00u);
}

void GameHudMarkersHost::Impl::place_marker(std::size_t index,
    const bsp::HudMarkerScreenBounds& bounds, const bsp::HudMarkerFlags& flags) {
    if (page == nullptr || sidemarker == nullptr || type_icon == nullptr) return;
    float centre_x = 0.5f * (bounds.min_x + bounds.max_x);
    float centre_y = 0.5f * (bounds.min_y + bounds.max_y);
    // 0063D224 (bsp.py show 0063d1e0 --asm) clamps the marker anchor before
    // writing its widgets. 0064361B..0064367F publishes BOTH horizontal
    // margin pairs from this frame's GUI extent; publish_clip_rect retains
    // that same state here. The old strict-interior test dropped edge markers.
    const bsp::band_0060_cc13::ScreenMargins margins{
        clip.left, clip.right, clip.left, clip.right, clip.top, clip.bottom};
    bsp::band_0060_cc13::clamp_to_screen_margins_00638b90(
        centre_x, centre_y, margins, wide_aspect);
    done("HudMarkers::clamp_to_screen_margins", 0x00638b90u);
    ++summary.markers_on_screen;
    GuiLayoutWidget* marker = nullptr;
    if (kHudMarkerPoolsBound) {
        // Pool A is taken in order through the frame cursor +7Ch; a cursor at
        // the end grows the pool by one entry. SUBSTITUTION: the entry is taken
        // where the host places the marker; 0063D1E0, whose acquisition point
        // and re-show this stands for, is not reconstructed.
        if (pool_cursor_a < pool_a.size()) {
            marker = pool_a[pool_cursor_a];
        } else {
            char key[64];
            std::snprintf(key, sizeof(key), "sidemarker_Group_%zu", pool_a.size());
            marker = menu.frontend().clone_runtime_widget(page->name, *sidemarker, *page->root,
                key);
            if (marker == nullptr) return;
            pool_a.push_back(marker);
            ++summary.markers_created;
        }
        ++pool_cursor_a;
        menu.frontend().set_widget_visible(*marker, true);
        set_marker_name(*marker, index, flags);
        const float offset_x = type_icon->transform.position.x
            - sidemarker->transform.pivot_x * sidemarker->transform.size.width;
        const float offset_y = type_icon->transform.position.y
            - sidemarker->transform.pivot_y * sidemarker->transform.size.height;
        menu.frontend().set_widget_local_position(*marker, centre_x - offset_x,
            centre_y - offset_y, sidemarker->transform.position.z);
        return;
    }
    auto found = markers.find(index);
    if (found != markers.end()) {
        marker = found->second;
    } else {
        char key[64];
        std::snprintf(key, sizeof(key), "sidemarker_Group_%zu", index);
        marker = menu.frontend().clone_runtime_widget(page->name, *sidemarker, *page->root,
            key);
        if (marker == nullptr) return;
        markers.emplace(index, marker);
        ++summary.markers_created;
    }
    set_marker_name(*marker, index, flags);
    // The authored template puts `type_Icon` at its own offset inside the group,
    // so the group is moved by the difference rather than to the point itself.
    const float offset_x = type_icon->transform.position.x
        - sidemarker->transform.pivot_x * sidemarker->transform.size.width;
    const float offset_y = type_icon->transform.position.y
        - sidemarker->transform.pivot_y * sidemarker->transform.size.height;
    menu.frontend().set_widget_local_position(*marker, centre_x - offset_x,
        centre_y - offset_y, sidemarker->transform.position.z);
}

// 00640620, __fastcall(this = screen 4Dh), RET, body 00640620..0064079B.
void GameHudMarkersHost::Impl::reset_marker_pools_00640620() {
    // Pool A: every entry from the cursor to the end is hidden through 006374B0
    // (24 widgets from entry+8h and the one at +68h). The host's entry is one
    // cloned sidemarker_Group, so hiding the group hides what 006374B0 hides.
    for (std::size_t i = pool_cursor_a; i < pool_a.size(); ++i) {
        if (pool_a[i] != nullptr) menu.frontend().set_widget_visible(*pool_a[i], false);
    }
    // Pools B and C: SetVisible(0) on the entries' widgets; both are empty here.
    for (std::size_t i = pool_cursor_b; i < pool_b_size; ++i) {}
    for (std::size_t i = pool_cursor_c; i < pool_c_size; ++i) {}
    // Pool D: destroy the tail through virtual 0 with 1, then 0063FFC0 shrinks
    // the vector to the cursor.
    if (pool_d_size > pool_cursor_d) pool_d_size = pool_cursor_d;
    pool_cursor_a = 0;
    pool_cursor_b = 0;
    pool_cursor_c = 0;
    done("HudMarkers::reset_marker_pool", 0x00640620u);
}

GameHudMarkersHost::GameHudMarkersHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudMarkersHost::~GameHudMarkersHost() = default;

const GameHudMarkersSummary& GameHudMarkersHost::summary() const noexcept {
    return impl_->summary;
}

void GameHudMarkersHost::attach_world(GameUnitsHost& units) {
    Impl& host = *impl_;
    host.units = &units;
    host.handles.clear();
    host.handles.resize(units.count());
    for (std::size_t index = 0; index < host.handles.size(); ++index) {
        host.handles[index].index = index;
    }
    host.fit_stand_in_camera();
}

void GameHudMarkersHost::update_006435d0(float seconds) {
    Impl& host = *impl_;
    host.bind_page_0063e280();
    if (!host.summary.page_bound || host.units == nullptr) {
        host.record("HudMarkers::update", 0x006435d0u);
        return;
    }
    host.wide_aspect = false;
    host.summary.markers_added = 0;
    host.summary.markers_rejected = 0;
    host.summary.markers_on_screen = 0;
    host.summary.markers_collapsed = 0;
    MarkerRuntimeBinding runtime(host);
    MarkersUpdateBinding update(host, runtime);
    const bool ran = bsp::hud_markers_screen_update(host.state, update, seconds);
    host.done("HudMarkers::update", 0x006435d0u);
    if (!ran) return;
    ++host.summary.frames;
    if (!host.logged_selection) {
        host.logged_selection = true;
        host.log.notef("markers selection: 006435d0 marks the controlled unit, the interface "
            "manager's target, the squad members within unit+7C4h, the objectives, the "
            "command units and the target group. This process has only the controlled unit, "
            "so the pass added %zu marker(s) and the other five sources are records "
            "(0068aca0's target, 004de610's squad list, 00643360, 00642c20 and 006434e0). "
            "Marking every ship would be an invented selection, not this routine's",
            host.summary.markers_added);
    }
}

void GameHudMarkersHost::report() {
    Impl& host = *impl_;
    if (!host.summary.page_bound) return;
    host.log.notef("summary mission markers added=%zu rejected=%zu on_screen=%zu "
        "collapsed=%zu widgets=%zu frames=%llu camera=orthographic half=%.0f",
        host.summary.markers_added, host.summary.markers_rejected,
        host.summary.markers_on_screen, host.summary.markers_collapsed,
        host.summary.markers_created, host.summary.frames,
        static_cast<double>(host.summary.camera_half_extent));
}

}  // namespace bsp::game
