#pragma once

#include <cstdint>

// Address-bearing hypotheses; no recovered class or field identity is claimed.
// OriginalABI: ECX only -> complete EAX, plain RET, no callee argument cleanup.
// The native bodies consume neither EDX, incoming EAX nor stack arguments.
extern "C" {
std::uint32_t __fastcall bsp_tagged_b4_00b401c0(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40300(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40310(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40490(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40510(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40550(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40560(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40570(const void* storage) noexcept;
}
