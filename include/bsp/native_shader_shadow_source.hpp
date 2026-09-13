#pragma once
#include "bsp/native_shader_struct_declarations.hpp"
#include <optional>

namespace bsp {
// Persistent host metadata, not added to the actual B0h builder. The existing
// B34F20 child retains its actual temporary headers and acquisition preimages.
// Keep frame/context/builder/string domain alive and exclude their retirement
// externally while running/failed. Existing helper cleanup is its own boundary;
// an entered header is not proof that its allocation remains owned. No FH3 or
// native terminal interception is installed; failed work cannot be replayed.
struct NativeShaderShadowSourceOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t function{}, native_site{}, completed_lines{};
    std::uint8_t sampled_shadow_byte{};
    bool shadow_byte_captured{};
    NativeMaterialProgramBuilderStorage* builder{};
    NativeShaderConstantHeaderContext* context{};
    const char* text{};
    std::optional<NativeShaderStructDeclarationOperation> line;
    NativeShaderShadowSourceOperation() = default;
    ~NativeShaderShadowSourceOperation();
    NativeShaderShadowSourceOperation(const NativeShaderShadowSourceOperation&) = delete;
    NativeShaderShadowSourceOperation& operator=(const NativeShaderShadowSourceOperation&) = delete;
    // Only AFTER manually resolving every retained child acquisition. Frees
    // nothing; acknowledges both parent and failed child for diagnostic retirement.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// B38230/B382B0: ECX actual B0h builder, no stacked args, plain RET. Complete
// normal bodies. Append intro via existing B34F20, THEN read byte AA once.
// Nonzero chooses projected (four lines total), zero filtered (three lines).
// Reuses exact shader_shadow_literals.inc text; no semantic builder projection.
// Explicit C++ interfaces, not original binary ABI replacements.
void append_native_shadow_helper_00b38230(NativeMaterialProgramBuilderStorage&,
    NativeShaderConstantHeaderContext&, NativeShaderShadowSourceOperation&);
void append_native_map_shadow_helper_00b382b0(NativeMaterialProgramBuilderStorage&,
    NativeShaderConstantHeaderContext&, NativeShaderShadowSourceOperation&);
} // namespace bsp
