# Actual physical provider and stream opening

This packet reconstructs the physical provider's actual pool, stream construction,
open/read/size leaves and failed-open recycling. It consumes the established
physical-path and actual pooled-string helpers; it never constructs a semantic
`PhysicalFileStream` or an alternate provider. Public interfaces are in
`include/bsp/native_physical_stream_open.hpp`, implementation in
`src/native_physical_stream_open.cpp`, evidence in
`reports/native_physical_stream_open.json`. Descriptive names are hypotheses.

## Bodies and ABI

All endpoints below are inclusive. Every listed body has complete source control
flow; the provider and recycle dispatch are qualified to the current numeric
profiles described below. These new C++ interfaces are not drop-in native ABI or
FH3/SEH replacements.

| Address span | Bytes | Original ABI and result | Coverage |
|---|---:|---|---|
| BF30C0..BF311E | 95 | ECX pool+4 header; requested stack; RET4 | complete |
| BF3670..BF36BF | 80 | ECX header; requested stack; RET4 | complete |
| BF3770..BF382C | 189 | ECX header; EAX stream/null; RET | complete |
| BF3930..BF396C | 61 | ECX header; RET | complete |
| BF42A0..BF4351 | 178 | no arguments; EAX current publication; RET | complete |
| BF4370..BF43A0 | 49 | ECX pool; flags stack; EAX original address; RET4 | complete |
| BF4BA0..BF4C66 | 199 | ECX provider; suffix/full flags stack; EAX stream/null; RET8 | complete qualified consumer |
| BF4F90..BF4F96 | 7 | ECX stream; EDX:EAX cached size; RET | complete |
| BF4FA0..BF4FBA | 27 | ECX stream; unused DWORD stack; EAX fresh low size; RET4 | complete |
| BF5020..BF5029 | 10 | ECX stream; AL handle validity; RET | complete |
| BF5030..BF5083 | 84 | ECX stream; buffer/count/optional-output stack; EAX actual; RET0C | complete consumer |
| BF5090..BF50CC | 61 | ECX stream; flags stack; EAX original address; RET4 | complete |
| BF50D0..BF50FA | 43 | ECX raw stream; EAX same address; RET | complete |
| BF5190..BF522A | 155 | ECX header; dead stream stack; RET4 | complete |
| BF52A0..BF54EC | 589 | ECX stream; path/full flags stack; RET8 | complete |
| BF5590..BF5594 | 5 | JMP BF52A0; same arguments and result | complete thunk |
| BF55A0..BF55BD | 30 | ECX zero-reference stream; RET | complete qualified consumer |

The report preserves original bytes, stored-body ranges, all direct/indirect CALL
sites, their current enclosing function, stack cleanup, register evidence and
the independent JMP thunk. BF3930's saved body originally stopped at BF3966;
raw PE bytes continue through ADD ESP,4 / POP EDI / POP ESI / RET at BF396C.
Its first free also concealed the loop continuation BF394B..BF3955. BF30C0's
post-free BF3111..BF3119 publishes data and capacity. BF4370/BF5090 each have
three hidden ADD ESP,4 bytes after scalar free. BF4FA0 was initially undefined.
The primary integrator owns saved-analysis definitions and flow repairs.

## Actual storage and composition

BF42A0 is the producer of the raw 10h pool: data/count/capacity at +4/+8/+C are
zeroed before D68EC0 is stored. Publication is the application's one 0109DC28
slot. The getter captures canonical singleton-manager section+10, locks and
increments depth, rechecks publication, allocates, publishes and registers through
the current manager, then decrements/unlocks before the final publication reload.
Shutdown is an explicit D68EC0 -> BF4370 dispatch in that application's existing
canonical lifetime callbacks. This module does not install another registry,
publication, domain, implicit destructor or callback chain.

BF3770 consumes that pool's actual 0Ch pointer header under the current B1CD90
lock owner's actual section+4. It pops the last cell before construction, leaving
the cell's old bytes. Empty lists allocate 20h. BF50D0 writes CEB130, count1,
D691B0, invalid HANDLE+8, and zero position+10/+14 and cached size+18/+1C.
Offset+C is not written. Both fresh and recycled storage use the same producer.

Reserve uses signed comparisons and minimum1, DWORD-wrapped byte arithmetic,
current count/data reads during copying, and data/capacity publication after free.
Resize preserves native repeated count-decrement stores. Append captures the
current lock, doubles capacity only when count equals capacity, and writes the
dead address without retaining it. BF3930 scalar-frees each already-dead stream,
resizes the pointer count to zero, then frees the array while retaining its stale
data/capacity fields. BF4370 clears publication first, stores CE3818, drains that
header and conditionally frees the owner. It does not unregister itself.

`NativePhysicalStreamOpenContext` borrows `NativePhysicalFileDateContext`, the
actual pool publication, canonical `SingletonLifetimeDomain`, and the existing
`NativeRenderBatchLifetime`. That last adapter supplies the already-established
B1CD90 shared lock owner; no physical-stream-only lock is fabricated.

The provider must hold numeric D69168 and the current +1C word must be BF3970.
Streams retain numeric D691B0; every reached validity/recycle/delete dispatch
reads the current table and expected +18=BF5020, +0=BF55A0, or +4=BF5090 word.
These numeric tables must be readable at their original addresses. Unexpected
profiles/slots throw an explicit source `invalid_argument`, which is a
reconstruction boundary, not recovered native error behavior. No vtable is
rewritten to a host address and no slot resolver or default provider is supplied.

## Open, reads and ownership ordering

BF4BA0 allocates/reuses the actual stream, calls current provider+1C with raw local
output and suffix, then passes the **returned** header pointer and unchanged flags
through BF5590 to BF52A0. It releases the local path before current stream+18.
Invalid streams receive one real InterlockedDecrement; zero invokes current+0.
BF55A0 first invokes current+4 with DWORD0, then reloads the actual pool and
appends the now-dead address. BF5090 always calls CloseHandle, even on -1,
invalidates+8 and writes D5C104 then CEB130. Only its flags bit0 frees storage.

Flags bit0 selects GENERIC_WRITE versus GENERIC_READ; share is `~flags & 1`.
Mask0/2/4/6/8 selects OPEN_ALWAYS/OPEN_EXISTING/CREATE_ALWAYS/CREATE_NEW/
TRUNCATE_EXISTING. For maskA/C/E the original **entire flags DWORD** remains the
creation disposition. Security, attributes and template are zero. Open never
closes an earlier handle and stores every CreateFileA result directly at+8.

Only initial ERROR_PATH_NOT_FOUND plus GENERIC_WRITE starts the directory loop.
It searches backslashes from offset3, copies each prefix through actual 469840,
accepts existing directories (error183), and stops on other directory errors.
It attempts the second CreateFileA even after that stop. All Win32 APIs are real
ANSI calls. The verified 4254B0 diagnostic is RET, so the source preserves current
name-pointer reads without inventing formatting or logging. GetFileSizeEx BOOL is
ignored; valid handles write directly to cached+18. Position high then low are
zeroed at the end, including failures.

BF4F90 returns cached64 bits; BF4FA0 calls fresh GetFileSize with local high0,
discards that high DWORD and returns EAX. Its one stacked argument is ignored;
it neither returns cached64 bits nor updates the cache. BF5030 calls synchronous
ReadFile with initial actual0. On failure it loads current published manager,
captures manager+18 into EDX, and consumes BD9E30: CALL current manager **field**+90
with ECX manager and unchanged EDX. That required original-ABI application callback
can return; null retains the native invalid-call boundary. No fallback or extra
error translation is inserted. It then adds actual to position low/high with
carry, writes optional output, and returns actual. Optional output may alias raw
stream storage; its store remains after the position update.

## Exception-state evidence and limits

Captured FH3 records E027D0/E02938/E029EC/E02AC4/E02AF0 and their funclets accompany
the report. BF3770 state0 releases the captured section, state1 has verified RET
placement-delete401130, and state2 frees the raw allocation. BF42A0 and BF5190
release their captured sections on unwind. BF4BA0 arms only path cleanup after
the path builder returns; no stream-release unwind state exists. Source `__finally`
preserves those cleanup scopes without claiming original FH3 handler ABI.

BF52A0's saved EH map has a string cleanup funclet, but its full body contains
no store arming state0. The source does not add exception cleanup around the
directory-prefix Win32 calls. The existing actual-string bridge's noexcept release,
allocator/new-handler and omitted zero-byte copy boundaries remain unchanged.
Arbitrary damaged pointers, mutation outside native locking, failed allocation,
native exceptions and alternative stream profiles were not dynamically compared.

## Verification

The MSVC Win32 Release `/W4 /WX` build and both existing CTests pass after seed
verification. No permanent test case was added. All 38 freshly guarded Ghidra
spans, 2,372 bytes, match the installed PE. The report includes full span bytes
and SHA256, current source/library/object and ignored probe artifact hashes.

The one ignored differential fixture in `local/physical_stream_an/` executes
the original instructions and the linked `bsp_core.lib` source against real
Win32 files confined to that directory. It compares 73 recorded words across
nine modes (2,32h,3,5,7,9,1,Ah,F3h), constructor untouched+C, invalid/valid
handles, cached and fresh sizes, read position carry, null output, and the
returning failure callback's ECX/EDX. Each variant executes nested directory
creation and full provider miss -> dead-slot recycle -> successful same-address
reuse -> read -> recycle -> canonical shutdown. Both pass.

The fixture bridges already-implemented allocator, canonical registration/lock,
physical-path and string helpers. Its manager facade is instrumentation for the
existing canonical lifetime service; it is not a new product owner. Calls into
those dependencies and the intentional error callback are not original-body
differential coverage of the bridged dependencies. No exceptions, OOM, arbitrary
profile changes, OS query failures after valid open, >4GB file or large
pointer-array growth were forced. A peer independently reviewed
open/read/recycle ordering against assembly and found no concrete mismatch.

Provider/index construction, remaining seek/write/other stream slots, lifetime
callback composition and application startup routing remain separate work. This
packet supplies an actual physical open/read owner chain for the integrator; it
does not claim complete VFS closure, original ABI compatibility or game validation.

## Integration correction from docs/NATIVE_LUA_VFS_BINDING.md

The primary defined BF4FA0 and decoded the reviewed BF30C0/BF4370/BF5090 gaps. BF3930 internal loop and final bytes are decoded, but the stored body still ends at BF3966; its final BF3967..BF396C tail remains outside that body. No full stored-tail repair is claimed. Actual numeric D68EC0 deletion now participates in the composition fixture canonical lifetime domain, and NativeVfsRuntimeBindings routes physical opens/reads/size/recycle into these functions. Real installed Lua file loads and shutdown pass with explicitly seeded actual manager/mount records. Manager/tree population, failure-service installation and complete application startup remain separate.

Evidence: reports/native_an_integration.json; reports/native_lua_vfs_binding.json.
