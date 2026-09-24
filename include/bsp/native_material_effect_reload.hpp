#pragma once
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_startup_shader_modes.hpp"
#include <array>

namespace bsp {
struct NativeMaterialEffectReloadContext {
    NativeMaterialEffectProgramsContext& programs;
    const game::GameNativeReadOnlyData& original_data;
    // Borrow the canonical startup shader state used by the compiler/cache.
    // Its variant byte must also be the one borrowed by programs.
    const NativeStartupShaderModes& shader_modes;
};

// Metadata for one invocation. Construct before entering the native operation
// and retain through deletion of every descriptor acquired by either load,
// including after success or failure. The program frames retain their actual
// native acquisitions; destroying this frame is not effect cleanup or rollback.
struct NativeMaterialEffectReloadFrame final {
    enum class Phase { fresh, release_derived, release_base, load_name,
        release_failed_derived, release_failed_base, make_fallback,
        load_fallback, complete, failed };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    std::uint32_t exception_state{0xffffffffu};
    std::array<NativeMaterialEffectProgramOperation,2> loads;
    NativeString fallback_name;
    bool fallback_live{};
};

// Complete B469A0..B46A6E. Original ECX effect, no stack operands, RET.
// Gate 108D6F1; derived then base release; B46950(CURRENT effect+B8).
// A false AL repeats both releases and loads the literal D61C88 on the SAME
// effect, ignoring its result. Only the fallback header has native FH3 cleanup.
// Requires initialized native pass slots and the same canonical owners/strings
// as the original admission. Current produced descriptors carry a callable
// teardown binding; raw numeric D61A44 descriptors require a separate
// recovered terminal provider and are rejected before a host vtable call.
// Native exceptions and binary ABI are not replaced by this C++ interface;
// failed children retain their acquisition evidence.
void reload_native_material_effect_00b469a0(NativeMaterialEffectStorage&,
    NativeMaterialEffectReloadContext&, NativeMaterialEffectReloadFrame&);
} // namespace bsp
