#pragma once

#include "bsp/native_gui_text_failure_origin.hpp"
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual Text glyph-quad storage requires MSVC Win32 x87 assembly.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeGuiTextGlyphQuadStorageAcquired;

// Mandatory genuine whole raw providers in ONE retained Text/Model/renderer/
// font/material/pool/string domain. No implementation/default is provided.
// All nested nonmoving invocation/acquisition frames exist before entry;
// each method publishes custody in that prepared backing BEFORE callbacks.
// A typed GuiTextLifetime/GuiWidgetOwner cast, pending-status return, no-op,
// substitute model/font/material or successful unavailable-domain result cannot
// implement these methods. Reuse the existing whole actual Source callees.
// current methods consume the ALREADY captured profile/target as identity
// selectors; resolution is pure/stable metadata and must not reenter, reread
// a newer target, or cast a numeric DATA word to a callable.
class NativeGuiTextGlyphQuadStorageCalls {
public:
    virtual ~NativeGuiTextGlyphQuadStorageCalls() = default;
    virtual void* allocate_00ab78a0(void* actual_pool, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void* construct_00ab9650(void* actual_allocation, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    // Qualified used route MUST forward this SAME retained envelope to actual raw93.
    // No adapter/default implementation or Native-origin capability is supplied.
    virtual void* construct_00ab9650(void* actual_allocation, NativeGuiTextGlyphQuadStorageAcquired&,
        NativeGuiTextFailureConstructorCall&) = 0;
    virtual void return_failed_00ab76f0(void* current_slot, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void append_005dad40(void* actual_vector, volatile std::uint32_t* actual_child_home,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void* node_current10(void* actual_node, std::uint32_t captured_profile,
        std::uint32_t captured_target, std::uint32_t flags, std::uint32_t parent,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void scene_node_00aa6720(void* actual_child, void* actual_node,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void sections_00ab8530(void* actual_child, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void child_current34(void* actual_child, std::uint32_t captured_profile,
        std::uint32_t captured_target, std::uint32_t value, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void listeners_00aa6bc0(void* actual_child, void* actual_listener,
        std::uint32_t flag, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void font_00ab8c30(void* actual_child, const void* actual_name_header8,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void shadow_00ab6c30(void* actual_child, std::uint32_t enabled,
        std::uint32_t position, std::uint32_t offset_bits, const volatile std::uint32_t* color4,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void shader_00ab8e70(void* actual_child, const void* actual_name_header8,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    // AB81C0 is composed directly over SAME ordinary F24 backing and Context.strings.
    virtual void content_00aba8d0(void* actual_child, const void* actual_utf16_header8,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void child_current50(void* actual_child, std::uint32_t captured_profile,
        std::uint32_t captured_target, const void* actual_color4,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void attach_00aaa5a0(void* actual_parent, void* actual_child,
        std::uint32_t flag, NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void child_current30(void* actual_child, std::uint32_t captured_profile,
        std::uint32_t captured_target, const volatile std::uint32_t* actual_pivot2,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void position_00aa6750(void* actual_reference, volatile std::uint32_t* actual_output3,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
    virtual void position_00aa8240(void* actual_child, const volatile std::uint32_t* actual_input3,
        NativeGuiTextGlyphQuadStorageAcquired&) = 0;
};

// Pure borrows. All constants/literals must come from the existing authenticated
// original-data supplier; scale is the SAME mutable process-state E12FD4 ref,
// not the widget E12FC4 cell. Pool/empty UTF16 are the genuine existing Text
// process backing, never a new F8 bank, typed child owner or null fallback.
struct NativeGuiTextGlyphQuadStorageContext {
    NativeGuiTextGlyphQuadStorageCalls& calls;
    NativeStringRawPoolContext& strings;
    void* const actual_text_pool_00f8bdf0;
    const volatile std::uint16_t* const actual_empty_utf16_00f8be50;
    const char* const shader_name_00cefd78;
    const volatile float& one_00d7a24c;
    const volatile float& vertical_scale_00e12fd4;
    const volatile double& width_00cec380;
    const volatile double& height_00cef1b8;
    const volatile float& shadow_00d5c5c0;
    const volatile float& pivot_00ce3800;
    const volatile double& x_subtract_00d7a358;
    const volatile double& y_subtract_00d5c7a8;
    const volatile double& z_subtract_00d7a220;
    const volatile double& input_x_bias_00d06880;
    // Optional qualified own Source-runtime borrow; null preserves the legacy API.
    // Preparation only, not binding proof, Native permission or a catch-domain query.
    const NativeGuiTextFailureOriginRuntime* failure_runtime{};
};

// Fixed, caller-prepared per-invocation scratch/diagnostics, not an owner.
// locals[] corresponds to Native F14..F48; no blanket initialization occurs.
// Ordinary DWORD backing permits genuine raw string/wide helper access to the
// SAME objects; reached native local accesses use explicit volatile views.
// Its SAME F24/F28 words are coordinates, string/wide headers and pivot/offset
// storage at different epochs. Every recursive/reached call needs a distinct
// retained record and genuine nested provider frames. No destructor cleans up.
struct NativeGuiTextGlyphQuadStorageAcquired final {
    enum class Phase { fresh, entered, complete, failed };
    Phase phase{Phase::fresh};
    void* actual_text{};
    const volatile std::uint32_t* home_identity{}; // Identity ONLY after normal return/reuse.
    std::uint32_t locals[14];
    std::uint32_t captured_glyph{}, captured_stream{}, captured_first_vertex{};
    std::uint32_t first_position{}, captured_white{}, captured_fourth_vertex{};
    void* captured_allocation{};
    void* captured_child{};
    void* captured_receiver{};
    std::uint32_t captured_profile{}, captured_target{};
    std::uint32_t active_call_site{}, escaped_call_site{};
    std::int32_t native_eh_state{-1}, escaped_eh_state{-1};
    bool child_append_entered{}, child_append_returned{}, cleanup_entered{}, cleanup_returned{};
    bool cleanup_failed{};
    std::exception_ptr failure;
    NativeGuiTextGlyphQuadStorageAcquired() noexcept = default;
    NativeGuiTextGlyphQuadStorageAcquired(const NativeGuiTextGlyphQuadStorageAcquired&) = delete;
    NativeGuiTextGlyphQuadStorageAcquired& operator=(const NativeGuiTextGlyphQuadStorageAcquired&) = delete;
private:
    friend void write_native_gui_text_glyph_quad_storage_00ab98f0(void*,
        volatile std::uint32_t (&)[10], NativeGuiTextGlyphQuadStorageContext&,
        NativeGuiTextGlyphQuadStorageAcquired&);
    NativeGuiTextFailureOrigin failure_origin_;
    NativeGuiTextFailureConstructorSlotArm constructor_slot_;
    NativeGuiTextFailureConstructorCall constructor_call_;
};

// Whole AB98F0..AB9FC2 [1747B/485ops]. Original ECX actual Text; ten physical
// DWORD homes; RET28. New ordinary Source ABI; incoming EDX is not an input.
// homes: glyph,pos3*,vertexStream,indexWritePtr,pos3*,pos3*,quadIndex,
// unsigned fontheightDWORD,firstVertexDWORD,unsigned UTF16unitDWORD.
// The full callee never reads homes4/5 (zero-based). Both callers alias pos3
// at homes1/4/5. Preserve mutable actual homes: 0,2,7 are overwritten, and
// child append receives ADDRESS of live home7. No home snapshot/restoration.
void write_native_gui_text_glyph_quad_storage_00ab98f0(void* actual_text,
    volatile std::uint32_t (&actual_homes)[10], NativeGuiTextGlyphQuadStorageContext&,
    NativeGuiTextGlyphQuadStorageAcquired&);

// Ambient x87 CW/status are neither reset nor saved/restored. Actual x87 FST/
// FSTP/reloads, +1/-1 and native extended intermediates remain; no float-return
// helper or C++ FP arithmetic adds rounding. SSE/integer bit moves keep native
// widths/order. Valid storage, stable captured glyph/stream/Text and retained
// providers through callbacks are caller proof; no range/font/glyph policy.
// After normal return the caller may refill/reuse its SAME ten homes: historical
// records must not follow them. Failure retains caller homes/pos/streams/frames;
// no replay/advance, unlock, inferred cleanup, typed owner or activation follows.
// Private scratch aliases, original physical ABI/FH3/SEH/fault/secondary-unwind,
// provider/runtime/game identity remain qualified. ZERO extra Original/vendor/
// duplicate-body credit; genuine entered-helper bindings remain an explicit gate.
} // namespace bsp
