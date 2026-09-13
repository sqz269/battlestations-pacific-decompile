# MPAK binding frontier: BB discovery

Addresses: 00bb7a20, 00bb7ba0, 00a40d60, 00bb6180, 00bb4140, 00bb4f40, 005efba0, 00bb7240

This read-only packet reconstructs **zero bodies**. It identifies the concrete
contracts still needed between the actual MPAK source runtime and startup. The
BA record copies are present at inspected commit `18a137b2`; they do not supply
the parser's STL operations. The existing registered startup MPAK token still
creates no provider. No production library binding, installed archive or game
path was executed by this discovery.

Target-verifying CLI queries used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Seven complete bounded spans, **837 bytes**, match
the installed PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report preserves native call rows, source hashes, retained evidence paths,
and the distinction between complete small bodies and partial growth analysis.

## Actual consumers and remaining contracts

`NativeMpakRuntime` composes the actual factory/provider/parser, lookup, entry,
stream and numeric VFS dispatch packets. Its constructor borrows contexts and
temporarily installs pointers in the existing VFS/lookup/conversion objects.
It does not instantiate any of these required library services:

| Interface operation | Native contract and current consumer |
| --- | --- |
| `append_file_00bb7a20` | ECX actual10h vector, one source24h pointer, RET4; parser BB7C50 appends its temporary file record |
| `append_directory_00bb7ba0` | Same ABI, source14h record; parser appends an initially empty directory before filling its name and members |
| `insert_offset_00a40d60` | ECX10h DWORD vector; result8h iterator, where-container, where-pointer, value-pointer; RET10h, EAX result iterator. Parser invokes it at current end only when no spare capacity remains |
| `invalid_parameter_00bf6713` | Returning CRT diagnostic contract; parser and BB5080 may reload repaired storage after it returns |
| `destroy_file_range_00bb6220` | ECX begin, EDX end, vector/owner stack words, RET8; provider destruction releases elements, then source frees current backing |
| `destroy_file_vector_00bb6e60` | ECX10h file vector, RET; provider constructor unwind owns this cleanup |
| `destroy_directory_vector_00bb71f0` | ECX10h directory vector, RET; normal provider destruction and constructor unwind |
| `copy_00bb6180` | ECX destination10h, source10h, EAX destination, RET4; BA file copy BB65A0 invokes it for record+14h |
| `file_at_00bb4140` | ECX file vector, unsigned index, EAX actual24h record, RET4; BB5BB0 cached-index path. Diagnose missing/out-of-range backing, then reload current begin before calculating the result |
| `find_member_directory_00bb4f40` | ECX first14h row, EDX captured end, key14h pointer, EAX row/end, RET4; BB68F0 invokes the library search over member lists |

The first seven operations belong to `NativeMpakContainerLibrary`; the last
three have separate offset-copy, checked-access and search interfaces. Searches
of tracked source/header/tool/test files found declarations and consumers but
**no concrete production implementation** of any of these four interfaces.
`NativeMpakRuntimeInputs` does not itself consume offset-copy: the compatible
container implementation must route element copy construction through BA's
`copy_construct_native_mpak_file_00bb65a0` and its offset library context.

All outer STL headers have opaque allocator/debug storage at +0 and actual
begin/end/capacity pointers at +4/+8/+C. File elements are24h, including pooled
name8h, two DWORDs, byte flag, three untouched padding bytes, and a nested10h
STL offset vector at +14h. Directory elements are14h and embed the **different
custom0Ch** string vector at +8h. No sidecar container may replace these live
headers: the parser itself writes offset elements and advances vector end on
its spare-capacity path; open, enumeration and entry code directly read them.

One allocation/free and pooled-string lifetime domain must span temporaries,
container-owned records, provider cleanup and BA copy callbacks. A binding must
preserve name-only constructor cleanup on nested-copy failure, exact publication
order, allocator/padding bytes, and native returning diagnostics. The genuine
library operations remain library coverage; binding them adds no reconstructed
STL bytes. Valid-input parse success alone cannot establish these contracts.

## A current-library candidate was checked, not accepted

One local ad hoc Win32 probe instantiated the installed MSVC14.51.36231 STL
with iterator debugging0. It compiled with strict warnings and an embedded
manifest. No original code was executed and no permanent test was added.

| Instantiation | Header / iterator bytes | Data-pointer offset | Maximum counts for DWORD / file / directory |
| --- | --- | --- | --- |
| Default `std::vector<T>` | 12 / 4 | 0 | 3FFFFFFFh / 071C71C7h / 0CCCCCCCh |
| `std::vector<T, TaggedAllocator<T>>`, four-byte state | 16 / 4 | 4 | Same three limits |
| Native reviewed contract | 16 / 8 checked iterator | 4 | Same three limits |

The stateful allocator is an ABI investigation candidate, **not** a compatible
binding. Matching size and limits do not establish object lifetime, opaque-word
preservation, invalid-parameter behavior, native checked iterators, allocation
callbacks or exception ordering. There is a concrete ordering difference:
native BB7240 copy-constructs an alias-protecting temporary at BB726D before
allocation BB733D, copies the old prefix at BB7360, then constructs inserted
elements at BB7370. The installed STL's vector reallocation implementation
allocates, constructs the inserted element, then copies/moves old elements
(`vector` lines884..910). Directly delegating to its `push_back` changes observable
allocation/copy order. Reinterpreting the raw10h header as this vector is not
accepted. Adding a hand-written growth algorithm would violate the library
boundary; relocating an oracle is not an already-proved runtime library.

A full binding packet therefore first needs a supplied/pinned compatible
library implementation and an acceptance comparison of those schedules. The
repository search and this one installed-toolchain probe establish no such
implementation. They do not establish that none could exist elsewhere.

## Ready independent game-owned packet

`005EFBA0..005EFC15`, **118 bytes**, is the shared custom-string-vector lookup
used as the game predicate by BB4F40's find-if-style library loop. It is not an
STL growth/copy routine. Its one library dependency is retained CRT `_stricmp`
at BF7FBF; no allocation, cleanup or EH frame is present.

ABI: ECX actual0Ch vector; one stack pointer to actual8h key; EAX signed index
or -1; RET4. Capture data and unsigned wrapping end `data + count*8` at entry.
Walk while unsigned cursor<captured end. For each8h row, compare the current
stored lengths; empty/empty is equal without data-pointer access, otherwise
call CRT `_stricmp` with the current row/key pointers. A match returns arithmetic
shift of wrapping `(cursor - CURRENT vector.data)` by3. Reloading vector data
for the result is observable across the library call. Do not replace this with
a length-bounded ASCII comparison, refresh the captured end, guard null data,
or substitute a semantic string-vector snapshot.

All **five live call sites in four functions** were inspected: BB4F54 uses
row+8; BE84C4 and BE90B3 use owner+4; 5F4375 and5F43B2 use the options screen's
custom vector. Every site passes ECX vector and one key-header pointer. Snapshot
lookup lists a fifth caller BB4D60; live xrefs/callers do not. This stale graph
edge is excluded from the verified call set, not silently counted as current.

Proposed ready packet `orch2_native_string_vector_lookup_bb`: lease005EFBA0
and disjoint `native_string_vector_lookup.hpp/.cpp`, its doc/report and owned
ledger rows; reuse `NativeStringVectorStorage` and `NativeString`, retain CRT
`_stricmp`, and register only its own deferred CMake source. The address was
unleased when checked; recheck before writing. Build with the existing strict
Win32 build and two existing CTests. A subsequent library-search binding may
use this predicate, but this helper alone does not remove BB4F40 or any other
production library interface.

## Integration order and evidence required

1. Reconstruct the118-byte shared lookup independently; do not edit BA's shared
   vector implementation or claim it closes the archive path.
2. Establish the compatible library implementation for all ten operations.
   An acceptance fixture must cover spare capacity and growth from populated
   vectors, actual BA copy/destructor callbacks, record aliasing, pointer/end
   publication, padding/allocator preservation, matched frees, a returning
   invalid-parameter repair, and cleanup after partial construction. Freeze
   original outcomes independently before candidate execution. Existing BA's
   fixed three-DWORD test offset contract is not this production binding.
3. Compose it with one existing `NativeMpakRuntime` and the actual manager,
   physical/memory stream, pool, registry, lock and conversion contexts. Exercise
   actual BB7C50 parser -> BB5BB0 open -> BB5080 uncompressed/compressed branches,
   cached reopen, enumeration/resolve and provider destruction. Do not recount
   existing source-composition methods as recovered bodies. Full library EH
   interoperability remains separate from source C++ exception comparison.
4. Wire the real startup owner. `game_hosts_vfs.cpp:110..124` registers the MPAK
   factory token and still says it creates nothing; `VfsProviderFactories` only
   creates physical, FileStore and MPKG mounts. Its old comment that the MPAK
   body is unreconstructed is stale relative to current raw source. The new
   binding must reach the actual factory/runtime; a token or another successful
   mock provider does not close this gap. Shared VFS/startup edits require their
   current owner's coordination and lease.

The chosen game-installation path had no loose `.mpak` or `.mpkg` files in a
case-insensitive extension scan. This does not prove there are no embedded,
renamed, extracted or externally installed assets. Before an installed-path
claim, identify and hash an actual MPAK archive and its startup mount trigger;
synthetic archive success is only fixture evidence. Final candidate build,
runtime reachability and gameplay validation belong to the integrator.

Retained evidence: `local/mpak-binding-frontier-bb-evidence/manifest.json`,
the seven native byte/listing captures, five caller-site excerpts, current STL
probe source/log/output/compiler-input hashes, and capture helper. The one
initial disassembly-page request began inside an instruction and was rejected;
its error is retained, then the page was read from an actual instruction start.
No Ghidra writes, project saves, exports, library ports or production edits were
performed by this discovery.
