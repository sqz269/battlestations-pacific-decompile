# CC12 page-load ownership transport 63

This is an opt-in Source lifetime bridge for a future retained GUI host. It does not activate native Text, supply a concrete native host, recover another Original body, or solve final graph retirement. Root baseline `7e8390fecd4bfe6c46e0a9145517263d5a27d738` was merged successfully into worker baseline `75c735ef190a2b5150a14b6e1b05bb33c32d8a7d` before editing.

The existing loader's local page and builder-local child can unwind before its no-argument failure hook runs. That hook cannot retain those same owners. The new `load_gui_page_retained_00aa5840` instead requires a persistent `GuiRetainedPageLoadFailureSink` and transports the exact operation into it before rethrowing. The old loader, builder, registry insertion, and callers keep their existing method bodies.

## Ownership and order

The new load API is declared in `include/bsp/gui_layout_loader.hpp:482`; the added implementation starts at `src/gui_layout_loader.cpp:796`. Access and graph mutation are externally serialized; serialized callback nesting is allowed.

1. A latched sink rejects a future load before allocation or host effects. Otherwise the Source operation and copied name are allocated before even the existing-page lookup. This differs from the Original allocation/exception profile. An existing page still receives the requested reference increment and returns without host entry.
2. For a new page, the operation owns the page, root, name, and persistent model-path argument before the first VFS/model/root callback. It marks `host_entered` and phase `entered` immediately before that callback boundary. This is a conservative Source effect boundary, not a recovered native exception frame.
3. The exact shared script result is stored in the page immediately after evaluation returns. The whole table owner keeps its actual nested tables alive. Every empty property table is a real owned `GuiTable`, stored before its first callback; there is no shared-pointer alias over a unique page/widget.
4. Before each child's first callback, both parent child vectors have spare capacity and the operation owns `pending_child`. A callback that installs a destruction hook or stores a node and then throws therefore leaves that same child owned. On success, nonthrowing vector operations transfer it into the owned tree before parenting/property callbacks continue.
5. Root and child property/virtual-hook order follows the existing builder. The new path does not introduce a clip-provider, count gate, or cleanup continuation. Whole-table and empty-table anchors are members of the successful page, declared before its root so they remain alive throughout root/child destruction callbacks. Legacy loading leaves this new anchor vector empty.
6. `register_retained_00aa52a0(std::unique_ptr<GuiLayoutPage>&)` finds the stable priority position and reserves registry capacity before moving the sole page owner. Equal priorities retain insertion order. Preflight/reserve failure leaves the owner in the operation; insertion shifts only `unique_ptr`s without another allocation or host callback. Only successful insertion sets phase `complete`.

## Failure contract

Any exception from `run()` stores the exception and phase `failed`, then supplies the same unique operation to the required `noexcept` sink before propagating the original exception. Constructor/name-allocation exceptions occur before an operation exists and propagate directly. A failed operation that never entered a host callback does not set the interruption latch or require process retention; the sink can inspect that distinction.

An entered failure sets the sink's persistent latch before its callback. The sink must retain **every** supplied entered-failed operation and the host/registry/VFS/renderer/factory and other borrowed runtime dependencies through process exit. Already-entered nested loads can each unwind into the same sink after the latch has been set. The latch rejects later entries; it does not ban callback nesting that was already admitted. There is no default no-op sink, cancel/reset, replay, graph-release API, or guessed cleanup. Discarding an entered incomplete operation calls `std::terminate()` before its graph members are destroyed.

The observation API exposes only phase, entry flag, owner identities, table-owner count, and the stored exception. It cannot clear the latch or release a failed graph. A future concrete host must check the sink before releasing borrowed dependencies and choose its actual process-exit containment path. This patch does not wire such a host or claim that a failed graph can be retired normally.

On success, the registry owns the page and its real table anchors; the temporary operation becomes complete and can be destroyed normally. Its caller still must establish real consumer quiescence and the correct native teardown order before deleting that page. This does not add a page/widget reference count, infer ownership from a native count, fix stale clip sources, or close final graph retirement. The added Source member changes `GuiLayoutPage`'s C++ layout; neither this type nor the new API is an Original binary ABI replacement.

## Verification and limits

The complete legacy `.cpp` text remains byte-identical after removing the newly added `<stdexcept>` include and appended opt-in implementation. Its normalized SHA-256 is `e0ca3e5052e26d80ca4b4d2c3c64e78b6c54f351bed0fe9474e08d10ba85fc56`. This checks all legacy bodies, not just selected functions. No legacy caller, CMake file, native owner, clip/count logic, Ghidra project, or ledger was changed.

The canonical Win32 build and existing three CTests are recorded in the accompanying JSON receipt. One approved `/MD` Win32 probe links the actual production `bsp_core` implementation. Its Source callback fixture throws after storing a pending child's node and destruction hook. It verifies the same incoming operation/root/pending-child identities, live whole/nested script tables, unchanged partial store, zero destruction callbacks, zero legacy failure-hook calls, empty registry, and rejection of a subsequent entry. Exit is zero, and the executable contains an `asInvoker` manifest.

The probe flushes its result and uses `_Exit(0)` to preserve the interrupted graph until OS process exit. It is not native Text activation, a real factory test, Original differential/ABI evidence, a successful publication-path runtime test, a discarded-operation termination test, nested-load runtime coverage, or normal retirement proof. The final source and artifact hashes, build result, and exact fixture boundaries are in `reports/cc12_native_page_load_retention_63.json`; ignored local artifacts are evidence only.
