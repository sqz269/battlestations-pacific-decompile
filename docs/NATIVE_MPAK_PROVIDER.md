# Actual MPAK provider, directory reader and cache publication

Addresses: `00BB5430`, `00BB5630`, `00BB6500`, `00BB6870`, `00BB7C50`,
`00BB7920`, `00BB7B80`, `00BB8240`, `00BB82F0`.

These descriptive names are hypotheses, not recovered symbols. The source uses
actual Win32 storage and the existing pooled-string and string-vector routines.
The three STL insertion specializations and their vector destruction operations
remain explicit library contracts in `NativeMpakContainerLibrary`; no successful
placeholder implementation is supplied. The VFS open operation is also an
explicit captured-target dispatch. This is not a drop-in ABI/FH3 replacement or
evidence of a running game.

## Evidence and original interfaces

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Analysis batches verify that target. The installed
PE is read-only; SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Address | Inclusive end | Bytes | Original interface |
| --- | --- | ---: | --- |
| BB5430 | BB5475 | 70 | ECX file record; RET |
| BB5630 | BB5663 | 52 | ECX cursor DWORD; stacked output8h; EAX output; RET4 |
| BB6500 | BB6571 | 114 | ECX directory record; RET |
| BB6870 | BB68EE | 127 | ECX record; name/first/second/flag stacked; EAX record; RET10h |
| BB7C50 | BB8239 | 1514 | ECX provider44h; RET |
| BB7920 | BB79D3 | 180 | ECX provider44h; RET |
| BB7B80 | BB7B9D | 30 | ECX provider; flags stacked; EAX captured owner; RET4 |
| BB8240 | BB82E9 | 170 | ECX provider; original system-name header stacked; EAX owner; RET4 |
| BB82F0 | BB8390 | 161 | ECX system-name header; RET; no hidden ESI input |

Assembly is authoritative: the saved generic prototypes and the decompiler's
`unaff_EBX`/`unaff_EDI` artifacts do not describe the incoming arguments. Local
CALL_RETURN overrides had hidden parser blocks at BB7E31, BB807F and BB8221,
the directory destructor tail at BB653A, and three-byte free-call continuations
in the file/provider destructors. The locked repairs preserve prior metadata,
check PE/live bytes, save and refresh exports. See
`reports/native_ay_flow_repairs.json` and
`reports/native_ay_function_definitions.json` for the original and final states.

## Actual storage and construction

`BB8240` first calls the existing `BB5590` base constructor with the original
name. It then writes D641F8, calls the actual `736C30` PAK registry getter and
copies current registry+0Ch to provider+18h. It opens **embedded provider+8**
through the current `0109CEEC` owner's slot4, with mode2, stores the returned
stream at +14h and calls `BB7C50`. The original name is still used for the
preceding trace argument load. The trace routine `4254B0` is a verified RET.

| Provider field | Meaning supported by these bodies |
| --- | --- |
| +00 | D641F8 primary profile after base construction |
| +04 | Intrusive reference word, initialized by BB5590 |
| +08/+0C | Actual pooled name header |
| +10 | Base device value, initialized by BB5590 |
| +14 | Opened stream, published before directory parsing |
| +18 | Captured registry ordinal/default from registry+0Ch |
| +1C | File-vector allocator word; left untouched |
| +20/+24/+28 | File-vector begin/end/capacity, initialized to zero |
| +2C | Directory-vector allocator word; left untouched |
| +30/+34/+38 | Directory-vector begin/end/capacity, initialized to zero |
| +3C/+40 | Initialized to zero; further meaning is not inferred here |

File records are 24h bytes: name+0/+4, DWORDs+8/+C, a byte flag+10, padding+11
through +13, untouched allocator+14, and offset-vector begin/end/capacity at
+18/+1C/+20. `BB6870` copies the name before publishing numeric fields. It
zeroes the destination header before its self-alias guard. Directory records
are 14h bytes: name+0/+4 and the existing actual0Ch string vector at +8.

## Directory read schedule

The reader performs one 16-byte stream read with null actual-count output. It
compares `MPAK`, but does not branch to a rejection path on a mismatch. Two
little-endian DWORDs at header+4/+8 are added with DWORD wrap. Their sum plus4
is allocated and read in one unchecked call through the current provider
stream. File and directory loop counts are unsigned16 at header+12/+14.

The scratch begins with `TOC\0`. Each file skips two bytes, reads two DWORDs
and a Boolean byte, reads a prefixed string, constructs/appends a file record,
then reads a uint16 count and that many DWORD offsets. Each stored offset adds
the captured size sum plus10h. The second input DWORD becomes record+8; the
first becomes record+Ch. Integer insertion uses the current last file record,
the original iterator diagnostics, and its raw integer-vector header.

`BB5630` increments the cursor before reading its unsigned prefix byte. The
following text is copied to the first NUL by the existing `41E870` constructor;
the prefix is **not** a copy bound. After that call, current cursor advances by
prefix+1. This reload matters if the string allocation callback changes it.

The existing actual-header `41E870` implementation is reused. Its direct copy
at native41E8B4 now uses overlap-safe `memmove`, matching BF7680's established
backward-copy branch at BF769A -> BF7844. The provider's direct BF7680 calls
use the same behavior. The correct `_memcpy` library name is retained in Ghidra;
this narrow dependency correction is not counted as another reconstructed body.

After `STOC`, each directory first appends an empty record, destroys the
temporary record and empty string, obtains the current last directory, fills
its name, then appends each prefixed member string through actual `4CDC20`.
The final `RAWD` comparison also has no rejection branch. Scratch is freed only
on the successful normal path. No seek, bounds check, short-read recovery,
signature validation, overflow guard, path normalization or extra cleanup was
added. No installed `.mpak`, `.mpkg` or `.pak` fixture was found in the recorded
recursive installation scan; generated fixtures must be identified as such.

## Cleanup and publication

Parser FuncInfo DFE278 uses map DFE29C. Its seven states are:

| State | Unwind action, then predecessor |
| ---: | --- |
| 0 | CC46C0: current temporary name; -1 |
| 1 | CC46C8: file record BB5430; 0 |
| 2 | CC46D0: empty string; -1 |
| 3 | CC46D8: directory record BB6500; 2 |
| 4 | CC46E0: directory record's name only; 2 |
| 5 | CC46E8: parsed directory name; -1 |
| 6 | CC46F0: parsed member name; -1 |

`BB5430` frees nonnull offset backing, clears its three vector words, then
returns the current pooled name. `BB6500` resizes member count to zero, frees
the current backing without clearing the stale backing/capacity words, then
returns its name. Its state0 action CC44D0 releases only that name if member
cleanup throws. Normal explicit returns capture block and length before the
pool getter; the getter and return call remain independently visible.

Constructor FuncInfo DFE2EC and destructor FuncInfo DFE254 both unwind
directory-vector -> file-vector -> provider base, according to armed state.
Construction failure never releases the stream that was opened before parsing.
Normal `BB7920` decrements its captured stream+4 and invokes current slot0 only
at zero, then destroys directory/vector storage and the provider base. It does
not clear the cache, stale stream pointer or free its own allocation. `BB7B80`
frees that captured owner only after successful destruction and flags bit0.

`BB82F0` captures global lock010904E0, enters it and increments +18. Null name
data or an empty string clears cache010904DC; otherwise it allocates44h, calls
the actual constructor and publishes the result. It never releases the old
cache. Unlock reloads the current global lock, decrements that lock's +18 and
leaves it. Native state0 frees the captured allocation on failure and stays
armed through publication and LeaveCriticalSection; no lock cleanup is armed.

The source factory adapter calls these real constructor/cache routines. The
canonical raw singleton deletion map gains CFEA1C and D6418C bindings for the
actual MPAK factory+4 and registry+8, respectively. Their contexts borrow the
same publication cells used for construction; no parallel lifetime manager is
created.

## Validation boundary and follow-up packets

The strict Win32 build and both CTests passed at source commit `7665d4eb`.
The sealed generated provider fixture passed three native/source comparisons
with 3,484 checks, plus one source-only first-read exception case. Independent
output decoding checked three files, six offsets, two directories, five member
names, current-cursor reload after allocation, and canonical raw lifetime drain.
The nine provider bodies total 2,418 bytes. The 52 original STL or support spans
(6,523 bytes) execute as explicit library oracles shared by both fixture sides;
they are outside reconstructed provider coverage. Results and physical input
hashes are in `reports/native_mpak_provider.json` and
`reports/native_ay_integration.json`.

The first two provider attempts remain sealed harness failures: the destructor
reached an unbound `InterlockedDecrement` import. The successful third attempt
binds the verified Win32 operation; no production change was required. Original
FH3/SEH, simultaneous cleanup exceptions, arbitrary aliases into native stack
spills, short-read indeterminate stack bytes, an independent execution of the
original backward-overlap copy, installed archives and gameplay remain unproven.

Follow-up work includes a concrete compatible library binding for the three
STL insertion specializations and vector destructors; provider lookup/open
methods at BB79E0/BB7A00 and their dependencies; real startup/VFS reachability;
and installed archive/gameplay validation when an actual fixture is available.
