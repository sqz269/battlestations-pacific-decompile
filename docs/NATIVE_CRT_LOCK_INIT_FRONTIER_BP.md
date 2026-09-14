# Native critical-section initialization frontier

The next bounded source packet is `native_crt_critical_section_primitives`:
complete `C17643` fallback (16 bytes) and `C17639` encoded-cache setter
(10 bytes). Both have concrete closed operation boundaries using actual
borrowed storage and a real Win32 API. The complete `C17653` wrapper remains
open because its actual Watson and native SEH4/frame domain are not supplied.
The static lock initializer `C11A93` depends on that full wrapper.

Discovery only, based on `0b3bbf7e21a202d2a31dcaba3e6c2631737b415b`.
No source, tests, Ghidra state or listing repairs were changed. PTD discovery
`f56cf12b4992af1137fa3ab49685de02ce6171f3` is reused with exact artifact
verification. The cookie and locale source worktrees remain untouched.
Full evidence is in `reports/native_crt_lock_init_frontier_bp.json`.

## Directly dispatchable primitive contracts

| Entry | Complete span | Original ABI and behavior |
| --- | --- | --- |
| `C17643` | `C17643..C17652`, 16 bytes | stdcall two DWORD arguments: critical-section pointer at entry ESP+4, ignored spin count at ESP+8; RET 8 |
| `C17639` | `C17639..C17642`, 10 bytes | cdecl one DWORD at entry ESP+4; ordinary store to actual encoded word `109E454`; plain RET |

`C17643` is the library function `___crtInitCritSecNoSpinCount@8`:

```text
FF 74 24 04             PUSH [ESP+4]
FF 15 0C 22 CE 00       CALL [CE220C]  ; real InitializeCriticalSection
33 C0                   XOR EAX,EAX
40                      INC EAX
C2 08 00                RET 8
```

The import is KERNEL32 `InitializeCriticalSection`, taking one pointer and
cleaning four argument bytes. The outer fallback consumes both original
arguments, ignores the API's incidental EAX, and returns exactly 1 on a normal
return. XOR then INC establishes the native arithmetic-flag effects. The
spin argument is neither loaded nor forwarded. This body owns no exception
handler, LastError repair, allocation, pointer validation, or initialized
critical-section storage. API exceptions propagate into the actual caller's
frame. A qualified source implementation must use the actual Win32 import and
borrow the caller's real writable critical-section object, not a fake callback
or new lock object.

`C17639` is exactly `8B 44 24 04 A3 54 E4 09 01 C3`: load the caller's encoded
DWORD into EAX, store it unchanged to `109E454`, and return. EAX still contains
that DWORD; the observed caller does not consume it as a semantic return.
MOV/RET do not alter arithmetic flags or nonvolatile registers. The helper
does not encode, decode, validate, initialize a replacement global, or provide
synchronization. Its source interface should borrow the actual writable cache
word and accept the already encoded input; any extra binding argument qualifies
the source ABI and fault continuation.

## Full C17653 selection, publication and return behavior

The complete physical body is `C17653..C17717`, 197 bytes. Its cdecl arguments
are critical-section pointer and spin count, read at EBP+8 and EBP+Ch after
the actual SEH4 prolog. The selected target is called as a two-argument stdcall
provider; the wrapper itself returns with plain RET.

1. Initialize the OS-platform local to zero. Read actual `109E454` and call
   existing `C04FDE` to decode it. A nonnull result is used directly, without
   querying platform/module state or republishing the cache.
2. On decoded null, call `BFBAB2` with the local's address. A nonzero status
   calls actual `BF65BB` with five zeros. If Watson returns, the omitted
   `C1768E ADD ESP,14h` restores the outgoing stack and selection continues.
3. Platform exactly 1 selects `C17643`. Every other value tries
   `GetModuleHandleA("kernel32.dll")`, then real
   `GetProcAddress(...,"InitializeCriticalSectionAndSpinCount")`. A missing
   module or missing export selects `C17643`; no LoadLibrary is introduced.
4. Call existing `C04F67` on the selected raw target and publish its returned
   encoded DWORD to actual `109E454`. Keep the raw target in ESI for this call.
   There is no null repair, compare/exchange, cache retry or private copy.
5. Only now set the native SEH try level to 0. Push spin count then the actual
   object pointer, CALL ESI, and retain the exact returned DWORD. Clear the try
   level, invoke the actual SEH4 epilog, and return that value without BOOL
   normalization. A zero API result does not clear the cache or retry fallback.

There is no general LastError save/restore. Normal returns retain the actual
providers' side effects; the memory-exception handler is the explicit error-8
write described below.

The exception-protected invocation region begins after decode, OS lookup,
Watson, module/export lookup, encoding and cache publication. Wrapping all those
earlier operations in a new catch policy would change behavior.

## Complete omitted exception path

Scope table `E037B8` contains these seven DWORDs:

```text
FFFFFFFE 00000000 FFFFFFCC 00000000 FFFFFFFE 00C176D9 00C176F0
```

`C176D9..C176EF` loads the exception code through the original EBP-14h exception
pointer chain and stores it at EBP-24h. It returns 1 exactly when that code is
`C0000017`, otherwise 0. Thus only that code selects this handler; other codes
continue exception search.

`C176F0..C17707` restores ESP from EBP-18h. It rechecks the saved code and, when
equal to `C0000017`, calls the real `SetLastError(8)` import at `CE22BC`. It then
clears the saved result at EBP-20h and joins the common return path. Preserve
that conditional recheck, rather than replacing it with an unconditional error
store. No errno write, broad exception swallowing, retry or cache clearing is
present in this handler.

The live listing omits both the three Watson-return cleanup bytes and this
47-byte filter/handler region. Complete current PE/live bytes and raw decoding
retain them without mutation. The handler's indirect CALL at `C176FE` is
separately byte/import-qualified, not a listing-verifier success. This evidence
does not establish native SEH4 frame or exception-continuation source parity.

## Actual cache and static storage initialization

The PE/database initial cache word is zero; that does not justify substituting
literal zero for the runtime's encoded-null initialization. Native `C04FD5`
pushes zero, calls actual `C04F67`, pops the argument, and returns its encoded
result. Complete 76-byte `BFBDFB __init_pointers` captures that value once in ESI
and passes it to `C17639` at call site `BFBE0A`, as well as to its other actual
cache setters. It performs no new encoding inside `C17639`.

The retained external `__mtinit` excerpt calls `BFBDFB` at `C054A0`, before
calling `C11A93` at `C054E8`. That provides the original ordering of cache setup
before static lock initialization. Full canonical startup ownership remains
separate; the excerpt is not a reconstructed complete startup body.

The reused complete 73-byte `C11A93` iterates 36 actual table entries at
`E16478`, each an eight-byte pointer/type pair. For type exactly 1 it publishes
the next pointer from `109E1C0`, pushes spin count 4000, reloads the published
pointer for the call, advances the pool by 18h, and calls full `C17653`.
On returned zero, it read/modify/writes only that current table pointer to zero
and returns 0. Earlier successful critical sections are not undone here;
exceptions do not take that normal zero-result cleanup branch.

The original table has 14 static entries, backed by the actual 150h-byte pool.
Successful initialization binds lock 10 (`E164C8`) to `109E268` and lock 12
(`E164D8`) to `109E280`. These are conditional startup results, not observations
of initialized locks in a running process. No host allocator or fabricated
initialized lock domain is needed or supplied by the proposed two primitives.

## Source readiness and validation

Current concrete source supplies qualified `BFBAB2` OS-platform retrieval,
`C04F67` encoding and `C04FDE` decoding. Those interfaces still borrow actual
OS/TLS/PTD and owning error state. Exact source and reconstruction entries are
absent for `C17639`, `C17643`, `C17653`, `C11A93`, `C04FD5`, `BFBDFB`, and
actual `BF65BB` Watson. Native `C07C00/C07C45` frame helpers also remain an
external source domain. The proposed next packet is only the two complete
26-byte primitives; keep full wrapper/static lock startup explicitly open.

Five freshly retained complete function spans total 308 bytes, plus four new
data/string spans. Four PTD-discovery spans are reused, including complete
`C11A93`, lock table, static pool, and a bounded external startup excerpt.
All 129 baseline artifacts were verified before reuse. Twenty direct-call rows
pass `verify_report_calls.py`; five indirect sites are separately qualified,
including the omitted handler call. The whole local directory has two
independent SHA256/SHA512 inventory passes. No source, build, runtime, ABI or
gameplay advancement is claimed by this discovery.
