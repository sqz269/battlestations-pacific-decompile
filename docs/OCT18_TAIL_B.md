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

## Batch 2: twenty further normal-path bodies

Same source/header/namespace and genuine Win32 storage contract as batch 1.
MSVC Win32 Release build and both existing CTests passed (2/2); ignored
receipt `local/tB_build2.log`. Error handlers and library allocation/insertion are required
providers, not reconstructed vendor bodies. Destructor and constructor EH
unwind paths are not reproduced; normal-return business/storage coverage is
distinct from original ABI, allocator identity, exception and gameplay proof.

| Original | Original ABI | Evidence / behavior |
|---|---|---|
| 0077C710 | ECX entity, stack message, RET 4 | +5Eh gate, matching holder at list +2A8h, signed message tick vs uint16 previous tick; reject signed difference <=7530h, then store and vslot 18Ch. Listing reviewed. |
| 0077DA20 | ECX entity, stack record, RET 4 | Erase matching holder from +2A4h through 0077D980; unlink/free matching +29Ch list node, decrement +2A0h after free. |
| 007844E0 | ECX network, RET | Restore 00D04274; delete peer/message objects; clear lists; unwind tracked lock depths in +44h/+24h/+34h order; free four sentinel heads. Complete normal decompile reviewed. |
| 00785170 | ECX client, RET | Enter +C0h tracked lock, delete +A0h/+B0h vector pointees, clear both ranges, reload lock on leave. |
| 007857F0 | ECX network, stack peer, RET 4 | Membership gate; deduplicated removal queue under +24h lock. Allocate, checked-size increment 00773600, link, timer +48h from supplied actual 00CE38C8 bits. The misleading xlen name is not an unconditional no-return. |
| 00785CC0 | ECX client, RET | Pending hosts drain front-first under +C0h lock; vslot 34h address comparison; replace before deleting old host, or use capacity fast append / bound library growth. Stable native pending cursor and post-callback reloads preserved. |
| 008363E0 | ECX director, stack endpoint/byte, RET 8, EAX receiver | Base constructor; actual phase-vtable values; reference storage fields; short-circuit class 0Bh/9 tests on current +34h endpoint. Preserve base permissions when either test succeeds. Listing reviewed. |
| 008360C0 | ECX director, stack visitor, RET 4 | `_gameUnit` scope (0071BDD0 is proven no-op), four enable bools, target ID and primary flag through vslot 0Ch boxed values. |
| 008362A0 | ECX director, stack visitor, RET 4 | Base property dump then four avoidance/heading bools and two float bit payloads. |
| 0084D810 | ECX block, stack squadron, RET 4, EAX receiver | Base endpoint +310h; vtables 00D0BD98/00D0BD7C; fighter vslot 18h(13h) queried even at behaviour zero; permission write order; new(40h) and 009F7590 with recovered ECX and two arguments. |
| 0084EC10 | ECX message, RET | Supplied actual stationary registry globals; unsigned index bound; conditional +1A4h byte, dead vslot 7Ch or live vslot 70h(1). |
| 008826F0 | ECX landscape, RET | Physics release uses current game+18h world receiver, vector receivers +364h/+374h. Floating-object removal uses ocean+0A8h wave field, not ocean itself. Listing verifies these missing pseudocode receivers. Node and refcount release order retained. |
| 00864CA0 | ECX AI, stack target/gun, RET 8 | Exact iterator owner check; miss allocates 8Ch, constructs and inserts record; hit reads node+10h into preserved EDI; 00862660 gets that record receiver and gun. Listing fixes the old decompile's missing receiver. |
| 0081BFE0 | ECX SHIP_CREATE message, RET, EAX instance | Listing recovers uint16 class ID at +1Eh with DL=1, factory vslot 28h(entity ID +1Ch), name +C0h read after creation, resize +154h without terminator, copy stored length. |
| 0091D620 | ECX receiver, RET, EAX receiver | Listing preserves ESI across required 0091CE90 constructor, then zeroes +284h. The field meaning remains provisional. No claim to close the callee's many container/EH owners. |
| 007701F0 | ECX session, stack index, RET 4 | Listing: capture record before 00783F20(client,index); forward copy name and six fields; load game global late; clear actual 00F1AF2C DWORD; destroy game+5F0h list; reset game+5FCh and native string +600h. |
| 007B8A80 | ECX type ID, EAX class | MOV DL,1 / JMP00964790; required provider has existing ECX/DL contract. |
| 00835610 | ECX director, stack message, target RET 4 | JMP0071C1E0. Existing target declaration in gameplay_seg38_cc13.hpp; no duplicated target body. |
| 008FE790 | ECX bot, stack target, target RET 4 | JMP006DF170. Existing target source declaration in cc13_seg33a.hpp. |
| 00852200 | ECX descriptor, no stack args, AL bool | JMP00828F20. Existing contract in ship_ai_throttle_ring.hpp. Its arithmetic remains the required callee's responsibility. |

The last four rows are game delegation bodies. A tail JMP alone does not prove
library or compiler-generated origin, so these are not exclusion classifications.
