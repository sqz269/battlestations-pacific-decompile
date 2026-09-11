#pragma once
#include "bsp/gui_text.hpp"
#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_widget_detach.hpp"
#include "bsp/locale_text_lookup.hpp"
#include "bsp/native_mesh_section.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace bsp {

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
// Unimplemented tail00ABA9BF..00ABAE89 allocates/builds/shares geometry, binds
// material parameters and changes shadow scene state; it must be resumed by a
// real owner implementation. FontGeometryOwner's fixed-font fragment is not a
// drop-in continuation, and this function never pretends that it is.
// Native __thiscall ECX=Text, one UTF16 wrapper stack arg, RET4. New C++ ABI.
// Valid inputs: null-free UTF16 fitting native signed32 length, same live
// widget.font/locale resources, registered actual main/shadow models and mesh
// sections after ensure_draw_sections. Invalid ownership fails explicitly.
// No caller-level final+50 color call is performed here.
GuiTextContentContinuation prepare_gui_text_content_00aba8d0_fragment(
    GuiTextContentBinding&, GuiTextContentEnvironment&, std::u16string_view);
} // namespace bsp
