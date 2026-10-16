# CC12 whole raw FrameBox record copy 103

`copy_construct_native_gui_frame_box_record_00ad0c60(void*, const void*, NativeStringRawPoolContext&)` reconstructs the whole actual record copy constructor `[00ad0c60,00ad0cf6)`: 150 bytes, 58 instruction starts. Original ABI is ECX actual destination, stacked actual source, EAX same destination, RET4. The explicit C++ interface is a new Source binding ABI. The established descriptive Ghidra name `BSP_GuiRecord2C_CopyConstruct` and its hypothesis/history remain intact.

The record is actual `2c`-byte raw storage: resource DWORD `+00`, actual 8-byte pooled-name header `{length +04, data +08}`, four x87 scalar transfers at `+0c/+10/+14/+18`, and four DWORD transfers at `+1c/+20/+24/+28`. No semantic FrameBox, string, vector, or shared-pointer owner is constructed or cast over it.

Caller supplies fresh DWORD-aligned destination storage, a valid live source, and the SAME genuine `NativeStringRawPoolContext`/publication `01090aa8`/gate `01090aa4`/actual-manager cell `01090aa0`. There is no implicit owner, default profile, second pool, or provider substitution. Context metadata/cell identities, resources, buffers, and all services must remain live and satisfy the existing raw-helper domain; metadata remains stable and disjoint from destination/callee scratch, with externally serialized access. Distinct record ranges must not partially overlap. Identical name-header identity is retained only for a borrowed construction preimage, not two owning record lifetimes, and clearing must not abandon an owned destination buffer. Name-buffer overlap/copy/zero-count behavior follows the existing raw string Source boundary. Actual resource/count storage must be valid, DWORD-aligned, and disjoint from destination. The caller's current x87 environment/stack must admit the four transfers; this function supplies no control/status defaults.

The ordered body preserves all actual reloads:

1. Capture source resource, compare stable name-header identities, store resource, then clear destination length/data before the self-header branch.
2. For distinct headers, `00ad0c8d CALL 0041dd40` uses actual destination name, captured source length, and preserve=true through the real existing raw-pool overload.
3. Test CURRENT source length after resize. When nonzero, capture CURRENT DESTINATION length, CURRENT source data, then CURRENT destination data, in that order. The native site is `00ad0ca3 CALL 00bf7680`; Source uses the existing overlap-admitting `memmove` boundary and omits a zero-count standard-library invocation after still performing those actual reads. This is an explicit Source byte-copy boundary, not an independently reconstructed Native CRT memcpy implementation.
4. Execute four distinct ordered FLD/FSTP pairs at `+0c/+10/+14/+18`, then four interleaved DWORD read/store pairs at `+1c/+20/+24/+28`. The x87 operations are explicit MSVC Win32 assembly. They preserve native instruction behavior for signaling NaNs, current control/status, rounding/traps, and scalar order rather than substituting DWORD/SSE moves; no runtime FP-environment equivalence test was performed.
5. Reload CURRENT destination resource and, when nonnull, apply the Win32 interlocked increment to its actual `+04`. Return the same destination identity.

There is no local native EH frame and no new Source catch, cleanup, rollback, release callback, or failure reset. Real resize/getter exceptions propagate under the existing raw helper contract. Resource acquisition is the final increment; a record that failed earlier holds no newly acquired resource reference, even though its raw pointer was already copied. Caller must retain the SAME partial record/source/context/resource graph BEFORE entry, must not invoke `00ad0860` on a failed record, and must not replay an interrupted construction. This function provides no internal retention owner or fail-closed destructor. Successful record retirement, raw record release, vector copy/tidy, factory integration, and complete graph cleanup remain separate obligations.

Root independently reviewed the full header/Source and all 58 Original starts plus the entire optimized primary Source object. The fresh strict object contains a 163-byte C++ body with these observed offsets:

| Source object offset | Effect |
| --- | --- |
| `0e/11` | captured resource load/store |
| `16/1d`, then `24/26` | name clears, invariant identity CMP and self-header branch |
| `33` | real raw resize overload |
| `38`, then `42/44/47` | current source-length test, current DEST count/source data/destination data |
| `51` | Source `memmove`, under nonzero-count guard |
| `62/65`, `68/6b`, `6e/71`, `74/77` | four ordered FLD/FSTP pairs |
| `7a/7d`, `80/83`, `86/89`, `8c/8f` | four interleaved DWORD pairs |
| `92`, then `99` | current resource reload, Win32 intrinsic `LOCK INC [resource+4]` |
| `a2` | return |

Two emitted differences are explicit. Native CMP `00ad0c72` precedes resource/name stores; optimized Source CMP at `+24` follows the clears. It compares the same stable identities, so the supported self-header result and memory-access schedule remain unchanged; private flag/register ABI identity is not claimed. The Win32 increment compiles to `LOCK INC`, whereas Original enters the increment through its IAT cell `00ce221c`; this proves Source service behavior, not Original import dispatch or ABI identity. No SSE transfer, FP conversion/reset, hidden release, or resource snapshot across the helper replaces the required operations.

`./scripts/build.ps1` passed the existing three CTests in 9.46 seconds. The new unit remains unregistered in CMake; it was directly compiled fresh with MSVC Win32 `/MD /EHsc /std:c++20 /O2 /Gy /W4 /WX /fp:strict /permissive-`, and its optimized assembly/relocations were inspected. No new test, fixture, probe, native comparison, runtime, visual, activation, or gameplay evidence is claimed. Existing tests do not exercise this unregistered function. Private Native ABI/FH3/SEH, complete CRT behavior, and successful retirement remain unproved.

Whole-function bytes and the static increment IAT cell matched live Ghidra and the unchanged installed Original SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; the 150-byte body also matches the frozen packet-98 evidence. Each live wrapper query verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`. Only owned `00ad0c60` was annotated under the write lock; prior name/comment history is `local/ghidra-annotations-20261016T053716Z.json`. Previous comments/ledger rows were preserved, the project saved, the two exports refreshed, and the index rebuilt.

Exact Source, strict object/ASM, existing build libraries/log, Native byte proof, refreshed exports, and annotation history are sealed in `local/cc12_native_gui_frame_box_record_copy_103/manifest.json`; the companion report is `reports/cc12_native_gui_frame_box_record_copy_103.json`. No local rebuild or artifact replacement follows the final seal before integrator capture.
