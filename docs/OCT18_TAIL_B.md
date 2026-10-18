# October 18 tB reconstruction tail

Lane `agent/oct18-tB`; input: cc16-progress `local/codex_tail_B.txt` (77 rows).
Names are descriptive hypotheses. The original image, installation and saved
analysis are preserved. No game hosts are wired by this packet.

## Batch 1: fifteen source-level storage bodies

Source: `src/leftovers_cc13.cpp`; declarations: `include/bsp/oct18_tail_b.hpp`.
Original pointers and offsets are Win32. The source API explicitly supplies the
game global and required virtual/direct callees through `Host`; it is not an
ABI-compatible replacement. No dependency returns a fabricated result. Valid
objects, live list links and canonical non-returning CRT error transport are
provider preconditions. Compiler EH frames and undefined upper return-register
bits are not reproduced. MSVC Win32 Release `scripts/build.ps1` passed, including compilation of
`leftovers_cc13.cpp` into bsp_game. Both registered CTests passed (2/2);
receipt: ignored `local/tB_build1.log`. No differential/ABI/game test was run.

| Original | Original ABI | Established behavior and evidence |
|---|---|---|
| 007810F0 | ECX object, stack query, RET 4, EAX bool | Vslot 5Ch accepts 60/2/1/0 or dynamic +C4h. Export and listing agree. |
| 0079A380 | ECX object, stack query, RET 4, EAX bool | Accepts 54/4C/0 or dynamic +C4h; exported body and prior class catalogue. |
| 0079D840 | ECX object, stack query, RET 4, EAX bool | Accepts 55/4C/0 or dynamic +C4h; exported body and prior class catalogue. |
| 007B8A90 | ECX plane, RET | Listing is exactly +9F9h byte = 0, RET. |
| 008364B0 | Unused receiver, stack category, RET 4 | Base director category 0 or 2; listing verified. |
| 00836790 | Unused receiver, stack category, RET 4 | Ship director category 0, 2 or 3; listing verified. |
| 00836EB0 | ECX class, stack target, RET 4, ST0 float | Vslot 5Ch(8), then +764h or +768h; listing has MOVSS/FLD and no arithmetic. |
| 00837DE0 | ECX settings, RET, EAX pointer | +80h for game+1FE4h zero, +F0h otherwise. Listing verified. |
| 0084EB60 | No arguments, cdecl | Lua text `StationaryClass=nil\n`, arguments 0,0,2. Export reviewed. |
| 00797DA0 | ECX track, stack float time, RET 4 | Walk list +6Ch; active cue +F8h with time < +F0h is cleared and optional +E4h vslot 2Ch called. Listing FCOMIP/JBE skips unordered values. This function does not store time. |
| 00836680 | ECX destination, stack source, RET 4 | Base copy 00720450; source vslot 48h(2); capture nonzero +23Ch, query vslot 2Ch then 00836240 on destination. EDI receiver and force-byte capture verified in assembly. |
| 008367F0 | ECX director, stack visitor, RET 4 | Base 00836510 then reference visits of +240h..+248h. Tag 3 bytes and tag 2 floats; no value-copy substitution. Export and argument-building listing reviewed. |
| 0084EFE0 | ECX stationary, RET, EAX pointer | Live decompile: nullable +19Ch object, return its +160h. Part ownership remains unknown. |
| 00784CD0 | ECX transport, RET, AL bool | Count nonzero peer record +1Ah bytes. Exactly one clears the FIRST peer's byte, even when another peer was the flagged one. Listing verified; flag meaning remains unknown. |
| 00968470 | ECX subscription, stack vector, RET 4 | Four ordered filter vslot 0 calls at +0Ch/+1Ch/+2Ch/+3Ch. Bounds checks stay lazy and vector/filter addresses reload after callbacks. Export reviewed. |

Class-test catalogues and typed host policies already described several leaves;
these new functions expose their complete raw-storage boundaries with address
suffixes. The source includes neither native callsite adoption nor gameplay proof.

## Coordination

0093A3F0 is skipped while covered by cc15-rB's 0092E0B0..00951F20 lease.
The new namespace is compiled through the already registered, separately leased
`src/leftovers_cc13.cpp` translation unit (bsp_game). This avoids the currently
held source registries; no CMake ownership is bypassed. No game hosts are wired.
The initial audit is retained in ignored `local/tB_audit.json`. An address mention
alone is not treated as an existing reconstruction; definitions are checked.

## Library and generated construction classification

These are exclusions, not reconstructed game-function credit. Each has
`recon_status: library_instantiation_out_of_scope` in the reconstruction ledger;
reviewed names/comments were applied under the Ghidra write lock and saved.

| Address | Classification evidence |
|---|---|
| 0077EDF0 | MSVC unsigned-key map subscript: lower bound, sentinel flag +15h, default-zero mapped byte, insert-hint 0077E950, return node+10h. No game policy. |
| 00780CC0 | Pair-like aggregate constructor: only two member copies (00884EF0, 00886DA0) and compiler EH rollback. Member names/types remain hypotheses. |
| 00780D10 | Corresponding synthesized aggregate copy: source and source+10h forwarded to those same two member copies, with EH state only. |
| 0081E350 | Refptr-vector assignment: self/size/capacity branches, copy/release ranges, allocate or construct suffix, set end; complete live body has no unit policy. |

00924A70 is also a generic `std::list` destructor (detach/rehead, free nodes,
free head, zero head pointer). Its ledger shards are held by cc15-rB, so only
this evidence note is written until that ownership clears.
