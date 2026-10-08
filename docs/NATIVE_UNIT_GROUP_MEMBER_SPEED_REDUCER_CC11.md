# Whole raw group member-speed reducer

`reduce_native_unit_group_member_speeds_0070d140` implements the complete
`[0070D140,0070D1A0)` body: 96 bytes, 28 instructions, no calls or relocations.
Installed PE, saved-program bytes, fresh Win32 Source COFF and linked Source
are identical: SHA256
`47b51f3e485a049f57c6a845b2baf5d17ba57214ada864d77b48eaa961c9da42`.
The descriptive function name is a hypothesis, not a recovered symbol.

ECX is the actual group; EDX has no input role; plain RET returns ST0. The
body captures signed group+4F8 once, walks real34-byte records at group+18,
and tests each member identity as an integer DWORD without dereferencing it.
Null records have their +30 speed reset to999 and are skipped. Other speeds
pass through the original FLD/FSTP-float/FLD scratch sequence before x87
comparison. Equality and masked unordered comparisons retain the minimum.
The seed is9999999. This body does not read or write retained group+504.

The only external reads are native DATA at absolute00CFD6F4, and, for a
positive captured count,00CF4888. The bounded verified bits are4B18967F and
4479C000. The header requires actual readable native cells or explicitly
qualified readonly PE-snapshot cells at those exact virtual addresses.
Production creates no mapping, replacement global, callback or default.
The two original8-byte MOVSS encodings use literal `_emit` bytes because
MSVC's numeric absolute-operand syntax otherwise adds redundant DS prefixes.

Ordinary admission is real aligned508-byte storage with valid reached
records; count1..24 is the bounded positive fixture domain. There is no added
guard or count clamp. Nonpositive count needs one free x87 slot for return;
a live record needs two. Production preserves the caller's control word.
Final flags come from ADD ESP,8. EAX retains the captured count; positive
ECX advances to group+48+count*34 and EDX becomes0. Nonpositive ECX/EDX are
unchanged. XMM0 holds the captured seed or999, while XMM1 changes only when
the minimum decreases.

One new standalone family freshly compiled four translation units: reducer,
current raw group views, current group constructor and the ignored fixture.
It linked no BSP archives. The strict build used /W4 /WX /fp:strict. Before
execution, all96 Original/COFF/linked bytes and the genuine current provider
bodies were checked, with176 actual host headers,6 system libraries,3 tools
and4 compiler/backend-support files pinned before and after.

The accepted fixture first reserved its own exact64KB native DATA range,
then opened the frozen PE. It copied original pages00CF4000/00CFD000 and made
them PAGE_READONLY. Full page bytes, scalar cells and VirtualQuery protection
bookends remained unchanged. The installed executable and live game were
untouched. These mapped bytes are frozen native data, not live-game evidence.

The single executed family had constructor-empty and one five-record phase
`[A,A,NULL,B,C]`. Current Source record/member views and publisher produced
negative zero, positive zero, signaling NaN7F812345 and a non999 null speed.
Both Original and Source ran on the same actual group storage, reconstructed
through the real current constructor and explicit caller-authored fields
before each run. Comparison snapshots were never passed as target inputs.

Both phases passed. All364 physical capture bytes matched except the
expected final-FLD instruction pointer: Original00C70059 versus
Source20001059 in this run. The adapter captured native scratch before any
register/flags-saving pushes and captured80-bit ST0 before a C++ spill.
All GPRs, full flags,8 XMM registers, MXCSR, stack balance,64 stack guards,
x87 control/status/tag/register state and1424 guarded receiver bytes were
compared. Six exact80-bit canaries occupied the input x87 stack, leaving
exactly two free slots; all six survived. The positive phase returned
negative zero, quieted scratch to7FC12345, changed SW4D20 to4D21 through
masked invalid, and reset only the null record speed to4479C000. The existing
retained getter still observed caller-authored group+504 value6.25.

Five immutable build attempts are retained: an alignment warning, a rejected
98-byte DS-prefixed body, an unsupported bare numeric MOVSS spelling, and
two clean96-byte builds. The first probe launch stopped at address reservation
before any constructor or reducer call. Reserving before the PE file view
fixed the setup order. Only the accepted launch executed the native family:
two Original and two Source reducer calls, with6723 checks and zero failures.
Check count includes byte/guard, artifact and setup checks; it is not a suite
of separate gameplay cases. No prior family was replayed.

The report identifies the sealed recipe, every attempt and all hash bookends.
Prior47- and25-artifact audits and the187-artifact direct-view family remain
unchanged. This establishes a raw component and bounded fixture contract.
Full project integration/build, original class/lifetime, membership/observer,
unmasked faults, concurrency, enclosing009F4DA0 throttle behavior and game
validation remain separate. It does not close0070DA00 or0070D1B0.
