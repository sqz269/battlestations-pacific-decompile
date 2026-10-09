# Type10/11 future case contracts (CC12)

This is a Source-only expected-case worksheet, **Source credit0**. The eight cases and input files are proposed data, not observations. No active/accepted fixture, old helper, compiler, Native/Ghidra/provider query or runtime process was accessed. Addresses and symbols come from current Source; the audit leases no addresses and changes only this doc/report.

Current functions are `construct_native_scene_property_record_type10_raw_array_storage_008ef3e0` and `construct_native_scene_property_record_type11_raw_array_storage_008ef460`. Each uses ECX=root, unused EDX and three stack DWORDs: count/input/full flag at entry+4/+8/+C. Both return the full root in EAX and execute RET0C. Source/header complete machine qualification is primary work. Six current CPP/HPP files are frozen with strict Main/worktree/copy before/after pins.

Type10 Source29–68 doubles the DWORD count twice, yielding4*count. Type11 Source29–69 loads count before saving ESI/EDI, forms3*count with LEA, doubles to6*count, stores phase/tag and doubles to12*count. The selected counts are4/1 ->16/4 bytes and2/1 ->24/12 bytes, inside the headers' positive nonoverflow domains. Both compare only the **low flag byte** and call the genuine current adapter/memcpy only on nonzero. Four full flag words per family are distinct and have nonzero high24 bits: high-only words must retain; low80 and01 must copy. Root chooses future Source/Original/ordinary lane assignment.

| Proposed case | Count | Bytes | Full flags / low byte | Root poison | Complete opaque input hex |
|---|---:|---:|---|---|---|
| type10_retain_large | 4 | 16 | 73A9D500 / 00 | A6 | D30074A91EF2586C90B70045E12A83FD |
| type10_retain_small | 1 | 4 | C41E6800 / 00 | 59 | 6A00E731 |
| type10_copy_large | 4 | 16 | 9B2F7180 / 80 | C3 | B82900C65D81EE47A016F3009C724BD5 |
| type10_copy_small | 1 | 4 | E063AC01 / 01 | 7D | E493005F |
| type11_retain_large | 2 | 24 | D862A100 / 00 | B2 | 7C00D249A61F83EB35900064C72BF1580DAE760094E843B5 |
| type11_retain_small | 1 | 12 | 45BC7900 / 00 | 6E | 926E00D831A4FB570CE379B6 |
| type11_copy_large | 2 | 24 | A71DE380 / 80 | D9 | A518F4007DC269EB3096D14F00B8732AE65C9107DD0048AF |
| type11_copy_small | 1 | 12 | 6FC95401 / 01 | 41 | 4ED72C8500A1F963B038E27A |

All arrays contain interior NULs and meaningful later bytes. These are binary spans: copy the entire16/4 or24/12 bytes, including all bytes after NUL. No strings, integer/float/vector payload semantics or element conversion are inferred. Retain must store the exact actual input address; copy must store an actual newly allocated child base and reproduce the full selected input. Inputs remain unchanged in both branches. No pointer value is invented.

| Root offsets (hex, exclusive end) | Expected bytes |
|---|---|
| [00,04) | D4 89 CE 00, literal phase identity only |
| [04,08) | Tag10 or11 DWORD in little endian |
| [08,18) | Original poison,16 bytes |
| [18,20) | Zero,8 bytes |
| [20,24) | Actual input or actual allocated-child pointer, little endian |
| [24,28) | Exact selected byte size DWORD |
| [28,2C) | Original poison,4 bytes |
| [2C,2D) | 01 in BOTH paths |
| [2D,34) | Original poison,7 bytes, including all owner+30 DWORD bytes |
| [34,38) | Zero,4 bytes |

Thus29 bytes are written and27 preserved. The JSON provides a complete56-byte template for each case; only four actual DATA-pointer bytes are symbolic pending future allocation/caller identity. Owner+30 remains the repeated poison word, not an owner address or publication. Marker1 cannot discriminate retained versus owned child storage. Initial root poison56 and the exact W/P mask are frozen explicitly.

Retention Source leaves ECX=root, EDX=input, restores ESI/EDI and does not directly touch EBX/EBP. Final CMP0,0 defines **8D5=44 including AF0**. Copy ECX/EDX provider residuals are unconstrained; ESI/EDI are restored and EBX/EBP preservation depends on genuine providers' ordinary ABI. Both enter the proposed current-CRT domain with DF0; this worksheet extends no DF1 case or blanket ES/FPU/MXCSR guarantee across providers.

For copy, let **T be the actual target-entry ESP**, as stated by the current headers. Source final ADD ESP,10h operates on `a=uint32(T-24)` and produces `s=uint32(a+16)=uint32(T-8)`. Its mask8D5 includes CF/PF/AF/ZF/SF/OF; AF is `((a^16^s)&10h)!=0`, with the other ordinary unsigned ADD flags described in JSON. No T or copy flag result is fabricated. If a future raw capture records pre-CALL/argument-top/outer ESP instead, Root must derive the exact mapping to target-entry T before applying this formula. No capture size, field offset, frame placement, guard schema, GPR seed or observed address has been selected here.

Each successful four-case family expects **four actual56-byte current-canonical root allocations plus two copied children**, six allocate/free pairs: Type10 children16 and4 bytes; Type11 children24 and12. A possible future order is allocate four roots, construct both retain then both copy cases, inspect all live storage, free both actual child bases once, then free all four root bases once. Borrowed inputs are never child-freed. Each new child/root stays exclusive/live/undisposed until its explicit cleanup; no retired memory read or globally unique-address requirement is introduced. Manual raw current-free cleanup must not also invoke a recursive release/destructor that would free the same child.

Each CPP12–16 adapter requests object/actual_bytes/actual_bytes. `singleton_lifetime.cpp:51–63` allocates host_bytes with malloc, invokes _callnewh on failure and retries or throws;65–67 frees with std::free. This is a Source contract, not loaded CRT/provider identity proof. Success only is proposed; zero/overflow/alias/failure/reentry and naked-frame EH are excluded. No noexcept is invented for these constructors.

Future Root work includes real raw/public ordinary callers, actual Native/emitted byte/ABI/RET/flag checks, whole fresh retained helper/import/cookie/stack/weak/cold gates, input/external guards, capture/serialization schema, loaded current allocator/free/memcpy provider bookends and a sole fresh process. Four requested56-byte roots do not establish adjacent heap guards or metadata proof. Private Native CRT, phase dispatch, class ownership, whole clone/publication and game domains remain separate. Existing independent Source qualifications are unchanged; Main metadata is frozen generation history, not a forever-live pin.

The local handoff is `local/t1011cases/receipt.json` and `artifact_manifest.json`. Exact recursive sealing excludes only those two root filenames and includes all Source copies, eight opaque inputs, matrix, utilities, logs, stops and commit records.
