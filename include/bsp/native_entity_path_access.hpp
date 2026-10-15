#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw entity/path access requires MSVC Win32.
#endif

namespace bsp {

// Complete 007AC9D0..007ACA2C, 93B. Original ECX entity, no stacked argument,
// plain RET/EAX pointer. The Source EDX placeholder is not an Original input.
// Null returns null without touching storage. Otherwise capture the receiver,
// reload its CURRENT vtable/slot+5Ch before each reached kind47h/48h/49h/4Ah
// call, restore ECX receiver after earlier predicates, and consume only AL.
// These are actual indirect calls: ECX receiver, EDX loaded target, one stacked
// kind DWORD, callee RET4. The backed current slot must be genuinely callable
// under that raw register/stack contract and preserve callee-saved registers.
// First true kind47/4A returns entity+1E4h; kind48 +170h; kind49 +310h; else
// null. Address adjustment is literal DWORD LEA, including modular wrap.
void* __fastcall get_native_entity_path_interface_007ac9d0(
    void* actual_entity, void* unused_edx);

// Complete 00923810..00923830, 33B. Original __thiscall ECX entity plus signed
// DWORD levels, RET4/EAX pointer. Source EDX placeholder is not an Original
// input. Read first parent at +3Ch unconditionally, decrement levels modulo32,
// then while parent nonnull and remaining signed-positive read its next +3Ch
// and decrement again. Zero/ordinary negative levels still return first parent;
// INT_MIN wraps to positive and can walk farther. Null receiver has no guard.
// Every reached parent cell must be backed. Cycles consume the finite signed
// count without cycle detection. No C++ signed-overflow arithmetic is used.
void* __fastcall get_native_entity_ancestor_00923810(
    void* actual_entity, void* unused_edx, std::int32_t levels);

// Raw storage and literal x86 entry contracts, not a host entity façade or
// recovered Native class/factory/vtable identity. Existing semantic APIs stay
// distinct. Exact body bytes alone do not prove Native virtual deployment,
// exception/SEH/CRT integration, runtime differential or gameplay equivalence.
// Evidence: docs/GOAL15H_ENTITY_PATH_ACCESS.md.

} // namespace bsp
