# Profile write completion chain
Addresses: 007FA670, 007FA220, 007F9500, 007F8F00, 007FA710, 007F9540, 008D64A0

The write request `007fa710` starts storage00bd3dc0 and drives storage with continuation007fa670.
Three callbacks then persist settings, persist profile, and deliver the
shared completion. They are now implemented in `profile_write.cpp`, invoking
the recovered settings and profile writers directly. The driver006adb50 is
now reconstructed in `storage_operation.cpp`; it may complete synchronously or
retain a continuation while an interactive prompt is visible.
`TextProfileWriteHost` binds it to actual text archive output.

All three callbacks use no arguments and RET; `007f9500` tail-jumps a nonnull
completion. `007fa670` and `007fa220` test storage[0109CECC]+8 exactly against1.
That arm calls007f8f00 on game+650h, takes00F87458, clears it, and invokes a
nonnull callback. It does not reset the full profile. The other arm creates
an8h stack store (vtable00ce4104, word+4=0), writes, restores vtable00ce3784,
then drives storage with the next callback without clearing00F87458.

| Callback | Normal call | Next continuation | Final instruction |
|---|---|---|---|
|007fa670|008d64a0 at007fa6e0|007fa220 at007fa6f9|007fa70c RET|
|007fa220|007f9540 at007fa297|007f9500 at007fa2b0|007fa2c3 RET|
|007f9500|take/clear/invoke completion in every state|none|007f9531 RET|

`007f9500` also clears the save name if state==1, but delivers completion for
all states. `007f8f00` constructs an empty pooled string and assigns only
profile+34h; ECX=profile, RET at007f8f90. The C++ version clears save_name_34.
Writer retirement is scoped before driving the next operation, including C++ unwind cleanup.
The abstract factory hooks stand for the native temporary backend lifetime;
they have no default writer or synthetic success result.

`dispatch_profile_io_task` connects the four observed continuation addresses to
their recovered read/write functions. This dispatcher is host composition,
bound to the recovered storage driver by `drive_profile_storage_task`. `ProfileArchiveIoHost` binds the
reader callback through the existing concrete Lua reader and recovered
profile read body. The reader slot must be initialized by its owner before
deserialization and remain valid until the owner's destroy operation.

Independent review checked native state tests, callback ownership and
writer-retirement order. Two absent callback functions were defined from
disk-matched byte ranges under the Ghidra lock; recorded in
`reports/startup_backend_definitions.json`. Final combined build/annotations
are recorded in `reports/startup_backend_integration.json`. There is no claim
of native ABI compatibility or successful game-save I/O.
