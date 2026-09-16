# Particle resource cache removal and derived lifetime

## Complete new bodies

| Address | Original span | Original ABI | Behavior |
| --- | --- | --- | --- |
| 870C10 | 870C10..870CB1, 162 bytes | ECX destination, stack source, EAX destination, RET4 | Assign the actual 2Ch name/aliases/payload record |
| 8714E0 | 8714E0..8716DE, 511 bytes | ECX cache inner, stack name header, RET4 | Find an alias and remove its record by replacing it with the current last |
| 871CA0 | 871CA0..871CFA, 91 bytes | ECX resource, RET | Stamp D0D418, remove resource name from actual cache, destroy AF4280 |
| 871FA0 | 871FA0..871FBD, 30 bytes | ECX resource, stack flags, EAX original pointer, RET4 | Derived scalar delete |

Assignment resizes/copies the current actual name, captures the source list
sentinel/first before clearing the destination list, inserts through the real
raw-pool alias providers, then copies all six tail DWORDs in order. It preserves
the destination unknown word +8 and performs no resource retain/release.
Self-assignment skips name/list work while retaining the six tail accesses.

Removal copies its input name into an actual stack header. It constructs a
second normalized header and immediately destroys that second temporary. The
comparison intentionally uses the **first, original copied name**. Thus `" FOO "`
does not match alias `"foo"`, although the discarded normalized scratch would.
This behavior follows 87151D..8715B0 directly; it is not repaired or simplified.

The search captures array end once, each list's sentinel once, and reloads the
actual sentinel for native validation. A fixed current CRT
`_invalid_parameter_noinfo` may return. The self-equality check at 871593 has no
reachable invalid-parameter edge. Equal lengths precede current-CRT `_stricmp`.
The two trace calls target the proved one-byte RET at 004254B0; argument loads
are preserved without inventing logging behavior.

On a match, current count/data determine the last record. A nonlast match is
assigned from that record; current count/data are then read again to destroy
the current last record. Current count is decremented only after destruction.
The allocation and capacity remain. No resource release occurs in removal.

## Genuine raw normalization

Existing complete 426060, 469840, BEE690 and BEE780 algorithms gain
`NativeStringRawPoolContext` overloads in their existing files. They compose the
same actual 419CC0 publication, BD1120/BD1510 pool and raw header operations.
Getter exceptions propagate instead of crossing a `NativeStringStorage`
noexcept release boundary. Native true-unwind cleanup uses noexcept RAII;
the older host interfaces retain their existing catch behavior.

The substring temporary/output flags and cleanup order come from D891D0 and
C61B50/C61B58. Output is armed only after copying completes; normal temporary
return uses its captured data and current length. BEE690 arms result cleanup
only after substring return. BEE780 clears its output flag before owning
cleanup. Native overlap-safe copying is selected only for raw overloads.

## Derived lifecycle and concrete context propagation

D0D418 is the loader-written resource profile. Its slot 0 is BD30E0 and scalar
slot +4 is 871FA0. The derived destructor obtains the actual F87668 cache via
871BD0, calls removal on cache+4 using resource+8, then invokes AF4280. State 0
and C96110 invoke AF4280 while a getter/removal exception unwinds; a second
cleanup exception terminates. The scalar's missing returning-free `ADD ESP,4`
was repaired by the parent after full live/disk agreement.

Context-aware overloads were added to cache release/clear/inner destruction and
Particle component destruction. They borrow the application's actual cache
publication, canonical manager publication and raw string-pool cells. On a
zero-reference terminal, current D0D418/D5D958 identity selects the genuine
BD30E0 path without another decrement. That path reloads the resource profile
before selecting its scalar slot +4 with flag 1; known derived/base identities
select the corresponding rebuilt body. Other profiles retain their current
callable virtual-slot boundary. No arbitrary concurrent vtable rewrite or
instruction-level equivalence is claimed by the finite identity dispatch.
Existing callable-vtable release/inner/component interfaces remain available.
Outer cache destruction already receives the actual context and now selects
the concrete path. No generic destructor callback or host owner was added.

This matters during shutdown: releasing a derived resource can remove its
record from the cache. The clear loop reloads count and skips a second record
destruction if removal emptied it. The component loop similarly observes its
current count after terminal dispatch.

## Evidence and limits

`reports/native_particle_resource_cache_removal_orch4.json` preserves fresh
complete live/disk spans, hashes, direct/indirect call rows, prior annotations,
source/object pins and final validation results. The focused source probe
composes actual pooled strings/aliases, nonlast assignment, the discarded
normalization behavior, known derived-profile scalar deletion, Particle
component release and cache shutdown. The separate original AF4280 comparison
is described in `NATIVE_PARTICLE_RESOURCE_LIFETIME_ORCH4.md`.

Full cache loading/parsing, arbitrary native child profile bindings, original
FH3/SEH frames, arbitrary OOM/fault injection and game validation are outside
these claims. Existing source CRT locale, allocation and exception domains
apply. Descriptive names are hypotheses, and correct library/scalar names are
preserved.

Final strict Win32 Release build and all three existing CTests passed, as did
the bounded lifetime/cache probe against the resulting archive. The source
fixture uses an already-published actual cache owner; it does not re-test the
cache getter's independently reconstructed lazy-allocation path.
