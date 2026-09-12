# Actual MPKG entry and vector storage

Addresses: `00BB9330`, `00BB93A0`, `00BB94A0`, `00BB9520`, `00BB9900`;
cleanup fragments `00CC47C0..00CC47D6`, `00CC47F0..00CC480B`.

The new source operates on actual raw headers through `NativeStringStorage` and
the existing `singleton_lifetime_allocate/free` services. It preserves native
entry order and ownership operations without introducing a parser or container
replacement. The earlier bounded `mpkg_archive.cpp` projection remains separate.

| Span, inclusive | Bytes | Coverage | Source API / original ABI |
| --- | ---: | --- | --- |
| BB9330..BB939D | 110 | complete | `copy_native_mpkg_entry_00bb9330`; ECX destination, stack source, EAX destination, RET4 |
| BB93A0..BB9494 | 245 | complete | `reserve_native_mpkg_entries_00bb93a0`; ECX vector, stack signed capacity, RET4 |
| BB94A0..BB951C | 125 | complete | `resize_native_mpkg_entries_00bb94a0`; ECX vector, stack signed count, RET4 |
| BB9520..BB9593 | 116 | complete | `append_native_mpkg_entry_00bb9520`; ECX vector, stack source, RET4 |
| BB9900..BB9916 | 23 | complete | `destroy_native_mpkg_entries_00bb9900`; ECX vector, no stacked arguments, RET |
| CC47C0..CC47D6 | 23 | partial: enclosing reserve cleanup fragment only | captured-local placement-delete argument computation, RET-only401130 |
| CC47F0..CC480B | 28 | partial: enclosing append cleanup fragment only | current count/base reads, RET-only401130 |

BB95B0 writes the raw24h entry: name length/data at0/4, local-header offset8,
resolved byteC, provisional data offset10, method word14, compressed/decoded
counts18/1C, and retained CRC-like DWORD20. Padding D..F and16..17 is untouched.
BB9984/BB9987/BB998A initialize archive+28/+2C/+30 as vector pointer/count/capacity.
The directory worker owns the parser; no ZIP specification substitutes for this
producer evidence. Both observed BB9900 consumers, CC488B and CC48BB, pass archive+28.

Copy clears the destination name before its self-alias decision. A self-copy
therefore abandons the prior name without freeing it. Distinct copy invokes
41DD40 on the actual destination; afterward it rereads source length, destination
length, source data and destination data in native order. BF7680 is overlap-capable
despite its saved `_memcpy` library name; the source uses `memmove`. Payload fields
are read and written individually after the string callback. No padding copy or
early payload snapshot is added. The existing host convention omits a zero-byte
standard C++ copy with null pointers.

Reserve clamps signed requests below1 to1 and returns when signed capacity is
already sufficient. Allocation size is lowDWORD(capacity*24h). The ascending copy
loop uses the current signed count and rereads the old base for each element.
The ascending release loop independently rereads count/base and captures each
name pointer and length+1 before the pool-return boundary. It frees the current
old base, reloads the current capacity argument after free, then publishes the
captured new base and capacity. Count is not restored after callbacks.

Reserve FuncInfoDFE3E0 uses UnwindMapDFE3D8: its only state0 action is CC47C0,
which computes arguments from captured locals and calls one-byte RET401130.
No allocation or completed-prefix rollback occurs when copying fails. Append's
FuncInfoDFE40C/UnwindMapDFE404 likewise has one state0 action CC47F0; it rereads
current count then current base before its no-op placement deletion. The source
catch preserves those volatile reads and rethrows. Growth occurs before append
arms that region, so reserve failures do not trigger append's cleanup reads.

Append grows only on count==capacity, doubles with DWORD wrap, and clamps signed
results <=1 to1. It computes its destination after reserve, preserves the original
source pointer without an alias-protecting temporary, and increments current count
after copy. Resize grows by zeroing only new name headers; shrink decrements count
before releasing each name from back to front, rereading current count afterward.
It finally stores the requested count. Destruction calls resize0 and frees the
current base, leaving its stale pointer and capacity unchanged.

All 670 owned bytes, EH tables/handlers, RET-only placement-delete leaf and the
entry producer were captured with read-only `bsp.py ghidra` queries guarded for
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and matched to the installed
PE. The primary repaired only the erroneous free-call flow at BB946F/BB990D and
refreshed exports. Reserve BB9474..BB9482 and destructor BB9912..BB9916 are now
present in their stored bodies. Raw handler tails CC47D7..CC47E0 and CC480C..CC4815
have no containing Ghidra function and are explicitly recorded as support only.

Strict Win32 `scripts/build.ps1`, `/W4 /WX /fp:strict`, passed both CTests after
`verify-seeds`. The ignored fixture is `local/mpkg_entry_vector_ax/run_fixture.py`:
`--repo <built checkout> --attempt <fresh path> [--build-log <path>]`.
Attempt02 passed **10 native/source states, 86 checks**, covering ordered growth,
padding, backwards release, destructor non-reset, payload/self alias behavior and
current count changes during copy. One source-only second-copy failure verifies
the orphan allocation/completed prefix before separately reclaiming it.

Before first execution, 88 inputs were physically sealed read-only: exact linked
sources, ten compiler-observed project headers, all three actual linked objects
equal to their archive members, archive, probe, driver and native bytes. MSVC
`/showIncludes` and selected `CL.read.1.tlog` groups determine header dependencies;
the complete BSP include tree is not copied. All inputs were rehashed unchanged,
all execution outputs are included in the after-manifest, and relocated code was
checked unchanged after calls. Failed capture01 and pre-execution attempt01 are
retained; the latter exposed relative `/showIncludes` path interpretation in the
driver, corrected before attempt02. No production fix was needed.

Native original bodies use explicit rebuilt string-resize/storage-return adapters
and the existing CRT allocation/free services in this fixture. This does not test
original string-pool lazy publication. `NativeStringStorage::release` is noexcept,
so throwing lazy-pool recreation, original FH3/SEH, invalid-memory behavior,
allocation-overflow safety, and gameplay remain unproven. No bounds, negative-count,
null-allocation recovery, duplicate policy, cache repair or new pool is introduced.
