#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual DWORD vector placement wrappers require MSVC Win32.
#endif

namespace bsp {

// Whole 00697D40..00697D7F:64B/17 instructions. Native ECX destination,
// EDX source, no stacked arguments, plain RET. Nonnull forwards the original
// two addresses to actual 00557590; null skips the child entirely.
void __fastcall place_native_checked_dword_vector_copy_00697d40(
    void* destination, const void* source);

// Whole 00697EC0..00697F07:72B/19 instructions. Native destination/source are
// TWO stacked words, RET8; incoming ECX/EDX are not semantic inputs. Native
// reads the source argument only on the nonnull branch. This is a distinct
// physical interface, despite the historical same-body ledger wording.
void __stdcall place_native_checked_dword_vector_copy_00697ec0(
    void* destination, const void* source);

// Native null paths leave EAX=previous FS0; nonnull paths leave the child
// destination result. Source void promises neither incidental residue nor a
// fabricated nullptr result. No source object read, allocator, validation,
// additional guard, rollback, catch/free or provider is introduced here.
// The child's ignored EDX is defined as zero; ambient register identity is
// not a semantic input. All actual constructor raw-storage/alias and returning
// diagnostic/exception preconditions remain those of accepted 00557590.
//
// Logical native placement-unwind scopes C7EBD0/C7EBF0 (17B/7 ops each) are
// effect-elided: exact raw maps select their private-frame loads/two pushes,
// literal 00401130 C3 RET and balanced ADD ESP,8/RET. No public native EBP,
// frame adapter, action function, destructor or unresolved stub is exposed.
// Constructor exceptions propagate; the actions add no public cleanup on
// admitted valid private frames/stacks. Their fault/alias/register effects
// and original FH3 execution are not Current C++ behavior proof.
//
// New MSVC Win32/C++ interfaces inherit accepted current CRT/exception policy.
// Original private frame/argument aliases, FS chain, FH3/RTTI, SEH/fault,
// heap/CRT, ambient registers/flags, mangling, binary ABI and gameplay identity
// remain unproved. Existing WordVectorHooks/typed APIs and native names stay
// intact; these descriptive C++ names are hypotheses.

} // namespace bsp
