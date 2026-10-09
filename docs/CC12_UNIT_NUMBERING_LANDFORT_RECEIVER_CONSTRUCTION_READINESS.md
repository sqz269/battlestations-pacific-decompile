# Land-fort receiver construction readiness for numbering

**Source-held: no production receiver owner or new Source packet is admitted.**
The complete allocator and constructor now establish the physical allocation
sequence and the derived profile/class stores. They do not close base construction,
descriptor ownership, publication, model providers or root teardown. A new 0x758-byte
buffer with copied Native table integers would not satisfy those dependencies.

## Complete owned Native evidence

| Body | Complete range | Bytes / operations | Physical calls |
| --- | --- | --- | --- |
| Allocator 00747000 | 00747000..00747082 | 131 / 40 | 4 |
| Constructor 00745940 | 00745940..00745A32 | 243 / 58 | 4 |

All 374 bytes / 98 operations and all eight call rows were reviewed. Original PE
and live Ghidra bytes and instruction starts agree. The constructor's existing
saved listing also agrees. The allocator had no historical export: its complete
live listing and PE body are now saved locally and rechecked. That fresh snapshot
is retained evidence, not an independent historical listing.

Every live query used bsp.py ghidra target verification for project bsp, program
/battlestationspacific.exe, x86 language and image base. Configuration selects
C:/Users/sqz269/bsp.gpr; Original image hash remains
b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6.
Function count remains 64729. No Ghidra or shared export mutation occurred.

The two leased four-byte cells also match PE/live bytes:
00CFF3F8 contains 00747090; 00CFF454 (=profile+5C) contains 006F5890.
The admitted 006F5890 leaf was not reopened. The slot0 target has an existing
scalar-deleting-destructor label and edges to 00745AA0/_free in metadata; neither
body was opened or promoted to lifetime proof.

## Allocation, ownership and ABI

00747000 receives the descriptor in ECX and one full flag DWORD at entry ESP+4.
It captures the descriptor in EDI, calls operator_new(0x758), captures EAX in ESI,
then calls _memset(ESI,0,0x758). ADD ESP,10 removes the three memset arguments
and retained operator-new size. **Zeroing precedes the allocation null test.**
The physical null arm is not a safe allocation-failure recovery contract.

With a nonnull allocation, it passes the original flag to 00745940 with ECX equal
to the allocation and retains the returned EAX as ESI. It then calls 009553D0 with
that root and the captured descriptor, even on the physical null arm, before
directly storing the descriptor at R+354. The setter's owning R+538 behavior is
existing metadata, not a freshly reviewed Native body. It returns EAX=R with RET4.
ESI/EDI are preserved; there is no normal root free or own global publication.

The allocator installs an FS exception frame with handler 00C87288, saves the
allocation in its local slot, writes state0 before construction and state-1 before
009553D0. The handler, unwind tables and funclets are unopened. Those state writes
do not prove rollback, deallocation on failure or C++ exception equivalence.

## Complete constructor and final derived identity

00745940 takes ECX=R and one raw flag word, preserves EBX/ESI/EDI and returns the
same R in EAX with RET4. It calls 0095CC90(R,flag,0), then calls 00809270 on the
actual subobject R+72C. These Native children remain unopened.

After those calls, it writes eight literal profile words at offsets
0,10,24,170,1E4,310,38C,72C. Their values, in that order, are:
CFF3F8,CFF3E0,CFF3D8,CFF3D4,CFF3CC,CFF3B4,CFF3B0,CFF3AC.
It zeros +740 and +744, writes **R+C4=1B**, and zeros +738/+73C.
It reloads +740 and retains the complete physical nonnull branch:
InterlockedDecrement(pointer+4), then actual pointer vslot0 if the returned count
is zero, then clear +740. That virtual call pushes no deleting flag or other new
argument. Exact PE import metadata identifies 00CE2220 as InterlockedDecrement.

For stable ordinary fresh storage, the directly preceding +740=0 makes that
branch skip; it is still preserved in all eight call rows. The common tail clears
+740 again and writes float +0 bits to +750 via XORPS/MOVSS. There is no x87.
The constructor's EH handler is 00C87102; state0 precedes the subobject call and
a low-byte state3 store precedes the physical release branch. Exception children
remain unproved.

These writes establish post-constructor R0=CFF3F8 and R+C4=1B under the normal
callee-preservation/storage contract. The factory then passes R through unopened
009553D0. The derived stores do not make that whole returned owner Source-ready.

## Where numbering and model storage come from

| Cell or stage | Evidence and remaining boundary |
| --- | --- |
| Initial R+35C and R+360 | The allocator's full zero-fill reaches both before any constructor. |
| Constructor-owned R+35C/R+360 stores | Neither complete owned body directly writes these fields after zeroing. |
| Initial numbering sentinel | Existing UNIT_INSTANCE_LAYOUT.md line130 attributes FFFFFFFF at +35C to 0087B670. This is inherited evidence; that Native base was not reopened. |
| Base dependency | Owned constructor calls 0095CC90, whose indexed base is 0087B670. Exact field effects, publication and unwind of that chain remain unopened here. Do not infer final +360=null from preimage alone. |
| Actual model assignment | Existing initialize_native_unit_health_parts_0087bcc0 Source assigns null to the borrowed model_360 cell when no part-set exists, or assigns the returned model after actual allocation/007135C0 requirements. |
| Actual numbering assignment | The same Source provider chooses saved/default/property numbering and writes the borrowed numbering_35c cell when changed; the no-part-set arm returns before this assignment. |
| Scene initialization | Existing initialize_native_unit_scene_00955420 calls that provider and then consumes the same actual model/scene fields. Both APIs require real borrowed storage and external providers. |

The existing complete-normal-caller Source statuses for 0087BCC0 and 00955420 are
retained without rerunning their Native fixtures. Their field-reference views do
not allocate, place, own or publish a full Native unit. No concrete application
subclass for those binding interfaces or UnitInstanceCreationHost was found in
the bounded current Source search. The present model/numbering provider chain
is therefore identified, but its production storage and lifetime are not supplied.

## Current Source and application owner evidence

UnitOwnedRefSlot has three uint32 fields and the existing 00809270 Source helper
writes its profile and two zero words. It is conditionally reusable only when the
actual R+72C object is placed with the required lifetime. It does not construct R.
The generic create_unit_instance routine is an injected semantic projection;
it explicitly states that it does not allocate/link/copy real memory.

Current SceneUnitCreatorBinding still returns a type_id+1 descriptor token and
then &GameSceneEntityRecord from create_instance_from_descriptor. GameUnitsHost
separately creates unique_ptr<GameUnitSlot>. That slot owns only the concrete
0x20 observer prefix plus semantic members; its partial table publication does
not construct a 0x758 Native unit. Current Lua unit Ptr publishers still put the
numeric entity ID into lightuserdata. These current Source reads corroborate the
prior caller-readiness boundary without opening the other worker's 00928A00 body.

The scene-base lifetime readiness remains relevant: real raw ID pair ownership,
registry/pending-init publication, world/hierarchy links, full virtual teardown
and EH/rollback are unresolved. A newly constructed zero-world root is not
already proven valid for normal destruction. Genuine allocation, construction,
publication, borrowers, withdrawal and root freeing must form one connected owner.

## Current Source121 context and conclusion

At capture time, all **121 raw Root inputs and four artifact hashes/sizes match**
reports/cc12_native_mlandfort_kind_primary_review.json. All 121 worker inputs match
after LF normalization; the sole raw difference is CRLF-only in
reports/cc12_pending_registry_tick_registration_constructor_readiness.json.
The Root build is 2026-10-09T20:46:26.766022Z..20:46:43.654990Z. Its pinned report
records 37 captured/replayed whole objects, 41 positive Core roots and three
passing existing checks. These are replayed Root context, not a worker build or
fresh object recapture. Source117 is historical. Later Root changes can replace
these artifacts; this report timestamps the successful comparison.

No new immediate **Source-ready production** prerequisite is established.
The next exact read-only frontier is 0095CC90's full receiver construction contract,
including its actual 0087B670 dependency and the ownership/publication obligations
they impose; the separate descriptor setter 009553D0 and current root slot0 target
00747090 remain distinct ownership/teardown gates. Existing 00809270 typed Source,
the admitted 006F5890 predicate, and the qualified memset provider are reusable
conditional components, not reasons to duplicate leaves or manufacture a blob owner.

A cohesive implementation must bind the actual descriptor, complete base chain,
placed R+35C/R+360 fields, actual health/model providers, identity-preserving world/
ID/Lua publication, executable current-profile dispatch, and complete withdrawal/
destruction. No callback facade, table-address cast or semantic-record copy closes
those requirements.

Only this document and its report are tracked outputs. No extra Native body lease
was used; 00928A00, the numbering leaf, the kind leaf and base Native bodies stayed
outside this packet. No C++, CMake, ledger, GPR, build, test or probe changes occurred.
All Source/build/ABI/startup/gameplay credits and newly ready Source packet counts
remain zero.

Evidence report:
reports/cc12_unit_numbering_landfort_receiver_construction_readiness.json.
Local complete capture:
local/cc12_unit_numbering_landfort_receiver_construction/capture.json.
