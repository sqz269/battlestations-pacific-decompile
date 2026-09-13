# Native MPAK storage services (BB)

`NativeMpakStorageServices` retains one `NativeMpakLookupStorage`, one
`NativeMpakOffsetCopyStorage`, and one `NativeMpakContainerStorage`. The
container's offset-copy reference names the retained offset service; lookup
supplies both the file-subscript and directory-search interfaces. No service
constructs a provider tree, manager, pool, stream or allocation domain.

The bundle borrows the actual string pool, the existing VFS runtime bindings,
the path/scratch allocator service, and the returning invalid-parameter
callback. `runtime_inputs()` fills exactly the storage interfaces in
`NativeMpakRuntimeInputs` and forwards the supplied non-storage owners. Construct
the bundle after these borrowed services, construct `NativeMpakRuntime` with
`bundle.streams()` and the returned inputs, then drain providers and destroy
the runtime before the bundle or any borrowed owner. The references in a
returned input value remain tied to the bundle and its borrowed services.

This is a source composition contract. Native body evidence and bounded
coverage remain in `NATIVE_MPAK_LOOKUP_STORAGE_AZ.md`,
`NATIVE_MPAK_OFFSET_COPY_STORAGE_AZ.md`, and
`NATIVE_MPAK_CONTAINER_STORAGE_AZ.md`. It does not prove a raw VFS owner exists
at a game startup path or that the original program invoked this composition.
