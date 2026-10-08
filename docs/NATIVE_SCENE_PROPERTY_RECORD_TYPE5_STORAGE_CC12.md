# Native scene property record type-5 storage (CC12)

The constructor at `008EF2B0` now has a physical MSVC Win32 Source entry that
accepts a fresh 56-byte root, a borrowed text pointer and two opaque DWORDs.
It uses the independently admitted string duplicate provider to create an owned
child. The worker's strict four-TU build and one native process passed.
Source admission, the full repository build and gameplay remain Root work.

## Native body and API

`[008EF2B0,008EF2EF)` contains 63 bytes and 19 instructions. Fresh installed-PE
and read-only Ghidra bookends match SHA256
`a3f703ea58a3c6f7fab8ce6d36764b8b1445cd101e5a0236a493d45718138e15`.
The sole `CALL` at `008EF2D2` targets `00438E40`; its operand occupies `[35,39)`.
The Source body preserves all other 59 bytes. Names describe the observed
operation and remain hypotheses about the original class.

```cpp
void* __fastcall construct_native_scene_property_record_type5_storage_008ef2b0(
    void* actual_record_ecx, void* unused_edx, const char* actual_text,
    std::uint32_t word_18_bits, std::uint32_t word_08_bits);
```

ECX carries the real root, incoming EDX is unused, and entry stack offsets
`+4/+8/+C` contain text/word18/word08. The placeholder EDX parameter preserves
these three physical stack slots. The constructor returns root in EAX and
executes `RET 0C`. It saves ESI; EBX/EDI/EBP follow the genuine provider's normal
ABI. Nonnull ECX/EDX residuals are recorded without guessed assertions.
Final `XOR EAX,EAX`, after the provider, establishes `EFLAGS & 8C5 == 44`;
AF is undefined. The declaration has no `noexcept` promise.

| Root bytes | Result |
|---|---|
| `00..03` | Opaque literal phase identity `00CE89D4` |
| `04..07` | Literal tag 5 |
| `08..0B` | Third stack DWORD, word08 |
| `18..1B` | Second stack DWORD, word18 |
| `1C..1F` | Full actual duplicate return, owned child or null |
| `20..23`, `24..27`, `34..37` | Zero |
| `2C` | One |

This writes 33 bytes and preserves 23: `[0C,18)`, `[28,2C)` and `[2D,34)`.
The DWORD at `+30` is preserved. Phase/tag/raw words are stored before the
provider call; the child/zero/byte stores occur afterward. A failed provider
may leave partial writes. Failure, reentry and naked-frame unwind are excluded.

## Ownership and dependency

The caller supplies fresh unowned writable root storage and either null or
stable readable NUL-terminated text. Root, text through the first NUL and the
active frame must be disjoint, with nonwrapping ranges and length-plus-one.
Require DF clear for the actual current CRT providers. No whole-call DF, ES,
FPU or MXCSR preservation is claimed. No numeric conversion occurs locally.

Input remains borrowed. A nonnull `+1C` result is a real owned copy in the
canonical allocation domain: observe it while live and free it exactly once
with `singleton_lifetime_free` before disposing the root. The opaque phase
must not be called as a Source vtable. No refcount, parser, class, World,
owner or destructor contract is established here.

Root admitted physical `duplicate_native_string_00438e40` in `bb285149e`.
The fresh fixture compiles that unchanged production TU and the unchanged
canonical allocator TU. It also gates the actual allocation adapter, cookie
helper and memcpy import; it does not substitute callbacks or forced returns.
The private baseline `7c79302e7` freezes the admitted provider association.
The earlier reference-readiness audit `604b1f321e` supplies bounded clone-caller
context; no broad class or clone body was queried for this implementation.

## Fresh evidence

`local/t5a` contains four new TUs: constructor, actual duplicate, canonical
allocator and ignored probe. MSVC 14.51.36231 x86 with SDK 10.0.26100.0 compiled
with `/O2 /MD /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`; the duplicate
uses `/Oi-`. The executable has an embedded `asInvoker` manifest.
There are no BSP archives, old objects or new tracked tests.

The full COFF/map/linked checks cover 14 spans: constructor63, duplicate57,
allocation adapter61, canonical allocation90/free6, ordinary Source caller26,
ordinary Original caller25, raw caller181, cold bad_alloc24, main4203,
complete cookie14, memcpy thunk6, cold throw thunk6 and map-owned chkstk48.
The cookie extent includes its external failure-tail JMP; no failure body is
admitted. `chkstk` includes the complete stack-page loop and five alignment
bytes through the next mapped function. Main's 166 relocations were normalized.
Both ordinary compiler callers were fully gated statically and not executed.

The one target process made exactly one Source call and one qualified
current-domain-bound Original call. Original63 was copied into fresh RX
storage with only `[35,39)` rebound to the same actual duplicate provider;
all other 59 bytes remained native. This is not private-original-CRT proof.

| Observation | Source | Bound Original |
|---|---|---|
| Actual root | `007BEF98` | `007BF058` |
| Actual owned child | `007B72A8` | `007B1998` |
| Borrowed input | `001A9B40` | `001A9B68` |
| Word18 / word08 | `D19A42C7 / 80000000` | `2EB56C38 / 7FFFFFFF` |
| Copied bytes including NUL | `67 C3 A1 2D 6F 00` | `52 E8 B7 73 34 00` |
| EAX / post ESP | `007BEF98 / 0018964C` | `007BF058 / 0018964C` |
| Post EFLAGS | `00000246` | `00000246` |
| Observed ECX / EDX | `00000000 / 2DA1C367` | `00000000 / 73B7E852` |

The complete 56-byte roots, six-byte children, 40-byte guarded inputs and
132-byte captures were observed while live. All three dead argument words
were captured before PUSHFD could overwrite a slot. Nonvolatile registers,
stack/capture guards, RET12, final defined flags and preserved storage passed.
The other root/capture and both input objects stayed unchanged after each call.
Two real root allocations plus two real child allocations were freed once each,
children first. Null is supported by the static/provider contract but was not
invoked in this minimal two-entry fixture.

Actual IAT targets for malloc/free/_callnewh mapped to SysWOW64 `ucrtbase.dll`,
SHA256 `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.
The memcpy IAT mapped to the distinct SysWOW64 `vcruntime140.dll`, SHA256
`2fa6efc053203460a23d3a25158f227d895d2dadc63acc1a372da97c3a4281c3`.
Physical NT paths, file IDs, full hashes, I386 images, MEM_IMAGE ownership,
export RVAs and normalized live prefixes were checked before zero entries and
after all frees. All 14 code spans also passed both runtime bookends.

## Sealed handoff

The immutable family has 362 artifacts plus its seal, SHA256
`51002603d10a2ac881fc3c17bdbbd9052facf66c8977bc58e8a33957fc525d10`.
The exact recursive inventory, sizes and hashes passed an independent reread.
All 4,277 old artifact pins, 48 inputs, 185 consumed headers and seven consumed
libraries remained unchanged. Candidate catalogs included 7,114 headers and
540 libraries. Six old metadata associations are copied and frozen; no old
fixture was replayed or relabeled. Post/seal stdout stayed outside the family.

Each prepare/build/static/launch/post stage ran once and passed. A shell-only
post-seal inventory check initially excluded all files named `seal.json`,
including a frozen historical copy; excluding only the root seal corrected
that check. No sealed file, process or stage changed or was rerun.

See `reports/native_scene_property_record_type5_storage_cc12.json` for exact
source/header/object/executable hashes, capture bytes, provider receipts,
whole-span ownership and remaining Root integration work. Worker Source credit
is zero pending independent Root review. Native private CRT/EH, forced failures,
invalid inputs, full startup and gameplay remain outside this evidence.
