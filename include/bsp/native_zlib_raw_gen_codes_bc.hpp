#pragma once

#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
// New returning cdecl entry for complete 00BCF140 gen_codes__prov. Three real
// transport words select custom EDX counts / EBX signed max_code / EDI tree.
// A real private CALL supplies the actual kernel frame; no borrowed ESP, copied
// scratch, fake input/provider, guard, saved continuation or noreturn exists.
// Raw lengths may address real inner/outer returns, wrapper saves and arguments.
// A changed inner RET can skip the wrapper suffix. All such aliases remain real;
// ordinary well-formed return is not a universal Source/Original ABI guarantee.
// Formal Original types/layout, fault frames, SEH, callers and game are unproved.
// Full SS-default accesses/partial registers/flags: GOAL16AB documentation.
extern "C" void __cdecl bsp_native_zlib_gen_codes_00bcf140(
    const void* counts, std::int32_t max_code, void* tree) noexcept;
#endif
