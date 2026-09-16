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
VFS entry path and the composed particle graph: `CE`, `CF`, `D0`, `D1`, `D5`,
`D6` and `D7`, each as one 64-KB band. The controlled parent/child handoff and
canonical data owner receive the same array. `GameNativeReadOnlyData` still
verifies the original executable and admits only read-only, non-executable
original-image pages; this packet adds no writable fixed-address mapping.

This is an access and mapping prerequisite only. The particle runtime is not
constructed, registered or dispatched. The actual resource manager/load cache,
texture cache, render owners, `00F8D344` parameter pool, particle-cache and atlas
publications, platform host, scratch/empty strings and load-event host remain
separate prerequisites. `GameResourceManager` is a semantic structured-resource
registry, and `GuiLuaRef` is a registry handle; neither is substituted for a raw
native owner.

Validation consists of the strict MSVC Win32 build, the repository's three
existing CTests, source inspection of the borrowed cell identities and the
seven-band bootstrap union, and `verify_report_calls.py` on the companion report.
These checks do not prove native ABI/FH3 behavior, particle dispatch, rendering,
gameplay, or a successful installed-game run.
