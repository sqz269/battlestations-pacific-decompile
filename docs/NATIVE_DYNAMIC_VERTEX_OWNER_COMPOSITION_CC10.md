# B287C0 dynamic vertex-owner composition evidence

One isolated current-source composition passed with a real D3D9 HAL device,
genuine physical/logical/declaration pool allocations, and genuine retirement.
This packet changes no production source, header, CMake, test or Ghidra state.
The run was completed at baseline `88e1a306269068a5b9982faf5565bb1b08f5bc79`;
closing the interrupted packet only checked and archived existing artifacts.

The tracked report is `reports/native_dynamic_vertex_owner_composition_cc10.json`.
The ignored archive is `local/cc10_dynamic_vertex_owner_composition_evidence.zip`;
its file manifest is `local/cc10_dynamic_vertex_owner_probe/manifest.json`.
The archive includes the exact ten run inputs, all 28 provider/source pins,
compiler object/map/executable, build and run logs, the full process-closure
record, frozen readiness, and the historical hot-fixture manifest/scaffold.
It preserves hashes for all 24 preceding evidence archives rather than nesting
those archives again. Each archive member has a size and SHA-256 manifest row.

## Native boundaries and original interface

Fresh live Ghidra and installed-PE bytes agree for `B287C0[234]`,
`[00B287C0,00B288AA)`. Its 77 instructions end with `RET 0Ch` at `B288A7`;
SHA-256 is `1403a4015028fcf34e9c40de61707a76b03b8fcfc51cd877bcc2413d85725b0b`.
Original ABI: ECX renderer; stacked count, flags, declaration; EAX stream.
The source's context and creator-output arguments are a new C++ interface,
not a promise of binary-call compatibility.

| Site | Target | Role |
| --- | --- | --- |
| `B287E0` | `B4B370` | Allocate logical slot |
| `B28804` | `B4BC00` | Construct stream |
| `B28836` | `B22D10` | Grow primary pointer array |
| `B28854` | `CALL EAX` | Current renderer profile slot `+58` |
| `B2887D` | `B22D10` | Grow optional secondary array |

The four direct call rows were checked against live Ghidra with zero failures
at closure. The indirect row is inventoried from exact bytes; the direct-call
verifier does not prove its target or runtime behavior. All live reads used the
existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` program.

Frozen readiness has 21 live/PE-equal spans: 1,949 code bytes and 158 data bytes.
It includes the complete physical producer fragment `[B2B067,B2B0E7)` and
`B4BC00[758]`, plus physical constructors, attachment and retirement boundaries.
Compiler cleanup `CBD230[8]`, `[CBD230,CBD238)`, loads `[EBP-10h]` into ECX and
jumps to `B49960`. Handler `CBD238[10]`, `[CBD238,CBD242)`, is
`b8c859df00e90199f3ff`: load EAX with `DF59C8`, then jump to `BF6B43` at
`CBD23D`. Its SHA-256 is
`a1f51bd6504c6538f2c8f6a217ab68b3199846db8fd9fd22b30b9c20ff95402b`.
It was missing as a Ghidra function at readiness. Root may define/annotate it
separately; this worker preserved that historical fact and made no mutation.
No native FH3/SEH handler or cleanup transport was executed by this fixture.

## Composition and observed ownership

The fixture uses an isolated aligned `1B90h` renderer field storage with a real
D3D9 device at `+1A10`, initialized `+19F4` lock and zero raw registry arrays.
It is not the renderer constructor, device initializer or full application.
The real HAL was an NVIDIA GeForce RTX 5090. Genuine physical pool acquisition
`B4B360` and `B4BBB0` construct a `2Ch` owner inside a `30h` slot, publish it at
renderer `+1974`, and attach a real 16 MiB DEFAULT vertex buffer with usage
`208h`, FVF 0 and flags `1000h` through `B4C370`. The COM temporary releases to1.

Direct CPU declaration allocation/construction `B488C0/B48AF0` and three
`B48330` appends use `(type3,usage0)`, `(type1,usage5)`, `(type0,usage5)`, all
with automatic offset `FFFFFFFF`. The native size table yields offsets 0/16/24
and stride28. This reaches no name decoder, cache admission or token-global
initialization.

**Declaration atomic lifetime is prepared by the fixture only.** Production
`B48AF0` writes raw count1 at `+4`, whereas `NativeVertexDeclarationReference`
expects a live `std::atomic<int32_t>` there. After that constructor the fixture
captures count1, checks size/alignment4 and the address alignment, placement
constructs the atomic at the SAME actual `+4`, then compares all four count
bytes. This starts one lifetime without adding a reference or separate count.
It is outside native schedule proof. Logical `B61E20` already starts its atomic;
the physical count remains its original raw LONG/DWORD field.

`B287C0` receives count4/flags1000 and the actual declaration. Returned logical
identity equals the creator-output cell; its actual `+58/+68` identities are the
physical owner/declaration, `+5C=FFFFFFFF`, `+60=1000`, `+64=4`, stride28 and
ID `FFFFFFFF -> 0`. One canonical companion per actual declaration/logical
storage uses its actual `+4`; no duplicate credit exists. Observed counts are
logical1/physical2/declaration2. Exactly one borrowed physical `+08` membership
and one renderer `+1AAC` membership exist. Secondary `+19AC=0`, so no `+19B0`
row exists; normal logical destruction does not remove that optional row.

Declaration creator release precedes logical retirement. Logical retirement
removes both reached memberships, reaches genuine declaration zero and returns
both slots. The physical creator remains alive through that retirement, then
its actual raw count is decremented to observed0. The fixture checks the current
`D61E7C`/`BD30E0`/`B4C230` terminal, performs genuine pooled deletion, and clears
renderer `+1974` only after return without rereading retired payload.

All 32 slots per pool are free at physical/logical/declaration slab offsets
`640/F40/1AC0`. The empty primary-array backing is freed; all three genuine
pools, allocator list, support and raw-string pool drain. Device/API COM release
counts are0/0, and the fixture window closes.

## Validation and limits

The preserved strict MSVC Win32 build passed its three existing CTests. The
standalone fixture used `/MD /fp:strict /W4 /WX`, active assertions and
`/MANIFEST:EMBED`; the link map identifies actual `bsp_core` providers and the
exact run inputs include all three project libraries. One controlled child
PID20288 exited0; its parent exited0. All ten input hashes remained unchanged
after the run. The full closure record reports zero matching processes.
No extra build or fixture run was performed to close this packet.

**The production `B317E0 -> NativeVertexDeclarationReference` lifetime path is
not validated or fixed by this fixture.** `B4E470` obtains a declaration through
`B317E0` and admits the companion immediately. The raw `B48AF0` count store does
not establish the C++ atomic lifetime expected by that companion. A separately
reviewed production lifetime change remains necessary before reusing that path.

This is current-source fixture evidence, not copied-original execution, ABI
equivalence, an application run or game validation. It admits no failure path,
nonzero secondary registry, mutable import/profile epoch, private buffer/device
recreation, declaration-name cache, full post-effect/model/camera/source0 path,
renderer/global startup, draw or gameplay claim. Existing valid pool/storage
and provider domains remain assumptions; native private stack aliases are not
a source interface.
