#pragma once

#include <cstdint>

// New raw interfaces; names and formal types are observed-operation hypotheses.
extern "C" std::uint32_t __cdecl bsp_tagged_bcbd_16k_00bc04b0() noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3030(const void* storage) noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3080(const void* storage) noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc3100(const void* storage) noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_bcbd_16k_00bc7a60(const void* storage) noexcept;
extern "C" std::uint8_t __cdecl bsp_tagged_bcbd_16k_00bd1830() noexcept;
