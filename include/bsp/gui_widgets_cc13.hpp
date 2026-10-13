#pragma once
// GUI widget library, segments 76-79 (00AA0F70-00AD5360), packet cc13-gui1,
// docs/CC13_GUI_1.md. Reconstructions of the widget class plumbing the earlier
// naming passes (cc13-gui6, cc13_gui_render, CC13_HUD_AND_WIDGETS) left without
// source: type descriptors, the type-table factories, the scalar deleting
// destructors and the per-class leaf virtuals.
//
// Names are hypotheses, not recovered symbols; the class names in comments
// (cGuiElement, cGuiCurve, cGuiProgressbar) are the strings the descriptor
// initializers store. Engine routines that are not reconstructed stay behind
// host interfaces, as in include/bsp/gui_manager_cc13.hpp. Native widget objects
// are handled as raw storage where the image writes fixed offsets (vtable words,
// vector fields); those writes keep the native addresses as values.
//
// Status: reconstructed and build-tested; not ABI-compatible, not game-validated.
#include <array>
#include <cstddef>
#include <cstdint>
#include <new>
#include <string>
#include <utility>

#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/gui_render.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_gui_widget_pools_cc13.hpp"

namespace bsp::gui_widgets_cc13 {

// ---------------------------------------------------------------------------
// Type descriptors
// ---------------------------------------------------------------------------

// The engine's run-time type records (the same family as the light and node
// descriptors in include/bsp/light_type_bootstrap.hpp): the class's own id, the
// ids of its ancestors nearest first, then the address of the class name. A
// process-wide guard byte makes the initializer run once.
template <std::size_t Depth>
struct GuiTypeDescriptor {
    std::uint32_t ids[Depth];
    std::uint32_t native_name_address;
};

// Leaf readers. The three exist per class with the descriptor address as an
// immediate (table kGuiTypeDescriptors):
//   own id   - MOV EAX, [descriptor]; RET              (ledger "GetTypeChain")
//   name     - MOV EAX, [descriptor + 4*Depth]; RET    (ledger "GetTypeId")
//   is kind  - __thiscall(this, id), RET 4: scans descriptor[0..Depth) for id
// None of them runs the initializer; a class whose descriptor is not yet
// initialized reports id 0 and a null name.
template <std::size_t Depth>
std::uint32_t gui_type_own_id(const GuiTypeDescriptor<Depth>& d) noexcept { return d.ids[0]; }

template <std::size_t Depth>
std::uint32_t gui_type_name_address(const GuiTypeDescriptor<Depth>& d) noexcept {
    return d.native_name_address;
}

template <std::size_t Depth>
bool gui_type_is_kind_of(const GuiTypeDescriptor<Depth>& d, std::uint32_t id) noexcept {
    for (std::uint32_t own : d.ids)
        if (own == id) return true;
    return false;
}

// The initializer, __fastcall(ECX = target descriptor). When the guard is clear:
// set the guard, store the name word, run the parent initializer, copy the
// parent's ids (Depth-1 words) behind the own slot, then take the next id from
// the shared counter (006FAC20, counter +4, post-increment). The guard is the
// class's, not the target's, exactly as the light bootstrap notes.
// 00ABCC20 and 00AC7BD0 are the variants that return the target in EAX.
struct GuiTypeCounterHost {
    virtual ~GuiTypeCounterHost() = default;
    // 006FAC20 BSP_TypeId_GetCounterSingleton.
    virtual TypeIdCounterStorage& counter() = 0;
};

template <std::size_t Depth, class ParentInit>
void gui_type_initialize(GuiTypeDescriptor<Depth>& target, std::uint8_t& guard,
    std::uint32_t native_name_address, const std::uint32_t (&parent_ids)[Depth - 1],
    ParentInit&& initialize_parent, GuiTypeCounterHost& host) {
    if (guard != 0) return;
    guard = 1;
    target.native_name_address = native_name_address;
    initialize_parent();
    for (std::size_t i = 0; i + 1 < Depth; ++i) target.ids[i + 1] = parent_ids[i];
    auto& counter = host.counter();
    target.ids[0] = counter.next_id_04++;
}

// One row per class whose leaf readers are in this segment range. 0 means the
// routine is not a separate function (or not in this range).
struct GuiTypeDescriptorRow {
    const char* class_label;     // the widget the routines serve
    std::uint32_t descriptor;    // own-id word
    std::uint32_t depth;         // ids before the name word
    std::uint32_t own_id_reader; // ledger "GetTypeChain"
    std::uint32_t name_reader;   // ledger "GetTypeId"
    std::uint32_t is_kind_of;
    std::uint32_t initializer;
};

inline constexpr std::array<GuiTypeDescriptorRow, 13> kGuiTypeDescriptors{{
    {"Element (cGuiElement, base of every widget)", 0x00F8BC88, 2, 0, 0, 0, 0x00AA8200},
    {"Icon", 0x00F8BDD8, 3, 0, 0, 0x00AA1CC0, 0x00AB3180},
    {"Movie", 0x00F8BD84, 3, 0, 0, 0x00AAFEB0, 0},
    {"Curve (cGuiCurve)", 0x00F8BE94, 3, 0x00ABC020, 0x00ABC030, 0x00ABDFE0, 0x00ABCBD0},
    {"Section", 0x00F8BEE4, 3, 0, 0, 0x00ABF4C0, 0},
    {"Line", 0x00F8BF34, 3, 0x00AC1B90, 0x00AC1BA0, 0x00AC2920, 0},
    {"Layer", 0x00F8BF88, 3, 0x00AC3F80, 0x00AC3F90, 0x00AA38B0, 0},
    {"Progbar (cGuiProgressbar, derived from Icon)", 0x00F8BFEC, 4, 0x00AC77A0, 0x00AC77B0, 0x00AC77F0, 0x00AC7B80},
    {"Scrollbar", 0x00F8C03C, 3, 0x00AC7C00, 0x00AC7C10, 0x00AC7C60, 0},
    {"Model", 0x00F8C0D4, 3, 0x00ACBCD0, 0x00ACBCE0, 0x00ACC780, 0},
    {"Sound", 0x00F8C124, 3, 0x00ACD0D0, 0x00ACD0E0, 0x00ACDB70, 0},
    {"ClipBox", 0x00F8C174, 3, 0x00ACE080, 0x00ACE090, 0x00ACE0C0, 0},
    {"FrameBox", 0x00F8C1C0, 3, 0x00ACEAE0, 0x00ACEAF0, 0x00AD26A0, 0},
}};

// The four initializers whose code was read: guard, name string, parent.
// 00AA8200: guard 00F8BC80, name 00D5C124 "cGuiElement", parent the root
//           descriptor 0109DB84 (00BEA780, LightTypeBootstrap::initialize_root).
// 00ABCBD0 / 00ABCC20: guard 00F8BE58, name 00D5C7DC "cGuiCurve", parent Element.
// 00AB3180: Icon (not read beyond its call from 00AC7B80).
// 00AC7B80 / 00AC7BD0: guard 00F8BFE8, name 00D5CCDC "cGuiProgressbar", parent
//           Icon 00F8BDD8 (three ids copied).
inline constexpr std::uint32_t kGuiElementTypeGuard = 0x00F8BC80;
inline constexpr std::uint32_t kGuiElementTypeName = 0x00D5C124;
inline constexpr std::uint32_t kGuiCurveTypeGuard = 0x00F8BE58;
inline constexpr std::uint32_t kGuiCurveTypeName = 0x00D5C7DC;
inline constexpr std::uint32_t kGuiProgbarTypeGuard = 0x00F8BFE8;
inline constexpr std::uint32_t kGuiProgbarTypeName = 0x00D5CCDC;

// ---------------------------------------------------------------------------
// Type-table factories (the 00AA6560 switch targets)
// ---------------------------------------------------------------------------

// Each is `source ? new T(*source) : new T()` with the class's pool operator
// new and delete: __fastcall(ECX = source widget or null), EAX = the new
// widget or null when the pool returned null. The size the stub loads into ECX
// before the operator-new call is discarded by the pool stub (MOV ECX, pool).
// If the constructor throws, the unwind funclet returns the slot through the
// pool's free thunk (read for Icon: 00CB6E20 -> 00AB24A0).
struct GuiWidgetConstructionHost {
    virtual ~GuiWidgetConstructionHost() = default;
    // The class's default constructor, __fastcall(ECX = slot), EAX = this.
    virtual void* construct_default(GuiWidgetType type, void* slot) = 0;
    // The class's copy constructor, __thiscall(slot, source), RET 4, EAX = this.
    virtual void* copy_construct(GuiWidgetType type, void* slot, const void* source) = 0;
};

struct GuiWidgetFactoryRow {
    GuiWidgetType type;
    std::uint32_t factory;
    std::uint32_t allocate_stub; // pool stub the factory calls
    std::uint32_t default_constructor;
    std::uint32_t copy_constructor;
};

inline constexpr std::array<GuiWidgetFactoryRow, 14> kGuiWidgetFactories{{
    {GuiWidgetType::Icon, 0x00AA1410, 0x00AB2B60, 0x00AB5C60, 0x00AB5D30},
    // Progbar uses the ICON pool: its 140h object lives in a 13Ch slot whose
    // slab-index dword is at +138h, the Progbar value field (see the doc).
    {GuiWidgetType::Progbar, 0x00AA14A0, 0x00AB2B60, 0x00AC77C0, 0x00AC7840},
    {GuiWidgetType::Line, 0x00AA1530, 0x00AC2380, 0x00AC2D00, 0x00AC2D60},
    {GuiWidgetType::Scrollbar, 0x00AA15C0, 0x00AC84F0, 0x00AC7C20, 0x00AC7CB0},
    {GuiWidgetType::Grid, 0x00AA1650, 0x00AC9EA0, 0x00ACB0A0, 0x00ACB3F0},
    {GuiWidgetType::Model, 0x00AA16E0, 0x00ACC490, 0x00ACC740, 0x00ACCF70},
    {GuiWidgetType::Movie, 0x00AA1770, 0x00AAD990, 0x00AAFE20, 0x00AAFFF0},
    {GuiWidgetType::Curve, 0x00AA1800, 0x00ABC790, 0x00ABE030, 0x00ABE100},
    {GuiWidgetType::Sound, 0x00AA1890, 0x00ACD7F0, 0x00ACDB30, 0x00ACDBA0},
    {GuiWidgetType::ClipBox, 0x00AA1920, 0x00ACEA10, 0x00ACE0A0, 0x00ACE0F0},
    {GuiWidgetType::Section, 0x00AA19B0, 0x00ABF080, 0x00ABF420, 0x00ABF5B0},
    {GuiWidgetType::FrameBox, 0x00AA1A40, 0x00AD0280, 0x00AD2600, 0x00AD27A0},
    {GuiWidgetType::AnimIcon, 0x00AA2340, 0x00AD3960, 0x00AD2D50, 0x00AA1C60},
    {GuiWidgetType::Button, 0x00AA4440, 0x00AAC260, 0x00AAC6F0, 0x00AA3C60},
}};

// The factory body shared by every row; Pool is the traits of the pool the
// row's allocate stub targets.
template <class Pool>
void* gui_widget_default_or_copy_factory(GuiWidgetType type, const void* source,
    GuiWidgetConstructionHost& host);

// Runtime dispatch over kGuiWidgetFactories; null for a type with no row.
void* gui_widget_factory(GuiWidgetType type, const void* source, GuiWidgetConstructionHost& host);

// ---------------------------------------------------------------------------
// Destructors
// ---------------------------------------------------------------------------

struct GuiWidgetDestructionHost {
    virtual ~GuiWidgetDestructionHost() = default;
    // A destructor body that is not reconstructed here, __fastcall(ECX = this).
    virtual void run_destructor(std::uint32_t native_address, void* self) = 0;
    // 00BF65AC _free (the CRT free the widget vectors use).
    virtual void free_block(void* block) = 0;
};

// Scalar deleting destructor, __thiscall(this, flags), RET 4, EAX = this:
// run the destructor, then return the slot to the class pool when flags bit 0
// is set. Some instances inline the pool free (PUSH ESI; MOV ECX, pool; CALL
// free), others call the pool's free thunk; the effect is the same.
struct GuiWidgetDeletingDestructorRow {
    std::uint32_t address;
    std::uint32_t destructor;     // run first
    std::uint32_t vtable_store;   // nonzero when the destructor is inlined as
                                  // "store this vtable, then call the base"
    GuiWidgetType pool_type;      // whose pool receives the slot
};

inline constexpr std::array<GuiWidgetDeletingDestructorRow, 13> kGuiWidgetDeletingDestructors{{
    {0x00AA1D50, 0x00AD2DC0, 0, GuiWidgetType::AnimIcon},
    {0x00AA3D10, 0x00AAC530, 0, GuiWidgetType::Button},
    {0x00AA9A90, 0x00AA9730, 0, GuiWidgetType::None}, // widget base, Widget pool
    {0x00AB0480, 0x00AAFEF0, 0, GuiWidgetType::Movie},
    {0x00AB6100, 0x00AB5E70, 0, GuiWidgetType::Icon},
    {0x00ABE010, 0x00ABDE90, 0, GuiWidgetType::Curve},
    {0x00AC7900, 0x00AB5E70, 0x00D5CC18, GuiWidgetType::Icon}, // Progbar, Icon pool
    {0x00AC8210, 0x00AA9730, 0x00D5CCF8, GuiWidgetType::Scrollbar},
    {0x00ACBC90, 0x00ACBBF0, 0, GuiWidgetType::Grid},
    {0x00ACC880, 0x00ACC7B0, 0, GuiWidgetType::Model},
    {0x00ACDEF0, 0x00ACDC40, 0, GuiWidgetType::Sound},
    {0x00ACE620, 0x00AA9730, 0x00D5D058, GuiWidgetType::ClipBox},
    {0x00AD29B0, 0x00AD2880, 0, GuiWidgetType::FrameBox},
}};

// Null when address is not a row.
void* gui_widget_scalar_deleting_destructor(std::uint32_t address, void* self, std::uint32_t flags,
    GuiWidgetDestructionHost& host);

// 00AC7830 (Progbar) and 00AC7CA0 (Scrollbar), __fastcall(ECX = this): store the
// class vtable (00D5CC18 / 00D5CCF8) and tail-jump to the base destructor
// (Icon 00AB5E70 / widget base 00AA9730). The classes own nothing else.
void gui_widget_destroy_with_base(void* self, std::uint32_t vtable, std::uint32_t base_destructor,
    GuiWidgetDestructionHost& host);

// 00AC28D0 Line destructor, __fastcall(ECX = this): store vtable 00D5C9E8, free
// the point vector at +F0h (begin; end +F4h and capacity +F8h are zeroed with
// it), then tail-jump to the widget base destructor 00AA9730. 00AC2CA0, the
// scalar deleting destructor, inlines this body and frees into the Line pool.
void gui_line_destroy(void* self, GuiWidgetDestructionHost& host);
void* gui_line_scalar_deleting_destructor(void* self, std::uint32_t flags, GuiWidgetDestructionHost& host);

// ---------------------------------------------------------------------------
// Property describers and readers
// ---------------------------------------------------------------------------

// The 8-byte variants a describer passes (tag, value) and the reader fills.
// Tags are GuiValueTag (gui_layout_loader.hpp). Floats are copied as stored.
struct GuiTaggedValue {
    GuiValueTag tag{GuiValueTag::String};
    std::array<float, 4> f{};
    bool b{false};
    const char* s{nullptr};
    std::int32_t i{0}; // tag 1 (an integer; PartialDisplayType)
};

// A describer visitor (the object a widget's vtable +1Ch receives).
struct GuiPropertyWriter {
    virtual ~GuiPropertyWriter() = default;
    // 00BD5680 BSP_TaggedValue_Equals(value, default).
    virtual bool equals_00bd5680(const GuiTaggedValue& value, const GuiTaggedValue& def) = 0;
    // visitor +0Ch, four dwords: (0, name), (tag, value).
    virtual void write(const char* name, const GuiTaggedValue& value) = 0;
    // visitor +04h / +08h: open a named sub-table, close it.
    virtual void open_table(const char* name) = 0;
    virtual void close_table() = 0;
    // The same two calls with an integer key (variant tag 1).
    virtual void open_index_table(std::int32_t key) = 0;
    virtual void write_index(std::int32_t key, const GuiTaggedValue& value) = 0;
};

// A reader visitor (vtable +18h). +0Ch reads with a default (six dwords), +10h
// without one (four); either leaves the field untouched when the key is absent
// and no default is given.
struct GuiPropertyReader {
    virtual ~GuiPropertyReader() = default;
    virtual void read_with_default(const char* name, GuiTaggedValue& field, const GuiTaggedValue& def) = 0;
    virtual void read(const char* name, GuiTaggedValue& field) = 0;
};

// What the describers need from the engine besides the visitor.
struct GuiDescribeHost {
    virtual ~GuiDescribeHost() = default;
    // Widget vtable +5Ch: the element kind; 1 is the page root.
    virtual std::int32_t element_kind(const void* widget) = 0;
    // 00AA5A00 + 00AAA480: the children sorted by case-insensitive name.
    virtual std::size_t sorted_children(const void* widget, std::array<const void*, 256>& out) = 0;
    // Child vtable +28h (the name C string) and +1Ch (describe into writer).
    virtual const char* child_name(const void* child) = 0;
    virtual void describe_child(const void* child, GuiPropertyWriter& writer) = 0;
    // The base describer of a derived class (for example 00AB2B70 for Progbar).
    virtual void describe_base(std::uint32_t native_address, const void* widget, GuiPropertyWriter& writer) = 0;
    // The base reader of a derived class (for example 00AB3310 for Progbar).
    virtual void read_base(std::uint32_t native_address, void* widget, GuiPropertyReader& reader) = 0;
};

// 00AAAED0 BSP_GuiWidget_DescribeProperties, __thiscall(widget, writer), RET 4.
// In order, each only when it differs from its default: Pos (+0Ch, Vec3, 0),
// Size (+20h, Vec2, 0), Pivot (+18h, Vec2, 0), Scale (+28h, Vec2, 1),
// Rotate (+48h, Float, 0), Color (+50h, 1,1,1,1), LowColor (+A4h, 0,0,0,1),
// HighColor (+B4h, 1,1,1,1), BlendFactor (+C4h, 0). The two colour defaults
// are function statics (00F8BCE0 under bit 0 of 00F8BCF0, 00F8BCCC under bit 0
// of 00F8BCDC). WideScreenAlign (+E0h) is written as "Left" (1) or "Right" (2),
// nothing for 0. Visible (+E4h, default true) only when vtable +5Ch != 1.
// MouseBlock (+84h) true is always written and then MouseHit is skipped;
// otherwise MouseHit (+78h, default false) when it differs. Then each child in
// name order: open_table(child name), child describe, close_table.
void gui_widget_describe_properties_00aaaed0(const void* widget, GuiPropertyWriter& writer,
    GuiDescribeHost& host);

// Progbar (cGuiProgressbar): +138h current value, +13Ch DefaultValue (both
// float). These lie past the Icon pool's 138h payload: see kGuiWidgetFactories.
inline constexpr std::uint32_t kGuiProgbarVtable = 0x00D5CC18;
inline constexpr float kGuiProgbarDefaultValue = 1.0f; // 00D7A24C

// 00AC7A90, __thiscall(progbar, reader), RET 4: the Icon reader 00AB3310, then
// DefaultValue (Float, default 1) into +13Ch, then +138h = +13Ch.
void gui_progbar_read_properties_00ac7a90(void* progbar, GuiPropertyReader& reader,
    GuiDescribeHost& host);
// 00AC7B00, __thiscall(progbar, writer), RET 4: the Icon describer 00AB2B70,
// then DefaultValue (+13Ch) when it differs from 1.
void gui_progbar_describe_properties_00ac7b00(const void* progbar, GuiPropertyWriter& writer,
    GuiDescribeHost& host);
// 00AC7820, __fastcall(ECX = progbar): FLD [ECX+138h]; RET (ST0 = value).
float gui_progbar_value_00ac7820(const void* progbar) noexcept;

// Scrollbar: +ECh value, +F0h MinValue, +F4h MaxValue, +F8h 1 = horizontal
// (Alignment other than "Vertical"), 0 = vertical.
inline constexpr std::uint32_t kGuiScrollbarVtable = 0x00D5CCF8;
// 00AC8770, __thiscall(scrollbar, writer), RET 4: base describer, then
// Alignment = "Vertical" (00D5CDA4) only when +F8h is 0, then MinValue and
// MaxValue when nonzero.
void gui_scrollbar_describe_properties_00ac8770(const void* scrollbar, GuiPropertyWriter& writer,
    GuiDescribeHost& host);

// Model: +F4h ScaleVector (Vec3), +100h RotationEuler (Vec3), +10Ch ModelName
// and +114h ModelTextureOverride (8-byte native strings: length, buffer).
// 00ACC4E0, __thiscall(model, writer), RET 4: base describer; ModelName always
// (an empty C string 00F8C0E4 for a null buffer); ModelTextureOverride when it
// differs from ""; ScaleVector when not (1,1,1); RotationEuler when not 0.
void gui_model_describe_properties_00acc4e0(const void* model, GuiPropertyWriter& writer,
    GuiDescribeHost& host);

// 00ACCE20, __thiscall(model, reader), RET 4: base reader 00AAA710; ModelName
// without a default (+10h), ModelTextureOverride with default "" (00CE3A0C),
// ScaleVector default (1,1,1), RotationEuler default 0; then 00ACCCF0
// BSP_GuiModel_LoadModel(+10Ch, +114h). The string reads go through the host
// because they assign native strings.
struct GuiModelReadHost {
    virtual ~GuiModelReadHost() = default;
    virtual void read_base_00aaa710(void* model, GuiPropertyReader& reader) = 0;
    virtual void read_string(void* model, std::uint32_t field, const char* name, const char* def_or_null,
        GuiPropertyReader& reader) = 0;
    virtual void load_model_00acccf0(void* model) = 0;
};
void gui_model_read_properties_00acce20(void* model, GuiPropertyReader& reader, GuiModelReadHost& host);

// ---------------------------------------------------------------------------
// Construction and leaf virtuals of Progbar, Scrollbar and Layer
// ---------------------------------------------------------------------------

struct GuiWidgetLifetimeHost {
    virtual ~GuiWidgetLifetimeHost() = default;
    // 00AA9390 widget base constructor, __thiscall(this, type id), RET 4.
    virtual void construct_widget_base_00aa9390(void* self, std::int32_t type_id) = 0;
    // 00AA9520 widget base copy constructor, __thiscall(this, source), RET 4.
    virtual void copy_widget_base_00aa9520(void* self, const void* source) = 0;
    // 00AB5C60 / 00AB5D30 Icon constructor and copy constructor.
    virtual void construct_icon_00ab5c60(void* self) = 0;
    virtual void copy_icon_00ab5d30(void* self, const void* source) = 0;
    // 00AA7170 BSP_GuiWidget_DeactivateAndRefreshBounds (the Icon loaded hook).
    virtual void loaded_base_00aa7170(void* self) = 0;
    // Progbar vtable +88h, __thiscall(this, int, int, float), RET 0Ch.
    virtual void progbar_virtual_88(void* self, std::int32_t a, std::int32_t b, float c) = 0;
};

// 00AC77C0, __fastcall(ECX = this), EAX = this: Icon constructor, vtable
// 00D5CC18, value +138h = 1.0, +60h = 5 (the type id). DefaultValue +13Ch is
// not written.
void* gui_progbar_construct_00ac77c0(void* self, GuiWidgetLifetimeHost& host);
// 00AC7840, __thiscall(this, source), RET 4: Icon copy, vtable, copies the
// value +138h only (DefaultValue +13Ch is not copied).
void* gui_progbar_copy_construct_00ac7840(void* self, const void* source, GuiWidgetLifetimeHost& host);
// 00AC78E0, vtable loaded hook: 00AA7170, then vtable +88h(0, 0, 1.0f).
void gui_progbar_on_loaded_00ac78e0(void* self, GuiWidgetLifetimeHost& host);

// 00AC7C20, __fastcall(ECX = this), EAX = this: base constructor with type 7,
// vtable 00D5CCF8, +ECh/+F0h/+F4h = 0.0f, +F8h = 1.
void* gui_scrollbar_construct_00ac7c20(void* self, GuiWidgetLifetimeHost& host);
// 00AC7CB0, __thiscall(this, source), RET 4: base copy, vtable, then the four
// fields +ECh..+F8h.
void* gui_scrollbar_copy_construct_00ac7cb0(void* self, const void* source, GuiWidgetLifetimeHost& host);
// 00AC7C90 (Scrollbar geometry virtual) and 00AC3FB0 (Layer attach virtual):
// a single RET.
inline void gui_widget_virtual_noop() noexcept {}

// 00AC3FA0, __fastcall(ECX = layer): LEA EAX,[ECX+100h]; RET. The layer name,
// an 8-byte native string (length, buffer).
void* gui_layer_name_00ac3fa0(void* layer) noexcept;
// 00AC57E0, __thiscall(layer, const name*), RET 4: self-assignment returns;
// otherwise 0041DD40 resize to the source length keeping contents, then, when
// the source length is nonzero, copy the destination length of bytes from the
// source buffer.
struct GuiNativeStringHost {
    virtual ~GuiNativeStringHost() = default;
    virtual void resize_0041dd40(void* header, std::uint32_t length, bool preserve) = 0;
};
void gui_layer_set_name_00ac57e0(void* layer, const void* name, GuiNativeStringHost& host);
// ---------------------------------------------------------------------------
// Widget base setters (segment 77), over GuiWidgetTransform (gui_widget.hpp)
// ---------------------------------------------------------------------------

// 00AA7910, __thiscall(widget, float z), RET 4: +14h = z, then 00AA7220.
void gui_widget_set_local_z_00aa7910(GuiWidgetTransform& w, float z, GuiWidgetTransformHost& host);
// 00AA7930, __thiscall(widget, float), RET 4: +48h Rotate, then 00AA7220.
void gui_widget_set_rotate_00aa7930(GuiWidgetTransform& w, float rotate, GuiWidgetTransformHost& host);
// 00AA7950, __thiscall(widget, const float[2]*), RET 4: +28h/+2Ch Scale through
// x87 FLD/FSTP, then 00AA7220.
void gui_widget_set_scale_00aa7950(GuiWidgetTransform& w, const float (&scale)[2], GuiWidgetTransformHost& host);
// 00AA7D40, __thiscall(widget, float z), RET 4: X and Y are stored back
// unchanged (an inlined position setter), Z = z, then 00AA7220 and 00AA70E0.
void gui_widget_set_local_z_and_bounds_00aa7d40(GuiWidgetTransform& w, float z, GuiWidgetTransformHost& host);
// 00AA7D80, __thiscall(widget, float x), RET 4: the position is stored back
// unchanged, 00AA7220, 00AA70E0, and only then +08h (the authored X the
// wide-screen fixup starts from) = x.
void gui_widget_set_authored_x_00aa7d80(GuiWidgetTransform& w, float x, GuiWidgetTransformHost& host);
// 00AA6820, __thiscall(widget, byte), RET 4: +75h, the flag that makes a
// visibility change stamp the whole node subtree (gui_widget_scene.hpp).
void gui_widget_set_visibility_recurses_00aa6820(void* widget, bool recurses) noexcept;
// 00AA6800, __fastcall(ECX = widget): MOV ECX,[ECX+4Ch]; JMP 00B6D800 (the
// scene node's name header).
struct GuiSceneNodeHost {
    virtual ~GuiSceneNodeHost() = default;
    virtual void* node_name_header_00b6d800(void* node) = 0;
};
void* gui_widget_node_name_00aa6800(void* widget, GuiSceneNodeHost& host);

// ---------------------------------------------------------------------------
// Sound, Model, Line and Curve members
// ---------------------------------------------------------------------------

struct GuiWidgetMemberHost {
    virtual ~GuiWidgetMemberHost() = default;
    virtual void construct_widget_base_00aa9390(void* self, std::int32_t type_id) = 0;
    virtual void copy_widget_base_00aa9520(void* self, const void* source) = 0;
    virtual void destroy_widget_base_00aa9730(void* self) = 0;
    // 00AA8320 BSP_GuiWidget_ReleaseSceneNodes (tail of Model ReleaseScene).
    virtual void release_scene_nodes_00aa8320(void* self) = 0;
    // 00AA7170 the base loaded hook, 00AA6A30 BSP_GuiWidget_SetActive(byte).
    virtual void loaded_base_00aa7170(void* self) = 0;
    virtual void set_active_00aa6a30(void* self, bool active) = 0;
    // A widget virtual by vtable offset with the given single pointer argument
    // or none (Line +50h with &this->+50h, Line +80h).
    virtual void call_virtual(void* self, std::uint32_t vtable_offset, void* argument) = 0;
    // The ref-counted object's vtable +0 when its count reaches zero.
    virtual void release_ref_object(void* object) = 0;
    // 00419CC0 + 00BD1510: return a native string buffer of (length + 1) bytes.
    virtual void release_string_buffer(void* buffer, std::uint32_t bytes) = 0;
    // 0079EE20, the std::vector copy constructor (ECX = destination, source).
    virtual void copy_vector_0079ee20(void* destination, const void* source) = 0;
    // 0086A350, reserve of the Mit qword vector {data, count, capacity}.
    virtual void reserve_qword_vector_0086a350(void* header, std::int32_t capacity) = 0;
};

// 00ACDB30, __fastcall(ECX = this), EAX = this: base(0Eh), vtable 00D5CFD0,
// +ECh..+104h (seven dwords) = 0.
void* gui_sound_construct_00acdb30(void* self, GuiWidgetMemberHost& host);

// 00ACC740, __fastcall(ECX = this), EAX = this: base(9), +ECh/+F0h = 0,
// vtable 00D5CF38, the two native strings +10Ch and +114h zeroed. ScaleVector
// +F4h and RotationEuler +100h are left to the reader.
void* gui_model_construct_00acc740(void* self, GuiWidgetMemberHost& host);
// 00ACC6A0, __fastcall(ECX = this): release the ref-counted scene object at
// +F0h (LOCK DEC of its +4h, vtable +0 at zero), clear +F0h and +ECh, tail-jump
// 00AA8320.
void gui_model_release_scene_00acc6a0(void* self, GuiWidgetMemberHost& host);
// 00ACC7B0, __fastcall(ECX = this): vtable 00D5CF38, release +F0h as above,
// return the +114h then +10Ch string buffers, base destructor 00AA9730.
void gui_model_destroy_00acc7b0(void* self, GuiWidgetMemberHost& host);

// 00AC2D00, __fastcall(ECX = this), EAX = this: base(4), vtable 00D5C9E8,
// +F0h..+FCh = 0 (the point vector and the committed count), +100h = 4.
void* gui_line_construct_00ac2d00(void* self, GuiWidgetMemberHost& host);
// 00AC2D60, __thiscall(this, source), RET 4: base copy, vtable, then the
// vector at +ECh copy-constructed from the source's (0079EE20). +FCh and
// +100h are not written.
void* gui_line_copy_construct_00ac2d60(void* self, const void* source, GuiWidgetMemberHost& host);
// 00AC1BB0, loaded hook: 00AA7170, vtable +50h(&+50h) (re-apply Color), then
// tail-jump vtable +80h (flush, 00AC29D0).
void gui_line_on_loaded_00ac1bb0(void* self, GuiWidgetMemberHost& host);

// Curve fields used here: +F8h widths (Mit float array {data, count,
// capacity}), +10Ch authored points (Mit qword vector), +11Ch..+128h UV rect.
// 00ABE030, __fastcall(ECX = this), EAX = this: base(0Dh), vtable 00D5C7F8,
// +ECh..+118h = 0, UV (0,0,1,1) at +11Ch..+128h, +12Ch and +134h..+13Ch = 0.
void* gui_curve_construct_00abe030(void* self, GuiWidgetMemberHost& host);
// 00ABC920, __fastcall(ECX = this): reserve(0) of +10Ch when its capacity is
// negative, count to 0, then 00818030 widths resize 0.
void gui_curve_clear_points_00abc920(void* self, GuiWidgetMemberHost& host);
// 00ABC970, __thiscall(this, const float[2]* point, float width), RET 8:
// append to +10Ch (grow to max(2*capacity, 1)) and the width to +F8h (same).
void gui_curve_add_point_00abc970(void* self, const float (&point)[2], float width, GuiWidgetMemberHost& host);
// 00ABC040, loaded hook: 00AA7170 then 00AA6A30(1).
void gui_curve_on_loaded_00abc040(void* self, GuiWidgetMemberHost& host);
// ---------------------------------------------------------------------------
// The std::sort instantiations over widget pointers (MSVC 9 <algorithm>)
// ---------------------------------------------------------------------------

// Two byte-identical instantiation sets sort std::vector<cGuiWidget*> by the
// case-insensitive name predicate (00AA2420 and its twin 00AA7810, reconstructed
// as name_less_00aa2420 in gui_manager_cc13.hpp; the predicate is inlined into
// the partition and heap routines). The routines below are the VS2008 library
// algorithms in the instantiations' operation order, so equal-name elements end
// up in the same permutation as in the image. Iterators are raw pointers here
// (the instantiations take them in registers: ECX = first, EDX = second).
//   routine              set A (00AA5A00)   set B (00AAAED0)
//   _Med3                00AA2A30           00AA7BF0   ECX,EDX,[esp+4] RET 8
//   _Push_heap           00AA2A90           00AA7C50
//   _Median              00AA2D90           00AA8080
//   _Adjust_heap         00AA2E30           00AA8120
//   _Unguarded_partition 00AA34A0           00AA8FB0
//   _Make_heap           00AA37D0           00AA92E0
//   _Sort_heap           00AA4550           00AAA260
//   _Insertion_sort      00AA2C80           00AA7F70
//   _Sort (introsort)    00AA4E00           00AAA480
// The _ISORT_MAX threshold is 32 elements and _Ideal starts at the count.
namespace msvc9_sort {

template <class T, class Pred>
void med3(T* first, T* mid, T* last, Pred& pred) {
    if (pred(*mid, *first)) std::swap(*mid, *first);
    if (pred(*last, *mid)) std::swap(*last, *mid);
    if (pred(*mid, *first)) std::swap(*mid, *first);
}

// last is inclusive (the partition passes last - 1).
template <class T, class Pred>
void median(T* first, T* mid, T* last, Pred& pred) {
    if (40 < last - first) {
        const std::ptrdiff_t step = (last - first + 1) / 8;
        med3(first, first + step, first + 2 * step, pred);
        med3(mid - step, mid, mid + step, pred);
        med3(last - 2 * step, last - step, last, pred);
        med3(first + step, mid, last - step, pred);
    } else {
        med3(first, mid, last, pred);
    }
}

template <class T, class Pred>
void push_heap(T* first, std::ptrdiff_t hole, std::ptrdiff_t top, T value, Pred& pred) {
    for (std::ptrdiff_t idx = (hole - 1) / 2; top < hole && pred(first[idx], value); idx = (hole - 1) / 2) {
        first[hole] = first[idx];
        hole = idx;
    }
    first[hole] = value;
}

template <class T, class Pred>
void adjust_heap(T* first, std::ptrdiff_t hole, std::ptrdiff_t bottom, T value, Pred& pred) {
    const std::ptrdiff_t top = hole;
    std::ptrdiff_t idx = 2 * hole + 2;
    for (; idx < bottom; idx = 2 * idx + 2) {
        if (pred(first[idx], first[idx - 1])) --idx;
        first[hole] = first[idx];
        hole = idx;
    }
    if (idx == bottom) {
        first[hole] = first[bottom - 1];
        hole = bottom - 1;
    }
    push_heap(first, hole, top, value, pred);
}

template <class T, class Pred>
void make_heap(T* first, T* last, Pred& pred) {
    const std::ptrdiff_t bottom = last - first;
    for (std::ptrdiff_t hole = bottom / 2; 0 < hole;) {
        --hole;
        adjust_heap(first, hole, bottom, first[hole], pred);
    }
}

template <class T, class Pred>
void sort_heap(T* first, T* last, Pred& pred) {
    for (; 1 < last - first; --last) {
        T value = last[-1];
        last[-1] = *first;
        adjust_heap(first, std::ptrdiff_t{0}, (last - 1) - first, value, pred);
    }
}

// 00AA1E60 STL_rotate_ptr, __fastcall(ECX = first, EDX = mid, [esp+4] = last):
// the VS2008 random-access rotate (gcd cycles, highest cycle start first).
template <class T>
void rotate(T* first, T* mid, T* last) {
    const std::ptrdiff_t shift = mid - first;
    const std::ptrdiff_t count = last - first;
    std::ptrdiff_t factor = count;
    for (std::ptrdiff_t s = shift; s != 0;) {
        const std::ptrdiff_t remainder = factor % s;
        factor = s;
        s = remainder;
    }
    if (factor < count) {
        for (; 0 < factor; --factor) {
            T* const start = first + factor;
            T* next = start;
            T value = *start;
            T* next1 = next + shift;
            if (next1 == last) next1 = first;
            while (next1 != start) {
                *next = *next1;
                next = next1;
                if (shift < last - next1) next1 += shift;
                else next1 = first + (shift - (last - next1));
            }
            *next = value;
        }
    }
}

// The instantiations compare the element still at `next` (not a copy) and move
// it into place with rotate(hole, next, next + 1); the rotate's own
// (first != mid && mid != last) guard is inlined at the call.
template <class T, class Pred>
void insertion_sort(T* first, T* last, Pred& pred) {
    if (first == last) return;
    for (T* next = first; ++next != last;) {
        T* hole = next;
        if (pred(*next, *first)) {
            hole = first;
        } else {
            while (pred(*next, hole[-1])) --hole;
        }
        if (hole != next) rotate(hole, next, next + 1);
    }
}

template <class T, class Pred>
std::pair<T*, T*> unguarded_partition(T* first, T* last, Pred& pred) {
    T* mid = first + (last - first) / 2;
    median(first, mid, last - 1, pred);
    T* pfirst = mid;
    T* plast = pfirst + 1;
    while (first < pfirst && !pred(pfirst[-1], *pfirst) && !pred(*pfirst, pfirst[-1])) --pfirst;
    while (plast < last && !pred(*plast, *pfirst) && !pred(*pfirst, *plast)) ++plast;
    T* gfirst = plast;
    T* glast = pfirst;
    for (;;) {
        for (; gfirst < last; ++gfirst) {
            if (pred(*pfirst, *gfirst)) continue;
            if (pred(*gfirst, *pfirst)) break;
            std::swap(*plast++, *gfirst);
        }
        for (; first < glast; --glast) {
            if (pred(glast[-1], *pfirst)) continue;
            if (pred(*pfirst, glast[-1])) break;
            std::swap(*--pfirst, glast[-1]);
        }
        if (glast == first && gfirst == last) return {pfirst, plast};
        if (glast == first) {
            if (plast != gfirst) std::swap(*pfirst, *plast);
            ++plast;
            std::swap(*pfirst++, *gfirst++);
        } else if (gfirst == last) {
            if (--glast != --pfirst) std::swap(*glast, *pfirst);
            std::swap(*pfirst, *--plast);
        } else {
            std::swap(*gfirst++, *--glast);
        }
    }
}

template <class T, class Pred>
void sort(T* first, T* last, std::ptrdiff_t ideal, Pred& pred) {
    std::ptrdiff_t count;
    for (; 32 < (count = last - first) && 0 < ideal;) {
        const auto mid = unguarded_partition(first, last, pred);
        ideal /= 2, ideal += ideal / 2;
        if (mid.first - first < last - mid.second) {
            sort(first, mid.first, ideal, pred);
            first = mid.second;
        } else {
            sort(mid.second, last, ideal, pred);
            last = mid.first;
        }
    }
    if (32 < count) {
        if (1 < last - first) make_heap(first, last, pred);
        sort_heap(first, last, pred);
    } else if (1 < count) {
        insertion_sort(first, last, pred);
    }
}

} // namespace msvc9_sort

// The element and predicate the two instantiation sets use: widget pointers,
// names through each widget's vtable +28h (an 8-byte native string), compared
// by name_less_00aa2420 (gui_manager_cc13.hpp).
struct GuiWidgetNameSource {
    virtual ~GuiWidgetNameSource() = default;
    virtual std::string name(const void* widget) = 0;
};
struct GuiWidgetNameLess {
    GuiWidgetNameSource* names;
    bool operator()(const void* left, const void* right) const;
};
// _Sort(first, last, last - first, pred), as 00AA5A00 and 00AAAED0 call it.
void sort_widgets_by_name(const void** first, const void** last, GuiWidgetNameSource& names);

// ---------------------------------------------------------------------------
// Sound, Grid, AnimIcon, Curve-copy and Button members
// ---------------------------------------------------------------------------

struct GuiWidgetCopyHost {
    virtual ~GuiWidgetCopyHost() = default;
    // 00ACD8E0 / 00ACD9E0: the two ref-pointer copy-assignments of Sound,
    // __fastcall(ECX = destination field, EDX = source field).
    virtual void copy_sound_ref_00acd8e0(void* destination, const void* source) = 0;
    virtual void copy_sound_ref_00acd9e0(void* destination, const void* source) = 0;
    // Sound +F0h object vtable +8h(0), called before it is released.
    virtual void stop_sound_instance(void* instance) = 0;
    // Grid helpers: 00ACB9B0(this, 0), 00ACA730 (destroy [begin, end) of the
    // column records), 00ACB290 (column vector copy, ECX = destination).
    virtual void grid_clear_00acb9b0(void* grid) = 0;
    virtual void grid_destroy_columns_00aca730(void* begin, void* end, void* vector) = 0;
    virtual void grid_copy_columns_00acb290(void* destination, const void* source) = 0;
    // 00AA7E00 BSP_GuiWidget_FindChildByName(this, name, 1).
    virtual void* find_child_00aa7e00(void* widget, const char* name) = 0;
    // 00BF65AC operator delete.
    virtual void free_block(void* block) = 0;
};

// Sound: +ECh and +F0h ref pointers, +F4h a dword, +F8h SoundEfx and +100h
// Category (8-byte native strings).
// 00ACDBA0, __thiscall(this, source), RET 4. NOTE: it runs the DEFAULT base
// constructor 00AA9390(0Eh), not the base copy, so position, size and the other
// base fields are not copied; nor are the two strings. Only +F4h and the two
// ref pointers come from the source.
void* gui_sound_copy_construct_00acdba0(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiWidgetCopyHost& copy);
// 00ACDC40, __fastcall(ECX = this): vtable 00D5CFD0, release +ECh, stop then
// release +F0h, return the +100h and +F8h string buffers, base destructor.
void gui_sound_destroy_00acdc40(void* self, GuiWidgetMemberHost& host, GuiWidgetCopyHost& copy);

// Grid (hud_widgets.hpp GuiGridState): column vector +ECh..+F8h.
// 00ACB0A0, __fastcall(ECX = this), EAX = this: base(8), vtable 00D5CE60,
// +F0h..+100h = 0, CellSize 0, +10Ch = 1, scroll 0, Visual_Group 0, content 0.
void* gui_grid_construct_00acb0a0(void* self, GuiWidgetMemberHost& host);
// 00ACB3F0, __thiscall(this, source), RET 4: base copy, vtable, columns copied
// (00ACB290), CellSize/+10Ch/scroll/content copied, then +118h re-resolved as
// the copy's own child named "Visual_Group" (00D5CE24) through 00AA7E00.
void* gui_grid_copy_construct_00acb3f0(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiWidgetCopyHost& copy);
// 00ACBBF0, __fastcall(ECX = this): vtable, 00ACB9B0(0), destroy and free the
// column vector, base destructor.
void gui_grid_destroy_00acbbf0(void* self, GuiWidgetMemberHost& host, GuiWidgetCopyHost& copy);

// 00AA1C60, __thiscall(this, source), RET 4: Icon copy 00AB5D30, vtable
// 00D5BCE0, FPS +138h, the dword +13Ch and the three flags +140h..+142h
// (PlaybyDefault, Loop, Forward).
void* gui_anim_icon_copy_construct_00aa1c60(void* self, const void* source, GuiWidgetLifetimeHost& host);

// 00ABE100, __thiscall(this, source), RET 4: base copy, then exactly the
// default constructor's Curve fields. The copy has no points and the default
// UV rectangle.
void* gui_curve_copy_construct_00abe100(void* self, const void* source, GuiWidgetMemberHost& host);

// Button: +ECh/+F0h dwords, +F4h an 8-byte native string. Every Button built by
// the default constructor is appended to the global std::list<cGuiButton*> at
// 00F8BD38 (head node 00F8BD3C, size 00F8BD40) and removed by the destructor.
struct GuiButtonRegistry {
    virtual ~GuiButtonRegistry() = default;
    // 00AAC210 buy node + 00AAC5D0 size check + link: push_back(button).
    virtual void push_back(void* button) = 0;
    // 00AAC310 std::list::remove(button).
    virtual void remove(void* button) = 0;
};
// 00AAC6F0, __fastcall(ECX = this), EAX = this: base(0Fh), vtable 00D5BED0,
// +ECh..+F8h = 0, then registry push_back.
void* gui_button_construct_00aac6f0(void* self, GuiWidgetMemberHost& host, GuiButtonRegistry& registry);
// 00AA3C60, __thiscall(this, source), RET 4: base copy, vtable, +ECh/+F0h
// copied, the +F4h string copied like 00AC57E0. The copy is NOT registered.
void* gui_button_copy_construct_00aa3c60(void* self, const void* source, GuiWidgetMemberHost& host,
    GuiNativeStringHost& strings);
// 00AAC530, __fastcall(ECX = this): vtable, registry remove, then the global
// dword 00F8BCF8 is cleared whenever it is nonzero (it is not compared with
// this button), the string buffer is returned, base destructor.
void gui_button_destroy_00aac530(void* self, GuiWidgetMemberHost& host, GuiButtonRegistry& registry,
    std::uint32_t& global_00f8bcf8);

// ---------------------------------------------------------------------------
// Icon (cGuiIcon, vtable 00D5C4C0; layout in include/bsp/gui_icon.hpp)
// ---------------------------------------------------------------------------

// Icon fields used here: +ECh current state (int16), +F0h..+FCh the
// std::vector of 40h-byte state records (first +F4h, last +F8h), +100h
// HasTexture, +101h DelayedTextureLoad, +104h ShaderName (8-byte native
// string), +10Ch DynamicVB, +110h PartialDisplayType (int), +114h
// PartialDisplayRatio, +118h..+124h crop, +128h/+12Ch temporary state, +130h
// AutoRotate, +134h cached filter flag. State record: +4h texture name (native
// string), +1Ch..+28h UV_LURB, +2Ch Pivot, +34h Size.
inline constexpr std::uint32_t kGuiIconVtable = 0x00D5C4C0;
inline constexpr GuiValueTag kGuiValueTagInt = static_cast<GuiValueTag>(1); // PartialDisplayType

struct GuiIconDescribeHost {
    virtual ~GuiIconDescribeHost() = default;
    // 00AB27A0, __thiscall(icon, float out[2], state index): the state's texture size.
    virtual std::array<float, 2> state_texture_size_00ab27a0(const void* icon, std::int32_t state) = 0;
};

// 00AB2B70, __thiscall(icon, writer), RET 4. When the icon has exactly one
// state, the current state is 0 and the widget Size (+20h/+24h) equals that
// state's texture size, Size is zeroed around the base describer 00AAAED0 (so
// the default-size icon does not serialise Size) and restored after. Then,
// each when it differs: DynamicVB (false), HasTexture (true),
// PartialDisplayType (Int 0), PartialDisplayRatio (1), DelayedTextureLoad
// (false), AutoRotate (0); ShaderName only when its length is nonzero, against
// "GuiDefault.mshd" (00D5C3D8). With states: open "States", and per state i an
// integer-keyed table i+1 holding Texture (default ""), Size (default the
// WIDGET's Size), Pivot (default the widget's Pivot) and, only when the UV is
// not (0,0,1,1), a "UV_LURB" table of four Floats keyed 1..4.
void gui_icon_describe_properties_00ab2b70(void* icon, GuiPropertyWriter& writer, GuiDescribeHost& host,
    GuiIconDescribeHost& icon_host);

struct GuiIconLifetimeHost {
    virtual ~GuiIconLifetimeHost() = default;
    virtual void copy_widget_base_00aa9520(void* self, const void* source) = 0;
    virtual void destroy_widget_base_00aa9730(void* self) = 0;
    // 00AB58F0 vector<state> copy (ECX = destination +F0h, source +F0h);
    // 00AB5850 vector<state> destructor (ECX = +F0h).
    virtual void copy_states_00ab58f0(void* destination, const void* source) = 0;
    virtual void destroy_states_00ab5850(void* states) = 0;
    // Icon vtable +80h (00AB3CB0 rebuild) with the current state index.
    virtual void rebuild_00ab3cb0(void* icon, std::int32_t state) = 0;
    // 00AB5700 singleton + 00AB50E0 cancel_owner(icon).
    virtual void cancel_delayed_loads(void* icon) = 0;
    // 00419CC0 + 00BD1510.
    virtual void release_string_buffer(void* buffer, std::uint32_t bytes) = 0;
};

// 00AB5D30, __thiscall(this, source), RET 4: base copy, vtable 00D5C4C0, then
// +ECh, the state vector (00AB58F0), +100h, +101h, +10Ch..+134h copied, but
// ShaderName (+104h/+108h) is set EMPTY rather than copied; finally vtable
// +80h(current state).
void* gui_icon_copy_construct_00ab5d30(void* self, const void* source, GuiIconLifetimeHost& host);
// 00AB5E70, __fastcall(ECX = this): vtable, cancel pending delayed loads,
// return the ShaderName buffer, destroy the state vector, base destructor.
void gui_icon_destroy_00ab5e70(void* self, GuiIconLifetimeHost& host);

// The delayed-load registry singleton at 00F8BD98 (GuiIconDelayedLoads in
// gui_render.hpp): 10h bytes, vtable 00D5C4B8, entry array at +4h.
struct GuiIconDelayedLoadsHost {
    virtual ~GuiIconDelayedLoadsHost() = default;
    // 00415350 lifetime manager: its lock (+10h, EnterCriticalSection with a
    // +18h recursion count) and 00BD0C30 registration of a new singleton.
    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual void register_singleton(GuiIconDelayedLoads& singleton) = 0;
};
// 00AB5700, cdecl, EAX = the singleton: double-checked creation under the
// lifetime-manager lock, then registration.
GuiIconDelayedLoads& gui_icon_delayed_loads_00ab5700(GuiIconDelayedLoads*& global_00f8bd98,
    GuiIconDelayedLoadsHost& host);
// 00AB5560, __fastcall(ECX = singleton): clear the entry array (00AB4850(0)),
// free its block, clear 00F8BD98, restore the base vtable 00CE3818.
void gui_icon_delayed_loads_destroy_00ab5560(GuiIconDelayedLoads& singleton,
    GuiIconDelayedLoads*& global_00f8bd98);
// 00AB58D0, cdecl(name, data), RET 8: singleton()->deliver(00AB51B0)(name,
// data). Returns the owners deliver_00ab51b0 hands to 00AB3840.
std::vector<const void*> gui_icon_delayed_loads_on_file_loaded_00ab58d0(const std::string& name,
    GuiIconDelayedLoads*& global_00f8bd98, GuiIconDelayedLoadsHost& host);

// ---------------------------------------------------------------------------
// Movie, FrameBox, AnimIcon and manager leaves
// ---------------------------------------------------------------------------

// 00AAC7F0, __fastcall(ECX = movie): 00AA7DC0 with a zero position on the
// stack, i.e. position (0,0,0), recompose 00AA7220, bounds 00AA70E0.
void gui_movie_reset_position_00aac7f0(GuiWidgetTransform& movie, GuiWidgetTransformHost& host);

struct GuiMovieHost {
    virtual ~GuiMovieHost() = default;
    // Movie vtable +78h (the loaded hook; it creates the player at +F0h).
    virtual void loaded_virtual_78(void* movie) = 0;
    // The material of the widget's scene node: 00B732C0(00B74640(node, 0, 0)).
    virtual void* node_material(void* node) = 0;
    // Player (+F0h) vtable +38h(slot) -> texture; vtable +10h().
    virtual void* player_texture(void* player, std::uint32_t slot) = 0;
    virtual void player_virtual_10(void* player) = 0;
    // 00B189F0 BSP_Material_SetTextureSlot, ECX = material +20h.
    virtual void set_texture_slot_00b189f0(void* sampler_block, std::uint32_t slot, void* texture) = 0;
};
// 00AAC820, __fastcall(ECX = movie): when the player +F0h is null run vtable
// +78h first; then for slots 0, 1, 2 (re-fetching the node material each time)
// bind player texture `slot` into material+20h slot `slot`.
void gui_movie_bind_video_textures_00aac820(void* movie, GuiMovieHost& host);
// 00AAC8C0, __fastcall(ECX = movie): when +F0h is non-null, tail-call its
// vtable +10h.
void gui_movie_forward_player_10_00aac8c0(void* movie, GuiMovieHost& host);

// 00AD3A20 timed-entry base kind: MOV EAX, 2; RET.
inline std::int32_t gui_timed_entry_base_kind_00ad3a20() noexcept { return 2; }

struct GuiNodeGeometryHost {
    virtual ~GuiNodeGeometryHost() = default;
    // 00B74640(node, 0), 00B72B20 (stream count), 00B73260(geometry, 0).
    virtual void* node_geometry_00b74640(void* node) = 0;
    virtual std::int32_t stream_count_00b72b20(void* geometry) = 0;
    virtual void* stream_00b73260(void* geometry, std::uint32_t index) = 0;
};
// 00ACEB00, __fastcall(ECX = framebox): the first vertex stream of the node
// +4Ch's geometry, or null when the geometry or its streams are missing.
// With a null node the native returns whatever EAX held on entry (EAX is not
// written on that path); this returns null there.
void* gui_framebox_stream_00aceb00(void* framebox, GuiNodeGeometryHost& host);
// 00AD2720, __fastcall(ECX = framebox): the count of 2Ch-byte state records in
// the vector +F0h..+FCh (signed division of last - first by 2Ch; 0 when first
// is null).
std::int32_t gui_framebox_state_count_00ad2720(const void* framebox) noexcept;

// 00AD2FF0, AnimIcon loaded hook: 00AB10F0 (Icon reset to first state); when
// Forward (+142h) is false, vtable +88h(states - 1, 0, 1.0f), i.e. start on the
// last state.
struct GuiAnimIconHost {
    virtual ~GuiAnimIconHost() = default;
    virtual void icon_reset_00ab10f0(void* icon) = 0;
    virtual void select_state_virtual_88(void* icon, std::int32_t state, std::int32_t mode, float blend) = 0;
};
void gui_anim_icon_loaded_00ad2ff0(void* icon, GuiAnimIconHost& host);

// 00AA2B30 BSP_GUI_PreloadTexture, cdecl-like __stdcall(name*), RET 4:
// 00AA2660(name, uv (0,0,1,1), size, 1.0f) and the returned reference is
// dropped, which keeps the texture loaded. The native size pair is an
// uninitialized stack slot; nothing reads it afterwards.
void* gui_preload_texture_00aa2b30(std::string_view name, const GuiTextureCallbacks& callbacks);

// 00AA59B0, __thiscall(manager, entry*), RET 4: 00AAA5A0(manager+44h group,
// entry+8h widget, 0), then push_back of the entry into the std::list at
// manager+34h (00AA4D30 buy node, 00AA43A0 size check, link before the head).
struct GuiHoistHost {
    virtual ~GuiHoistHost() = default;
    virtual void attach_child_00aaa5a0(void* group, void* widget, std::int32_t index) = 0;
    virtual void list_push_back(void* list, const void* entry) = 0;
};
void gui_manager_adopt_hoisted_widget_00aa59b0(void* manager, const void* entry, GuiHoistHost& host);

// 00AA6EB0 / 00AACD10, __fastcall(ECX = owner slot): when *slot is non-null
// call its vtable +0 (00AA6EB0) or +8 (00AACD10) with 1 (delete), then null it.
struct GuiVirtualDeleteHost {
    virtual ~GuiVirtualDeleteHost() = default;
    virtual void call_virtual_with_1(void* object, std::uint32_t vtable_offset) = 0;
};
void gui_delete_owned_object(void** slot, std::uint32_t vtable_offset, GuiVirtualDeleteHost& host);

// 00AA3480, __fastcall(ECX = native string header): when the buffer is
// non-null, return length + 1 bytes to the sized pool (00419CC0/00BD1510).
void gui_native_string_release_00aa3480(void* header, GuiWidgetMemberHost& host);

// 00AA57A0, __fastcall(ECX = list): 00AA51D0 clear, then operator delete of the
// head node (+4h) and +4h = 0.
struct GuiListHost {
    virtual ~GuiListHost() = default;
    virtual void clear_00aa51d0(void* list) = 0;
    virtual void free_block(void* block) = 0;
};
void gui_string_list_destroy_00aa57a0(void* list, GuiListHost& host);

// ---------------------------------------------------------------------------
// Movie subtitle containers
// ---------------------------------------------------------------------------

// Movie: +104h a byte (subtitle shown), +108h the subtitle Text widget, +10Ch
// the std::vector of 14h-byte subtitle records (first +110h, last +114h),
// +120h the std::vector of 24h-byte "extratitle" records (first +124h, last
// +128h; each holds an owned object at +18h). Both record kinds start with an
// 8-byte native string.
struct GuiMovieSubtitleHost {
    virtual ~GuiMovieSubtitleHost() = default;
    // 00ABBE50 BSP_GuiText_SetLocalisedCString(ECX = text, "", 0).
    virtual void text_set_localised_00abbe50(void* text, const char* value, bool localise) = 0;
    // The vector members of the two record types: insert n copies before
    // `where` (00AAF820 / 00AAFB20) and erase [first, last) (00AAF580 /
    // 00AAF620). record_bytes selects the type (14h or 24h).
    virtual void insert_n(void* vector, std::uint32_t record_bytes, void* where, std::uint32_t count,
        const void* value) = 0;
    virtual void erase(void* vector, std::uint32_t record_bytes, void* first, void* last) = 0;
    // The extratitle clear tail: 00AAE130 range copy of [last, last) onto first,
    // 00AAF120 destroy of the vacated range, last = first.
    virtual void clear_extratitles_tail(void* vector) = 0;
    // Owned object vtable +4h(1).
    virtual void delete_object_virtual_4(void* object) = 0;
    // 00419CC0 + 00BD1510 (the by-value record's string).
    virtual void release_string_buffer(void* buffer, std::uint32_t bytes) = 0;
};

// 00AB00C0, __fastcall(ECX = movie): hide (+104h = 0), blank the subtitle
// text (when present) and forget it, erase every subtitle, delete each
// extratitle's +18h object (nulling the slot), then clear the extratitles.
void gui_movie_clear_subtitles_00ab00c0(void* movie, GuiMovieSubtitleHost& host);

// 00AB0220 (14h records) and 00AB0350 (24h records): std::vector::resize(n,
// value), __thiscall(vector, n, value by value), RET 4 + sizeof(value). Grows
// with insert_n at the end, shrinks with erase(first + n, last); the by-value
// record's leading native string is released on exit. The ledger called them
// Push*Record; they are resize.
void gui_movie_resize_records(void* vector, std::uint32_t record_bytes, std::uint32_t count, const void* value,
    GuiMovieSubtitleHost& host);

// ---------------------------------------------------------------------------
// Readers with sub-tables: Line, Group, Curve
// ---------------------------------------------------------------------------

// The table navigation half of the reader visitor (+04h open, +08h close,
// +10h read without default, +14h has-key), with named or integer keys.
struct GuiTableReader {
    virtual ~GuiTableReader() = default;
    virtual bool has(const char* name) = 0;
    virtual bool has_index(std::int32_t key) = 0;
    virtual void open(const char* name) = 0;
    virtual void close() = 0;
    virtual void read_index(std::int32_t key, GuiTaggedValue& value) = 0;
    virtual void read_string(const char* name, void* native_string_header) = 0;
};

struct GuiReaderHost {
    virtual ~GuiReaderHost() = default;
    virtual void read_base_00aaa710(void* widget, GuiTableReader& reader) = 0;
    // 004222C0 std::vector<float[2]>::push_back (ECX = vector).
    virtual void push_back_vec2_004222c0(void* vector, const float (&value)[2]) = 0;
    // 0043B920 range copy used to erase [first, last) of the +130h vector.
    virtual void clear_vec2_vector(void* vector) = 0;
    // 00ABDD80 BSP_GuiCurve_SetTexture(ECX = curve, name header).
    virtual void curve_set_texture_00abdd80(void* curve, void* name_header) = 0;
    // Curve vtable +7Ch (tessellate, 00ABCED0).
    virtual void curve_virtual_7c(void* curve) = 0;
    // 00BF6713, the checked-iterator failure (_invalid_parameter path).
    virtual void checked_iterator_failure_00bf6713() = 0;
};

// 00AC2DE0, __thiscall(line, reader), RET 4: base reader; when "Points"
// (00CFD91C) exists, open it and append Vec2 entries under keys 1, 2, ... to
// the point vector at +ECh until a key is missing; close.
void gui_line_read_properties_00ac2de0(void* line, GuiTableReader& reader, GuiReaderHost& host);
// 00AC6FD0 (no Ghidra function; 5 bytes): JMP 00AAA710.
void gui_group_read_properties_00ac6fd0(void* group, GuiTableReader& reader, GuiReaderHost& host);

// 00ABE1D0, __thiscall(curve, reader), RET 4. Base reader; erase the authored
// point vector (+130h, first +134h, last +138h) and resize the widths (+F8h)
// to 0; when "PosArray" exists read Vec3 entries 1, 2, ...: XY appended to
// +130h, Z appended to the widths (grow to max(2*capacity, 1)). Then, when
// fewer than two points were authored (`cmp eax,2; jae` at 00ABE349), the
// routine starts building the padded control array at +ECh from points [0]
// and [1] and the checked iterators fail (00BF6713) for 0 or 1 points; with
// two or more points it skips straight on. Finally Texture (+104h, no
// default), 00ABDD80, and vtable +7Ch.
void gui_curve_read_properties_00abe1d0(void* curve, GuiTableReader& reader, GuiReaderHost& host);

// ---------------------------------------------------------------------------
// Named entries, timed entries, FrameBox destructor
// ---------------------------------------------------------------------------

// The 0Ch-byte {native string name, dword value} entry of the manager's
// hoisted-widget list (value = widget) and of string/value maps.
// 00AA3D30, __thiscall(entry, const entry* source), RET 4: zero the header;
// unless copying onto itself, resize to the source length (0041DD40, keep) and
// copy the destination length of bytes when the source is non-empty; the value
// +8h is copied in every case (so a self-copy keeps the value but loses the
// string, whose buffer is abandoned, not freed).
void* gui_named_entry_copy_00aa3d30(void* entry, const void* source, GuiNativeStringHost& strings);
// 00AA9A20, __thiscall(entry, const native string* name, dword value), RET 8:
// the same string copy from `name`, then +8h = value.
void* gui_named_entry_construct_00aa9a20(void* entry, const void* name, std::uint32_t value,
    GuiNativeStringHost& strings);
// 00AA44D0 (__fastcall(ECX = slot, EDX = source)) and 00AA4AB0 (__stdcall(slot,
// source), RET 8): placement construction, 00AA3D30 when the slot is non-null.
void gui_named_entry_place_copy(void* slot, const void* source, GuiNativeStringHost& strings);

// 00AA8B80, __thiscall(widget, int index), RET 4: the timed-entry pointer
// array at +88h (data +88h, count +8Ch). Nothing when count < index; when index
// == count the array is first grown to index + 1 (00AA77B0, new slot null);
// then a non-null entry is deleted through its vtable +0 (flag 1) and the slot
// cleared.
struct GuiTimedEntryHost {
    virtual ~GuiTimedEntryHost() = default;
    virtual void resize_pointer_array_00aa77b0(void* array, std::int32_t count) = 0;
    virtual void delete_entry(void* entry) = 0;
    // 00BF6989 free of the array block.
    virtual void free_block(void* block) = 0;
};
void gui_widget_delete_timed_entry_00aa8b80(void* widget, std::int32_t index, GuiTimedEntryHost& host);
// 00AA7BD0, __fastcall(ECX = array): 00AA77B0(0), then free the data block.
void gui_timed_entry_array_destroy_00aa7bd0(void* array, GuiTimedEntryHost& host);

// 00AD2880, __fastcall(ECX = framebox): vtable 00D5D130, return the +104h
// string buffer, destroy the 2Ch-record vector at +F0h (00AD21B0), base
// destructor.
struct GuiFrameBoxHost {
    virtual ~GuiFrameBoxHost() = default;
    virtual void destroy_state_vector_00ad21b0(void* vector) = 0;
};
void gui_framebox_destroy_00ad2880(void* framebox, GuiWidgetMemberHost& host, GuiFrameBoxHost& framebox_host);

// ---------------------------------------------------------------------------
// AnimIcon update (vtable +40h)
// ---------------------------------------------------------------------------

// AnimIcon fields: +138h FPS (float), +13Ch the last step time in ms (dword,
// converted unsigned), +140h PlaybyDefault (playing), +141h Loop, +142h
// Forward; the Icon current state at +ECh (int16) and states at +F0h.
struct GuiAnimIconUpdateHost {
    virtual ~GuiAnimIconUpdateHost() = default;
    // 00AB1150 BSP_GuiIcon_Update(seconds).
    virtual void icon_update_00ab1150(void* icon, float seconds) = 0;
    // [01090AB0] vtable +14h: the clock's {counter, frequency} int64 pair.
    virtual std::array<std::int64_t, 2> clock_counter_and_frequency() = 0;
    // Icon vtable +88h(state, 0, 1.0f).
    virtual void select_state_virtual_88(void* icon, std::int16_t state, std::int32_t mode, float blend) = 0;
};

// 00AD3040, __thiscall(icon, float seconds), RET 4 (x87 read from the bytes:
// DE F2 = FDIVRP ST(2),ST(0), DE C2 = FADDP ST(2),ST(0), DE F9 = FDIVP
// ST(1),ST(0)). Icon update first; nothing more unless playing with at least
// one state. now = float(1000.0 * float(counter / frequency)); when now is
// greater than 1000.0 / FPS + (unsigned)last (kept in x87 precision, not
// rounded), last = trunc(float(float(counter / frequency) * 1000.0)) (FISTP
// under a chop control word) and the state steps: forward from the last state
// wraps to 0 only with Loop; backward from 0 wraps to the last state only with
// Loop; otherwise current +/- 1. Intermediates are double here, which is the
// x87 result under the 53-bit precision control the game runs with
// (docs/X87_CONTROL_WORD.md); an extended-precision run could differ in the
// last bit of the comparison.
void gui_anim_icon_update_00ad3040(void* icon, float seconds, GuiAnimIconUpdateHost& host);

// 00AD2A80 FrameBox vtable +8Ch, __thiscall(framebox, const record* r, int16
// index), RET 8. LOCK INC of the count at (r->+0h)+4h (the record's first
// field is a ref-counted object; the increment is not balanced here), then
// index == count: push_back (00AD29D0); otherwise records[index] = *r
// (00AD0D00, index checked: 00BF6713 when out of range). Finally, when the
// current state (+ECh, int16) is that index, vtable +88h with no arguments.
struct GuiFrameBoxRecordHost {
    virtual ~GuiFrameBoxRecordHost() = default;
    virtual void push_back_record_00ad29d0(void* vector, const void* record) = 0;
    virtual void assign_record_00ad0d00(void* element, const void* record) = 0;
    virtual void checked_iterator_failure_00bf6713() = 0;
    virtual void framebox_virtual_88(void* framebox) = 0;
};
void gui_framebox_insert_state_record_00ad2a80(void* framebox, const void* record, std::int16_t index,
    GuiFrameBoxRecordHost& host);

// 00AC8890 Scrollbar vtable +78h (loaded hook), __fastcall(ECX = scrollbar):
// looks up "_BackArrow_Icon", "_ForwArrow_Icon", "_Background_Icon" and
// "_Slider_Icon" with 00AAA2B0 and DISCARDS each result (EAX is not stored;
// each temporary native string is returned to the pool), then
// 00AC7D00 SetValue(MinValue +F0h) and 00AA7170.
struct GuiScrollbarLoadHost {
    virtual ~GuiScrollbarLoadHost() = default;
    virtual void* find_child_by_suffix_00aaa2b0(void* widget, const char* name) = 0;
    virtual void set_value_00ac7d00(void* scrollbar, float value) = 0;
    virtual void loaded_base_00aa7170(void* widget) = 0;
};
void gui_scrollbar_on_loaded_00ac8890(void* scrollbar, GuiScrollbarLoadHost& host);

// 00AC6040 Layer copy constructor, __thiscall(this, source), RET 4: base copy,
// vtable 00D5BE38; +ECh, +F0h (the camera store), +F5h, +F8h, +FCh (applied
// Priority) and the +100h name copied (+F4h is not); the store descriptor
// +108h..+11Ch is RESET to its defaults (0, 0.1 (00D7A2F0), 1000.0
// (00CE3804), 1.0, authored Priority 0, RenderOrder 0.0) rather than copied;
// +120h/+121h copied; then 00AC59A0 (scene/camera/store setup) and, when +F5h
// is set, the store's +20h count is incremented.
struct GuiLayerCopyHost {
    virtual ~GuiLayerCopyHost() = default;
    virtual void copy_widget_base_00aa9520(void* self, const void* source) = 0;
    virtual void setup_00ac59a0(void* layer) = 0;
};
void* gui_layer_copy_construct_00ac6040(void* self, const void* source, GuiLayerCopyHost& host,
    GuiNativeStringHost& strings);

// ---------------------------------------------------------------------------
// VS2008 container internals of segments 76/77, over raw native nodes
// ---------------------------------------------------------------------------

// Layouts (VS2008, _SECURE_SCL on):
//   std::list<T>   {allocator +0, head node +4, size +8}; node {next +0, prev +4, value +8}
//   std::_Tree     {comparator/allocator +0, head node +4, size +8}; node {left +0,
//                  parent +4, right +8, value +0Ch.., color +14h (1 = black), isnil +15h}
//   std::vector<T> {allocator +0, first +4, last +8, end +0Ch}
// operator new is 00BF681B, operator delete 00BF65AC, free 00BF6989. These
// reconstructions use ::operator new / ::operator delete / std::free on raw
// storage; the native allocator is the same CRT pair.
namespace msvc9_containers {

// 00AA1040 _Tree::_Max, __fastcall(ECX = node): follow right links while the
// right child is not the nil head; EAX = node.
void* tree_rightmost_00aa1040(void* node) noexcept;
// 00AA2920 _Tree::_Buynode(): 18h bytes, links null, color 1, isnil 0 (value
// bytes +0Ch..+13h left uninitialized).
void* tree_buy_head_node_00aa2920();
// 00AA2C10 / 00AA3420 / 00AA5730, __fastcall(ECX = tree) (00AA3420 is the
// constructor taking the comparator and allocator, RET 8): head = buy node,
// head->isnil = 1, head->parent = head->left = head->right = head, size 0.
void* tree_init_head(void* tree);
// 00AA1210 _Tree::const_iterator::_Dec, __fastcall(ECX = iterator {owner +0,
// node +4}); 00AA2280 / 00AA28B0 operator--() call it and return the iterator.
// A null owner, or decrementing end() of an empty tree, reaches 00BF6713;
// decrementing begin() moves to the head without a failure.
struct TreeIteratorHost {
    virtual ~TreeIteratorHost() = default;
    virtual void checked_iterator_failure_00bf6713() = 0;
};
void tree_iterator_decrement_00aa1210(void* iterator, TreeIteratorHost& host);
void* tree_iterator_pre_decrement(void* iterator, TreeIteratorHost& host);
// 00AA28F0 _Tree::clear, __fastcall(ECX = tree): _Erase(root) (00AA22C0), then
// head->parent = head, size = 0, head->left = head->right = head.
// 00AA4CF0 / 00AA5150 _Tree destructors: erase(begin(), end()) (00AA49E0),
// free the head, head = 0, size = 0.
struct TreeEraseHost {
    virtual ~TreeEraseHost() = default;
    virtual void erase_subtree_00aa22c0(void* tree, void* root) = 0;
    virtual void erase_range_00aa49e0(void* tree, void* first, void* last) = 0;
};
void tree_clear_00aa28f0(void* tree, TreeEraseHost& host);
void tree_destroy(void* tree, TreeEraseHost& host);

// 00AAC180 list _Buynode() sentinel: 0Ch bytes, next = prev = itself.
void* list_buy_sentinel_00aac180();
// 00AAC210 list _Buynode(next, prev, const T& value) for a 4-byte T, RET 0Ch.
void* list_buy_node_00aac210(void* next, void* prev, const std::uint32_t& value);
// 00AA4D30 list _Buynode(next, prev, const entry&) for the 0Ch-byte named
// entry: 14h bytes; the entry is copy-constructed by 00AA3D30 (an EH frame
// frees the node if that throws).
void* list_buy_named_entry_node_00aa4d30(void* next, void* prev, const void* entry, GuiNativeStringHost& strings);
// 00AAC1D0 list::clear for 4-byte values: unlink everything, size 0, delete each
// node.
void list_clear_00aac1d0(void* list) noexcept;
// 00AAC3B0 list destructor: clear, delete the head, head = 0.
void list_destroy_00aac3b0(void* list) noexcept;
// 00AAC500 / 00AA3400 list constructors: head = sentinel of node_bytes (0Ch via
// 00AAC180 for the button list, 14h via 00AA2820 for the named-entry list),
// size 0.
void* list_construct(void* list, std::uint32_t node_bytes);
// 00AA57C0 list copy constructor, __thiscall(list, const list*), RET 4: head =
// 00A9B720 sentinel, size 0, then insert(end, source.begin, source.end)
// (00A9D560).
struct ListCopyHost {
    virtual ~ListCopyHost() = default;
    virtual void* buy_sentinel_00a9b720() = 0;
    virtual void insert_range_00a9d560(void* list, void* where, void* first, void* last) = 0;
};
void* list_copy_construct_00aa57c0(void* list, const void* source, ListCopyHost& host);

// std::_Allocate<T>(count) for sizeof(T) = 0Ch (00AAB830), 14h (00AA1B40,
// 00AACD30), 18h (00AA1BF0) and 24h (00AACD90), __fastcall(ECX = count):
// count 0 allocates 0 bytes; count > 0 with 0xFFFFFFFF / count < sizeof(T)
// throws std::bad_alloc (00BF6340 / 00BF6885); otherwise operator new.
void* allocate_elements(std::uint32_t count, std::uint32_t element_bytes);
// allocator::deallocate(p, n), __thiscall(alloc, p, n), RET 8 (00AA10F0,
// 00AA1190, 00AAB770, 00AACAE0, 00AACB00): operator delete(p).
void deallocate(void* pointer) noexcept;
// 00AA6E80 / 00AAB7F0 / 00AACCE0, __fastcall(ECX = slot): free(*slot) when
// non-null; the slot is NOT cleared. 00AA1AD0: operator delete(*slot) when
// non-null and the slot cleared.
void free_pointer(void** slot) noexcept;
void free_and_clear_00aa1ad0(void** slot) noexcept;
// vector::size for 40h (00AA1D00), 14h (00AAC910) and 24h (00AAC970) byte
// elements, __fastcall(ECX = vector): 0 when first is null.
std::uint32_t vector_size(const void* vector, std::uint32_t element_bytes) noexcept;

} // namespace msvc9_containers

// ---------------------------------------------------------------------------
// Manager cursor, render queue, Curve texture/destructor, Model copy
// ---------------------------------------------------------------------------

// 00AA1FA0, __thiscall(manager, float x, float y), RET 8: +5Ch = x, +60h = y,
// then the cursor widget at +50h gets 00AA7D00(x, y) (the local XY setter).
struct GuiCursorHost {
    virtual ~GuiCursorHost() = default;
    virtual void set_local_xy_00aa7d00(void* widget, float x, float y) = 0;
};
void gui_manager_set_cursor_position_00aa1fa0(void* manager, float x, float y, GuiCursorHost& host);

// 00AA2B80, cdecl: 004C11F0 (the render command queue singleton) then tail-jump
// 00B1C4B0 on it.
struct GuiRenderQueueHost {
    virtual ~GuiRenderQueueHost() = default;
    virtual void* render_command_queue_004c11f0() = 0;
    virtual void reset_queue_00b1c4b0(void* queue) = 0;
};
void gui_reset_render_command_queue_00aa2b80(GuiRenderQueueHost& host);

// 00ABDD80 Curve SetTexture, __thiscall(curve, const native string* name),
// RET 4. Nothing when 00435C40 says +104h already equals name. Otherwise copy
// the name into +104h (self-assignment skipped), reset the UV rect +11Ch..+128h
// to (0,0,1,1), resolve with 00AA2660(name, uv, size, 1.0f) (size is an
// uninitialized local), set the texture into slot 0 of the material at +12Ch
// (00B189F0), then drop the resolver's reference (LOCK DEC +4h; vtable +0 at
// zero).
struct GuiCurveTextureHost {
    virtual ~GuiCurveTextureHost() = default;
    virtual bool names_equal_00435c40(const void* a, const void* b) = 0;
    virtual void* resolve_texture_00aa2660(const void* name, float (&uv)[4], float (&size)[2], float scale) = 0;
    virtual void set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    virtual void release_ref_object(void* object) = 0;
};
void gui_curve_set_texture_00abdd80(void* curve, const void* name, GuiCurveTextureHost& host,
    GuiNativeStringHost& strings);

// 00ABDE90 Curve destructor, __fastcall(ECX = curve): vtable 00D5C7F8; release
// the material ref +12Ch; operator delete of the +134h vector block and zero
// +134h..+13Ch; the Mit qword vector +10Ch (reserve(0) when its capacity is
// negative, count 0, free); the +104h texture name buffer; widths +F8h
// (resize 0, free); the Mit qword vector +ECh likewise; base destructor.
struct GuiCurveDestroyHost {
    virtual ~GuiCurveDestroyHost() = default;
    virtual void release_ref_object(void* object) = 0;
    virtual void reserve_qword_vector_0086a350(void* header, std::int32_t capacity) = 0;
    virtual void release_string_buffer(void* buffer, std::uint32_t bytes) = 0;
    virtual void destroy_widget_base_00aa9730(void* self) = 0;
};
void gui_curve_destroy_00abde90(void* curve, GuiCurveDestroyHost& host);

// 00ACCF70 Model copy constructor, __thiscall(this, source), RET 4: base copy,
// vtable 00D5CF38, the scene +ECh/+F0h NOT copied (zeroed), ScaleVector and
// RotationEuler (+F4h..+108h) copied, both strings copied, then 00ACCCF0
// LoadModel(+10Ch, +114h) on the copy.
struct GuiModelCopyHost {
    virtual ~GuiModelCopyHost() = default;
    virtual void copy_widget_base_00aa9520(void* self, const void* source) = 0;
    virtual void load_model_00acccf0(void* model, void* name, void* texture_override) = 0;
};
void* gui_model_copy_construct_00accf70(void* self, const void* source, GuiModelCopyHost& host,
    GuiNativeStringHost& strings);

} // namespace bsp::gui_widgets_cc13
