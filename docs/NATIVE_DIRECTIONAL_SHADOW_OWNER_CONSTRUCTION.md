# Directional shadow owner construction

Addresses: `00A8E2E0`, `00A8FA30`, `00A8FD30`.

The three source entries construct actual 508h shadow storage using the existing
camera, viewport, texture-cache and frame-target providers. They consume one
prepared `NativeDirectionalShadowConstructionBlock::Admission`; nested
factory/derived/base bodies share that same execution. Names are descriptive
hypotheses, not recovered game symbols.

| Routine | Original ABI | Coverage |
|---|---|---|
| A8E2E0..A8E85F, 1408 bytes | ECX actual owner, stack light, EAX owner, RET4 | Complete normal base body; 19-state cleanup composition |
| A8FA30..A8FCB0, 641 bytes | ECX actual owner, stack light, EAX owner, RET4 | Complete normal derived body; states -1/0/1/2 |
| A8FD30..A8FD8F, 96 bytes | ECX light, EAX owner/null, RET | Complete factory body; states -1/0 |

Normal-body attribution is **2145 bytes**. Referenced EH actions and all existing
callee bodies receive no additional credit. Source C++ context/admission
interfaces add arguments and host bookkeeping; they do not reproduce original
register, public-stack or private FH3 ABI. The derived range includes the
unreached three-byte alignment instruction A8FBFD..A8FBFF.

## Preparation and borrowed domains

The caller first prepares DY's stable block: four scene credits, four lifetime
credits and nine named viewport admissions, with four optional camera companion
pairs and persistent cache-operation storage. The public source entry validates
the token and exposed domain identities before consuming it or writing native
storage. It uses the concrete VFS-name binder and the existing static camera-pool
binder. The latter rejects an already-bound different pool.

The context borrows the same camera environment/constants/installed registry,
canonical node runtime, actual texture cache and VFS context, DU destruction
context, live F8D394/F8BBF0 publication cells, D5F0A8 table and original literals.
Available string/cache/load/notification, texture-owner, renderer-cell and
frame-target/surface identities are checked. These checks compare bindings,
not current renderer/target values or table slots. The cache's private raw
string-pool cells cannot be inspected: wiring them to the camera raw pool's
01090AA8/01090AA4/01090AA0 cells remains an explicit caller obligation.

The supplied owner for a direct constructor must be aligned, writable, unused
508h storage. Its extent is a caller contract; the source API has no extent
argument. Actual light and provider storage must remain valid at all reached
native accesses. Neither direct entry frees the caller's outer allocation.
Only the factory requests and owns `{object,0x508,0x508}`.

## Native schedule and exact admissions

The base stamps CEB130/count1/D5B574 and writes only the reached fields. It reads
F8BBF0 separately for +388 and +38C, copies the borrowed light and light+A4, then
captures F8D394 before creating the actual white-string header. Resize9/preserve1
and overlap-safe BF7680 composition precede the current profile/slot64 read.
Supported D5F0A8/+64 dispatch invokes existing B319B0 with word0 and the block's
persistent acquisition. It publishes returned +384 without an extra retain.
Normal string cleanup captures data before state disarming, then length+1 before
the current 419CC0 getter and concrete BD1510 return.

| Persistent slot | Native use |
|---|---|
| V0 | Base standalone viewport +504 |
| V1..V4 | First B71A80 viewport for cameras C0..C3 |
| V5..V8 | Replacement viewports published at +10/+14/+18/+1C |
| C0..C3 | Actual 45Ch pool slots; owners/references published through +20/+24/+28/+2C |

The five standalone viewports use exact 34h shared allocations and B1F850. The
parent owns their raw-free states; registry construction consumes the exact
record admission after successful initialization. Camera owners begin only
after actual pool allocation and local-name construction. Each B71A80 consumes
its own first-viewport admission. Successful return sets `camera_completed`
before nonallocating lifetime binding and publication; no retain is added.

Four render-mode calls precede four clear-flags calls, then four clear-color
calls. Current camera identities are resolved through the canonical runtime for
operations requiring companions. Four replacement publications precede B71990
calls; each reads current fields at its native step. This allows a callback to
replace a camera with an independently prepared canonical camera. Each depth
call performs its own FLDZ/FSTP32 or FLD1/FSTP32. The final +504 dimensions use
current saved +388/+38C. SSE moves preserve the final zero/one/scalar field-store
schedule, reading the shared D7A24C and CE77FC bindings at the final sites.

Derived construction arms state0 only after the base returns, stamps D5B5D8,
and sets each quadrant using unsigned DWORD SHR1. It rereads dimensions for
each call and reuses the existing A8AAA0 and sequential x87 4134F0 copy into
+244/+284/+2C4/+304. It captures A8FDA0 once, then constructs four 40h targets
and one fifth target. Each is published before color/depth setup. Each depth
setup freshly reads F8BBF0/A8FDC0 and reloads the published target after the color
call. Explicit allocator-null arms remain; the existing shared allocation
provider ordinarily throws on exhaustion, so no successful null child or
safe-null callback is fabricated.

## Cleanup, persistent lanes and settlement

DY supplies raw aligned base4Ch, derived60h and factory4-byte lanes. Header and
scalar access starts only reached native stores, without initializing whole
lanes. The base public word is reused for current raw allocations. Derived
width storage later holds the loop allocation; its public light word becomes
loop count4, then the fifth raw target. Matrix temporary +0C..+4B retains its
actual 40h identity. Phase/state/site metadata is separate host bookkeeping.

| Base state | Next | Action |
|---|---|---|
| 0 | -1 | BD30F0 only |
| 1 | 0 | Current white-header destruction |
| 2, 15, 16, 17, 18 | 0 | Free current public raw viewport allocation |
| 3, 6, 9, 12 | 0 | Return current public raw camera slot |
| 4, 7, 10, 13 | 3, 6, 9, 12 respectively | Consume name flag and destroy current header |
| 5, 8, 11, 14 | 0 | Same conditional name actions; map-only states never published normally |

The implementation consumes each cleanup edge before its action. Previously
published children are not reclaimed by base unwind. Prepared camera companions
are abandoned before the matching raw pool return; callee-unwound dead
companions remain persistent. A source-only reference-binding rejection after
B71A80 success suppresses raw return and preserves the live companion. It is
outside admitted native exception equivalence and can pin the block indefinitely.

**DX correction:** A8E5DA..A8E5F4 normal fourth-name cleanup does not clear bit8.
The implementation preserves that bit. Its EH action CB6496 still consumes bit8
before calling 41DD20. Earlier normal name cleanups consume bits1/2/4.

Derived state1 frees only the current loop raw target; state2 frees only the
fifth raw target. Both continue to state0/A8DEC0. Factory state0 frees only its
508h allocation after inner unwind. A consumed state records that a raw identity
has been freed; stale lane bytes are not a recovery API. A second cleanup
exception terminates under the stated source boundary, without claiming native
FH3/foreign-unwind/hardware-fault parity.

Executing admission destruction settles only after normal/native cleanup and
cancels unused credits. It never releases published survivors or resets the
cache operation. Existing indexed camera retirement marks host facts; later
`reset_after_host_quiescence()` requires actual retirement and independent proof
that semantic borrows ended. Late B71A80 orphan viewport records and failed cache
trees can remain pinned indefinitely. No dead-camera read, orphan accessor,
automatic release or forged completed phase is introduced.

## Evidence and limits

The report pins the accepted DX/DV packets, source providers, normal bytes,
call sites/owners and native state maps. All 28 DX input pins matched the
implementation base d68f8770 before edits. DY is an explicit source dependency;
the existing DU and DW bodies are consumed unchanged.

This module adds no host allocation during its native sequence beyond existing
providers. The full cache/VFS/texture child path can allocate retained host
metadata; this is not a whole-call no-allocation claim. Provider string-getter,
COM, pool, current-table, canonical-registration, callback and noexcept limits
remain. Unsupported bindings produce source diagnostics. Direct base return
does not add the deferred D5B574/A8E160 deleting dispatch. Concurrent invalidation,
arbitrary private-frame aliases, unmasked faults, native binary compatibility,
runtime, GPU and game behavior require separate evidence.

The single clean source commit `c6ea046ecdde8a4bc6cc892e2c0787e5f9987dcb`
passed the Win32 Release build with no compiler warnings/errors. All 2632 inputs
(2631 tracked inputs plus the verified seed header) remained unchanged. Both
existing CTests passed after all eight seed ranges matched the executable.
Primary and independent source reviews found no remaining issue after adding
two required direct includes; primary checked all 91 direct call rows with zero
failures. The indirect slot64 remains separately current-profile-qualified.

The report pins the full input manifest, four libraries, build log and generated
object. No new tests or constructor fixture ran. Separate primary/independent
generated-code review is pending at this metadata commit; compilation and these
general math tests do not establish constructor runtime or game behavior.
