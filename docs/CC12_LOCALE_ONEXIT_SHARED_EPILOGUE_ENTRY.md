# Locale numeric target entry at C07C45

The accepted caller at `00C11C12` contains `E8 2E 60 FF FF`, targeting
`00C07C45` with physical return address `00C11C17`. This packet establishes
only the target's three-byte entry: `8B 4D F0`, MOV ECX,dword ptr [EBP-0x10].
The load uses current EBP, SS and memory; a caller frame or loaded-value meaning
does not follow from its spelling or the saved name `__SEH_epilog4`.

Two initial exact-entry typed captures and one closing capture agree at
modification 98 in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
All four listing aliases identify the same three-byte InstructionDB unit.
Default/effective flow is FALL_THROUGH to `C07C48`, with no override or
direct target. Four count checks returned 64,730 functions. Complete raw HTTP
JSON, base64, digests and all eleven command stream sets are retained; no
Native collection attempt failed.

Root admitted only the exact entry bytes after independently checking metadata.
Both three-byte reads agree. Independent physical decoding gives opcode `8B`
(MOV r32,r/m32), ModRM `4D`: mod=01, reg=001 (ECX), r/m=101 (EBP with disp8).
Displacement `F0` sign-extends to -16. No SIB or prefix is present, so the EBP
memory operand uses default SS under saved 32-bit addressing. Actual detailed
Capstone 5.0.7 agrees on the destination/source operands and encoding fields.
The single context-zero saved instruction agrees.

On normal completion under saved default-32 operand/address decoding, let B
be EBP at this instruction. ECX receives the dword through current SS at
effective offset (B-16) modulo 2^32. ESP, EBP, other general registers and
flags remain unchanged by this MOV. The read observes memory at this point;
it may alias prior writes, including caller stack writes, if addresses coincide.
No initial-memory snapshot or stable original caller frame is assumed. No
data cell was queried, and the loaded dword has no established pointer,
exception-registration, saved-register, argument, status or ownership meaning.

Saved ownership is one complete 20-address-unit range `C07C45..C07C58`, with
non-thunk and no-return false metadata. The library name `__SEH_epilog4`
is preserved. The remaining 17 bytes `C07C48..C07C58` are unread. Neither
the name nor the saved `undefined ... (void)` signature establishes a frame,
SEH restoration, argument count, return behavior or actual ABI.

Only four prototype prefix lines were displayed and interpreted. The
`--lines 4` CLI response was 281 bytes; its additional 158-byte suffix is
retained opaque. Entire automatic target objects remain opaque. No body
interior, adjacent instruction, callee/provider body or data query occurred.

The finite proof contains six actual frozen Git commit/tree objects and eight
positive files: the immediate 168-byte body and primary doc/report pairs,
target configuration and the three CLI producers. Root gate/receipt and its
prior physical producer are retained under distinct names, along with worker
producer preimages. Both original and physically distinct copy core ZIPs pass
restricted replay with one ZIP read, one pinned pure-helper execution, one
receipt write and zero unexpected audit denials. No earlier archive was traversed.

Final delivery verifies eight unchanged inputs and exactly two owned outputs,
including actual Git objects and working/index/committed bytes. Source subtree
IDs were recorded from this worker's actual frozen revision after prior Source
integration; older packet IDs were not reused. They establish worker revision
identity only, with no current Root equality, Source content read or census,
or implementation credit. No build, new test, probe, original program,
OS/audio, startup or gameplay result is claimed. Ghidra was read only.

Private evidence: `local/cc12_locale_onexit_shared_epilogue_entry/`.
Core SHA-256: `5a7e82744a384fda3fb13c344dedd13a9bc5cf7eee6e7dddc0e196fa5db1e497`.
The report pins raw captures, physical decode, Root gate lineage, preimages
and both restricted replay receipts. Delivery archive and receipts are generated
after this commit and handed off separately; no self-referential digest is claimed.
