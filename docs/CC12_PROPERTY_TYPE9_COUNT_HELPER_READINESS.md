# Type9 clone count-helper readiness (CC12)

The complete integer leaf at `008EF7F0` is independently evidenced for Root review. This audit adds zero Source bodies and zero reconstruction credit. The existing two-value semantic count rule agrees arithmetically, but has a different interface from the native ECX-record getter. Whole clone, parent/class ownership, private allocator/EH and game admission remain separate.

The whole interval `[008EF7F0,008EF81A)` is **42 bytes / 17 instructions / zero CALLs**, with three plain RET exits. Live before/after bytes equal the installed I386 PE. SHA256: `e84a191988638f684ec2d81e68f96fae767f4035f8d21a2ae736501afbeb8ce7`.

```text
8b410483e809741b83e801741683e801740333c0c3b8abaaaaaaf761248bc2c1e803c38b4124c1e802c3
```

The 17 instruction starts are recorded in the report; the complete listing and all supported query receipts are frozen under `local/t9count`. The installed executable SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The live prototype's zero parameters are misleading: assembly and callers establish ECX as the actual record root.

| Physical path | Result | Registers and final defined flags |
|---|---|---|
| Tag9/10 | `uint32([root+24]) >> 2` | EDX unchanged; SHR2 defines CF/PF/ZF/SF (`C5`); CF=input bit1; SF=0; AF/OF undefined |
| Tag11 | `high32(uint64([root+24])*AAAAAAAB) >> 3` | EDX=MUL high DWORD; SHR3 CF=high DWORD bit2; PF/ZF/SF as above; AF/OF undefined |
| Every other tag, including8 | 0 | EDX unchanged; terminal XOR has `8C5=44`; AF undefined |

All paths preserve ECX, EBX/ESI/EDI/EBP, DF and ES. No stack arguments are consumed: RET at `008EF804`, `008EF812` or `008EF819` removes only the return address. The body has no writes, external calls, heap/global reads or x87/SIMD operations. It always reads the tag DWORD at +04, and reads +24 only for tags9/10/11. It does not read DATA+20, marker+2C, owner+30, ordinal+34 or child bytes. Callers must keep these accessed fields live and stable; this is not complete owning-object proof.

Unsigned behavior covers all DWORD values. Tags9/10 floor byte count/4, maximum `3FFFFFFF`; tag11 floors byte count/12, maximum `15555555`. For tag11, `AAAAAAAB=(2^35+4)/12`; the product shifted35 floors `B/12+B/(3*2^35)`. The correction is less than1/24 and the largest remainder is11/12, so the unsigned quotient is exact. There are no positivity, divisibility, signed-negative or saturation guards. Derived examples (not runtime observations): `80000000 -> 20000000` for tag9/10; `FFFFFFFF -> 3FFFFFFF` or `15555555`; 0..3 yields0 for tag9/10, and 4..7 yields1. Nonmultiples discard their low two bits.

All narrow incoming CALLs are pinned: four functions, 20 physical E8 operands. The historical clone-only comment is stale and retained unchanged.

| Caller entry | Actual CALL starts |
|---|---|
| `008F4F60` clone | `008F51D6`, `008F521E`, `008F5266` |
| `00468600` | `0046860D`, `0046864F` |
| `008F0E20` | `008F0EED`, `008F0EFA`, `008F0F07` |
| `008F1CA0` | `008F1FD8`, `008F1FE7`, `008F2014`, `008F2065`, `008F2083`, `008F2092`, `008F20BF`, `008F210F`, `008F212D`, `008F213C`, `008F216B`, `008F21E1` |

The Type9 clone witness is the complete branch fragment `[008F51AE,008F51FA)`, 76 bytes / 24 instructions, SHA256 `0856515378d5cccafc4934e298058a4d8a0a9448c5c6effcf695fd9468fa070c`; it is not a whole-clone reconstruction. `PUSH38` at51AE / allocator CALL at51B0 obtains EDI. Clone entry4F60 saves the receiver in ESI at4F77. At51CE it reads source DATA+20, then queues `PUSH1`, DATA, ECX=ESI and helper CALL51D6. Plain helper RET preserves queued arguments. `PUSH EAX` at51DB, ECX=EDI and constructor CALL51DE therefore supply count/data/flags at stack+4/+8/+C. Source ordinal+34 is copied at51E7. `008F51BA` is a MOV inside this branch, not an entry or CALL. The null branch to528A, allocator00BF681B and class/EH/publication remain unexpanded.

`00468600` uses its ECX root and an unsigned index/count loop; its custom caller/output contract is unexpanded. The formatter's real table at8F0F50 sends indices9/10/11 directly to the three helper CALLs; the unrelated preceding tag8 arm changes ECX then jumps away. Linear listing adjacency is not a predecessor proof. Its unrelated x87 arms were observed and expansion stopped; whole-caller recovery would require Astra. `008F1CA0` restores ECX from EDI and uses counts in output/loop checks; named0094FF10/00950050 and the serializer/class body remain unexpanded. Type10/11 constructor bodies at8EF3E0/8EF460 were not queried.

The existing Source rule in `src/scene_property_bag_merge.cpp:139–151`, declared by `include/bsp/scene_property_bag_merge.hpp:88–95`, takes type and byte size as two values. It does not preserve native receiver reads, ECX/EDX or flags. Labels9/10/11 in `include/bsp/scene_property_bag.hpp:31–33` provide Source tag identities only, not numerical payload semantics.

The separate current raw Type9 constructor is `008EF360` in `native_scene_property_record_type9_four_byte_array_storage.cpp` and its header. Its two DWORD additions store scaled byte size+24; DATA+20 and marker+2C=1 occur in both branches. The header17–32 qualifies positive count `N in[1,3FFFFFFF]`, exactly4N readable bytes, nonwrapping/disjoint spans and successful current-provider DF0 operation. Faithful Type9 byte-size round-trip requires stable `B=4N>0`, divisible by4, plus separately established input/root/child lifetime and copy preconditions. B0..3 gives count0 outside that constructor Source domain; nonmultiples truncate. Arbitrary overflowing original count yields `(4N) mod2^32`, so this helper can recover only `N & 3FFFFFFF`. These arithmetic statements do not admit huge allocations or connected clone/class ownership. Marker1 is not an ownership discriminator.

A future raw leaf declaration could be `uint32_t __fastcall count_native_scene_property_array_elements_008ef7f0(const void* actual_record_ecx) noexcept`, with no stack arguments and incoming EDX unused. This is a proposal only. Root must independently review/register it and authorize a fresh whole-body/ABI validation before Source admission. No external provider is needed by this leaf; connected clone producer/allocator/EH/null/ordinal/ownership/publication require separate evidence.

Five actual Source/header files and three supported query tools have strict Main/worktree/frozen before/after hashes. Ten live query pairs, installed PE, whole42 body, clone76 fragment and all20 operands match. Every supported query calls `Client.verify()` for project bsp, program `/battlestationspacific.exe`, x86 language and image base. Configured `C:/Users/sqz269/bsp.gpr` plus the frozen saved ProgramManager corroborate physical project location; live verification does not print a physical locator. Three frozen metadata images are generation history, not immutable assertions about later Main metadata. One failed regex-only Source search is retained, followed by a new literal search; no prior query/helper stage was replayed.

The handoff is `local/t9count/receipt.json` plus `artifact_manifest.json`, schema `cc12_exact_all_files_manifest_v1`; only those exact root filenames are excluded from the recursive inventory. All remaining data, copied Source, utilities, logs, failure record and commit artifacts are included. No compiler, fixture, provider query, native execution, Ghidra mutation, Source edit, class admission or game validation occurred.
