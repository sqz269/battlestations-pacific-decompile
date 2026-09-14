# Raw online frame pump

Address: `00A409F0..00A40ACF`, 224 bytes, 64 saved instructions and no gaps.

`pump_native_online_00a409f0` composes the complete normal pump using one
captured actual3F0h manager. Original entry is ECX manager and plain RET; the
descriptive C++ interface is not a binary replacement. Dispatch, UI, storage
and achievements use the same manager and providers, without state mirrors.

The sequence is dispatcher, UI, clock virtual20, upload if current state7,
download if reread state3, achievements(force0), then clock virtual20 again.
Both clock calls reload the canonical clock and use the same16-byte output
image. The first sample is observable even though its numeric result is unused.

The second timestamp executes FILD ticks, CMP current first-byte, FILD
frequency, FDIVP, and FSTP float. A nonzero captured first-byte initializes
F8ABFC with that rounded float. The source then clears the first-byte and
compares extended x87 `current_float - saved_float` against the confirmed
double2.0 at D7A308. FCOMIP/SETA preserves unordered/NaN rejection.

Only ordered delta>2 reaches the callback gate. Gate requires current manager+8C
nonzero, except byte119 nonzero together with DWORD11C zero disables it. Capture
the current callback20 and invoke it with ECX0 when nonnull/enabled. Store the
captured current float into F8ABFC afterward, overwriting callback mutation.
The skipped threshold path does **not** store F8ABFC, except first initialization.
The older pseudocode's apparent unconditional final store is misleading.

The one focused fixture compares copied original A409F0 with the source across
ten cases: initial sample, exact2, a value rounding to2, the next float above2,
NaN current/saved, disabled gates/null callback, and SDK mutation from upload
to download. It compares all manager bytes, timestamp bits, first-byte and
call traces. Five native child calls are redirected to the same reconstructed
raw child implementations; no original SDK/account work runs. The fixture
also covers dispatcher partial output and fake update calls in child processes.

Same-owner construction/destruction, real SDK asynchronous lifetimes, full
client virtual behavior, original ABI/FH3/SEH and gameplay remain open. The
clock's failed-QPC boundary and UI locale's valid-text semantic domain remain
explicit dependencies. See `reports/native_online_pump.json` for exact spans,
call sites and the final registered-library validation checkpoint.
