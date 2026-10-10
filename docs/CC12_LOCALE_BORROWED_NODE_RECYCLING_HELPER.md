# CC12 borrowed locale node recycling helpers

Two dormant normalized helpers now return an actual locale node slot and perform
payload-first scalar deletion. They borrow an already initialized locale pool and
reuse the accepted raw payload helper. No owner construction, slab producer,
profile trim, map caller or application registration is added.

The implementation is in
[`native_locale_map_node_recycling.cpp`](../src/native_locale_map_node_recycling.cpp),
with its public preconditions in
[`native_locale_map_node_recycling.hpp`](../include/bsp/native_locale_map_node_recycling.hpp).
One exact `bsp_core` source entry is added to `cmake/startup.cmake`; all other
entries are unchanged. The [machine report](../reports/cc12_locale_borrowed_node_recycling_helper.json)
retains full compiled sections, relocations, normal flow and validation receipts.

## Authority and native contract

This packet consumes the accepted [provider contract](CC12_LOCALE_NODE_PROVIDER_PRIMARY.md)
at `00736090`/`0073C220`, the unchanged
[payload helper](CC12_LOCALE_MAP_RAW_NODE_PAYLOAD_HELPER.md), and the bounded
[profile/unwind design](CC12_LOCALE_POOL_PROFILE_UNWIND_READINESS.md). It makes no
fresh Ghidra query, GPR mutation or ledger edit. The retained 105-byte provider and
32-byte wrapper are dependencies, not newly recovered bodies.

The API takes the caller's actual initialized `E1768C` pool and actual allocated
slot. The pool must have its live OS critical section at `+0C`, coherent current
table/block identities, and valid free-index storage. A `void*` view or zeroed
buffer cannot establish construction, membership or canonical authority. The
helpers create no replacement owner, registry, domain or publication cell.

`NativeStringRawPoolContext` is reused unchanged and borrows the same actual
`01090AA8`, `01090AA4`, `01090AA0` cells. Payload and borrowers must finish before
pool teardown. Stable Enter/Leave imports are a factoring precondition: the
accepted map caches Enter, while the native provider reloads it per call.

The earlier readiness report's proposed next-packet metadata had two incorrect
labels. Its optional `allocator_registry.hpp` path should be
`include/bsp/allocator_list.hpp`; these helpers need neither a domain API nor a
domain edit. Its `00E18AA8/00E18AA4/00E18AA0` dependency label should be the
`01090AA8/01090AA4/01090AA0` cells named by the existing raw context. The old proof
is preserved unchanged; these corrections do not alter its native profile or
unwind evidence. No incorrect literal pointers appear in this implementation.

## Preserved schedule

`return_native_locale_node_slot_00736090` follows the native member routine
(ECX pool, stacked DWORD slot, RET4) through a new C++ interface:

1. Compute section `pool+0C`, enter it, then increment CURRENT depth DWORD `+24`.
   Retain this recursive Enter/depth/Leave even when a map caller holds the lock.
2. Only now read slot identity DWORD `+18`, CURRENT pool table `+28`, and the
   indexed block. Retain that captured identity and block pointer.
3. Subtract address bits modulo 32 bits, use `memcpy` to reinterpret signed32,
   divide by 28 toward zero, then narrow modulo 16 bits. There is no unrelated
   pointer subtraction, signed-overflow expression or guessed slab extent.
4. Capture WORD count at block `+7800`; write the index at
   `block+7000+2*captured_count`. Then increment CURRENT WORD `+7800` with an
   actual memory ADD, preserving the read after the free-index write.
5. Compare the captured block identity with CURRENT unsigned minimum `pool+34`,
   lowering it only when smaller. Decrement CURRENT depth before Leave.

Raw x86 DWORD/WORD accessors preserve these accesses without introducing a typed
shadow pool. There are no null/bounds guards, metadata clearing, payload cleanup,
unlock guard, rollback, block reclamation or physical node free.

`scalar_delete_native_locale_map_node_0073c220` follows the native ECX-node,
stacked-flags, EAX-original-bits, RET4 wrapper through a new C++ interface. It first
calls the existing `destroy_native_locale_map_node_payload_00737ce0` with the
original node and raw string context. Only after normal return does a volatile
low-byte read test flags bit zero; other bits are ignored. When set, it returns
that same node to the supplied actual locale pool. There is no early slot/table
read or pool access on the flags-clear path. The result retains original address
bits and gives no live-object guarantee.

A propagating payload exception prevents the flag read and slot return. The new
wrapper adds no local cleanup or rollback. Ordinary C++ payload unwind keeps the
existing payload helper's limits. The new functions are not original binary
entry replacements; register/stack ABI, fault timing, FH3, SEH, exception identity
and double-failure equivalence remain unproved.

## Source and compiler evidence

The clean baseline is `75db5de51b296781b1a73915bf47081d5c088519`, after merging
the instructed Main `98b511361`. Its 4,215 Source inputs match actual committed
Git blobs. Relative to the preserved profile epoch, only the two already accepted
render-context files differ; that earlier Source epoch is not relabeled.

The final 4,217 Source inputs match actual staged Git blobs byte-for-byte. Only
the two helper files are new; the sole existing-input change is the one CMake
line. Seven dependency document/report pairs retain 14 actual Git blobs. The
existing payload, string and allocator implementations remain unchanged.

MSVC uses toolset directory `14.51.36231`; the generated compiler record reports
version `19.51.36244.0`, architecture X86. These are distinct identifiers. Compiler
binaries and generated project settings are retained. C++ exception handling is
`Sync` (`/EHsc`), and the new TU uses the existing Release configuration.

The complete new COFF object contains a 254-byte public slot-return section and
a 283-byte scalar-wrapper section. The compiler inlines slot return into the
wrapper while retaining its standalone public definition. In that wrapper,
payload CALL is at `+0E`, flags-byte load at `+13`, bit test at `+19`, and Enter
at `+2A`. Both compiled paths retain signed division by 28, WORD narrowing,
free-index write before CURRENT WORD ADD, unsigned minimum comparison and depth
decrement before Leave. Complete normal flow and direct branch destinations are
checked from section bytes. No local FH3/terminate symbol or `.sxdata` is emitted
in the new object; this is compiler evidence, not native exception equivalence.

The actual Core linker symbol index resolves both public definitions to the new
member, the wrapper's payload reference to `native_locale_map_node_lifetime.obj`,
and that payload's raw wide/narrow references to their existing members. These
are positive archive provider records, not a claim that the new dormant helpers
were selected into an application image.

Core grows from 1,989 to 1,990 objects. Every old object payload is byte-identical.
The worker's actual App directory remains 80 objects, all byte-identical; this
is a finite directory census, not Root/project membership evidence. The new
header has one recorded compiler consumer, its own TU. A finite scan of 4,210
current `include/`/`src/` files finds five API-name occurrences, all in the new
header/implementation. No application caller is introduced.

The three retained executables (`bsp_game`, `bsp_math_tests`,
`bsp_native_math_tests`) each differ from baseline in exactly four bytes, all in
the PE and debug-directory timestamps. All remaining bytes match. Core archive
bytes change with the new member. These artifact facts do not establish runtime
behavior or canonical owner availability.

## Validation and remaining gates

Fresh baseline and final `./scripts/build.ps1` runs pass the normal MSVC Win32
Release build and all three existing checks: `reconstructed_math`,
`native_math_differential`, and `tool_tests`. The first post-edit build also passed;
the final rebuild followed only physical CRLF-to-LF normalization so compiled
Source matches staged Git bytes exactly. No test, ad hoc executable or runtime
case for the new helpers was added.

The proof ZIP is
`local/cc12_locale_borrowed_node_recycling_helper/evidence.zip`: 87,665,404 bytes,
4,573 payload files / 482,457,983 uncompressed bytes, SHA-256
`d734df05c47cfc96f41a3a9badcb69ac8da621d1f6dad45fec0107fce4e43c2e`.
A guarded read-only replay verifies manifests, Source/Git objects, dependency
blobs, retained original PE slices, complete Core/selected COFF data, positive
linker providers, App census, consumer records, build logs and timestamp deltas.
An independently copied ZIP produces the same PASS with external reads,
process/network calls, dynamic loading and directory access denied during replay.

This is Source implementation, build and static artifact evidence. Actual
canonical pool storage/lifetime, slab/slot production, profile binding and trim,
unwind action semantics, payload-before-pool teardown, map caller integration and
application/game validation remain separate gates. No owning lifecycle, startup,
caller registration or application GO is claimed.
