#pragma once

#include <cstdint>

namespace bsp {
// Complete original __ValidateImageBase, C16D50..C16D78 (41 bytes).
// MSVC Win32 cdecl: [entry ESP+4] is the actual borrowed image-base pointer;
// RET leaves its one word for caller cleanup. Returns EAX=0/1 after testing
// MZ, PE signature and PE32 optional-header magic, in that order. Reads and
// address additions have the original fault and wrapping-32-bit behavior.
// ECX is the image pointer on an early rejection, otherwise the result;
// EDX and nonvolatile registers are untouched. Final flags are from XOR EAX
// on early rejection or the optional-magic CMP. DF is unchanged.
std::int32_t __cdecl validate_native_crt_image_base_00c16d50(
    const void* image_base);

// Complete original __FindPESection, C16D80..C16DC1 (66 bytes).
// MSVC Win32 cdecl: [entry ESP+4]=actual borrowed image base, +8=32-bit RVA;
// RET leaves both words for caller cleanup. Returns the actual first matching
// 40-byte section-header address in EAX, or null. Uses VirtualAddress and
// VirtualSize with original unsigned comparisons and wrapping addition.
// Saves/restores EBX/ESI/EDI, leaves EBP unchanged; EAX/ECX/EDX and arithmetic
// flags have the exact native path effects. DF is unchanged. It does not
// validate the signatures or supply bounds checks, page protection or SEH.
const void* __cdecl find_native_crt_pe_section_00c16d80(
    const void* image_base, std::uint32_t rva);

// Both entries read caller-owned live memory, including possibly malformed
// headers. They create no image/global owner and install no exception frame.
// The C16DD0 fixed-00400000 image, scope/funclets and C07C90 dispatcher cycle
// remain separate dependencies. Source/static proof is not gameplay proof.
} // namespace bsp
