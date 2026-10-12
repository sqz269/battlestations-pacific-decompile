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

#include "bsp/gui_layout_loader.hpp"
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

} // namespace bsp::gui_widgets_cc13
