#pragma once

#include <cstdint>

// Address-bearing source hypotheses, not recovered class/field/function names.
// OriginalABI and complete machine-state effects are documented per entry in
// docs/GOAL15P_TAGGED_LEAF_B4_03.md. No unused EDX/stack inputs are fabricated.
extern "C" {
// Qualified semantic Source; whole emitted bodies differ from Native machine state.
void* __fastcall bsp_tagged_b4_00b40120(void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40180(void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40190(void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40e80(const void* storage) noexcept;
// Exact whole-body instruction/operand matches under the recorded Win32 flags.
std::uint32_t __cdecl bsp_tagged_b4_00b40150() noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40160(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40170(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40580(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40740(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40850(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40970(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b41030(const void* storage) noexcept;
}
