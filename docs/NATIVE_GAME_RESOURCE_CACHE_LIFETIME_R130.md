# Game resource lookup and cache cleanup

R130 implements the complete 44-byte `00B806F0` cache clear and supplies the
game destructor's resource defaults at `004DD126` / `004DD12D`. The getter
delegates the existing complete `004C1400` implementation. The new clear calls
the existing complete `00B7FF20` subtree destructor. These are source interfaces
over actual storage, not drop-in original ABI replacements.

## Native contract

`B806F0` receives the actual manager in ECX and uses plain `RET`. Capture its
head at `manager+18`, pass the captured root at `head+4` and tree at `manager+14`
to `B7FF20`, then perform these ordered writes:

1. Reload current head and set its root to itself.
2. Reload current head, zero the tree count, and set that head's left link.
3. Reload current head again and set its right link.

Retain the sentinel, debug word, manager publication, parser tree, factory and
work words. The 1Ch cache nodes own their keys but only borrow their mapped
resource pointer at `+14`; clearing the cache never releases those resources.
The existing subtree service returns keys through the current actual pool and
frees the nodes. A partial exception does not gain rollback or safe replay.

`4C1400` already supplies its complete 189-byte behavior: captured fast return;
otherwise captured lifetime-manager lock, double check, actual 28h construction,
publication, second lifetime getter, then current-publication registration.
R130 binds that implementation without replacing its construction or cleanup.

## Parent bindings

`NativeGameLifetimeContext::resources` borrows the existing resource context.
Both parent defaults also receive the game's existing profile context. Before
storage access they require that profile's call service and the same actual
singleton-manager publication, string-pool publication and small-return gate.
Separate raw context wrappers over the same cells are accepted. The resource
context's internal name/factory services remain caller-supplied real bindings.

The clear receives the getter's returned pointer. It does not require that
pointer to equal the current resource publication: the native parent carries
the captured return value directly. The parent retains its existing failure
site and no-replay operation; this packet adds no automatic partial-tree repair.

## Evidence and verification

- 315 bytes match the original PE and live Ghidra: 44 newly implemented bytes,
  82 bytes of existing subtree reference, and 189 bytes of existing getter.
  All three listings have no gaps; no flow repair was needed.
- Strict MSVC Win32 build and all three existing CTests pass.
- 24 copied-original pairs, 168 state snapshots, 220,280 normalized bytes match.
  Both actual parent defaults are exercised.
- The copied getter shares actual construction, parser-name dispatch,
  registration and locks with the source. Tests cover fully cold startup,
  an existing lifetime manager held recursively twice, and an existing pool;
  every case also checks the fast getter return.
- Real manager construction registers all six default parsers. Cache graphs
  have 0/1/3/7 nodes, null key headers and 149/150-byte owned keys. Shutdown
  small-return gate 0/1, repeated empty clear, retained sentinel, unchanged
  parser/factory/work state and borrowed resource words are checked.
- One case replaces the resource publication before clearing the captured
  manager. Its publication remains untouched until explicit fixture teardown.
- Actual singleton drain clears the resource manager, six parser publications
  and pool. The probe uses the existing controlled child mapper for verified
  original read-only data in the CE and D6 bands; no game session is launched.
- Fourteen missing/foreign-domain context guards reject before storage access.
  Distinct raw-context wrappers sharing the actual cells pass normal cases.
- The controlled parent comparison still passes 52 cases, 3,537 snapshots and
  141,402,076 bytes, plus four source failure/replay cases. It checks both new
  context arguments at the two parent sites.

The cache topology and borrowed payload storage are fixture-provided. The
comparison observes entry/final state, pool returns, parser tree and drain;
source internal node frees are not instrumented. The source getter's internal
boundaries are also not instrumented. It shares the existing constructor and
services with the copied reference, so this does not independently re-prove
every dependency. Uninitialized manager/debug/work words are omitted before
the fixture assigns them; unused node tail bytes, freed bytes and native private
stack/EH state are omitted. Known pointers are normalized. No callback mutation
of the head during subtree destruction or new allocation/partial-free failure
case is claimed; current-head reload ordering is preserved from assembly.

No new permanent test suite was added. World/physics address dependencies,
payload virtual bindings, ordinary raw-game application admission, native
exception ABI, concurrency and gameplay remain open. See the machine-readable
`reports/native_game_resource_cache_lifetime_r130.json`.
