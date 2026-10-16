#pragma once

#include <cstdint>

// Address-bearing hypotheses, not recovered game symbols or class identities.
// Native physical contract: ECX input, EAX result, RET without stack cleanup;
// EDX and stack arguments are not consumed. See GOAL15M_TAGGED_LEAF_B4_01.md.
extern "C" {
std::uint32_t __fastcall bsp_tagged_b4_00b40000(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40010(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40020(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40030(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40070(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40080(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b400a0(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b400d0(std::uint32_t ecx_value) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b400e0(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40100(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40130(const void* storage) noexcept;
std::uint32_t __fastcall bsp_tagged_b4_00b40140(std::uint32_t ecx_value) noexcept;
}
