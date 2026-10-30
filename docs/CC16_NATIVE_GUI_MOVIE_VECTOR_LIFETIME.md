# Raw Movie vector tidy contracts

Pinned private base: `bf5820f4eaffdff1d03acf3679d2d05c29a110d1`. It contains
exact range50d24663b24b84f29b44ef1d6d47973423f94b4a and allocation
a10686893116585ca6b7e1d1f848fd9d44781712 prerequisites. Their required source
blobs are unchanged. They are explicit private inputs here; this packet does
not inherit common-build, link, caller or activation admission from them.

Only raw AAF720/AAF7A0 vector tidy is added. No new/duplicate/vendor native
body credit or original ABI/activation claim. Names are descriptive hypotheses.

## Complete native schedule

Both original bodies are61 bytes/21 instruction starts, inclusive
AAF720..AAF75C and AAF7A0..AAF7DC. Native ECX is the actual16-byte
`{opaque,first,end,capacity}` header; no original stack inputs, plain RET,
no semantic EAX result established. Source instead takes raw header and a
required `NativeStringRawPoolContext&` under a new two-argument cdecl interface.

| Operation | Subtitle | Extratitle |
| --- | --- | --- |
| Capture CURRENT first+4 | AAF724 | AAF7A4 |
| Initial null branch to clears | AAF729 | AAF7A9 |
| Nonnull-only CURRENT end+8 read | AAF72F | AAF7AF |
| Matching captured-range call | AAF734->AAF0E0 | AAF7B4->AAF120 |
| Reread CURRENT first+4 | AAF739 | AAF7B9 |
| Free that current value | AAF73D->BF65AC | AAF7BD->BF65AC |
| Clear first/end/capacity | AAF745/74C/753 | AAF7C5/7CC/7D3 |

Initial null skips end, range and free, but still clears all three pointer
fields in that order. Nonnull first captures the end and destroys that original
range, even if it is empty. After the range returns, CURRENT first is reloaded
for the real backing return. There is no additional null test: a callback's
null replacement still reaches free(null). Do not free the cached start, adopt
another range, add a null guard, cache a second header or repair replacements.
The opaque word+0 is never read or changed.

Each clear is a separate volatile DWORD store after normal backing return.
Callback replacements are overwritten in native order. If a range operation
throws, free and all clears are skipped; no local catch, retry, prefix rollback
or alternate cleanup is introduced. Partial record destruction/backing and all
independent failure metadata remain the actual outer caller's responsibility.
The future AAFEF0 state/demotion schedule is not supplied by this helper.

## Actual prerequisites and ownership

The functions directly call the genuine50d raw14h/24h string-range helpers.
Those consume only first8-byte NativeString headers using the same actual
RawPoolContext and capture their end across callbacks. They do not delete an
extratitle record+18 object, clear row headers or free vector backing.
Actual context suppliers remain GameNativeStringProcess::raw_context() and
GameVfsHost::raw_strings(); no substitute service/default/global is constructed.

Backing return directly calls canonical `singleton_lifetime_free`, whose real
definition calls actual Source CRT free. Every current pointer passed there
must be a matching real allocation from the a106 AACD30/AACD90 object/native=
host byte-extent `singleton_lifetime_allocate` domain. This includes a pointer
replaced during callbacks. The caller must prove that identity; no callback,
registry, class tag or successful preflight is invented to authenticate it.
Foreign/private heaps, arbitrary new/new[], semantic allocations, string-pool
buffers, interior record pointers and widget-pool slots are not admitted.

Require genuine aligned actual header/constructed raw ranges, independent
metadata and the SAME pool/cells/providers through all reached calls. Header
and metadata must not overlap backing that can be freed. Captured ranges and
all possible current backing retain their proper ownership until their native
consumption; released addresses become opaque, potentially dangling bits.
Do not retry them. There is no new heap owner, scalar/flags1 permission,
destructor latch or arbitrary reentry/concurrency guarantee.

## Strict static evidence

Read-only target-verified BSP wrappers checked the existing configured
C:/Users/sqz269/bsp.gpr and /battlestationspacific.exe, x86/image400000,
live/snapshot77631. Full pseudocode preceded both complete assemblies/live61
bytes and independent installed-PE/COFF inspection. The report pins original
bytes and four direct-call rows; the existing checker validates those rows.

Strict MSVC Win32 compilation passed for the new source, exact private range
source and unchanged actual singleton service: `/std:c++17 /O2 /Ob2 /DNDEBUG
/MD /EHsc /W4 /WX /fp:strict /Gy /c`. No executable or allocation ran.
Each new public entry is61 bytes/19 instructions under its Source interface.
The physical relocations call its matching real range at+15 and canonical
free at+1E; both definitions exist in the separately compiled real objects.

Static inspection confirms first capture+05, initial null jump+0A to+26,
nonnull-only end read+10, postrange CURRENT first load+1A, and no second null
guard before free. Exact clears at+26/+2D/+34 write offsets4/8/C with zero,
and opaque0 is untouched. This is emitted-effect/definition evidence, not
identical Native instructions/register/private-stack/FH3/SEH/fault or final
link proof. Existing prerequisite/header schemas and source hashes are unchanged.

No common build, new tests, executable/probe, game, module/DLL load, CMake,
ledger/Ghidra/guard or frozen-peer operation occurred. Only the four declared
new files changed; all three root/subtitle/child-Page guards remain false.
The primary owns independent review, common build/link and admission.

Whole AAFEF0, current decoder scalar, base/model/observer/borrower custody,
flags1/pool ownership, raw insertion/construction, factory/loaded/properties/
Page consumer and playback remain separate. These two tidy contracts do not
make a populated Movie or its failure-retirement graph closed.
