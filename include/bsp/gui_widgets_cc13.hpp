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
#include <string>
#include <utility>

#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_widget.hpp"
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
} // namespace bsp::gui_widgets_cc13
