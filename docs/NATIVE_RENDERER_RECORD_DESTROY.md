# Native renderer record-array destruction

Addresses: `00B29BA0`, `00B29BE0`, `00B29C20`, `00B29C60`.

The four complete 61-byte bodies receive an actual 0Ch header in ECX and return
with plain `RET`. The header contains current data +00, signed count +04 and
signed capacity +08. The source operates directly on these bytes and reuses the
existing native renderer record reserve functions and shared lifetime free.
Descriptive names are hypotheses; no independent container or owner is created.

| Body | Inclusive range | Reserve target | Parent header |
| --- | --- | --- | --- |
| `destroy_native_renderer_records16_00b29ba0` | B29BA0..B29BDC | B228B0, stride10h | renderer+1CF4 |
| `destroy_native_renderer_records20_00b29be0` | B29BE0..B29C1C | B22940, stride14h | renderer+1D00 |
| `destroy_native_renderer_records24_00b29c20` | B29C20..B29C5C | B229D0, stride18h | renderer+1D0C |
| `destroy_native_renderer_records40_00b29c60` | B29C60..B29C9C | B22A70, stride28h | renderer+1D18 |

All four have complete source coverage. Current negative capacity causes a call
to the matching reserve with request zero. The existing reserve clamps that
request to one, performs its observed x87/DWORD record copy, frees old data and
publishes fresh data/capacity only after its free returns. These bodies add no
catch, rollback, or failure cleanup: a reserve exception prevents their subsequent
count/free work while retaining the allocator's earlier effects.

After any returning reserve, each body tests current signed count and repeatedly
decrements its current value while positive. A zero or negative count skips the
loop. The critical final order is **capture current data, publish count zero,
free captured data**. Source volatile accesses retain the count writes and that
capture-before-zero order. There is no per-record destructor, direct capacity
store, data-pointer clearing, or header free in these bodies.

After normal return, count is zero. With initially nonnegative capacity, the
capacity and data pointer retain their original bits, and that data has been
freed. A returning negative-capacity reserve normally leaves capacity one and
the fresh pointer bits; that fresh allocation is then freed here. Those stale
pointer bits are not a live allocation or an input that may be destroyed again.
Native raw extents, lifetimes, allocator ownership and wrapping-address validity
remain preconditions. No malformed-header or concurrency repair is introduced.

The read-only parent FH3 actions CBE00C/CBE01A/CBE028/CBE036 load the captured
owner from EBP-14h, add +1CF4/+1D00/+1D0C/+1D18, and tail-jump to the respective
bodies. Parent state interpretation and reconstruction remain integrator-owned.

The saved Ghidra bodies stop at the free calls. Required returning-call overrides
are B29BD3, B29C13, B29C53 and B29C93; their full tails end at B29BDC, B29C1C,
B29C5C and B29C9C respectively. Every complete span is 61 bytes, 244 bytes total,
verified live against the installed PE. No Ghidra mutation was performed by this
worker. The integrator owns listing repairs, old-value-preserving comments,
names, save and refreshed exports after the lease is released.

Verification is recorded in `reports/native_renderer_record_destroy.json`:
the strict Win32 build and both CTests after eight verified native math seeds;
one original/source fixture covering positive count, signed negative count,
negative-capacity reallocation and empty/null storage; and the direct-call audit.
The complete original destructor instructions run from relocated fixture memory,
with only their reserve/free CALL operands bound to the same substantive source
providers used by the built library. This proves the tested raw-header postimages,
not original CRT/private-frame/SEH exception identity or game behavior. Allocation
failure is a static/source-boundary contract, not an injected fixture case.

An immutable local archive holds source, library, fixture, logs and tool evidence.
Executable artifacts and loaded runtime DLLs have separate manifests. The probe
records DLLs actually loaded in its running Win32 process, resolves their files
through handles opened in that process, and records the loaded PE machine. DLL
file hashes and archived copies are taken afterward; they are not mapped-image
hashes. Tool executable hashes/versions are evidence of the observed environment,
not a self-contained archive of every toolchain dependency.

## Integrated validation at 33ac99a2

The integrated source at `33ac99a202f5630d2fe4e706609cb84fc7ffb7f9` passed the strict MSVC Win32 build and both CTests. Four complete original destructor bodies versus source,16 postimage cases. Allocation failure remains static evidence.

The existing focused fixture was relinked against that exact `bsp_core.lib` with `/MD`, `/W4 /WX`, `/sourceDependencies` and an embedded manifest; loaded runtime module paths were measured.

The packet passed 8 numeric direct-call checks. Reviewed names and evidence comments were applied, saved and read back in Ghidra while preserving previous comments. All 23 affected exports were forced after the snapshot refresh; eight repaired returning tails were checked in the live bodies and refreshed assembly. The combined checkpoint covers 22 new entry bodies including thunks and one overload of an existing body, with distinct per-entry validation scopes in the report. It retains 3390 immutable source, build, compiler, fixture and measured runtime artifacts at `local/checkpoints/33ac99a2/native-renderer-cleanup-refreshed/validation.json` (SHA-256 `9e496724999d084f197d606cf7351cc861f75a14050c0e5ad85c1ca145bef4c0`). Worker manifests and intermediate flow-repair reports remain historical inputs. Full renderer lifetime, active-frame execution, application adoption, native exception ABI and gameplay remain unvalidated.
