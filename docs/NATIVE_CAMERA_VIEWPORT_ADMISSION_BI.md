# Explicit first-viewport admission for raw camera construction, BI

Addresses: none added. Existing native context is `00B71A80` and its viewport/EH
dependencies. This is a design-only follow-up, with zero native-body credit.
First-viewport registration before production camera publication is still absent.
The parent's BH fixture registers that first viewport manually after B71A80;
the existing admitted factory is used for its replacement.

Worker base: `a92db8bda296bb245b8259aeb59c23ea2dd69495`. Viewport producer source
was read in `J:/PROG/battlestations-pacific-decompile-orch3-20260910`, pinned clean
at `9b890a53e587ae0c62bfbfdc10d3f1c9502cfad8`; the report records file hashes.
The earlier BH viewport and BG composition documents supply ownership boundaries;
their historical missing-API statements are superseded by the current source.

## Smallest API and shared body

Add only this public overload in `native_camera_owner.hpp`:

```cpp
void* construct_native_camera_00b71a80(NativeCameraOwner&,
    const void* actual_name_header, const NativeNodeRawConstants&,
    NativeViewportRegistry::Admission&& first_viewport);
```

Move the current raw body into one file-private helper taking those first three
arguments plus `NativeViewportRegistry::Admission*`. The existing raw overload
passes null; the new overload passes the address of its argument. Keep the
`NativeString` semantic overload and its body unchanged. It has a different
name-pool/constructor path and needs no admitted API for this raw-production
packet. A generic callback/template strategy would widen the change without
improving this boundary.

Inside the helper, preserve the existing preflight order: prepared owner,
`require_raw_name_pool()`, then exact shared D7A24C cell. Before changing phase or
calling the native node constructor, if a token was supplied:

1. Call its existing `require_registry()` to validate the installed registry and
   reserved record.
2. Require `&owner.environment.viewport_views` to equal the returned registry's
   `CameraViewportResolver` base address. The managed path must publish into the
   same canonical resolver its camera frame uses. Legacy paths remain unchanged.
3. Move the token into a local `Admission prepared`. Retain only a local Boolean
   selecting the admitted path; never consult the caller's token after this move.

These checks and the nonthrowing move occur before `owner.phase = constructing`
and before B6F5A0. There is no environment field, shared next-token slot, token
queue or token retained by `NativeCameraOwner`.

At the existing first allocation/publication expression, use
`allocate_native_viewport_owner(environment.viewport, std::move(prepared))` for
the admitted path and the unchanged ordinary overload otherwise. Assign the
returned pointer directly to `tail.viewport_180` as today. The current admitted
factory validates/moves its token, calls the original allocation/initialization
wrapper, then invokes nonthrowing `registry.constructed` before returning.
That establishes the live record after successful B1F850 and before camera+180
publication. It neither retains the viewport nor initializes another count.

## Native ordering and unwind evidence

Target-verified BSP CLI read the complete live B71A80 listing: 133 instructions,
`00B71A80..00B71CDB`, 604 bytes, ECX camera, stacked name, EAX same, `RET 4`.
ESI captures the receiver at B71AA0 and is unchanged until restoration; EBX is
zero from B71AAC. B71ABA calls BF681B after `PUSH 34h`; B71ABF cleans four bytes.
B71AD1 calls B1F850 on the captured allocation. B71ADD publishes the returned
pointer to camera+180. The new host registration belongs between successful
B1F850 return and that publication; it must not read camera+180 beforehand.

B71AEF restores state0 after publication. B71B10 clears +438 before state2 is
armed at B71B21. The remaining listing, including both renderer queries,
viewport setters, x87 FLDZ/FLD1 materialization, look-at call, constants, stores
and epilogue, is unchanged by the proposed shared-body extraction.

CC1AF1 loads FuncInfo DFAA38 and tail-jumps FH3. Its magic is `19930522`,
maxState3, map DFAA20..DFAA37. The complete map and funclets are:

| State | Next | Action |
| --- | --- | --- |
| 0 | -1 | CC1AD0..CC1AD7: captured camera, tail B6F440 at CC1AD3. |
| 1 | 0 | CC1AD8..CC1AE2: saved raw viewport, BF65AC at CC1ADC, `POP ECX`, return. |
| 2 | 0 | CC1AE3..CC1AF0: captured camera+438, tail 605FD0 at CC1AEC. |

B1F850's complete normal listing and CBCCB0 base-unwind funclet were also read.
Its two renderer callbacks precede successful return. The complete B6F440 and
605FD0 listings confirm the scoped cleanup targets; neither adds camera+180
cleanup. This is a scoped native ordering/EH audit, not new FH3/SEH equivalence
or a fresh reconstruction of every called dependency.

## Failure, reentry and record ownership

| Boundary | Required token/record result |
| --- | --- |
| Preflight rejects before transfer | Leave supplied token and owner unchanged. Caller can correct the contract or cancel the reserved record. |
| Raw node fails after transfer | Keep existing end-tail/native cleanup; local unused token cancels during unwind. Caller still returns the camera slot exactly once. |
| Viewport allocation/initialization fails | Existing wrapper performs exact raw free after B1F850 cleanup; its moved local token cancels. No live viewport record or +180 publication exists. |
| Factory succeeds | `constructed` consumes the exact record before returning; all local tokens are empty when +180 is published. |
| Camera later fails in state0 or state2 | Preserve its already-live viewport and record. Keep existing +438/node cleanup. Token destruction must not release, retire or forget that viewport. |

Cancellation marks a record `cancelled`; it does not unlink/recycle its storage.
Use existing `forget_quiescent` before reclaiming cancelled storage. A consumed
record can remain live after the camera becomes dead. Its persistent owner must
survive independently of camera preparation, helper failure and admission-ticket
destruction; do not reread ended camera storage to recover its identity.

Same-thread callbacks in node construction, allocation or B1F850 may prepare and
construct independent owners with independent tokens/records. The outer token
has already moved into private local storage, so callbacks cannot consume it by
using the caller's moved-from variable. No registry mutation invokes user code.
The single installed registry must remain installed; nesting does not install a
second binding. Reentry into an operation's mutation, concurrent mutation, reuse
of the same constructing camera, and borrowing a finished view during B1F850's
earlier callbacks remain unsupported. This does not forbid unrelated release,
field mutation or independently prepared nested construction. Native viewport
borrows still require their actual owner to survive the last field read.

## Ready implementation packet and limits

Own only `include/bsp/native_camera_owner.hpp`, `src/native_camera_owner.cpp` and
the implementation's doc/report. Prerequisites are the merged viewport registry,
admitted viewport factory and camera binding overloads. No viewport-registry,
reference, CMake, ledger, native-layout or native helper-constructor edits are
needed. Reuse the parent's coupled fixture with this overload replacing manual
first registration; retain its byte/trace and early/late failure checks. Build
the exact combined tree with matching headers/libraries when implementing.

This packet runs only static/source review, JSON/diff checks and
`tools/verify_report_calls.py` (eight direct/tail rows checked, zero failures);
no build or tests. Full cockpit construction,
second-viewport/helper sequencing, persistent block reclamation, arbitrary
destructive callback safety, native execution and game validation remain outside
this design. The overload and first-production-publication behavior are not yet
implemented by this document.
