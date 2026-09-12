# Avoid-zone selected edges and lazy corner clearance

Addresses: 00415190 004158A0 004158E0 00417630 00417A40 00419FC0 00423190 004F3730 0085C910

Packet `orch6_avoid_zone_clearance`, owner `agent/orch6-corner-clearance`.
Ghidra target was verified by each wrapper batch: `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Ghidra remained read only. Descriptive
names are hypotheses; existing names/comments are preserved and proposed
updates are in the repository name ledger for primary integration.

`avoid_zone_clearance.hpp/.cpp` implements the complete lazy clearance loop
and the actual polygon-edge selection/list dependencies. It also provides
complete Win32 instruction schedules for crossing and box/segment tests.
The earlier `gun_bot_remainder.cpp` crossing projection used an exact-zero
denominator substitute; this module calls the existing actual
`native_segment_parameters_004f3630` instead. No shared geometry source was
changed. Consumers must choose the new kernel explicitly.

| Routine | Inclusive native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| 00415190 | 00415190-004151BF | ECX=node; stack start,end; EAX=node; RET8 | complete normal flow |
| 004158A0 | 004158A0-004158D3 | ECX=head slot; RET | complete, including recovered flow gap |
| 004158E0 | 004158E0-00415963 | ECX=head slot; stack start,end,out; EAX=hit node; RET0Ch | complete |
| 00417630 | 00417630-00417884 | ECX=zone; stack box; EAX=head; RET4 | complete normal flow |
| 00417A40 | 00417A40-00417AC1 | ECX=group; stack box; EAX=head; RET4 | complete normal flow |
| 00419FC0 | 00419FC0-0041A02A | ECX=head slot; stack query; ST0=distance; RET4 | complete |
| 00423190 | 00423190-004234F8 | ECX=zone; stack corner; RET4 | complete normal flow through explicit manager, allocation and CRT access |
| 004F3730 | 004F3730-004F3801 | ECX=start0, EDX=end0; stack start1,end1,out; AL; RET0Ch | complete native instruction schedule |
| 0085C910 | 0085C910-0085CAC7 | ECX=box min, EDX=box max; stack start,end; AL; RET8 | complete native instruction schedule |

The public clearance, list and producer interfaces adapt native member-call
ABIs. `AvoidZoneNativeStorage` and `ShipAiPathLateralRecord` reuse the established
native layouts. `AvoidZoneClearanceGroupView` borrows the actual native group's
zone pointers/count; it is neither the native 14h group allocation nor the
semantic `AvoidZoneLayerGroup` vector. The primary can mirror group indices
between its semantic `AvoidZoneTable` and these native pointer arrays.

## Producer and traversal

00415190 writes two endpoint pairs, zeros next/previous/next-run and the
byte at +1Ch, and leaves the last three bytes untouched. Allocation is 20h
at00417798/0041779F; `ADD ESP,4` at004177A4 confirms one cdecl size argument.
The node layout is endpoints +00h/+08h, next +10h, previous +14h, next-run
+18h, close marker +1Ch. These are contiguous edge runs, not a spatial tree.

00417630 first rejects ordered-separated zone/query bounds. Equality passes
this zone gate, and unordered comparisons also pass. It reads corner[count-1]
before checking the iteration bound. A bounds-overlapping zone therefore
requires a nonempty native-valid corner array; no empty-polygon repair was
added. Edges run last-to-first, then first-to-second and so on.

Each edge's bounds must overlap the query box strictly on both axes; a
touching edge is excluded. Native `FCOMIP/JC` chooses each min/max branch on
less **or unordered**; the later `JBE` overlap gates reject unordered. The
remaining edge passes004F2B00, whose entire body is
`LEA EDX,[ECX+8]; JMP0085C910`. Thus the SAT uses the query box's +0/+8 pairs.
Selected nodes retain the whole edge; they are not clipped to the box.

Consecutive selected edges link through next/previous. When a gap ends a run,
every node of that run gets the next run's first node at +18h. If the final
edge is selected, the producer joins it to the first selected run; when the
whole result is one run, its tail gets close marker1. Otherwise the returned
head rotates to the second run and the joined final/first run's continuation
links are cleared. 00417A40 appends each zone's runs in group array order,
updating the prior final run's continuation links, preserving this topology.

Every list consumer advances to +10h only when close marker is zero and the
next pointer is nonnull; otherwise it takes +18h. 004158E0 leaves output
untouched for an empty list. For a nonempty list it first copies the supplied
end into output, then tests `start -> current output` against every edge.
Successful intersections shorten output; the returned pointer is the last
edge that did so. Equal-distance later hits can replace the earlier pointer.
00419FC0 takes an ordered minimum of actual00419AB0 distances, starting with
`00D7A248=7F7FFFFF` (FLT_MAX). Empty/all-NaN lists retain FLT_MAX.

004158A0's exported pseudocode falsely stops at its first `_free` because
Ghidra omitted the returning flow. Bytes004158C8-004158D1 are
`83 C4 04 85 FF 89 3E 75 DF 5F`:
`ADD ESP,4; TEST EDI,EDI; MOV [ESI],EDI; JNZ004158B0; POP EDI`.
The complete routine frees every traversed allocation and leaves head zero.
It captures the next pointer before freeing the current node. The report
records this unresolved analysis gap without mutating Ghidra.

## Clearance and actual boundaries

The `COMISS/JA` cache guard at004231B8/004231C4 returns for positive values.
Zero, negative, and NaN values compute again; there is no second guard after
locking. `manager_critical_section_004218e0()` must return the actual nullable
manager+4h pointer. A nonnull canonical `TrackedCriticalSection` is entered,
then its +18h depth is incremented. The required layer-selection callback
runs later, corresponding to the second singleton lookup and004120D0 call.
It must resolve actual group data from zone.layer and keep it valid through
unlock. Null and empty groups legitimately yield no selected nodes; there
is no default success/list substitute. This packet does not own the singleton
or layer lookup reconstruction.

For corner position p and its derived offset direction d:

1. Set base=p+d, radius=800, candidate=base+800*d. Construct the box centered
   on candidate with half extent800 through the existing00415010 include-point
   kernel and select actual edges for the zone's layer.
2. Intersect base to base+1600*d against the selected list. On a hit, set
   radius to half the00414C60 cutoff length of base-hit and recompute candidate.
3. Measure the minimum selected-edge distance q. While q<radius-1, set radius
   to the smaller of radius-50 and (radius+q)/2. Values below25 become20 and
   terminate immediately; otherwise recompute candidate and q.
4. Store corner+20h, destroy the selected list, decrement depth, and leave the
   critical section. A positive cache avoids all manager and allocation access.

All products/base sums used by the loop retain the binary32 spill boundaries.
The radius-minus-one comparison and radius-plus-distance half sum keep their
native unspilled precision; these float operands/range fit the double
intermediate exactly. The private00414C60 adaptation retains the instruction
schedule and uses required borrowed `CameraAxesCrtAccess`, not current-host
sqrt. Constants were checked from live bytes at00CE3930-00CE3953 and00D7A248;
the prior lateral-record packet supplies the25,1 and0.5 constant evidence.

The normal flow is complete. Native SEH/hardware-fault ABI is not reproduced.
C++ unwinding destroys a returned list and releases an entered gate. Native
allocation failure inside an unreturned producer list leaks preceding nodes;
that recovery behavior is preserved, not replaced with a new rollback policy.
Allocation access must implement actual allocate-or-throw and matching free.

## Caller, register and call-site checks

Both clearance callers were read:00423534 passes the indexed corner before
an offset-point query, and009D5923 passes the just-attached path-node record.
The list-construction callers are004224E8 and004232EE. The minimum-distance
callers are004233DF and0042348D. The list-clear callers include its thunk,
layer change00419FA0, list replacement004224C0, three ship-AI enabled-state
transitions009DA724/009DA76E/009DA7BB, and three destructor fields at
009E0218/009E0228/009E0238. All use a head slot, with no stack arguments.

All eight list-hit call sites were read:00423362,009EB819 and six sites in
009DC2E0 (009DC457,009DC510,009DC617,009DC725,009DC82C,009DC984).
Each supplies start/end/output. The entire009DC2E0 listing was filtered for
ESI writes: ESI receives the list owner at009DC2EE and is unchanged at those
six calls; it is reassigned only after them at009DCA17. For004158E0, EBP is
the start pointer, EDI is the output pointer, ESI walks the node and EBX
retains the hit node. The complete00417630 and00417A40 listings establish
zone/group in EDI/EBX and run/head registers before their uses.

004F3730's nine call sites in eight functions were checked, including both
004BC120 calls and the float3-to-float2 wrapper004B6720. Its three stack
pointers are confirmed by RET0Ch, not guessed from the decompiler. 0085C910
has only004F2B00 as caller; that adjustor's full body and selected-edge use
were read. The report includes exact call-site/callee/containing-function
rows for machine verification, including these supporting callers.

## Corrections and boundaries

| Was | Is | Evidence |
| --- | --- | --- |
| 004158A0 frees one node and returns | full traversal/free loop clears head | recovered bytes004158C8-D1 and surrounding body |
| selected nodes described as a segment tree | linked contiguous edge runs with continuation pointers |00415190 writes;00417630/00417A40 link producers |
| lazy-clearance membership/lifetime unresolved | actual selected-layer polygon edges, strict edge overlap plus SAT, complete run lifetime |00417630,00417A40,004158A0 bodies |
| older crossing exact-zero denominator substitute | existing native relative solver and original x87 result/interpolation schedule |004F3730 calls004F3630 at004F3781 |
| abbreviated FMUL ST2/ST1 interpreted as writing ST0 | SAT writes ST2 at0085C93D and ST1 at0085C988 | raw DC CA/DC C9; the original-byte fixture caught the direction error before commit |

No `bsp_game.exe` host source was changed in this packet. Executable binding,
mission validation and gameplay remain primary integration work. Compilation,
call verification, and the single ignored focused differential probe are
reported separately in `reports/avoid_zone_clearance.json`. No tracked tests
were added.

The final strict Win32 build and both existing CTests passed. The single
ignored `local/clearance_probe.cpp` matched 17 selected edges over three boxes
(12 whole-group edges, five edges in separated runs, zero edges for a touching
box), including endpoint bits, every link, hit identity/output, distance and
complete freeing. All 12 corner-clearance results from two concrete polygons
matched original-code binary32 results; positive cache, lock-before-selection
and balanced226 allocations/226 frees also passed.

The probe copies the installed PE into its own process without starting the
game. Original preferred-address mapping collided with host reservations, so
17 explicitly bounded routine spans use24 absolute operand relocations;
relative control flow is unchanged. Those spans were compared against live
Ghidra and installed disk bytes before fixture patching. The ignored relocation
manifest and the report record hashes and exact boundaries. The only fixture
bindings are real malloc/free, the actual fixture manager, Windows critical
section imports and the same existing reconstructed native CRT access kernel
used by the C++ side. The fixture uses the mapped initial dispatch flag0 and
finite nonnegative square roots; exceptional CRT/new-handler behavior remains
unexercised. This is bounded differential evidence, not gameplay or a complete
legacy-runtime validation.

## no_ghidra_function and gaps

No missing function entries. Every owned routine has the exact body above.
Unlisted bytes within bodies:004158C8-004158D1 is the decoded free-loop tail;
004176BD-004176BF and00419FDD-00419FDF are `8D 49 00` alignment instructions
after unconditional jumps. No other owned-body gaps were reported by the
read-only flow query. Primary annotation/flow repair and refreshed exports
remain separate from this read-only packet.
