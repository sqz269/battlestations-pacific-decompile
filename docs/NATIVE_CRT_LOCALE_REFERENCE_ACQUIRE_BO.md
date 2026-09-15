# Native locale reference acquisition

This packet implements the complete 134-byte `___addlocaleref` body at
`00C0190B..00C01990` through a qualified MSVC Win32 source interface. It borrows
the actual locale record and actual narrow-C-locale sentinel identity. It does
not create locale/PTD state or supply a heap, lock, release, or error callback.

The implementation is `src/native_crt_locale_reference_acquire.cpp`, with its
public contract in `include/bsp/native_crt_locale_reference_acquire.hpp` and
verification evidence in `reports/native_crt_locale_reference_acquire_bo.json`.
The original library name is preserved; no Ghidra state was changed.

## Native order and alias behavior

The function caches the actual `InterlockedIncrement` import from native IAT
slot `00CE221C` in EDI once. The source declares that real KERNEL32 export using
its Win32 LONG/volatile LONG*/stdcall ABI, avoiding SDK intrinsic macros. The
compiler object retains the imported call boundary, not an inline counter
operation or a caller-supplied callback.

It atomically increments the DWORD at the root record first. Next it loads and,
when nonnull, increments the pointed-to counts at offsets B0h, B8h, B4h, and
C0h, in precisely that order. Each field load follows the preceding call.

It then performs six iterations with q = 50h + 10h*i. If the pointer at q-8 is
unequal to the actual sentinel, a nonnull count pointer at q is incremented.
Then, if the pointer at q-4 is nonnull, a nonnull count pointer at q+4 is
incremented. Finally, it reloads the pointer at D4h and unconditionally increments
the DWORD at that pointed-to object's B4h offset.

Repeated count targets receive repeated increments. A count target may affect a
later pointer load, so the implementation does not snapshot or deduplicate
fields. It compares sentinel pointer identity, never string contents. The root
and final time-locale pointer have no new null guard. There is no added overflow
repair, lock, release, error translation or exception handler.

## Qualified source ABI

The native entry has one cdecl pointer argument at entry ESP+4, preserves
EBX/EBP/ESI/EDI, and returns with plain RET. The source keeps that original
argument slot and adds the actual sentinel pointer as its second cdecl argument.
After the four original saved registers, that binding is at ESP+18h.

The native seven-byte comparison at offset 4Bh is replaced by these seven bytes:

```text
8B 4C 24 18    MOV ECX, [ESP+18h]
39 4B F8       CMP [EBX-8], ECX
```

The native sentinel is the address `00E161D0`. The source argument must supply
that identity in the actual corresponding locale domain and remain stable
through the call. It does not allocate or own a substitute sentinel.

The added argument, stack access, ECX use and new code addresses qualify entry
ABI, volatile-register, stack-alias and fault-continuation identity. The source
is not a drop-in native entry or an implementation of native exception/frame
ownership. Actual record storage and synchronization remain the owner's
obligations, including readable fields through offset D7h and writable selected
reference counts.

## Verification and remaining scope

Base commit: `52efcfc1f9e87d10cbf8149a9ecc8bdae347e5e8`. The discovery remains
unchanged at `f56cf12b4992af1137fa3ab49685de02ce6171f3`; all 129 retained
artifacts were checked by exact path set, size, SHA256 and SHA512 before reuse.
Current target-verified Ghidra bytes match the installed PE and the pinned
complete body and sentinel bytes.

MSVC Win32 emits 134 bytes with one DIR32 relocation for
`__imp__InterlockedIncrement@4`, loaded into EDI at offset 8. All eight original
static CALL EDI sites remain at offsets 0Fh, 1Ch, 29h, 36h, 43h, 5Bh, 6Bh and
7Fh. The loop can execute its two call sites repeatedly. Apart from the import
relocation and the explicit seven-byte sentinel sequence, every emitted byte
matches the original body, preserving reload positions, branches and increment
order.

The strict `/W4 /WX /fp:strict` build succeeds through deferred CMake source
registration. Retained compiler command/read/write records identify the exact
source, header and output object. The full retained `bsp_core.lib` contains one
matching object member and one definition of the source symbol; its object
bytes match the compiler output. The complete packet-local directory has two
independent SHA256/SHA512 inventory passes.

All eight existing seeds match. Both existing CTests, `reconstructed_math` and
`native_math_differential`, pass. No tests were added; those math checks do not
exercise this routine. No source/helper/native body was executed for runtime or
game validation, and no native ABI or gameplay parity is claimed.

The PTD initializer, canonical TLS/errno/locale ownership, actual lock startup,
and matching release/cleanup chain remain separate prerequisites. This primitive
does not close the `C050F8`/`C051B7`/`BFFB8B` path by itself.
