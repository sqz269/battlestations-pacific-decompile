#pragma once
#include "bsp/native_render_resource_init_post664.hpp"

namespace bsp {
struct NativeRenderResourceInitTailNames {
    const char* wave_00d5e1e4; // Passtrough_Wave.mshd
    const char* clear_00d5e1d0; // cleartargets.mshd
};

// ONLY the normal resource tail [B12C9A,B13010). The following 25-byte native
// register/SEH/RET0C epilogue is observed evidence, not a source ABI wrapper.
// Retain both independent canonical blocks, all names, predecessors and SAME
// original providers/contexts through parameter borrows and dependent work.
// Current focus owners/root/node storage must satisfy the existing raw-domain
// helper contracts. No extra owner credit or lifetime admission is implied.
struct NativeRenderResourceInitTailState final {
    enum class Phase { fresh, preparing, running, normal_work_complete_at_b13010, failed };
    NativeRenderResourceInitTailState() = default;
    NativeRenderResourceInitTailState(const NativeRenderResourceInitTailState&) = delete;
    NativeRenderResourceInitTailState& operator=(const NativeRenderResourceInitTailState&) = delete;
    Phase phase{Phase::fresh};
    NativeRenderResourceInitPost664State* previous{};
    const NativeRenderResourceInitTailNames* names_identity{};
    NativeRenderResourceInitEntryState* entry_identity{};
    const volatile NativeRenderResourceInitArguments* argument_cells_identity{};
    std::uint32_t native_site{};
    int native_state{-1};
    std::uint32_t temporary_mask_esp10{},ebx_bits{};
    void* raw_wave{};
    void* returned_wave{};
    void* raw_clear{};
    void* returned_clear{};
    bool field654_published{},field660_published{};
    NativePostEffect20ConstructionBlock wave;
    NativePostEffect20ConstructionBlock clear;
    std::array<NativeString,7> names; // Wave effect, five parameters, clear effect.
    std::array<bool,7> name_constructed{},name_return_started{},name_returned{};
    std::array<const void*,5> parameter_sources{};
    std::array<void*,5> parameter_materials{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};
    NativeFrameTargetOwnerStorage* captured_frame{};
    void* captured_frame_post{};
    // POP EDI B12FB0 and POP EBX B12FBE are observations only. After these,
    // continuation.edi_bits / ebx_bits retain LAST PRE-RESTORE values, not the
    // unknown saved caller values. Neither value is consumed again here.
    bool native_edi_restore_observed{},native_ebx_restore_observed{};
    bool first_focus_gate_sampled{},second_focus_gate_sampled{};
    std::uint8_t first_focus_gate{},second_focus_gate{};
    void* captured_focus_batch{};
    void* captured_focus_root_owner{};
    bool focus_batch_marked{},focus_root_invalidated{};
};

// Consume the exact B12C9A predecessor once, preserving inherited EBX=service4,
// EBP=-1 and raw664 EDI/ESP14 until their actual replacements. Configure+654,
// construct separate+660 and perform exact current gate/owner focus calls.
// Successful clear-name cleanup leaves mask400 set, as native does. Record
// register restore sites without fabricating saved values. Stop at B13010;
// no argument read, native epilogue/RET, full initializer/lifetime, teardown,
// FH3/SEH, physical private-stack alias, composed runtime or game claim.
void continue_native_render_resource_init_00b12c9a_fragment(
    NativeRenderResourceInitPost664State&, NativeRenderResourceInitContinuationContext&,
    const NativeRenderResourceInitTailNames&, NativeRenderResourceInitTailState&);
} // namespace bsp
