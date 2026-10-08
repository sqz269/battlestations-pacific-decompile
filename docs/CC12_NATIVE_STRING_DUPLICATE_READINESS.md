# CC12 native string duplicate readiness

`00438E40` supports a qualified proposal for a future raw implementation using the current canonical allocator/free pair and the actual standard copy provider. This audit implements and admits **zero Source functions**. Root must review and register the proposed packet, then validate a fresh implementation before the type-2 constructor can use it.

The complete native range `[00438E40,00438E79)` is **57 bytes / 30 instructions**, SHA-256 `28510a4c8a423997dfca7d8e257c1b250a7bb9e79cce177fc9c501e7b64735c4`. Live bytes, installed PE and complete saved instruction starts agree. The installed PE remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Contract | Bounded evidence |
| --- | --- |
| Entry/return | Actual text pointer in ECX, no stack arguments, full pointer/null in EAX, `RET` with cleanup zero. |
| Null | No read, allocation or copy; EAX becomes zero, ECX remains zero and incoming EDX survives. XOR defines CF/OF/SF=0 and ZF/PF=1; AF is undefined. |
| Nonnull | Inline byte scan finds NUL, computes `n+1`, allocates that many bytes and copies the terminator. Requires a live readable contiguous source, representable length and no address wrap. |
| Registers | EBX is saved on both paths; ESI/EDI on the nonnull path. EBP is locally untouched. Genuine CDECL providers must preserve nonvolatiles. Nonnull ECX/EDX remain volatile. |
| Stack/flags | For native entry ESP=S, copy returns at S-28; final `ADD ESP,10h` produces S-12. Pops restore S and RET produces S+4. Arithmetic flags derive from that actual ADD. |
| Ownership | Successful current allocation is disjoint from the live source and must be released once through unchanged `singleton_lifetime_free`; no reads after free. |

Only two actual CALL operands may change: allocation at `00438E61`, offsets **34..37**, and copy at `00438E6B`, offsets **44..47**. All other **49 bytes** must remain literal. Allocation currently calls `00BF55BE`, whose complete five-byte JMP targets `00BF681B`. Copy currently calls `00BF7680`; only its first 32 bytes and saved CDECL three-argument prototype were inspected. Neither private CRT implementation is admitted.

The installed VC 14.51 x86 `vcruntime.lib` contains a 45-byte import member at archive offset 394526 mapping `_memcpy` to **`VCRUNTIME140.dll!memcpy`**. Its `IMPORT_CODE` / `IMPORT_NAME_NOPREFIX` record supplies the concrete link candidate. The pinned I386 file is `C:/Windows/SysWOW64/vcruntime140.dll`, SHA-256 `2fa6efc053203460a23d3a25158f227d895d2dadc63acc1a372da97c3a4281c3`; `memcpy` is a direct export at RVA `000161A0`.

UCRT also exports `memcpy`, and both installed DLLs currently have identical first 32 copy-entry bytes. A matching prefix therefore cannot identify the provider. The future fixture must bind the actual import thunk and IAT to the mapped I386 module and opened physical file, including full hash, NT path, volume/file identity, export RVA and relocation-normalized bytes. No duplicate process or mapped copy provider was executed or dynamically attested here.

The pinned local `vcruntime_string.h` declares three CDECL arguments and the destination-return/copy postconditions. Microsoft documents the same [copy/return/nonoverlap contract](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/memcpy-wmemcpy?view=msvc-170) and [caller stack cleanup](https://learn.microsoft.com/en-us/cpp/cpp/cdecl?view=msvc-170). The raw body pushes destination, source and size in the correct physical positions, ignores memcpy's temporary EAX and returns saved EDI.

The unchanged canonical Source provides real current `malloc` / `_callnewh` retry semantics and matching `free`. Prior accepted complete bodies are allocator 90 bytes/34 instructions, free 6/1 and the analogous genuine size adapter 61/17; these counts are historical context and must be derived again after a fresh build. The older plain-CDECL `bsp::duplicate_00438e40` remains unchanged and receives no physical ECX/RET0 claim.

The sole caller witness is the whole 60-byte/20-instruction type-2 initializer `008EF1B0`: it passes its real stacked text through ECX at call `008EF1D6`, then stores returned EAX at destination+0x0C and sets ownership flag byte+0x2C. This proves pointer storage flow only. No clone arm or other property leaf was queried. The typed graph contains 13 native/current/proposed nodes and 10 edges, **23/24** total, including incomplete private provider and historical release boundaries.

The proposed future packet owns exactly:

- `include/bsp/native_string_duplicate.hpp`
- `src/native_string_duplicate.cpp`
- `docs/NATIVE_STRING_DUPLICATE_CC12.md`
- `reports/native_string_duplicate_cc12.json`

Its API is `char* __fastcall bsp::duplicate_native_string_00438e40(const char* actual_text_ecx);`, restricted to MSVC Win32. It has no `noexcept` promise. A new private noinline CDECL size adapter must call the unchanged canonical allocator with `{object, n, n}`; it earns zero Native credit. The literal copy CALL must reach the actual standard import thunk, with no substitute copy wrapper or callback.

A fresh three-TU family must compile the new raw body/adapter, unchanged canonical allocator/free Source and a new probe. Full COFF and linked spans, both relocations, actual CRT libraries and provider IATs must pass before entry. The bound Original changes the same two operands to those same genuine providers and keeps all other bytes literal. It establishes a current-domain comparison only.

The minimal future fixture uses two inputs: null and one guarded nonempty NUL-terminated byte array. Raw Original and Source each take both inputs; one ordinary compiler Source call also takes the nonnull input. Five entries yield three real allocations to observe and free once. Record full EAX, nonvolatiles, actual stack and defined flags; retain volatile outputs without guessed equality assertions. Gate actual malloc/free/new-handler and memcpy modules before any target entry, then bookend complete bodies, IATs, physical files, inputs and older artifacts. Provider callbacks, forced returns/OOM, reentry, overlap, freed reads and invented owners are excluded.

Original private CRT/EH, failure unwind through a naked entry, class/phase/destructor ownership, world integration and gameplay remain unadmitted. The tag-2 constructor remains a separate packet until this duplicate is actually implemented and accepted. Root owns registration, shared metadata/build integration and later Ghidra mutations.

Evidence is in `local/cc12_native_string_duplicate_readiness20261008a/{proposal_final.json,readiness_receipt_final.json,artifact_manifest.json}` and [the tracked report](../reports/cc12_native_string_duplicate_readiness.json). Four native spans, 26 initial inputs plus three supplemental helper/tool pins, both physical provider files and 618 older artifacts passed bookends. The scalar family's exact 44-file inventory and embedded historical associations remain unchanged. An initial count-output assertion failed before any address query; its original helper/attempt/count and the corrected new helper are retained. No compilation, native execution or Ghidra mutation occurred.
