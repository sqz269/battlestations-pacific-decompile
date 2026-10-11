# CRT dispatch body: bounded listing and operand evidence

The saved `00C07991..00C07A74` body contains 228 bytes partitioned into 84 complete instruction units. Four complete per-byte Native metadata passes match exactly at epoch 98 / 64,730 functions. Two exact 228-byte reads agree; an independent opcode/ModRM/relative decoder, detailed Capstone 5.0.7, and 84 saved context-zero instruction texts agree on every operand. This establishes conditional local effects, not a callable CRT or OriginalFH3 contract.

The operation census retains all 47 CLI calls: 20 typed requests, 24 counts, two code reads, and one text request. All 1,114 raw typed responses agree by address. Of these, 912 belong to four complete passes; the other 202 are the first interrupted attempt (128 accepted plus 13 rejected) and second interrupted attempt (61 rejected). Both atomic capture-save WinError 5 failures and the partial aggregate remain in the archive. The final complete closing pass matches; no extra byte/text reads were used.

Saved default/effective flows match with no override: 62 fallthroughs, 15 conditional jumps, two direct jumps, three direct calls, one computed call, and one RET. Every internal jump reaches a qualified whole-unit start. Direct call-site operands are `00C07997 -> 00C0522E`, `00C079F9 -> 00C069A2`, and `00C07A65 -> 00C07623`; `00C07A4A` calls EDX. The three automatically returned target-function objects per complete pass remain entirely opaque, and no external target was followed.

Let entry ESP be S and EBP/EBX/ESI/EDI be B/X/I/J. The prologue sets EBP=H=S-4 and saves B/X/I/J at S-4/-8/-C/-10. The first call pushes return `00C0799C` at S-14. Its returned ESP=Q0, EBP=F0, EAX=P, all other registers, flags, memory and cleanup are unknown. Later D=DWORD[F0+18] and C=DWORD[F0+8] therefore have conditional origins: D is incoming [S+14] and C incoming [S+4] only if F0=H and those words remain intact. The earlier Original action EBP=storeESP+20 remains unbound.

The local comparisons route through numeric constants `E06D7363`, `80000026`, `19930522`, `19930521`, mask `1FFFFFFF`, and byte masks `1`/`66`. The report records each ordered unsigned branch condition without assigning authenticated field meanings. All local reads after the first call come from its unknown returned state.

| Later call | Payload in ascending stack order after the return address | Local continuation |
| --- | --- | --- |
| `00C079F9 -> 00C069A2` | Lc, L14, D, FFFFFFFF | Unknown return ESP Q1; ADD ESP,10; EAX=1 |
| `00C07A4A -> EDX` | C, Lc, L10, L14, D, L1c, L20, U8 | Unknown return ESP Q2 / EAX A2; ADD ESP,20; carry A2 |
| `00C07A65 -> 00C07623` | C, Lc, L10, L14, D, L24, L1c, L20 | Unknown return ESP Q3; ADD ESP,20; EAX=1 |

Loffset means the DWORD frame load at its own instruction. Earlier outgoing pushes may alias later frame loads because Q0 and F0 are unrelated unknowns; the report records each load timing. U8 is zero-extended byte[F0+24] loaded before computed-route pushes; default-route L24 is a full DWORD loaded after two pushes. Every external return has unknown ESP, EBP, EAX, remaining registers, flags, memory and cleanup.

At the shared suffix, Z is Q0 without another call, Q1+10, Q2+20, or Q3+20. POP EDI/ESI/EBX/EBP and RET read current words [Z..Z+10], and final ESP=Z+14. There is no MOV ESP,EBP. Normal restoration requires first Q0=S-10, later Qi=Q0-argument_bytes, and intact saved register/return words; only under those conditions is final ESP=S+4 and the original return taken. Returned later EBP values are unused before POP EBP.

The standalone archive is `local/cc12_application_crt_dispatch_body_listing_metadata/evidence.zip` (935922 bytes, SHA-256 `7f31819b764f5ece25467478f9087049c883d7a9447d584b96c597a3d1c998ab`). It contains exactly four helper bodies, every successful/failed Native capture and stream, the pinned Root admission, ten immediate input working copies and Git blobs, and eight Git commit/tree objects. Guarded replay passes on the original and a physically distinct copy with equal output; after preloading standard libraries, it forbids new imports, non-archive reads, writes, processes, networking and native library loading. Replay manually decodes the stored bytes and checks the retained Capstone result; it does not execute Capstone again.

Frozen worker base: `439ec2b23eabcdc610c07a7e066d88d8ff938f1a`; root tree `1814a13ced84901287db56bc49ed296bf246ffb1`; src `405897b6b5d8720b023862bddeb44e463a56a37b`; include `a58d44af6071fee132ec49fdda541182d7ece633`. These are worker base/final identities, with no future Root Main equality claim or Source content census. Exactly this note and its report are owned tracked changes. No Ghidra write, build, probe, test, Source edit, ABI proof, runtime invocation or game validation occurred. OriginalFH3 remains unauthenticated.

Full operands, conditional routing, stack equations, query accounting and replay pins: [`reports/cc12_application_crt_dispatch_body_listing_metadata.json`](../reports/cc12_application_crt_dispatch_body_listing_metadata.json).
