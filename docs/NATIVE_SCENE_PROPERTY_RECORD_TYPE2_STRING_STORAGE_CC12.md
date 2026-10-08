# Physical type-2 string storage constructor (CC12)

Worker implementation and its fresh qualified fixture passed. Source admission remains **0**, with Root independent validation, main CMake/build integration, ledgers and saved-analysis publication pending. Only `008EF1B0` is reconstructed.

## Native evidence and interface

The complete native range `[008EF1B0,008EF1EC)` is **60 bytes / 20 instructions**, SHA-256 `be944adca54cbd0702bc89a12e88ab0c8ac84f306bb8c14188266b8f70ae5a04`. Live Ghidra bytes, original PE bytes and saved listing starts agree before/after. Each live batch verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the worker made no Ghidra mutation. The saved prototype `FUN_008ef1b0 undefined(void)` remained incomplete.

The new function is `void* __fastcall construct_native_scene_property_record_type2_string_storage_008ef1b0(void* actual_record_ecx, void* unused_edx, const char* actual_text_stack)`, without `noexcept`. Actual ECX is the receiver; EDX is unused on entry; the sole stack DWORD is nullable borrowed text. Full EAX returns the receiver and `RET4` removes that DWORD. EBX/EBP/ESI/EDI survive normal return. The actual text argument is loaded before any receiver store.

Only CALL operand `[39,43)` binds `duplicate_native_string_00438e40`. All other **56 bytes** remain literal. The linked constructor is `0x27001000`, its child `0x27001080`. The phase word `00CE89D4` is identity only, never a Source vtable.

The unchanged child Source SHA-256 is `48c86a1ad7763861d099d909a7a8b314b3258ee9b5508f702f093a64188946e0`, header `b101c630eba5844d1f5d21530aef1200c0969b0139fb72de1d3870e75814f477`. Root independently admitted and published that physical 57-byte ECX/RET0 provider at `bb285149e`. The legacy CDECL `bsp::duplicate_00438e40` is not this provider. Dependency metadata is frozen at private base `7c79302e7276263fe41dc079c589920336c7a918`.

## Stores and ownership

Receiver storage must be fresh, unowned and writable for **56 bytes**, disjoint from borrowed text and live frames. Never overwrite an owned copy. Nonnull borrowed text stays readable through NUL, with representable length plus one and no address wrap.

| Offset | Width | Value and order |
| --- | --- | --- |
| `+00` | DWORD | Literal `00CE89D4`, before child |
| `+04` | DWORD | Tag `2`, before child |
| `+18,+1C,+20,+24,+30` | DWORD each | Zero, before child |
| `+34` | DWORD | Zero, after successful child |
| `+0C` | DWORD | Actual current-owned copy or null |
| `+2C` | BYTE | `1`, including null input |

Exactly **37 bytes are written / 19 preserved**. Preserved intervals are `[08,0C)`, `[10,18)`, `[28,2C)`, `[2D,30)`. The complete literal body establishes order. Seven pre-call DWORD writes already occur if the child fails; no strong failure guarantee is claimed.

Inspect a nonnull copy while live and free it exactly once via `singleton_lifetime_free` before caller-owned receiver disposal, reuse or lifetime end. Null creates no copy. The fixture uses a genuine aligned96 stack object with pre16/root56/post24, resetting it only after prior copy cleanup. The stack receiver has no invented heap free, and freed copies are never read. Child allocation/copy providers are real current services.

## Nested stack and flags

Let `T` be raw caller ESP before CALL, pointing at actual stack text. Constructor entry is `T-4`, saved ESI `T-8`, saved EDI `T-12`, child entry `T-16`. Child saved EBX/ESI/EDI are `T-20/T-24/T-28`; size argument `T-32`. Copy arguments destination/source/length are `T-44/T-40/T-36`, copy entry `T-48`, copy return **`T-44`**. Final `ADD ESP,16` yields **`T-28`**, child `RET0` returns to `T-12`, outer `RET4` to **`T+4`**.

After the child, only MOV/POP/RET preserve its flags. Null XOR gives masked `0x8C5 = 0x44`; AF is undefined/excluded, ECX is zero, EDX retains incoming bits. EAX returns the nonzero receiver despite ZF1. Nonnull defined arithmetic flags under `0x8D5`, including AF, equal **`ADD32(T-44,16)`**. Nonnull ECX/EDX are volatile and unasserted. DF must be clear for current CRT use; no FP/MXCSR/segment or whole-EFLAGS equality is claimed.

## Fresh fixture evidence

Family: `J:\PROG\battlestations-pacific-decompile-cc12_type2_string_storage_source\local\cc12_type2_string_storage_worker20261008a`. Four fresh TUs compile constructor, unchanged admitted duplicate, unchanged canonical allocator/free, and new ignored probe. Strict MSVC Win32 `/MD /O2 /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`, child `/Oi-`, `/OPT:NOICF`, embedded `asInvoker` manifest passed. No old objects/BSP archives. Main CMake registration and `scripts/build.ps1` remain Root's integration step.

Fresh COFF/map/linked bytes derive **11 complete spans** (bytes/instructions): constructor60/20, duplicate57/30, adapter61/17, allocator90/34, free6/1, ordinary18/5, rawcaller140/52, bad_alloc24/6, cookie14/4, memcpy6/1, coldthrow6/1. Full relocations/targets are retained. The library-owned cookie span includes successful RET and cold failure tail JMP, excluding alignment. Cold bytes do not claim exception execution or closure.

Before **zero target entries**, every code span and actual malloc/free/_callnewh/memcpy IAT is checked against `GetProcAddress`, MEM_IMAGE owner, I386 headers, physical export RVA, mapped NT path, held file identity, complete file SHA-256 and relocated export prefix. Heap APIs resolve to actual SysWOW64 UCRT; memcpy to distinct actual SysWOW64 VCRUNTIME140. Prefix equality alone is insufficient. All module/physical/code checks repeat afterward.

**Five calls passed**: bound Original null/nonempty, Source null/nonempty, ordinary Source nonempty. Original changes only its one CALL operand to the **same actual child** used by Source; no original private duplicate/allocator/copy runs. Exactly **three live copied strings and three completed current frees**. All96 receiver bytes and borrowed48 bytes/guards pass, including preserved bytes and flag+2C1 for null. Copied input is `95e24b81631900`. Raw ECX, actual stack slot/value, before/after GPRs, ESP and flags are captured. The same receiver, text and raw frame are reused only after cleanup. Equal allocator addresses happened but are never required.

Raw flags `[582, 518, 582, 518]` mask to `[0x44,0x04,0x44,0x04]`. Actual `T=1725588`, copy return `1725544`, final inner ESP `1725560` and outer ESP `1725592` satisfy the nested contract. A metadata-only decoder rechecks saved receiver images and captures without rerunning the executable.

Post checks preserve **40 explicit inputs, 185 consumed headers, seven libraries, 890 prior artifacts**. Prepare/build/static/native/post each ran once. No old recipe/helper module/process was imported/executed. A subsequent metadata-only inline draft had a nested-string syntax error before writing files; its failure receipt is retained. Corrected helper syntax was compiled before write/run; no successful stage replay followed.

Core seal covers95 artifacts, SHA-256 `2858befb84cd21738535bfdbb1128be6b11282fd58c305e5a4fa3229008c1b1a`. Final exact inventory adds metadata helpers, failure receipt, handoff and commit message. Post/seal console output remains outside the family.

## Admission limits

Worker build/fixture evidence is qualified to the same actual current provider. Root independent admission and combined main build remain pending. Original private CRT/new-handler/OOM/EH, native class ownership, phase dispatch, destructor, whole Clone, consumer class lifetime and gameplay are not admitted. No game validation is claimed.
