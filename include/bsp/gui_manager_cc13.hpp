#pragma once
// GUI manager and widget-base helpers from partition segments 76 (00AA0F70-00AA6720)
// and 77 (00AA6750-00AACE40). Packet cc13-gui6, docs/CC13_GUI_MANAGER.md.
//
// Every routine here is a reconstruction from the decompiler output and the
// listing; names are hypotheses, not recovered symbols. The engine objects stay
// behind host interfaces in the style of include/bsp/blocking_screen.hpp; the
// value types reuse GuiWidgetTransform (include/bsp/gui_widget.hpp) where the
// field set matches. Status: reconstructed and build-tested, not game-validated.

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/gui_widget.hpp"

namespace bsp::gui_manager_cc13 {

// ---------------------------------------------------------------------------
// 00AA2110, the safe-zone frame toggle
// ---------------------------------------------------------------------------

// Manager +7Ch is safezone_43_FrameBox and +80h safezone_169_FrameBox, both
// children of the _Highlight group created by 00AA5E20
// (docs/APP_INIT_FONTS_GUI.md entries 9 and 10). Byte +84h records whether one
// of them is shown; 00AA5E20 clears it last.
struct GuiSafeZoneState {
    bool shown{false}; // manager +84h
};

enum class GuiSafeZoneFrame { FourByThree, SixteenByNine };

struct GuiSafeZoneHost {
    virtual ~GuiSafeZoneHost() = default;
    // Platform object [0109CF04] byte +0Dh, the wide-screen flag
    // (src/game_hosts_hud.cpp substitutes it the same way).
    virtual bool platform_widescreen() = 0;
    // The frame widget's virtual +34h, the visibility setter, one byte argument.
    virtual void set_frame_visible(GuiSafeZoneFrame frame, bool visible) = 0;
};

// 00AA2110. __thiscall(manager, byte show), RET 4. The one caller,
// BSP_Game_OnInitOnce (004DD5B0), passes the byte at 00F88A31.
// Showing picks one frame by the wide-screen flag; hiding always hides both
// (169 first, then 43). Repeated calls with the same value do nothing, and the
// flag is not re-read while a frame is shown, so a later aspect change keeps
// the frame chosen at show time.
void set_safe_zone_visible_00aa2110(GuiSafeZoneState& state, GuiSafeZoneHost& host, bool show);

// ---------------------------------------------------------------------------
// 00AA3F70, update a caller-supplied page list
// ---------------------------------------------------------------------------

using GuiPageHandle = void*;

struct GuiPageListUpdateHost {
    virtual ~GuiPageListUpdateHost() = default;
    // Manager +6Ch. Holds the first page of the list while the call runs and is
    // zeroed at the end; the hit test 00AA2F10 skips every page but this one
    // while it is non-null.
    virtual void set_pointer_scope_page(GuiPageHandle page) = 0;
    // 00AA3910 BSP_GuiManager_UpdatePointer, ECX = the manager.
    virtual void run_pointer_pass() = 0;
    // Page virtual +38h, no arguments, returns a byte.
    virtual bool page_visible(GuiPageHandle page) = 0;
    // Page virtual +40h, one float.
    virtual void page_update(GuiPageHandle page, float seconds) = 0;
};

// 00AA3F70. __thiscall(manager, std::vector<page*> by value (16 bytes),
// float seconds), RET 14h; the callee frees the vector's storage.
// Unlike 00AA4F80 there is no live-entry test (00AA7EF0) and no highlight
// reset: only visible pages update. The list must not be empty: the image
// dereferences its first element unconditionally (the checked iterator calls
// 00BF6713, the invalid-parameter handler, on an empty range).
// Callers: BSP_Frontend_UpdateActiveScreens (004D8620) and the please-wait
// frame pair 0060D570 / 0060D650 (docs/CC13_HUD_AND_WIDGETS.md item 25).
void update_page_list_00aa3f70(GuiPageListUpdateHost& host, const std::vector<GuiPageHandle>& pages,
    float seconds);

// ---------------------------------------------------------------------------
// 00AA4EF0, wide-screen relayout of every page
// ---------------------------------------------------------------------------

struct GuiWideScreenRelayoutHost {
    virtual ~GuiWideScreenRelayoutHost() = default;
    // 004D35D0 copies the manager page vector (+14h proxy, +18h first,
    // +1Ch last) into a temporary freed after the walk.
    virtual std::vector<GuiPageHandle> snapshot_pages() = 0;
    // 00AA8710 BSP_GuiWidget_ApplyWideScreenLayout, ECX = the page.
    virtual void apply_widescreen_layout(GuiPageHandle page) = 0;
};

// 00AA4EF0. __fastcall(ECX = manager), RET. Its one caller is
// BSP_Settings_ApplyAll (008D5B50), when 00E198C4 is null
// (docs/OPTIONS_SETTINGS_COMMIT.md step 22).
void relayout_all_pages_00aa4ef0(GuiWideScreenRelayoutHost& host);

// ---------------------------------------------------------------------------
// Widget geometry helpers on GuiWidgetTransform
// ---------------------------------------------------------------------------

// 00AA6B10. __thiscall(widget, const float point[2]) -> bool in AL, RET 4.
// The rectangle is the widget's resolved position (00AA6750) minus its own
// pivot times size, extended by the size; all four bounds are inclusive and
// every comparison is FCOMI + JB, so a NaN coordinate fails. The right and
// bottom edges are rounded to float (FSTP to a stack slot) before comparing.
// No static reference to 00AA6B10 exists in the image (no rel32 call, no
// absolute dword), so this is a dead helper; the live pointer test is
// 00AA6A40 BSP_GuiWidget_ContainsPointer, which tests the global cursor.
bool contains_point_00aa6b10(const GuiWidgetTransform& widget, float x, float y) noexcept;

// 00AA69E0. __thiscall(widget, float width, float height), RET 8. Divides the
// requested extent by the widget size (+20h, +24h) and passes the pair to
// virtual +48h, the Scale setter 00AA7950. A zero size divides by zero in x87
// (inf or NaN, no trap under the game's control word). No static reference.
std::array<float, 2> scale_for_extent_00aa69e0(const GuiWidgetTransform& widget, float width,
    float height) noexcept;

// Widget +38h..+44h: four floats in the order left, right, top, bottom
// (00AA8F30 builds them that way). Zeroed by the base constructor 00AA9390.
// A hit-test clip: the reader is the hit walk 00AA8BD0, which rejects the
// widget when its hit box leaves the rectangle on any edge whose limit is
// non-zero (00AA8CDF left: limit > box left; 00AA8D41 right: box right >
// limit; 00AA8D69 top; 00AA8D89 bottom; a limit of exactly 0.0 is off). The
// row builders that call 00AA8F30 copy a list box's extent onto each row, so
// rows scrolled outside the box cannot be hit. A sweep of 00A90000-00AD8000
// for functions touching all four offsets on one register found the writer
// 00AA8EC0 (positive control), the constructors 00AA9390/00AA9520, the hit
// walk, and seven others whose register is not a widget where checked (for
// example 00AC2BC3 in BSP_GuiLine_Flush reads vertex element offsets); no
// drawing use was found. include/bsp/gui_layer.hpp GuiHitLimits labels the same
// four fields from its box naming (gate_right, min_left, ...); the edge
// order here follows the writer 00AA8F30.
struct GuiWidgetRect {
    float left{0.0f};
    float right{0.0f};
    float top{0.0f};
    float bottom{0.0f};
};

// The fields 00AA8EC0 touches: the rectangle and the child list (+64h..+6Ch).
struct GuiRectNode {
    GuiWidgetRect rect{};
    std::vector<GuiRectNode*> children{};
};

// 00AA8EC0. __thiscall(widget, const GuiWidgetRect*, byte recursive), RET 8.
// Copies the rectangle, then with the flag set walks the children in list
// order, each with the same rectangle and the flag.
void set_rect_00aa8ec0(GuiRectNode& widget, const GuiWidgetRect& rect, bool recursive);

// 00AA8F30. __thiscall(widget, const widget* source, byte recursive), RET 8.
// The rectangle is the source's resolved position (00AA6750, called four
// times) extended by the source size; unlike 00AA6B10 the pivot is not
// subtracted. Callers: 0055D380 BSP_ControlBindingScreen_BuildSett...,
// 00561BE0 and 00561EB0 BSP_LeaderboardsScreen_FillRows.
GuiWidgetRect rect_from_widget_00aa8f30(const GuiWidgetTransform& source) noexcept;

// ---------------------------------------------------------------------------
// Widget names
// ---------------------------------------------------------------------------

// A widget's name is the 8-byte native string {length, chars} its virtual +28h
// returns. The tree below holds just what the name routines read.
struct GuiNamedWidget {
    std::string name{};                       // virtual +28h
    std::string node_name{};                  // scene node (+4Ch) name, 00B6D800
    GuiNamedWidget* parent{nullptr};          // +70h
    std::vector<GuiNamedWidget*> children{};  // +64h..+6Ch
};

// 00AA2420 (and its byte-identical twin 00AA7810), the sort predicate of
// std::sort in 00AA5A00 and 00AAAED0. Case-insensitive name order through
// _stricmp, with the empty name first: an empty left is less than any
// non-empty right, a non-empty left is never less than an empty right.
bool name_less_00aa2420(const std::string& left, const std::string& right) noexcept;

// 00AA9AB0. __thiscall(widget, NativeString* out) -> out, RET 4. With no
// parent the name itself; otherwise parent's qualified name + "." + name
// (00CE3A70 is "."), recursing up the parent chain. Called only by itself.
std::string qualified_name_00aa9ab0(const GuiNamedWidget& widget);

struct GuiPathVisitHost {
    virtual ~GuiPathVisitHost() = default;
    // Child virtual +6Ch with the child's dotted path. The base implementation
    // is 00AAA0F0 itself; overrides call it (00AB4CE0, 00AD1A80).
    virtual void visit_child(GuiNamedWidget& child, const std::string& path) = 0;
};

// 00AAA0F0, the widget-base virtual +6Ch (present in at least twelve tables,
// for example 00D5C130+6Ch and 00D5BED0+6Ch). __thiscall(widget, NativeString
// prefix by reference), RET 4. For each child in list order: virtual +6Ch on
// the child with prefix + "." + child name.
void visit_child_paths_00aaa0f0(GuiNamedWidget& widget, const std::string& prefix,
    GuiPathVisitHost& host);

// 00AAA2B0. __thiscall(widget, const NativeString* suffix) -> child or null,
// RET 4. Direct children only, in list order: the first child whose scene node
// name ends with `suffix`, compared case-insensitively over the last
// suffix.length characters (the tail is re-measured with strlen, so an
// embedded NUL ends it). 19 call sites, among them BSP_BriefingScreen_Enter and
// the scroll bar's 00AC8890 / 00AC8A80.
GuiNamedWidget* find_child_by_name_suffix_00aaa2b0(const GuiNamedWidget& widget,
    const std::string& suffix);

// ---------------------------------------------------------------------------
// 00AA9C90, hoisting flagged children into the manager
// ---------------------------------------------------------------------------

struct GuiHoistNode {
    std::string name{};                    // virtual +28h
    std::int32_t type_id{0};               // +60h; 1 is the page root (Screen)
    bool hoist{false};                     // byte +76h
    std::vector<GuiHoistNode*> children{}; // +64h..+6Ch
};

struct GuiHoistHost {
    virtual ~GuiHoistHost() = default;
    // 00AA59B0 on BSP_GuiManager_GetOrCreate(): manager +44h widget
    // AttachChild(child, 0) (00AAA5A0, which unlinks it from this parent), then
    // push_back {path, child} on the manager list at +34h (head +38h).
    virtual void adopt(const std::string& path, GuiHoistNode& child) = 0;
};

// 00AA9C90. __thiscall(widget, NativeString path by value), RET 8.
// The iterator is advanced before a flagged child is adopted, so removing it
// from this list is safe. A flagged child is recorded with the path passed to
// this widget (its own and this widget's names are not appended) and is not
// descended into. Every other child is recursed into with the same path when
// this widget is a page root (type 1), else with path + "." + this widget's
// name (00AA9DF2..00AA9E70). Nothing outside its own recursion calls it, so the
// manager list at +34h stays empty in this build.
void hoist_flagged_children_00aa9c90(GuiHoistNode& widget, const std::string& path,
    GuiHoistHost& host);

// ---------------------------------------------------------------------------
// 00AA5390, show only some widgets
// ---------------------------------------------------------------------------

struct GuiVisibilityNode {
    float visibility{1.0f};                     // scene node (+4Ch) +ACh
    GuiVisibilityNode* parent{nullptr};         // +70h
    std::vector<GuiVisibilityNode*> children{}; // +64h..+6Ch
};

struct GuiVisibilityHost {
    virtual ~GuiVisibilityHost() = default;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor(factor, 0) on node +4Ch.
    virtual void set_visibility(GuiVisibilityNode& widget, float factor) = 0;
};

// 00AA5390. __thiscall(manager, const std::vector<widget*>*), RET 4; unreferenced. Saves each listed
// widget's node factor as an int (CVTTSS2SI at 00AA5446), sets factor 0 on every
// widget reachable from the manager's pages (+18h/+1Ch) in breadth-first order,
// then for each listed widget sets the saved value (CVTSI2SS at 00AA5680) on it
// and every ancestor. The int round trip truncates: a saved 0.5 comes back 0.
void show_only_widgets_00aa5390(const std::vector<GuiVisibilityNode*>& pages,
    const std::vector<GuiVisibilityNode*>& shown, GuiVisibilityHost& host);

// ---------------------------------------------------------------------------
// The Button widget (type 15, vtable 00D5BED0, 0FCh bytes)
// ---------------------------------------------------------------------------

// Fields after the 0ECh-byte widget base. The listener at +ECh receives
// virtual +4 with the button on a click; +F0h is the child named by the Label
// property, which carries the button's visual state.
struct GuiButton {
    void* listener{nullptr};       // +ECh
    void* label_widget{nullptr};   // +F0h, found by name under the button
    std::string label_name{};      // +F4h/+F8h, string property "Label" (00D5C23C)
    bool visible{true};            // base virtual +38h
    bool disabled{false};          // base byte +77h
    bool label_contains_pointer{false}; // 00AA6A40 on the label widget
};

// The value handed to the label widget's virtual +80h. 0 = normal, 2 =
// focused, 3 = disabled. A disabled button reports 3 whether focused or not.
int button_label_state(const GuiButton& button, bool focused) noexcept;

struct GuiButtonFocusHost {
    virtual ~GuiButtonFocusHost() = default;
    // Label widget virtual +80h with the state of button_label_state.
    virtual void set_label_state(GuiButton& button, int state) = 0;
    // [00F8BCFC], __fastcall(byte cl, byte dl). Installed by 00AAB5E0, whose
    // only caller BSP_Game_OnInitOnce passes 00941250
    // BSP_GuiSound_RequestSelectionFlags. 00AAB980 tail-jumps to it with
    // cl = 1, dl = 0 (00AAB9E1..00AAB9E6) after a focus change; the hover pass
    // calls it with cl = 0, dl = 1 after a click (00AAC4EE..00AAC4F2).
    virtual void selection_sound(bool cl, bool dl) = 0;
    // [00F8BC84], __thiscall(bool* out). Installed by 00AA6BE0 with 00696430
    // BSP_GuiInput_ReadPointerActivation.
    virtual bool read_pointer_activation() = 0;
    // Listener virtual +4 with the button.
    virtual void notify_listener(GuiButton& button) = 0;
};

struct GuiButtonLabelHost {
    virtual ~GuiButtonLabelHost() = default;
    // 00AA7E00 BSP_GuiWidget_FindChildByName(&name, 1) with ECX = the button.
    virtual void* find_child_by_name(GuiButton& button, const std::string& name) = 0;
    // Label widget virtual +80h (a JMP, so the caller's RET 4 is the label's).
    virtual void set_label_state(void* label_widget, int state) = 0;
};

// 00AABD70, Button virtual +18h (00D5BEE8), the property reader.
// __thiscall(button, reader*), RET 4. After the base reader 00AAA710 it reads
// the string "Label" through reader virtual +0Ch with default 00CE3A0C (the
// empty string). A non-empty value re-resolves +F0h by name; an empty one
// leaves +F0h as it was. A non-null +F0h then gets state 3 when the button is
// disabled, else 0 (not the focus state: a focused button re-read loses its
// highlight until the next focus change).
void read_button_label_00aabd70(GuiButton& button, const std::string& label_value,
    GuiButtonLabelHost& host);

// 00AAC270, Button virtual +1Ch (00D5BEEC), the property describer. After the
// base describer 00AAAED0 it writes "Label" through writer virtual +0Ch only
// when 00BD5680 reports the value differs from the empty default; a null
// string pointer is described as 00F8BD44, a static empty string.
bool button_label_is_described_00aac270(const GuiButton& button) noexcept;

// [00F8BCF8], the focused button. Any Button destructor (00AAC530) clears it,
// not only the focused one's.
struct GuiButtonRegistry {
    std::vector<GuiButton*> buttons{}; // std::list at 00F8BD38, sentinel 00F8BD3C
    GuiButton* focused{nullptr};       // 00F8BCF8
};

// 00AAB980. __fastcall(ECX = new focus or null). Demotes the old focus's label
// to state 0/3, stores the new focus, promotes its label to 2/3, then
// tail-jumps through [00F8BCFC] with (1, 0) even when the new focus is null.
// Called only from the hover pass below.
void set_focused_button_00aab980(GuiButtonRegistry& registry, GuiButtonFocusHost& host,
    GuiButton* button);

// 00AAC410 (no Ghidra function; body 00AAC410-00AAC4F9, plain RET). Picks the
// last registered button that is visible, enabled, has a label, and whose
// label contains the pointer; refocuses when that differs from the current
// focus (including to null); then, when the activation reader reports a press
// and a button is focused, notifies the focused button's listener (if any) and
// calls the selection callback with (0, 1). No static reference to 00AAC410
// was found (no rel32 call, no absolute dword), so in this image the hover
// pass never runs.
void update_button_hover_00aac410(GuiButtonRegistry& registry, GuiButtonFocusHost& host);

} // namespace bsp::gui_manager_cc13
