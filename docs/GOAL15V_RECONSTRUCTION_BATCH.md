# GOAL15V tagged B3 packet 06 integration

Twenty-two complete Source functions close 123 Native bytes and emit 183
Source bytes. Eleven whole entries match exactly (41 bytes); eleven have
explicit qualifications (82 Native / 142 Source bytes). These are complete
functions, including the genuinely empty Native bodies, not unresolved stubs.

Worker pin `daa232aedef032270656f0405a09ba1fc7ca962c` received independent
review with no blockers. The primary checked all entire strict and registered
executable sections, actual symbols at section offset zero, no executable
relocations or undefined providers, fresh live/installed Native bytes, and
the entire actual Core archive member. Source integration `01cc651fc` passed
`scripts/build.ps1` and all three existing checks. No routine tests or runtime
probes were added.

Five three-DWORD zeroers use ordinary C++ memcpy stores. Their actual Source
instructions write four bytes at offsets 0, 4 and 8 in that order, with no
overstore, wider access, alignment requirement, helper or speculative class.
Source XORs EAX after the first store; Native does so before it. Normal memory,
EAX and defined XOR flags agree; a fault at the first store has a different
EAX/flags preimage. All eleven and twenty-three bytes are retained separately.

The byte and DWORD setters expose meaningful new ECX/EDX Source interfaces.
Original uses ECX plus an actual stack byte/word and RET 4. Input read,
argument-stack aliases, cleanup and failed-store register contexts remain
qualified. Four empty bodies emit complete three-byte RET 0 instructions,
with valid-return-stack poststate agreement and different code-fetch extents.
Opaque stdcall words describe cleanup slots that Original never reads.

The three memory decrements retain literal ADD DWORD [ECX+4], -1 followed by
RET. Each entire five-byte Source body matches Native, preserving its actual
read/modify/write and flags. For old word X, CF is X!=0, AF is (X&15)!=0,
and OF is X==80000000h; replacing ADD with DEC would change CF and AF. No
LOCK, atomicity guarantee, invented EDX input or semantic EAX result is added.
The byte-zero bodies retain actual XOR AL,AL and incoming upper EAX.

Saved Ghidra signatures describe Original physical inputs and cleanup;
setter prototypes have exactly one automatic ECX input plus one explicit
stack argument, rather than the Source EDX input or a duplicated receiver.
Old names, prototypes and comments were preserved. Existing tags/evidence
remain; affected exports, typed metadata, snapshot and index were refreshed.

The original 79-candidate screening inventory now has 75 complete Source
functions / 374 Native bytes: 57 exact / 235 bytes and 18 qualified / 139.
Two global bindings, one 85-byte SSE stack copy and one x87 load remain
deferred. This does not prove complete B3-band, caller or game reconstruction.

Worker setup-through-handoff time was about 993.6 seconds; its inferred start
has roughly one-second precision. The warm registered build took 28.861
seconds. Review and primary integration are additional costs; no controlled
throughput speedup is claimed. Primary proof is
`reports/goal15v_reconstruction_batch.json`; preserved inputs/review are under
`local/goal15v_b3_author/` and `local/goal15v_b3_review/`. Original ABI adoption,
fault equivalence for the qualified bodies, and gameplay remain unproved.
