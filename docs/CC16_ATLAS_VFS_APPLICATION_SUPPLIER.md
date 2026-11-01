# Actual VFS application identity supplier

GameVfsHost::native_application returns the same GameNativeVfsApplication owned by its existing unique_ptr. It creates no application, raw-string pool, manager, VFS state or provider. This ordinary Source identity accessor allows the atlas caller to pass the actual application into the private Root ownership check.

The accessor performs no readiness check, Native payload read, I/O, publication or cleanup. As with existing native_owners/raw_strings accessors, callers retain the facade and application through their complete use and shared drain. Actual atlas operations still require the concrete same graph, ready renderer, Source lifetime route and all loader/parser/caller gates. No current caller is added or activated. No Original function, vendor body, binary ABI or runtime credit follows.
