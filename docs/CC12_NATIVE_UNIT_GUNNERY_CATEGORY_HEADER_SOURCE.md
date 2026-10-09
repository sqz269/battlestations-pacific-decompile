# Native unit gunnery category header constructor candidate

Root admitted the bounded Source change after the fresh 509-input Win32 build and emitted review. Current validation and limits are in docs/CC12_SELF_REFRESH_RENDERER_SOURCE_PRIMARY_REVIEW.md and its JSON receipt. The worker candidate statements below describe its earlier unregistered/unbuilt capture. Whole receiver/model, Native ABI/EH and gameplay admission remain separate.

Packet: `cc12_native_unit_gunnery_category_header_constructor`.

The candidate supplies the complete `00952640..0095264C` leaf through `construct_native_unit_gunnery_category_header_00952640`. It writes three actual DWORD fields at offsets 0/4/8 and preserves the observed register behavior. It is not yet registered, built, emitted-code reviewed or admitted. Root owns those steps.

## Original evidence

The complete body is 13 bytes and six instructions:

| Address | Bytes | Operation |
| --- | --- | --- |
| `00952640` | `8B C1` | `MOV EAX,ECX` |
| `00952642` | `33 C9` | `XOR ECX,ECX` |
| `00952644` | `89 08` | `MOV DWORD PTR [EAX],ECX` |
| `00952646` | `89 48 04` | `MOV DWORD PTR [EAX+4],ECX` |
| `00952649` | `89 48 08` | `MOV DWORD PTR [EAX+8],ECX` |
| `0095264C` | `C3` | `RET` |

Body SHA-256: `1e09d34a9de7a48bd061703ef73f341df8596595468b282c5f19c36b6bf0b98d`.

The original PE, fresh live bytes and prior saved byte capture agree. All six fresh live instruction starts agree with the PE decoding and the prior fresh saved instruction capture. Verified CLI queries used `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. A fresh prototype query still reports no function at this address. The local instruction capture is not represented as a historical function export or a newly created Ghidra function.

The preceding shared-base readiness packet supplies the direct placement witness: the parent passes this callback to the first array call with storage R+394h, stride Ch and count Ch. That parent was not reopened for this packet; its reviewed report is pinned. No further Native body or global cell was opened.

## Source contract

`NativeUnitGunneryCategoryHeaderStorage` contains exactly three `std::uint32_t` fields: `word_00`, `word_04` and `word_08`. These names record offsets without asserting recovered count, pointer, link or ownership semantics. Compile-time assertions cover its standard layout, 0Ch size, four-byte alignment and exact field offsets.

The public MSVC Win32 `__fastcall` entry accepts one pointer in ECX. The implementation follows the repository's existing naked-leaf convention and spells out all six instructions:

- Valid actual writable storage must back all three DWORD fields. There is no pointer, alignment or range check and no read of prior contents.
- EAX receives entry ECX before ECX is cleared. Stores occur at +0, +4 and +8 in that order.
- Normal return leaves EAX equal to the original receiver and ECX zero. EDX and every nonvolatile register are untouched.
- There are no stack arguments; RET consumes only the return address. The body makes no call, branch, allocation, global access or exception-chain change.
- XOR produces the ordinary zero-result arithmetic flags; later MOV/RET instructions do not change them. The function adds no `noexcept` declaration or exception handler. Invalid backing can fault during any store; the earlier stores have already happened.

The Source storage is the actual three-field header, not a complete unit owner or a semantic copy of one. The leaf does not release previously populated storage. It supplies no array iterator, destructor, callback dispatcher, profile table, model, numbering service or receiver publication.

## Verification and remaining admission

The worker reviewed the complete header, implementation, document, report and all six Native instructions. Static evidence comparison covers all 13 original bytes and all six starts. The normal configured build and emitted/indexed object review have not run for this candidate. Layout assertions are present but not described as having passed compilation.

Root must register the source, run the normal Win32 build and existing required checks, inspect the complete emitted body and its indexed Core definition, and assign any Source admission. There are no worker tests or probe executables, and there is no Original-ABI, application, startup or gameplay validation.

The worktree intentionally retains preceding readiness commit `fa9178ebcbd15369d5aa46357b73f5f572c300b5` as its parent while Root integrates it. The candidate changes only the four leased files. CMake, ledgers and GPR state are untouched.

## Frozen context

Root retained Source121 in `local/cc12_self_refresh_renderer_Source_primary/Source121_retained`, with manifest `Source121_frozen.json` frozen at 2026-10-09 21:32:54.126656 UTC. This worker verified all 121 retained raw source inputs, all 121 corresponding worker LF-normalized inputs and all four retained artifact pins. The only worker raw difference is the existing CRLF/LF report difference.

Those artifact matches describe the frozen historical snapshot. Root's current artifacts are being rebuilt for other providers; this packet neither reads them for comparison nor claims their equality to Source121. Earlier startup smoke and object/check receipts supply no new credit for this constructor candidate.
