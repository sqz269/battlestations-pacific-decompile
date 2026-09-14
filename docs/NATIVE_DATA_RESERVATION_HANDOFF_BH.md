# Controlled native-data reservation handoff (BH)

`GameNativeReadOnlyData` needs numeric tables at their original virtual addresses.
The direct constructor still reserves the selected 64 KiB bands itself and rejects
an occupied band. The BG timing probe found that the dynamic loader sometimes
creates a secondary heap at `D10000` before executable TLS. Suspending a child
before its loader runs is the usable reservation point. This change adds an
explicit source-side handoff; it does not modify the original executable.

The parent constructs `GameNativeDataBootstrapChild` for a controlled Win32
child executable, passing the exact required spans. It creates an anonymous
shared section and uses `STARTUPINFOEXW` with a one-handle
`PROC_THREAD_ATTRIBUTE_HANDLE_LIST`; only that section handle is inherited.
The child starts suspended. `reserve_and_resume()` reserves every selected
band with fixed-address `VirtualAllocEx(MEM_RESERVE,PAGE_NOACCESS)`, writes the
child PID and complete band mask into the section, marks it ready, then resumes
the thread. A failed allocation releases only bands allocated by this call and
leaves the child suspended for inspection/destruction. A failed resume rolls
back those bands and terminates the controlled child. Before successful resume,
the parent owns the remote reservations; afterward it never frees them. Process
teardown covers an early child failure.

At its entrypoint the child calls `accept_native_data_handoff(spans,count)`.
The argument names the inherited section handle. Acceptance checks the record
version, child PID, exact span-derived mask and ready state, then atomically
claims it once. Every selected band must still have its exact 64 KiB
allocation base, `MEM_RESERVE`, `PAGE_NOACCESS` allocation protection and
`MEM_PRIVATE` type. Merely seeing `MEM_RESERVE` is insufficient. The returned
`GameNativeDataReservation` is move-only. It releases its verified bands if
mapping never consumes them. The mapper's adopted constructor validates the
same span mask and states again, transfers every band at once, verifies the
complete supported original PE by SHA-256, commits and copies only selected
`.rdata` pages, then protects them `PAGE_READONLY` (non-executable). The mapper
owns the transferred bands until destruction; construction failure releases
them and reports failure. A second claim, an empty capability, a changed band,
or a different span set is rejected. The direct constructor has no adoption
path or fallback.

The child must retain `GameNativeReadOnlyData` through all raw consumers and
their singleton drain. The parent should call `wait_for_mapping(timeout_ms)`;
it returns only after the mapper has completed hash/copy/protection. It reports
rejection, early child exit and timeout; on rejection or timeout it terminates
the controlled child. Destruction without that acknowledgement also terminates
the child. The caller may use `process_handle()` to await the rest
of the child run. The production `bsp_game` entrypoint and CMake registration
are integration work owned separately. A direct launch remains exposed to the
pre-TLS heap collision.

Strict MSVC Win32 `/W4 /WX /EHsc /MD` translation-unit builds passed. An
ignored fixed-base Win32 probe with an embedded manifest and TLS callback
launched eight children through the handoff. All eight completed original-image
SHA-256 mapping, read-only/non-executable page checks, one-time claim checks,
and release checks. The probe also surfaced a bad-image mapper failure and an
early child exit. An injected `D10000` committed read/write competitor made
the parent reservation fail; its owned `CF0000` band returned to `MEM_FREE`
while the competitor remained committed. A separate suspended direct-launch
child confirmed the original constructor rejects that competitor and rolls
back `CF0000`. An adapted ignored BG probe completed the full raw VFS manager
graph under an adopted handoff: four factories, three mounts, disk-equal
657-byte member read, search groups and shared drain. The exact ignored
artifacts and hashes are in `reports/native_data_reservation_handoff_bh.json`.

These are source, linked-probe and fixture results. They do not establish
arbitrary-launch reliability, production `bsp_game` integration or gameplay
validation. The inherited section protocol prevents accidental adoption of
unrelated reservations in a controlled child; it is not a security boundary
against hostile code already executing in that child. Callers must not mutate
or release transferred bands outside the capability and mapper.
