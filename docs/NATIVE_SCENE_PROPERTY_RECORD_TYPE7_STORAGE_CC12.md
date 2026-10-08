# Native property-record type-7 raw storage, CC12

The whole leaf at `008EF270` partially initializes supplied 56-byte storage
from a borrowed, disjoint 12-byte span. The new Win32 Source body exactly
matches Original: 63 bytes, 19 instructions, no CALLs and no relocations.
The descriptive name is a hypothesis; the operation copies raw words.

The worker implementation, strict three-TU build and bounded fixture passed.
Source admission remains pending Root's independent complementary fixture,
full main Win32 build/checks and publication. Parsing, native vector classes,
phase dispatch, clone, native allocator/EH and owning lifetimes remain outside
this leaf's contract.

Whole range: `[008EF270,008EF2AF)`. Original, fresh Source COFF and unique
linked body share SHA-256
`9010f6007c93fa311d1e2d7f6c89a06aa802af3575be8e5c22edd26aa36d6872`.

```cpp
void* __fastcall construct_native_scene_property_record_type7_storage_008ef270(
    void* actual_record_ecx, void* unused_edx,
    const void* actual_payload_words) noexcept;
```

ECX supplies actual fresh/unowned writable 56-byte destination storage. The
DWORD at entry `ESP+4` is the actual pointer to a stable readable 12-byte input
span. Input and destination must be disjoint from each other and the active
target frame. The caller retains both lifetimes. The input pointer is neither
retained nor freed; do not overwrite a live owning native property object.

Incoming EDX is unused. EAX returns the destination, ECX becomes zero and EDX
retains the **second** input DWORD, at `input+4`. `RET4` consumes the pointer.
EBX, ESI, EDI, EBP and DF are preserved. Final defined XOR flags are
CF=OF=SF=0 and ZF=PF=1; AF is undefined and excluded from checks.

| Destination | Exact effect |
| --- | --- |
| `+00,+04` | DWORD opaque phase identity `00CE89D4`, then DWORD tag 7 |
| `+0C,+10,+14` | Raw input DWORDs at `+0,+4,+8`, respectively |
| `+18,+1C,+20,+24,+30,+34` | Zero DWORDs |
| `+2C` | Byte 1 |
| `[08,0C),[28,2C),[2D,30)` | Preserve all 11 bytes |

Exactly 45 bytes are written. Phase/tag writes precede the first input read,
and the three loads and stores are interleaved. The Source retains this order.
There is no overlapping-memory/snapshot-copy or invalid-input fault contract.
No target SSE, x87, MXCSR or numeric conversion occurs. The opaque phase word
must never be dispatched as a Source vtable.

The separately sealed readiness audit established the genuine clone caller:
tag-7 cell `008F52D4` selects `[008F5134,008F5168)`. That arm allocates `38h`,
forms the actual pointer `source+0C`, supplies the returned destination in ECX
and calls this leaf at `008F514C`. It then copies the ordinal at `+34`.
No caller/provider boundary was expanded during this Source packet.

The ignored family `local/t7a/` compiled exactly the new constructor, unchanged
current `singleton_lifetime.cpp`, and a new probe with MSVC 14.51.36231 / SDK
10.0.26100.0, Win32 `/O2 /MD /W4 /WX /fp:strict /EHsc /Gy /GL-`. The executable
has an embedded `asInvoker` manifest. No BSP archive, old object or tracked
test was added. Complete code comparisons normalize all actual COFF
relocations and gate nine linked spans before allocation and after freeing:
constructor 63/19, allocator 90/34, free 6/1, direct ordinary caller 18/5,
indirect ordinary caller 66/26, raw caller 156/59, bad_alloc helper 24/6,
main 3359/877, and the full map-owned cookie helper 14/4 (bytes/instructions).
The cookie's named failure tail JMP is included; its distinct failure body
is not admitted. Ordinary callers and allocator exhaustion are static-only.

The ordinary callers transport the full pointer DWORD. The raw caller saves
the actual dead pointer slot at `[ESP-4]` before PUSHFD can overwrite it,
captures return registers and flags, and checks `RET4`. Two stack guards and
16-byte pre/post guards surround each 116-byte capture. Actual input arrays
have three DWORDs with 16-byte guards on both sides, 44 bytes each.

Actual malloc/free/_callnewh IAT addresses resolve to mapped I386
`ucrtbase.dll`. Held physical files match the mapped NT path, file identity,
full SHA-256, PE/export metadata and normalized live export prefixes before
roots and after frees. The physical DLL hash is
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.

One process executed one Source and one unchanged RX Original call:

| Case | Three raw input words | DF | Destination poison |
| --- | --- | --- | --- |
| Source | `D19A42C7 80000000 10203040` | 0 | `A6` |
| Original | `2EB56C38 7FFFFFFF A5C37E91` | 1 | `59` |

Both calls passed whole-56-byte destination checks, whole-input/guard checks,
other-live-root and capture checks, actual pointer-slot checks, register,
nonvolatile, DF, ES and defined-flag checks. Both observed EDX values equal
the respective second input word. Exactly two actual canonical allocations
and two matching frees occurred; the Original RX allocation was released.
All nine linked code spans, Original bytes, CRT providers and native PE
bookends remained unchanged. There was no floating-point test suite.

Preparation pinned 43 inputs, 7,110 candidate headers and 540 candidate
libraries; the fresh build consumed 184 headers and 7 libraries. Extensionless
headers are included, and short frozen names are checked for collisions before
compilation. All 3,216 earlier artifact pins remained unchanged, including
the 225-file readiness audit, Root's accepted 342-file `t4p2` family and the
316-file stopped, unexecuted `t4p1` family. Private worktree metadata was used.

A receipt-only review script initially indexed PUSH ESI instead of the
preceding malloc-IAT MOV. The stopped script and correction history are
preserved; a separately named corrected review passed before execution.
Compilation, linking, static gating and the successful target process were
each performed once. No historical successful phase was replayed.

The terminal seal inventories 343 artifacts; the complete directory contains
344 files including the seal. Every artifact hash and the exact recursive
file set were independently checked. Seal SHA-256:
`0bb39bc58ae2e3591c1be0d15054d75a1477f6627b3f6f6e20efcdac773ac4dd`.
Post stdout was kept outside the sealed family.

The [machine-readable report](../reports/native_scene_property_record_type7_storage_cc12.json)
contains physical contract details, actual captures, whole-span hashes and
receipt paths. This is bounded raw-leaf fixture evidence; no whole-class ABI,
private CRT/EH, native ownership, startup or gameplay validation is claimed.
