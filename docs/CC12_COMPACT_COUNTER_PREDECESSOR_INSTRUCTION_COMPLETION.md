# Compact counter predecessor instruction completion

The complete six-byte instruction at `00CD8674..00CD8679` is
`MOV DWORD PTR [01090434], ECX`. It stores the incoming ECX value to that absolute
address, then has ordinary fallthrough to `00CD867A`. This establishes one local
write; it does not identify the source of ECX, the destination's semantic role,
an initializer, or the original function owner/ABI.

Baseline: `51dfcc2c957e09abae8346a6f3b88494a6b44177`, after Root integrated the
existing Compact Source fragment. The only tracked changes are this document
and `reports/cc12_compact_counter_predecessor_instruction_completion.json`.
Complete receipts are under
`local/cc12_compact_counter_predecessor_instruction_completion/`.

## Exact gate and physical scope

The first lease covered only `[00CD8674,00CD867A)` and the new output files.
Fresh typed metadata at `00CD8674` establishes an exact/containing six-byte
`InstructionDB`; exact and containing functions are null and complete function
records are empty. Default and effective flow are both `FALL_THROUGH` to
`00CD867A`, override `NONE`, no fallthrough override, and no target arrays or
direct-call records.

Root's coordination clarification excluded a new query at `00CD867A`. Its
earlier epoch-10 metadata is retained only as historical context. No fresh
instruction, bytes, prototype or body at that fallthrough address were opened.
No nearest-function inference or listing repair was used.

The exact six original/live bytes are `890d34040901`, SHA-256
`33702649eac3c6e7bf2ccf46bd7c14a026a9082dac4578a35205b7f92d3e87e8`.
Capstone x86-32 decodes exactly one complete instruction from the verified start;
its absolute memory operand has width four and its source register is ECX.

The disk read reused the retained `.text` mapping in the Compact predecessor
report: `00CD8679` maps to offset 9,275,001, so `00CD8674` maps to 9,274,996.
Only one unbuffered semantic seek/read of six bytes was made. No new PE headers
were parsed and no other semantic window was opened.

Whole-file identity was freshly hashed without interpreting additional bytes:
12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The original size and modification timestamp match the retained identity and
remain stable across hashing and the selected read. The new instruction's last
byte at `00CD8679`, `01`, equals the retained first byte of the prior approved
15-byte prefix. That earlier tail byte is now included in a complete decode;
the remainder of the old prefix was not reopened or decoded here.

## Newly named destination: metadata only

After decoding the explicit destination, the lease was extended to `01090434`
for typed metadata only. The response identifies a one-byte `DataDB` code unit
at and containing the exact address, with `defined=false`. Instruction,
instruction-flow, exact/containing function, and direct-call metadata are null
or empty as appropriate. No destination bytes or value were read.

The instruction's four-byte access does not establish a defined four-byte data
item, object extent, descriptor field, parent token, guard, name, or initializer
role. No adjacent cells, references, data structures, or inferred targets were
opened. The incoming ECX producer remains unknown in this packet.

## Epoch, historical context, and Source separation

Three fresh typed responses pass strict offline validation: initial `00CD8674`,
then `00CD8674` and newly leased `01090434` after the physical observation.
All before/after and batch modification numbers are `10`; duplicate instruction
payloads are identical. Actual `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, `x86:LE:32:default`, image base `00400000`, and
`ram` space are checked. The byte command retains five full raw HTTP responses
before parsing, including identity before/after. All eight new raw response
hashes replay. This is stable database evidence, not runtime game memory.
Java CodeSource remains unattested.

The separately retained `00CD867A` epoch-10 response records a call to
`006FAC20`. It is explicitly historical and is replayed separately from the
three fresh responses. Current metadata at `00CD8674` establishes fallthrough
to that address; it does not freshly validate the historical call or its callee.
The selected instruction can precede the retained call on normal continuation,
but this adds no whole-function, register-argument, or callee ABI contract.

The existing ordinary Compact Source fragment still covers only
`00CD867A..00CD868D` and remains unowned. Its complete unchanged header,
implementation and report/document are pinned against this baseline. The newly
completed preceding store is not silently incorporated into that Source function,
and no additional Source parameter, descriptor role, storage, or guard is invented.
Prior camera, SkinModel, Compact boundary, and Compact Source bundles remain
unchanged.

No C++/CMake/ledger/provider/configuration edits, GPR mutation, POST/script,
restart, further Native window, build, test, probe, link or runtime execution
occurred. Owner/entry/extent, ECX provenance, destination role/layout/backing,
guard/name/parents, complete initializer, callers/CRT, Native ABI/EH, startup,
and game behavior remain held. No further Native scope is proposed here.

## Primary review

Root replayed all85 report-pin occurrences and18 baseline Git/current files, all three fresh strict typed responses at historical epoch10, the separately classified one historical fallthrough response, and five physical HTTP hashes. The current original PE whole hash matches and the same six-byte instruction reproduces MOV[01090434],ECX; its final byte agrees with the earlier retained tail. All52 archive payloads,53 ZIP entries and CRCs pass. No new live query or Native semantic window was opened.

Only the local store is accepted. The target remains a one-byte undefined Ghidra data unit despite the instruction's four-byte access. ECX provenance, target role/extent, owner, initializer, ABI and startup remain held. Existing Compact Source stays unchanged; this review admits no Source extension or descriptor role.
