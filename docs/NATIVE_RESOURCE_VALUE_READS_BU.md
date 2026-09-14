# Actual resource value reads BU

Addresses: BE4360, BF02C0, BE99D0, B932E0, B93310, B936E0, BE9A00,
BE9FE0, BEA010. Source: `native_resource_value_reads`.

These nine complete ordinary bodies total 318 bytes. They supply the raw
float, DWORD and string readers needed by B7EB90 hierarchy fields, B7F430 root
bounds and the resource parsers. The existing typed full-read abstractions are
separate implementations with stricter policies; no typed object is cast into
raw node/reader storage here.

| Address | Bytes | Original ABI | Operation |
|---|---:|---|---|
| BE4360 | 26 | ECX stream; stack optional actual; ST0; RET4 | Pointer-seeded read4, FLD32 |
| BF02C0 | 39 | ECX reader; stack budget; ST0; RET4 | Current slot44, FSTP32/FLD32, actual debit |
| BE99D0 | 15 | ECX handle; ST0; RET | Current node/reader/budget float adapter |
| B932E0 | 46 | ECX handle; EDX output; RET | Four sequential float32 stores |
| B93310 | 66 | ECX handle; EDX output; RET | Six sequential float32 stores |
| B936E0 | 48 | ECX handle; EDX output; RET | Sixteen sequential float32 stores |
| BE9A00 | 15 | ECX handle; EAX DWORD; RET | BF0280 node budget adapter |
| BE9FE0 | 34 | ECX node; stack output; EAX output; RET4 | BF0510 string adapter |
| BEA010 | 29 | ECX handle; stack output; EAX output; RET4 | Dereference node then BE9FE0 |

BE4360's incoming optional actual pointer is also the initial contents of its
four-byte read buffer. A short transfer preserves the unwritten pointer bytes;
null actual starts that buffer at zero. It captures the stream's current
slot24 and reads once, then loads the resulting float32 through x87. BF02C0
captures the reader's current stream and slot44. Its local actual-count word
is uninitialized before that call. The known BE4360 target delegates to the
existing concrete raw stream services, which report actual bytes. A different
reached float target throws as an unresolved binding. There is no fabricated
zero initialization, full-transfer check, retry or pre-read budget limit.

BF02C0 stores the returned ST0 to a float32 local, reloads it, then subtracts
the actual count from the current budget DWORD with wrapping. BE99D0 captures
the handle's current node, its budget address and reader for one call. The
sphere/bounds/matrix readers capture the handle address and output address,
but dereference that handle anew for every scalar call. Each completed read
is stored before starting the next. Values are neither sorted nor normalized;
the matrix is not transposed or expanded and has no default identity. These
properties remain observable when an output store overwrites the handle.

Small assembly bridges retain the FLD and FSTP/FLD boundaries without adding
C++ float temporaries between them. Integer-only C++ code selects the borrowed
stream and raw read service. The source uses its own calling interface and
stack slots. General registers/EFLAGS, original stack addresses and arbitrary
aliases into those private slots are not original binary ABI guarantees.
Unmasked exceptions, x87 instruction/data pointers and asynchronous faults
remain outside the validated domain. The source does not change the caller's
x87 control word or clear its status as part of a read.

BE9A00 forwards the current node reader and budget to the existing BF0280
DWORD reader. BE9FE0 forwards node reader, captured output and node budget to
BF0510, ignoring its return and returning captured output. BEA010 captures the
current node and output, calls BE9FE0, then returns captured output. Their
zeroed unused native stack words are not output-header initialization and are
not the BF0510 actual-count slot. The BS string lifetime rules remain in force.

## Evidence and validation

Every ordinary byte is compared with the preserved installed PE and every
instruction start with current Ghidra function ownership. No function creation,
flow repair or native no-return change is needed for this packet. Each routine
receives an ABI analysis view and an appended evidence comment; older comments
and correct names are retained and exports refreshed.

One focused probe uses the existing controlled-child bootstrap, actual memory
streams, raw string pool and source reference release. Six complete original
float bodies are copied into private executable allocations. Only their 12
known relative-call displacements are relocated. The original stream virtual
table uses a fixture ABI adapter to the same reconstructed memory read service;
the source follows the actual D642C0 numeric table through its existing binding.
The original installation and its code/data are never modified or executed as
a game. This is a bounded relocated-body oracle, not a whole native process.

The probe passes 612 paired cases: 16 scalar patterns for each of the three
scalar layers, and one sphere/bounds/matrix sequence, across four rounding
directions and three x87 precision modes. Patterns include signed zero,
finite values, infinities, quiet/signaling NaNs and subnormals. It compares
output bits, reported-byte budgets, read counts and masked x87 exception-status
bits; it also checks the control word and empty x87 stack after each call.
It does not compare all condition bits, FIP/FDP or unmasked trap behavior.

Sixty short-read cases cover zero through four supplied bytes at all six
layers in both source and original modes. Each read verifies its buffer began
with that call's actual pointer bits; expected stored values are normalized
against the observed seed for that invocation because the two stacks have
different addresses. This is seed-aware short-read evidence, not a claim that
pointer-derived output bits match across different stack placements.

Additional source checks exercise output/handle aliasing (subsequent scalar
calls use the newly stored node), an exception after the second actual read
(only the first output/budget debit commits), and the DWORD/string wrappers.
Stream allocation counters finish at zero. The native exception mechanism,
unmasked x87 faults, other stream families in this fixture and gameplay remain
untested. Raw hierarchy/item/root dispatch, manager/factory/parser construction,
B80720 loading and queue worker shutdown remain open.
