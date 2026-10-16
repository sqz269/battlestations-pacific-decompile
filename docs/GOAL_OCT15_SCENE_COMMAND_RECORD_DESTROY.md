# Raw command-record destruction

Addresses: 00469010, 00C61A50, 00469B00, 0046A090.

This packet supplies whole ordinary raw-storage control flow for the three
normal routines and a separately qualified cleanup-action mapping. It uses
the actual raw pool publications, gate and manager context and current CRT
free. There is no Host, callback, missing-call substitute or new owning record
type. The command-record name is a layout hypothesis supported by the existing
producer documentation; owner0 is never read or reset by these routines.

| Routine | Coverage | Observed native ABI |
| --- | --- | --- |
| 00469010 | Whole119B/38 instructions | ECX record, unusedEDX, plainRET/semanticvoid |
| 00C61A50 | Qualified whole11B/3 instructions | CURRENT[EBP-10]+4, tail cleanup |
| 00469B00 | Whole30B/11 instructions | ECX record, unusedEDX, LIVE flags4, RET4/EAX captured record |
| 0046A090 | Whole35B/15 instructions | ECX pointer-cell, unusedEDX, plainRET/semanticvoid |

The total is195 bytes/67 instructions, excluding adjacent alignment and the
unowned FH3 handler. Older Seg6Host projections remain separate; neither the
deleting wrapper nor its compiler name is replaced with a recovered-symbol
claim. Their correct native names and existing comments are preserved.

The core writes the explicit saved-this4 cell for native PUSH ECX and its
following saved-this store. It captures current record+10 data, then arms
state0 even if that data is null. Nonnull data causes a current record+C length
read, DWORD length+1 wrap, the actual no-argument pool getter, then actual
return-block with the captured data and size. The native mode1 stack argument
is unused by the return provider; that provider's real shutdown gate remains
live and is read only where its own body requires it. Resolving the getter
still occurs for every nonnull block, including large and disabled returns.

After first release returns, the core freshly reads current record+8 data,
disarms cleanup to state-1, and only if nonnull reads current record+4 length
and repeats the getter/return sequence with DWORD length+1. It never clears
either header, repairs their contents or reloads captured data/size after the
getter. Changes made by a provider remain visible to subsequent fresh reads.
No positive-size, alignment, empty-header or extra ownership guard is added.

The scalar deleting wrapper captures the original record, invokes the real
core, then reads only the CURRENT flags BYTE and tests mask01, **bit0**. A set
bit frees the captured record through the existing current CRT provider. It
returns that raw captured value even after freeing it, with no live-object or
ownership promise. No flag snapshot or higher-byte read is introduced.

The pointer helper captures the actual slot and its record once. A null record
skips every child/free/write. Otherwise it invokes the core, frees that same
captured record, then zeros that same captured slot. It does not reread a
pointee or substitute an owner address after a returning provider. The reached
final write must be valid under the supplied actual storage/CRT contract.

| Native transfer | Role |
| --- | --- |
| 00469046 CALL 00419CC0 | First actual getter |
| 0046904D CALL 00BD1510 | First return-block |
| 0046906B CALL 00419CC0 | Second actual getter |
| 00469072 CALL 00BD1510 | Second return-block |
| 00C61A56 JMP 0041DD20 | Qualified action |
| 00469B03 CALL 00469010 | Scalar child |
| 00469B10 CALL 00BF65AC | Scalar free |
| 0046A09C CALL 00469010 | Pointer child |
| 0046A0A2 CALL 00BF65AC | Pointer free |
| 0046AA0C CALL 00469010 | External list-clear incoming call |

The getter has no native stack arguments: both groups of three pushes belong
to the following return-block, which returns with RET0Ch. Scalar flags are the
single enclosing argument consumed by RET4. Both wrapper frees have one push
and ADD ESP,4. All three core incoming setups were read. Live xrefs found no
references to either wrapper; no indirect caller or application adoption is
inferred. The external list-clear body and the producer are not owned here.

The genuine handler C61A5B selects FuncInfo D89068, whose magic19930522,
maxstate1 and D89060 unwind map select C61A50 for state0→-1. The action rereads
CURRENT explicit saved-this4, adds4 using wrapping x86 word arithmetic and
uses the real raw header destructor. Only that current command header is
destroyed; no record/owner reset or invented free occurs. The Source guard
supplies ordinary C++ cleanup. Original FH3/SEH state/private-frame identity,
secondary cleanup exceptions, fault transport and historical heap behavior
are not proved by this mapping.

A C++ exception while the first release is armed invokes the action using the
current saved-this cell. Once disarmed, a second-getter exception escapes
without that action. If cleanup itself throws during C++ unwinding, ordinary
C++ termination behavior applies. This guard does not promise cleanup for
Win32/SEH faults under the current build or Original FH3 secondary-throw rules.
The emitted core's Boolean guard arm follows the target-data capture and its
disarm follows the fresh command-data capture. A separate compiler-private EH
state write occurs before the first data read; it has no Original private-state
or fault-transport credit. Raw capture, length, provider and publication order
matches the Source contract.

All new APIs explicitly borrow the saved-this backing. The scalar frame also
borrows actual flags4 backing; only its first byte is read, after child return.
Binding metadata and context objects stay stable and disjoint from every
reached raw access. The actual cells, pool publications/gate and record fields
retain the stated aliases and live schedules. Implicit native compiler,
private-unwind and outgoing-argument homes are not supplied or credited.

Existing complete raw getter/return/header-destroy Source and their actual
storage contracts are reused. The raw getter has the published current
__finally composition; current free is the existing concrete std::free
binding. This packet adds no clone/duplicate graph/text domain. Each reached
provider's actual allocation, lifetime, lock and storage requirements remain
in force. Original heap, binary ABI, virtual dispatch and game equivalence
remain unproved; these are new ordinary C++ interfaces.

Guarded analysis checks C:/Users/sqz269/bsp.gpr and program
/battlestationspacific.exe. The report retains full live/installed bytes,
typed transfer rows, actual provider hashes and prior annotation values.
Locked annotation/save/export/readback and primary complete-Source review
passed. The primary accepted draft5d6eceae9 with unchanged Source hashes and
registered it once at efa7febae744d66c5665e61b64bf8c67d36ff3f6. Its normal
scripts/build.ps1 MSVC Release Win32 gate passed all three existing checks
in 9.55 seconds. The report pins the exact primary receipt/log hashes and
unique archived object. All four emitted Source functions were inspected
for their raw capture/publication schedules; provider hashes are unchanged.
Three ordinary APIs are Source-ready, with the cleanup action separately
qualified. No worker build, new tests, native entry execution, Original
heap/FH3/private ABI, application adoption or game validation is claimed.
