# Ordinary CRT calloc lock-4 unlock helper, 00C1C60B

`unlock_native_crt_calloc_lock4_00c1c60b` covers the complete nine-byte,
four-instruction ordinary entry at 00C1C60B..00C1C613. The provisional ledger
name is `BSP_CRT_CallocUnlockLock4`; it is not a recovered symbol. The original
live name `LIBCRT_unmatched_00c1c60b`, full comments and untyped prototype are
saved as preimages. No Ghidra name, comment, prototype or listing is changed by
this Source packet.

The MSVC Win32 naked cdecl body has no incoming arguments or own frame:

```asm
PUSH 4
CALL actual canonical unlock
POP ECX
RET
```

The one owned transfer is `00C1C60D CALL 00C11B31`, bound directly to published
`unlock_native_crt_canonical_00c11b31`. It remains a separate call, with POP ECX
rather than a tail jump or stack ADD. On normal return ECX is 4, EAX/EDX and
arithmetic flags retain the child's state, and nonvolatiles inherit the actual
child/Win32 contract. The semantic Source return is void. There is no added
context, provider, default callback, global, type overlay, guard, lock or owner.
The existing lock-10 helper remains an unchanged reference only.

The caller must provide the actual readable DWORD descriptor at E16498h
(E16478h + 8 * 4), pointing to a valid initialized Win32 critical section
acquired by the current thread through the call. Readable page admission does
not initialize or acquire that lock. The helper supplies neither operation.
The canonical provider continues to call actual `LeaveCriticalSection`.

Fresh whole typed metadata verifies C:/Users/sqz269/bsp.gpr and
/battlestationspacific.exe. All nine installed PE bytes equal live bytes and
Root's independent five-check readiness. All four current instruction starts,
the ordinary prototype and the sole encoded CALL agree. The old lookup graph
lists both C07C45 (__SEH_epilog4) and C11B31; that stale graph is not the body.
Fresh bytes/listing/typed flow establish exactly one call to C11B31.

The preceding five bytes at C1C606 fall through this helper in the larger
private-frame sequence, as recorded in Root's separate readiness. That prefix
and the complete fourteen-byte private action have no proven whole Source here.
They were not read, repaired or emulated in this packet. Calloc C1C50F and its
FH3/SEH4/allocator closure remain OPEN. No fake public EBP reader, cleanup stub
or action record is introduced.

The normal `./scripts/build.ps1` MSVC Win32 Release build passes all three
existing suites in 8.30 seconds. The known unrelated duplicate spawn-symbol
LNK4006 warning remains. No new tests, probes, alternate or clean builds, helper
Native execution, or game checks were added; these suites are not direct lock
initialization or helper-body tests.

The complete actual CMake object has one nine-byte/four-instruction CODE section
and exactly one REL32 relocation at offset 3 to the actual canonical provider,
with encoded addend zero. All nine bytes match Native after masking only the
four verified child-placement operand bytes, offsets 3..6. No helper code,
prologue, EBP access, EH, GS/cookie, BSS, weak data, table or padding is emitted.
Non-code sections are directives 47B, debug metadata 212B and checksums 32B,
all without relocations. Every instruction, section and relocation was reviewed.
New code placement and the genuine child relocation still change fault PCs and
private return frames; Original runtime, CRT/PTD lifecycle, SEH/FH3, fault and
game equivalence remain unproved.

Both canonical provider files match the published Main LF fingerprints. Prior
hint Source50 frozen/precommit/annotation receipts, the original 23ec object
archive and all older 22 archives remain unchanged. Before this normal build,
23 current cited objects were copied by hash with an explicit original-path
mapping; no earlier receipt was rewritten. Root preliminary review passed eight
checks with no findings. Its receipt is immutable; final frozen-pin acceptance
and any Ghidra closeout require a separate later authorization.
