# Ship AI avoid-zone cache refresh

Addresses: 009D7050, 00419FA0, 004224C0. Existing Ghidra project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

`src/ship_ai_avoid_zone_search.cpp` reconstructs the complete normal bodies:
009D7050..009D724E, 00419FA0..00419FB9 and 004224C0..004224F2.
All three native interfaces take ECX as receiver, one stack argument and RET4.
The descriptive names are hypotheses, not recovered symbols. The new C++
interfaces are not binary replacements; cache refresh returns a new diagnostic
boolean rather than exposing incidental native return-register contents.

The searcher's enabled byte is at +0, cached bounds at +4..+10, query key at
+14, selected-segment head at +18 and the list's independent layer key at +1C.
009E4330 initializes enabled=true, query key=-1, head=null and list key=0;
it leaves the four bounds words alone. Callers of this interface must supply
their initial bounds. Both 009DA6E0 and 009DC2E0 call with disjoint stack queries.
The latter's query production remains separate work.

009D7050 first gates on enabled. For equal integer query keys (CMP at009D7068,
despite the misleading float pseudocode), it tests both query diagonal points
against the existing half-open box through00414F50. A cache hit leaves every
field and allocation unchanged. A refresh keeps the larger of the old half-size
and the expanded query half-size, then applies the500 floor. The multiplier at
CEC160 is exactly1.2000000476837158203125, the widened binary32 value; replacing
it with double1.2 changes products. Explicit x87 spills and unordered comparisons
retain the listing's choices. The new cached key is stored before list helpers.

00419FA0 clears the list only when its layer key changes, including an empty
list, then stores the new key. 004224C0 clears a nonempty list, fetches the
manager before reading the live layer key, selects the group through004120D0,
and calls the existing00417A40 segment-run selector. The semantic manager must
contain group slot zero, which the original group lookup reads without a guard.
The new adapter rejects an empty table explicitly. A native nullable group slot
and allocation/SEH failures are outside this representation's normal domain.

`GameAvoidZoneRuntime::refresh_search`, `clear_search` and `search_segment` bind
this cache to the already loaded manager, original zone ordering, existing
allocator, complete segment selector and004158E0 crossing kernel. The caller
owns each selected list and must clear it before the geometry allocator dies.
No native manager publication ABI is implied by this process adapter.

The Win32 build and both existing tests pass. The ignored native differential
fixture and loaded-map probe are recorded in the associated reports; no tracked
test suite was added. See also `SHIP_AI_AVOID_ZONE_SEARCH_PROOF.md` when present.
The comparison covers float bits, cache/list/node state and callback order under
masked x87 exceptions. Output control/status words, TOP and MXCSR are not
compared. Unmasked fault sequencing is outside the claim: for example native
009D71EE reads query z before the min_x store, although its spill follows that
store at009D71F5. Both known callers supply disjoint records.

## Runtime connection still required

This batch does not enable the sector scan's avoid-zone gates. Its persistent
director+242 owner and the arc-clipping callback00415970 are still missing from
that integration path. WeaponDirector's reconstructed constructor writes true
at00836730, but borrowing a temporary default would omit the live property
setters. Native009DA6E0 also manages all three searchers; only searcher zero is
queried there. Correct initial bounds ownership, the other query producer and
the existing request-field writers must be connected before claiming this
path changes ship steering. Build and isolated geometry evidence are not
gameplay validation.
