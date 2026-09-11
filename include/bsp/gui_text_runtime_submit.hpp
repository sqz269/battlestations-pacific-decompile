#pragma once
#include "bsp/gui_text_runtime_content.hpp"
#include <memory>
#include <string>

namespace bsp {
struct GuiTextSubmitServices {
    GuiTextRuntimeContentServices& content;
    const volatile float& default_width_00d7a260; // Native -1.0f, read live.
};

// Outer caller temporaries, not another Text/cache/font owner. Both strings
// remain alive across pending geometry; clipped is destroyed before converted,
// and both before the caller's final CURRENT color50. No destructor completes
// or unlocks unfinished native geometry. The caller retains all borrowed owners.
struct GuiTextSubmitContinuation {
    GuiTextLifetime* lifetime{};
    GuiTextSubmitServices* services{};
    bool final_color{};
    std::u16string converted;
    std::u16string clipped;
    std::unique_ptr<GuiTextRuntimeContentContinuation> content;
};
enum class GuiTextSubmitStatus { unchanged_source, complete, pending_content };
struct GuiTextSubmitResult {
    GuiTextSubmitStatus status;
    std::unique_ptr<GuiTextSubmitContinuation> pending;
};

// New C++ ABI over actual content and style ownership. Supported Text profile
// has current50=AB6B50. Typed strings project native copies/cleanup; original
// string-pool callbacks, SEH and allocation-failure ABI are not reproduced.
// Inputs are terminated, null-free and <=INT32_MAX; source/cache remain valid
// during the existing host CRT comparison. Locale tables/runtime and actual font
// associations are the SAME services used by content, with their existing
// supported input domains. A pending value is never completed submission.

// AB6AB0: ECX Text, UTF16 wrapper stack, RET4. Always final color50, including
// when the inner content comparison skips rebuilding or clears empty text.
GuiTextSubmitResult submit_gui_text_utf16_00ab6ab0(
    GuiTextLifetime&, std::u16string_view, GuiTextSubmitServices&);

// ABAED0: ECX Text, narrow wrapper and low-byte localization flag, RET8.
// Native length/current-CRT equality returns before flag/conversion/color.
// Changed source is cached BEFORE localization or bytewise zero-extension.
GuiTextSubmitResult submit_gui_text_source_00abaed0(
    GuiTextLifetime&, const std::string&, bool localize, GuiTextSubmitServices&);

// ABB000: ECX Text, source/width/localize stack, RET0Ch. Same cache guard.
// Ordered equality with live -1 chooses current canonical width BEFORE
// conversion. Zero and NaN do not select that fallback. Resolve current font
// after conversion, ellipsize, submit content, destroy both temps, color50.
GuiTextSubmitResult submit_gui_text_ellipsis_00abb000(
    GuiTextLifetime&, const std::string&, float width, bool localize,
    GuiTextSubmitServices&);

// ABB1D0: ECX Text, no stack args, RET. Copy current text, clear its stored
// cache, submit saved copy, then destroy copy. No caller-level color50.
GuiTextSubmitResult rebuild_gui_text_content_00abb1d0(
    GuiTextLifetime&, GuiTextSubmitServices&);

// ABBF30: ECX Text, size-pair pointer stack, RET4. Native AA7970 pair stores
// and actual recompose precede the live current-text capture/rebuild. The base
// size setter does not refresh bounds. Pair aliasing follows native x87 order.
GuiTextSubmitResult resize_gui_text_00abbf30(
    GuiTextLifetime&, const GuiWidgetSize&, GuiTextSubmitServices&);

// ONLY after the actual missing glyph-child tail completes. Retains the outer
// caller while inner content is pending; destroys caller temporaries and runs
// final color only after all inner post-tail work completed. Undefined-format
// or alignment boundaries remain pending and reject this child-only resume.
GuiTextSubmitStatus resume_gui_text_submit_after_child(
    std::unique_ptr<GuiTextSubmitContinuation>&);
} // namespace bsp
