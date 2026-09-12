#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS factory registration requires MSVC Win32.
#endif

namespace bsp {

// Complete BDAB20..BDAB52. Native ECX/EDX unconsumed; stacked(next,previous,
// factory-slot address); RET0C, EAX allocated0Ch node. Allocate through the
// existing CRT/new-handler service, write next then previous, then read the
// CURRENT factory slot for node+8. Individual destination null tests retain
// the native DWORD-address arithmetic. Source CRT/exception identity differs.
void* __stdcall allocate_native_vfs_factory_node_00bdab20(
    void* next, void* previous, void* const* actual_factory_slot);

// Complete BE0660..BE0692. Native ECX manager, EDX unconsumed, stacked factory;
// RET4; no semantic result. Manager list begins+30, sentinel+34, count+38.
// Capture sentinel and its previous node before allocation. Allocate BEFORE
// the BDECE0 library count check; on failure the native detached node is not
// reclaimed. On success set captured sentinel.previous, then reload current
// new-node.previous and set its next. Null and duplicate factories are stored.
// The exact unsigned3FFFFFFF subtraction guard and captured count store reuse
// the existing native SBO/NativeAliasListLengthError source transport. BDECE0
// remains a consumed library contract, not a new library reconstruction.
void __fastcall register_native_vfs_provider_factory_00be0660(
    void* actual_manager, void* unused_edx, void* factory);

// Native argument placement is represented above; original FH3/SEH, catch/RTTI
// identity, arbitrary caller-stack aliases and register/flag outputs are not
// reproduced. No host vector, new allocator domain, factory ownership or
// manager initialization is introduced. See docs/NATIVE_VFS_FACTORY_REGISTRATION.md.
} // namespace bsp
