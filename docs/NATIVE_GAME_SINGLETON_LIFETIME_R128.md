# Game singleton deletion and cleanup no-op

R128 supplies four defaults consumed by `004DCF90`: the three explicit
singleton deleters `004C0C30`, `004C0CE0`, `004C0D90` and the actual one-byte
`008D88F0` no-op. The three normal deletion bodies total 488 bytes; all 489
bytes match the original PE and the live `bsp.gpr` program. No listing repair
was needed. Descriptive names are hypotheses, not recovered symbols.

## Native behavior

All four entries take no native input and use plain `RET`. The source adds
borrowed publication cells, required call services and retained diagnostics.

| Entry | Publication | Registered pointer | Scalar slot | Body |
| --- | --- | --- | --- | --- |
| `004C0C30` | `00E18E6C` | captured publication + `0C` | `0C` | 162 bytes |
| `004C0CE0` | `00E18D80` | captured publication + `08` | `0C` | 162 bytes |
| `004C0D90` | `00F89B34` | current publication | `00` | 164 bytes |
| `008D88F0` | none | none | none | one `C3` byte |

An initially null publication skips everything. Otherwise the first actual
`00415350` manager result supplies a captured raw section at `+10`. If nonnull,
enter the Win32 section and then increment its raw `+18` recursion word.
Recheck the publication. The first two routines capture and adjust the object
before the second manager lookup; the third loads it after that lookup.
Unregister through actual `00BCFCA0`, reload the current publication for the
scalar call with flags `1`, then clear the publication after the callback.
Finally decrement and leave the originally captured section. Neither a replaced
manager publication nor a replaced manager lock changes that exit target.

The actual unregister implementation removes the first matching registry slot
without compacting, removing duplicate matches, or shrinking the vector. The
three payload destructor classes remain required virtual bindings. Recovering
their callers does not establish their bodies.

## Source integration and failure boundary

`NativeGameLifetimeContext::singletons` borrows the actual manager/publication
domain used to construct the game. Its manager cell must be the same cell used
by the game's string/profile services; the caller supplies that identity. Each
default checks that the context belongs to the same call service. Parent sites
`4DD0E1`, `4DD0E6`, `4DD0F0` pass separate retained child operations; `4DCFE3`
calls the concrete no-op. Missing/foreign contexts fail before touching storage.

The child records the current native call site and captured lock. Source failure
retains partial state and rejects replay. Diagnostic retirement requires the
caller to resolve the partial object/registry graph and any retained entered
lock. It performs no cleanup itself. The parent refuses retirement while a
child remains running or failed. Original FH3 guard cleanup and SEH are not
implemented by this source interface.

## Validation

- Strict MSVC Win32 build and all three existing CTests passed.
- 49 copied-original pairs: 324 boundary observations and 94,392 matching
  normalized bytes, using the four actual parent defaults.
- Real raw manager construction/getter/registration/unregister and Win32 locks;
  separate direct checks exercise the unoverridden concrete service defaults.
- Null publication/section, manager slow creation, nested lock depth, holes and
  duplicate registrations, and publication changes at both getters, enter,
  unregister, scalar callback and leave. Manager/lock replacement checks the
  captured exit section. All three adjusted/current receiver schedules match.
- Three source scalar failures retain the publication and entered lock and
  reject replay; six null/foreign context guards pass.
- The existing controlled parent comparison still passes 52 cases, 3,537
  snapshots and 141,402,076 matching bytes, plus four failure/replay cases.
  It also checks the identity of the three child operation arguments.

The ignored probe normalizes known allocation pointers and observes lock depth
and Win32 recursion count; it omits opaque OS critical-section internals,
private native stack/EH state and allocation slack. The manager's constructor-
untouched first DWORD is initialized consistently by the fixture. Payload
virtual calls use controlled fixture bodies. The parent fixture controls its
callee boundaries; it is separate from the actual-default comparison.

No new permanent test suite was added. Native FH3/SEH, malformed memory,
concurrency, payload destruction, drop-in binary ABI, ordinary application
admission of the raw game owner and gameplay are not established. World/physics,
Lua/global-vector and resource-cache cleanup bindings remain open in the parent.
Machine-readable evidence: `reports/native_game_singleton_lifetime_r128.json`.
