#pragma once

#include <cstdint>

// New raw interfaces matching measured physical inputs. No original class,
// formal return types or semantic field ownership is inferred.
extern "C" std::uint32_t __fastcall bsp_tagged_b78_16g_00b726a0(std::uint32_t address_bits) noexcept;
extern "C" std::uint32_t __fastcall bsp_tagged_b78_16g_00b72b30(const void* storage) noexcept;
extern "C" void __fastcall bsp_tagged_b78_16g_00b87080(void* destination, const void* source) noexcept;
extern "C" void __fastcall bsp_tagged_b78_16g_00b87110(void* destination, const void* source) noexcept;
extern "C" void* __stdcall bsp_tagged_b78_16g_00b875e0(void* destination, const void* source) noexcept;
extern "C" void* __stdcall bsp_tagged_b78_16g_00b87610(void* destination, const void* source) noexcept;
extern "C" void* __fastcall bsp_tagged_b78_16g_00b88b50(void* storage) noexcept;
