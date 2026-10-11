# CC12 C04EF3 controlled entry metadata diagnosis

Controlled attempt 02 captured four read-only Native commands with a stable
observed count **64,927** and epoch **4,984**. Opening and closing stdout both
report `live function count 64927; snapshot 64927; same`. All six captured
program-info replies and both count replies agree. Both typed-flow wrappers and
both raw before/after epoch pairs report 4984. The two 2,165-byte raw metadata
payloads are byte-identical: SHA-256
`5d51a748d8c604f159a36c1b501208dd549a071ef121a2140ad552253782037f`.

This is metadata evidence only. No baseline or physical admission follows from
it. No bytes, instruction text, context or successor were queried; instruction
opcodes, behavior and ABI remain unestablished.

## Complete saved entry metadata

Each of `instruction_at`, `instruction_containing`, `code_unit_at` and
`code_unit_containing` identifies the instruction at `00C04EF3`, saved length 7,
class `ghidra.program.database.code.InstructionDB`, with `defined=null`.
Function-at and function-containing both identify entry `00C04EF3`. The complete
saved owner has one inclusive range `00C04EF3..00C04EFA`, length/address count 8.
Saved `no_return`, `is_thunk` and `thunk_target_recursive` are false; direct thunk
target is null. `BSP_Crt_ClearDebuggerHookState` is an unauthenticated descriptive
label, not established behavior.

Default and effective flow independently report `FALL_THROUGH`, with every
call/jump/conditional/computed/terminal flag false. Both target arrays are empty;
both saved fallthrough addresses are `00C04EFA`. Override is `NONE`, fallthrough
is not overridden, and direct-call companions are empty. The named fallthrough
and remaining owner byte were not queried.

All project/program observations identify `bsp`,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, base `00400000`, default space `ram`. The report retains the
complete raw payload, all flags, arrays, owner ranges and original HTTP records.

## Original failures remain separate

The earlier ENTRY stop remains **64,747 observed versus 64,742 required**, with
snapshot 64,730, no observed epoch, preparation failure 1 and collector assertion
failure 1. Its Native subprocess succeeded. Its original report and histories
remain unchanged.

Attempt 01 of this packet opened at **64,860**, snapshot 64,748 (`changed`). Its
first typed wrapper reported **64,863** both before and after, with raw/wrapper
epoch **4,317**. The collector stopped at the count comparison after exactly two
Native commands/eight HTTP responses. It did not run the second typed pass or
closing count. Its collector assertion failure is 1; preparation, Native command,
transport, decoder, inspection and display failures are 0. The 200-file original
stop manifest is unchanged. All 200 files plus that manifest and its handoff were
copied and checked byte-for-byte as 202 distinct retained originals.

The full original attempt-01 typed payload differs from controlled attempt 02 at
exactly two top-level fields: `modification_number_before` and
`modification_number_after`, each changing from `4317` to `4984`. Excluding only
those fields leaves the complete payloads equal. Both original epoch fields and
the full difference list are retained; the failed attempt gains no admission.

Controlled attempt 02 uses a separate prepinned producer, driver, output paths,
invocation and original streams. Root opened a maximum 120-second coordination
window at `2026-10-11T18:25:04Z`; the original ready record is retained as pinned
bytes inside the local control record. Root subsequently reported early release
after 118.740 seconds, with no timeout or GPR writes. Current attempt-02
preparation, collector, Native, transport, decoder, inspection, display and
offline-publication failures are all 0. Earlier worker/Root histories retain
their own original scopes.

## Frozen inputs and retained evidence

Both attempts freeze actual Main
`f2e9f28b9229d69150237075855843aa03e65674`. The controlled capture separately
records observed Main and `origin/main`; external ref observations do not admit
or rebase the capture. Eight named inputs are the BF66EF BODY/BODY_PRIMARY four
documents/reports, `config/target.json` and three CLI sources. Fourteen actual Git
objects and eight LF/index/working bindings were verified. All three CLI files
were physically read immediately before and after each Native command.

Frozen source/include identities are
`764245dc8b8865faea6c9c5ef4403e932b80d58d` and
`5efef0ab7c6b3f0ccb07527b88269ebf4ccf0025`. The earlier ENTRY source/include and
accepted parent BODY source/include identities remain distinct in the report.
No source body or source absence census was read.

The accepted incoming `00BF6707: E8 E7 E7 00 00` has displacement +59,367,
next/encoded return `00BF670C`, target `00C04EF3`. Its accepted physical/manual/
actual records and paired saved call arrays remain bound independently from the
new metadata observations.

The local controlled capture manifest pins 420 files. It is 132,739 bytes,
SHA-256 `04116a2fc6c47e71fc8c8a135e7884b2d29ad43e0e4d53283e475660c8b69761`.
The report retains the full comparison, counts, epochs, project identity,
original queries, manifests and separately scoped failure histories. All raw
artifacts and final handoff remain under the leased packet's ignored `local/`
directory. No accepted archive/helper was rerun. No Ghidra mutation, snapshot
update by this worker, new build, test, probe, game, OS, audio or runtime
experiment occurred. Byte/context/physical work remains held for Root review.
