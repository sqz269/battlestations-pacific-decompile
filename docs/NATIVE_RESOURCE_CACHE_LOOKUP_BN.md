# Actual resource-cache lookup BN

The two complete lookup bodies now have independent C++ source over actual
Win32 tree and string storage. They reuse the existing raw-header
`less_native_string_headers_00443d00` comparator. Similar source at
`B19B90/B19D60` concerns different native addresses and is not used as a
lookup alias. Descriptive names remain hypotheses.

| Routine | Native ABI | Coverage |
|---|---|---|
| B7DFA0..B7DFF2, 83 bytes | ECX tree; stack query; EAX node; RET4 | Complete ordinary body |
| B7E7B0..B7E815, 102 bytes | ECX tree; stack output/query; EAX output; RET8 | Complete ordinary body |

Source interfaces are `lower_bound_native_resource_cache_name_00b7dfa0` and
`find_native_resource_cache_name_00b7e7b0`. Find receives the existing explicit
`SingletonLifetimeCallbacks` returning-CRT service. There is no new allocator,
manager, map, cache snapshot, resource reference operation or dispatch stub.

## Storage and comparison evidence

Tree+4 is the head and head+4 is the root. Producer `B7F220` writes node links
at +0/+4/+8, owned key length/data at +C/+10, borrowed resource at +14, color
at +18, and nil at +19. The node allocation is 1Ch. These match the prior
[ownership audit](RESOURCE_CACHE_OWNERSHIP.md) and
[classification/erase audit](RESOURCE_CLASSIFICATION_AND_CACHE_ERASE.md).
Lookup does not read allocator word+0, count+8, color, or mapped resource.

Stored length0 means empty regardless of the data pointer. Empty sorts before
nonempty. Two nonempty C strings compare using the current CRT `_stricmp`,
with right data captured before left data, no stored-length bound and no
length tie-break. `B7DFA0` inlines exactly that ordering at B7DFB5..B7DFD8;
the independent source body uses the existing 443D00 source comparator.
Find calls native 443D00 directly at B7E7E1 and consumes AL. Its rejection
check is `query < candidate`, following lower-bound's `node < query` search.

## Current and captured storage

Lower-bound captures head and root once on entry, then walks current child
pointers and nil bytes. A less-than key descends right without replacing the
candidate; otherwise capture that node and descend left. Return the first
not-less-than candidate or the initially captured sentinel. An empty tree
does not read the query header.

Find first calls lower-bound and captures its result. Only afterward does it
check tree-null through `BF6713`, whose body can return. No earlier null
validation or invented fallback is inserted. The source continues after a
returning handler, as the native listing does; a null tree still requires the
native raw-memory preconditions for the preceding lower-bound dereference.

Find compares the captured candidate against **current** tree+4. On a miss or
greater candidate it reloads **current** tree+4 again for the result. It then
captures both selected owner/node words before publishing output+0 followed
by output+4. Output may overlap tree storage, a candidate key or the input
header. It returns the originally supplied output pointer. No post-store read
of the tree or selected node is substituted for these captures.

The current direct callers are:

| Call site | Caller | Setup |
|---|---|---|
| B7E7BD -> B7DFA0 | B7E7B0 | ECX captured tree; one pushed query; callee RET4 |
| B801D5 -> B7E7B0 | B801C0 | ECX manager+14; local iterator/query; callee RET8 |
| B8075D -> B7E7B0 | B80720 | ECX manager+14; local iterator/original name; callee RET8 |

The two manager callers establish use of the actual resource cache; they do
not make manager lifetime, insertion, loading or deletion source-complete.
This packet does not change those callers or production routing.

## Verification and limits

Every Ghidra batch verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 language and image base400000. Both full
listings were reviewed: 36 and 39 instructions, respectively, with zero gaps.
No native body fragments or `no_ghidra_function` intervals remain inside the
owned routines. No Ghidra writes were made. Four fresh installed/live byte
spans pin both bodies, the comparator and the layout producer. The report
records every owned direct call and both manager call-site rows for the
repository's live call checker.

Strict MSVC Win32 C++17 compilation passes with `/EHsc /W4 /WX /O2 /fp:strict
/MD`. The ignored fixture compiles the new TU and borrows the explicitly
pinned integrator library from `battlestations-pacific-decompile-orch4-20260910`:
`134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`.
Pre-copy, post-copy and frozen-copy hashes agree. Eleven source/header inputs
and all linked libraries are retained with hashes under `local/cache_lookup_bn`.
The probe links with `/MANIFEST:EMBED` and runs reconstructed source only.

The single raw-storage fixture passes empty markers with invalid data pointers,
case-equivalent nonempty strings with different stored lengths, greater and
missing lower bounds, unchanged tree/input/resource storage, and output aliases
over tree+4, a candidate key and the input header. An empty tree accepts an
unused invalid query-header pointer without reading it. The fixture uses the
C locale. Returning null-tree CRT continuation is listing-verified only; it
does not fabricate mapped memory at address4 to force that path.

The checklist's callee, boundary, direct-caller, producer, coverage, argument,
register, lease and report rules are satisfied by the linked evidence. Its
frame-runtime rule is not exercised: these APIs are not bound into a running
game route and this packet makes no frame or gameplay claim. No permanent
tests, full CMake build, manager/loader integration or shared metadata edits
were added; the primary owns integration and Ghidra evidence annotation.

The APIs carry additional source services and differ from original stack/SEH
ABI. Valid raw backing memory and current CRT behavior remain caller
requirements. Arbitrary aliases of the original stack/register spill locals,
concurrent mutation, native exception equivalence and gameplay are unproved.
