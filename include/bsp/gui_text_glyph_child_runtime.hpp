#pragma once
#include "bsp/gui_text_glyph_child.hpp"
#include "bsp/gui_text_glyph_reference.hpp"
#include "bsp/gui_text_runtime_factory.hpp"
#include "bsp/native_gui_text_model_clone.hpp"
#include <exception>

namespace bsp {
struct GuiTextGlyphChildTailServices {
    GuiTextRuntimeFactory& factory;
    GuiTextBufferServices& buffers;
    GuiTextFontNameServices& fonts;
    const GuiMaterialBindingServices& style;
    ActualNativeStringPoolStorage& strings;
    const volatile std::uint32_t* mesh_vtable_00d62d60;
    const char* shader_name_00cefd78; // Live native literal: GuiFontBilinear.mshd.
    const volatile float& shadow_offset_00d5c5c0;
    const volatile float& one_00d7a24c;
    const volatile float& pivot_00ce3800;
    GuiTextGlyphPositionConstants position;
};

enum class GuiTextGlyphChildTailStatus {
    ready, complete, pending_content, point_light_owners_required, domain_required
};

// This is the retained native caller frame, not another Text/model owner.
// Its original call arguments and position backing remain borrowed. child owns
// the SAME detached layout until AAA5A0 transfers it to the parent's live list.
// The glyph vector contains only that borrowed pointer, published before clone.
// Acquired clone resources, actual pooled string headers and all nested content
// remain alive across pending results. Never discard a started pending frame:
// its destructor rejects that invalid lifetime transition rather than deleting
// a published/incomplete child or releasing a live mapped-string input.
struct GuiTextGlyphChildTailContinuation final {
    GuiTextGlyphChildTailContinuation(const GuiTextGlyphChildCallFrame&,
        GuiTextGlyphChildTailServices&);
    ~GuiTextGlyphChildTailContinuation() noexcept;
    GuiTextGlyphChildTailContinuation(const GuiTextGlyphChildTailContinuation&) = delete;
    GuiTextGlyphChildTailContinuation& operator=(const GuiTextGlyphChildTailContinuation&) = delete;

    GuiTextGlyphChildCallFrame call;
    GuiTextGlyphChildTailServices& services;
    GuiTextGlyphChildTailStatus status{GuiTextGlyphChildTailStatus::ready};
    std::uint32_t pending_native_address{0x00ab9d33};
    std::exception_ptr failure;
    std::unique_ptr<GuiLayoutWidget> child;
    GuiLayoutWidget* child_identity{}; // Borrowed alias remains valid after attach.
    NativeGuiTextModelCloneAcquired acquired;
    NativeString shader_temporary;
    std::string shader_projection; // Existing typed AB8E70 argument only.
    struct Utf16Header { std::uint32_t length{}; char16_t* data{}; } code_unit;
    bool shader_temporary_live{};
    bool code_unit_live{};

    GuiTextRuntimeImplementation& implementation();
    GuiTextRuntimeContentContinuation* pending_content();
};

// AB98F0 fragment AB9D33..AB9FAD ONLY. Entry is AFTER ordinary quad/index writes,
// all four glyph UV clears and the native character-membership gates. Never
// repeats that prefix or unlocks either mapped stream. Allocates unbound Text,
// publishes +198, clones CURRENT reference model (flags26,parent0), transfers
// creator to child, ensures sections, current34, listener/mouse hit, CURRENT
// reference font, shadow, shader temporary, code unit, ABA8D0/current50, string
// cleanup, actual attachment, pivot and existing actual position tail.
// Native ECX parent with ten original stack words, RET28h belongs to the whole
// AB98F0 function; this typed continuation is not an original ABI replacement.
// Positive-light clone and unavailable/throwing domain operations retain the
// frame at their exact phase and never report completion. The latter records
// failure for inspection, not permission to retry a partially executed call.
GuiTextGlyphChildTailStatus begin_gui_text_glyph_child_tail_00ab98f0_fragment(
    GuiTextGlyphChildTailContinuation&);

// Call after the SAME child implementation has completed its nested pending
// content (using its saved child frames and resume_after_glyph_child). While
// it still has a pending operation this returns pending_content without effects.
// Only then release UTF16 storage, attach, pivot and position. No extra color50.
// Other boundary statuses cannot resume through this child-content-only entry.
GuiTextGlyphChildTailStatus resume_gui_text_glyph_child_tail_after_content(
    GuiTextGlyphChildTailContinuation&);
} // namespace bsp
