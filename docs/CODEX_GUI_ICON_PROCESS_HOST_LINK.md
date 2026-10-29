# Icon process host linkage

This integrates the existing process service from `82c09a7399`; it adds no
reconstructed body, native getter, Icon creation or Page activation. The
source plan is `CODEX_GUI_ICON_PROCESS_HOST_LINK_PLAN.md`. Changes are limited
to the startup host declaration/implementation and this record.

Immediately after successful VFS phase2, startup prepares the genuine delayed
service using `game_native_string_process().raw_context()`. This permanent
context survives VFS deletion. Admission compares the addresses of all three
pool/control/manager cells with the VFS view and the manager cell with the
singleton host. It rejects a previously prepared process, installed host borrow
or occupied Icon deletion arm; post-drain rehosting remains unsupported.

Successful preparation installs only
`native_deletion_bindings().gui_icon_delayed_loads`, then retains the process
borrow. The metadata constructor does not call AB5700, cancel, delete, register
or read a native Icon payload. Existing D5C4B8 dispatch already calls AB57C0
through this same service. No dispatcher or native allocation was added.

The existing central renderer guard also observes the stored process pointer.
It neither allocates metadata nor calls a lazy getter. Idle and ordinary
registered custody are permitted before drain. Unprepared-after-binding,
incomplete and failed operations take the existing log-close/_Exit policy,
retaining providers. Destructor, process exit, application shutdown and manager
destruction already pass this guard before provider release.

Both shared drain branches now run inside the sticky failure catch, including
the former no-factories early return. After successful full renderer/fallback
drain, before shared factory facade release, the host checks idle status, null
Icon publication, null canonical manager and the unchanged deletion-service
identity. It never clears a cell, invokes delete/get to repair a result or
reads retired payloads. Repeated successful drain observations keep the same
permanent service. The binding and process borrow survive the singleton host's
fallback destructor, which precedes VFS deletion.

The Win32 build and both existing suites passed (13.68 seconds), receipt
`local/cc16_icon_process_host_build.log`. Independent final diff review and a
frozen short startup/teardown smoke remain pending. A passing idle smoke will
establish metadata binding and idle/null closure only. It cannot establish
AB5700 allocation/publication/registration, popped-owner AB57C0 retirement,
delayed file delivery, raw Icon Model transport, rehosting, full UI, visual or
Original ABI behavior. No new test cases or test framework were added.
