# Live effect manager frame update

Addresses: `00867EE0`, `0081B010`.

The manager update now composes the recovered point advancement, frame jobs,
child updates, retirement, and deferred deletion routines over the same actual
application owners. `src/live_effect_update.cpp` introduces typed interfaces;
these are not native vtable replacements. Descriptive names are hypotheses.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| `00867EE0` manager update | Complete `00867EE0..00867FC6`, 231 bytes | ECX actual 28h manager; stack float delta and borrowed reference; RET8 at `00867FC4` |
| `0081B010` owning raw-array erase | Complete `0081B010..0081B092`, 131 bytes | ECX actual 0Ch array; stack address of iterator; RET4 at `0081B090`; iterator unchanged |

Both spans match the installed executable and saved `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. The executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Exact body hashes and call-site evidence are in
[the report](../reports/live_effect_update.json).

## Four phases

The first traversal reads the owning array count before its backing pointer and
captures the resulting end once. It skips null cells. Every nonnull point runs
complete `00867D00`, with an x87 FLD/FSTP copy of the original delta before each
call. This invokes the actual restart/stop controls, parent/node transforms,
and sampling stages documented in [POINT_EFFECT_ADVANCE.md](POINT_EFFECT_ADVANCE.md).
Callbacks may change cells in the captured span; replacing the array header
does not redirect that first traversal.

Next, complete `00866C60` receives another x87 copy of the original delta and
the original borrowed reference. It queues the captured non-owning event span
through the actual frame singleton and worker pool, then dispatches even an
empty span. The eight-byte job singleton and execution-time shared arguments
are documented in [NATIVE_EFFECT_JOBS.md](NATIVE_EFFECT_JOBS.md).

The third phase reloads the owning array backing and its live end. Each cell
must be nonnull here. It runs complete `00867790`, then reloads that same cell
before testing retirement: byte `+09` nonzero, signed auxiliary count `+1C`
less than or equal to zero, byte `+0A` zero, and all pointers in the captured
primary `+0C/+10` span null. A retiring point is erased without advancing the
cursor, so a tail moved into that cell is visited next. Otherwise the cursor
advances four bytes. The current array header supplies the next end after
every child callback and erase. Child/event lifetime behavior is documented
in [POINT_EFFECT_CHILDREN.md](POINT_EFFECT_CHILDREN.md).

Finally complete `008671A0` drains the actual pending-deletion list. The manager
adds no entry lock, defensive retain, exception rollback, or replacement owner
registry. Required borrowed spans and objects must remain valid until their
native last access.

| Native call site | Callee |
| --- | --- |
| `00867F18` | `00867D00` point advancement |
| `00867F31` | `00866C60` non-owning event jobs |
| `00867F53` | `00867790` child update |
| `00867F9C` | `0081B010` owning array erase |
| `00867FBA` | `008671A0` pending deletion flush |

## Owning raw-array erase

Erase captures the destination iterator and initial tail. For distinct cell
addresses with distinct values it publishes the tail into the destination,
retains that actual owner's atomic `+04`, then releases the captured old
owner through its current canonical terminal action. It then reloads the
current array header, captures the current tail, releases its nonnull value,
and clears that captured tail after terminal reentry. The live count is
decremented modulo 2^32 last. No incoming retain/release occurs for identical
values; the final tail release still occurs. The caller's iterator is unchanged.

This raw storage route uses the same canonical point owners and actual counts
as `PointEffectReleaseRuntime`; it does not reinterpret a host companion pointer
as the native primary owner. Valid nonempty spans and iterators are required.
Canonical terminal operations are nonthrowing; unsupported bindings remain
explicit errors rather than invented implementations.

## Validation and limits

The combined strict MSVC Win32 build and both existing CTests passed in
`local/effect-frame-ah-final-build.log`. The worker control and advancement
reports contain complete original-body comparisons, including real x87 sampling
precision and callback-sensitive ownership. Job lifetime checks cover real
Win32 synchronization, secondary registration, worker construction, and shutdown.
The complete original 231-byte manager, including RET8 and stack balance, now
matches C++ across normalized 114h point state and lifetime counts.
`local/live-effect-frame-probe-ah.log` covers actual parent refresh/full4x4
sampling, restart with real factories, nine nonempty phase-two rumble updates,
nine serial updates/completions, three cancellations, owning erase, deferred
zero-count deletion after active dispatch, and an empty frame. Actual node,
string, component, definition, event, frame-job and singleton teardown passed.

The original manager calls five ABI bridges into the same recovered C++ callees.
Population of the borrowed event span, including after restart, is an explicit
fixture input. The manager fixture executes the C++ raw erase; static live/disk
comparison establishes that its original 131 bytes exactly match the already
tested `00867210` body. This is a single-point retirement fixture, not an
exhaustive manager callback-mutation test.

Independent static review found no mismatch within the valid-span, nonconcurrent
domain. Eleven recovered Ghidra signatures/names and evidence comments were saved,
read back, checked for preserved prior comments, and re-exported. See the report
for the original values, new values, body hashes, bridge details, and limits.

These checks do not establish native object/vtable ABI replacement, execution
of the original exception dispatcher, physical device output, or gameplay.
Other event families require their existing application implementations.
