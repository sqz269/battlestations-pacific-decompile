# Point-effect terminal dispatch and actual-owner binding

Packet `orch3_point_effect_release_y` reconstructs complete
`008683E0..00868415` and connects actual point-effect references to the existing
frame pool, live manager, pending deletion queue, scalar destructor and node
terminal path. `PointEffectReleaseRuntime` keeps canonical borrowed host
associations; it creates no substitute effect, reference count or native queue.
Whole `008680B0` construction remains incomplete.

Evidence comes from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed executable with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`reports/point_effect_release.json` contains the live/disk hashes, annotation
history and validation evidence. The name is descriptive, not a recovered symbol.

## Complete native dispatcher

Native ABI: ECX is the captured nullable raw effect, plain RET; no meaningful
return register is established. It does not decrement a reference count.

1. Save the incoming raw pointer before calling `004C1130` unconditionally.
2. Call the returned owner's secondary `+04` current virtual `+10`, using AL.
   Verified `CE7554[4] = 004BFAB0` tests signed secondary `active+18 > 0`.
   It does not inspect the current thread ID or test just the field's low byte.
3. Inactive: if the captured effect is nonnull, call its current virtual `+04`
   with scalar flags 1; null returns after obtaining/querying the frame owner.
4. Active: call actual live-manager getter `004D1100`, then `00868010` with
   that manager and the captured raw pointer. A null payload is still enqueued.

There is no retain, second count decrement, queue flush or local exception
cleanup. The copied 54-byte body confirms both branches without a decompiler
register-input assumption. `NativeFrameJobExecution::is_dispatching_current_10`
validates the current `D68650`/`CE7554` profile and slot before using the recovered
predicate. Unsupported profiles fail instead of assuming a successful dispatch.

## Canonical ownership and scalar composition

`NativePointEffectReference` borrows `PointEffectInstanceStorage.references_04`
without initializing or retaining it. Construction verifies current `D0D3EC`
slots `8683E0`/`867CE0` and registers one stable companion for the actual address.
`PointEffectReleaseRuntime` rejects duplicate/missing associations and implements
the existing `NativeRenderActualOwners` and `EffectDeletionDispatch` interfaces.
Its association vector contains borrowed companions, not another native effects
list or pending queue. It never owns a diagnostic reference count.

The standard intrusive zero callback validates current virtual zero and executes
the recovered dispatcher. When deferred, the same storage and companion remain
alive at count zero. Queue flush dispatches current scalar `+04` directly through
the canonical association; it performs no additional count operation or virtual
zero call. Scalar lookup is independent of the count, so a forced scalar call at
a positive count retains the original scalar semantics.

Scalar execution uses complete `867CE0`/`867680` with the supplied existing node
lifetime runtime, node terminal-owner lookup, parent projections and actual
`F87604/F87600` counters. The node path performs logical release and then the
separate current-node `+04` decrement. With `NativePlainNodeReference`, the zero
lookup reaches real `B6F440` destruction, actual string return, scene/lifetime
unbinding and `B6E490` node-pool return.

After scalar success, the point companion is removed from the lookup and retired
through a required callback. Flags bit 0 reports whether native storage was freed.
On a throwing scalar teardown, recovered member unwind has completed but the
native allocation has not been freed; only its unusable host association is
retired before propagating the exception. The retirement callback must never
free/release the effect, node or other native owners. It may delete the companion;
no access to storage or companion follows it. Flags 2 and failure leave raw
deallocation to the native caller's actual policy.

Intrusive terminal callbacks retain the repository's nonthrowing boundary.
Missing bindings or exceptions there terminate; explicit scalar and queue paths
can propagate failure. These are new C++ interfaces with native storage fields
projected through canonical companions, not executable original integer vtables
or native exception ABI.

## Shared lifetime and verification

The runtime borrows the application's actual `0109CF08`, `F8765C`, `F87654`
publication cells, frame execution/lifetime bindings and singleton domain. Its
deletion-lock adapter uses that same domain. Application shutdown must retain
the runtime and bindings while dispatching the actual frame, live-manager and
deletion-lock scalars. The frame worker-entry and concrete game-job bindings
remain required application integration points, as documented in
`NATIVE_FRAME_JOB_LIFETIME.md`.

The strict MSVC Win32 build and both existing CTests passed. One ignored focused
probe, `local/point_release_probe_y.cpp`, executed the complete original 54-byte
dispatcher and compared it with the reconstruction using canonical dependencies:

- Unconditional lazy frame creation for null input; actual suspended Win32
  worker/event ownership and clean shutdown, with a child-only processor count
  of 2.
- Immediate actual point scalar deletion and real node slot reuse after terminal
  cleanup; no instrumented substitute node destructor.
- Zero release from a real frame caller-dispatch job: storage remains live at
  count zero, the actual pending list owns the raw pointer, and later flush
  reaches the same scalar/point/node chain.
- Signed inactive `-1` and active `256`; deferred null payload is genuinely
  queued. The probe inspected and manually removed that null list entry rather
  than executing the native invalid scalar payload.
- Shared-domain live-manager shutdown flushes a pending real point owner before
  string/frame dependencies are destroyed.
- Current scalar-profile rejection before mutation, positive-count flags 2,
  and a concrete node lookup failure: effect member unwind/host retirement
  occur while native effect storage remains allocated.

The fixture rebinds the copied dispatcher's three direct calls to recovered
getter/enqueue implementations. For its two indirect calls it temporarily maps
the actual owner's table word to an executable fixture table, then restores the
original identity before canonical dispatch. Those bridges preserve actual raw
owner addresses; production code never calls the original integer identities.
The real native point scalar body itself is not copied into this probe: it uses
the separately reconstructed and validated teardown. Preinitialized type tokens,
fixture job behavior and companion retirement instrumentation are explicit.
Five actual table spans and the dispatcher were matched to live/disk bytes.

No permanent tests or production test hooks were added. This proves the focused
release/ownership composition, not whole point-effect construction, native EH,
general concurrency, binary ABI compatibility or gameplay.
