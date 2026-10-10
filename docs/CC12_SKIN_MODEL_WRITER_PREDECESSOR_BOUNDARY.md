# SkinModel writer predecessor boundary

This bounded inspection resolves the immediate ECX provenance for the already
retained six-byte store at `00CD8524`. It does not identify an initializer entry,
owning function, descriptor layout, backing owner, or original ABI.

Baseline: `5a2027ce606ec678de0b6a31401bea87e0c3aac9`. The only tracked changes
are this document and `reports/cc12_skin_model_writer_predecessor_boundary.json`.
The complete raw receipts and pinned Source copies are retained under
`local/cc12_skin_model_writer_predecessor_boundary/`.

## Boundary gate and exact physical scope

Root authorized typed metadata at `00CD8523` first, then a physical inspection
strictly within `[00CD8515,00CD8524)` after usable instruction boundaries and a
range lease were established. The initial typed response places `00CD8523`
inside the three-byte instruction beginning at `00CD8521`. No containing
function is defined there. The lease was then extended to the approved window.

Metadata for every address in that window identifies four complete instruction
starts: `00CD8516` (5 bytes), `00CD851B` (3), `00CD851E` (3), and `00CD8521` (3).
At `00CD8515`, `instruction_at` is null; `instruction_containing` starts at
`00CD8510` and has length six. That start was returned as metadata only.
No byte before `00CD8515` was opened. The byte at `00CD8515` was compared but
excluded from decoding; its containing instruction begins outside the envelope.

The exact 15 original-file bytes equal Ghidra's exact 15 returned bytes:
`01e80527a2ff8b48048d5101895004`. Their SHA-256 is
`cb554ae7c2c8215cdf0d46c6f29a694a19dbffb5dab82f70d330a2a8c06dd71b`.
Only the 14-byte suffix `[00CD8516,00CD8524)` was decoded, each instruction
separately from its verified start and length using Capstone x86-32.

The PE read used one unbuffered seek/read of exactly 15 bytes at file offset
9,274,645, derived from the nearby retained camera audit's `.text` mapping
(`00CD8380` -> 9,274,240). Historical writer evidence independently identifies
`00CD8524` as `.text`. PE headers and the whole-image hash were not reread.
The current file size is 12,223,752 bytes and its stat remained unchanged across
the read. The historical image hash is retained as historical identity, not a
fresh whole-file verification. Exact selected-byte equality is current evidence.

## Local behavior

| Address | Bytes | Observed instruction |
| --- | --- | --- |
| `00CD8516` | 5 | `call 006FAC20` |
| `00CD851B` | 3 | `mov ecx, dword ptr [eax+4]` |
| `00CD851E` | 3 | `lea edx, [ecx+1]` |
| `00CD8521` | 3 | `mov dword ptr [eax+4], edx` |

On normal continuation from the call, let `p` be the EAX value and `n` the
32-bit value read at `p+4`. The sequence leaves ECX equal to `n`, calculates
EDX as `n+1` modulo 2^32, and writes that incremented value to `p+4`.
The previously retained `00CD8524` store then writes ECX to `01090344`.
The increment publication therefore precedes the known SkinModel publication,
which receives the old value. Neither LEA nor the final MOV changes ECX.

This describes the selected instruction sequence conditional on control reaching
it and the call/accesses completing. It establishes no guard, initialization
frequency, failure path, exception mapping, synchronization, or lifetime.
The identity, extent and validity of `p` remain unresolved in this packet.
Disjointness of `p+4` and `01090344` is also unproved; if they alias, the later
known store overwrites the increment with the old value.

The call has default and effective `UNCONDITIONAL_CALL` flow to `006FAC20`,
fallthrough `00CD851B`, override `NONE`, and no fallthrough override. Its
automatically included target metadata records the current descriptive name
`BSP_TypeId_GetCounterSingleton`, `no_return=false`, and `is_thunk=false` with
no direct thunk target. No callee body, bytes, prototype or data were opened.
The other three selected instructions have matching default/effective
`FALL_THROUGH`, override `NONE`, and no target arrays; the last falls through to
`00CD8524`. These records are not proof of runtime return behavior or a callee ABI.

All queried addresses have null exact and containing functions. Thus the call
and subsequent straight-line suffix do not establish an owned Native function
or its entry. The leading partial instruction does not establish an entry
either. The known six-byte writer was refreshed as typed metadata only; its
physical bytes and original decoding were reused from the prior packet.

## Current Source context and evidence limits

Ten complete existing Source files were pinned against baseline Git and copied.
`TypeIdCounterStorage` has an existing `next_id_04` member at offset four;
`TypeIdCounterLifetime::get_006fac20()` is the existing Source provider, bound
through its supplied `0109DB7C` slot and lifetime access. That Source contract
is consistent with this local arithmetic. It does not turn the call-site
observation into fresh Native callee, backing, ABI, or production-binding proof.

The existing SkinModel getter and resource/point-light contexts still consume a
borrowed `01090344` cell. The retained type-storage and VFS/resource application
files supply no newly admitted SkinModel publisher. No C++ or backing was added.
SkinModel `01090344`, skined-mesh `01090454`, excluded `01090370`, camera resource
`01090288`, and compact `0109042C` remain distinct families.

All 24 complete raw typed responses pass the current strict validator with
actual `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86-32 language,
image base `00400000`, and `ram` space. Before/after response and batch
modification numbers are all `5`; duplicate payloads across the three captures
are identical. The read-memory command retained its five full raw HTTP
responses before parsing, including target verification before and after.
Typed metadata after the physical read closes the same modification envelope.
This is database-observation stability, not a running-game memory observation.
Runtime Java CodeSource remains unattested.

While extracting the old VA/file-offset mapping, the already retained camera
audit's historical 112-byte hex was inadvertently displayed. This was promptly
reported to Root. It caused no new Native read, query, decode, or validation
credit. Prior camera and SkinModel artifacts were not edited; tracked inputs
match the approved baseline and the recorded local bundle hashes remain intact.

No POST, script endpoint, Ghidra mutation, body/prototype repair, restart,
configuration change, Native callee/data/table/handler opening, C++/CMake/ledger
edit, build, test, probe, or runtime execution occurred.

## Remaining gate

The immediate incoming ECX question is resolved for this selected suffix.
A whole initializer or Source publisher remains gated on owning extent/entry,
guard/name/parent stores, backing and lifetime, callee ABI, partial-state paths,
callers/CRT ownership, and production composition.

If Root later chooses to complete the immediately preceding instruction, the
smallest additional physical proposal is `[00CD8510,00CD8515)` (five bytes),
combined with the already retained byte at `00CD8515`, after fresh typed
boundary/identity verification and an exact new lease. This is an optional
inspection proposal, not a function-entry claim or authorization. It was not
performed and would not by itself establish initializer ownership.

## Primary review

Root retained 91 pin occurrences, verified ten baseline/current Source files, replayed all 24 typed raw responses at historical modification 5 and all five physical/identity HTTP hashes, and verified all 60 ZIP payload hashes. Root compared the exact 15 original bytes and decoded only the verified 14-byte suffix. The original whole-image hash also verifies currently. No live Ghidra query or mutation was added. Later parent annotations do not refresh these historical metadata captures.

A separately owned ordinary Source publication fragment may cover the observed counter-call/increment suffix plus the previously verified six-byte store, exactly CD8516..CD8529 (20 bytes). It can borrow the genuine existing counter and actual current 01090344 word, preserve both volatile stores even if they alias, and supply no guard, initializer entry, owning function, backing or caller. This narrow interface would receive only ordinary Source credit; whole initializer, Native ABI/EH/fault and production/CRT gates remain held.
