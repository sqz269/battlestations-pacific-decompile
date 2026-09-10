# Camera type bootstrap and leaves

The five reconstructed entries bind the existing camera descriptor and reuse the
same root/node bootstrap and type counter already used by the light family.
The guard is the actual byte at `0108FF9C`; the descriptor contains camera, node,
and root tokens at `0108FFA0/A4/A8`, followed by native name word `0108FFAC`.
The name value is `00D62CE4` (`cCamera`). It preserves native address identity and
must not be dereferenced as a host string.

`CameraTypeBootstrap` takes the **same `TypeIdCounterLifetime` object** used by
the supplied `LightTypeBootstrap`, including its actual `0109DB7C` global slot
and its shared `SingletonLifetimeDomain`. This is a construction precondition:
the existing light API keeps that reference private, so the camera wrapper cannot
check its identity without changing the shared API. No new counter, singleton
domain, root/node descriptor, or root/node guard is created. Node storage comes
directly from `LightTypeBootstrap::storage().node_0108ff90`, and initialization
calls its existing `initialize_node_00b6f110` with that actual descriptor.
The new POD/storage bindings do not reset process data.
The C++ and proposed Ghidra names are descriptive interpretations, not recovered
original symbols.

| Original entry | Original ABI and behavior | C++ method |
| --- | --- | --- |
| `00B6FB60` | Incoming ECX owner ignored; EAX is live `[0108FFA0]`; `RET 0` | `type_id_00b6fb60` |
| `00B71CE0` | Incoming ECX owner ignored; token from `[ESP+4]`; AL boolean; `RET 4` | `is_type_00b71ce0` |
| `00CD7D80` | Static entry, no arguments; no stable result; `RET 0` | `initialize_static_00cd7d80` |
| `00B719E0` | ECX target descriptor; no stable result; `RET 0` | `initialize_00b719e0` |
| `00B71A30` | ECX target descriptor; EAX same target on both paths; `RET 0` | `construct_00b71a30` |

The getter and predicate never initialize anything or call the counter getter.
The predicate compares the three current global token words in ascending address
order and short-circuits on the first match. The name word is excluded. In a
zero-filled descriptor, token zero therefore matches even with a clear guard.
Only AL is a defined native boolean result; the new typed C++ methods are not
native ABI replacements.

Every initializer reads the one process guard and does nothing if it is any
nonzero byte. On a cold path it sets the guard to one before storing the name
and before calling the node initializer. Static `00CD7D80` captures both source
ancestry words from `0108FF90/94` before storing either destination. Lazy
`00B719E0` and `00B71A30` instead read node-own, store target `+4`, read node-root,
and store target `+8`. They then get the same actual counter, capture its `+4`,
increment that word with unsigned 32-bit wraparound, and write the captured ID
to target `+0`. These differences in memory-access order remain explicit.

The lazy target is the incoming ECX descriptor, while its guard is still global.
Initializing an alternate target can consequently leave `0108FFA0` untouched
while preventing subsequent initialization of the global descriptor or another
target. No rollback or guard reset is added after a failing dependency call.
Neither the guard nor counter increment gains an additional lock or atomic
operation. Process startup ordering and concurrent initialization remain outside
this bounded reconstruction.

## Evidence and validation

Current bounded `bsp.py ghidra bytes` reads each verified
`C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before comparison with
the installed PE. Ten code spans and thirteen data spans match the installed
binary or its PE zero-fill. The five camera entry extents were inspected through
their final `RET`; none had its own function definition in the initial index.
The audit records complete address extents, per-span SHA-256, original ABI,
dependency ownership, native relocation records, and local artifact hashes.
Ghidra definitions, naming, ledgers, export refresh, and CMake integration belong
to the primary integration pass.

One local MSVC Win32 fixture executes the original installed instructions and
the reconstructed methods through the same focused sequence: nine matching
checkpoints, forty-seven words per checkpoint, and 125 predicate calls per side.
It checks cold static root/node/camera bootstrap, directional light consuming
the same counter, a noncanonical nonzero guard, unchanged alternate targets,
mutation of actual node/camera tokens, alternate-target lazy initialization,
global-guard suppression, and the lazy constructor's return pointer. To exercise
each cold entry within that single sequence, the fixture explicitly clears the
camera guard twice; these are test probes, not reconstructed runtime behavior.
Native name words are normalized by the image relocation delta for comparison.
No native bootstrap/helper call is replaced by a simulated implementation.

The original counter is prepublished for the native fast getter path. The host
uses the existing real lifetime manager and verifies that the counter is its
single registered object before shared shutdown. Native slow counter allocation,
registration, shutdown, EH/unwind, concurrent races, and wraparound were not
executed by this fixture. Alternate targets were separate storage; overlapping
target/source aliases were inspected by instruction order only.

The new source, existing light bootstrap, singleton lifetime, and focused fixture
pass MSVC Win32 `/std:c++20 /W4 /WX /fp:strict`. The existing repository build and
its existing tests also pass. This establishes reconstruction, strict compilation,
and focused native fixture agreement. Camera construction/composition, game
startup ordering, binary replacement compatibility, and gameplay validation are
not established by this packet.
