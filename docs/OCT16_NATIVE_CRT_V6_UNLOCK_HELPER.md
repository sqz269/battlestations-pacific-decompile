# Ordinary V6 CRT lock-4 unlock helper, 00BF9E9C

`unlock_native_crt_v6_heap_00bf9e9c` covers the complete ordinary entry
00BF9E9C..00BF9EA4: nine bytes and four instructions. The provisional name
`BSP_CRT_V6HeapUnlockLock4` describes behavior, not a recovered symbol. Original
`LIBCRT_unmatched_00bf9e9c` name, full comments and stored untyped prototype are
saved as preimages. No Ghidra name, prototype, flow or listing is changed by this
Source packet.

The zero-argument MSVC Win32 naked cdecl implementation is exactly:

```asm
PUSH 4
CALL actual canonical unlock
POP ECX
RET
```

The sole owned transfer is `00BF9E9E CALL 00C11B31`, directly bound to published
`unlock_native_crt_canonical_00c11b31`. It does not alias the distinct Source52
calloc helper C1C60B. The separate child call and POP are retained, without a
tail jump, stack ADD, own frame or EBP reader. ECX is 4 on normal return;
EAX/EDX/arithmetic flags and nonvolatiles inherit the actual child's contract.
The semantic Source return is void; the stored Original prototype remains
untyped and is not replaced with that C++ declaration.

The caller owns the actual readable DWORD descriptor E16498h (base E16478h,
plus 8 * 4) and its valid initialized Win32 critical section, acquired by the
current thread throughout. Readable page admission does not initialize or
acquire the lock. No new context, callback, global, default, owner, guard or
initialization/acquisition service is added. The canonical provider calls actual
`LeaveCriticalSection`.

Fresh typed whole-body metadata verifies C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. All nine installed PE bytes equal live bytes and
Root's independent eight-check readiness. All four current starts and the sole
encoded call/returning flow agree. The stale cached lookup lists five callees;
fresh bytes/listing/typed flow establish only the actual canonical call. No
listing or flow repair was performed. Lua's frozen V6 parent readiness is cited
for provenance, without promoting its parent Source scope.

The normal default `./scripts/build.ps1` Win32 Release build passes all three
existing suites in 8.11 seconds. These are existing checks, not direct helper
or lock-initialization tests. No new tests, probes, alternate/clean builds or
helper Native execution were added. The unrelated existing duplicate-spawn
LNK4006 warning remains.

The complete actual CMake object contains one nine-byte/four-instruction CODE
section, with one REL32 at offset 3 and encoded addend zero to the actual
canonical provider symbol. There are no helpers. Every byte matches Native
after masking only the four verified child-placement operand bytes at 3..6;
all other instruction bytes remain exact. All code, relocation and data
sections were reviewed: directives 47B, debug metadata 208B, checksums 32B, no
non-code relocations, BSS, weak data, GS, EH, xdata, SafeSEH table or padding.
The relocation and new placement still qualify Original fault PCs/private
return frames. Whole V6 heap, SEH4/compiler frames, SBH, heap/bootstrap,
runtime and game behavior remain unproved.

The actual CMake registration gate was queued first, claimed for one append and
released before preservation and build. A separate short actual gate protects
the address-owned ledger additions. No unreviewed Main work was merged. Before
building, 24 currently cited objects were copied by hash with explicit path
mappings. Source52's 26bd and Source50's 23ec immutable objects, earlier 22/23
archives, frozen receipts, pins, Root checks and publication receipts remain
exact. Int's 41959 mapping preserves historical f963/4338 originals even when
later foreign current build paths advance; no old receipt is rewritten.

Root Source-only preliminary review passed eight checks without findings. A
distinct Source-plus-actual-COFF preliminary review then passed sixteen checks,
including all 803 object bytes and symbols. Both receipts remain immutable.
Final frozen-pin acceptance and any Ghidra closeout remain separate future
authorizations. The published canonical provider's two files retain their LF
fingerprints; no whole allocator/SEH/runtime closure is implied.
