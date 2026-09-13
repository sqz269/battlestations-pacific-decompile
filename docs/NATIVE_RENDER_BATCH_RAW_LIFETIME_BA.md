# Raw render-batch singleton lifetime binding (BA)

`NativeRenderBatchLifetime` now borrows `SoundLifetimeAccess`. Production passes
the stable actual `01090AA0` manager publication, while existing source fixtures
may continue passing `SingletonLifetimeDomain` through its implicit access
constructor. The companion still borrows the live `0108FE8C` pool and
`0109DBBC` lock-owner publications and the verified immutable D5E5AC table.
All borrowed cells and this context must survive manager drain. There is no
second manager, implicit drain, or new original-ABI entrypoint.

The full native `00B1E870..00B1E922` and `00B1CD90..00B1CE4D` getters first
read publication. On a miss they resolve the manager, capture its `+10` section,
enter it and increment `section+18`, then recheck publication. Creation writes
the native 10h or 8h owner before publishing it. A *second* manager resolution
precedes registration of the *current* publication, including a null or changed
one. The captured section is decremented and left, then publication is reloaded
for return. `CapturedSoundLifetimeSection` and `SoundLifetimeManagerView` retain
those source semantics for raw and semantic access.

`00B1E930..00B1E961` clears `0108FE8C` before destroying dead slots, then
optionally scalar-frees the owner. `00B1D530..00B1D567` sets D5E5D4, destroys
the owned section, clears `0109DBBC`, sets CE3818, then optionally frees.
Both return the original address. The final three bytes after either optional
free CALL are `ADD ESP,4`; the saved assembly exporter omitted them from its
instruction listing, but live bytes confirm their ownership.

The existing 8h lock-owner `TrackedCriticalSection*` member remains a source
ABI representation. This packet does not establish the identity or native
binary layout of every tracked-section creator/consumer. The current source is
a new C++ interface, not a drop-in binary replacement. Original game process
reachability is not claimed. The coordinated singleton-dispatch integration
binds D5E5DC/D5E5D4 deletion
profiles to this retained context. An ignored Win32 source fixture linked the
integrated core and observed two raw registrations, both registered-owner
deletions during manager drain, and both publications cleared. This does not
exercise the original game process. See
`reports/native_render_batch_raw_lifetime_ba.json` for CALL sites, ABI and
validation limits.
