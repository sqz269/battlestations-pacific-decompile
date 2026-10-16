#pragma once

#include "bsp/native_gui_texture_normalization.hpp"
#include <cstdint>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Nonowning projection of already-held genuine D5D130/D61948/CE3978/CEC380/
// CEF1B8 data and the SAME real157 profile context. The supplied live scalar is
// the current cell corresponding to E1301C, NOT147's E12FC4 or a copied/default
// value. Construction checks held spans/known storage only, never a receiver
// or reached selector/value. Retain all these complete objects independently.
// SAME data-owner identity inside157's private context remains caller proof.
class NativeGuiFrameBoxGeometryContext final {
public:
    NativeGuiFrameBoxGeometryContext(const game::GameNativeReadOnlyData& data,
        const NativeGuiTextureNormalizationProfiles& normalization,
        const volatile float* live_scale_00e1301c);
    NativeGuiFrameBoxGeometryContext(const NativeGuiFrameBoxGeometryContext&) = delete;
    NativeGuiFrameBoxGeometryContext& operator=(
        const NativeGuiFrameBoxGeometryContext&) = delete;
private:
    friend struct NativeGuiFrameBoxGeometryAccess;
    const game::GameNativeReadOnlyData* data_;
    const NativeGuiTextureNormalizationProfiles* normalization_;
    const volatile float* live_scale_;
    const void* aliases_[5];
};

enum class NativeGuiFrameBoxGeometryPhase : std::uint32_t {
    fresh, entered, state_selected, state_returned, uv_captured,
    texture_selected, normalized, emitting, completed, failed
};
struct NativeGuiFrameBoxGeometryDiagnostics final {
    NativeGuiFrameBoxGeometryPhase phase{NativeGuiFrameBoxGeometryPhase::fresh};
    NativeGuiFrameBoxGeometryPhase failed_from{NativeGuiFrameBoxGeometryPhase::fresh};
    void* actual_box{};
    void* actual_stream{};
    const void* actual_size{};
    std::uint32_t initial_index{}, widget_profile{}, state_target{};
    const void* returned_record{};
    std::uint32_t saved_index_bits{}, count_begin{}, count_end{}, selected_begin{};
    const void* texture{};
};

// Distinct persistent private storage must exist before callbacks. The
// user-provided empty constructor initializes only Source diagnostics (including
// nested157's diagnostics); raw fake-F/capture storage remains uninitialized
// under BOTH default and value initialization. fake-F+08..83 reproduces the
// Original124B private scratch; 00..07 are unused spacing, never saved registers.
// No reset/copy/assignment/placement reuse or retry of a reached lifetime.
// These diagnostics/aliases own no graph; destruction performs no raw cleanup.
class NativeGuiFrameBoxGeometryFrame final {
public:
    NativeGuiFrameBoxGeometryFrame() noexcept {}
    NativeGuiFrameBoxGeometryFrame(const NativeGuiFrameBoxGeometryFrame&) = delete;
    NativeGuiFrameBoxGeometryFrame& operator=(
        const NativeGuiFrameBoxGeometryFrame&) = delete;
    const NativeGuiFrameBoxGeometryDiagnostics& diagnostics() const noexcept {
        return diagnostics_;
    }
    const NativeGuiTextureNormalizationDiagnostics& normalization_diagnostics()
        const noexcept { return normalization_.diagnostics(); }
private:
    friend struct NativeGuiFrameBoxGeometryAccess;
    NativeGuiFrameBoxGeometryDiagnostics diagnostics_{};
    NativeGuiTextureNormalizationFrame normalization_;
    alignas(4) std::uint32_t native_frame_[0x84/4];
    volatile std::uint32_t captured_index_;
};

// Whole ACF9B0 [ACF9B0,ACFF69),1465B/351 starts. Ordinary C++ Source interface,
// NOT Native ECX-box/stack-stream,size/RET8 ABI. Supported entry profile is the
// genuine current D5D130/+98 AD2750 route; captured/current record rules and
// real BF6713 are preserved. Only genuine qualified2D textures/157 and actual
// mapped-stream160 are supplied, without typed owners or callback substitutes.
//
// Native/public Box120h, stream's observed2Ch header, size8 and live scalar4
// may overlap one another where genuine provenance/backing permits feedback.
// Source preflight protects complete frame/context/data/profile metadata from
// these regions and held aliases; it does NOT snapshot public geometry/layout.
// Caller independently retains the full actual state/vector/texture/resource/
// creator/Model/mesh/stream/map/device/layout/SDK-handler graph and all contexts,
// private caller/callee backing and distinct recursive frames BEFORE callbacks
// and through failure. The complete mapped backing/current tables/records must
// be valid for every reached wrapped access and may not alias Source-private
// storage. Known checks cannot prove opaque graph/placement/lifetime authority.
//
// All native scratch and normalized output stay private. Exact math/spills and
// nine real160 calls share one asm block, preserving XMM3..7 across that actual
// verified body. No live x87 crosses a throwing Source admission. Source phases,
// guards/catch and physical helper frames are added boundaries; no arbitrary
// FPU/fault/Native CRT/FH3/SEH/private-ABI/game or production-admission claim.
// Entered ordinary C++ failure is sticky, with no cleanup/rollback/replay or
// automatic resource return. Full AD0D80/owner/clip behavior remains separate.
void write_native_gui_frame_box_geometry_00acf9b0(void* actual_box,
    void* actual_stream, const void* actual_size,
    const NativeGuiFrameBoxGeometryContext& context,
    NativeGuiFrameBoxGeometryFrame& frame);

} // namespace bsp
