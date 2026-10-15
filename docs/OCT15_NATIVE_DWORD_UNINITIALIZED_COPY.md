# Native DWORD uninitialized copy

Addresses: `00696D40`, `00697860`.
Source: `src/native_dword_uninitialized_copy.cpp`.
Header: `include/bsp/native_dword_uninitialized_copy.hpp`.
Report: `reports/oct15_native_dword_uninitialized_copy.json`.

| Established native name | Coverage | Source function |
| --- | --- | --- |
| `STL_UninitCopy_00696d40` | complete scalar loop and output result | `copy_native_dword_uninitialized_00696d40` |
| `STL_TagDispatch_00697860` | complete dispatch and result propagation | `copy_native_dword_uninitialized_dispatch_00697860` |

The established STL analysis names are preserved. These leaves operate on actual
four-byte scalar storage, with no allocation, owning object, diagnostic,
constructor/destructor service or owner-unwind action. No generic vector or
modern copy service is introduced.

## Native ABI and access order

The core receives first in ECX and last in EDX. Its four physical stack DWORDs
are destination (`ESP+4`), owner (`+8`), opaque metadata (`+C`), and tag (`+10`).
`RET 10h` removes all four; EAX returns the advanced destination. Only the first
stack word is consumed. The native code compares first/last, loads destination
even when they are equal, then branches using the preserved comparison flags.
An empty range returns that destination without accessing pointed memory.

For a nonempty range, each nonzero destination reads the CURRENT source DWORD
immediately before its store. A zero destination skips both accesses. Both
cursors still advance four with DWORD wrap, until first equals the captured
last. There is no count conversion, value snapshot, null-source check, overlap
rejection or range guard. A forward overlap retains the progressive values
written by preceding iterations; a memmove substitute would change that.

The dispatcher receives ECX owner, unconsumed EDX, and stacked first, last,
destination. `RET 0Ch` removes those three words. It passes the core destination,
owner, duplicate destination bits, and a dead tag made from owner bits with only
the low byte cleared. It propagates the core EAX result directly. The independent
count-based return used by the fill_n dispatcher does not belong here.

The third core stack word is deliberately exposed as an opaque DWORD. Both
dispatchers supply destination bits there, while direct general-insertion
producers supply their requested count. Neither form is read by the core.
Likewise, the direct producers' high tag bytes come from native private frame
words; only their low byte is explicitly cleared. No ownership or callback
meaning is assigned to those dead words.

## Current body and producer proof

Read-only batches verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before/after their queries. Complete current bodies
are `[00696D40,00696D66)` (38 bytes,15 instructions) and
`[00697860,00697885)` (37 bytes,14 instructions). All75 bytes match the installed
PE image. All29 current instruction starts agree with disk decoding and the
saved native listings. The sole owned CALL invokes the owned core; neither
routine has an indirect call or external tail jump. Hashes are in the report.

| Site | Actual containing function | Consumed contract |
| --- | --- | --- |
| `0069787C` | `00697860` | owned dispatcher, dead destination duplicate/tag, propagated result |
| `006974CC` | `006974B0` | external dispatcher with the same physical core schedule |
| `00697A7A` | `00697990` | growth prefix: current begin to insertion, output=new buffer |
| `00697AA2` | `00697990` | growth suffix: insertion to reloaded current end, output=post-fill cursor |
| `00697B28` | `00697990` | short-tail copy to insertion+count*4 |
| `00697B92` | `00697990` | long-tail copy to old end; returned cursor is published |

```text
0069787C CALL 00696D40
006974CC CALL 00696D40
00697A7A CALL 00696D40
00697AA2 CALL 00696D40
00697B28 CALL 00697860
00697B92 CALL 00697860
```

All four core xrefs and both dispatcher xrefs are accounted for. Each has current
instruction, containing-body, default/effective-target, call-byte and producer
setup proof. The separately read-only producer bodies total587 bytes; they do
not enlarge this packet's Source ownership. Cached graph metadata attributes
some normal sites to the neighboring catch; current complete typed bodies put
the two in-place sites in the second normal insertion range beginning at
`00697B06`. No Ghidra mutation or speculative repair was performed.

## Source domain and validation

The explicit `__fastcall` interfaces preserve the physical input shapes, but
do not promise original code generation, native private-stack aliases, ambient
volatile registers or binary ABI equivalence. Volatile DWORD loads/stores retain
the progressive accesses on MSVC Win32/x86; addresses advance as unsigned32-bit
values. Portable pointer provenance/alignment/fault behavior and original
CRT/SEH identity remain outside that Source claim. No exception catch or cleanup
is introduced.

Earlier checked-word, string and settings storage fragments and their fixtures
remain unchanged. This packet supplies only these two actual leaves. It does
not reconstruct the general insertion, InsertOne, Append, allocator, length-error
or other in-place range entries.

The Win32 build and all three existing suites passed (8.22 seconds total).
Report and Markdown checks each passed six transfer claims; diff checks also
passed. Frozen Source/header hashes are recorded in the report. No new tests
or original-native/gameplay execution are claimed.
