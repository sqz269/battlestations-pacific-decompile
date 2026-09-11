#pragma once
#include "bsp/gui_text_geometry.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/native_font_resources.hpp"
#include <array>
#include <memory>
#include <optional>
#include <string>

namespace bsp {
struct GuiTextWrappedServices {
    NativeFontResourceOwners& fonts;
    NativeLogicalBufferMappingContext& mapping;
    NativeRenderActualOwners& actual_owners;
    GuiWidgetOwnerRuntime& widgets;
    NativeNodeParentingRuntime& parenting;
    GuiTextGlyphChildCalls& children;
    const volatile float& vertical_scale_00e12fd4;
};

enum class GuiTextWrappedPending {
    glyph_child,             // AB98F0 has stopped at AB9D33, caller at ABA730.
    decoded_position,        // 004768D6: actual stream+50 is nonzero.
    unsupported_alignment   // ABA53A: mode outside verified initialized 0..3.
};

// Native local frame, borrowing the SAME live Text/section/font/stream owners.
// No destructor unlocks, releases or completes pending native work. All string
// backing and selected glyph addresses must remain valid until completion.
struct GuiTextWrappedContinuation {
    GuiTextLifetime* lifetime{};
    GuiTextWrappedServices* services{};
    NativeMeshStorage* mesh{};
    NativeMeshSectionStorage* section{};
    void* vertex_stream{};
    void* index_stream{};
    void* indices{};
    const FontGlyphData* space_glyph{};
    const FontGlyphData* glyph{};
    const char16_t* text_cursor{};
    const char16_t* line_end{};
    FontGlyphPlacement placement{};
    // Native arguments2/5/6 ALL alias this one float3. Stable allocation
    // survives moving this caller frame into optional/outer continuations.
    // It is distinct from pen locals: child-tail writes must not alias pen_x.
    std::unique_ptr<std::array<float, 3>> native_position;
    std::int32_t signed_height{};
    float height_float{};
    std::uint32_t container_width{};
    std::uint32_t quad_index{};
    std::uint32_t first_vertex{};
    float pen_y{};
    float line_bottom{};
    float minimum_y{10000000000.0f};
    float maximum_y{-10000000000.0f};
    float emitted_width{};
    float space_step{};
    float scan_width{};
    std::uint32_t saved_width{};
    std::uint32_t spaces{};
    bool natural_end{};
    bool first_resources{true};
    float vertical_offset{};
    std::uint32_t position_index{};
    GuiTextWrappedPending pending{GuiTextWrappedPending::glyph_child};
};

// ABA270: ECX Text, UTF16 wrapper and section stack arguments, RET8. New C++
// interface: typed UTF16 assignment owns its allocation; native string-pool
// allocation/SEH equivalence is not claimed. Source may be the SAME Text string.
// Clear runs first; font height is then captured, string copied if distinct,
// actual streams mapped and live metrics/resources read in native order.
// nullopt means completion through final position stores and BOTH unlocks.
// A value preserves the native frame and active mappings at the stated gap.
// Keep the enclosing content continuation alive and do not run its post tail.
// No finite/capacity clamp or overflow sanitization is added: native x87/SSE
// conversions, DWORD counters and low16 final draw ranges are retained. Inputs
// must supply valid readable terminated UTF16 and sufficient actual mappings;
// the existing quad writer independently validates its supported layout.
std::optional<GuiTextWrappedContinuation> build_gui_text_wrapped_00aba270(
    GuiTextLifetime&, const std::u16string& source, NativeMeshSectionStorage&,
    GuiTextWrappedServices&);

// Consume once, ONLY after the actual AB98F0 child tail has completed using
// saved call arguments. Does not perform or substitute that missing lifecycle.
std::optional<GuiTextWrappedContinuation>
resume_gui_text_wrapped_after_child_00aba270(GuiTextWrappedContinuation);

// Partial 004768D0, ECX stream, out float3 and index stack, RET8. Implements
// +50==0 branch 4768D0..68DF -> 476B18..6B40. Returns false BEFORE any read or
// output store for the excluded packed/scale-bias branch 4768E0..6B17.
bool read_gui_text_float3_position_004768d0_fragment(
    void* actual_vertex_stream, std::uint32_t index, float (&output)[3]);
} // namespace bsp
