# Canonical hierarchy pool in application startup

## Result

`GameNativeResourcePoolProcess` owns the application's distinct `38h` hierarchy
pool storage corresponding to `0109022C`. Its `NativeMaterialParameterPool`
companion borrows the same `E188B4` allocator-list domain as the existing physical
and particle process pools. This does not create the separate `F8D3E4` material
parameter pool or introduce another allocator list.

After verified native-data bootstrap, `game_main` calls the reconstructed
`CD82D0` initializer after the selected `CD7830` and `CD78B0` particle
initializers. Original initializer-table entries at `CE34EC`, `CE34F4` and
`CE35B0` establish that relative order. All twelve table bytes and both complete
hierarchy initializer/shutdown bodies match the installed PE and live Ghidra.
This establishes the order of these selected initializers, not reconstruction
of the entire original CRT initializer table.

The existing `CD82D0` body constructs through `B18340`, then registers the real
`CE0ED0` callback with `atexit`. That callback selects the same companion and
destroys it through `B18470`. A once guard prevents repeated initialization and
returns the original registration status. A thrown attempt cannot be retried;
a nonzero registration status does not invent rollback or substitute cleanup.
The accessor rejects use before startup returns.

The shared allocator owner and process bookkeeping are fully constructed before
the native callback is registered. The callback therefore runs while both are
alive. C++ metadata destruction performs no second native pool destruction.
Clients must destroy their payloads before CRT exit. The same companion is
available for resource-container operations through `hierarchy_pool_0109022c`.

## Validation

The strict MSVC Win32 build and all three existing CTests pass. A focused ignored
probe links the built library and uses the actual four process pool owners. It
checks distinct raw storage in one doubly linked allocator list, access before
startup, repeated-start identity, real allocation and return, and shared trim
through all four reconstructed virtual-zero bodies.

The probe then leaves one raw slab in each pool and returns normally. Observer
callbacks registered between the real native registrations verify CRT exit
order: physical, hierarchy, particle parameter, then particle model. Each
native callback removes only its owner, stamps the base profile, and leaves
later owners alive. The final observer sees an empty shared list while process
bookkeeping still exists. No replacement exit registry or explicit manual pool
destruction is used. Both the probe body and real process exit succeed.

The slots in this probe contain no constructed payload objects. This verifies
pool ownership and source callback lifetime; it does not validate destruction
of loaded resource payloads. No permanent tests were added.

## Remaining boundaries

The complete resource-load/cache/parse dispatch graph is not installed by this
change. The application startup call was compiled; the focused probe directly
exercised the same source process owners without launching a game window.
Original image execution, original CRT exception identity, native register ABI,
allocation or callback-registration failure execution, concurrent startup and
gameplay remain unvalidated. See
`reports/native_resource_pool_application_orch4.json` for byte, call and artifact
receipts.
