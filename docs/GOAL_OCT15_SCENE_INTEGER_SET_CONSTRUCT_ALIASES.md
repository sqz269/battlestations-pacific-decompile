# Actual integer-set constructor variants

Both complete bodies initialize an actual integer-set tree using its accepted
`14h` (20-byte) head provider. Their original descriptive STL names are retained;
these are behavior hypotheses, not recovered symbols. No prior Source or indexed
reconstruction was found for either body.

| Entry | Inclusive body | Bytes / instructions | Original ABI and raw result |
| --- | --- | --- | --- |
| `00468940` | `00468940..00468968` | 41 / 14 | ECX tree, EDX unused, ESI preserved, no stacked arguments, RET; EAX is the last current-head capture |
| `0046CF10` | `0046CF10..0046CF3A` | 43 / 15 | ECX tree, EDX unused, ESI preserved, no stacked arguments, RET; EAX is original tree |

Both are single-block bodies with one physical native call. There are no live
code/data xrefs or callers for either entry, so no caller establishes a semantic
return type for the first body's raw EAX. The new ordinary Source interfaces
return actual pointers to expose the established register values; they do not
claim the original calling convention.

At `00468943`, `00468940` calls `004682B0`.
At `0046CF13`, `0046CF10` calls `004682B0`.
That concrete accepted provider requests exactly `14h` bytes through the real
current CRT allocation service, preserving its allocation/new-handler/failure
behavior. The admitted provider returns nonnull or throws. Its conditional link
zero stores and color10=1/nil11=0 stores remain in the provider; no replacement
allocator, callback, stub, assumed host node or success-only wrapper is supplied.

Allocation completes before any tree write. Each body publishes the allocated
head into tree+4, then writes nil11=1 through the captured allocation. It reloads
CURRENT tree+4 separately before each parent+4, left+0 and right+8 self-link,
and zeros count+8 last. Tree+0, scalar key+Ch and head padding+12h/+13h are
untouched by the constructor. Only the three current captures determine those
self-links; they are not replaced by the original allocation. Reached raw storage
and releases must be valid; no implicit alias to Source/native private frames,
register homes or concurrent mutation is added.

`00468940` leaves EAX equal to the third captured current head after the right
self-link and final count store. Source retains that value across the count
store. `0046CF10` instead executes MOV EAX,ESI after the final count write and
therefore returns the original tree. The latter's complete memory/result schedule
matches accepted `00468EA0` Source, which it reuses. The native `00468EA0` variant
has two unread stack words at entry+4/+8 and RET8; these new native variants have
plain RET and no stacked words. The existing ordinary provider deliberately omits
those unread words. This reuse describes Source composition only; both actual
native transfers target the concrete head allocator directly.

Neither body has an EH frame, local cleanup, prior-head destruction, rollback or
extra parameter validation. Allocation failure reaches no constructor tree writes;
all concrete provider behavior remains composed at its established current CRT
boundary. Original CRT/global identity, native register/stack ABI, instruction and
fault-address identity, and application/game behavior are unproved.

## Verification

The complete live bodies match all 84 installed PE bytes / 29 instructions.
The two typed native call rows are checked separately from Source composition.
Correct library names and old comments/evidence are retained through the locked
annotation tool, with prior-value log, save and refreshed exports recorded in the
report. Unique CMake registration was appended after an immediate unleased check
and removal of this worker's queue request, without an exclusive whole-file lease.
The primary independently reviewed both complete native listings and the exact
Source/header draft `01a59b48453ea5ad3731b0387757d5b909f86c0e`, without a mismatch.
Strict Win32 build, existing checks and focused compiler inspection are pending.
No new tests are added; packet-specific native differential/game checks are unrun.
