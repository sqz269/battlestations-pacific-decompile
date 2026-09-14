# Native post-effect stream geometry (BT)

This packet implements complete admitted bodies at B4D2D0..B4D4B2 (483 bytes)
and B4D4C0..B4D4FA (59 bytes), based on
`32e81952f34ffd0afe2e98afdb146aee50cf4d9b`. Descriptive source names are hypotheses;
the worker proposes two bodies / 542 bytes without changing ledgers or Ghidra.
Existing BO creates the 24h D61EC8 parent with stream+18/count+20 and declaration
`pf44uf42cc.mvfm`. BR's 20h D61EC0 family has different fields and is not used here.

## Writer ABI and ordered accesses

B4D2D0 has no direct or indirect callees. Incoming ECX is unused. Four native
stack DWORDs are output, first output record, record count, and input pointer;
`B4D4B0 RET10h` removes them. ESI and EDI are saved. The source C++ interface adds
explicit pointers to the actual CE3800 and D7A24C cells; it does not claim native
ABI compatibility or semantic meaning for incidental EAX/EDX/XMM return values.

Native `IMUL first,168; ADD output` uses the low DWORD; input is not advanced by
this first-output-record index. TEST/JBE exits only for a zero unsigned count.
For a nonzero count, the code loads the two current constant words once, then
advances input by 36 and output by 168 per iteration. The captured original
cells contain 3F000000 (0.5f) and 3F800000 (1.0f). No default constants, clamping,
allocation, resource retention or floating arithmetic are introduced.

For disjoint stable input, the nine DWORDs at input offsets 00..20 produce the
following six vertices of stride28. Position is four words, followed by two
coordinate words and one late-captured color word. Field names are descriptive
layout hypotheses consistent with the producer declaration; the API uses raw
storage instead of inventing an object type.

| Vertex | Position words x,y,z,w | Coordinate words u,v |
| --- | --- | --- |
| 0 | input00,input0C,CE3800,D7A24C | input10,input1C |
| 1 | input00,duplicate vertex4 y/z/w | input10,duplicate vertex4 v |
| 2 | input08,duplicate vertex3 y/z/w | input18,duplicate vertex3 v |
| 3 | input08,input0C,CE3800,D7A24C | input18,input1C |
| 4 | input00,input04,CE3800,D7A24C | input10,input14 |
| 5 | input08,input04,CE3800,D7A24C | input18,input14 |

MOVSS transfers words without floating conversion. The eight duplicate y/z/w/v
operations use FLD followed by FSTP, loading the **current destination** written
earlier. These conversions can quiet signaling NaNs and set x87 status flags.
The last input20 color word is loaded only at B4D484, after all position and
coordinate accesses, then written in reverse vertex order 5,4,3,2,1,0. This table
is not an alternative implementation for overlapping storage: the source keeps
the full native sequence in one assembly block. Later input reads can observe
earlier output writes, including writes into the next input record.

SUB EDI,1 at B4D36B sets the loop flags before many remaining stores. None of
those operations changes EFLAGS; B4D4A8 JNZ uses that subtraction. The saved
listing has 89 instructions and one eight-byte gap B4D308..30F. An unconditional
jump skips the gap; disk/live bytes are `LEA ESP,[ESP]; NOP` alignment. No listing
repair, flag change or inferred execution of those bytes is necessary.

The caller supplies valid accessed storage and a free x87 stack slot. The admitted
domain uses nontrapping controls. The writer changes neither x87 CW nor MXCSR and
preserves the original load/store conversions. Hardware fault locations, private
compiler frame timing, unmasked FP trap routing and native FH3 are not established.
Zero records dereference neither input/output storage nor the two constant cells.

## Wrapper and real mapping providers

B4D4C0 saves ESI and captures ECX as receiver. It has **one stacked input pointer**,
removed by RET4 at B4D4F8. The input is not zero: B4D4D8 loads it from the stack,
and B4D4DF pushes it as the writer's fourth argument. The complete schedule is:

1. B4D4C3 reads current receiver+18, then B4D4C6 reads current +20. Current stream
   profile/slot+10 selects B49980. Map `(count*3 modulo DWORD,0,0)` at B4D4D6.
2. B4D4DC rereads +20 **after map**. B4D4E8 writes `(mapped,0,current_count>>1,input)`.
3. B4D4ED rereads +18 **after writing**. Current profile/slot+14 selects B49A80;
   B4D4F5 unmaps that current stream, which need not equal the earlier capture.

The D61D6C table contains B49980/B49A80 in those slots. Source dispatches directly
to the already reconstructed `NativeLogicalBufferMappingContext` providers;
numeric native entries are checked selectors, never host function pointers.
Mapping uses current native renderer synchronization, physical profile selection,
DWORD byte products and actual cached+08 behavior. Unlock reloads physical after
guard entry and clears current logical+08 only after physical Unlock returns.
The wrapper adds no automatic unlock, rollback, retain or fallback if an existing
provider throws. An odd unchanged parent count maps three extra vertices which
the writer leaves untouched; a changed count after map is honored without a clamp.
Valid mapped extent remains a caller/provider prerequisite.

All four observed wrapper calls occur in B529A0. EBP is captured ECX at B529BC
and is not rewritten before its final POP. Calls use receiver/input pairs:
B53FD1 `(parent+30 pointer,parent+24 pointer)`;
B54004 `(parent+68 pointer,address parent+44)`;
B54028 `(parent+88 pointer,parent+7C pointer)`;
B54068 `(parent+B0 pointer,parent+A4 pointer)`.
The native caller's CCh lifetime and the providers establish these as 24h children.
The larger B529A0 update body and rendering submission remain separate source work.

## Evidence and limits

The report pins source/provider files, original PE/live code/data spans, exact
build inputs and artifacts, native call sites and ABI contracts. Generated-code
review compares the writer's entire 414-byte loop against native bytes and checks
parameter setup, count-zero branch, constant capture, wrapper reloads and direct
provider calls. Full Win32 build runs only existing configured tests.

One small ignored writer differential trajectory checks two overlapping records,
a wrapping nonzero first-output index, signed-zero/signaling-NaN/subnormal input,
all 768 bytes per path, x87 CW/status and MXCSR, and untouched guards. The native
code page and shared constant page are mapped before running, made read-only or
executable as appropriate and verified unchanged afterward. Only the two absolute constant
operands are rebased before execution; relative flow and the complete loop stay
unchanged. The manifest-bearing probe links
the three current libraries; source/build stamps and before/after fixture pins
guard the run. Exact outcomes and any failed attempts are recorded in the report.

The probe does not execute the wrapper's renderer mapping path, test provider
failure trajectories, establish unmasked floating exceptions or native FH3,
replace the binary ABI, or validate GPU output/gameplay. Mapping context identity,
physical provider/control-transition prerequisites and integration with B529A0
remain explicit. No Ghidra or ledger mutations were made by this worker.
