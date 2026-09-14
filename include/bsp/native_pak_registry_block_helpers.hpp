#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native PakRegistry block helpers require MSVC Win32.
#endif

namespace bsp {
class NativeStringStorage;

// Full BB5670..BB5760[241]. Native ECX=actual output8h length/data header,
// EDX=actual input8h header, EAX=output, RET. Compare the FIRST case-sensitive
// strstr(".mpak") position (FFFFFFFF when absent) with storedLength-5 using
// DWORD wrap. On equality, copy-construct; otherwise construct a temporary
// suffix, concatenate input+suffix, then release that current temporary.
// Both paths clear output before the identity branch, abandoning any old
// output buffer. An input/output identity therefore yields empty on equality
// and just ".mpak" otherwise. A separate length4/no-match input is copied.
// The temporary becomes cleanup-owned only after successful construction;
// output cleanup during concat remains the existing 4261A0 dependency's job.
void* construct_native_pak_block_name_00bb5670(void* actual_output_header,
    const void* actual_input_header, NativeStringStorage& strings);

// Full actual-manager leaves. Counter DWORD+1C wraps; byte+20 is read/written
// in the supplied manager storage. They do not capture/resolve0109CEEC: the
// observer must reload that actual publication separately before each call.
// BD9200[15]: ECX=manager, RET; decrement, reread, byte=(signed count>0).
void decrement_native_vfs_block_count_00bd9200(void* actual_manager) noexcept;
// BD9210[10]: ECX=manager, stack DWORD's low byte, RET4. No Boolean coercion.
void set_native_vfs_block_active_00bd9210(void* actual_manager,
    std::uint32_t byte_word) noexcept;
// BD9220[11]: ECX=manager, RET; byte=(current signed DWORD+1C>0).
void recompute_native_vfs_block_active_00bd9220(void* actual_manager) noexcept;
// BD9FA0[12]: ECX=manager, RET; increment DWORD+1C, unconditionally byte=1.
void increment_native_vfs_block_count_00bd9fa0(void* actual_manager) noexcept;

// These source interfaces add explicit string storage and omit native EAX/AL
// residue on the void leaves. They are not original binary/FH3/SEH ABI bridges.
// NativeStringStorage supplies the existing bound allocation/noexcept-release
// domain; hardware faults, raw getter throws and unsupported buffer overlap
// remain its dependency boundaries. Evidence: NATIVE_PAK_REGISTRY_BLOCK_HELPERS_BN.md.
} // namespace bsp
