#pragma once
#include <cstdint>
#include <vector>

namespace bsp {
// Value projections only, not native layouts or ABI replacements. Addresses and
// the evidence boundary are recorded in docs/HUD_ROOT_UNIT_ROWS.md.
struct HudUnitListView {
    const std::uint32_t* units{};
    std::uint32_t count{};
};
struct HudUnitSelection {
    bool secondary{};             // root+ C2h / result+0
    std::uint16_t index{0xffff};   // root+ C4h / result+2, signed native tests
    // Native byte +1 is padding and is deliberately not represented.
};
struct HudRootSelectionState {
    HudUnitSelection selection;
    bool pending{};               // +C0h; this routine never clears it
    bool primary_previous{};      // +EBh, action 8Dh
    bool primary_next{};          // +EAh, action 8Ch
    bool secondary_previous{};    // +E8h, action 8Eh
    bool secondary_next{};        // +E9h, action 8Fh
};
struct HudRootSelectionHost {
    virtual ~HudRootSelectionHost() = default;
    virtual bool unit_is_group_leader(std::uint32_t unit) = 0; // 00778890, AL
    virtual bool unit_is_group_member(std::uint32_t unit) = 0; // 007788B0, AL
    virtual std::uint32_t group_leader(std::uint32_t unit) = 0; // 007788D0
    virtual bool input_action_pressed(int action) = 0;        // 004C43C0
    virtual bool base_screen_active() = 0;                   // 0068A140
    virtual bool panel_54_active() = 0; // [00E198C4+54h]+5
    virtual std::uint32_t controlled_unit() = 0;              // 00E188D8
    virtual bool unit_virtual_124(std::uint32_t unit) = 0;
    virtual std::uint32_t selected_unit() = 0; // 00644A60 using current selection
};
// Native 00644C20 and 00644CC0: ECX=root, stack=(result*, unit), RET 8,
// EAX=result*. Stable valid lists are required; CRT vector checks are external.
HudUnitSelection hud_root_find_primary_unit(HudUnitListView primary,
    std::uint32_t unit, HudRootSelectionHost& host);
HudUnitSelection hud_root_find_unit(HudUnitListView primary, HudUnitListView secondary,
    std::uint32_t unit, HudRootSelectionHost& host);
// 00644DB0: ECX=root, one unused four-byte stack argument, RET 4.
void hud_root_poll_selection(HudRootSelectionState& state, HudUnitListView primary,
    HudUnitListView secondary, HudRootSelectionHost& host);

struct HudClosedRowsState {
    std::int32_t closed{}; // +1Ch: only exactly 0 and 1 have transitions
    std::int32_t interface_id{}; // +CCh
    std::uint32_t restore_payload{}; // +E4h
};
struct HudClosedRowsHost {
    virtual ~HudClosedRowsHost() = default;
    virtual std::uint32_t controlled_unit() = 0;
    virtual std::uint32_t first_member(std::uint32_t unit) = 0; // unit+3D0h
    virtual bool unit_is_kind(std::uint32_t unit, int kind) = 0; // virtual +5Ch
    virtual bool unit_flag_379(std::uint32_t unit) = 0;
    virtual int unit_owner_1a8(std::uint32_t unit) = 0;
    virtual int local_team() = 0; // [00E188A8]+18ECh
    virtual void push_interface_request(int id, std::uint32_t payload) = 0; // 004CC460
};
// 00647080, ECX=root, RET. Native caller guarantees the required unit/member.
void hud_root_toggle_closed(HudClosedRowsState& state, HudClosedRowsHost& host);

enum class HudRowWidget : std::uint32_t {
    UnitName = 0x34, WeaponInfo = 0x3c, Payload = 0x50,
    FlagJP = 0x64, FlagUS = 0x68, HpDamage = 0x7c,
    HpHealthy = 0x80, Command = 0x84
};
enum class HudWeaponProbe { Bomb, Torpedo, DepthCharge, Rocket };
enum class HudWeaponCount { Payload, Rocket, SubmarineTorpedo };
struct HudWeaponSource {
    const char* key{"ingame.selector_noweapon"};
    bool counted{}; // source is "." + decimal(count) + "x |" + key when true
    std::int32_t count{};
};
struct HudRootRowState {
    std::uint32_t previous_unit{}; // +6Ch, written even when selected unit is null
    float previous_healthy_width{}; // +88h, written only in nonzero metric branch
};
// Four ST0 results are individually spilled to float at 00649625..00649646.
// Their underlying gameplay meanings are unresolved, so use native addresses.
struct HudHealthTerms {
    float metric_939f70{};
    float metric_939f80{};
    float metric_939fc0{};
    float metric_939fb0{};
    float scale_3c8{1.0f}; // tuning+3C8h only when selected unit+A44h == 4
    float scale_3cc{1.0f}; // tuning+3CCh only when selected unit+A44h == 3
    float divisor_36c{};   // selected unit+36Ch
    long double powerup_first{};  // first 00470440(3, selected unit)
    long double powerup_second{}; // second call, kept separate
};
// 006496A6..00649705: retains the two explicit float multiplier spills. MSVC
// long double is 64-bit; native x87 extended precision is not claimed bit-exact.
// The native code does not clamp or guard division by zero/NaN.
float hud_root_healthy_width(long double health, const HudHealthTerms& terms) noexcept;
std::uint32_t hud_root_command_icon_state(bool special, std::uint32_t descriptor) noexcept;

// Integration boundary for 00648C20. No default implementations. Tokens are
// caller-owned unit/descriptor identities; the module never dereferences them.
// Unresolved NativeString formatting/cache, animation, and gameplay calls stay
// here. The host must preserve the actual source string, not a localized label.
struct HudRootRowsHost {
    virtual ~HudRootRowsHost() = default;
    virtual void rebuild_unit_lists() = 0; // 00648290
    virtual std::uint32_t controlled_unit() = 0;
    virtual std::uint32_t selected_unit() = 0; // 00644A60
    virtual bool panel_byte(std::uint32_t manager_offset, std::uint32_t byte_offset) = 0;
    // 00648C66..00648C82: +74h panel, +5 active, +20 nonnull, +9D4 nonnull.
    virtual std::uint32_t panel_74_override_unit() = 0;
    virtual int panel_50_weapon_mode() = 0; // panel+44h
    virtual bool unit_is_kind(std::uint32_t unit, int kind) = 0; // virtual +5Ch
    virtual std::uint32_t first_member(std::uint32_t unit) = 0; // +3D0h
    virtual bool weapon_probe(std::uint32_t unit, HudWeaponProbe probe) = 0;
    // Bomb/Torpedo/DepthCharge: 007C1DB0; Rocket: 007C1DE0; sub: 00815850.
    virtual std::int32_t weapon_count(std::uint32_t unit, HudWeaponCount kind) = 0;
    virtual bool unit_machinegun_byte(std::uint32_t unit) = 0; // +C24h
    // 00449AF0 compares emptiness then __stricmp; preserve case-insensitive comparison.
    virtual bool weapon_source_differs(const HudWeaponSource& source) = 0;
    virtual void set_weapon_source(const HudWeaponSource& source, bool resolve) = 0; // 00ABAED0
    virtual void cache_weapon_source(const HudWeaponSource& source) = 0; // root+70h
    virtual void widget_scalar(HudRowWidget widget, float value) = 0; // virtual +4Ch
    virtual void stop_weapon_animation(int slot) = 0; // 00AA8B80 on +3Ch
    virtual std::uint32_t weapon_animation(int slot) = 0; // 00AA8B00 on +3Ch
    virtual void animation_interval(std::uint32_t animation, float start, float end) = 0; // 00AC2F20
    virtual void animation_field_4(std::uint32_t animation, float value) = 0;
    virtual bool special_command_state(std::uint32_t unit, int argument) = 0; // 00927F30
    // Unit virtual +114h; if nonnull, 0071BE40. Return 0 if either is null.
    virtual std::uint32_t command_descriptor(std::uint32_t unit) = 0;
    virtual void widget_state(HudRowWidget widget, std::uint32_t state,
        int zero, float one) = 0; // virtual +88h
    virtual const char* unit_name(std::uint32_t unit) = 0; // virtual +14h
    virtual void unit_name_source(const char* source, float minus_one, bool resolve) = 0; // 00ABB000
    virtual int controlled_unit_country(std::uint32_t unit) = 0; // +54h
    virtual void widget_shown(HudRowWidget widget, bool shown) = 0; // virtual +34h
    virtual std::uint32_t unit_class(std::uint32_t unit) = 0; // +538h
    virtual std::uint32_t class_payload_icon(std::uint32_t unit_class) = 0; // 00653200
    virtual long double kind_45_health(std::uint32_t unit) = 0; // 006D2560
    virtual long double unit_health(std::uint32_t unit) = 0; // 00923BE0
    // virtual +48h receives the two-float pair (width, 1.0f).
    virtual void widget_width(HudRowWidget widget, float width, float one) = 0;
    virtual float health_metric(std::uint32_t selected_unit, std::uint32_t address) = 0;
    virtual int unit_field_a44(std::uint32_t unit) = 0;
    virtual float tuning_float(std::uint32_t offset) = 0; // 00424C40 then field
    virtual long double powerup_multiplier(int kind, std::uint32_t unit) = 0; // 00470440
    virtual float unit_float_36c(std::uint32_t unit) = 0;
};
// Native 00648C20: ECX=root, RET. Covers branch policy and host call order;
// native string allocator/SEH machinery and host callees are not reconstructed.
// No Medal_Icon (+5Ch) or Medal_Text (+60h) writes exist in this function.
void hud_root_update_rows(HudRootRowState& state, HudRootRowsHost& host);

// ---- packet cc9_force_select_unit (docs/CONTROLLED_UNIT.md, "ForceSelectUnit") ----
// The HUD root's two unit vectors and cursor. Units are nonzero handles; 0 is
// null. Descriptive names are hypotheses.
struct HudRootUnitLists {
    std::vector<std::uint32_t> primary_8c;    // +8Ch..+94h
    std::vector<std::uint32_t> secondary_9c;  // +9Ch..+A4h
    HudUnitSelection cursor;                  // +C2h byte, +C4h signed word
    bool force_pending_c0{false};             // +C0h, set only by 00644DB0 (input)
};
struct HudRootUnitListsHost {
    virtual ~HudRootUnitListsHost() = default;
    virtual std::uint32_t controlled_unit() = 0;                  // [00E188D8]
    // [game+1974h], the head of game+1970h (004C3CB0's walk 0, not ordnance),
    // in list order.
    virtual const std::vector<std::uint32_t>& local_unit_list_1970() = 0;
    // 00645060(ECX = unit, EDX = [game+18ECh], 1), AL.
    virtual bool unit_selectable_00645060(std::uint32_t unit) = 0;
    virtual bool unit_is_group_leader_00778890(std::uint32_t unit) = 0;
    virtual bool unit_is_group_member_007788b0(std::uint32_t unit) = 0;
    virtual std::uint32_t group_leader_007788d0(std::uint32_t unit) = 0; // 0 without a group
    // [unit+188h] == 9 or == [game+18ECh] (00648418..0064842F, 00648464..00648473).
    virtual bool role_word_188_open(std::uint32_t unit) = 0;
    virtual int group_member_count_4f8(std::uint32_t leader) = 0;     // [[leader+284h]+4F8h]
    virtual std::uint32_t group_member_at_0070d060(std::uint32_t leader, int slot) = 0;
    // 006485A0's tail.
    virtual void set_controlled_unit_00645600(std::uint32_t unit) = 0; // 0 releases
    virtual bool multiplayer_1fe4() = 0;                              // game+1FE4h
    // The interface manager [00E198C4]: +4h (applied id), +20h (pending id),
    // and whether +1Ch equals +38h.
    virtual int manager_applied_04() = 0;
    virtual int manager_pending_20() = 0;
    virtual bool manager_1c_equals_38() = 0;
    virtual void push_interface_request_004cc460(int interface_id) = 0; // 34h or 2Ah, payload 0
    virtual void push_controlled_unit_interface_00647040() = 0;
};
// 00644A60, __fastcall(root): the list entry at the cursor, or 0.
std::uint32_t hud_root_cursor_unit_00644a60(const HudRootUnitLists& lists) noexcept;
// 00645710, __fastcall(root), RET: settles the cursor on the rebuilt lists.
void hud_root_settle_cursor_00645710(HudRootUnitLists& lists, HudRootUnitListsHost& host);
// 00648290, __fastcall(root), RET: rebuilds both vectors, re-finds the cursor
// on the controlled unit when its list kind changed, settles it, then
// tail-jumps to 006485A0 when +C0h is set.
void hud_root_rebuild_unit_lists_00648290(HudRootUnitLists& lists, HudRootUnitListsHost& host);
// 006485A0, __fastcall(root), RET: clears +C0h, rebuilds, and controls the
// unit at the cursor (or releases control and pushes 34h/2Ah on an empty list).
void hud_root_force_select_unit_006485a0(HudRootUnitLists& lists, HudRootUnitListsHost& host);
}
