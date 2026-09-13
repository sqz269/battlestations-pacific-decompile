# Actual render-service base publication and destruction

Addresses: 00b0f020, 00b0f0c0, 00412430, 00b0cf30, 00b0d100

The module reconstructs five complete native bodies, 319 bytes: three new
base-lifetime bodies and raw-domain extensions of two existing typed getters.
Names describe established behavior and remain hypotheses, not recovered symbols.

| Native entry | Inclusive end | Bytes | Original ABI / behavior |
| --- | --- | --- | --- |
| `00B0F020` | `00B0F0B0` | 145 | ECX receiver, no arguments, EAX original receiver, RET; publish/register base. |
| `00B0F0C0` | `00B0F158` | 153 | ECX receiver, no arguments, void, RET; unregister/clear base. |
| `00412430` | `00412436` | 7 | ECX receiver, no arguments, void, RET; store `00CE3818` at +0. |
| `00B0CF30` | `00B0CF36` | 7 | ECX actual service, no arguments, EAX=ECX+84h, RET. |
| `00B0D100` | `00B0D106` | 7 | ECX actual service, no arguments, EAX=ECX+1D8h, RET. |

The source borrows the actual manager cell `01090AA0` and service cell `00F8D39C`.
It calls the completed raw manager getter `00415350`, raw vector registration
`00BD0C30`, removal `00BCFCA0`, guard destructor `00411EE0` and actual Win32
critical sections. No typed service layout, substitute manager or callback stub
is introduced. Raw getter overloads compute 32-bit pointer identities without
reading memory; existing typed getter functions remain available.

The publisher arms base cleanup before its `D5E158` profile write. It gets the
manager, captures its section+10h, enters that section, increments depth+18h,
then arms guard cleanup. It publishes the original receiver, looks up the
manager again, and only then reads the current publication for registration.
It decrements and leaves the captured section, returning the original receiver.
A returning registration validator may change publication; the registered
argument and returned receiver remain captured. A registration exception retains
publication, runs guard cleanup, then writes `CE3818` to the original receiver.

The destructor writes `D5E158` before arming base cleanup. Its capture/enter and
second manager lookup follow the same order, but removal receives the current
publication, which need not be the original receiver. Successful removal is
followed by an unconditional publication clear, captured-section leave and
original-receiver `CE3818` store. Failure skips the normal clear and performs
guard then base cleanup. The source guard stays armed through the normal leave,
matching the native state's lifetime.

The two native FH3 handlers `CBBBF0..CBBBF9` and `CBBC10..CBBC19` load
`DF3E2C`/`DF3E60` then jump to `BF6B43`. Each 36-byte FuncInfo references a
two-entry, 16-byte map at `DF3E1C`/`DF3E50`: state0 invokes the eight-byte action
at `CBBBE0`/`CBBC00`, loading `[EBP-18h]` for `412430`; state1 invokes the
eight-byte action at `CBBBE8`/`CBBC08`, addressing the captured eight-byte guard
at `EBP-14h` for `411EE0`, then continues to state0. Both handlers were absent
when the worker first queried them; the integrator subsequently defined and
saved their exact ten-byte bodies. The worker made no Ghidra mutations.

Verification and replay evidence is recorded in
`reports/native_render_service_base.json`. The external fixture directory is
`C:/Users/sqz269/bsp-az-render-service-base`. It preserves complete source inputs,
original bytes, metadata, generator, logs and exact library pins. The worker
baseline build excludes this unregistered module, which is separately compiled
with `/W4 /WX` and explicitly included in the worker fixture. The default replay
after integration uses only `probe.cpp` and the current three project libraries.

The strict module compile, seed verification, baseline Win32 build and both
existing CTests passed. Twelve numeric native call rows passed live checking;
four imported section calls are recorded separately. Eight live/disk spans
match: 319 normal-body bytes, 52 handler/action bytes and 104 FH3 data bytes.
The fixture passes ten parent original/source pairs plus all three complete
leaves, comparing 11,400 normalized state bytes and 26 event DWORDs. It checks
lazy raw manager construction, existing-manager registration, genuine throwing
and returning registration validation, current publication distinct from the
receiver, first-match removal with duplicates and raw getter identity/wrapping.
Two destructor cases temporarily redirect the first count CALL operand of the
private probe's linked `BCFCA0`: the wrapper calls the real count, then changes
begin before the real bounds validation. Its actual CRT handler throws or
returns after repairing storage and mutating publication/section. All 115
linked provider bytes are restored and verified afterward. All three library
files and every source/native-byte input remain unchanged across the fixture.

The source adds context arguments and does not reproduce the original callable
ABI or private EH stack frames. Valid actual storage and source provider/CRT
domains are required. Hardware faults, incoming-stack or EH-spill aliases,
original exception identity, concurrent access, full derived initialization and
destruction, runnable game linkage and gameplay remain unproved. Controlled
destructor fixture mutation, when used, is input delivery between native reads;
it is not evidence of a naturally reached removal failure or concurrency.
First manager allocation/construction failure, first-enter failure, null-section
paths and a second exception during cleanup are source/assembly-reviewed but
not fixture-tested. The fixture uses the host `__CxxFrameHandler3` and native
FH3 metadata; six parent operands and four action operands relocate required
source providers/handler trampolines. It does not claim untouched whole-parent
byte identity or original exception-runtime identity.

## AZ integration analysis refresh

The integrator saved and read back all 27 AZ original signatures and complete
normal-body ranges, and refreshed exports. CBBBF0 and CBBC10 are ten-byte
analysis-only EH handlers defined under leases and the write lock. Earlier
missing-function observations remain worker capture history. The batch adds
22 complete body records and extends five existing bodies with raw interfaces;
the two EH definitions add no normal-body count. Exact combined validation
follows separately from worker fixture evidence.

## AZ exact merged validation

The exact combined source commit `7813bdc988999a39fa150b0ef9ab3529668fe27b` passed the strict Win32
build, both existing tests and five current-library-only original-byte fixtures.
See `reports/native_system_sources_az_validation.json` for hashes, preserved
captures, case coverage and limits. Earlier pending statements describe worker
capture stages. Full rendering, native ABI and general concurrency remain open.
