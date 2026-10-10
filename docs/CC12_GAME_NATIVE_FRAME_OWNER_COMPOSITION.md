# Game frame owner composition: source readiness

This packet does not admit dormant game construction or startup attachment. The
captured source has useful owners and concrete cleanup bodies, but it does not
yet supply the complete, consistent ownership graph required by the actual
`71A0h` game. In particular, genuine Application `+14` and duplicate-name
publication owners remain unestablished, and the grid path has two allocation
domain mismatches plus an unbound callable-table contract. No C++ changes are
proposed as ready in this packet.

The positive correction to a broad “current game owner missing” interpretation
is that `GameNativeSettingsProcess` already owns a retained `void* volatile game`
cell. Its `game_00e188a8()` accessor returns that cell, and its profile context
borrows the same reference. A future composition must borrow this cell for
construction, lifetime and current-game consumers. A new private pointer would
create a second publication authority.

## Evidence and scope

Source is frozen at worker `62ce95b08110543322924dd7451df0d60be670ec`, with
Main `7edffef9b2b2293278377ad77b75c8afa3bb17c6` at capture. All 4,208 tracked
`src/` and `include/` files have complete Git-blob and working-byte indexes and
archives. The retained whole dependencies include the application-queue owner
review, allocation-domain review, free-default implementation and primary
review, the primary grid text qualification, and the node-helper implementation
and review. Their old proof references are provenance; this packet does not
recursively replay those earlier proofs.

The older Source3352-wide epoch, worker Source3352 free-default epoch, primary
Source3354 node epoch and primary Source3354 free-default epoch are retained as
four separate manifests and complete archives. Each includes its own four
historical build artifacts. None is represented as a new build or unchanged
artifact result for this packet. The latest primary text qualification is
preserved alongside the original accepted primary document and report.

The report records complete lexical searches over the frozen 4,208-file corpus,
bounded source excerpts, and a replayable source audit. Search counts include
comments and declarations; they do not establish a call graph, production
reachability, or absence of a provider expressed under another name. There are
zero fresh Native queries, Ghidra writes, builds, tests, original executions,
OS probes, or new runtime/ABI claims.

## Borrowing and construction contracts

| Required identity | Existing source provider | Readiness and remaining obligation |
| --- | --- | --- |
| Current game `00E188A8` | `GameNativeSettingsProcess::Impl::game`, exposed by `game_00e188a8()`; the process profile context borrows it | Concrete cell exists. Borrow the exact reference in construction, lifetime, profile and consumers after settings initialization. Do not duplicate it. |
| Application `+14` | `GameNativeGameApplicationFrame` accepts a reference to the actual cell | No genuine raw application owner is established in the reviewed startup. `ApplicationFrameState` and `GameStateSlot` are semantic projections. The frame accepts a caller reference; that does not prove its authority. |
| Duplicated name `00E1AE78` | The frame accepts `char* volatile&` and a raw string-pool context | No retained production cell owner is established. `BootstrapHost::command_line()` returning `std::string` is insufficient; neither a local pointer nor `c_str()` establishes the required ownership. |
| Actual game allocation | `GameNativeGameApplicationFrame` uses the canonical allocator for `NativeGameStorage` | Concrete `71A0h` storage and a one-shot frame exist. The frame zeroes its allocation; this is an explicit source preimage, not proof that every byte is initialized by the original constructor. |
| Strings and manager | `GameNativeStringProcess`, `GameSingletonHost`, VFS raw services | Borrow the same `01090AA0` manager, `01090AA8` pool and `01090AA4` return flag through all raw and storage adapters. The process retains these owners beyond application callbacks. |
| Dynamics engine/world | `GameNativeDynProcess`, initialized by existing application startup | Concrete retained cells and source dispatch tables exist. Construction, simulation and physics cleanup must use this exact process and its allocation callbacks. Dynamics memory defaults remain a distinct domain from canonical game storage. |
| Player profile | `GameNativePlayerProfileContext` | Concrete constructor context borrows VFS strings, the actual settings/profile cells, and mapped constants. It does not supply the parent lifetime call service; the cleanup context must use the same raw string cells and parent calls. |
| Game tables | `GameNativeGameTables` | Concrete unit/rank storage exists and construction borrowing is one-shot. An explicit opaque unit-frame-word preimage is still required. No assembled game owner using this provider is established. |
| Resource parsers/manager | `GameNativeResourceApplication`, exposed through `GameVfsHost` | Real parser publications and manager context exist. Borrow after the host's readiness gate, with its canonical manager and strings; retain them through all interrupted frames and cleanup. |
| Renderer/grid graphics | `GameNativeRendererApplication::game_grid_context()` and `GameGridGraph` | Concrete graphics/string domain checks exist. They do not settle the CPU allocation pairs, outer-grid freeing, callable grid table, or the three descriptor preimages. |
| Global configuration load | `GlobalConfigContext` and `NativeGlobalConfigLoadContext` contract types; Lua/VFS/settings services provide some inputs | No complete current game composition is established. Required inputs include the actual global-config singleton/manager/effect dispatch, same strings, actual Lua bootstrap and file services, actual sound-cache/effect lifetime, and the live FOV divisor from settings. Bodies existing separately do not supply this assembled owner. |
| Remaining lifetime publications | Parent and child cleanup contexts | Actual movie, miscellaneous singleton, class-header, Lua-header and embedded-flag owners are not established for this game composition. Do not supply local zero cells, dummy headers or fabricated tables to make construction callable. |

The construction context takes references to four publication cells (game and
three grids), a movie cell, strings, profile, call service, embedded/array
constants, dynamics, tables, global-config contexts, resource parsers, optional
grid context, and three explicit grid descriptor preimages. Those requirements
are simultaneous. `GameNativeGameRuntime` owns operation frames and a callable
primary game table; copying contexts retains references, not the underlying
owners. Its existing checks compare game/grid cell identities, the canonical
dynamics process, and physics-call identity. They are not comprehensive
validation of the movie, name, application cell, every child service, allocation
family or preimage.

## Grid boundary that blocks composition

The accepted parent/container free correction is present: both relevant
`free_00bf65ac` defaults now call `singleton_lifetime_free`. This closes those
two source defaults for canonical storage. It does not change the independent
`NativeGameGridCpuCalls` service or prove foreign-domain interchangeability.

The captured paths are:

1. Game construction allocates each of the three outer `84h` grid objects via
   the canonical `allocate_00bf681b` path. Grid scalar deletion reaches
   `NativeGameGridCpuCalls::free_00bf65ac`, whose current default uses
   `::operator delete`.
2. Grid initialization allocates six internal CPU buffers through
   `NativeGameGridCpuCalls::allocate_00bf55be`, whose current default uses
   `::operator new`. Their cleanup loop calls `free_00bf6989`, whose default
   uses `std::free`.
3. `GameGridGraph` owns the unmodified default CPU service. No derived allocator
   override in that graph closes either pair.
4. Grid construction and destruction stamp numeric `00CFD484`. The game
   lifetime's concrete current-virtual dispatcher needs a genuine current
   source method table and receiver. Numeric original-image identities do not
   satisfy that contract. The runtime's primary game table does not supply a
   grid table.

Both source allocation/free pairs therefore remain unproven for composition.
This is a source contract finding, not a measured heap failure or evidence about
original CRT interchangeability. No grid method, table or allocation path is
changed here.

## Queue, state and consumer identity

The target is the actual game: state at `G+5D4`, the native 20-byte queue header
at `G+5D8`, and hold byte at `G+5EC`. The header consists of the opaque first
word and map/capacity/head/count words at `+4/+8/+C/+10`. Game construction
zeros the latter four fields and hold byte; it does not establish a semantic
STL object at that address. The frame's full zeroed allocation supplies its own
initial opaque-word preimage.

The container cleanup reduces count, resets head when count reaches zero,
frees the block map entries in reverse order, frees the map, and clears
map/capacity. The opaque word is preserved. Through the corrected parent
service, canonical raw map/block buffers can use the established free route;
a future producer must prove its corresponding allocation, growth and alias
contracts before using it. The existing `GameStateRequestQueue` is a vector of
owned blocks with semantic `size_t` state. Reinterpreting it as this 20-byte
header is not valid. The menu host currently owns that separate semantic queue,
and application frame state reads a separate `GameStateSlot`.

`GameFixedStepHost::attach_native_game` exists, but the complete captured
lexical search has only its declaration and definition. Mission-frame
construction creates the fixed-step host and attaches other semantic services;
it does not attach a game runtime. `game_on_move()` records unimplemented native
simulation when its runtime pointer is null. If attached, it calls the retained
runtime, whose simulation requires a live phase and equality between the
current-game cell and its owned game storage before reading the actual world at
`G+18`.

The smallest eventual attachment must therefore borrow that same completed
runtime into the actual mission-frame fixed-step instance. Every created
instance needs an explicit lifetime contract. A standalone attachment call,
private current-game pointer or semantic state adapter would not close the
ownership graph.

## Publication, failure and teardown order

The raw constructor publishes the current game before its later worker/world
steps. Runtime phase becomes live only after construction returns. The frame
then publishes Application `+14` and returns the temporary name through its
pool; its own phase becomes live only after this completes. Thus non-null
publication alone is insufficient for attachment. Required gates are completed
frame and runtime phases, resolved name temporaries, exact current-game/storage
identity, and completed same-domain child services.

Failure can retain a partially constructed/published game or an application
publication with unresolved name cleanup. The source operations deliberately
retain diagnostic state and do not implement original SEH rollback. Diagnostic
acknowledgement is not a substitute for cleanup. All borrowed services and
publication owners must outlive that retained state.

Before retirement, quiesce queue producers and every current-game consumer,
then detach the actual fixed-step instance with `attach_native_game(nullptr)`
while the runtime is still valid. The frame class does not clear Application
`+14`; an eventual genuine application owner must establish that obligation
and schedule from evidence. Do not invent a store or infer it from the parent
destructor's separate current-game clearing.

Parent destruction clears the current-game publication before completing grid
retirement and later queue cleanup. Child contexts must retain the same parent
call service and underlying owners: arrays need canonical strings and the
existing observer lifetime; profile needs the same raw pool; singleton cleanup
needs genuine publication cells and manager; class cleanup needs the actual
headers/registry; Lua cleanup needs the two actual `10h` headers; physics needs
the same dynamics cells, memory callbacks and valid body context when reached.
Nullable context pointers are fail-on-use requirements, not permission to omit
reached cleanup.

Game/grid/profile/resource cleanup and all retained diagnostics must finish
before renderer/VFS/Lua/observer/string/manager owners are drained. Existing
startup teardown deletes semantic menu state and drains shared services; it
does not establish this raw-game retirement schedule. This packet leaves those
startup semantics unchanged.

## Bounded next packet

The next useful packet is a source-only grid provider allocation and table
readiness review. Bound it to `native_game_grid.cpp/.hpp`,
`game_native_renderer_grid.inc`, the three outer allocations in
`native_game_construction.cpp`, and the current-dispatch contract. Trace each
outer/internal allocation to the exact reached free service, enumerate actual
callers and alternate domains, and identify the smallest matching service pair
and callable grid-table ownership contract. Any C++ implementation needs a
separate explicit admission based on those results; this report does not admit
changing all grid frees or installing a speculative table.

After that independent boundary closes, identify genuine Application `+14`,
duplicate-name and remaining publication/header owners and their construction
and teardown placement. Only a fully proven provider graph can justify a
dormant game-owner implementation. Native queue producer/consumer wiring and
production activation remain later, separate contracts.

The machine-readable report and finite retained bundle are
`reports/cc12_game_native_frame_owner_composition.json` and
`local/cc12_game_native_frame_owner_composition/`. Replay instructions and
strict source-only results are recorded in the report.
