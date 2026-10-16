#pragma once

#include <cstdint>

namespace bsp {
namespace game { class GameNativeReadOnlyData; }

// Borrows the SAME initialized data owner and genuine D61948/CE3978/CEC380/
// CEF1B8 aliases. Construction only checks already-held spans; it neither maps
// a missing band nor reads a texture/output. Retain the complete context/data
// owner and aliases through every consumer and failure. This is no owner/token.
class NativeGuiTextureNormalizationProfiles final {
public:
    explicit NativeGuiTextureNormalizationProfiles(
        const game::GameNativeReadOnlyData& data);
    NativeGuiTextureNormalizationProfiles(
        const NativeGuiTextureNormalizationProfiles&) = delete;
    NativeGuiTextureNormalizationProfiles& operator=(
        const NativeGuiTextureNormalizationProfiles&) = delete;
private:
    friend struct NativeGuiTextureNormalizationAccess;
    const game::GameNativeReadOnlyData* data_;
    const volatile std::uint32_t* texture_2d_;
    const float* uint_bias_;
    const double* width_divisor_;
    const double* height_divisor_;
};

enum class NativeGuiTextureNormalizationPhase : std::uint32_t {
    fresh, entered, width_selected, width_queried, height_selected,
    height_queried, completed, failed
};

// Diagnostics describe Source progress, not Native EH states or proof that
// external actual storage/creator lifetimes are genuine. They own no graph.
struct NativeGuiTextureNormalizationDiagnostics final {
    NativeGuiTextureNormalizationPhase phase{
        NativeGuiTextureNormalizationPhase::fresh};
    NativeGuiTextureNormalizationPhase failed_from{
        NativeGuiTextureNormalizationPhase::fresh};
    void* output{};
    const void* texture{};
    std::uint32_t width_profile{}, width_target{}, width_value{};
    std::uint32_t height_profile{}, height_target{}, height_value{};
};

// Prepare distinct persistent storage before entry/callbacks. Construction
// initializes Source diagnostics only; the actual eight-byte Native scratch
// stays uninitialized until reached writes. No copying, resetting, placement
// reuse or retry of a reached lifetime is permitted. Destruction is metadata
// destruction only and never cleans a failed actual graph.
class NativeGuiTextureNormalizationFrame final {
public:
    NativeGuiTextureNormalizationFrame() noexcept {}
    NativeGuiTextureNormalizationFrame(
        const NativeGuiTextureNormalizationFrame&) = delete;
    NativeGuiTextureNormalizationFrame& operator=(
        const NativeGuiTextureNormalizationFrame&) = delete;
    const NativeGuiTextureNormalizationDiagnostics& diagnostics() const noexcept {
        return diagnostics_;
    }
private:
    friend struct NativeGuiTextureNormalizationAccess;
    NativeGuiTextureNormalizationDiagnostics diagnostics_{};
    alignas(4) volatile std::uint32_t scratch_[2];
    volatile std::uint32_t captured_height_profile_;
    volatile std::uint32_t captured_height_target_;
};

// Whole ACEB70 [ACEB70,ACEBD8),104B/36 starts over genuine qualified actual2D
// storage. Source interface, NOT Native ECX-output/EDX-texture/plain-RET ABI.
// Require live DWORD-aligned texture50h/output8 and readable current numeric
// tables through +4C. At each reached selector only genuine D61948 with current
// B3CE70/B3CE80 is admitted; actual saved +34/+38 leaves supply the values.
// Output MAY overlap texture, including its header: BOTH queries/conversions
// precede the two separate x87 output stores; no texture read follows them.
// All other known regions must be disjoint. Entry checks known context/data/
// aliases/frame/texture/output ranges before entered writes, not transitive
// graph/creator/placement provenance. Caller retains that full independent
// graph and all private backing/services through completion or escaping failure,
// with serialized exclusion and no aliases into callee/private Source storage.
// No ownership/count/cleanup, rollback, fallback or lazy mapping is added.
// Entered C++ failures are sticky; preflight failures leave the frame fresh.
// Hardware/FPU faults, SEH/FH3, private ABI and runtime activation are unclaimed.
void* normalize_gui_texture_size_00aceb70(void* output,
    const void* actual_texture,
    const NativeGuiTextureNormalizationProfiles& profiles,
    NativeGuiTextureNormalizationFrame& frame);

} // namespace bsp
