# Native physical-provider pool lifecycle

Addresses: `00BF3250`, `00BF3430`, `00BF33A0`, `00BF2E30`, `00CD9010`, `00CE10F0`.

This reconstructs the operational lifecycle of the actual raw `38h` provider
pool corresponding to `0109DBF0`. It composes with `native_physical_provider_pool`
and the application's canonical `AllocatorListDomain`, whose head represents
`00E188B4`. It does not construct or destroy providers inside slots. The physical
**stream** pool at `BF30C0/BF3670` is a different pool.

Names below are descriptive hypotheses. Source interfaces are new MSVC Win32
C++ interfaces; original FH3 metadata, CRT typeinfo, binary vtable execution and
fixed-image address identity remain external boundaries. No game validation.

| Entry | Last instruction | Byte end exclusive | Original ABI | Coverage |
|---|---|---|---|---|
| BF3250 | BF3322 RET | BF3323 | ECX fresh owner; EAX same; RET | complete operational source |
| BF3430 | BF34CF RET | BF34D0 | ECX owner; D68E68 virtual0; RET | complete |
| BF33A0 | BF3429 RET | BF342A | ECX owner; RET | complete |
| BF2E30 | BF2E3D RET | BF2E3E | ECX table header; RET | complete |
| CD9010 | CD9025 RET | CD9026 | no arguments; EAX atexit result | complete raw listing/source |
| CE10F0 | CE10F5 JMP (5 bytes) | CE10FA | select ECX0109DBF0; tail BF33A0 | complete |

All six spans, totaling 555 bytes, were compared between the installed PE and
live Ghidra. Every live batch used `bsp.py ghidra`, whose client verifies
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before querying.
The report carries exact bytes, hashes, stored body ranges and all xrefs.

## Storage and publication

BF3250 is the owner-field producer: canonical allocator element at `+00/04/08`,
real `CRITICAL_SECTION` at `+0C` (18h bytes), explicit depth at `+24`, pointer
table/count/capacity at `+28/+2C/+30`, first available block at `+34`. BF2D50,
reviewed in the preceding primitive packet, produces 1F4h blocks with eight
3Ch slots, each block ID at slot+38, reverse WORD free indices at block+1E0,
and WORD free count at +1F0. Final block+1F2 padding is untouched.

Host setup binds the actual D68E68/BF3430 virtual0 implementation before
publishing an element. `bind_native_physical_provider_pool_trim` changes no raw
bytes or links. BF3250 then prepends the same element in the shared list, writes
D68E68, initializes its real section and headers, and publishes capacity32 before
allocating80h. It retains current-count/current-table reloads, old-table free and
replacement publication even though ordinary startup begins with zero count.
The binding is host metadata; destructor/unwind does not erase it or own storage.

Static setup additionally borrows the one actual owner/domain until CRT exit.
CD9010 calls BF3250 and then real `std::atexit` with the CE10F0 source callback.
Registration status is returned without rollback. There is no lazy startup or
singleton-manager registration. Rebinding, repeat startup and concurrent trim
are outside the contract. Primitive callers supply the same owner/domain through
their existing `NativePhysicalProviderPoolContext`.

## Trim, teardown and failure ordering

BF3430 takes no lock. For each block whose free count equals8, it frees the
block, reloads the table/count, moves the last pointer into the vacated cell,
decrements count, rewrites **all eight** moved slot+38 indices, and retries that
position. Thus a moved empty block is freed during the same pass. It retains the
backing table/capacity and finally resets first to FFFFFFFF before scanning for
the first block with nonzero free count. That scan captures its table cursor once
while reloading count. No provider payload or block padding is rewritten.

BF33A0 captures its initial count comparison before storing D68E68, frees every
current block in ascending table order, frees a nonnull current table, drains
positive **signed** depth with real `LeaveCriticalSection`, deletes the section,
then publishes D7A0C0 and unlinks from the same list. It leaves its own old links
and table/count/capacity/available words intact and does not free the owner.
BF2E30 separately captures a table's first pointer and frees it only if nonnull;
the three header words remain unchanged.

Constructor handler CC7A6E loads E02794 and tail-transfers to original FH3
BF6B43. Unwind map E0277C is state0->-1 CC7A50/403970 (base), state1->0
CC7A58/402F70 (section), state2->1 CC7A63/BF2E30 (table). The main body stores
state0 at BF3298 and goes directly to2 at BF32C3 after section/header setup.
Source `__try/__finally` expresses that cleanup chain with the actual raw table,
real section and shared list. These consumed funclets and runtime dispatcher
were read; they are not separately reconstructed or named by this packet.

| Containing body / call site | Callee or service | Reviewed contract / cleanup |
|---|---|---|
| BF3250 / BF32A2 | IAT CE220C InitializeCriticalSection | captured pool+0C; stdcall4 |
| BF3250 / BF32D2 | BF55BE operator_new | malloc/new-handler/retry/throw; PUSH80, ADD ESP4 at BF32D9 |
| BF3250 / BF3304 | BF6989 CRT free | current old table; ADD ESP4 at BF3309 |
| BF3430 / BF3451 | BF65AC -> BF9DC8 CRT free | block; ADD ESP4 at BF3469 after table/count writes |
| BF33A0 / BF33B8 | BF65AC -> BF9DC8 CRT free | current indexed block; ADD ESP4 at BF33C0 |
| BF33A0 / BF33D0 | BF6989 CRT free | current table; ADD ESP4 at BF33D5 |
| BF33A0 / BF33ED | IAT CE2210 LeaveCriticalSection | EBX captures IAT at BF33E2; decrement depth first |
| BF33A0 / BF33F7 | IAT CE2214 DeleteCriticalSection | actual pool+0C; stdcall4 |
| BF2E30 / BF2E37 | BF6989 CRT free | captured nonnull pointer; POP ECX at BF2E3C |
| undefined CD9010 / CD9015, CD901F | BF3250, BF6FF5 atexit | raw CALL rows separately recorded; POP ECX at CD9024 |

BF55BE, both free implementations, BF6FF5/BF6FB9, shared section cleanup402F70
and base unlink403970 were inspected before assigning these host contracts.
Deep CRT allocator/SEH internals remain library boundaries. Source allocation
uses existing `singleton_lifetime_allocate/free` and actual Windows services.
The CRT exit thunk and FH3 funclet jumps are separate from checked CALL rows.

## Saved-analysis follow-up for the integrator

Worker Ghidra access was read-only. CD9010 is still undefined and needs explicit
`[CD9010,CD9026)` definition. CE10F0 is defined; its saved `CG_static_init` name
misidentifies the exit callback. Parent annotations must preserve old comments.

False no-return call-site overrides hide these reachable spans (ends exclusive):
BF3250/BF3304 -> `[BF3309,BF330C)`; BF3430/BF3451 -> `[BF3456,BF348D)`;
BF33A0/BF33B8 -> `[BF33BD,BF33C8)`; BF33A0/BF33D0 -> `[BF33D5,BF33D8)`;
BF2E30/BF2E37 -> `[BF2E3C,BF2E3D)`. Trim gaps `[BF343D,BF3440)` and
`[BF34A8,BF34B0)` are unreachable alignment. Full raw/live-byte review covers
every operational instruction despite those saved listing gaps.

## Validation and retained evidence

`scripts/build.ps1` passed with process-scoped `MSBUILDDISABLENODEREUSE=1`.
After `ghidra_export.py verify-seeds`, both existing CTests passed (2/2).
`verify_report_calls.py` checked all six ordinary CALL rows with zero failures.
No permanent tests were added.

One ignored fixture in `local/provider_pool_lifecycle_ar_v4/` compared 4,525
raw block/header values between original and source runs, including actual shared
list virtual trim, middle-block removal followed by a moved-empty retry, all
eight moved IDs and retained payloads, availability rescans, retained capacity,
exact three-block then table CRT free order, actual recursion2 drain, and tail/head unlink. Null and
nonnull table cleanup retained all three header words. Guard-page exception
injection before section setup and after header setup verified source state0 and
state2 unwind to base unlink. That test does not execute original FH3 dispatch.

Separate native/source subprocesses exercised real CRT registration and LIFO
exit cleanup, with a verifier registered before pool startup. Six original
operational bodies (555 bytes) were byte-identical before/after execution. Code
pages were relocated by30000000; absolute pool/list/IAT cells remained explicit.
External CRT allocation/free entries route to the existing real host CRT service;
original BF65AC still tail-transfers to BF9DC8's redirected library boundary.
The absolute CE10F0 registration cell has a fixture bridge to the unchanged
relocated CE10F0 body. A fixed-array observer around the host free IAT immediately delegates to the captured
genuine CRT free and verifies the three-block/table release order. These are
fixture ABI bindings, not original CRT/FH3 ABI.

The first fixture in `local/provider_pool_lifecycle_ar/` failed reserving the
whole native image before entering any original body. Its executable, inputs and
failure result remain frozen separately. The successful v2 fixture also remains
intact; v3 failed linking an x86 import name before execution, corrected in the
separate v4 fixture. The final successful version freezes source,
all compile headers, relevant CMake object files, actual linked archives, probe
source and build command before compilation; it freezes the executable/object
before running. Its input and executable hashes were checked again afterwards.

Selected SHA256 values (full manifests and commands are in the report):

| Artifact | SHA256 |
|---|---|
| lifecycle source | 6da8617044b9bedb8762aa325f9041e38ded869aaf455f21c95e2a14b6aba9d7 |
| lifecycle header | 04175010f383b41ebaa628a2044af2cb408ed85636cb049bd7913c4f50c57e53 |
| compiled lifecycle object | 0253f6a84819cc5b8b558ba5c753cea675df6be1a30a974b8a9c0526ec3a784c |
| linked bsp_core.lib | 343b7caef18abfee67a574073a7937a962946c160b55992f317eb20659166f0b |
| successful probe.exe | 6a6a7b1b9f69c8c40ccdc4d0de740c1a0b4cb2a2a42b9eb23b3ee1df9f67dbbc |
