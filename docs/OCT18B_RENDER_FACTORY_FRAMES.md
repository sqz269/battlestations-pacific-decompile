# OCT18B procedural factory scratch frames

`GameNativeMissionFactoryFrames` binds the existing publication, construction,
registry registration and retirement interfaces to stable typed scratch. It
does not initialize payload words, copy unknown input values, construct a fake
map, register a substitute factory or change native counts. Its constructors
only create immutable reference views. Payload storage remains untouched until
the corresponding genuine native Source producer writes it. Use the new
borrowed-frames application composition; the older copy-input constructor would
read untouched fields and is outside this helper's contract.

The qualified insertion domain is deliberately narrow: the actual factory
registry is absent or empty before setup, the exact four names are inserted in
BBC900 order, and the map remains externally quiescent:
`CausticsTextureSource < ShoreWaveTextureSource0 < ShoreWaveTextureSource1 < ShoreWaveTextureSource2`.
The native lexical comparator orders these distinct nonempty ASCII keys in
that order. Each new lower_bound is the end iterator. First insertion takes
B1B0BB's empty-tree direct link; later insertions take B1B142..B1B175's end-hint,
maximum-less-than-new-key direct right link. B1AF90 fallback is never reached.
Its direction cell receives only a byte before a full-DWORD push on one path;
the unresolved upper three preimage bytes are preserved and **not consumed**.
This helper cannot be reused for an arbitrary populated tree or key order.

| Reached payload | First producer before read |
| --- | --- |
| Registration name/factory | BBC5F0/BBC740 push sites seed both current arguments |
| Registration result iterator | B1B3B4 calls B19D60 before B1B3C2 reads its result |
| Value name | B1B3DE passes the captured registration name |
| Temporary pair length/data/value | B1B2B0 constructs zero header, resizes/copies, then stores value0 before insertion/cleanup |
| Hint pair/owner/node/output | B1B2B0 pushes actual temporary/result pointers and lower_bound iterator before B1B346 |
| Direct link pair/parent/side/output | Empty-hint and end-hint native push sites write all four words, using full side1 or0 |
| Node incoming five arguments | B1AE1E..B1AE24 pushes full color0, pair, head, parent, head |
| Allocation/placement cleanup | B1AD3D/B1AD47 store returned allocation before states0/1 can consume the cells |
| Node constructor five arguments | B1AD52..B1AD68 reads those initialized incoming arguments and pushes the constructor arguments |
| Value result iterator | Direct B1ADA0 writes node then tree before the caller reads them |
| Retirement cleanup self | BBC520 saves captured table before any armed cleanup reads self |
| Retirement normal/cleanup flags | BBC4E0/BBC500 capture owner/profile/target, then push full flags1 before scalar reads flags |

The hint's local12B result/padding, fallback direction/cursor/link backing and
link's length-error SBO backing remain live but untouched and unread on this
domain. Counts0..3 exclude the length-error path. Real node allocation failure
does not arm a node cleanup until the allocation returns. A constructor escape
can consume its cleanup words only after both have been stored. Existing
native Source exceptional behavior is reused, without extra rollback or a
claim of safe disposition after hardware/foreign faults.

Retirement uses existing BBC5D0/BBC520 and slot frames. BBC890/BBC8E0 delegate
to the existing scalar base/free routines; they require no registry erasure
frame. Registry nodes borrow the factory bits. All lookups must be quiescent
before retirement, and the borrowed frames/providers remain alive through the
later registry drain. This helper itself releases nothing.

Original parent order is verified by a live xref: Application Initialize calls
BBCB40 at0073D8CC, before73BF80 at0073DD12. The previous737E20 call can clobber
ECX, so no constant Original incoming register value is inferred. The outer
composition supplies a known Source caller identity. Its saved BBCB40 word is
dead before allocation returns and overwritten before armed cleanup can read
it; outer frame derivation is owned by the model worker.

Analysis used the configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the CLI before live batches. Stored
Original assembly and live B1B132, B1AD33 and B1B3A0 listings confirm the
first-store ordering above. Existing complete bodies and their actual argument
identities are reused. The helper creates no Original ABI wrapper, stack
location/alias preimage, native saved-register parity, private FH3/SEH or new
native-body credit. Source concurrent mutation and arbitrary insertion paths
remain excluded. `git diff --check` passed. Primary owns build and runtime
verification; this worker adds no tests and performs no build or game run.
