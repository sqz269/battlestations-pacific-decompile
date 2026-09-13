# Cockpit constructor composition contract, BG

Address: `00B3C800`. Evidence-only review, 2026-09-13. The complete constructor
body is recoverable, and the BF raw-camera and BG helper-lifetime dependencies
exist. A production constructor packet still needs a concrete preparation and
viewport-view lifetime provider. The current interfaces do not establish that
provider merely by being bindable. This packet adds no reconstructed C++ or
native-body credit.

Worker base: `8cd30e0bec7792afa60143b56bca039c58e9f881`. Provider integration
pin, read separately in `battlestations-pacific-decompile-orch3-20260910`:
`daa9706c969f965ceb70790ccd2ef68f68ca0b1b`. The camera, helper lifetime, raw
string, viewport, renderer-parameter, scene and node-lifetime files cited below
have no changes between those pins. The root's new model-base bootstrap was
also read: its distinct `0109008C` node pool is not the `0108FFB0` camera pool.
The old local BC/BD readiness notes supplied investigation leads; all conclusions
below are grounded in current source and target-guarded BSP CLI reads.

Ghidra identity: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, base `00400000`, bridge `127.0.0.1:8089`. Every live batch
used BSP CLI's verified client; there were no Ghidra writes or export mutations.

## Native body and argument ownership

| Routine | Inclusive body | Bytes | Original ABI | Coverage |
| --- | --- | ---: | --- | --- |
| B3C800 | B3C800..B3C952 | 339 | ECX actual helper, no stack args, EAX same, RET | complete static body review; implementation absent |

The complete assembly has 108 instructions and no flow gaps. ESI captures
incoming ECX at B3C81B and is never reassigned. EBX is zero at B3C81D and becomes
1 only at B3C875 after successful local-string construction. EBP is zeroed at
B3C82E and stays zero. EDI captures the raw camera slot at B3C854; it becomes
the actual constructed viewport at B3C914, or zero at B3C918. All are restored
in the epilogue. This establishes receiver, zero and local-owner provenance
across every transfer, including the last release.

The actual helper allocation is 24h. B14F12 calls BF681B with PUSH24h and
ADD ESP,4 at B14F17; B14F29 calls this constructor with ECX=allocation;
B14F2E publishes EAX to service+0C. Both call sites belong to the actual
B14A10..B14F5B body. B14A10's remaining construction is outside this packet.

Helper stores are ordered base CEB130, count1, state0, profile D61854,
then zero +08/+0C/+18. Words +10/+14/+1C/+20 retain the allocation preimage.
D61854 contains exactly `[BD30E0,B3C6C0]`; D6185C..D61869 is the literal
`CockpitCamera` including its NUL. A new constructor must start the typed 24h
storage lifetime without erasing preserved words. It must not allocate
`sizeof(NativeCockpitHelperOwner)` as the native object or add another count.

| Call site | Native target | Reviewed contract and cleanup |
| --- | --- | --- |
| B3C84F | B71930 | No size argument. Callee overwrites apparent ECX458h with 0108FFB0 and tail-jumps B71770. Capture raw 45Ch slot, with 458h payload and separate +458 metadata. |
| B3C86C | 41E870 | ECX actual local 8h header; stack D6185C; clears header, scans NUL, raw resize preserve1, copies current length+1; EAX header, RET4. |
| B3C886 | B71A80 | ECX captured raw slot, stack same local header; raw-camera overload must forward that header unchanged; EAX actual camera, RET4. |
| B3C8AE | 419CC0 | No arguments; current pool getter. Data and wrapping length+1 were captured and all three return arguments were pushed before this call. |
| B3C8B5 | BD1510 | ECX returned current pool; stack captured data, captured length+1, unused1; RET0Ch. |
| B3C8C7 | B6FBF0 | Reload helper+0C; one float32 word from current D7A2F0 via FLD32/FSTP32; callee MOVSS captures argument, masks +2F0 with FFFFFF41, writes +1D4; RET4. |
| B3C8D9 | B6FC10 | Reload helper+0C; current CE38B8 via FLD32/FSTP32; same mask then +1D8; RET4. |
| B3C8E3 | B6FE10 | Reload helper+0C; stacked DWORD6; writes all of +188; returns supplied flags; RET4. |
| B3C8F1 | B6FE20 | FLD1/FSTP32 argument; reload helper+0C; MOVSS to +18C; RET4. |
| B3C8F8 | BF681B | cdecl PUSH34h, ADD ESP,4 at B3C8FD; save raw allocation at B3C900. |
| B3C90F | B1F850 | ECX actual nonnull 34h allocation, no args; EAX same, RET. |
| B3C923 | B71990 | Reload helper+0C at B3C91A; pass actual local viewport EDI; state0 before call; RET4. Publish new +180, retain new, release captured old; identity equality skips all. |

Two indirect calls remain explicit: B3C92C invokes current IAT CE2220 on
`EDI+4`; if zero, B3C936/B3C938 load that same viewport's current profile and
slot0, then B3C93C invokes it with ECX=EDI and no stack arguments. Camera+180
is not reread as the local-release receiver. For the concrete viewport domain,
slot0 is BD30E0, forwarding flags1 to B1F8F0. Existing viewport release helpers
implement that concrete domain directly; they do not dispatch arbitrary
replacement profiles. Unknown profiles require a concrete additional provider.

Null branches are not successful fallback paths: a null camera reaches the
setters, and a null viewport reaches its +4 decrement. Admission may describe
the supported nonnull allocator domain but must not return a fabricated success.
The CRT BF681B provider's exhausted allocation throws after its new-handler loop.

## Unwind states and publication

FuncInfo DF74C4 has magic19930522, maxState5 and map DF74E8..DF750F. The live
funclets/handler now have their complete inclusive bodies:

| State | Next | Action | Inclusive extent |
| --- | --- | --- | --- |
| 0 | -1 | helper base only: CBECE3 JMP BD30F0, ECX `[EBP-1C]` | CBECE0..CBECE7 |
| 1 | 0 | return saved raw camera slot: CBECEB JMP B71350, ECX `[EBP-18]` | CBECE8..CBECEF |
| 2 | 1 | clear live bit0 before CBED03 JMP41DD20 on header `[EBP-14]` | CBECF0..CBED08 |
| 3 | 0 | same conditional string action; no normal-body assignment to state3 | CBECF0..CBED08 |
| 4 | 0 | BF65AC on saved viewport allocation, CBED0D CALL; POP ECX; RET | CBED09..CBED13 |

CBED14..CBED1D loads DF74C4 and tail-jumps the existing FH3 runtime at BF6B43.
This is metadata interpretation, not a replacement of that runtime. No missing
functions or truncated bodies remain in the constructor's scoped EH set.

Canonical post-prolog ESP+10/+14/+18/+1C/+20/+2C correspond respectively to
handler EBP-20 live bits, EBP-1C helper, EBP-18 allocation, EBP-14 string length,
EBP-10 data and EH state. Push-adjusted offsets at B3C87D/882 and B3C91E name
these same slots.

State1 is armed after the raw slot is saved. A 41E870 exception returns the
raw slot and runs helper base cleanup. State2 is armed, with the string live
bit recorded, before B71A80. A camera-construction exception first takes that
callee's cleanup, then string cleanup, raw-slot return and helper base cleanup.
Do not delete a completed camera as a substitute for B71350 raw return.

B3C892 publishes the camera before state0 at B3C895 and before the local
string return. Any later getter/setter/allocation failure runs only helper
base cleanup: it does not release the published camera. The normal string
return can reuse raw41DD20 because it captures data and length+1 before its
current getter and leaves the header untouched. Consume the caller's state
before this operation so a thrown return is not retried by automatic RAII.

State4 protects only the raw viewport while B1F850 constructs it. On failure,
its own base cleanup precedes free of that exact allocation and helper base
cleanup. State0 is restored before B71990. Neither that call nor the final
local decrement has an armed local viewport cleanup. Nested throwing cleanup
requires the existing explicit C++ termination boundary; unrestricted FH3/SEH
equivalence remains outside the source projection.

## Providers now available

* `native_camera_pool.cpp`: existing static B71930 allocation and B71350 return
  over the actual bound 0108FFB0 pool. Pool, allocator-list domain and Win32
  section survive every slot and process-exit registration. The model-base
  0109008C pool/bootstrap supplies no substitute for this identity.
* `native_physical_file_date.cpp:162` and `native_string.cpp:129`: concrete raw
  41E870 and 41DD20. `NativeStringRawPoolContext` borrows current 01090AA8,
  01090AA4 and 01090AA0 publications; no semantic dummy pool is needed.
* `native_camera_owner.cpp:256`: raw B71A80 and raw-domain `finish_node` are
  implemented. Node and viewport must borrow the same actual D7A24C cell.
  Owner preparation preserves 458h preimage and leaves +458 pool metadata.
* `native_camera_configuration_leaves.cpp`: use the actual-address leaves.
  Their EDX arguments are addresses of already-materialized DWORDs, not native
  stacked arguments. The new caller must separately emit all three original
  x87 materializations; do not replace them with MOVSS, bit copies, or a C++
  `1.0f` constant and claim x87 exception/rounding behavior.
* `native_viewport_owner.cpp:57`: actual 34h construction and shared malloc/free
  allocation wrapper preserve native state4 cleanup. It reloads F8D394
  independently for width and height. `native_renderer_parameters.cpp` provides
  an actual +1A14-region binding and current dispatch access, with no fallback.
* `native_cockpit_helper_lifetime.cpp`: BG owner/reference bind already-live
  24h storage, and use the canonical node lifetime registry for +0C/+08.
  They neither construct B3C800 nor prepare a camera association automatically.

These are current source contracts, not new full dependency reconstructions or
fresh dependency runtime validation by this packet.

## Remaining blockers and concrete next packet

The next useful packet is **cockpit construction admission and persistent
companions**, with no native-body credit. Own new
`include/bsp/native_cockpit_construction_runtime.hpp` and
`src/native_cockpit_construction_runtime.cpp`, their evidence artifacts, and
explicitly coordinated edits to scene/node lifetime registration and viewport
view registration/retirement. Do not start the 339-byte implementation by hiding
these dependencies behind a successful generic callback.

1. Before native constructor events, allocate persistent companion storage and
   reserve exclusive scene/lifetime association capacity. Current
   `NativeCameraOwner` preparation calls `SceneAttachmentRuntime::bind`
   (`scene_attachment.cpp:269`, vector push_back at280); after B71A80 succeeds,
   `NativeCameraReference` calls `GeneratedModelLifetimeRuntime::bind`
   (`generated_model_lifetime.cpp:42`, push_back at48). Neither runtime exposes
   a reservation/admission API. Merely preallocating the owner leaves those host
   allocation failures inside the native sequence. Add an explicit capacity
   reservation whose consumption and uniqueness checks cannot allocate on the
   admitted path; define cancellation and reentrant consumption limits. Keep
   one canonical identity registry. This is a host boundary outside native EH.
2. Place the camera owner over the actual slot only after B71930 returns, using
   reserved storage. After successful B71A80, bind its reference without a new
   allocation before publishing helper+0C to later callbacks. Preserve prepared,
   constructing, live and dead phases. On partial camera failure abandon its
   host preparation, then return the raw slot exactly once. On a later helper
   failure preserve the published live camera and its persistent companions;
   do not let a stack destructor silently clean it up or invalidate it.
   Later scalar calls use freshly loaded actual helper+0C. B71990 must resolve
   that current identity to its canonical camera owner, not close over the first
   allocated owner; document the concrete supported profile and alias domain.
3. Supply a persistent resolver for both the camera's B71A80-created viewport
   and the replacement B3C800 creates. The only concrete resolver found in
   current `src/`/`include/bsp/` is `native_camera_probe.cpp:33`, with one optional
   prebound view; it is insufficient as the production composition contract.
   Pure lookup cannot allocate or synthesize a new view. Specify registration
   before any permitted frame borrow, and retirement after the actual owner and
   every borrow allow it. Audit allocation/publication points inside B71A80,
   B71990's old-owner release and final viewport destruction when selecting the
   narrow provider edits. A stale address-keyed view must not alias reused raw
   storage. Native viewport counts remain authoritative.
4. Bind real F8D394 publication and its current renderer dispatch, actual
   +1A14 parameter region, shared D7A24C and D7A2F0/CE38B8 source cells, actual
   camera/node tables, type/CRT providers, raw string context, pool and scene
   associations. These and disposal contexts outlive helper destruction and
   queued camera references. No fallback dimensions, identity matrices, dummy
   pools, transient stack companions or opaque successful native calls qualify.
5. Prepare helper companion storage separately, then bind the existing BG
   `NativeCockpitHelperOwner`/reference only to successfully constructed typed
   storage. Decide retirement ownership explicitly: freeing the helper cannot
   free a camera companion still retained by a queue. Fresh constructor +18 is
   zero; an unknown nonnull producer blocks unrestricted later helper use, not
   the established fresh-construction branch.

After this provider packet is reviewed, the disjoint implementation packet owns
only `B3C800`, new `native_cockpit_helper_construction.hpp/.cpp`, its doc/report,
and integrator-owned source registration/name ledger changes. Implement actual
typed 24h storage, the twelve direct calls, both indirect effects in their
concrete domain, x87 materialization and the five-state metadata projection.
Return the actual helper address. Reuse the existing raw string/camera/viewport
providers and BG lifetime; grant only 339 bytes of constructor credit.

The source packet should run `scripts/build.ps1`, existing relevant checks and
one focused preimage/publication/failure check only if an uncovered behavioral
risk requires it. Inspect the generated Win32 caller assembly for x87 loads and
spills. This evidence packet passes `tools/verify_report_calls.py` with 19 numeric
call rows checked and zero failures; the two indirect rows were reviewed manually.
It makes no build, fixture, binary ABI, original FH3/SEH, concurrency or game-validation
claim. Full B14A10 render-service construction remains upstream-gated.
