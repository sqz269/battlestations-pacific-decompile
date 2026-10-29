# Icon process host hookup

Read-only Source integration proposal against root `b920a83f1a4796b0c75d6d16b1deb6544166168a`
and Icon process `82c09a7399`. Only this plan is changed. The process TU is
already compiled; no CMake change, native getter, build, game, Ghidra mutation,
raw Icon Model transport or P4 activation is supplied.

## Exact context lifetime

`GameNativeVfsApplication::Impl::raw_strings` is an APPLICATION-owned context
(cpp57/87), returned through GameVfsHost::raw_strings (cpp122). Its three
references alias the process string pool, returns-disabled and manager cells;
the context object itself dies with VFS. Do not describe it as process-owned.

Prefer `game_native_string_process().raw_context()` for this permanently retained
Icon service. That process owns both the context and its three cells
(game_native_string_process.hpp34/53-58); its accessor retains the process with
no atexit destruction. GameSingletonHost's manager reference is initialized from
that SAME process manager (game_hosts_singletons.cpp18-19). VFS's pool/disabled
references also originate there (game_native_vfs_application.cpp48-51).
At startup compare the ADDRESSES of all three raw-context reference members
against vfs_->raw_strings(), and the manager-cell address against
singletons_->manager_publication_01090aa0(). Never copy current pointer values
into replacement cells or supply a temporary raw context.

The delayed service stores only that raw context and the Icon process publication
reference; it has no GameNativeReadOnlyData borrow. Existing VFS owner/deletion
views still borrow the verified data mapping and must keep their established
whole-host lifetime. game_main1114-1119 supplies the data owner and host1318
borrows it; this hookup neither replaces nor shortens that lifetime.

The older process doc's vfs_->raw_strings() snippet is usable only while that
EXACT context survives. The permanent process context above is a deliberate
stronger lifetime choice. The service rejects a different context object after
preparation even when all three cell aliases match (process.cpp18-31).

## Smallest shared-host edits

Root owns only game_hosts.hpp/.cpp edits: include/forward declarations, one
nullable `GameNativeGuiIconProcess*` host borrow, and private prepare/guard/
after-drain helpers. Preserve the rest of NativeSingletonDeletionBindings.

Prepare immediately after successful vfs_->phase2 at game_hosts.cpp2450, before
startup factories/platform/frontend/any future raw Icon can enter. In ordinary
throwing startup, obtain game_native_gui_icon_process(), require the host borrow
and existing gui_icon_delayed_loads arm are empty, require process status
unprepared (new-host reuse is unsupported), and verify the exact cell aliases.
Then:

```cpp
auto& service = process.prepare_delayed_loads(process_strings.raw_context());
singletons_->native_deletion_bindings().gui_icon_delayed_loads = &service;
native_gui_icon_process_ = &process;
```

Both assignments after preparation are nonthrowing. Requiring unprepared is a
HOST one-shot admission; the lower-level API's documented same-context repeated
metadata access is not permission to rehost or restart after drain. Require
idle/null after preparation. No get_00ab5700/cancel/delete/manager registration
is called. Preparation failure leaves no entered native operation; propagate
normally. Never replace the entire deletion-bindings aggregate.

Any future raw Icon context must borrow THIS service and the SAME singleton
deletion object, plus raw strings over those same three cells. Its existing
preflight checks service equality before destruction (native_gui_icon_storage
cpp69-72); ordinary AB5E70 reaches AB5700 then AB50E0 even with delayed=false.
Existing native_singleton_destruction.cpp92-97 already routes D5C4B8 to this
service's delete_registered_00ab57c0. No new dispatcher arm is needed.
Serialize all service/status/guard observations with Icon operations and native
drain; these borrowed process APIs supply no additional synchronization.

## Sticky guard and successful drain

The noexcept guard uses ONLY the stored nullable host pointer. Do not call
game_native_gui_icon_process() there: its first access performs C++ new and can
throw. Do not prepare or call the native getter from a guard.

| Observed status | Before full drain |
| --- | --- |
| No host borrow | No prepared Icon service to inspect. |
| idle | Continue. |
| registered_owner | Normal native custody; continue to real shared drain while preserving every provider. |
| incomplete_operation / failed_operation | Log, close log, and use existing sticky _Exit(1) policy; retain the WHOLE graph. |
| unprepared despite installed borrow | Host invariant failure; retain/exit, never repair or retry. |

Do not use low-level service.requires_process_retention() as the failure test:
it deliberately includes ordinary registered custody. Process.cpp45-53 supplies
the precise status split without reading registered payloads.

Call the guard at the existing destruction/exit guard clusters:
host destructor2048, exit_process2222, application_shutdown3204 and
destroy_singleton_lifetime_manager3339, plus centrally BEFORE native drain1932.
On a drain exception retain the graph and exit; no later ordinary provider
destruction may proceed merely because a publication was cleared partway.

Put successful Icon verification in BOTH drain_native_singletons branches,
including the current no-startup-factories early-return branch1933-1937. Only
after renderer.drain_singletons() has returned from full after_native_drain
(renderer_application.cpp1296-1344), or the genuine fallback singleton shutdown
has completed, require status idle AND publication_00f8bd98()==nullptr. Also
require the same deletion arm and null shared manager. Read no old payload.
An unexpected registered/active/failed/non-null result retains/exits; never
invoke delete/cancel/get as a fix or clear the publication manually.

Keep the service binding and process borrow through GameSingletonHost's fallback
destructor/shutdown (singletons.cpp37/164). The retained process metadata itself
is not destroyed. Existing host order deletes singletons2139 before VFS2161;
permanent raw-context backing also avoids leaving the Icon process bound to the
later-deleted VFS context. Other raw Icon, string, manager, data and callback
borrowers remain quiescent through every drain and failure disposition.

Dormant hookup can prove preparation/binding/idle-null closure only. Genuine
AB5700 allocation/publication/registration and AB57C0 popped-owner retirement
remain lazy; neither compilation nor an uncalled service proves that lifecycle.
Post-drain native reuse, another host, delayed insertion/file delivery and a
complete raw Icon/Page graph remain unsupported.
