# Concrete MPKG runtime dispatch

Addresses: existing call boundaries `00BB99A0`, `00BB9AAE`, `00BDB078`,
`00BE1FFD`, `00BD30EB`; no additional native routine is counted by this binding.

`NativeMpkgArchiveRuntimeOperations` calls the complete raw34h archive constructor
and destructor. It supplies the provider's previously explicit archive boundary
without invoking the semantic archive parser. `NativeMpkgRuntimeServices`
borrows the existing VFS runtime, actual retained-memory counters and canonical
pool services for captured manager/stream methods and explicit cleanup getters.

The VFS runtime gains captured-entry manager-open and stream-length entry points.
They consume the original target without another owner-table read. Existing
table-based callers still validate their supported profiles before forwarding.
The archive's converted stream deletion accepts BB8F90 and passes the captured
owner and flags to the complete actual memory-stream destructor.

Factory creation accepts BB9D90 through a borrowed MPKG provider context. The
same context supplies provider deletion BB9EE0 during manager destruction and
the separate BD30E0 current-slot deletion path. D64390 MPKG providers and D15AD8
memory backings are valid reference-owner identities; backing deletion uses the
existing complete 8D4470 implementation. The caller still performs every
reference decrement.

`bind_mpkg_provider` explicitly connects the borrowed provider context after the
manager/archive service graph has been constructed. It returns the prior binding
so the caller can restore it before the context expires. It creates no owner,
pool, publication, implicit factory or reference count. Missing contexts and
unsupported numeric targets remain explicit source errors.

The MPKG entry-open/compression route is still separate work. Successful archive
construction and mount registration do not prove entry-open runtime dispatch or
a runnable application. The planned native/source archive fixture and source-only
binding checks will be recorded in `reports/native_ax_integration.json`; they do
not establish original FH3/SEH compatibility or gameplay.

Follow-up packets: recover the remaining actual MPKG entry-open/stream route,
then the MPAK factory/provider path used by installed archives, with their real
container and stream dependencies. Recheck current leases and packet evidence
before assigning those addresses.
