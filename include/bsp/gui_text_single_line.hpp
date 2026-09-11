#pragma once
#include "bsp/gui_text_geometry.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_font_resources.hpp"
#include <optional>
#include <string_view>

namespace bsp {
struct GuiTextSingleLineServices {
    NativeFontResourceOwners& fonts;
    NativeLogicalBufferMappingContext& mapping;
    NativeRenderActualOwners& actual_owners;
    NativeMeshSectionLayoutServices& layouts;
    const volatile float& vertical_scale_00e12fd4;
};

// Borrowed native call-frame locals at ABA1F8, not additional Text state or
// resource ownership. Both original streams remain mapped. The current glyph
// and string backing, all owners/services and the section must remain live.
// No destructor silently unlocks or advances an unfinished child operation.
struct GuiTextSingleLineContinuation {
    GuiTextLifetime* lifetime;
    GuiTextSingleLineServices* services;
    NativeMeshStorage* mesh;
    NativeMeshSectionStorage* section;
    void* vertex_stream;
    void* index_stream;
    void* indices;
    const FontGlyphData* glyph;
    const char16_t* text_cursor;
    FontGlyphPlacement placement;
    std::uint32_t quad_index;
    std::uint32_t first_vertex;
    std::uint16_t height;
};

// Partial projection with a complete ordinary path and explicit optional-child
// continuation. AB9FD0: ECX Text, unused UTF16-wrapper and section stack args,
// RET8. Uses SAME live stored Text string, font association, metrics and
// actual mesh/section/resources. No placement vector, metric snapshot or
// texture wrapper. Exact x87 int64 conversion (low DWORD, masked-invalid
// behavior), unsigned width alignment and per-glyph float spills.
// nullopt means native completion through index unlock, vertex unlock and
// B865A0. A value means optional AB98F0 child continuation is still required
// at AB9D33: UVs are already blanked and BOTH mappings remain active. Caller
// must keep this frame and the enclosing content continuation, not run the
// post-builder color/shadow tail or treat the result as completed geometry.
// No special-child factory, font loader or renderer layout factory is supplied.
std::optional<GuiTextSingleLineContinuation> build_gui_text_single_line_00ab9fd0(
    GuiTextLifetime&, std::u16string_view unused_source,
    NativeMeshSectionStorage&, GuiTextSingleLineServices&);

// Actual ABA1FD..continuation. Call ONLY after the required AB98F0 child tail
// has completed using the saved call arguments; this entry does not perform
// or assume that operation by default. Returns the next pending child or
// nullopt after native stream unlock/layout completion. Consume each frame
// once. Active Text string storage must not relocate, as in the native call.
std::optional<GuiTextSingleLineContinuation>
resume_gui_text_single_line_after_child_00ab9fd0(GuiTextSingleLineContinuation);
} // namespace bsp
