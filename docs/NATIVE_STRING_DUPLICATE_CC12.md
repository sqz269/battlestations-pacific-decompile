# CC12 raw native string duplicate

## Fresh whole-helper qualification

The current standalone qualification is `local/dup57p2`, sealed at130 exact files.
Root independently checked all52 code spans, all retained COFF/map bodies and
relocations, the whole2280-byte/616-instruction Main, raw/ordinary callers,
cookie/chkstk and complete normal/cold helper closure. The only unexpanded
frontier is GS failure, which remains unadmitted. Actual weak aliases are mode1;
mode3 is retained policy.

The sole new process passed5 entries/3 successful results/3 frees,4 actual
Capture72 records,5 full48-byte borrowed input bookends and3 complete8-byte
copies including NUL. Independent recorded decoding, UCRT3/VCRT1 physical
provider and live Native57 post bookends passed;13534 prior pins are unchanged.
The sealed family SHA256 is `63e41c374d57d2c333f235fab6a403e74377b0a6ed221bf1b5ec7eb4c473e206`.

Captured pre-CALL ESP is S; target entry S-4 and copy ADD input S-32 are derived.
Nonnull volatile ECX/EDX remain observed and unasserted; no ES is serialized.
This admits the bounded current DF0 raw success/null entry and ordinary Source
nonnull lane. Private CRT, allocation failure/EH/unwind, original callers, class
behavior, whole-game ABI, startup and gameplay remain unvalidated.

The earlier fixture and its coverage hold below are historical evidence. Their
bytes, observations and separate qualified consumer families are preserved.

`bsp::duplicate_native_string_00438e40(const char*)` reconstructs the complete native `[00438E40,00438E79)` entry: **57 bytes / 30 instructions**, actual text in ECX, full EAX result, no stack arguments and `RET 0`. The older plain-CDECL `bsp::duplicate_00438e40` API is unchanged. This implementation is qualified to successful/null execution using current canonical allocation/free and standard copy providers.

The original body SHA-256 is `28510a4c8a423997dfca7d8e257c1b250a7bb9e79cce177fc9c501e7b64735c4`. Source preserves **49 literal bytes** and exactly two real CALL operand relocations: `[34,38)` for allocation and `[44,48)` for copy. The inline NUL scan, byte/word widths, retained allocation argument, copy argument order and final `EAX=EDI` remain literal. Live Ghidra, installed PE and saved instruction starts agree with zero gaps; the saved prototype remains incomplete and was not edited by this worker.

| Physical contract | Behavior/evidence |
| --- | --- |
| Null ECX | Returns zero without target allocation/copy. ECX stays zero and incoming EDX survives. XOR defines CF/OF/SF=0 and ZF/PF=1; AF remains unasserted. |
| Nonnull ECX | Requires a live readable NUL-terminated byte object, representable `n+1` and no address wrap. Allocates `n+1`, copies that many bytes including NUL, returns the actual owned result. |
| Nonvolatiles | EBX is saved on both paths; ESI/EDI on the nonnull path. EBP is locally untouched. All four matched in the raw fixture. |
| Volatiles | Nonnull ECX/EDX may change across the real providers. Observed values are recorded without guessed equality requirements. |
| Stack/flags | With raw caller ESP=T immediately before CALL, the target returns to T. Nonnull memcpy returns at T-32; final ADD16 produces T-16 before pops/RET. Defined arithmetic flags derive from that actual ADD. |
| Ownership | Results belong to the current canonical malloc/free domain and are freed once through unchanged `singleton_lifetime_free`. Source remains borrowed and unchanged. |

The new private noinline CDECL adapter passes `{SingletonAllocationKind::object, actual_bytes, actual_bytes}` to unchanged `singleton_lifetime_allocate`. It uses real allocation/new-handler behavior and earns **zero Native credit**. No callback, fabricated provider/result, alternate heap or synthetic owner is introduced. The public raw entry has no `noexcept` promise; failure unwind through naked frames is unadmitted.

The literal copy CALL resolves to the real `_memcpy` import thunk, a six-byte terminal JMP through IAT `26004078` imported from **`VCRUNTIME140.dll`**. Its three CDECL arguments occupy the original stack positions. The genuine ordinary compiler caller is nine bytes/two instructions: load ECX from its CDECL stack argument, then tail-JMP to the raw entry. Native RET0 returns directly through that caller's existing return address.

The fresh three-TU fixture compiled this Source/adapter, unchanged `singleton_lifetime.cpp` and a new probe under MSVC 14.51 Win32 `/MD /O2 /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`; the raw TU additionally disables intrinsics. The link uses `/OPT:NOICF`, fixed base `26000000`, and an embedded `asInvoker`, `uiAccess=false` manifest. It consumed 184 pinned headers and seven resolved libraries, with no BSP archive or old object. No new tracked tests were added.

All complete local spans were derived from this fresh build and bookended in the sole process:

| Span | Bytes / instructions |
| --- | ---: |
| Raw duplicate | 57 / 30 |
| Genuine allocation adapter | 61 / 17 |
| Canonical allocate / free | 90 / 34; 6 / 1 |
| Ordinary caller / raw capture caller | 9 / 2; 128 / 48 |
| `bad_alloc` constructor / cookie helper | 24 / 6; 14 / 4 |
| memcpy import thunk / cold throw import thunk | 6 / 1; 6 / 1 |

The complete canonical allocator retains its current retry/throw branch. Cold constructor, cookie and throw-thunk bytes are covered, but failure/reentry/exception paths were not forced or dynamically admitted. Cookie failure targets and runtime exception implementation remain outside the normal-domain claim.

Before **zero target entries**, all actual malloc/free/new-handler IAT pointers matched GetProcAddress and physical export RVAs in the same mapped I386 `ucrtbase.dll`. Copy matched the distinct mapped I386 `vcruntime140.dll`. Each mapping passed MEM_IMAGE, loader-path versus mapped/opened NT-path, volume/file ID, full file hash, PE identity and ASLR-normalized entry-byte checks while read handles denied write/delete sharing. Both physical modules and IATs passed post-execution checks. UCRT's identical memcpy prefix supplied no identity evidence.

- Heap provider: physical `C:/Windows/SysWOW64/ucrtbase.dll`, SHA-256 `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.
- Copy provider: physical `C:/Windows/SysWOW64/vcruntime140.dll`, SHA-256 `2fa6efc053203460a23d3a25158f227d895d2dadc63acc1a372da97c3a4281c3`, export RVA `161A0`. Loader DOS `System32` paths are WOW64 aliases; mapped and opened NT paths identify the actual SysWOW64 files.

The sole run passed **five entries, three real returned allocations and three canonical frees**: raw current-domain-bound Original and Source each took null and the same guarded nonempty byte array, followed by one ordinary Source nonnull call. The Original RX copy changed only those same two CALL operands to the same genuine current providers. The copied bytes were `51 82 F1 6B 23 00`; the borrowed source and all guards remained unchanged. Each result was observed while live and freed once. All three allocations naturally reused address `6978872`; numerical address equality was never required or forced.

The raw caller used T=`1726248`. Null flags were `246h` (defined mask `8C5h` gives `44h`). Nonnull memcpy-return ESP=`1726216`, final ADD result=`1726232`, post-return ESP=T, flags=`206h` (mask `8D5h` gives `4`). All nonvolatiles matched. Nonnull ECX=`0`, EDX=`1810989649` were recorded only. This establishes the qualified physical entry and normal current-provider behavior, not an original-private-CRT binary replacement.

Source SHA-256: `48c86a1ad7763861d099d909a7a8b314b3258ee9b5508f702f093a64188946e0`; header: `b101c630eba5844d1f5d21530aef1200c0969b0139fb72de1d3870e75814f477`. The unchanged canonical Source remains `97733266a44569114fca8142035375b728144e20658a6d914156b2a78655ec62`.

Evidence is in `local/cc12_raw_native_string_duplicate_worker20261008a` and [the tracked report](../reports/native_string_duplicate_cc12.json). The core seal is `82e88f05c8093e8b3701f4e9c2cc2c310716fd3e24657947d06d35dc999bc1fc`; the final manifest additionally includes the commit handoff. All 37 explicit inputs, consumed headers/libraries, executable/objects, provider files, original native body and 670 older artifacts passed bookends. Historical metadata, a pre-prepare syntax draft failure and a harmless read-command quoting failure are preserved. All accepted build/static/process/post stages passed once; no accepted native run was replayed. Post/seal terminal output stayed outside the family.

Original private allocator `00BF681B`, private memcpy `00BF7680`, historical release `00BF9DC8`, original EH, malformed input/overflow/overlap, failure/reentry, property class/owner/destructor, world integration and gameplay remain unadmitted. Root owns shared CMake/ledger registration, independent validation, locked Ghidra annotation/export/save, full Win32 build/tests and main integration. Worker verification is the fresh standalone component fixture only.

## Primary integration

Whole physical string duplicate[00438E40,00438E79),57B30. ECX borrowed readable nullable NUL-terminated nonwrapping text; no stack arguments, plain RET0, EAX actual owned copy or0. Exactly CALL operands[34,38)/[44,48) bind genuine size adapter/current canonical allocation and actual VCRUNTIME140 memcpy import; all49 other bytes literal. Genuine private adapter61B17 has zero Native credit. Null XOR flags mask8C5=44 excludes undefined AF and preserves EDX; nonnull final ADD16 flags mask8D5 independently derived from copy-returnESP=T-32 and resultT-16; final RET returnsESP=T. Nonnull ECX/EDX volatile/unasserted. Current CRT DF0 only; no blanket FP/ES preservation. Independent fresh3TU strict Win32 process uses new guarded8-byte93fe716da4c28000 input/register sentinels,5 entries/3 real copies/3 current frees,10 complete code spans including full14-byte cookie and actual6-byte imports; complete physical UCRT/VCRUNTIME identity and code/input/Native bookends. Exact95 sealed artifacts plus seal,12740 original prior pins plus373 supplemental immutable pins=13113. Historical prelaunch basename failure stopped before target, separately recorded recovery, no successful phase or old target replay. Supplemental current merged-main Win32 build/all3 checks passed. Legacy CDECL duplicate_00438e40 remains unchanged; it is not the admitted physical interface. Normal current-provider domain only; original private CRT/EH, owning class/destructor/vtable and game ABI/gameplay remain unadmitted.

Independent seal `7a759f86390ca916952a15c7c3bb2508204472e6c2bfb346c68f453f4d85c852`; supplemental current build revision `b1c4e140710d84681710e0865e208ed45e0481ea`.

## Standalone fixture helper coverage reopened

Root reviewed the independent published dup57p1 audit: the10-span gate omits15 retained probe bodies (14 normal including full Main3679B1004 and one error reporter), actual43-byte stack helper and nine6-byte normal import thunks. Six cold standard TU bodies and two cold exception thunks plus separate GS/sized-delete frontiers are also unadmitted. Standalone complete-helper fixture Source admission is reopened to0 pending a fresh complementary whole-helper gate. Whole Native57B30/physicalECX/plainRET0 reconstruction and recorded successful guarded-copy observations remain historical evidence; this hold does not invalidate separately compiled/gated Type2 t2p5 or reference ref48p2 raw consumer evidence. No Source edits, old helper/process replay or new Native/provider queries.
