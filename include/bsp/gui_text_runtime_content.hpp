#pragma once
#include "bsp/gui_text_content.hpp"
#include "bsp/gui_text_single_line.hpp"
#include "bsp/gui_text_wrapped.hpp"
#include <memory>
#include <variant>

namespace bsp {
struct GuiTextGlyphChildTailServices;
struct GuiTextGlyphChildTailContinuation;
struct GuiTextRuntimeContentServices {
    GuiTextContentEnvironment& content;
    GuiTextNonemptyEnvironment& nonempty;
    GuiTextSingleLineServices& single_line;
    GuiTextWrappedServices& wrapped;
    // Explicit optional actual child implementation. Bind after constructing
    // its same factory/services domain and keep it alive through all frames.
    // Absent services preserve the existing explicit glyph-child boundary.
    GuiTextGlyphChildTailServices* glyph_children{};
};

// One outer ABA8D0 stack-frame projection. Its allocation and separate native
// string temporary stay stable while the selected builder is pending. All
// resource, Text, string/glyph backing, service and module lifetimes are borrowed.
// Destruction does not unlock mappings or finish any native operation.
struct GuiTextRuntimeContentContinuation {
    GuiTextRuntimeContentContinuation();
    ~GuiTextRuntimeContentContinuation();
    GuiTextRuntimeContentServices* services{};
    GuiTextNonemptyContinuation content;
    std::variant<std::monostate, GuiTextSingleLineContinuation,
        GuiTextWrappedContinuation> builder;
    std::unique_ptr<GuiTextGlyphChildTailContinuation> child_tail;
    // Host-only same-frame callback guard. Never enter a moved builder or
    // replay native child cleanup from a reentrant completion request.
    bool child_operation_active{};
};

enum class GuiTextRuntimeContentStatus { unchanged, cleared, complete, pending_builder };
struct GuiTextRuntimeContentResult {
    GuiTextRuntimeContentStatus status;
    std::unique_ptr<GuiTextRuntimeContentContinuation> pending;
};

// ABA8D0, native ECX Text, one UTF16-wrapper argument, RET4; new C++ ABI.
// Runs the actual prefix, nonempty preparation, selected real layout builder,
// then color/shadow tail ONLY after that builder returns complete. No caller's
// subsequent virtual50 is included. Uses the component modules' supported
// allocation/ownership/layout domains, not original string/pool/SEH ABI.
// A pending result MUST remain alive, with both mapped streams and their
// owners, until its exact native dependency is completed. Configured glyph
// services execute the actual child tail; missing services and unsupported
// native formats/alignment remain explicit pending boundaries.
GuiTextRuntimeContentResult build_gui_text_content_00aba8d0(
    GuiTextLifetime&, std::u16string_view, GuiTextRuntimeContentServices&);

// Conditional continuation, never a substitute child implementation. Call only
// after the actual AB98F0 child tail completed using the saved caller arguments.
// Unsupported wrapped reasons are rejected without consuming the outer frame.
// Nulls pending only after successful builder AND post-tail completion. Native
// faults, C++ allocation failures and destructive reentry are outside the domain;
// this is not exception-safe rollback/resumption of interrupted native calls.
GuiTextRuntimeContentStatus resume_gui_text_content_after_child_00aba8d0(
    std::unique_ptr<GuiTextRuntimeContentContinuation>& pending);

// Execute the configured ACTUAL child tail, recursively complete any nested
// child Text content, then advance its suspended caller. Repeat for later
// glyphs; post-builder color/shadow and stream unlocks occur only through the
// original continuations after completed child work. Missing services, native
// undefined cases or retained child boundaries leave the same frame pending.
GuiTextRuntimeContentStatus complete_gui_text_content_children_00aba8d0(
    std::unique_ptr<GuiTextRuntimeContentContinuation>& pending);
} // namespace bsp
