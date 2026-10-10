# Application singleton handler: bounded listing contract

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** The admitted handler prefix at `00CC7240`
is exactly two instructions: load `00E01CC4` into EAX, then jump to `00BF6B43`.
Neither address was followed. The prefix supplies a candidate descriptor and
an external transfer target; it does not establish the descriptor's format,
the target's ABI, an actual FH3 frame, or any unwind action.

The saved listing has a concrete flow discrepancy: the second instruction is
an encoded near JMP, but Ghidra's `CALL_RETURN` override changes its effective
flow to `CALL_TERMINATOR`. This packet preserves that observation without
repairing the override or inventing a function around the unowned instructions.

## Source and Native epochs

Source was frozen after fast-forwarding the isolated worker to Main
`281e91b6d5fa9dba441e5ec60b14874e4b9c90d6`. The finite proof retains all **4,210**
tracked src/include files as actual Git blobs and working bytes, plus 18
supporting Source/build-tool/document/report files. Four Source paths differ
from the preceding base packet's `8cf81aaf571bcd8f452990161826cfefbb9cfba1`:

* `include/bsp/native_locale_map_node_recycling.hpp`
* `src/native_locale_map_node_recycling.cpp`
* `include/bsp/native_particle_model_lifetime.hpp`
* `src/native_particle_model_lifetime.cpp`

These already integrated implementations and their build results have separate
provenance. This packet adds no implementation/build credit. Its prior base
proof's 230 declared pins are copied unchanged and replayed, preserving nested
184 constructor pins, 124 authority pins, seven historical selected Source/build
epochs, and earlier full Source corpora. The prior normal 145-byte singleton-base
contract remains Native epoch **79**. Root's subsequent comment-only acceptance
is retained as its own committed document/report, not a new worker body capture.

Fresh typed captures here identify project `bsp`, marker
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`, modification
**85**, and 64,730 functions. The per-instruction before captures and combined
after capture match. CLI project/program verification precedes each live query.
No project, program, body, listing, name, comment, prototype, no-return or flow
property was changed by this worker.

## Instruction admission and exact bytes

Prototype/typed metadata again found no function at or containing `CC7240`.
The existing instruction at that exact address has length five. Its exact range
was leased before querying context zero and those five bytes. Typed metadata
identified `CC7245` as both default and effective inline successor. That next
start was claimed and independently verified as a five-byte instruction before
its exact range was leased and read. It also has no function ownership.

| Start and leased range | Exact bytes | Raw instruction |
| --- | --- | --- |
| `00CC7240..00CC7244` | `B8 C4 1C E0 00` | `MOV EAX,00E01CC4` |
| `00CC7245..00CC7249` | `E9 F9 F8 F2 FF` | `JMP 00BF6B43` |

The ten concatenated bytes have SHA-256
`47ce0b872dfcf6b83806b53a2341e0f8b587c13167cc90b8535a7fa735f12909`.
Capstone independently decodes the exact bytes; retained context-zero listing
responses match each instruction and contain no surrounding instructions.
The signed E9 displacement from `CC724A` resolves to `BF6B43`.

The walk stops at the second instruction: neither default nor effective flow
has an inline fallthrough, and effective flow is terminal. Only **two starts /
ten bytes** were consumed, below the authorized eight-start / 64-byte limits.
`CC724A` was not queried. The end of these two contiguous instruction ranges is
not asserted to be a recovered function boundary. No gap, padding, neighboring
body, outside JMP target, descriptor or cleanup action was inspected.

## Flow and register contract

At `CC7240`, default and effective flow are `FALL_THROUGH` to `CC7245`, with
override `NONE`. At `CC7245`, the complete typed record distinguishes:

| Property | Default instruction semantics | Saved effective listing semantics |
| --- | --- | --- |
| Flow name | `UNCONDITIONAL_JUMP` | `CALL_TERMINATOR` |
| Is jump / call / terminal | true / false / false | false / true / true |
| Target | `00BF6B43` | `00BF6B43` |
| Inline fallthrough | None | None |

The saved override is `CALL_RETURN`; the fallthrough itself is not overridden.
The instruction-context text and exact E9 opcode remain JMP. Effective call
metadata is therefore not evidence of a machine CALL, a pushed return address,
or a native RET. The prefix is retained as a listing/flow defect handoff only;
this evidence does not authorize analysis repair.

For these two instructions alone, EAX is overwritten with the immediate and
EIP transfers to the encoded target. They do not access/change the stack, write
other general registers or alter flags. EAX is an outgoing transfer value, not
a demonstrated caller return. The destination's parameter registers, incoming
exception/establisher-frame arguments, preserved-register rules, return behavior
and fault handling were not read or executed. Assigning a callable prototype
or claiming original FH3 invocation would exceed this prefix.

## Failure and Source boundaries

The immediate/transfer shape is consistent with a compiler exception-dispatch
stub; calling `E01CC4` a descriptor is provisional until its data is inspected.
No type, magic value, state count, map pointer, cleanup target or destructor is
recovered from these ten bytes. In particular, the earlier base's state 0/1
spills still identify reached stores rather than post-unwind outcomes.

Eight complete Source queries and 14 bounded excerpts replay against the frozen
corpus. There are no hits for these handler/descriptor/target addresses or the
queried explicit FH3 ABI-definition terms. The four Application-authority-name
hits remain comments, with no `D68BC4`, `D68BC8` or `CFEAB0` deletion profile.
Positive raw manager getter, actual guard release and registration providers
remain present; their availability does not connect this handler to a cleanup
action or create a raw Application/E1AE90 authority.

The preceding base contract remains separate: original receiver publication
precedes second manager lookup and current-publication registration; normal
release uses the captured section, and EAX returns the original receiver.
Actual raw-owner storage, its +14 reference, publication/registry identities and
borrowed services must survive unresolved escaped references, startup/shutdown
and retained failure records. No reset, unregister, free, retry or destructor
policy follows from this prefix. Callable profiles and normal retirement remain
open. `E1AE78` process-duplicate ownership remains a separate gate.

## Finite proof and smallest next scope

The proof replays current Source/Git closure, all eight searches and 14 excerpts,
the unchanged preceding proof, per-start metadata, capture ordering, both exact
instructions and the flow discrepancy. Physical relocation copies only declared
files and report; guarded replay permits reads only from that exact set and no
writes, subprocesses or network. A separate actual-Git check verifies current
and preceding base Source/support blobs and the exact two committed files.
No C++/CMake/GPR/ledger edit, test, build, probe, Native/game/OS-provider call,
binary ABI result, actual unwind invocation or activation is added.

Next proposed packet: `cc12_application_singleton_descriptor_prefix_readiness`.
After a mapped-data/metadata gate, admit only the exact **12-byte candidate data
prefix `00E01CC4..00E01CCF`**, the first three DWORDs at the directly encoded
address. Preserve raw values first; no descriptor type or field semantics are
pre-accepted. This is a concrete proposal, not permission to read it now.
Any larger descriptor, derived EH table, cleanup body, destructor or `BF6B43`
target needs its own explicit finite scope before a fresh read. No function
creation or flow repair is part of that proposal.

Evidence: `reports/cc12_application_singleton_handler_listing_contract_readiness.json`
and its declared local finite bundle.
