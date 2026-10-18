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

0093A3F0 was initially skipped under cc15-rB's range lease. That owner later
released the range and shards; tB claimed the freed resources for batch 3.
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
| 00924A70 | Generic POD-list destructor: detach/rehead, zero count, free nodes without element policy, free sentinel, zero head. Recorded after the former metadata lease cleared. |

00924A70's prior bounded pending-owner equivalence fragment is preserved. The
new library exclusion does not replace or promote that fragment's admission.

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

## Batch 3: seven closure bodies after ownership cleared

Same compiled translation unit and source API qualifications. MSVC Win32 Release
and 2/2 existing CTests passed; ignored receipt `local/tB_build3.log`.

| Original | Original ABI | Evidence / behavior |
|---|---|---|
| 0093A3F0 | ECX repair task, RET, ST0 float | Listing: scan 10h records via +18h/+1Ch, nullable name +8h; canonical stricmp against EngineJam; last matching +0Ch float wins, default positive zero. MOVSS/FLD only, no x87 arithmetic. |
| 0095C550 | ECX class ID, RET | Supplied actual census vector 00F8A09C: duplicate scan, capacity fast append, required 00441F30 growth binding. Does not create another host census. |
| 009277E0 | ECX entity, RET through callee | JMP00923840. Directly reuse existing complete raw flag-storage source in entity_mission_seg60_argon.cpp. |
| 00922DC0 | ECX entity, DL forced 1, AL result | MOV DL,1 / JMP00922C80. Reuse the existing source target with its actual Seg60Host and allow_far=1. |
| 008429C0 | ECX descriptor, stack LuaObject row, target RET 4 | JMP00960230. Target's annotated contract independently confirms descriptor/row and RET 4; Lua loading and lifetimes remain required provider responsibility. |
| 00968AF0 | ECX subscription, stack parameter vector, RET 4 | Export is structurally identical to 00968470: same four filter offsets and lazy range checks. Reuse the new complete checked-storage helper; do not duplicate its algorithm. |
| 008EE670 | ECX nullable text, RET, EAX scratch 00F88C50 | New raw scratch interface preserves native forward alias/read/write order and the backslash/n/q escape rules, discards unknown pairs, terminates and returns scratch. Actual scratch is supplied explicitly. Valid-input preconditions retain no lone trailing backslash and sufficient scratch space. Existing owning-string parser remains unchanged. |

This last decoder is additional raw-storage coverage. It does not broaden the
existing parser's admitted string domain, reentrancy, ownership or game evidence.

## Batch 4: three profile rows after the last metadata hold cleared

The 007F name/reconstruction shards became free during final publication.
tB claimed them and completed these rows rather than retain stale held skips.
Same actual-storage, required-provider and source-only ABI qualifications.
MSVC Win32 Release and both existing CTests passed (2/2); ignored receipt
`local/tB_build4.log`.

| Original | Original ABI | Evidence / behavior |
|---|---|---|
| 007F8890 | ECX profile, stack NativeString ID, RET 4, AL bool | List head +D0h; node length +8h and data +0Ch. Length check precedes canonical stricmp. Empty IDs match without inspecting either data pointer. |
| 007FB990 | ECX profile, RET | Listing confirms saved set +70h, pending set +7Ch, two-DWORD iterator owner/node. Insert before increment; required canonical subtree erase then rehead parent, zero count, left and right in native order. Source NativeString ownership stays with the actual library owners. |
| 007FBE20 | ECX profile, stack NativeString key / signed value, RET 8 | Listing: capture +A4h sentinel before map find at +A0h; validate iterator owner. Miss stores supplied signed value, including negatives. Hit indexes once to compare and again only when raising the counter. No zero-floor substitution. |

## Complete worklist disposition

All 77 input rows were reviewed. New source bodies: 45 (15 + 20 + 7 + 3).
Five library/generated exclusions; 15 retained existing implementations/models
with their original qualifications; twelve explicit contract gaps remain.
Both former metadata holds cleared during this turn and their ready rows were
closed. Existing bounded models are not promoted to native whole-body credit
by this audit. No game-host switches, original ABI or gameplay claim.

| Address | Disposition | Evidence / remaining work |
|---|---|---|
| 007701F0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0077C710 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0077DA20 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0077EDF0 | library/generated exclusion | Recorded out-of-scope in ledger; evidence in classification table |
| 00780CC0 | library/generated exclusion | Recorded out-of-scope in ledger; evidence in classification table |
| 00780D10 | library/generated exclusion | Recorded out-of-scope in ledger; evidence in classification table |
| 007810F0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007844E0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00784C40 | uncertain/contract gap | Timeout policy read; unspilled ST0 return/threshold comparisons require an explicitly qualified x87 provider contract |
| 00784CD0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007850E0 | uncertain/contract gap | Silence policy read; x87 and warning/disconnect callback/global reload contract require qualification |
| 00785170 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007857F0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00785CC0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00787850 | uncertain/contract gap | Only borrowed cursor fragment already reconstructed; full ring timing, unsigned x87 conversion and message-factory composition remain unbound |
| 00797DA0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0079A380 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0079D840 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007B8A80 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007B8A90 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 007D1360 | existing bounded source | src/pilot_command_path.cpp; existing listing-reviewed control-message fragment; no new whole-body credit |
| 007F8890 | new batch 4 | Metadata ownership cleared during this turn; explicit raw profile/list/set/counter policy with canonical library providers, see batch 4 |
| 007FB990 | new batch 4 | Metadata ownership cleared during this turn; explicit raw profile/list/set/counter policy with canonical library providers, see batch 4 |
| 007FBE20 | new batch 4 | Metadata ownership cleared during this turn; explicit raw profile/list/set/counter policy with canonical library providers, see batch 4 |
| 0081B630 | uncertain/contract gap | World transform/effect/event path read; effect ref ownership and warning argument transport not closed |
| 0081B7C0 | uncertain/contract gap | Camo path read; resource-manager ref ownership, register arguments and diffuse-color tail not closed |
| 0081BFE0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0081E350 | library/generated exclusion | Recorded out-of-scope in ledger; evidence in classification table |
| 00835610 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008360C0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008362A0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008363E0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008364B0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00836510 | uncertain/contract gap | Reference visitor tail uses mutable stack aliases for fireTargetID/primary before StoreFireTarget; preserve qualification instead of inventing temporary defaults |
| 00836680 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00836790 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008367F0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00836EB0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00837DE0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008429C0 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0084D810 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0084EB60 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0084EC10 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0084EFE0 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00852200 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00864880 | existing bounded source | src/ship_gunnery_ver_cc13.cpp; artillery-ranging model, not complete raw constructor |
| 00864CA0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008826F0 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00892660 | uncertain/contract gap | Existing mission Lua host covers name; original debug-format/angle result and call-frame transport remain unqualified |
| 008CBCB0 | uncertain/contract gap | Existing mission Lua host covers name; original object-handle/tree error transport and Lua frame lifetime remain unqualified |
| 008D08B0 | uncertain/contract gap | Existing mission Lua host covers name; original optional integer/string/table and voiceover ownership branches remain unqualified |
| 008D2F50 | uncertain/contract gap | Large Lua loader only partly reconstructed by existing fragments; all table mappings, native Lua owners and latency producer need a separate packet |
| 008D44C0 | existing source | src/game_settings.cpp: set_xbox_compatibility_008d44c0 and src/lua_runtime_globals.cpp publication; no duplicate |
| 008E2EE0 | uncertain/contract gap | Ping source policy read; rate-limit x87 timing, native message/chat ownership and integer formatting remain open |
| 008EE670 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 008EF170 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type1_float_storage.cpp; retain existing admission qualifications |
| 008EF1B0 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type2_string_storage.cpp; retain existing admission qualifications |
| 008EF230 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type4_storage.cpp; retain existing admission qualifications |
| 008EF270 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type7_storage.cpp, src\native_scene_typed_array_vector_providers.cpp; retain existing admission qualifications |
| 008EF2B0 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type5_storage.cpp; retain existing admission qualifications |
| 008EF2F0 | existing native source | Address-suffixed definition already in src\native_scene_property_clone.cpp, src\native_scene_property_record_type8_byte_array_storage.cpp, src\native_scene_typed_property_providers.cpp; retain existing admission qualifications |
| 008EFB00 | existing bounded source | src/scene_file.cpp: admitted quoted run collection; native builder lifetime remains unbound |
| 008F0340 | existing native source | Address-suffixed definition already in src\native_reference_payload_copy.cpp, src\native_scene_property_record_assignment.cpp; retain existing admission qualifications |
| 008F41F0 | existing native source | Address-suffixed definition already in src\native_scene_hidden_entity_record_copy_construct.cpp, src\native_scene_property_bag_storage.cpp, src\native_scene_property_clone.cpp, src\native_unit_scene_initialization.cpp; retain existing admission qualifications |
| 008F54F0 | existing native source | Address-suffixed definition already in src\native_scene_property_bag_merge.cpp; retain existing admission qualifications |
| 008F81F0 | existing bounded source | src/winmain_startup.cpp and process bindings in src/game_hosts.cpp; original raw SEH/lifetime remains qualified |
| 008FE790 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0090C560 | existing source | src/profile_unlock.cpp plus include/bsp/profile_unlock.hpp: present/nonzero completion predicate; typed model only |
| 0091CE90 | uncertain/contract gap | Existing persisted defaults only; dozens of heterogeneous STL-head owners and EH rollback require a complete lifetime/provider schema |
| 0091D620 | new batch 2 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00922DC0 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00924A70 | library/generated exclusion | POD list destructor; rehead, free nodes and sentinel, zero head; ledger exclusion recorded after cc15 ownership cleared |
| 009277E0 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0093A3F0 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 0095C550 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00968470 | new batch 1 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |
| 00968AF0 | new batch 3 | Address-suffixed source in src/leftovers_cc13.cpp; original ABI, evidence and qualifications in batch table |

Coordinator action: retain the five exclusions and existing native-property
admission limits, and size separate packets for the twelve explicit ABI, x87,
Lua/message, visitor or container-lifetime gaps. No row remains skipped solely
because of another lane ownership hold. The worktree is retained for cleanup
by the coordinator.
