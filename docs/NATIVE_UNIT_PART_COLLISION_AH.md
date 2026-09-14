# Complete unit-part collision construction

Addresses: 00712440, 00711460, 00711020, 00723170, 0098AAB0, 0098A920, 00711ED0.

`build_native_unit_part_collision_00712440` reconstructs the complete 1,139-byte
caller reached by unit-part construction `007135C0`. It builds actual linked
shape storage, publishes the base shapes into the existing node query array,
and calculates local bounds, extents and centre. The six accompanying complete
helpers close its direct dependencies, using the existing CRT new-handler
allocator and legacy length-error owner. These are new C++ interfaces;
descriptive names are hypotheses, not recovered symbols.

| Native entry | Original ABI | Complete source operation |
| --- | --- | --- |
| 00712440..007128B2 | ECX part; no stack args; AL result | Decode, classify, allocate, link, publish, accumulate bounds |
| 00711460..0071149E | ECX list; stack next, previous, source; RET0C | Allocate 30h node and construct its value at+8 |
| 00711020..00711070 | Stack destination, source; RET8; ECX unused | Copy 28h shape with base/derived vtable publication |
| 00723170..0072319C | ECX bounds source; stack min,max; RET8 | Copy source+38/+44 through six x87 transfers |
| 0098AAB0..0098AADC | ECX shape; stack min,max; RET8 | Store shape+4/+10 through six x87 transfers |
| 0098A920..0098AA7A | ECX spatial node; stack min,max; RET8 | Store local bounds, extent and centre with original x87 spills |
| 00711ED0..00711F62 | ECX list; stack increment; RET4 | Grow count+8 with unsigned limit 06666666 |

## Actual storage and ordering

The selected set at part+160 contains the checked vector at+3C, with begin/end
at+40/+44 and eight-byte records. Record+0 supplies the object whose bounds are
read; record+4 is the identity searched in damage-group rows. Shape+1C receives
the existing part, +20 receives record+4, and +24 receives record+0. Those raw
producer relationships are established; they do not alone establish every
pointed-to object's full class or lifetime.

The outer group vector is the existing part+168 descriptor with 16-byte rows.
Its constructor and producer are recorded in `NATIVE_MODEL_GROUP_SELECTION.md`:
`007135C0` initializes the descriptor, `00713380` selects/grows rows, and
`00506DD0` appends node pointers through each row's+8 end. The collision caller
starts searching at row **one**, skipping row zero. Matches enter list+194;
other records enter list+188 and publish their actual value addresses into
part+D0, incrementing count+F8. Both categories contribute to aggregate bounds.
The original ten-slot publication is unchecked; valid callers must provide room.
No source truncation, list reset, null-set shortcut or duplicate suppression is added.

The caller captures its initial source-vector identity and current iterator,
then reloads the live set and bounds after callbacks. BF6713 may return; its
continuations, captured iterator endpoints and repeated validations remain.
Allocation precedes checked count growth. A length-error interruption therefore
leaves the newly allocated node unlinked, as the original caller has no cleanup
for it. The canonical allocator returns storage or throws; native faults from
invalid pointers or a hypothetical null allocation are outside this source ABI.

The initial shape coordinates read the current F87574/78/7C words. This address
is in the PE's loader-zeroed data tail, not file-backed bytes; it is not replaced
with a synthesized zero vector. Aggregate seeds read D7A248/D7A244 (the installed
image contains positive/negative maximum finite float). Unordered comparisons
leave the previous accumulator untouched. An empty input returns false without
calling the node-bounds setter; nonempty input returns true even if all shapes
belong to non-base groups.

The complete node setter stores local min/max at+10C/+118, then extent at+130
and centre at+124. It retains the live double D7A280 on the x87 stack, rounds
each scaled coordinate to a float, subtracts the rounded values for extents,
then reloads and scales the local bounds again for centre addition. The two
FMUL register encodings were checked against disk bytes; DCC9 multiplies ST1
by ST0. Aliasing, signed zero, signaling-NaN quieting and masked status effects
are preserved by the source assembly. The older `spatial_local_extent_0098a920`
value projection remains explicitly partial.

## Evidence and validation

The live Ghidra program and installed PE agree on all seven body spans, the
constant data and the loader-zeroed vector. Eight existing native seeds match.
All 34 report call rows pass the live call-site checker. The list-count body
matches existing `004CEE30` after normalizing the bound, handler and branch/call
addresses; its complete FuncInfo/unwind map and temporary-string cleanup also
match. It reuses the existing owning legacy exception transport, retaining the
library name in Ghidra.

One ignored differential harness runs 64 complete original-caller comparisons
and 48 original helper alias comparisons across eight x87 control words. It
covers empty, base-only, ignored row zero, mixed groups, all non-base groups,
duplicate identities, ten inline entries and a returning validator that repairs
the actual vector. The paired executions construct 208 linked nodes per side.
Normalized pointer identities, all raw part bytes, shape values, list links,
counts, publication, bounds and FP exception/TOP/control state agree. Separate
source checks cover the owning length error and null copy destination.

The original oracle executes all seven body copies, with 40 declared operand
relocations. Its allocation call reaches the same reconstructed CRT boundary;
the invalid-parameter call reaches the fixture repair. Native exception helpers
are guarded and unexecuted. Original FH3, allocation faults, unmasked FP delivery,
private native stack aliases and gameplay are unproved. Part/set/group backing
is fixture input; full upstream creation/loading and mission admission remain
outstanding. No repository test cases or worker dispatches were added.

The Win32 build and both existing CTests pass. Evidence, original bytes, oracle
relocations, raw results and build records are retained under
`local/unit_part_collision_ah/`; `reports/native_unit_part_collision_ah.json`
records the exact scope and hashes.

## Integrated validation

Commit `686a4506eecda8ae61da6d18d19494b307686452` passes the Win32 build and both existing CTests. All 112 comparisons pass against that library. The 120-frame USN01 compatibility run passes its finite trajectory, unchanged Airfield2, avoidance, generic tick, participant, world-list and observer/pending-owner teardown checks. The preserved executable has SHA-256 `a967129008de58eb9eaeec7460b11d9837a403428c0b52a0bbeb2d837644a251`. The mission run does not establish collision-builder admission or gameplay parity. The report references an immutable manifest retaining original bytes, fixture output, linked objects, compilation dependencies and mission artifacts.

After merging separately published loading-owner work, combined commit `fb1a0424b95e77615a901533cc7ba74fd1b29862` passes the build, both CTests and the same 120-frame compatibility checks. Its executable SHA-256 is `3f24d061eb5064f1c244bb49887405c51c09eb5b9aad176e306c5bbe4213bc8a`. All 5 production objects linked into the collision fixture remain byte-identical to the sealed proof. The additional integration manifest is retained separately.
