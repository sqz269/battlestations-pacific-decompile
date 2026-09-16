# Native particle application-service prerequisites

Addresses: 01090AA0, 01090AA4, 01090AA8, 0109CEEC, 00F8D31C.

This packet exposes existing application-owned services needed by the composed
native particle graph. It adds no original function body and makes no Ghidra
change. All names remain descriptive source names rather than recovered symbols.

`GameNativeVfsApplication` now retains one `NativeStringRawPoolContext`. Its
three references are the same mutable `01090AA8` string-pool publication,
`01090AA4` return gate and `01090AA0` singleton-manager publication already
owned by the VFS and singleton hosts. The context allocates no storage and adds
no provider or callback. `raw_strings()` returns that stable context by
reference. `borrow_raw_services()` forwards the existing runtime bundle: the
live `0109CEEC` VFS publication, native binding table, name-resolution context
and physical date route. `GameVfsHost` exposes both views; its VFS accessor uses
`active_runtime()` so an interrupted native operation still requires process
retention rather than further use.

`GameSingletonHost::game_resource_factory_alias_00f8d31c()` returns a reference
to the existing WinMain alias cell populated by `008F840B..008F8414`. It does not
create another factory publication or collapse the alias with `00E19B90`.
Native owner deletion continues to clear `00E19B90` and leave the `00F8D31C`
bits intact. The existing `manager_publication_01090aa0()` and
`sound_lifetime()` accessors remain the borrowed lifetime boundary.

The normal and exceptional host paths still drain the singleton manager while
the singleton host, VFS application, raw string context, VFS runtime bindings
and publication cells are alive. VFS retirement happens immediately after the
shared drain. Borrowed views must not be used after that drain; in particular,
the retained `00F8D31C` value may then be dangling and is never dereferenced by
this packet.

`game_main.cpp` now requests the explicit seven-band union needed by the existing
VFS entry path and the composed particle graph: `CE2000`, `CF`, `D0`, `D1`,
`D5`, `D6` and `D7`, each selecting one 64-KB band. `CE2000` is the declared
`GameNativeReadOnlyData` begin address; `CE0000` is invalid even though it would
name the same band. The controlled parent/child handoff and
canonical data owner receive the same array. `GameNativeReadOnlyData` still
verifies the original executable and admits only read-only, non-executable
original-image pages; this packet adds no writable fixed-address mapping.

After that verified handoff and before `GameStartupHost` construction,
`game_main.cpp` obtains the one `GameNativeParticlePoolProcess`. It calls
`initialize_model_once_00cd7830()` and then
`initialize_parameters_once_00cd78b0()`. The DATA references at `00CE34EC` and
`00CE34F4` establish that relative CRT order; this packet does not claim or
reconstruct the initializer between them. Both pools use the existing
`00E188B4` allocator-list domain owned by `GameNativePhysicalPoolProcess` and
retain their actual `0x38`-byte storage for process lifetime.

Each initializer's actual `atexit` return status is logged. A nonzero status is
not converted into application failure or rollback: the initialized pool remains
retained and no substitute callback is installed. A thrown initializer is an
explicit startup failure and returns process status 1. If the parameter startup
throws after model startup completed, the model owner's genuine callback remains
registered. The process wrapper rejects retry after a throwing attempt.

This remains an access, mapping and pool-startup prerequisite. The particle
runtime is not constructed, registered or dispatched. The actual resource
manager/load cache, texture cache, render owners, particle-cache and atlas
publications, platform host, scratch/empty strings and load-event host remain
separate prerequisites. `GameResourceManager` is a semantic structured-resource
registry, and `GuiLuaRef` is a registry handle; neither is substituted for a raw
native owner.

Validation consists of the strict MSVC Win32 build, the repository's three
existing CTests, source inspection of the borrowed cell identities and the
seven-span bootstrap union, a focused `native_data_band_mask` acceptance/rejection
probe, and `verify_report_calls.py` on the companion report.
These checks do not prove native ABI/FH3 behavior, particle dispatch, rendering,
gameplay, or a successful installed-game run.
