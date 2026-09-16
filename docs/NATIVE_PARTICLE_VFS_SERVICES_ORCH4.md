# Borrowing the application VFS services

`GameNativeVfsRuntime::borrow_raw_services()` exposes a reference-only view of
its existing live VFS publication, concrete runtime bindings and name-resolution
context. These are the same objects used by the runtime's existing operations;
the view creates no second manager, publication cell, stream or provider.

The raw particle text loader AF5850 already accepts these three services and a
caller-owned actual TextBuffer, resolution frame and raw string-pool context.
Its native one-read behavior and failure ownership differ from `read_all()`.
The borrowed view lets the new resource-loader composition call AF5850 directly.
It does not use `read_all()` or change either operation's behavior.

The consumer must supply the same actual string-pool, return-gate and singleton
manager cells used by the VFS owner services. The returned publication is a live
reference. Constructing the view is allowed before core registration; execution
still requires the native caller's initialized domains. Retain the runtime and
all its borrowed inputs through consumers and the shared singleton drain.
Existing failed resolution frames still require their native headers, contexts
and runtime to survive through process exit; no discharge or retry is added.

This is application source wiring, not a new native reconstruction or original
ABI entry. Strict Win32 compilation and existing checks validate the API. Actual
particle loader installation, asset loading and gameplay require the remaining
resource parser, loader and lifetime composition.
