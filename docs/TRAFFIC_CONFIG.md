# Traffic configuration owner and party-pair loading

Addresses004A43C0 and0049D690 construct the traffic owner published at game+21D0
and load its party-pair tables. Names here are descriptive hypotheses. The
loader uses `Scripts\datatables\TrafficGlobals.lua`, then `PartyPairs.Soldiers`
and `PartyPairs.Vehicles`; these are nested tables, not independent globals.

## Canonical state and native layout

The game allocates58h bytes before004A43C0 and publishes the returned owner
before calling0049D690. `TrafficConfig` is the canonical C++ projection. The
integration caller supplies `TrafficConfigAllocationWords`, constructs the shell,
then calls `construct_traffic_config_004a43c0`. The constructor does not initialize
the old group pointer+8, word+C, or any of the six allocator/comparator words.
Those unconsumed words are explicit inputs, not invented zero defaults.

| Native offset | Recovered storage |
| --- | --- |
| 00 | Vtable00CE68D0 |
| 04 | Pointer to actual1Ch `TrackedCriticalSection` |
| 08 | Pointer replaced by loader with a fresh20h group |
| 0C | Untouched by constructor; loader writes zero |
| 10 | Actual0Ch runtime tree header;18h sentinel |
| 1C | Actual0Ch runtime tree header;24h sentinel |
| 28 | Actual0Ch runtime tree header;18h sentinel |
| 34 | Actual0Ch runtime tree header;1Ch sentinel |
| 40 | Soldier pair map, pooled string to pooled string |
| 4C | Vehicle pair map, pooled string to pooled string |

Each runtime header is allocator-word/head/count. Sentinel construction first
writes null left,parent,right, color1 and isnil0. The owner then publishes head,
writes isnil1, sets parent,left,right to head in that order, and zeroes count.
The corresponding color/isnil byte offsets are14/15,20/21,14/15,18/19. Payload
and padding bytes remain untouched. These are actual allocated headers and
sentinels; no runtime value types or separate shadow containers are invented.
The first four headers also retain their native owner offsets in the Win32
projection, checked at compilation.

The two string maps use the existing `std::map<NativeString,...>` semantic
convention. Their native sentinel size is20h; key+C, value+14h, color1C, isnil1D.
The optional C++ maps engage in native constructor order, after the four runtime
trees. Their nodes, iterator ABI, sentinel allocation timing and allocator-word
layout are not native replacements. The critical section is created last using
the existing concrete00BD1860 implementation. No lock is acquired in this loader.

## Loader ordering and ownership

0049D690 allocates20h and inlines group construction. The actual `TrafficGroup`
layout is32 bytes: vtable00CE6600 at0; words4/8 zero; untouched allocator word+C;
12h list sentinel pointer+10; count+14 zero; raw float bitsD01502F9 at+18;1.0 at+1C.
The sentinel's next/previous point to itself, while its payload at+8 is untouched.
The name `TrafficGroup` is provisional; these addresses alone do not establish
the semantic role of the two float fields or its runtime elements.

Group construction completes before the owner+8 store, then+C is zeroed. This
occurs before Lua owner construction/open(mask1), so bootstrap callbacks observe
the new group. Each call replaces+8 without deleting the old group. A caller
that intends to reclaim an earlier empty group must retain it explicitly.

The loader opens the existing realLua5.1 owner, constructs the37-byte path with
preserve=true and a terminator copy, and invokes the existing script-with-overrides
runtime. It releases that pooled path before getting globals/PartyPairs. The
globals temporary is released immediately after PartyPairs lookup. It retains
PartyPairs, key, value and current-table references while processing Soldiers,
then assigns the current table from Vehicles and repeats. The current table is
released before value, key and PartyPairs, and all references retire before close.

For each entry the native code unconditionally scans the key C string, constructs
its pooled copy including the terminator, captures the value's nullable Lua
C-string pointer, and calls0049C9C0. Only after that map lookup/insertion returns
does it measure the captured value length. Null conversion means an empty value;
there is no fabricated source text. Value resizing preserves no old contents.
The assignment writes exactly the current destination length after resize.

The Soldiers loop rereads the temporary key header for cleanup. The Vehicles
loop retains the earlier key pointer, except that the positive-length allocation
arm reloads it after value resizing. Both rules are represented explicitly.
Pooled key cleanup precedes the next lua_next step.

## Map helper closure

00485790 performs lower-bound using the existing00443D00 empty-string/CRT
case-insensitive ordering. It does not sort by counted length.0049C9C0 returns
the existing mapped string at native node+14h when equivalent. An absent entry
constructs a temporary pair with a copied key and an explicitly empty mapped
string (0048E1F0), then copies both into a node (00490990/0048E500).00489D50
releases temporary value before key after publication. The source reuses the
existing actual-header pooled copy/destruction helpers for those operations.

The standard map implements the audited lower-bound/unique hinted-insertion
semantics of00499480. The mapped string starts empty before the loader allocates
its value buffer. Existing key spelling, node/value identity and omitted entries
survive subsequent loads. Library balancing, invalid-iterator continuation,
allocation exception timing and extreme native max_size behavior are outside
the semantic projection. Normal containers and stable valid iterators are required.

00492750 currently has the misleading label `STL_xlen_throw_00492750`. Inspection
shows a returning node-link/rebalance routine with a length-error arm; this packet
neither owns nor reconstructs that whole library routine. The label issue is
reported for central annotation rather than changed by this worker.

## Lifetime boundary

`release_empty_traffic_group` and `release_traffic_config_startup_storage` are
explicit host cleanup conveniences, not native-destructor reconstructions. They
reject a populated/noncanonical group or any populated runtime tree before any
owner mutation. On accepted startup-only state they release the critical section,
empty group, vehicle pairs, soldier pairs and four sentinel allocations. Every
pooled mapped value and key is released. The first four runtime payloads and the
group's active element lifetimes remain separate reconstruction work.

The native owner destructor0049F0E0 has a false CALL_RETURN after _free at0049F14C;
its continuation and remaining runtime clears were not reconstructed. The group
destructor00489DD0 can invoke004872D0 and virtual deletion of linked runtime entries.
Neither is disguised as a no-op or represented as completed. Constructor-only host
cleanup additionally requires its supplied old group pointer to be valid or null.

## Evidence and verification

`reports/traffic_config.json` records original ABI, inclusive final instruction,
last instruction length, decoded counts and SHA256 for bounded bodies. Each
listed body was independently compared between the installed executable and live
Ghidra bytes in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The worker
made no Ghidra writes.

004A43C0 ends004A44FA RET;0049D690 ends0049DBF6 RET;0049C9C0 ends0049CA73 RET4.
The loader's sole exported gap0049DA28..0049DA2F decodes as JMP0049DA30 and LEA
EBX,[EBX]. It is skipped by the preceding branch to0049DA32 and by the loop back
edge to0049DA30; no loader flow repair is required. No missing entry definitions
were found in this packet's reconstructed bodies.

Standalone `scripts/build.ps1` passed for MSVC Win32 Release, including the existing
reconstructed_math CTest. One ignored fixture, `local/traffic_config_fixture.cpp`,
uses actual Lua, the real script runtime and an explicit file provider. It verifies
sentinel shape and scratch preservation; group publication before bootstrap;
Soldiers-before-Vehicles lookup; empty mapped publication; case-insensitive key/
value identity; persistence on reload; old-group retention; rejection before
cleanup on populated runtime state; and balanced pooled strings. The parameterized
`local/run_traffic_config_fixture.ps1` can relink against an integration tree.

Inputs require nonnull string-convertible keys and tables stable during lua_next.
Numeric-key conversion/iteration has not been separately validated; the existing
registry-backed Lua adapter remains the boundary. Native SEH and exact allocation
failure transport are not reproduced. These are new C++ interfaces, not a drop-in
native ABI or game-runtime validation claim.
