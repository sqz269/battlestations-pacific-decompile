#pragma once
#include "bsp/native_font_glyph_selection_storage.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_render_context.hpp"
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text single-line geometry requires MSVC Win32 and x87 inline assembly.
#endif

namespace bsp {
// Genuine missing raw operations, not semantic GuiText/Font/Model adapters.
// Every receiver/table/backing/context belongs to the SAME actual Text resource
// graph and survives callbacks. No default glyph/font/table/owner is supplied.
struct NativeGuiTextSingleLineStorageBindings {
    virtual ~NativeGuiTextSingleLineStorageBindings() = default;
    virtual void* model_geometry_00b74640(void* captured_model, std::uint32_t index) = 0;
    // Pure metadata lookup of this captured CURRENT raw profile. SAME live
    // table, at least18h; no reentry, Native operation or numeric-code call.
    virtual const volatile std::uint32_t* current_stream_table(void* captured_stream,
        std::uint32_t captured_profile) = 0;
    // Only OTHER targets use these operations. Observed B49980/B49B60 and
    // B49C70/B49A80 use the existing concrete raw mapping services directly.
    // Unknown targets require genuine whole Source bodies, never success/no-op.
    virtual void* other_vertex_map_10(void*, std::uint32_t captured_target,
        std::uint32_t count, std::uint32_t offset, std::uint32_t readonly_word) = 0;
    virtual void* other_index_map_0c(void*, std::uint32_t captured_target,
        std::uint32_t count, std::uint32_t offset, std::uint32_t readonly_word) = 0;
    virtual void other_index_unmap_10(void*, std::uint32_t captured_target) = 0;
    virtual void other_vertex_unmap_14(void*, std::uint32_t captured_target) = 0;
    // Whole AB98F0, not a pending/success substitute. Actual mutable callee
    // homes: glyph,pos3*,vertex,indexptr,pos3*,pos3*,quad,height,first,UTF16.
    // All three pos3* words alias the SAME retained float3. Callee may mutate
    // homes (notably0/7), so the caller keeps glyph/height separately.
    virtual void write_glyph_quad_00ab98f0(void* captured_text,
        volatile std::uint32_t (&actual_homes)[10]) = 0;
    virtual void rebuild_layout_00b865a0(void* current_section, void* captured_mesh) = 0;
};

struct NativeGuiTextSingleLineStorageContext final {
    NativeGuiTextSingleLineStorageBindings& bindings;
    NativeLogicalBufferMappingContext& mapping;
    NativeRenderActualOwnerRegistry& owners;
    // SAME authenticated mapped data/cells, not copied numeric defaults.
    const volatile double& width_00cec380;
    const volatile float& unsigned_bias_00ce3978;
    const volatile double& half_00d7a280;
    // Required retained actual F8BE50 initial-wide-NUL role. This packet makes
    // no replacement zero array or new process owner; binding remains caller proof.
    const volatile std::uint16_t* const empty_wide_00f8be50;
};

enum class NativeGuiTextSingleLineStoragePhase : std::uint8_t {
    fresh, entered, complete, failed
};
struct NativeGuiTextSingleLineStorageAcquired final {
    using Phase = NativeGuiTextSingleLineStoragePhase;
    enum class Boundary : std::uint8_t {
        none, model_geometry, vertex_stream, vertex_map, index_map,
        width, measure_glyph, alignment, emit_glyph, texture0, texture1,
        glyph_quad, index_unmap, vertex_unmap, layout
    };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t native_site{}, escaped_site{};
    void* text{};
    void* model{};
    void* mesh{};
    void* vertex{};
    void* index{};
    void* index_mapping{};
    void* measured_glyph{}; // diagnostic identities after callbacks/failure
    void* emitted_glyph{};
    std::uint32_t vertex_count{}, index_count{};
    std::uint32_t vertex_map_target{}, index_map_target{};
    std::uint32_t index_unmap_target{}, vertex_unmap_target{};
    std::uint32_t measured_units{}, emitted_units{};
    bool vertex_map_entered{}, vertex_map_returned{};
    bool index_map_entered{}, index_map_returned{};
    bool index_unmap_entered{}, index_unmap_returned{};
    bool vertex_unmap_entered{}, vertex_unmap_returned{};
    bool layout_entered{}, layout_returned{};
    std::exception_ptr failure;
    NativeGuiTextSingleLineStorageAcquired() = default;
    NativeGuiTextSingleLineStorageAcquired(const NativeGuiTextSingleLineStorageAcquired&) = delete;
    NativeGuiTextSingleLineStorageAcquired& operator=(const NativeGuiTextSingleLineStorageAcquired&) = delete;
};

// Raw caller scratch only; no initializer/destructor/native ownership. These
// separate homes preserve float32 spills and live quad arguments through failure.
// Their offsets are Source-private, not a claim of Original stack alias identity.
struct NativeGuiTextSingleLineStorageScratch final {
    volatile std::uint16_t saved_control, truncation_control;
    alignas(8) volatile std::int64_t converted_width;
    volatile std::uint32_t width_bits, height, pen_bits;
    volatile std::uint32_t position_bits[3];
    volatile std::uint32_t quad_homes[10];
    volatile std::uint32_t cursor, next_unit, advance;
    volatile std::uint32_t first_vertex, quad_index, index_position, first_glyph;
    // Reached selector calls borrow this real WORD home, not a typed DWORD
    // alias. Reuse only after normal return; a failed call retains both fields.
    volatile std::uint16_t font_code_unit_home;
    NativeFontGlyphSelectionStorageScratch font_glyph;
};

class NativeGuiTextSingleLineStorageFrame final {
public:
    using Phase = NativeGuiTextSingleLineStoragePhase;
    // Constructor stores metadata only. homes[0] is NEVER read by this body;
    // homes[1] remains the actual CURRENT draw-section DWORD argument home.
    NativeGuiTextSingleLineStorageFrame(void* captured_actual_text,
        volatile std::uint32_t (&actual_homes)[2],
        NativeGuiTextSingleLineStorageContext&, NativeGuiTextSingleLineStorageAcquired&) noexcept;
    ~NativeGuiTextSingleLineStorageFrame() noexcept = default;
    NativeGuiTextSingleLineStorageFrame(const NativeGuiTextSingleLineStorageFrame&) = delete;
    NativeGuiTextSingleLineStorageFrame& operator=(const NativeGuiTextSingleLineStorageFrame&) = delete;
    NativeGuiTextSingleLineStorageFrame(NativeGuiTextSingleLineStorageFrame&&) = delete;
    NativeGuiTextSingleLineStorageFrame& operator=(NativeGuiTextSingleLineStorageFrame&&) = delete;
    Phase phase() const noexcept { return phase_; }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::entered || phase_ == Phase::failed;
    }
private:
    friend void build_native_gui_text_single_line_storage_00ab9fd0(
        NativeGuiTextSingleLineStorageFrame&);
    void* const text_;
    volatile std::uint32_t (&homes_)[2];
    NativeGuiTextSingleLineStorageContext& context_;
    NativeGuiTextSingleLineStorageAcquired& acquired_;
    NativeGuiTextSingleLineStorageScratch scratch_; // no default zeroing
    Phase phase_{Phase::fresh};
};

// WHOLE AB9FD0..ABA263[660B/195ops], native ECX Text, unused first UTF16
// argument + CURRENT draw-section home, RET8. New ordinary Source ABI only.
// Uses actual Text EC/F0 loops, unsigned glyph12 advances, x87 operation/spill
// order and Native truncating64/lowDWORD width conversion. Native FNSTCW,
// FLDCW(truncate), FISTP64, FLDCW(saved) are retained at their actual phases;
// no exception masking/reset, fenv guard, extra rounding or cleanup is added.
// Root retains genuine raw Text/Font/glyph/Model/Mesh/material/stream/renderer
// owners, mappings/backing, actual argument homes and this nonmoving frame.
// All metadata/scratch are disjoint from those raw resources and providers.
// Failure retains entered maps/partial geometry/arguments; never retry or
// auto-unlock/repair from destruction. requires_process_retention() is a caller
// contract, not a new process terminal policy. Unknown/missing genuine providers
// remain explicit; this does not activate changed Text/layout/glyph recursion.
// Native register/FH3/SEH/private-stack/fault/unmasked-FP runtime equivalence is
// unproved; ambient CW/status are not normalized by this Source implementation.
void build_native_gui_text_single_line_storage_00ab9fd0(
    NativeGuiTextSingleLineStorageFrame&);
} // namespace bsp
