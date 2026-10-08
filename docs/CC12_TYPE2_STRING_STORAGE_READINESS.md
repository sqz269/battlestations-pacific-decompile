# CC12 type-2 string storage readiness

This update is **conditional**, with **Source 0 / ready Source 0**. Root must independently admit the physical string duplicate `00438E40` before registering constructor work. The frozen duplicate worker implementation supplies a concrete current allocation/copy/free prerequisite; it does not establish native class ownership, destruction or private CRT/EH behavior.

The constructor `[008EF1B0,008EF1EC)` is complete **60 bytes / 20 instructions**, SHA-256 `be944adca54cbd0702bc89a12e88ab0c8ac84f306bb8c14188266b8f70ae5a04`. Current live bytes, installed PE and complete saved listing starts agree. Its sole CALL at `008EF1D6` targets `00438E40`. A future Source body may rebind only operand **[39,43)** to the real physical duplicate; all other **56 bytes** must remain literal. The saved `undefined FUN_008ef1b0(void)` prototype remains incomplete and was not changed.

ECX is the actual fresh/unowned writable 56-byte receiver. The sole stacked DWORD is nullable borrowed text; after PUSH ESI, `MOV ECX,[ESP+8]` reads it before any receiver store. ESI retains the receiver. Normal return places the actual receiver in full EAX and uses `RET4`; the child pointer/null is stored separately at receiver+0x0C. Incoming EDX is unused. Constructor ESI/EDI saves and the complete child/provider ABI preserve nonvolatiles.

| Ordered writes | Value |
| --- | --- |
| Before duplicate: DWORD +00, +04 | Literal `00CE89D4`, tag `2` |
| Before duplicate: DWORD +18, +1C, +20, +24, +30 | Zero |
| After normal duplicate return: DWORD +34 | Zero from preserved constructor EDI |
| Then DWORD +0C | Actual current-owned copy pointer or null |
| Then BYTE +2C | One, including the null-input case |

Normal return writes 37 bytes and preserves 19 inside the minimum 56: `[08,0C)`, `[10,18)`, `[28,2C)`, `[2D,30)`. No integer payload is written at +08. The phase word is literal identity only; it must not become a host vtable or be dispatched/destructed through. Its old frozen cell association with `004E6730` is retained as incomplete historical context.

The receiver, text and active call frame must be disjoint. Nonnull text must remain a live readable NUL-terminated byte object with representable `n+1` and no address wrap. The operation must not overwrite a live owning record: it does not release any preexisting +0C pointer. After observing a real nonnull current-owned output, the caller frees it exactly once using unchanged `singleton_lifetime_free`, before releasing, reusing or ending the lifetime of its receiver storage. No copy reads follow that free. Null produces no child-free obligation despite flag byte +2C=1. Receiver storage ownership remains with its actual caller; this manual disposal is not native destructor proof.

The nested flags differ from a standalone duplicate fixture. Let raw constructor caller ESP=T point to the actual text argument immediately before CALL:

| Event | ESP |
| --- | --- |
| Constructor entry / after its PUSH ESI and PUSH EDI | T-4 / T-12 |
| Nested duplicate entry | T-16 |
| Inner memcpy entry / return | T-48 / T-44 |
| Inner final `ADD ESP,16` result | T-28 |
| Duplicate RET0 back to constructor | T-12 |
| Constructor two pops / RET4 | T-4 / T+4 |

Only MOV/POP/RET follow the duplicate CALL, so final flags come from the child. For null text, inner XOR defines CF/OF/SF=0 and ZF/PF=1; use mask `8C5h`, exclude undefined AF, and do not derive flags from returned receiver EAX. ECX stays zero and EDX retains its incoming value on that complete null route. For nonnull text, defined arithmetic flags come from actual **(T-44)+16**, mask `8D5h`; ECX/EDX remain provider-volatile and must not be asserted. DF must be clear for the normal current CRT domain; blanket FP/MXCSR/segment/DF and EH preservation remain outside the claim.

The prerequisite is frozen worker commit `abad4d43a48ac905be66cb02d103ed693c148cb9`, with `bsp::duplicate_native_string_00438e40(const char*)` physically taking ECX, no stacked arguments and RET0. Source SHA-256 is `48c86a1ad7763861d099d909a7a8b314b3258ee9b5508f702f093a64188946e0`; header is `b101c630eba5844d1f5d21530aef1200c0969b0139fb72de1d3870e75814f477`. Its genuine canonical size adapter/current UCRT allocation/free and actual VCRUNTIME memcpy gates are frozen worker evidence. Root admission and the future constructor process require their own validation. The older plain-CDECL `bsp::duplicate_00438e40` cannot supply this physical call.

The only caller refreshed here is Clone arm `[008F5083,008F50B1)`, **46 bytes / 13 instructions**, SHA-256 `2ff7155d5b1de7707a24cf7bda49ae4c13bef37dedd598d6ef9ac086a36a2978`. It loads source+0x0C into EDX, pushes that pointer, sets ECX from EAX and calls the constructor at `008F5099`; after return it overwrites destination+0x34 from source+0x34. Earlier destination allocation/source ownership, other clone arms, full Clone/EH and destruction remain uninspected. This is one genuine caller witness, not whole-parent closure.

A conditional future packet would own exactly:

- `include/bsp/native_scene_property_record_type2_string_storage.hpp`
- `src/native_scene_property_record_type2_string_storage.cpp`
- `docs/NATIVE_SCENE_PROPERTY_RECORD_TYPE2_STRING_STORAGE_CC12.md`
- `reports/native_scene_property_record_type2_string_storage_cc12.json`

Its proposed API is `void* __fastcall bsp::construct_native_scene_property_record_type2_string_storage_008ef1b0(void* actual_record_ecx, void* unused_edx, const char* actual_text_stack);`, without `noexcept`. The unused EDX formal preserves the genuine single stack argument. No owner/destructor wrapper, fake provider, callback or substituted phase pointer is proposed.

The future fixture needs four fresh TUs: constructor, unchanged admitted physical duplicate/adapter, unchanged canonical allocation/free and new probe. Raw boundOriginal and Source each receive null and the same guarded nonempty text using genuine fresh/unowned receiver storage; one ordinary Source nonnull call verifies compiler lowering. Five constructor entries would produce three real owned copy outputs and three matching child frees. Check all 37 written/19 preserved bytes, EAX/RET4, raw frame/nonvolatiles/defined nested flags, borrowed text and guards, live owned-copy bytes, and lifetime-safe disposal. Numerical allocated-address equality is never required. A declared guarded stack receiver needs no fabricated root-free function; actual heap receiver ownership, if used, releases it only after child cleanup.

Before any constructor/duplicate entry, fresh actual IAT/export/mapped-I386/physical-file gates must bind the child's heap operations to matching UCRT and memcpy to VCRUNTIME140, using full identities rather than identical entry prefixes. Full constructor/child/helper/caller/Original spans, inputs, toolchain, consumed headers/libraries, providers and prior families need bookends. No original-private-CRT execution, forced failure/reentry/overlap or class/game claim follows. Pre-call receiver writes already affect 28 bytes; failure unwind/rollback is unadmitted.

The bounded typed graph has **13 nodes + 10 edges = 23/24**, retaining named incomplete clone, phase/destruction, original private allocator/copy/release and owning-class/EH boundaries. Root-owned `00438E40` was not live-queried. No other property leaf was queried.

Evidence: `local/cc12_type2_string_storage_readiness20261008a/{proposal.json,readiness_receipt.json,artifact_manifest.json}` and [tracked report](../reports/cc12_type2_string_storage_readiness.json). Both native spans, all 25 explicit input pins and 765 older artifacts passed bookends. The duplicate family's exact 95-file inventory, old scalar comments/receipts and all historical Source/config associations remain unchanged. There was no Source/header/config/ledger/CMake change, compilation, probe/native execution, Ghidra mutation or agent spawn. Root alone may register work after the duplicate is actually admitted.
