#pragma once
// The GUI "Icon" widget: the stateful image class the Lua page loader builds
// for any key whose suffix is `Icon` (type id 6, 138h bytes, allocator thunk
// 00AA1410). It owns a vector of 40h-byte state records, each a texture name,
// an atlas-resolved UV rectangle, an authored UV_LURB rectangle, a pivot and a
// size; the active state index selects one record and the class rebuilds a
// four-vertex strip from it.
//
// Addresses: 00AB5C60 (constructor), 00AB5D30 (copy constructor), 00AB2B70
// (property writer, vtable +1Ch), 00AB3310 (property reader, vtable +18h),
// 00AB66C0 (add state, vtable +A4h), 00AB1710 (select state, vtable +88h),
// 00AB1110 (vtable +84h), 00AB10F0 (vtable +78h), 00AB10D0 (vtable +8Ch),
// 00AB1680 (record UV setter), 00AB17B0 (native size from texture and UV),
// 00AB24B0 (record native-size test), 00AB2600 (filter choice), 00AB3B30
// (delayed-load placeholder), 00AB3CB0 (rebuild, vtable +80h, documented in
// docs/GUI_GEOMETRY_DISPATCH.md).
//
// Every name here is a hypothesis, not a recovered symbol. Evidence is in
// docs/GUI_ICON_WIDGET.md. Nothing here is binary compatible: the native
// object is a 138h-byte derived class over the 0ECh-byte widget base with a
// vtable, an std::vector of records and reference-counted texture pointers,
// none of which are reproduced. Texture and atlas resolution is not duplicated
// here: it is taken as a contract over bsp::resolve_gui_texture_00aa2660.
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/gui_geometry.hpp"
#include "bsp/gui_widget.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants read out of the listing
// ---------------------------------------------------------------------------

// The base constructor argument at 00AB5C6F. docs/GUI_LAYOUT_LOADER.md's type
// table gives the same id for the `Icon` key suffix.
inline constexpr std::int32_t kGuiIconTypeId = 6;

// 00AB5C60 allocates 138h bytes and the type table agrees. Recorded so a reader
// can check the field offsets below against the block size.
inline constexpr std::size_t kGuiIconNativeSize = 0x138;

// The record stride. Every count in the class is `(end - begin) >> 6`
// (00AB2B84, 00AB3D6B, 00AB175B) and every index is `index * 40h`.
inline constexpr std::size_t kGuiIconStateRecordSize = 0x40;

// 00CEC380 and 00CEF1B8, the two doubles 00AB17B0 divides the texture's pixel
// dimensions by. They are the logical GUI page size, so a texture drawn at one
// texel per logical unit has width/960 by height/720 extent.
inline constexpr double kGuiLogicalPageWidth = 960.0;
inline constexpr double kGuiLogicalPageHeight = 720.0;

// 00D5C3B8, compared against the native width at 00AB24CD. Below it the point
// filter is never chosen. 0.0364583 logical units is 35 texels at 960.
inline constexpr float kGuiIconMinPointFilterWidth = 0.0364583321f;

// 00D7A268, the double 00AB24F7 and 00AB2518 compare the absolute size
// difference against. Written as a double because the native comparison is one.
inline constexpr double kGuiIconNativeSizeEpsilon = 1.0000000000000001e-04;

// The three GUI material names 00AB3CB0 picks between at 00AB3E2E, 00AB3E6D and
// 00AB3E72, and the placeholder 00AB3B30 loads for a delayed state.
inline constexpr char kGuiIconFadeMaterial[] = "guifade.mshd";
inline constexpr char kGuiIconDefaultMaterial[] = "guidefault.mshd";
inline constexpr char kGuiIconPointMaterial[] = "guidefault_point.mshd";
inline constexpr char kGuiIconDelayedPlaceholder[] =
    "interface/textures/common/transparent.tga";

// ---------------------------------------------------------------------------
// One state record: 40h bytes, built on the stack by 00AB66C0
// ---------------------------------------------------------------------------

// Field offsets are the native ones. 00AB66C0 zeroes +00h..+08h, writes
// (0,0,1,1) into both UV rectangles, clears the pivot and size and clears the
// two trailing flags, then overwrites what its arguments supply.
struct GuiIconState {
    // +00h. The reference-counted texture. Null until resolved; 00AB3B30 puts
    // the transparent placeholder here when DelayedTextureLoad is set.
    void* texture{nullptr};

    // +04h length, +08h data: a native eight-byte string (see
    // include/bsp/native_string.hpp) holding the `Texture` key's value.
    // 00AB66C0 deep-copies the caller's string into it.
    std::string texture_name{};

    // +0Ch..+18h. What the draw actually samples: the atlas sub-rectangle
    // 00AA2660 wrote, after 00AB1680 applied the two flip patterns below.
    GuiUvRect resolved_uv{};

    // +1Ch..+28h, the `UV_LURB` key in authored order left, up, right, bottom.
    // 00AB66C0 copies all four verbatim and the property writer emits them
    // again, but the only values the resolved rectangle reacts to are the two
    // reversed patterns 00AB1680 tests. Default (0,0,1,1).
    GuiUvRect authored_uv{};

    // +2Ch/+30h, the `Pivot` key. Defaults to the widget's own +18h/+1Ch when
    // 00AB66C0's third argument is null.
    float pivot_x{0.0f};
    float pivot_y{0.0f};

    // +34h/+38h, the `Size` key. Defaults to the widget's own +20h/+24h when
    // 00AB66C0's fourth argument is null, and to the texture's native extent
    // when that pair is (0,0).
    GuiWidgetSize size{};

    // +3Ch and +3Dh. 00AB3B30 sets +3Ch and clears +3Dh for a delayed state;
    // 00AB66C0 clears both. Their readers are outside this packet, so only the
    // writes are established.
    bool load_pending{false};
    bool flag_3d{false};
};

// ---------------------------------------------------------------------------
// The derived part of the widget: +ECh..+134h over the 0ECh-byte base
// ---------------------------------------------------------------------------

// Defaults are 00AB5C60's stores. The base part lives in GuiWidgetTransform;
// the two are kept separate because the base is another packet's contract.
struct GuiIconWidget {
    // +ECh, signed 16-bit. -1 from the constructor. Every read sign-extends it
    // and the range check at 00AB3D5C compares it unsigned, so -1 fails the
    // check rather than selecting a state.
    std::int16_t current_state{-1};

    // +F0h..+FCh: an std::vector<GuiIconState>. +F0h is the allocator pad and
    // +F4h/+F8h/+FCh are first/last/end.
    std::vector<GuiIconState> states{};

    // +100h, `HasTexture`. Default true. False routes the whole draw down the
    // untextured path: no state lookup, no texture, UV (0,0,1,1) and the fade
    // material.
    bool has_texture{true};

    // +101h, `DelayedTextureLoad`. Default false. True makes 00AB66C0 skip the
    // resolve and hand the record to 00AB3B30 instead.
    bool delayed_texture_load{false};

    // +104h length, +108h data, `ShaderName`. Empty by default, and both the
    // property writer and 00AB2600 test the length. Non-empty replaces the
    // built-in material name and disables the point filter.
    std::string shader_name{};

    // +10Ch, `DynamicVB`. Default false. Carried through the serialiser; its
    // reader is outside this packet.
    bool dynamic_vb{false};

    // +110h, `PartialDisplayType`, and +114h, `PartialDisplayRatio`. Default 0
    // and 1. Passed straight to the quad writer as GuiQuadParameters::mode and
    // ::ratio.
    std::int32_t partial_display_type{0};
    float partial_display_ratio{1.0f};

    // +118h..+124h. The crop rectangle the quad writer interpolates the UVs
    // and the geometry with. Default (0,0,1,1). No property writes it, so a
    // page cannot author it; only code can.
    GuiUvRect crop{};

    // +128h and +12Ch, written together by 00AB1110 after it has forwarded the
    // state change. Nothing in this packet reads them.
    float field_128{0.0f};
    std::int16_t field_12c{0};

    // +130h, `AutoRotate`. Default 0. Serialised by 00AB2B70 and read by
    // 00AB3310; its consumer is outside this packet.
    float auto_rotate{0.0f};

    // +134h. 00AB3CB0 caches 00AB2600's answer here so it can keep the
    // existing material when the filter choice has not changed.
    bool cached_prefers_bilinear{false};
};

// ---------------------------------------------------------------------------
// Pure selection and geometry rules
// ---------------------------------------------------------------------------

// 00AB1680, __thiscall(record, float, float, float, float), RET 10h. Stores the
// atlas rectangle into the record's resolved UVs, then swaps the V pair when
// the authored rectangle reads (_, 1, _, 0) and the U pair when it reads
// (1, _, 0, _). Those are the only two authored patterns the resolved UVs react
// to; an arbitrary authored sub-rectangle is stored but not applied.
void gui_icon_set_resolved_uv_00ab1680(
    GuiIconState& state, const GuiUvRect& atlas_uv) noexcept;

// 00AB17B0, __fastcall(out, texture, const float uv[4]), RET 4. The texture's
// pixel dimensions divided by the logical page size and multiplied by the
// absolute UV extent, so a full-page texture measures 1 by 1. Both extents are
// taken through a sign mask, which is how a flipped rectangle keeps a positive
// size.
GuiWidgetSize gui_icon_native_size_00ab17b0(std::uint32_t texture_width,
    std::uint32_t texture_height, const GuiUvRect& uv) noexcept;

// 00AB24B0, __thiscall(record), RET 4, returning the byte 00AB2600 tests.
// False means the record is drawn at its texture's native extent, which is the
// case the point-sampled material exists for. True is returned for a native
// width under kGuiIconMinPointFilterWidth or for either axis differing from the
// native extent by more than kGuiIconNativeSizeEpsilon.
bool gui_icon_state_differs_from_native_size_00ab24b0(const GuiIconState& state,
    std::uint32_t texture_width, std::uint32_t texture_height) noexcept;

// 00AB2600, __thiscall(widget), RET, returning a byte. True selects
// `guidefault.mshd`, false `guidefault_point.mshd`. False needs every one of:
// a texture, no ShaderName override, the platform byte set, no rotation, unit
// scale on both axes, and a state drawn at its native extent.
bool gui_icon_prefers_bilinear_filter_00ab2600(const GuiIconWidget& icon,
    const GuiWidgetTransform& widget, bool platform_allows_point_filter,
    bool state_differs_from_native_size) noexcept;

// The name 00AB3CB0 hands to 00535320. The ShaderName override wins outright;
// otherwise an untextured widget takes the fade material and a textured one
// takes the filter choice above.
const char* gui_icon_material_name(
    const GuiIconWidget& icon, bool prefers_bilinear_filter) noexcept;

// The 00AB1710 clamp: [0,1] for ordered values, NaN unchanged, taken through
// two COMISS/JA tests rather than a min/max pair.
float gui_icon_clamp_partial_ratio_00ab1710(float ratio) noexcept;

// 00AB1710, __thiscall(widget, short index, int type, float ratio), RET 0Ch.
// Returns true when the caller must rebuild, which is when the state index
// changed to one that is either in range or untextured, or when the partial
// display type or the clamped ratio changed. On true the two partial display
// fields are written first and the index is left to the rebuild. On false
// nothing is written at all.
bool gui_icon_select_state_00ab1710(GuiIconWidget& icon, std::int16_t index,
    std::int32_t partial_display_type, float ratio) noexcept;

// The index arithmetic every reader repeats, including the unsigned compare
// that rejects the constructor's -1. Returns null when the index is out of
// range, where the native calls __report_rangecheckfailure.
const GuiIconState* gui_icon_state_at(
    const GuiIconWidget& icon, std::int16_t index) noexcept;

// The quad the rebuild feeds to gui_write_cropped_quad_00ab1860, and the
// pivot/size the rebuild copies back into the widget base on the way. On the
// textured path both come from the selected record; on the untextured path the
// UV is (0,0,1,1) and the size is the widget's own, with the pivot untouched.
struct GuiIconQuadSetup {
    GuiQuadParameters quad{};
    bool writes_widget_pivot{false};
    float pivot_x{0.0f};
    float pivot_y{0.0f};
    GuiWidgetSize size{};
};

// The part of 00AB3CB0 between the material and the vtable +7Ch call: the state
// lookup, the copies into widget +18h..+24h and the crop/ratio/mode the quad
// writer needs. Returns false when the widget is textured and the index is out
// of range, which is the native range-check failure.
bool gui_icon_build_quad_setup(const GuiIconWidget& icon,
    const GuiWidgetTransform& widget, GuiIconQuadSetup& setup) noexcept;

// ---------------------------------------------------------------------------
// The add-state path over an injected host
// ---------------------------------------------------------------------------

// One method per native call site inside 00AB66C0. There are no defaults:
// nothing here stands in for unrecovered behaviour.
struct GuiIconHost {
    virtual ~GuiIconHost() = default;

    // 00AA2660 through the GUI manager 004C12B0, with scale 1.0. The UV
    // rectangle goes in as (0,0,1,1) and comes back as the atlas
    // sub-rectangle; the size goes in as the authored pair and is filled with
    // the texture's extent only when both components are zero. Reuse
    // bsp::resolve_gui_texture_00aa2660 rather than reimplementing it.
    virtual void* resolve_texture(const std::string& name, GuiUvRect& uv,
        GuiWidgetSize& size, float scale) = 0;

    // The texture manager 00F8D394 virtual +64h that 00AB3B30 calls for
    // kGuiIconDelayedPlaceholder, with the reference it takes on the result.
    virtual void* load_placeholder_texture(const std::string& name) = 0;

    // The texture's pixel dimensions, the two virtuals 00AB17B0 calls at +48h
    // and +4Ch. Only reached on the fallback path, where the resolve left the
    // size at (0,0).
    virtual std::uint32_t texture_width(void* texture) = 0;
    virtual std::uint32_t texture_height(void* texture) = 0;
};

// 00AB66C0, __thiscall(widget, const NativeString* name, const float uv[4],
// const float2* pivot, const float2* size), RET 10h, returning the new state's
// index. A null pivot or size takes the widget's own pair. The authored UV goes
// into the record verbatim; the resolve is handed (0,0,1,1) and its answer
// becomes the resolved rectangle. DelayedTextureLoad skips the resolve and
// takes the placeholder instead.
std::int16_t gui_icon_add_state_00ab66c0(GuiIconWidget& icon,
    const GuiWidgetTransform& widget, const std::string& texture_name,
    const GuiUvRect& authored_uv, const float* pivot, const GuiWidgetSize* size,
    GuiIconHost& host);

// ---------------------------------------------------------------------------
// What a page authors
// ---------------------------------------------------------------------------

// The property set 00AB2B70 writes and 00AB3310 reads, in listing order, with
// the default each one is compared against. `States` is an integer-keyed array
// the derived class walks itself, because the base reader skips integer keys.
struct GuiIconAuthoredState {
    std::string texture{};       // `Texture`, default ""
    GuiWidgetSize size{};        // `Size`, default the widget's +20h/+24h
    float pivot_x{0.0f};         // `Pivot`, default the widget's +18h/+1Ch
    float pivot_y{0.0f};
    GuiUvRect uv{};              // `UV_LURB`, indices 1..4, default (0,0,1,1)
};

struct GuiIconAuthoredPage {
    bool dynamic_vb{false};             // `DynamicVB`, default false
    bool has_texture{true};             // `HasTexture`, default true
    std::string shader_name{};          // `ShaderName`, default ""
    std::int32_t partial_display_type{0};  // `PartialDisplayType`, default 0
    float partial_display_ratio{1.0f};  // `PartialDisplayRatio`, default 1
    bool delayed_texture_load{false};   // `DelayedTextureLoad`, default false
    float auto_rotate{0.0f};            // `AutoRotate`, default 0
    std::vector<GuiIconAuthoredState> states{};
};

// The scalar half of 00AB3310, in its order, followed by one
// gui_icon_add_state_00ab66c0 per `States` entry. The reader itself is a walk
// over the loader's variant visitor, which is docs/GUI_LAYOUT_LOADER.md's
// contract, so only the field assignment is reproduced.
void gui_icon_apply_authored_page_00ab3310(GuiIconWidget& icon,
    const GuiWidgetTransform& widget, const GuiIconAuthoredPage& page,
    GuiIconHost& host);

} // namespace bsp
