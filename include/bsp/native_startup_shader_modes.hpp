#pragma once
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
// Source counterparts of four distinct native loader-zero bytes. Borrow these
// SAME cells in compiler/cache/reload consumers; do not recalculate options.
struct NativeStartupShaderModes {
    volatile std::uint8_t load_variants_0108d6f0{};
    volatile std::uint8_t source_mode_0108d6f1{};
    volatile std::uint8_t hires_mode_0108d4ba{};
    volatile std::uint8_t reload_resources_0108d4bb{};
};
struct NativeStartupShaderModeLiterals {
    const char* genshaders_00ce7f7c;
    const char* devshaders_00ce7f70;
    const char* hiresmode_00ce7f98;
    const char* reloadresources_00ce7f88;
    const char* devrr_00ce7f68;
};
struct NativeStartupShaderModeOperation final {
    enum class Phase { fresh, running, complete, failed };
    Phase phase{Phase::fresh};
    NativeString temporary;
    std::uint32_t native_site{}, completed_copies{};
    bool temporary_live{};
    NativeStartupShaderModeOperation()=default;
    ~NativeStartupShaderModeOperation();
    NativeStartupShaderModeOperation(const NativeStartupShaderModeOperation&)=delete;
    NativeStartupShaderModeOperation& operator=(const NativeStartupShaderModeOperation&)=delete;
};
// Exact normal fragment73D4C2..73D603 inside73D410: native EBP captures the
// second stacked initialize argument. Three genuine pooled copies, five CRT
// case-sensitive substring searches, native signed DWORD-offset predicates,
// four byte assignments and conditional devrr override of source/reload only.
// No tokenization, '-' requirement, option normalization or cache creation.
// Keep mode, literals, shared raw pool and operation alive; failure retains
// completed mutations/acquisitions rather than emulating original FH3 unwind.
void apply_native_startup_shader_modes_0073d4c2(const char* mode,
    NativeStartupShaderModes&,const NativeStartupShaderModeLiterals&,
    NativeStringRawPoolContext&,NativeStartupShaderModeOperation&);
} // namespace bsp
