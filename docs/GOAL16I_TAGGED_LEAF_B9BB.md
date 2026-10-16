# GOAL16I tagged B9–BB leaves

This packet reconstructs 15 complete entries / 61 Native bytes / 25 instructions. All 65 emitted Source bytes / 25 instructions are accounted for. Thirteen complete bodies (59 Native bytes) are exact; two complete one-byte Native RET bodies map to three-byte Source RET0 bodies. No global cell, external service, allocator, class or abstract provider is introduced.

| Native entry | Complete operation | Native bytes |
|---|---|---:|
| 00B94B20 | No data input, plain RET | 1 |
| 00B96E50, 00B9A1C0 | No data input, RET4 | 3 each |
| 00B9A3D0 | Raw DWORD ECX+BCh to EAX | 7 |
| 00B9A4B0 | Raw DWORD ECX+14h to EAX | 4 |
| 00B9A500 | Raw BYTE ECX+10h to AL | 4 |
| 00BA7170, 00BA7180, 00BA7190 | Raw DWORD ECX+8/30/44h to EAX | 4 each |
| 00BB8FF0 | No data input, RET0Ch | 3 |
| 00BBB620 | XOR AL,AL, RET10h | 5 |
| 00BBB630 | No data input, plain RET | 1 |
| 00BBCE40 | Raw DWORD ECX+94h to EAX | 7 |
| 00BBCE50 | Raw DWORD ECX+8h to EAX | 4 |
| 00BBCE70 | Raw BYTE ECX+99h to AL | 7 |

Fresh typed metadata identifies complete single-range bodies, no thunks and no-return false for all fifteen. Every byte matches live Ghidra memory and the installed PE. Complete pseudocode/listing inspection exposed RET4/RET0C/RET10 cleanup hidden by empty pseudocode. Unread stack DWORDs are physical cleanup hypotheses, not recovered Original argument types. The constant-zero entry clears only AL; upper24 EAX bits survive and literal XOR flags/undefined AF are retained.

Nine raw getters use wrapping Win32 integer address arithmetic and fixed-size memcpy, without alignment, object extent, Boolean normalization or ownership assumptions. Actual strict17 and registered17 bodies retain the complete MOV/RET operations, including upper EAX preservation in byte forms. Existing Host callbacks and caller comments describe related operations; they do not provide these standalone physical entry bodies. Existing callers remain unchanged.

For 00B94B20 and 00BBB630, both a first naked MASM form and the final ordinary empty C++ form emit C20000 rather than Native C3. Both retire only the return address and leave registers/data/EFLAGS/x87 unchanged on normal completion. Source fetches two additional instruction bytes; codefetch faults, placement, full Original binary ABI and caller adoption remain unproved. The complete first 65-byte object and failed identity-gate evidence are retained. There is no executable prefix masking or omitted helper.

The required normal MSVC Win32 build exited zero in 18.583 seconds. All three existing CTests passed (7.55-second test interval); no new tests/probes/fixtures or Native/game execution were added. All fifteen strict/registered executable sections, every COFF symbol/metadata relocation, and the unique complete actual Core member/both public linker dictionaries are retained for independent review. [Report](../reports/goal16i_tagged_leaf_b9bb.json) records exact hashes, flags, physical contracts and qualification.

Discovery screened 22 TRIV_ names across B9/BA/BB. Seven global readers/writers remain outside this packet pending actual canonical owner/publication binding qualification. Existing borrowed-reference publication or type-info descriptions are recorded; no invented storage is substituted. Other tags in these bands remain unexamined. The inclusive clock starts at a conservative 06:36 UTC floor and includes classification, attempts/repair, build, metadata and handoff; the closing receipt records elapsed time. No controlled throughput speedup is inferred.

At author closure, independent review, primary locked Ghidra annotation/save/export/snapshot and Main integration remain pending. Descriptive names and formal type interpretations are provisional. Static body identity and build checks do not establish runtime, full fault/ABI, caller/game adoption or completion of the larger reconstruction goal.
