# Native particle-model pool lifetime

Addresses: `00AF6860`, `00AF5FF0`, `00AF6940`, `00AF5D00`, `00CD7830`,
`00CE0B90`, `00CBAD6E`.

The actual static pool at `00F8D2D0` now has reconstructed construction,
empty-slab trimming, destruction and CRT startup/exit bindings. It shares the
application's `00E188B4` allocator list and real Win32 critical section. The
typed C++ interfaces borrow that storage and list; descriptive names remain
hypotheses. They do not construct the2DCh particle-model payload.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| `AF6860..AF6932` constructor | Complete211 bytes | ECX fresh38h owner; EAX same owner; RET |
| `AF5FF0..AF6079` destructor | Complete138 bytes | ECX owner; RET |
| `AF6940..AF69DF` trim | Complete160 bytes | ECX owner; current `D5DA38` virtual0; RET |
| `AF5D00..AF5D0D` table cleanup | Complete14 bytes | ECX0Ch table header; RET |
| `CD7830..CD7845` startup | Complete22 bytes | No arguments; EAX actual atexit status; RET |
| `CE0B90..CE0B99` exit thunk | Complete10 bytes | SelectF8D2D0, tail-jumpAF5FF0 |
| `CBAD6E..CBAD77` FH3 dispatcher | Defined and analyzed; no C++ entry | EAX=DF2BDC, tail-jumpBF6B43 |

The installed executable and saved `bsp.gpr` byte spans match. The report records
hashes, exact calls, previous Ghidra values and original/typed fixture boundaries.

Construction prepends the actual allocator-list prefix, publishes `D5DA38`,
initializes section0C and depth24, clears table28/count2C/capacity30, and sets
earliest34 toFFFFFFFF. It publishes capacity32 before allocating80h of pointer
cells. The subsequent copy reads the current count/backing, frees the current
old table and publishes the replacement last. Even though ordinary construction
starts empty, those access/publication rules are retained.

The FH3 descriptor `DF2BDC` uses map `DF2BC4`: state0 to-1 invokes base unlink
`CBAD50 -> 403970`; state1 to0 invokes section cleanup `CBAD58 -> 402F70`;
state2 to1 invokes table cleanup `CBAD63 -> AF5D00`. The main body advances
directly0 to2 after initializing the section and table metadata. C++ abnormal
termination follows table, section, base order. Original FH3 exception execution
and allocation failure in the constructor were not tested.

Each5C44h slab has32 slots of2E0h, with2DCh payloads, trailing slot IDs,
free-index WORDs at5C00 and free-count WORD5C40. Trimming frees only slabs whose
free count equals32, reloads table/count after free, moves the current last
pointer into the hole and decrements count. It rewrites all32 moved IDs and
retries the hole, then recomputes earliest using a captured table cursor and
current count. It adds no lock and retains capacity.

Destruction captures the initial nonempty comparison before publishing the pool
profile, frees slabs in ascending order using the live count/backing, frees the
current table, drains positive signed lock depth, deletes the section and unlinks
the same list element. It does not destroy model payloads, clear stale metadata
or free the static owner. `AF5D00` likewise preserves its table header after free.

Static binding installs the real trim callback before publishing the owner and
changes no raw bytes, list links or CRT callbacks. Startup constructs the bound
owner and registers the real source exit callback with `std::atexit`, preserving
registration status and lack of rollback. Both borrowed storage and list must
survive until process exit. `CE0B90` was misclassified as a static initializer;
the startup body proves it is the registered destructor callback.

The strict Win32 build and both existing CTests passed. The existing owner
fixture was adapted to these bytes/layouts: original constructor/destructor/
table/trim bodies match C++ with real OS sections and shared lists,65 allocations,
middle-slab compaction, all32 rewritten IDs, preserved2DCh payloads, return after
compaction, empty retries, real virtual0 trim dispatch and depth2 drain. Original
and reconstructed static callbacks passed in separate processes through real
CRT exit. Allocation in this fixture uses the worker's actual pool implementation.
No permanent tests were added. Native ABI substitution, exception dispatch,
successful particle-model construction, rendering and gameplay remain unvalidated.
