#pragma once
// `cGuiLayer`, the page-root widget the GUI manager keeps in its page list, and
// the manager-side routines that create it, walk it every frame, route the
// pointer into it and tear it down.
//
// Addresses: 00aa3840, 00aa38b0, 00aa38e0, 00aa38f0, 00aa3bd0, 00ac5480,
// 00ac6600, 00ac4c50, 00ac4d30, 00ac4450, 00ac3f80, 00ac3f90, 00ac3fa0,
// 00ac3fb0, 00ac57e0, 00ac51a0, 00ac4c40, 00ac4e50, 00aa4b30, 00aa3910,
// 00aa2f10, 00aa8bd0, 00aa87b0.
//
// Supporting addresses read but not owned: 00ac59a0 / 00ac6040 (the camera-store
// acquisition and copy construct, docs/GUI_RENDER_ORDER.md), 00aa5840 /
// 00aa52a0 / 00aa3140 / 00aa6560 (docs/GUI_LAYOUT_LOADER.md), 00aa4f80 /
// 00aa0f70 / 00aa0e00 / 00aa0e50 (docs/GAME_BLOCKING_SCREEN.md), 00aa8450
// (BSP_GuiWidget_PropagateVisibility), 00aaa710 / 00aaaed0 (the widget property
// reader and describer), 00aa7dc0 / 00aa7d00 (the local transform setters),
// 00b6d890 (node -> scene bind), 004155b0 (the float clamp), 00419210 (the
// 2-float vector length), 004bda70 / 00aa46b0 (the map iterator and erase).
//
// Every name here is a hypothesis, not a recovered symbol. Evidence is in
// docs/GUI_LAYER_MANAGER.md. Nothing here is binary compatible: the native
// layer is a 124h-byte pool block whose base half belongs to the widget class,
// and the scene, camera and Lua state are engine objects this packet only names.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_render_order.hpp"

namespace bsp {

// The three literals 00AC59A0 assigns into native strings when a layer builds its
// own camera, light set and directional light.
inline constexpr std::string_view kGuiCameraNamePrefix = "GuiCam_";
inline constexpr std::string_view kGuiLightSetName = "GuiLights";
inline constexpr std::string_view kGuiDirectionalLightName = "GuiDirectionalLight";

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------

// Every allocation site loads 124h into ECX before calling 00AC51A0
// (00AA3BE9 in the factory, 00AA5924 in BSP_GuiManager_LoadPage). 00AC51A0
// overwrites ECX with its pool pointer immediately, so the immediate is a dead
// `operator new(sizeof(cGuiLayer))` leftover -- which is exactly why it is good
// evidence for the class size. The constructor's highest write is the byte at
// +121h, consistent with 124h after padding.
inline constexpr std::size_t kGuiLayerSizeBytes = 0x124;

// The class vtable. 33 slots, +00h..+80h; the float pair at 00D5BEBC follows it.
inline constexpr std::uint32_t kGuiLayerVtable = 0x00D5BE38u;
inline constexpr std::size_t kGuiLayerVtableSlotCount = 33;

// The fixed-size pool every layer is allocated from and returned to.
inline constexpr std::uint32_t kGuiLayerPool = 0x00F8BF50u;

// The layer's own type id and the three-entry base chain 00AA38B0 scans.
inline constexpr std::uint32_t kGuiLayerTypeChainFirst = 0x00F8BF88u;
inline constexpr std::uint32_t kGuiLayerTypeChainEnd = 0x00F8BF94u;
inline constexpr std::uint32_t kGuiLayerTypeId = 0x00F8BF94u;

// One row of the vtable, as read out of 00D5BE38. `overridden` is false when the
// slot holds the same target as the neighbouring widget vtable at 00D5BECC.
struct GuiLayerVtableEntry {
    std::uint16_t offset;
    std::uint32_t target;
    bool overridden;
    std::string_view role;
};

// Only the slots this packet established are described; the rest are listed with
// an empty role so the table stays a faithful dump rather than a guess.
inline constexpr std::array<GuiLayerVtableEntry, 12> kGuiLayerVtableRoles{{
    {0x00, 0x00BD30E0u, false, "ref-counted deleting-destructor thunk"},
    {0x04, 0x00AA38F0u, true, "scalar deleting destructor, RET 4"},
    {0x08, 0x00AC3F80u, true, "type chain begin, returns [00F8BF88]"},
    {0x0C, 0x00AA38B0u, true, "IsKindOf(type id), RET 4"},
    {0x14, 0x00AC3F90u, true, "own type id, returns [00F8BF94]"},
    {0x18, 0x00AC4C50u, true, "ReadProperties(visitor), RET 4"},
    {0x1C, 0x00AC4D30u, true, "DescribeProperties(visitor), RET 4"},
    {0x2C, 0x00AC57E0u, true, "SetName(const NativeString*), RET 4"},
    {0x34, 0x00AC4450u, true, "SetVisible(bool), RET 4"},
    {0x38, 0x00AA38E0u, true, "IsVisible(), returns byte +0F5h"},
    {0x40, 0x00AA87B0u, false, "Update(float), the shared widget update"},
    {0x7C, 0x00AC3FA0u, true, "GetName(), returns this+100h"},
}};

// ---------------------------------------------------------------------------
// The object
// ---------------------------------------------------------------------------

// The base-class fields the layer's own routines touch. They are written by the
// widget constructor 00AA9390 and by the property reader, not by anything here;
// they are modelled so the layer rules can be expressed without reaching into
// another packet's type.
struct GuiLayerBaseFields {
    const void* scene_node{nullptr};    // +4Ch, the layer's own scene node
    std::uint8_t propagate_flag{0};     // +75h, passed straight to 00AA8450
    std::uint8_t light_init_done{0};    // +74h, gates the tail of 00AC59A0
    std::int32_t applied_priority{0};   // +FCh, the key the page vector sorts on
};

// `cGuiLayer` from +0ECh up. The layout is the one 00AC6600 writes in order at
// 00AC6624..00AC6710 and 00AA3840 writes at 00AA3852..00AA3877.
struct GuiLayerImage {
    GuiLayerBaseFields base{};

    const void* scene{nullptr};       // +0ECh, owned or borrowed from the store
    GuiCameraStore* store{nullptr};   // +0F0h, the shared camera store
    bool owns_store{false};           // +0F4h, 1 when 00AC59A0 created the store
    bool visible{false};              // +0F5h, the flag 00AC4450 maintains
    std::string name{};               // +100h length, +104h data (NativeString)
    GuiCameraStoreKey key{};          // +108h..+11Ch, the store descriptor
    bool share_scene{false};          // +120h, the load flag; see below
    std::uint8_t reserved_121{0};     // +121h, zeroed by 00AC6600, no reader found
};

// The second stack argument of BSP_GuiManager_LoadPage lands at layer+120h.
// docs/GUI_LAYOUT_LOADER.md carries it through without interpreting it; what it
// selects is the reuse branch of 00AC59A0 at 00AC59D2: a layer may adopt an
// existing store's scene only when this byte is set. Every screen passes 1 and
// the loading screen passes 0, so the loading screen always gets its own scene.
inline constexpr std::uint16_t kGuiLayerShareSceneOffset = 0x120;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

// 00AA3840, __fastcall(ECX = raw block), RET 0. Runs the widget constructor with
// argument 1, stamps the vtable and zeroes the layer half: name empty, flags 0,
// near 0.1f, far 1000.0f, scale 1.0f, Priority 0, RenderOrder +0.0f. It does not
// touch +0ECh, +0F0h, +0F4h, +0F5h or +120h; only 00AC6600 writes those, so an
// instance built through this path starts with whatever the widget constructor
// left there.
GuiLayerImage construct_00aa3840();

// 00AC6600, __thiscall(ECX = block, const NativeString* name, int share_scene,
// char unused_high_byte), RET 0Ch. The script-backed constructor
// BSP_GuiManager_LoadPage uses. Same field block as 00AA3840, plus the name
// copy, +120h from the argument, +121h zeroed and the widget scale pair at
// +20h/+24h set to 1.0f.
GuiLayerImage construct_from_script_00ac6600(std::string_view name, bool share_scene);

// 00AA3BD0, __fastcall(ECX = source layer or 0), RET 0. The type-1 case of
// BSP_GuiLayout_CreateWidgetOfType: allocate through 00AC51A0, then default- or
// copy-construct. Returns nothing when the pool is empty, as the native returns 0.
enum class GuiLayerCreateKind : std::uint8_t { Default, Copy };
GuiLayerCreateKind create_kind_00aa3bd0(bool has_source) noexcept;

// ---------------------------------------------------------------------------
// The two authored properties
// ---------------------------------------------------------------------------
// The key names, tags and defaults are in gui_render_order.hpp and are not
// repeated. What this packet adds is the asymmetry between the reader and the
// describer.

// 00AC4C50 writes `Priority` to BOTH the applied copy at +FCh and the authored
// copy at +118h, and `RenderOrder` to +11Ch.
void apply_read_properties_00ac4c50(
    GuiLayerImage& layer, std::int32_t priority, float render_order) noexcept;

// 00AC4D30 (no Ghidra function; 00AC4D30..00AC4D64) describes exactly one
// property, `Priority`, from the APPLIED copy at +FCh with default 0, then
// chains to BSP_GuiWidget_DescribeProperties. `RenderOrder` is read but never
// described, so a describe/read round trip loses it.
inline constexpr std::size_t kGuiLayerDescribedPropertyCount = 1;
bool layer_property_is_described_00ac4d30(std::string_view key) noexcept;

// ---------------------------------------------------------------------------
// Ordering
// ---------------------------------------------------------------------------

// 00AC59A0 tail (00AC5EFE..00AC5F14): when the applied Priority differs from the
// authored one it takes the authored value and re-registers the page, which is
// the only way a page moves inside the manager's vector after registration.
bool needs_reregistration_00ac59a0(const GuiLayerImage& layer) noexcept;
void adopt_authored_priority_00ac59a0(GuiLayerImage& layer) noexcept;

// 00AC59A0 at 00AC59D2: the reuse branch needs both a descriptor match and the
// +120h flag. Otherwise the layer creates its own scene, camera, light set and
// store, and 00AC59BE sets +0F4h to 1.
bool reuses_store_00ac59a0(bool share_scene, bool store_found) noexcept;

// 00AC5EF6: the scene is given the layer's flags OR 6.
inline constexpr std::int32_t kGuiLayerSceneFlagMask = 6;
std::int32_t scene_flags_00ac59a0(std::int32_t layer_flags) noexcept;

// ---------------------------------------------------------------------------
// Visibility
// ---------------------------------------------------------------------------

// 00AC4450, __thiscall(ECX = layer, bool visible), RET 4, vtable +34h.
// The store count moves only on a real edge (00AC4458 CMP / JZ).
int store_count_delta_00ac4450(bool current, bool requested) noexcept;

// The five stack arguments 00AC4478..00AC447E push to
// BSP_GuiWidget_PropagateVisibility, in stack order low to high. It is called
// unconditionally, on an edge or not.
struct GuiPropagateVisibilityCall {
    std::int32_t arg0{1};
    std::int32_t arg1{1};
    bool visible{false};
    std::uint8_t propagate_flag{0};  // layer +75h
    std::int32_t arg4{1};
};
GuiPropagateVisibilityCall propagate_call_00ac4450(
    const GuiLayerImage& layer, bool visible) noexcept;

// 00AC4485..00AC449B: only when the node at +4Ch exists. Binding to the scene on
// show, unbinding (argument 0) on hide.
struct GuiLayerSceneBinding {
    bool called{false};
    const void* scene{nullptr};
};
GuiLayerSceneBinding scene_binding_00ac4450(
    const GuiLayerImage& layer, bool visible) noexcept;

// 00AC44A0..00AC44BE: the hook at 00F8BF4C, installed by 009870A0 and cleared by
// 00989A80, is called as __fastcall(ECX = this->GetName(), DL = visible) on a
// real edge only. The flag write at 00AC44C4 happens last, so the hook still
// sees the old flag.
bool notifies_hook_00ac4450(bool hook_installed, bool current, bool requested) noexcept;

// The whole slot.
void set_visible_00ac4450(GuiLayerImage& layer, bool visible) noexcept;

// ---------------------------------------------------------------------------
// The per-frame update
// ---------------------------------------------------------------------------

// The layer does not override the update: vtable +40h is 00AA87B0, the widget
// update every non-page widget also uses. Both BSP_GuiManager_Update (00AA4F80,
// over the page snapshot) and 00AA87B0 (over a widget's child list at +68h) gate
// each entry with the same rule, so the manager's page walk and a widget's child
// walk share one predicate.
bool updates_this_frame_00aa87b0(bool visible, bool has_live_entries) noexcept;

// 00AA87B0 at 00AA87BE: the elapsed accumulator at widget +80h, advanced before
// the children are walked and regardless of the widget's own visibility.
inline constexpr std::uint16_t kGuiWidgetElapsedOffset = 0x80;
float advance_elapsed_00aa87b0(float elapsed, float seconds) noexcept;

// ---------------------------------------------------------------------------
// Pointer routing
// ---------------------------------------------------------------------------

// 00AA3910, __fastcall(ECX = manager), RET 0. BSP_GuiManager_Update calls it
// first, and only when its own flag argument is 0.
//
// It reads the cursor device at DAT_00F8BBF4+94h (element 0 of the device
// vector), takes X through the device's vtable +3Ch and Y through +40h, both as
// signed ints, and differences them against the latch at 00F8BC64/00F8BC68. The
// latch is primed on the first call, guarded by bit 0 of DAT_00F8BC6C, so the
// first frame contributes no motion.

// 00AA3A36: the whole motion, clamp, hit test and cursor-state block is skipped
// when the manager's byte at +48h is zero. That is the only reader of the byte
// BSP_GuiManager_SetEnabled writes, which docs/GAME_BLOCKING_SCREEN.md records
// as unresolved.
inline constexpr std::uint16_t kGuiManagerEnabledOffset = 0x48;

// 00AA3A45 and 00AA3A65, both loaded as doubles and multiplied in x87. The
// product is 1/480 per device unit, applied as two float-rounded steps.
inline constexpr double kGuiPointerScaleFirst = 0.0010416667209938169;  // 00D5BEC8
inline constexpr double kGuiPointerScaleSecond = 2.0;                   // 00D7A308
float scale_pointer_delta_00aa3910(float raw_delta) noexcept;

// 00AA3AC6..00AA3B1D for X and 00AA3B24..00AA3B59 for Y. Y always uses the
// narrow pair; X widens when the byte at [0109CF04+0Dh] is set. 004155B0 is a
// clamp, __fastcall(ECX = &value, EDX = &lo, [ESP] = &hi), result in ST0.
struct GuiPointerBounds {
    float lo;
    float hi;
};
inline constexpr GuiPointerBounds kGuiPointerBoundsNarrow{
    0.009999999776482582F, 0.9900000095367432F};  // 00D7A238, 00CE4E0C
inline constexpr GuiPointerBounds kGuiPointerBoundsWide{
    -0.15666666626930237F, 1.15666663646698F};  // 00D5BEBC, 00D5BEC0
GuiPointerBounds pointer_x_bounds_00aa3910(bool wide_screen) noexcept;
float clamp_pointer_004155b0(float value, GuiPointerBounds bounds) noexcept;

// The Y clamp is inlined and compares the high bound as a double
// (00AA3B41 FLD double [00CED5D0]), which is the same 0.99f promoted, so it
// agrees with the float compare for every representable input.
float clamp_pointer_y_00aa3910(float value) noexcept;

// 00AA3AA1..00AA3AC4: when the FE cursor icon reports hidden through its vtable
// +38h and the length of the scaled delta exceeds 0.01f, the icon is shown.
inline constexpr float kGuiPointerWakeLength = 0.009999999776482582F;  // 00D7A238
bool wakes_pointer_00aa3910(bool icon_visible, float delta_length) noexcept;

// The manager fields the pass reads and writes.
struct GuiPointerState {
    float x{0.5F};        // +5Ch, seeded with 0.5f by the constructor 00AA5D70
    float y{0.5F};        // +60h
    float delta_x{0.0F};  // +64h, this frame's scaled delta
    float delta_y{0.0F};  // +68h
};
struct GuiPointerLatch {
    bool primed{false};   // bit 0 of DAT_00F8BC6C
    float x{0.0F};        // 00F8BC64
    float y{0.0F};        // 00F8BC68
};

// The motion half of 00AA3910: latch, difference, scale, accumulate, clamp.
void advance_pointer_00aa3910(
    GuiPointerState& state, GuiPointerLatch& latch,
    std::int32_t device_x, std::int32_t device_y, bool wide_screen) noexcept;

// ---------------------------------------------------------------------------
// Hit testing
// ---------------------------------------------------------------------------

// 00AA2F10, __fastcall(ECX = manager), RET 0, called at 00AA3B7E. It publishes
// the pointer position to 00F8BC74/00F8BC78, clears the hit widget at 00F8BC70
// and seeds the score at 00F8BC7C with 1e10f, then walks the page vector at
// manager+18h..+1Ch -- the same vector, in the same ascending-Priority order,
// that the update walks.
inline constexpr float kGuiHitScoreSeed = 1.0E10F;  // 00CE4970

// 00AA2FA9: a page is descended into only when its vtable +38h reports visible.
// 00AA2FBB: when manager+6Ch is non-null only that one page is tested, which is
// the manager's exclusive-page filter. The constructor zeroes +6Ch; no writer was
// found, so the field is named from its use.
inline constexpr std::uint16_t kGuiManagerExclusivePageOffset = 0x6C;
bool page_is_hit_tested_00aa2f10(
    bool page_visible, const void* page, const void* exclusive_page) noexcept;

// 00AA8BD0, __thiscall(ECX = widget, const float origin[3]), RET 4. One node as
// the hit test reads it. `half_extent_x` is the product at 00AA8BD5 of the size
// at +18h and the scale at +20h, and it is also the score.
//
// The box is taken as an input rather than derived. 00AA8C6C hands four float
// pointers to the widget's vtable +64h, which rewrites them in place, and the
// four register values the comparisons then use are the rewritten ones; Ghidra
// lost their register attribution (unaff_EDI/EBP/ESI), so deriving the edges
// from +30h/+34h here would be a guess. The comparisons themselves are exact.
struct GuiHitBox {
    float left{0.0F};
    float right{0.0F};
    float top{0.0F};
    float bottom{0.0F};
};

// The four fields at +38h, +3Ch, +40h and +44h. Each is disabled by an exact
// +0.0f compare (00AA8CD3, 00AA8CE7, 00AA8CFB, 00AA8D0F) before it rejects.
struct GuiHitLimits {
    float gate_right{0.0F};   // +38h: reject when non-zero and greater than right
    float min_left{0.0F};     // +3Ch: reject when non-zero and less than left
    float min_bottom{0.0F};   // +40h: reject when non-zero and greater than bottom
    float min_top{0.0F};      // +44h: reject when non-zero and less than top
};

struct GuiHitNode {
    float pos[3]{0.0F, 0.0F, 0.0F};      // +0Ch, +10h, +14h
    float size[2]{0.0F, 0.0F};           // +18h, +1Ch
    float scale[2]{1.0F, 1.0F};          // +20h, +24h
    GuiHitBox box{};                     // as vtable +64h leaves it
    GuiHitLimits limits{};                // +38h..+44h
    bool mouse_hit{false};               // +78h low byte
    bool suppressed{false};              // +77h
    bool visible{false};                 // read through vtable +38h
    std::vector<GuiHitNode> children{};  // the list at +68h, in list order
};

// 00AA8CB4..00AA8D1C, in listing order. False rejects the widget itself; the
// children have already been walked by then either way.
bool hit_limits_allow_00aa8bd0(const GuiHitBox& box, const GuiHitLimits& limits) noexcept;

// 00AA8D28..00AA8DE4: the pointer must sit inside [left, right] and
// [top, bottom]. The left compare is a separate early return.
bool hit_box_contains_00aa8bd0(
    const GuiHitBox& box, float pointer_x, float pointer_y) noexcept;

// 00AA8C14..00AA8C36: the child origin the recursion passes down. X and Y drop
// the half extents; Z subtracts the double at 00D7A258, which is +0.0, so the
// child inherits the parent's absolute Z unchanged.
inline constexpr double kGuiHitChildZBias = 0.0;  // 00D7A258, FSUB double

// The winner rule, and the single most surprising thing in the packet: among the
// widgets whose box contains the pointer the one with the SMALLEST
// `half_extent_x` wins (00AA8E0C COMISS / JBE against 00F8BC7C). Depth, Z and
// tree order decide nothing; a strict `<` means the first widget reaching a
// given score keeps it.
bool hit_candidate_wins_00aa8bd0(float half_extent_x, float best_score) noexcept;

struct GuiHitResult {
    const GuiHitNode* widget{nullptr};  // 00F8BC70
    float score{kGuiHitScoreSeed};      // 00F8BC7C
};

// The recursion: children first, in list order and only when visible, then the
// widget itself. `origin` is the parent origin the caller passes; the child
// origin is this widget's absolute position minus its half extents.
void hit_test_widget_00aa8bd0(
    const GuiHitNode& widget, const float origin[3], float pointer_x,
    float pointer_y, GuiHitResult& result) noexcept;

// 00AA2F10 as a whole, over the page list in registration order.
GuiHitResult hit_test_pages_00aa2f10(
    const std::vector<const GuiHitNode*>& pages, float pointer_x, float pointer_y,
    const GuiHitNode* exclusive_page) noexcept;

// 00AA3B83..00AA3BB4: the cursor state the FE icon is told after the hit test.
// The icon's vtable +88h takes (state, 0, 1.0f).
inline constexpr float kGuiPointerStateBlend = 1.0F;  // FLD1 at 00AA3B9E
std::int32_t pointer_icon_state_00aa3910(
    const GuiHitNode* hit_widget, bool hit_widget_byte_84) noexcept;

// ---------------------------------------------------------------------------
// Teardown
// ---------------------------------------------------------------------------

// 00AC5480, __fastcall(ECX = layer), RET 0. It re-stamps the vtable, then takes
// one of two paths on +0F4h, releases the name buffer to the sized pool and
// chains to the widget destructor 00AA9730.
enum class GuiLayerTeardownPath : std::uint8_t {
    ReleaseSharedScene,  // +0F4h == 0: drop one scene reference, null +0ECh
    DestroyOwnedStore,   // +0F4h != 0: manager remove-store, then destroy scene
};
GuiLayerTeardownPath teardown_path_00ac5480(const GuiLayerImage& layer) noexcept;

// 00AA38F0, __thiscall(ECX = layer, byte flags), RET 4: destructor, then the
// pool free 00AC4C40 when bit 0 of the flags is set. Returns the layer.
bool frees_block_00aa38f0(std::uint8_t deleting_flags) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site the layer's create, update and teardown leave
// through, in the style of bsp::run_application_frame. Nothing has a default
// implementation: no unrecovered behaviour is stood in for here.
struct GuiLayerHost {
    virtual ~GuiLayerHost() = default;

    // 00AC6600: the Lua state, `_Common` prelude, the page script and the
    // `GuiScreen` global the property visitor is built over.
    virtual void run_common_script_00b69d40(std::string_view name) = 0;
    virtual bool run_page_script_00b69d40(std::string_view name) = 0;

    // 00AC6814: this->vtable[+18h](visitor). The two values the layer's own
    // reader takes out of the table; anything else belongs to 00AAA710.
    virtual std::int32_t read_priority_00ac4c50(std::int32_t fallback) = 0;
    virtual float read_render_order_00ac4c50(float fallback) = 0;

    // 00AC4C9E: this->vtable[+78h](), which is 00AC59A0. `store` is the result of
    // BSP_GuiManager_FindCameraStore on the layer's descriptor.
    virtual GuiCameraStore* find_camera_store_00aa3280(const GuiCameraStoreKey& key) = 0;
    virtual const void* create_scene_00b724e0(std::string_view layer_name) = 0;
    virtual void retain_scene_00ce221c(const void* scene) = 0;
    virtual const void* create_camera_00b71a80(std::string_view camera_name) = 0;
    virtual GuiCameraStore* create_camera_store_00aa5070(
        const void* camera, const void* scene, const GuiCameraStoreKey& key) = 0;
    virtual void set_scene_flags_00b6fe10(const void* scene, std::int32_t flags) = 0;
    virtual void add_scene_lights_00b83c50(const void* scene) = 0;

    // 00AC5F14: BSP_GuiManager_RegisterPageByPriority, after the applied Priority
    // is taken from the authored copy.
    virtual void register_page_00aa52a0(GuiLayerImage& layer) = 0;

    // 00AC4480 and 00AC449B.
    virtual void propagate_visibility_00aa8450(const GuiPropagateVisibilityCall& call) = 0;
    virtual void bind_node_to_scene_00b6d890(const void* node, const void* scene) = 0;

    // 00AC44BE, only when the hook at 00F8BF4C is installed.
    virtual bool visibility_hook_installed_00f8bf4c() = 0;
    virtual void visibility_hook_00f8bf4c(std::string_view layer_name, bool visible) = 0;

    // 00AA502D, the per-page update the manager issues, and the live-entry gate
    // 00AA7EF0 in front of it.
    virtual bool page_has_live_entries_00aa7ef0(const GuiLayerImage& layer) = 0;
    virtual void update_page_00aa87b0(GuiLayerImage& layer, float seconds) = 0;

    // 00AC5480: 00AA4B30 on the manager, then 00B72250 for the owned scene, or
    // the shared-scene release.
    virtual void remove_camera_store_00aa4b30(GuiCameraStore* store) = 0;
    virtual void destroy_scene_00b72250(const void* scene) = 0;
    virtual void release_scene_00ce2220(const void* scene) = 0;

    // 00AC5480 tail and 00AA9730.
    virtual void free_name_buffer_00bd1510(std::string_view name) = 0;
    virtual void destruct_widget_base_00aa9730(GuiLayerImage& layer) = 0;
};

// BSP_GuiManager_LoadPage's construction half, from 00AC6600 through the
// property read and 00AC59A0 to the registration at 00AA5957. Returns false when
// the page script did not evaluate, which is the branch 00AC6600 takes silently.
bool create_layer_for_page(
    GuiLayerHost& host, GuiLayerImage& layer, std::string_view name,
    bool share_scene);

// The per-page half of BSP_GuiManager_Update (00AA4FE0..00AA502D) for one layer.
bool update_layer(GuiLayerHost& host, GuiLayerImage& layer, float seconds);

// 00AA38F0 with bit 0 set: destructor then pool free.
void destroy_layer(GuiLayerHost& host, GuiLayerImage& layer, std::uint8_t deleting_flags);

// ---------------------------------------------------------------------------
// What the installed scripts actually use
// ---------------------------------------------------------------------------
// Counted over the 97 `interface/*.lua` files of this installation; the tables
// are in docs/GUI_LAYER_MANAGER.md.

inline constexpr std::size_t kInstalledInterfaceScriptCount = 97;
inline constexpr std::size_t kInstalledPageCount = 96;              // `_common.lua` has no GuiScreen
inline constexpr std::size_t kInstalledPagesWithPriority = 37;
inline constexpr std::size_t kInstalledDistinctPriorities = 25;
inline constexpr std::size_t kInstalledPagesWithRenderOrder = 2;    // _debugtexts, _highlight
inline constexpr std::size_t kInstalledDistinctRenderOrders = 2;    // 0.0f and 4.0f

}  // namespace bsp
