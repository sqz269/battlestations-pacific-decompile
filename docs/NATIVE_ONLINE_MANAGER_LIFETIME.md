# Actual online manager construction and destruction

Addresses: `00A40DF0`, `00A3F9D0`, `00A3F840`, `00A3FDC0`.

These are reconstructed normal bodies over the actual 3F0h allocation. The
constructor takes its owner from `NativeOnlinePumpContext::notifications.manager`;
base publication, sign-in reset, pump and return therefore share that address.
The lifetime borrows the same F8ABE8 and 01090AA0 cells through singleton drain.
Names describe observed behavior and are not recovered symbols.

`A40DF0` calls the raw base constructor before initializing derived fields.
It preserves all allocation bytes not written by the listing, including vector
header +360 and fields +398/+39C. It abandons old vector pointer fields and +3A0
without freeing them. Two callback DWORDs are copied to +20/+24. Application
startup's later +18=737D60 publication remains a separate caller obligation.

The constructor captures the current renderer for B1FEF0's device read, then
reloads the renderer to pass its actual mutable +1A28 present-parameters address.
The 1Ch initialize image is cleared; the caller-owned 400-byte WSADATA image is
not. XLiveInitializeEx, XOnlineStartup, XWSAStartup and port-setting results are
ignored. The WORD WSADATA version is tested even if startup failed or wrote only
part of the output. A mismatching version calls cleanup. XSocketNTOHS's entire
DWORD result is forwarded to the system-link port setter. Listener return bits
are retained at +1C regardless of success. The verified 4254B0 diagnostic sink
is a bare RET, so it has no source operation.

The actual +3AC pointer slot is initialized through the raw IPC provider. After
sign-in reset and the complete raw pump return, +14C and +12C are cleared. In
particular, a buffer installed at +14C by a nested pump callback is abandoned at
that point without release. Source callers explicitly supply valid preimages
and keep the SDK, IPC, CRT, clock, pool and profile services alive.

`A3F9D0` rewrites D2413C, closes the retained IPC handle, tests/reloads/tests the
current +14C pointer before releasing it, and clears that field only after the
release returns. It then releases current vector +364 and clears +364/+368/+36C
before base unregistration. It does not close the listener, shut down the SDK,
release +3A0, or clear +3AC. `A3F840` is the vector cleanup with ECX pointing at
the embedded +360 header. `A3FDC0` runs destruction and releases the captured
allocation only for flags bit 0, returning its address even after release.
Source scalar deletion requires `new NativeOnlineManagerStorage` without value
initialization and uses the matching C++ delete domain.

The two FH3 unwind maps each contain exactly vector-then-base cleanup. Constructor
metadata DE9F60 points to DE9F50: state 0 calls CB4260 -> A3F5D0, and state 1
calls CB426B -> A3F840 before state 0. State 0 is installed at A40E7D and state 1
at A40E9B. The base call at A40E13 precedes both. Destructor metadata DE9E08
points to DE9DF8: CB4160 is base cleanup and CB4168 is vector cleanup. State 1
is installed before IPC close; state -1 is installed at A3FA51 before the normal
base call. No IPC, +14C or +3A0 unwind action exists in either map. Source guards
preserve ordinary C++ cleanup order and terminate if cleanup itself throws.
They do not reproduce original FS/EBP/FH3 ABI or asynchronous SEH.

`NativeSingletonDeletionBindings::online_lifetime` routes D24138/D2413C to these
raw scalar wrappers using the popped owner, without requiring it to equal the
current publication. It is mutually exclusive with the legacy projected owner
binding. `GameSingletonHost::bind_native_online_lifetime` selects this alternative
and retains the borrowed context through shutdown. The game startup host still
needs to construct and publish this raw online lifetime and connect raw cursor
and frame services; these entrypoints alone do not establish that adoption.

Independent assembly/EH review accepted the four source bodies after restoring
the destructor's second +14C test. The report records exact saved/disk byte spans,
all call sites, review and validation results. Builds, focused fixtures, original
ABI compatibility and gameplay validation remain separate claims.
