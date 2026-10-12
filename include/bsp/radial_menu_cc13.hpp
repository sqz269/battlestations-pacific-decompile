#pragma once

// Support-manager radial menus (cc13-radial1): the two scrolling rings of large
// icons, the pruning of large slots whose entity has gone, and the small
// loadout ring with its highlight. Names are hypotheses, not recovered symbols.
// Evidence, ABIs, the per-builder availability rules and the orders verdict are
// in docs/CC13_RADIAL_MENUS.md.
//
// None of these routines issues an order or command: every call outside the
// menu objects is a read, the "unavailable" sound (0065F170) or the device-class
// resolve 00443490 that the loadout icon lookup goes through. The selection
// cursor writes in 006605C0 (+5Ch, +2C4h, +324h) are screen state only.
//
// The callers (00675C40 and 0066D020, cc9's support-manager code) are not
// reconstructed here. These are C++ interfaces over a model of the native
// screen, not drop-in binary replacements.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast and unspilled intermediates are doubles.

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace bsp::radial_cc13 {

using Widget = std::uintptr_t;  // native GuiWidget*; 0 = null
using Record = std::uintptr_t;  // catalog class record (+3A4h or +2A0h)
using Unit = std::uintptr_t;    // unit object (IsKindOf at vtable slot +5Ch)

struct Vec3 {
    float x{};
    float y{};
    float z{};
};

// The support-manager screen fields these routines read or write, named by
// their native offsets. Each large slot k has a group widget (+ACh + 4k), its
// `icon_L_Icon` child (+BCh + 4k) and its `circle_HL_Icon` child (+DCh + 4k);
// the arrays are laid out back to back, so +B8h/+C8h/+E8h are index 3.
struct RadialScreen {
    std::int32_t page_5c{};                    // +5Ch
    std::array<Widget, 4> catalog_group_7c{};  // +7Ch, rotated by 00668D40
    std::array<Widget, 4> catalog_icon_8c{};   // +8Ch, counter-rotated
    std::array<Widget, 4> slot_group_ac{};     // +ACh
    std::array<Widget, 4> slot_icon_bc{};      // +BCh (icon_L_Icon)
    std::array<Widget, 4> slot_highlight_dc{}; // +DCh (circle_HL_Icon)
    Widget widget_f8{};                        // +F8h, shown when the slot ring stops
    Widget widget_fc{};                        // +FCh, hidden by 006605C0, shown by 00661B50
    Widget widget_100{};                       // +100h, shown when the catalog ring stops
    Widget widget_104{};                       // +104h
    std::array<Widget, 4> loadout_first_108{}; // +108h, per quadrant: first-entry template
    std::array<Widget, 4> loadout_rest_118{};  // +118h, per quadrant: template of the rest
    Widget slot_template_138{};                // +138h, cloned into slot 3 by 006605C0
    Widget catalog_leaving_group_13c{};        // +13Ch, faded out and destroyed by 00668D40
    Widget catalog_leaving_icon_140{};         // +140h
    Widget slot_leaving_group_148{};           // +148h, faded out and destroyed by 00661B50
    Widget slot_leaving_icon_14c{};            // +14Ch
    bool scroll_reverse_154{};                 // +154h: picks the direction of both scrolls
    std::vector<Widget> loadout_icons_15c;     // std::list +15Ch (head +160h, size +164h)
    std::uint32_t list_a_count_1f4{};          // +1F4h (list +1ECh)
    float line_step_x_290{};                   // +290h, quadrants 0 and 3
    float line_step_y_294{};                   // +294h
    Record hangar_record_2a0{};                // +2A0h (0065AE90)
    float angle_base_2a4{};                    // +2A4h
    float angle_step_2a8{};                    // +2A8h
    float catalog_scroll_2b8{};                // +2B8h, 1 -> 0
    float slot_scroll_2bc{};                   // +2BCh, 1 -> 0
    bool catalog_scrolling_2c0{};              // +2C0h, cleared when +2B8h reaches 0
    bool slot_scrolling_2c1{};                 // +2C1h, cleared when +2BCh reaches 0
    std::int32_t selection_2c4{};              // +2C4h, index into list +3E0h
    bool prune_pending_2d4{};                  // +2D4h
    std::int32_t removed_slot_2d8{};           // +2D8h
    bool removing_2dc{};                       // +2DCh: one removal at a time
    bool removing_2dd{};                       // +2DDh
    float removal_fade_2e0{};                  // +2E0h, set to 1.0f (00D7A24C)
    std::int32_t quadrant_314{};               // +314h, 0..3 picks the templates and layout
    std::int32_t slot_324{};                   // +324h
    std::int32_t loadout_highlight_334{};      // +334h, the highlighted loadout index
    std::uint32_t list_b_count_34c{};          // +34Ch (list +344h)
    Record catalog_record_3a4{};               // +3A4h (0065AB40)
    std::uint32_t list_c_count_3e8{};          // +3E8h (list +3E0h)
    std::array<std::int32_t, 4> slot_ids_438{};  // +438h, entity ids (+174h) of the large slots
};

struct RadialHost {
    virtual ~RadialHost() = default;

    // GUI widget virtuals and helpers (menu objects only).
    virtual void set_visible(Widget widget, bool visible) = 0;   // vslot +34h
    virtual void set_rotation(Widget widget, float angle) = 0;   // vslot +44h
    virtual void set_alpha(Widget widget, float alpha) = 0;      // vslot +4Ch
    virtual void set_icon_state(Widget widget, std::int32_t state, std::int32_t arg,
                                float scale) = 0;                // vslot +88h
    virtual void destroy(Widget widget) = 0;                     // vslot +4h(1)
    virtual Widget clone_subtree(Widget source) = 0;             // 00AAB4C0(source, parent 0)
    virtual Widget find_child(Widget widget, std::string_view name) = 0;  // 00AA7E00(name, 1)
    virtual Vec3 resolved_position(Widget widget) = 0;           // 00AA6750
    virtual void set_resolved_position(Widget widget, const Vec3& position) = 0;  // 00AA8240
    virtual void icon_add_state(Widget icon, void* texture,
                                const std::array<float, 4>& uv) = 0;  // 00AB64C0

    // State reads that decide what the menus show.
    virtual bool record_has_flag(Record record, std::int32_t flag) = 0;  // record vslot +18h
    virtual std::int32_t record_loadout_count(Record record) = 0;        // 00951F10: record+128h
    // 009552E0(loadout) on the record -> +4 -> +8 -> +4 resolved by 00443490
    // (DL = 0) -> +74h -> +34h; the native resolves twice, once per field.
    virtual void* loadout_small_texture(Record record, std::int32_t loadout) = 0;  // +78h
    virtual std::array<float, 4> loadout_small_uv(Record record, std::int32_t loadout) = 0;  // +80h..+8Ch
    // List +3ACh (head +3B0h): does any node's object (+08h) carry u16 id +174h == id?
    virtual bool entity_list_has_id(std::int32_t id) = 0;
    // List +3E0h (head +3E4h): byte +14h of the node at index (live unit, not a record).
    virtual bool list_c_entry_is_live_unit(std::int32_t index) = 0;
    virtual Unit selected_unit_a() = 0;                          // *(+304h)+8
    virtual bool unit_is_kind(Unit unit, std::int32_t kind) = 0; // vslot +5Ch

    // The one effect outside the menu objects: the "unavailable" sample at
    // screen+10h (environment/environment_unavailable.fsb), audio only.
    virtual void play_unavailable_sound_0065f170() = 0;
};

// 0065E870: __stdcall(widget, float alpha), RET 8. Alpha on the children
// icon_L_Icon, circle_HL_Icon and circle_L_Icon.
void set_large_icon_alpha_0065e870(RadialHost& host, Widget widget, float alpha);

// 0065ABB0: __thiscall(screen), RET. Destroys every loadout icon, then clears list +15Ch.
void clear_loadout_icons_0065abb0(RadialScreen& screen, RadialHost& host);

// 006605C0: __thiscall(screen), RET. Drops large slots whose entity id left list +3ACh.
void prune_slots_006605c0(RadialScreen& screen, RadialHost& host);

// 00661B50: __thiscall(screen, float dt), RET 4. Steps the slot ring scroll.
void step_slot_ring_00661b50(RadialScreen& screen, RadialHost& host, float dt);

// 00662020 / 00662210: __thiscall(screen), RET. Highlight the loadout icon at +334h.
void highlight_loadout_00662020(RadialScreen& screen, RadialHost& host);
void highlight_hangar_loadout_00662210(RadialScreen& screen, RadialHost& host);

// 00665F30 / 00666DC0: __thiscall(screen), RET. Build the loadout ring for the
// catalog record +3A4h / the hangar record +2A0h.
void build_loadout_ring_00665f30(RadialScreen& screen, RadialHost& host);
void build_hangar_loadout_ring_00666dc0(RadialScreen& screen, RadialHost& host);

// 00668D40: __thiscall(screen, float dt), RET 4. Steps the catalog ring scroll
// and rebuilds the loadout ring when it stops on page 2.
void step_catalog_ring_00668d40(RadialScreen& screen, RadialHost& host, float dt);

}  // namespace bsp::radial_cc13
