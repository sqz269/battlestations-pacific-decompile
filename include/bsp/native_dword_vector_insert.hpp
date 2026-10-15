#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD vector insertion requires MSVC Win32.
#endif

namespace bsp {
// Whole 00697990: both normal ranges, 550 bytes / 216 instructions. Native
// ECX=receiver, incoming EDX unconsumed; FOUR stacked DWORDs are unused owner,
// position, unsigned count and value address. All three exits RET10h. Its only
// caller discards EAX; the Source return is void. Header opaque/begin/end/capacity
// offsets are 0/4/8/C; opaque0 is untouched. Elements are unowned 4-byte DWORDs.
//
// Eagerly capture the pointed DWORD and current capacity even for count0.
// Preserve x86 SAR2 distances, unsigned comparisons and wrapping arithmetic.
// The growth path reuses actual allocation/length error, forward copy and
// fill_n; reloads CURRENT header fields; frees CURRENT old backing; publishes
// capacity, end, begin. Its catch 00697AF1 frees ONLY saved new backing/rethrows.
// The short-tail path copies, fills uninitialized storage, adds to CURRENT end,
// then fills the gap. Its catch enters INTERIOR00697AFD and only rethrows.
// The long-tail path publishes the copied end BEFORE backward move/gap fill.
// No rollback, scalar destructor, provider, extra guard or new storage model.
void __fastcall insert_native_dword_vector_count_00697990(
    void* receiver, std::uint32_t unused_edx, void* unused_owner, void* position,
    std::uint32_t count, const void* value);

// Reached raw storage must support the actual accesses and finite helper
// equality termination; header must survive old-backing free and publication.
// Current CRT allocation/free/new-handler and accepted owning C++ length-error
// transport apply. Both native catch scopes map lexical Current C++ catches;
// 00697AFD remains a shared interior fragment, never a whole ordinary function.
// Source/private argument/prototype aliases, ambient dead tag bits, original
// FH3/RTTI/catch/heap identity, asynchronous mutation, native faults/SEH, register
// residue and game ABI/runtime equivalence are not established. Established STL
// identities and older qualified interfaces remain separate; new names are
// hypotheses. No original EBP frame or native handler implementation is added.
} // namespace bsp
