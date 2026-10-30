#pragma once
#include "bsp/native_font_glyph_selection_storage.hpp"
#include "bsp/native_gui_text_glyph_children.hpp"
#include "bsp/native_gui_text_glyph_quad_storage.hpp"
#include "bsp/native_logical_buffer_mapping.hpp"
#include "bsp/native_render_context.hpp"
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw wrapped Text storage requires MSVC Win32 and x87 inline assembly.
#endif

namespace bsp {
struct NativeGuiTextWrappedStorageAcquired;
// SAME 64h-byte Native private image S+10..S+73, where S is ESP after four
// register saves. Ordinary backing permits actual helper accesses; every
// reached Native access below uses raw asm at its original byte width.
// S+54 is reused as signed conversion/qword/FILD scratch AND getter float3;
// S+68 is the one Glyph position float3. No initializer or destructor runs.
// Caller supplies the authentic prior S+28 DWORD preimage, including for a
// first line with alignment outside0..3. Never zero/replace this image. Other
// unwritten private bytes likewise remain actual supplied backing/residue.
struct NativeGuiTextWrappedStorageScratch final {
    std::uint32_t words_10_50[17];
    float conversion_and_position_54[3];
    std::uint32_t height_60;
    std::uint32_t index_stream_64;
    float glyph_position_68[3];
};

struct NativeGuiTextWrappedStorageBindings {
    virtual ~NativeGuiTextWrappedStorageBindings() = default;
    // Pure SAME captured live table, through18h; no reentry, Native operation,
    // codeword cast, newer-profile substitution or fallback table.
    virtual const volatile std::uint32_t* current_stream_table(void* captured_stream,
        std::uint32_t captured_profile) = 0;
    // ONLY other targets use these. The four observed map/unmap targets call
    // existing raw services directly with the SAME mapping context.
    virtual void* other_vertex_map_10(void*, std::uint32_t captured_target,
        std::uint32_t count, std::uint32_t offset, std::uint32_t readonly_word) = 0;
    virtual void* other_index_map_0c(void*, std::uint32_t captured_target,
        std::uint32_t count, std::uint32_t offset, std::uint32_t readonly_word) = 0;
    virtual void other_index_unmap_10(void*, std::uint32_t captured_target) = 0;
    virtual void other_vertex_unmap_14(void*, std::uint32_t captured_target) = 0;
    // Pure metadata selection, tied to THIS parent. Distinct already-prepared
    // retained whole Glyph record with genuine nested frames for EVERY reached
    // call/recursion. No operand reads, Native work, reentry, allocation, FP
    // effect, reset/reuse, shortage/default/capacity or fallback policy.
    virtual NativeGuiTextGlyphQuadStorageAcquired& prepared_glyph_quad_record(
        NativeGuiTextWrappedStorageAcquired& parent) = 0;
    // WHOLE genuine4768D0, actual stream/out/index. Reuse the existing raw
    // read_native_vertex_position_004768d0 for its initialized cases using the
    // real retained half import. Its false result is an explicit boundary,
    // NEVER a successful no-op/default output or permission to rerun the body.
    // Other cases need the authentic callee private scratch/FP operation, not
    // a format gate, invented output or typed stream owner. No preliminary
    // classification read may replace the getter's own current read epochs.
    // The SAME ordinary caller float3 at S+54 is passed, with its prior bits.
    virtual void position_004768d0(void* captured_stream, std::uint32_t captured_index,
        float (&actual_output)[3], NativeGuiTextWrappedStorageAcquired& parent) = 0;
};
struct NativeGuiTextWrappedStorageContext final {
    NativeGuiTextWrappedStorageBindings& bindings;
    NativeGuiTextGlyphChildrenContext& children;
    NativeLogicalBufferMappingContext& mapping;
    NativeRenderActualOwnerRegistry& owners;
    NativeGuiTextGlyphQuadStorageContext& glyph_quad;
    // Authentic initialized original-data borrows, no copied constants/bank.
    const volatile float& minimum_00ce4970;
    const volatile float& maximum_00ce4adc;
    const volatile float& unsigned_bias_00ce3978;
    const volatile double& half_00d7a280;
    const volatile double& one_00d7a210;
    const volatile double& line_spacing_00ce6618;
    // Raw strings/widthCEC380/heightCEF1B8/currentE12FD4/actualF8BE50 borrow DIRECTLY
    // from glyph_quad, so parent and child use the SAME references/backing.
};

struct NativeGuiTextWrappedStorageAcquired final {
    enum class Phase : std::uint8_t { fresh, entered, complete, failed };
    enum class Boundary : std::uint8_t {
        none, clear, height, copy, model_geometry, vertex_stream,
        vertex_map, index_map, width, space_glyph, scan_glyph, scan,
        alignment, emit_glyph, texture0, texture1, glyph_record,
        glyph_quad, advance, line_metrics, vertical_metrics,
        position, position_store, index_unmap, vertex_unmap
    };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none}, escaped_boundary{Boundary::none};
    std::uint32_t native_site{}, escaped_site{};
    void* actual_text{};
    const volatile std::uint32_t* home_identity{};
    NativeGuiTextWrappedStorageScratch* private_image{};
    void* captured_model{};
    void* captured_mesh{};
    void* captured_vertex{};
    void* captured_index{};
    void* index_mapping{};
    void* selected_glyph{};
    std::uint32_t captured_profile{}, captured_target{}, map_count{}, position_index{};
    bool vertex_map_entered{}, vertex_map_returned{}, index_map_entered{}, index_map_returned{};
    bool index_unmap_entered{}, index_unmap_returned{}, vertex_unmap_entered{}, vertex_unmap_returned{};
    NativeGuiTextGlyphChildrenAcquired clear;
    NativeGuiTextGlyphQuadStorageAcquired* selected_quad{};
    std::exception_ptr failure;
    NativeGuiTextWrappedStorageAcquired() = default;
    NativeGuiTextWrappedStorageAcquired(const NativeGuiTextWrappedStorageAcquired&) = delete;
    NativeGuiTextWrappedStorageAcquired& operator=(const NativeGuiTextWrappedStorageAcquired&) = delete;
};
class NativeGuiTextWrappedStorageFrame final {
public:
    using Phase = NativeGuiTextWrappedStorageAcquired::Phase;
    // Metadata-only birth. Reads/initializes NONE of the two homes or private
    // image. Preserve their authentic bytes and addresses until disposition.
    NativeGuiTextWrappedStorageFrame(void* captured_actual_text,
        volatile std::uint32_t (&actual_homes)[2], NativeGuiTextWrappedStorageScratch&,
        NativeGuiTextWrappedStorageContext&, NativeGuiTextWrappedStorageAcquired&) noexcept;
    ~NativeGuiTextWrappedStorageFrame() noexcept = default;
    NativeGuiTextWrappedStorageFrame(const NativeGuiTextWrappedStorageFrame&) = delete;
    NativeGuiTextWrappedStorageFrame& operator=(const NativeGuiTextWrappedStorageFrame&) = delete;
    NativeGuiTextWrappedStorageFrame(NativeGuiTextWrappedStorageFrame&&) = delete;
    NativeGuiTextWrappedStorageFrame& operator=(NativeGuiTextWrappedStorageFrame&&) = delete;
    Phase phase() const noexcept { return phase_; }
    bool requires_process_retention() const noexcept {
        return phase_ == Phase::entered || phase_ == Phase::failed;
    }
private:
    friend void build_native_gui_text_wrapped_storage_00aba270(NativeGuiTextWrappedStorageFrame&);
    void* const text_;
    volatile std::uint32_t (&homes_)[2];
    NativeGuiTextWrappedStorageScratch& scratch_;
    NativeGuiTextWrappedStorageContext& context_;
    NativeGuiTextWrappedStorageAcquired& acquired_;
    volatile std::uint32_t quad_homes_[10]; // SAME call-site homes, refill only after normal return
    volatile std::uint16_t font_key_;
    NativeFontGlyphSelectionStorageScratch font_scratch_;
    // Ordinary retained CPU-register homes. Written only when Native writes
    // that register; separate from diagnostic records and physical arguments.
    std::uint32_t ebx_, esi_, ebp_, code_, glyph_;
    Phase phase_{Phase::fresh};
};

// WHOLE ABA270..ABA8C6[1623B/457ops/84x87], ECX actual Text, two DWORD homes,
// RET8. New ordinary Source ABI; native undefined(void) prototype is wrong.
// home0: source header -> savedCW LOWWORD -> all-spaces LOWBYTE -> line-end
// DWORD -> raised-height float. home1: CURRENT section -> span -> zero/offset.
// No snapshot/restoration of either home. Actual Text/header/stream/Font/glyph/
// Model/material/texture/mapping/provider/recursive graphs and all argument/
// scratch/metadata backing stay valid, stable, serialized and disjoint except
// the EXPLICIT internal aliases above; no private Native stack ABI is claimed.
// Height is captured MOVSX16 before copy; full signed DWORD bits reach Glyph.
// Position homes1/4/5 alias SAME S+68 float3, Y/Z written once per nonempty
// line and X before each call. Ten homes may be refilled only on normal return;
// historical acquired records then retain identity only, never follow reuse.
// Preserve actual staleS+28 preimage for every alignment, unsigned arithmetic,
// unguarded division, x87/SSE status/ambient rounding and ordered/unordered
// branches. Only Native FNSTCW/truncation/FISTP64/restore changes the CW.
// No C++ FP, FP reset, native local EH, B865A0, typed owner, range/depth/count
// gate, cleanup/unmap on escape, retry, rollback, retain or new terminal policy.
// Any escape retains entered graph/maps/homes/frame; phase/exception_ptr is
// retention metadata ONLY, not Native-vs-host origin or cleanup permission.
// Actual caller binding, undefined getter/private-scratch proof, FH3/SEH/fault/
// secondary failure, ABI/runtime and activation remain explicit gates.
void build_native_gui_text_wrapped_storage_00aba270(NativeGuiTextWrappedStorageFrame&);
} // namespace bsp
