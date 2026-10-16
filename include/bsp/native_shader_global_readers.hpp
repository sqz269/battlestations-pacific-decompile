#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native shader global storage requires MSVC Win32.
#endif

namespace bsp {

struct NativeShaderStateDefinitionsStorage;

// Canonical mutable Source storage for the actual Native DWORD at 00E13078.
// Its image preimage is 0000004D. Constant initialization makes it available
// before GameNativeShaderProcess is constructed, without allocating a process
// or duplicating its previous live field. Production Source users share this cell.
volatile std::uint32_t& native_shader_register_limit_cell_00e13078() noexcept;

// Whole 00B343F0..00B343F5: MOV EAX,[00E13078]; RET. No consumed register or
// stack arguments. This reads the canonical Source cell, with an address
// relocation rather than claimed placement at the Original Native address.
std::uint32_t __cdecl native_shader_register_limit_00b343f0() noexcept;

// Same single live Source publication borrowed by the production renderer's
// constructor, compiler and destructor contexts. Its image preimage is null;
// binding a renderer to this cell does not reset an existing publication.
void* volatile& native_shader_system_registry_cell_0108fe94() noexcept;

// Whole 00B34490..00B34495: MOV EAX,[0108FE94]; RET, with no consumed inputs.
// Return the current pointer bits without dereferencing, guarding or creating
// a registry/renderer. Original Native placement and caller adoption unproved.
void* __cdecl native_shader_system_registry_00b34490() noexcept;

// The existing production registry owner and descriptor contexts borrow this
// one live publication. Zero initialization precedes dynamic owner creation;
// binding a reference preserves an earlier publication rather than resetting it.
NativeShaderStateDefinitionsStorage* volatile&
native_shader_state_definitions_cell_0108fe90() noexcept;

// Whole 00B400F0..00B400F5: MOV EAX,[0108FE90]; RET, no consumed inputs.
// Return the current pointer bits without dereferencing or creating an owner.
// Source storage binding does not assert Original placement or caller adoption.
void* __cdecl native_shader_state_definitions_00b400f0() noexcept;

} // namespace bsp
