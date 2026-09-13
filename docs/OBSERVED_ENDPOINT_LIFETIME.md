# Observed endpoint lifetime

Addresses: `006953C0`, `00695760`.

This packet extends the existing `NativeObserverLifetime` with the observed,
or primary, endpoint role. It uses the actual raw owner/edge/dispatch storage,
the same `00E198E4` publication, existing array mechanics, real recursive Win32
sections and the current source CRT allocation domain. No new library body,
owner layout or virtual-deletion fallback is introduced.

| Entry and inclusive end | Native ABI | Coverage | Source method |
| --- | --- | --- | --- |
| `006953C0..00695525` | ECX actual primary base, no consumed EDX/stack input; RET at `00695525` | Complete normal behavior; source exception/provider limits below | `detach_observed_006953c0` |
| `00695760..00695820` | ECX actual primary base, no consumed EDX/stack input; RET at `00695820` | Complete normal behavior; source exception/provider limits below | `destroy_observed_owner_00695760` |

These C++ methods have a new class-method ABI. They are not drop-in native
replacements. Descriptive names are hypotheses; the existing reviewed
`BSP_ObserverEndpoint_Destruct` name is retained.

## Role and producer evidence

The existing `NativeObserverOwnerStorage` is a 10h-byte prefix: vtable at +0,
edge-array data/count/capacity at +4/+8/+C. Edges hold their primary endpoint
at +4, callback owner at +8 and reference count at +C. Constructors establish
these fields; no consumer-derived second layout is declared here.

The actual unit has distinct primary and callback bases at unit+0 and unit+10h.
`00925CFF` stamps the primary base `00CECCC8` and clears its array words;
`00925D13` stamps the callback base `00CE3CD4` and clears its own array words.
In `00925780`, ESI remains the unit and EDI is unit+10h. The call at `0092589D`
destroys the callback base with `00695870`; the call at `009258AC` then destroys
the primary base with `00695760`. The adjacent unit-prefix packet owns that
constructor/destructor projection and its host binding.

`local/observed_endpoint_role_equivalence.json` verifies both complete primary
spans and their existing callback counterparts against live Ghidra and the
installed PE. After normalizing relative calls and the explicitly recorded
handler/vtable constants, the detach pair differs only at `00695465` versus
`006955D5`: `CMP [EAX+4],ECX` versus `CMP [EAX+8],ECX`. Thus primary detach must
select `edge.first_04`; reusing callback detach directly would leave the wrong
copied slots live. Existing callback/individual-edge invalidation retains its
previous selection through the private helper's default argument.

## Ordered behavior

Primary detach captures and enters the actual observer section returned by
`00694280`, including its wrapping raw depth at +18h. It validates and scans the
actual shared dispatch vector, clearing every nonnull slot whose edge's primary
field matches the supplied owner. Null slots and other primary endpoints remain.
Captured iterator owner/end identities and returning validation-handler ordering
are inherited from the existing invalidation helper.

It then creates an empty temporary count/capacity array and calls the existing
`006944C0` exchange. This copies array contents; the owner's original allocation
and capacity remain, with count zero. A captured temporary cursor/end controls
deletion. For every edge, capture first and callback endpoints before removal,
remove it from the first array and then the callback array with existing
`00694F60`, reread its vtable and invoke deleting slot zero with flag 1. There is
no reference-count decrement or conditional retention, even for references >1.
The known actual `00CF7E64` profile uses existing concrete `00693CA0`; other
profiles retain the already-declared external service boundary and are unproven
by this fixture. The temporary allocation is freed before the captured lock exits.

The primary destructor first stamps `00CECCC8`. It captures an outer section,
enters a separately obtained nested section for the count read, releases that
nested section, and calls primary detach only when the captured count is nonzero.
After outer unlock, it reloads and frees the owner's backing array. It frees no
owner storage and clears no array fields; the data pointer may therefore remain
dangling. Callers must not run this destructor twice or free that pointer again.

## Exception and metadata boundaries

Detach's FH3 descriptor `00DAB68C` has two unwind states. State 1 calls
`00C7EA18 -> 0042BED0` to free the temporary array; state 0 calls
`00C7EA10 -> 00411EE0` to release the captured guard. The C++ temporary and guard
have that destruction order. The destructor descriptor `00DAB720` likewise has
two states: `00C7EA7B` releases the outer guard, then `00C7EA70` frees the current
owner+4 array. Source exceptions preserve this ordering and rethrow. Native FH3,
RTTI/SEH identity, asynchronous faults and mutable unwind-stack aliases remain
outside the new source ABI.

Root repairs decoded the three bytes after the false-noreturn free calls:
`006954FF..00695501` and `0069580B..0069580D`. Both locations still have no
containing Ghidra function, despite correct restored instructions. The full
verified PE spans establish source coverage; they do not prove complete stored
body membership. Skipped padding `0069542A..0069542F` was left unchanged. The root
defined/saved exact ten-byte handlers `00C7EA20..00C7EA29` and
`00C7EA83..00C7EA8C`. Consumed array-cleanup helper `0042BED0` has its own
recorded post-free decoding limit. Worker Ghidra access remained read-only.

## Focused verification

The ignored fixture uses two original spans totaling 551 bytes, actual
source-created observer edges and shared dispatch owner, and real Win32 locks.
Only native calls are relocated to the existing canonical array/lock/free
providers. The original `00CF7E64` value remains in each edge: an isolated
read-only mapping of its actual slot-zero address routes original virtual calls
to existing concrete `00693CA0`, with the original flag 1. No substitute edge
profile or uncertain virtual service is executed.

One paired scenario covers primary detach, an unrelated primary sharing a
callback owner, duplicate copied slots, references >1, retained array reuse,
nonempty destruction and empty destruction. The recorded 44 words compare
vtable/array fields, other endpoints, preserved references, copied-slot results
and raw/OS lock depths. Four original actual-edge deletes and three original
array frees are also checked. A source-only throwing validation service checks
guard release and retained owner fields on unwind; original FH3 is not executed.
Array-free behavior in that exceptional case follows the compiled source body,
not an independent allocator-event trace. Fixture-only repair of already-freed
array fields permits teardown of the still-live peer edge afterward.

Reproduce with `python local/prepare_observed_endpoint_probe.py`,
`local/build_observed_endpoint_probe.cmd` and `local/observed_endpoint_probe.exe`.
The probe links with `/MANIFEST:EMBED /DYNAMICBASE:NO /BASE:0x00400000` so its
isolated process can reserve the original profile address. Two earlier runs
stopped before native execution because that address lay in a private reserved
region beginning at `00C70000` (`VirtualAlloc` error 487). Their source, EXE,
object, map and logs are retained under `local/observed_endpoint_failure_*`.
Only the probe's layout changed; the production build keeps its normal flags.
The final fixture passes all 44 canonical words, four actual edge deletions,
three native array frees and the declared source-only exception observation.
`./scripts/build.ps1` passes Win32 Release and both existing CTests.
The report carries final outcomes, exact
source/object/library/executable hashes and call-site checks. No tracked test
was added. This is isolated native/source proof, not unit-host or gameplay proof.
