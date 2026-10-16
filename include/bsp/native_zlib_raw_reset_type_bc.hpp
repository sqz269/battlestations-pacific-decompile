#pragma once

#include <cstdint>

// New ECX-only raw-backing interfaces, not recovered z_stream/deflate layouts
// or Original declaration spellings. No caller/callback adoption is implied.
extern "C" std::int32_t __fastcall bsp_native_zlib_raw_reset_00bc9650(void* stream_backing) noexcept;
extern "C" void __fastcall bsp_native_zlib_raw_type_00bcee90(void* state_backing) noexcept;
