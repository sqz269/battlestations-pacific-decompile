# Canonical original-ABI locale reference acquisition FK

Address: `00C0190B..00C01990` (`___addlocaleref`, 134 bytes).

The separate entry `bsp::acquire_native_crt_canonical_locale_references_00c0190b`
implements the complete original one-word cdecl body. It uses the actual fixed
`00E161D0` sentinel identity and real Win32 `InterlockedIncrement` import. The
existing two-argument source-context interface is unchanged. This closes the
extra-argument dependency identified by FH; it does not provide the missing PTD,
native lock initialization, locale allocation/release, pointer codec or SEH owners.

| Routine | Coverage | ABI | Evidence level |
|---|---|---|---|
| `C0190B` | Complete, all 134 bytes and all branches | One pointer at entry ESP+4; cdecl; EBX/EBP/ESI/EDI saved; plain RET | Reconstructed, Win32 build and emitted-body/archive inspection; no body execution |

Source pin: `15cc82222fe06a3081ccf5c5baad78e8a47c0fba`. The implementation is in
`src/native_crt_canonical_locale_reference.cpp`; the header gives the caller's
storage and lifetime contract. The local packet is `local/canonical_locale_reference_fk/`,
and its machine-readable summary is `reports/native_crt_canonical_locale_reference_fk.json`.

## Complete general behavior and actual state

The original instruction sequence loads the imported function pointer at
`CE221C` into EDI once. The PE import directory identifies that slot as
`KERNEL32.dll!InterlockedIncrement`. The source declares the genuine imported
LONG/stdcall API directly to avoid SDK intrinsic substitution. There are no
other callees, context lookups, accessors, bindings or substituted globals.

In this exact order:

1. Increment the DWORD at the supplied record address without testing it for null.
2. Reload each DWORD pointer at record offsets B0h, B8h, B4h, C0h in that order;
   increment each nonzero selected target before loading the next field.
3. For six iterations, q = 50h + 10h*i. Compare `[record+q-8]` against the
   immediate address `E161D0`. If unequal, reload `[record+q]` and increment it
   if nonzero. Then test `[record+q-4]`; if nonzero, reload `[record+q+4]` and
   increment that target if nonzero. Preserve both tests and both load positions.
4. Reload `[record+D4h]`, add B4h and increment that address unconditionally.

This is the general body, including nondefault categories and all optional
counts. It does not specialize on the initial default locale. The six category
pairs occupy the original 48h..A7h area; the highest record read is D4h..D7h.
The original copy producer `C01A1D` copies 36h DWORDs (D8h bytes), clears the
destination's root count and calls this primitive. No new record layout or
named field type is imposed on these observed offsets.

The sentinel is a pointer identity comparison, with no dereference of the
sentinel contents. A distinct string containing `C` must follow the unequal
branch. Aliased count targets are incremented repeatedly. A count may affect
fields that are read later, so fields are not snapshotted, reordered, or
deduplicated. Arithmetic, branch structure, volatile register effects, and
reload/fault ordering are retained by the original instruction sequence.
There is no null repair for the root or final pointer, overflow repair, rollback,
error conversion, exception frame, allocator, release operation or lock here.

The actual fixed default locale and sentinel are owned by the existing
`GameNativeCanonicalDataOwner` / `GameNativeMutableCrtData`. Current source
commits E15000 and E16000 read/write, validates the supported PE's full SHA256,
and copies its exact two initialized pages. Fresh analysis-image bytes agree
with the installed PE at the complete routine, sentinel, default record,
default-current pointer and final time count. The retained initial values are
`[E162B0]=E161D8`, `[E161D8+D4]=E159F0`, and the final count at E15AA4. The four
optional count pointers and all category count pointers are initially zero.
Thus the initial default path selects E161D8 and E15AA4, both within those
admitted mutable pages. These are initial image values, not observed runtime state.

For an arbitrary real locale argument, all record/count allocations and their
lifetimes remain owned by its actual producer, as they do at the original entry.
This primitive performs the complete native operation on those addresses; it
does not manufacture arbitrary locale objects or establish that their missing
allocation/release owners are executable. Calling it with invalid or released
storage may fault after earlier increments. Source presence is no permission
to call it on a synthetic record or before the original caller's state is ready.

## Caller synchronization and one-word proof

The current target-verified xrefs contain three direct callers. Each caller's
full current containing body was retained before deriving this contract.

| Containing routine | Call site | Argument and caller cleanup | Original owner context |
|---|---|---|---|
| `C01A1D __copytlocinfo_nolock` | C01A38 | C01A37 PUSH EAX; C01A3D POP ECX | Source record copied into destination, root count cleared; parent `_setlocale` locks 12 at C02815 before its C02827 copy call |
| `C01A41 __updatetlocinfoEx_nolock` | C01A53 | C01A50 PUSH EDI; C01A5A POP ECX | Publishes replacement before acquisition, then releases old record; parents lock 12 at C01AC0 or C02871 |
| `C050F8` PTD initializer | C05196 | C05193 PUSH [ESI+6C]; C0519B POP ECX | Locks 12 at C05177; publishes supplied locale or current E162B0; finally cleanup C051AE unlocks 12 |

The copy/update helpers acquire no lock themselves. Their current call xrefs
lead to `___updatetlocinfo` and `_setlocale`; all observed acquisition paths are
under native lock 12. `_setlocale` uses C028DA/C028E6 cleanup and the updater
uses C01AE9; their original cleanup instructions call the unlock owner with 12.
`C11C21` selects `[E16478+8*lock]`, lazily establishes a missing lock through its
actual owner and calls the real EnterCriticalSection import. `C11B31` calls the
real LeaveCriticalSection import through that table pointer. Successful native
initialization/lifetime of that table remains required. An interlocked count
increment alone cannot protect surrounding pointer fields from replacement
or destruction.

The older local snapshot's extra `C01B5B sync_legacy_variables_lk` caller edge
is stale: the current complete body C01B5B..C01BAD has no call instruction.
No call is attributed to that routine in this packet, and no shared snapshot,
function body, flow, prototype, no-return flag or Ghidra comment was repaired.
The correct library name `___addlocaleref` is retained in the desired annotation
plan; existing name evidence is appended in the ledger for primary review.

## Static build and artifact verification

The required eight existing original-math seed spans agree with the installed
PE before the single `scripts/build.ps1` invocation. That script performs the
MSVC Win32 Release build with `/W4 /WX /fp:strict` and runs the existing CTests.
No test was added. These math checks do not exercise this locale routine.

The pre-build guard was captured before compilation, with no owned object
present. It records hashes and retained copies of the exact owned source and
header, old untouched interface, build registration/configuration and relevant
owner/verifier sources. It also records the installed toolset and both possible
x86 compiler-host binaries. The post-build guard requires the same input and
toolchain hashes and retains the exact owned CL command/read blocks. The grouped
write record retains the exact selected source/output strings, positions, batch
size and original full-batch hashes, without copying 1289 unrelated source/output
paths. The read block must include both the exact source and header paths.
This bounds the compiler-time provenance; it does not infer it from a later Git diff.

The actual owned COFF function is 134 bytes, with one DIR32 relocation at body
offset 0Ah to `__imp__InterlockedIncrement@4`. Substituting only that four-byte
import operand produces byte-for-byte equality with the complete original.
In particular the seven bytes at offset 4Bh are exactly
`81 7B F8 D0 61 E1 00`, restoring the original sentinel comparison without the
extra stack load or ECX write. The eight CALL EDI sites remain at offsets
0Fh, 1Ch, 29h, 36h, 43h, 5Bh, 6Bh and 7Fh. There is no added prolog, epilog,
context word, helper call, private static state or branch replacement.

The packet retains the actual compiler object, extracted identical
`bsp_core.lib` member, its original archive header and archive SHA256/SHA512.
The archive's first linker table has exactly one definition of the canonical
symbol pointing at that member. It does not duplicate the full compile tree
or archive. Original body/data bytes, verification methods, CLI outputs and
bounded source inputs remain locally reviewable.

Original entry argument count, preservation and native operations are supported
by static instruction/relocation evidence. The function has a new source code
address and imported slot in the rebuilt image; original return/fault addresses,
unwind continuation identity and original image placement are not reproduced.
No packet source body, original locale body, native initialization, forced-DLL
path or game was executed. Full PTD/init-lock/SEH closure and gameplay remain
unimplemented or unvalidated outside this packet.
