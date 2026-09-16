# Axial, floating, and sprite particle lifetime (orch4 k11)

This packet reconstructs nine complete bodies in
`src/native_particle_three_lifetimes.cpp`: three type destructors and both
scalar entries for each family. It borrows `NativeParticleTypeLifetimeContext`,
including the application's same actual F8D344 parameter pool and raw string
publication/gate/manager. No generic destructor callback or duplicate pool is
introduced. Shared parameter/common-base providers are a separate parent packet.
The descriptive names are hypotheses, not recovered symbols.

## Complete original bodies

| Entry | Inclusive end | Bytes | Role |
| --- | --- | ---: | --- |
| B05940 | B059BB | 124 | Axial destructor |
| B07730 | B077C3 | 148 | Floating destructor |
| B088B0 | B08943 | 148 | Sprite destructor |
| B008C0 | B008DD | 30 | Axial definition scalar |
| B05CE0 | B05CFD | 30 | Axial base scalar |
| B008E0 | B008FD | 30 | Floating definition scalar |
| B07C60 | B07C7D | 30 | Floating base scalar |
| B00900 | B0091D | 30 | Sprite definition scalar |
| B089C0 | B089DD | 30 | Sprite base scalar |

The destructors contain 35/42/42 instructions and end in RET followed by INT3
padding in the installed PE. Independent disk disassembly therefore establishes
their full endpoints, rather than inferring completeness from a zero-gap query.
Each scalar has 11 instructions, calls its listed family destructor, tests flags
bit0, conditionally calls BF65AC, restores EAX to the original owner, and RET4.
The initial live listings each omitted the three-byte `ADD ESP,4` after free.
The parent owns the six locked flow repairs; the report retains their exact sites
and missing half-open spans. The worker makes no Ghidra changes.

The paired report preserves full live/installed-PE SHA-256 hashes and all 600
owned bytes, original names/comments, 31 exact body calls and three cleanup tail
jumps, complete EH records, and all six profile pairs. Queries target the saved
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Raw member schedules

Destructors receive the actual owner in ECX and return with RET; EAX is incidental.
Axial stamps D5DF30 and releases parameter pointers +8C, then +90. Floating stamps
D5DFB0; sprite stamps D5DFF4. Both release +84, then +80, then +88. The first
pointer is captured before arming EH state0. Each later pointer is read only
after the preceding complete release. Null pointers skip both provider calls.

For each nonnull pointer, AFFDF0 disposes the payload, then B00090 returns the
same captured slot to the actual parameter pool. Do not reload that owner's
member between the two calls, clear the member, or gather all parameters before
starting. The common B00FB0 destructor runs after disarming the derived cleanup.
All other owner bytes and the common cleanup schedule are preserved. Factory
allocation evidence establishes A4h axial, 8Ch floating, and 90h sprite storage;
the lifetime bodies neither resize nor initialize those owners.

| Family | Base profile/+4 | Definition profile/+4 |
| --- | --- | --- |
| Axial | D5DF30 / B05CE0 | D5DCC0 / B008C0 |
| Floating | D5DFB0 / B07C60 | D5DCEC / B008E0 |
| Sprite | D5DFF4 / B089C0 | D5DD18 / B00900 |

All six profiles have BD30E0 at slot0. Numeric profiles are original ABI evidence;
callers must compose proven source dispatch rather than execute original numeric
addresses in a host process. This packet does not change resource callers.

## Exception ownership

| Family | Handler | FuncInfo | Map | State0 cleanup |
| --- | --- | --- | --- | --- |
| Axial | CBB7E8 | DF3930 | DF3928 | CBB7E0 -> B00FB0 |
| Floating | CBB8B8 | DF3A44 | DF3A3C | CBB8B0 -> B00FB0 |
| Sprite | CBB948 | DF3AF8 | DF3AF0 | CBB940 -> B00FB0 |

Each complete 36-byte FuncInfo has one unwind state, mapped 0->-1. Its action
reads the saved owner and tail-jumps to B00FB0. A failure during parameter
release therefore destroys the common base, without retrying derived slots or
visiting remaining derived fields. State-1 is set before the normal base call,
so a base exception never invokes that same base twice. The source's noexcept
guard terminates if common-base cleanup throws while another C++ exception is
already active; it does not replace the first exception or continue cleanup.

## Validation scope

The strict MSVC Win32 build and all three CTests passed, including
native_math_differential after verify-seeds. All 34 call/tail rows verified;
the current nine bodies have zero gaps. Independent parent source review found
no normal/EH schedule issue. The final original/source probe passed against the
worker's primary library; hashes and results are in the paired report.
The probe uses copied complete
original bodies with external calls rebound to the genuine shared source
providers and fixed heap free. It compares raw owner images, actual parameter
pool return order, payload effects, common cleanup, and each scalar entry.
No permanent test suite is added.

These are owning C++ interfaces, not original binary thunks. Original FH3/SEH,
fault/concurrency behavior, and second-unwind-exception execution remain outside
the normal probe. Exception schedules are supported by the inspected assembly
and complete maps. No gameplay validation is claimed.
