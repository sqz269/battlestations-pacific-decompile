# cc15-rA1: live Dyn/Mit bodies of segment 0

Packet cc15-rA1 reconstructs the segment-0 Dyn/Mit header bodies that are still referenced. The range is
00401010-00410240, the first linked object (see `docs/CC13_DYN_LIBRARY.md`). The code is in
`src/cc15_dyn_live.cpp` and `include/bsp/cc15_dyn_live.hpp`. Every body is reconstructed and
build-tested only. Nothing is wired, and none is ABI-compatible: the register-ABI bodies take C++
arguments, and the global pools are passed as instances. Names are descriptive hypotheses, not
recovered symbols.

## Method

- Callers come from a byte census: `tools/image_ref_scan.py`, which finds E8/E9 rel32 references and
  absolute dwords in every section. Ghidra xrefs were cross-checked against the census. The two agree
  for every row except 0040EE00: the index attributes its call sites to the wrong function, but the
  census shows three CALLs.
- Every body was read from the assembly. Where the pseudocode hides a register ABI, the ABI was
  taken from the listing.
- The CRT calls go through `DynLiveCrt`, so the host chooses the heap: 00BF681B and 00BF55BE
  (`operator new`), 00BF6989 (`_free`) and 00BF9F1A (`_malloc`). The critical sections, `Sleep`,
  `RDTSC`, `__allmul`, `_memset` and `_vsprintf_s` are bound to the host's equivalents.
- Both global pools use the `cc13_dyn2::MitPoolAllocator36x128` layout. The <20,128> offsets are proven
  by 0040D860's absolute operands: base 0109ED28, lock +0Ch, depth +24h, pages +28h, count +2Ch,
  capacity +30h, hint +34h.
- No x87 arithmetic except 00407000's FILD/FSTP. That conversion is an exact load followed by one
  round-to-nearest store, so the precision control word does not affect it.

## Rows

| Address | Name | Status | Original ABI | Evidence | Uncertainty |
|---|---|---|---|---|---|
| 004077F0 | Mit_PoolAllocator_36_128_Allocate_004077F0 | reconstructed | no inputs; EAX = slot; RET | Lock 0109EA18. If the hint is -1: hint = count, `new(1504h)`, 004084D0, grow the page vector to (cap+1)*2, append. Then pop the slot from the stack; on depth 0, rescan from hint+1. 7 CALLs (00C51480, 00C5CAE0). JMP thunk 00C369A0 is unreferenced. | A failed `new` faults on the pop, as in the image. |
| 0040D860 | Mit_PoolAllocator_20_128_Allocate_0040D860 | reconstructed | as 004077F0 | Same instructions; pool 0109ED28, page D04h, 0040DA40, stride 18h. CALL 00407B71. JMP thunk 0040CD90 is unreferenced. | as above |
| 004084D0 | Mit_PoolAllocator_36_128_PageConstruct_004084D0 | reconstructed | EAX = page, EDI = index; returns EAX; RET | Depth 80h, stack 7Fh..0, slot+24h = index. Only caller 00407832. | none |
| 0040DA40 | Mit_PoolAllocator_20_128_PageConstruct_0040DA40 | reconstructed | as 004084D0 | +D00h/+C00h, slot+14h, stride 18h. Only caller 0040D8A2. | none |
| 0040D9D0 | Mit_PoolAllocator_20_128_Free_0040D9D0 (was DYN_header_0040d9d0) | reconstructed | EAX = slot; RET | Locked push of (slot-page)/18h; the unsigned compare lowers the hint. | Out of line it is reached only by JMP thunk 0040CDA0, which nothing references. The body runs inlined in 00408340. |
| 00407A50 | DYN_EdgeHashMap_Find_00407A50 | reconstructed | ECX = key, EAX = out; stack map; RET 4 | k = (a<<16)+b; h = (k+E092C689h)^4E67C6A7h; 4 rounds of h ^= (h<<5)+(h>>2)+k; bucket h&7Fh; walk the chain at +10h. CALL 00C38D3E. | none |
| 00407AF0 | DYN_EdgeHashMap_FindOrInsert_00407AF0 | reconstructed | EBX = entry, EDI = map; EAX = node; RET | Hit: JMP 004080D0. Miss: ++[+200h], 0040D860, 00408470, head insert with prev at +0Ch. CALL 00C38D72. | A null node faults, as in the image. |
| 004080D0 | DYN_EdgeHashMap_CopyKeyValue_004080D0 | reconstructed | EAX = dst, ECX = src; EAX = dst; RET | Two key words, then count words (count at +8, signed compare), then the count. Reached by tail JMP 00407B87. | The value array holds two uint16 entries; the copy loop is unbounded, as in the image. |
| 00408470 | DYN_EdgeHashMap_CopyValue_00408470 | reconstructed | as 004080D0 | The key is copied as one dword. CALL 00407B7C. | as above |
| 00408340 | DYN_EdgeHashMap_Clear_00408340 | reconstructed | __stdcall(map); RET 4 | Frees every node of the 128 buckets through the inlined 0040D9D0, then `_memset(map,0,200h)` and count = 0. Callers 00407A31, 00C38D72, 00C38F2F. | none |
| 0040AF00 | DYN_Pool68_Construct_0040AF00 | reconstructed | __stdcall(pool); EAX = pool; RET 4; SEH | Zeroes fields, sets +90h = +10h and +50h = +54h, `_malloc(109A0h)` (1000 blocks of 44h, next pointer at +40h, last = 0), head +0Ch, push_back with growth cap*2+2. | The roles of +10h..+98h are not established. The SEH unwind is not modelled. Its only callers (00C37086, 00C37097) are in the unreachable SAPBroadPhaseManager constructor 00C37030. |
| 0040D7B0 | DYN_Vector4_CopyConstruct_0040D7B0 | reconstructed | ESI = dst, EBX = src; EAX = dst; RET | Empty source gives {0,0,0}; otherwise `new(size*4)` and an unsigned copy loop. 5 CALLs. | none |
| 0040EE00 | DYN_Vector4_SetCapacity_0040EE00 | reconstructed | ESI = vector, EAX = cap; RET | cap = EAX; `new(cap*4)`, or FFFFFFFFh on overflow; signed copy; `_free(old)`. CALLs 0040611D (AppendTriangle 004060E0, 7 callers), 0040C566, 0040C5C8. The cc13 "Dead" note is corrected in the names ledger. | none |
| 0040EEF0 | DYN_Vector4_SetCapacity_0040EEF0 | reconstructed | as 0040EE00 | Byte-identical to 0040EE00. CALLs 00C5CB67, 00C5CB96, 0040C713, 0040C768. | none |
| 00407000 | DYN_ProfileTimer_CalibrateFrequency_00407000 | reconstructed | __thiscall(ECX = 0109EA44 unused); EAX = 0109EA44; RET | Read RDTSC, Sleep(100), read RDTSC again; the delta times 10 (`__allmul`) is stored by FILD/FSTP into the double [0109E9E8]. Static initializer 00CC89E0 JMPs here. | Returns the instance that was passed; the double is a reference. |
| 0040CFC0 | Mit_FormatString1024_0040CFC0 | reconstructed | __cdecl(buffer, format, ...); EAX = result | `_vsprintf_s(buffer, 400h, format, va)`. CALLs 00C32134, 00923B2B. | Uses the host's CRT. |

## Dropped rows

None. Three rows are referenced only through dead code; they were reconstructed anyway, with the
status recorded:

- 0040D9D0: its only reference is thunk 0040CDA0, and nothing references that thunk.
- 0040AF00: its only caller is the unreachable 00C37030.
- 0040EEF0: its 0040C710/0040C750 callers are unreferenced, but its BuildTriangles callers are live.

The JMP thunks 00C369A0 and 0040CD90, which jump to the two allocators, and 0040CDA0, which jumps to
the <20,128> free, are unreferenced. They were not given separate entries.

## Ghidra

- Renamed 0040D9D0 from `DYN_header_0040d9d0` to `Mit_PoolAllocator_20_128_Free_0040D9D0`.
- Replaced the evidence on 0040EE00, which was "Dead", with the call census.
- Both changes were applied with `tools/ghidra_annotate.py --apply`. The prior values are in
  `local/ghidra-annotations-20261018T210730Z.json`. The project was saved and the two exports
  refreshed.
