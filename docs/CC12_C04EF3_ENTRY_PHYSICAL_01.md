# C04EF3: conditional physical analysis of the first seven owner bytes

The captured bytes at `00C04EF3` are `83 25 a8 ee 09 01 00`. Under default
32-bit x86 operand and address sizes, they encode `AND dword DS:[0109EEA8], 0`.
The prewritten manual analysis and the first fresh detailed Capstone 5.0.7 decode
agree. This establishes a conditional interpretation of seven bytes, with the
per-address CPU context and live execution state unqueried.

The saved owner spans eight addresses, `00C04EF3..00C04EFA`. Only
`00C04EF3..00C04EF9` was physically analyzed. The next cursor `00C04EFA`, its
remaining owner byte, and end-plus-one `00C04EFB` remain unqueried. This packet
does not admit a complete function body, replacement ABI, runtime behavior, or
the meaning of the saved descriptive function name.

## Physical instruction

| Field | Interpretation |
|---|---|
| Opcode `83` | Group 1 operation with an imm8 sign-extended to operand width |
| ModRM `25` | `mod=00`, `/4=AND`, `r/m=101`: disp32 memory addressing |
| Bytes `a8 ee 09 01` | Little-endian displacement `0109EEA8` |
| Byte `00` | Signed imm8 zero, sign-extended to 32-bit zero |
| Prefixes / SIB | Neither is present |
| Operand / address width | Four bytes / 32 bits under the stated mode condition |

Without a segment override, this addressing form selects DS. `0109EEA8` is an
effective offset; the DS base is unknown, so a flat linear address or a named
global is not authenticated. No memory-target or provider query was made.

On normal completion, the instruction reads the old dword, computes its bitwise
AND with zero, and writes zero. The read remains part of the read-modify-write
semantics despite the constant result. The instruction has no LOCK prefix and
does not establish an atomic read-modify-write operation. It neither uses an
address GPR nor writes a GPR or an encoded stack operand.

The completed result implies `CF=0`, `OF=0`, `SF=0`, `ZF=1`, `PF=1`; AF is
undefined. Capstone reports the generic instruction effects: reset CF/OF,
modify SF/ZF/PF, and undefined AF. Those generic effects agree with the manual's
result-specific values; they are not observations of live EFLAGS.

Successful execution requires the applicable segment and page access, including
write permission. Segment selection, rights or limits may fault; instruction
fetch and the memory access may encounter paging faults. The encoded offset is
four-byte aligned, while actual linear alignment depends on the unqueried DS
base and alignment controls. Result, flags and sequential next EIP are claims
only about normal completion. Fault delivery, handlers, MMIO, interrupts and
debug traps were not modeled or tested.

## Captured metadata and incoming edge

All four exact-entry typed payloads agree in full at count `65052`, epoch `7572`.
They match the accepted entry payload after excluding only the two top-level
epoch fields. Both saved default and effective flow are `FALL_THROUGH`, with
override `NONE`, empty targets and next address `00C04EFA`; this agrees with the
physical instruction's seven-byte sequential cursor. The eight-address saved
owner is metadata and does not establish the remaining byte's instruction.

The accepted incoming edge remains `00BF6707: e8 e7 e7 00 00`: its signed rel32
is `+59367`, its encoded next/return address is `00BF670C`, and its numeric target
is `00C04EF3`. The accepted ENTRY binding and BF66EF BODY_PRIMARY physical record
agree on that edge. Its saved target label and flags remain identity metadata;
the label is not authenticated as the behavior of the target.

## Capture and retained evidence

Root reviewed the complete producer, driver and transport guard, then launched
the capture once in `Root_window_03`. The eight commands were opening count,
two complete typed-entry reads, the conditional paired exact seven-byte reads,
two complete typed-entry reads, and closing count. Both pre-byte typed payloads
had to agree with one another and with the full accepted payload before the
first byte read. No capture command was rerun.

The retained originals contain eight command records, 32 HTTP responses and
16 observations of the three CLI source files, totaling 48 physical source
reads. Opening and closing stdout both report live count `65052`, snapshot
`65052`, `same`. The paired bytes are identical, SHA-256
`9f9dfebe73b681fe3ed11133a57fce4523fbbe7fc480de52d3760622583a96dd`.
The reservation was cooperative; it did not freeze GUI or auto-analysis.

`manual_entry.json` was physically written and pinned before decoder authoring
and before the first fresh detailed Capstone import. Root reviewed the complete
manual, 101-line decoder and 42-line driver, then ran the decoder once. That
execution returned zero with 95 stdout bytes, empty stderr and unchanged pinned
inputs. All eight detailed comparisons passed. The decoder imported no game
Source and performed no live query.

The finite freeze used actual Main and WT commit
`f7fb43633f7082f188a13abe75042d41c29c1e8f`; the separately observed origin/main was
`bda1f7a5b6a517834ab5d3fce08a4c04b46864e2`. Eight core positive inputs and two
supplemental Root batch inputs were bound to ten LF payloads and 16 actual Git
objects. Frozen Source tree identities were recorded without reading Source
bodies; they are not a claim about a later integrated Main or a permanent
baseline.

The 107 selected historical originals preserve the preceding two-command drift
stop, the four-command controlled metadata success, and Root's first-review
readiness-envelope correction. Their command and failure scopes remain separate
from this capture. Unselected historical archives, other Root packet streams,
old helper execution and a Source absence census were excluded.

Current preparation retained two offline inspection process failures and two
display truncations; their original tool returns remain in the conversation,
while full original stdout and input prepins are unavailable. Bounded follow-up
observations are separate evidence. No preparation producer, capture or decoder
execution failed. Pre-execution corrections moved an accidentally authored
driver into the leased local directory and corrected byte-query lowercase
handling. The unexecuted six-command proposal was revised to eight before its
capture prepin; that scope revision was not a failure or an executed attempt.

The companion JSON report contains the complete manual and decoder records,
saved default/effective comparison, accepted incoming binding, original command
records, finite freeze and named private-artifact pins. Local publication
manifest and handoff records bind the two public outputs. No C++ implementation,
Source body, target/provider, listing text, per-address CPU context, test, probe,
game/OS/audio runtime, GPR mutation, ABI claim or gameplay validation was added.
