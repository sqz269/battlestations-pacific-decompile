#pragma once
#include "bsp/gui_text.hpp"
#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_widget_detach.hpp"
#include "bsp/locale_text_lookup.hpp"
#include "bsp/native_mesh_section.hpp"
#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace bsp {
class GuiTextLifetime;
class NativeFontResourceOwners;
struct GuiTextShaderServices;
struct GuiMaterialBindingServices;
struct NativeMaterialParameterAccess;
struct NativeMaterialStorage;
struct NativeMeshStorage;

// Borrows the SAME Text projection, retained GUI owner, current shadow slot
// and glyph-child collection. There is no new Text owner or reference count.
// The glyph collection projects Text+198/+19C, not the general GUI child list.
// Its children must already have their actual Text ownership/dispatch binding.
struct GuiTextContentBinding {
    GuiWidgetOwner& widget;
    GuiTextWidget& text;
    NativeNodeBinding*& shadow_188;
    std::vector<GuiLayoutWidget*>& glyph_children_198;
};

// Required actual ownership operations: no default/no-op child owners.
// AB9D38/AB9D4F create these children as Text (vtable D5C6C8). Scalar delete4
// is00AB8EE0, which performs00AB8250 then returns pool storage iff flags&1.
// GuiWidgetOwnerRuntime::retire_tree is a DIFFERENT page-manager sequence
// (virtual20 before deletion), so it must not stand in for this operation.
struct GuiTextGlyphChildCalls {
    virtual ~GuiTextGlyphChildCalls() = default;
    // C++ allocation-lifetime transport, not a native call: retain the SAME
    // detached object until actual deletion/re-attachment. It must not run
    // before_destroy's page-retirement fallback or alter native-visible state.
    // Transfer precedes the native post-detach slot reload: callbacks may have
    // replaced that slot with a DIFFERENT child. No duplicate widget tree.
    virtual void accept_detached_child(std::unique_ptr<GuiLayoutWidget>) noexcept = 0;
    // Must run the actual Text destructor/pool return, consume that instance's
    // transferred ownership if any, and clear before_destroy before its C++
    // wrapper is destroyed. Dropping a unique_ptr is not native virtual04.
    virtual void delete_text_child_virtual4(GuiLayoutWidget& child, std::uint32_t flags) = 0;
};

//00AB80C0..00AB81B6 complete valid-collection control flow. Native __thiscall
// ECX=Text, no stack args, RET. Detach then reload slot, delete4(1), zero slot;
// recheck live size per iteration; clear size retaining allocation at end.
// Callbacks may change elements/size, but a deleting callback must not move or
// invalidate the captured slot. Same precondition as native EDI slot capture.
// Missing slots after detach fail explicitly instead of native CRT termination.
void clear_gui_text_glyph_children_00ab80c0(GuiTextContentBinding&,
    GuiWidgetOwnerRuntime&, NativeNodeParentingRuntime&, GuiTextGlyphChildCalls&);

struct GuiTextContentCalls : GuiTextGlyphChildCalls {
    // Host service identity, not a native call. A locale adapter returns the
    // actual deletion transport retained by Text lifetimes and wrapped layout;
    // a combined implementation can retain the original default identity.
    virtual GuiTextGlyphChildCalls& glyph_child_calls() noexcept { return *this; }
    // Locale-changed arm of __wcsicmp00C03A39: __wcsicmp_l00C0392A with
    // null locale (current native CRT locale), two valid terminated strings.
    virtual int compare_current_locale_00c0392a(const char16_t*, const char16_t*) = 0;
};
struct GuiTextContentEnvironment {
    const LocaleTables& locale;
    LocaleTextRuntimeHost& locale_runtime;
    // SAME live CRT mode flag read by00C03A39. Zero takes ASCII-only folding;
    // nonzero takes the required current-locale comparator. No assumed default.
    const std::int32_t& crt_locale_changed_0109de1c;
    GuiTextBufferServices& buffers;
    GuiTextContentCalls& calls;
};

enum class GuiTextContentBranch {
    unchanged,
    cleared,
    needs_nonempty_geometry // Not successful completion of native00ABA8D0.
};
struct GuiTextContentContinuation {
    GuiTextContentBranch branch;
    // Native temporary wrapper remains separate from Text+EC/F0 assignment.
    // Only the pending nonempty branch needs this copy and the captured section.
    std::u16string transformed_text;
    NativeMeshSectionStorage* main_section{};
    void* main_mesh{};
};

// PARTIAL00ABA8D0: implements entry through00ABA9BA plus common temporary
// cleanup. Nonempty returns the exact continuation point00ABA9BF, AFTER width
// reset, real child clear, main section capture and stored text assignment.
// The nonempty stages below resume this state. The selected native geometry
// builder remains an explicit gap; FontGeometryOwner's fixed-font fragment is
// not a drop-in continuation.
// Native __thiscall ECX=Text, one UTF16 wrapper stack arg, RET4. New C++ ABI.
// Valid inputs: null-free UTF16 fitting native signed32 length, same live
// widget.font/locale resources, registered actual main/shadow models and mesh
// sections after ensure_draw_sections. Invalid ownership fails explicitly.
// No caller-level final+50 color call is performed here.
GuiTextContentContinuation prepare_gui_text_content_00aba8d0_fragment(
    GuiTextContentBinding&, GuiTextContentEnvironment&, std::u16string_view);

struct GuiTextNonemptyEnvironment {
    GuiTextShaderServices& shader;
    NativeMaterialParameterAccess& parameters;
    NativeMeshSectionLayoutServices& layouts;
    const GuiMaterialBindingServices& style;
    NativeFontResourceOwners& fonts;
    const float& blend_factor_00f8be54; // Actual live global, borrowed by parameter.
};
enum class GuiTextGeometryBuilder { single_00ab9fd0, wrapped_00aba270 };
struct GuiTextNonemptyContinuation {
    GuiTextLifetime* lifetime;
    std::u16string transformed_text; // Same separate native temporary.
    NativeMeshStorage* main_mesh;
    NativeMeshSectionStorage* main_section;
    NativeMaterialStorage* main_material; // Captured BEFORE shader callbacks.
    bool shader_selection_attempted; // AL result, including a null load.
    GuiTextGeometryBuilder builder; // Captured only AFTER main layout rebuild.
};

// Partial00ABA8D0,00ABA9BF..00ABAB69/77. Allocates actual buffers, resets
// section ranges, selects/publishes the shader and real borrowed parameters,
// then rebuilds the main layout. Returns BEFORE the selected builder call.
// Does not invoke a placeholder geometry builder or claim completed content.
GuiTextNonemptyContinuation prepare_gui_text_nonempty_00aba8d0_fragment(
    GuiTextLifetime&, GuiTextContentContinuation&&, GuiTextNonemptyEnvironment&);

// Partial00ABA8D0,00ABAB7D..00ABAE89 plus temporary cleanup. Caller may enter
// ONLY after the selected actual native builder completed using this SAME
// lifetime, temporary text and main section; no intervening unrelated work.
// The builders00AB9FD0/00ABA270 are NOT implemented by this module. Their
// allocation/metrics/optional-child behavior cannot be replaced by success.
// Captured raw resources and all borrowed parameter fields must survive native
// callbacks. This adds no retain, second glyph owner, material cache or metric
// snapshot. Font is re-read late through its canonical descriptor association.
void finish_gui_text_content_after_geometry_00aba8d0_fragment(
    GuiTextLifetime&, GuiTextNonemptyContinuation&&, GuiTextNonemptyEnvironment&);

// Complete00B73260 valid array index0..5, ECX actual mesh/index stack/RET4.
// Reads +64[index] irrespective of live count; caller must select initialized
// storage. Out-of-array native reads are outside this C++ domain.
void* native_mesh_vertex_stream_00b73260(const NativeMeshStorage&, std::uint32_t index);

//00B6DAB0 Model-profile fragment: ECX node, stack XYZ pointer, tail JMP
// CURRENT virtual38 with the SAME local+B0 matrix. Writes x/y/z through x87,
// then actual00B6DB10 invalidation/notification. Other derived+38 profiles
// (including camera overrides) are explicitly outside this entry.
void set_native_model_local_position_00b6dab0_fragment(
    NativeModelOwner&, const std::array<float, 3>&);
} // namespace bsp
