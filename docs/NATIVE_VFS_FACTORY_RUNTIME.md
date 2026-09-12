# Concrete factory and provider lifetime dispatch

Addresses: existing call boundaries `00BDB078`, `00BE1FA1`, `00BE1FFD`,
`00BD30EB`; no additional native routine is counted by this binding packet.

`NativeVfsRuntimeBindings` now implements the captured factory Create dispatch and
the manager's existing log/provider deletion interface. It also accepts the actual
D68D04 derived manager for the already reconstructed open and existence routes;
current D68D08/D68D0C bytes equal BDF310/BDD440, matching the base profile's slots.

| Captured target | Source action | Required borrowed context |
|---|---|---|
| BE8120 | FileStore factory Create / lazy cached actual2Ch provider | existing actual string storage, stream dispatcher and validation callbacks |
| BF4DF0 | physical factory Create / actual3Ch provider pool slot | `NativePhysicalProviderContext` |
| BE8090 | FileStore scalar deleting destructor | same FileStore lifetime context |
| BF4DD0 | physical scalar deleting destructor | same physical provider context |
| 7376A0 / 737CC0 | base / derived file-access-log scalar deleting destructor | `NativeFileAccessLogLifetimeBindings` |

The optional constructor parameters borrow physical-provider and file-log contexts.
Existing six-argument callers retain their existing behavior. Invoking a physical
or logger path without its required context is an explicit source error. The
binding creates no private provider pool, publication cell, cached lifetime
manager or placeholder callback. Factory and deletion methods consume the exact
captured target and owner without a second vtable read.

BD30E0 is different: its native zero-reference invoker accepts null first, reloads
the owner's current slot4, and supplies flags1. Its finite source dispatch now
accepts the actual FileStore and physical provider identities in addition to the
existing stream identities. It performs **no decrement**. The caller's reference
operation and this terminal invocation remain distinct. BE1FFD already captured
the deleting entry; it does not reuse a helper that reads the table again.

Manager destruction directly invokes each mount's provider deletion with flags1;
the mount byte is not a deletion gate. Repeated mounts of a single FileStore
cached provider are therefore not made safe by a new refcount policy. Factory
cache clearing and provider rollback are likewise not invented by the binding.

The canonicalizer runtime service now has an overload borrowing the actual raw
01090AA0 publication, consistent with `ActualNativeStringPoolStorage`. Every
getter still invokes the existing complete 419CC0 path; no parallel semantic
`SingletonLifetimeDomain` is allocated for the application's strings. The original
semantic-domain overload remains available for its existing callers.

`reports/native_vfs_factory_runtime.json` identifies the verified table spans and
call-site contracts. Integrated source and native fixture evidence is recorded in
`reports/native_aw_integration.json`. The source interfaces and explicit callback
services do not establish original binary/FH3/SEH compatibility or game startup.

Follow-up packets: add the remaining verified factory and provider targets as
their complete actual-storage bodies become available; recover the failure
callback's application setup before treating the full mount sequence as runnable.
