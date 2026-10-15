# Ordinary typed array and three-word property providers

This packet reconstructs three complete ordinary successful bodies through new
C++ interfaces in `native_scene_typed_array_vector_providers.hpp/.cpp`. Descriptive
names are hypotheses. It supplies no Original class, private-CRT, register ABI,
exception transport, parser startup or game equivalence.

| Original interval, exclusive end | Bytes / instructions | Entry and result |
| --- | --- | --- |
| `008F3590..008F3604` | 116 / 36 | ECX actual bag; stack key/data/count/full flags; RET10h; EAX bag |
| `008F3610..008F3684` | 116 / 36 | Same transport; raw type11 constructor |
| `008F3690..008F3701` | 113 / 41 | ECX actual bag; stack key/borrowed12B input; RET8; EAX bag |

All 345 disk/live bytes match the complete existing exports, totaling 113
instructions, nine direct CALL sites and four immediate targets. There are no
indirect calls or x87 instructions. Cached Ghidra void/no-parameter prototypes
were not accepted as transport evidence; the complete listings establish the
ordinary entry/return roles. The sealed readiness family remains unchanged at
`local/TypedPropertyNextProviderReadiness01` and is copied into this packet's
`local/WholeSceneTypedArrayVectorProvidersSource01/frozen_readiness`.

Both arrays allocate exactly 38h bytes through the accepted canonical
`singleton_lifetime_allocate({object,0x38,0x38})` service. For a nonnull result,
type10 calls whole raw `008EF3E0`, type11 whole raw `008EF460`, with
**count/data/full DWORD flags**, then whole actual bag insertion `008F33F0`.
The caller-facing order stays key/data/count/flags. The full flag word is passed
unchanged. Only its low byte selects retention versus copy inside the accepted
raw constructor: upper bits alone do not request a copy.

The raw constructors write 29 bytes and preserve 27, including +30 until
insertion. No root zero-fill or invented initialization is added. Type10 accepts
positive count through 3FFFFFFFh and copies/retains exactly 4*count opaque bytes;
type11 accepts positive count through 15555555h and uses 12*count bytes. Their
existing stable-span, disjointness, nonwrapping and successful-provider limits
remain. This packet adds no count guard, numeric conversion or generalized
parsing policy. If root allocation returns null, the wrapper still passes null
to insertion, preserving the Original path; ordinary admission requires
successful allocation and supplies no fault recovery.

A copied child becomes part of the incoming owning graph; the original input
stays caller-owned. A retained pointer must already identify a sole-owned genuine
allocation from the same current CRT. The caller manages it until successful
publication transfers it with the root. The raw retention constructor alone
does not transfer ownership. Borrowed, stack, static, shared and foreign payloads
cannot become retained owning array inputs: tag10/11 release unconditionally
frees any nonnull stored payload, independently of the copy flags and +2C.
That byte equals one in both constructor branches and grants no free permission.

The three-word wrapper uses the same genuine 38h allocation and explicitly
reuses accepted whole raw `construct_native_scene_property_record_type7_storage_008ef270`.
Original `008F3690` inlines these effects; it does not call that constructor.
Both implementations store phase/tag7 before three interleaved input DWORD
reads and root stores at +C/+10/+14, zero +18/+1C/+20/+24/+30/+34 in order,
then store byte1 at +2C. They write 45 bytes and preserve the same 11 bytes at
[08,0C), [28,2C), [2D,30). The input stays borrowed, is never retained/freed,
and transfers no ownership. Tag7 has no separately owned input allocation.
No float-valued formal, arithmetic, ST0, SSE or MXCSR operation is introduced.

The raw helper uses different scratch register/flag effects and adds a Source
call/return. Its final +2C store precedes wrapper argument staging, whereas
Original stages arguments before that store. Actual memory-effect order is the
same. Stable disjoint input/root/frame and the call-free, nonthrowing raw helper
make those staging differences unobservable within this ordinary domain. This
reuse supplies no Original ABI or class claim. The full comparison is retained
in `type7_reuse_static_review.DATA.json`.

All three wrappers publish through the existing actual-storage insertion and
lifetime context. Require a genuine raw61-produced live 114h bag and its real
108h map at +4. The same retained concrete string/physical processes provide
the actual E175B0 pool, E188B4 allocator-list domain, raw key context and
E177E4/E17654 fallback cells. Explicit pool startup precedes context construction;
these wrappers start nothing. Owning graphs are unique, finite, unshared,
acyclic and disjoint from any replaced graph, key/query, map/node and frame.
Successful publication transfers incoming root/child ownership. Retire actual
payloads/records, then keys/nodes before pool teardown; keep processes, cells,
generations and the common list live and externally serialize list mutation.

The two Original array frames install handlers CA496B/CA498B, retain allocation
words and use state0 across construction, returning to state-1 before insertion.
Their FuncInfo records DD7618/DD7644 reference one-entry maps DD7610/DD763C;
cleanup actions CA4960/CA4980 load [EBP-10] and call BF65AC. Disk/live records
are frozen in `Original_EH_review.DATA.json`. New Source adds no native FS frame,
constructor-unwind free, cleanup transaction or rollback. Provider throws,
failure, faults, aliasing, overflow, reentry and concurrent mutation remain
outside the ordinary successful contract.

The first and only strict compile returned `2f4341/exit0` with MSVC Win32
19.51.36244 and `/std:c++17 /MD /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`.
The 2787-byte object SHA256 is
`5cad4a34b34f3a98733954bb2f22b01e711890028e7f58aaa6ea2e156d60c165`.
Complete review covers all seven sections, 27 symbol indices including
auxiliaries, three code sections/378 bytes/141 instructions, all 15 code
relocations and zero metadata relocations. The two arrays each emit 132B/49
instructions; the three-word wrapper emits 114B/43. Full DWORD argument loads,
null paths and literal provider bindings are preserved. Real security-cookie
bindings, literal MD_DynamicRelease/MSVCRT/msvcprt/OLDNAMES directives, debugS,
weak AVX datum, checksums, complete string table and every file byte are retained.
No symbol or relocation normalization was used. Compiler-reported 86 source/
include inputs and the actual tool files are pinned.

The required worker `./scripts/build.ps1` ran once with original launch
`6ca04e/session57901` and natural terminal `9575c9/exit0`; both existing checks
passed in 7.70 seconds. Its unrelated existing LNK4006 duplicate
`spawn_request_id_matches` warning is retained in the log. The new TU is
intentionally unregistered, so this repository build is separate from its strict
compile gate. Root owns CMake/metadata/analysis integration and independent/Main
admission. No new tests, probe executable, runtime replay or game launch occurred.

Original returned compiler/build tool objects are bound literally to the
worker's known raw rollout UUID `01a14050-c68c-7302-a3db-435713ee4394`, with a
frozen prefix through line2863 (7832558 bytes; SHA256
`e5df250f60a0cf95d4029d3e099a314a51bb7bd3f39b6a98ba58d75c5a7b041a`).
The full Source/strict review, precompile inputs, compiler dependency manifest,
tool receipts and provenance live in the owned local packet directory.
