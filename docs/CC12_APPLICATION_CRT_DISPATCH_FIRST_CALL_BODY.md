# CRT dispatch first-call body: bounded instruction evidence

The complete saved `C0522E..C05245` body is 24 bytes and 11 instructions. Four per-byte metadata passes agree raw-by-address at epoch 98 / 64,730 functions. Two exact 24-byte reads match; an independent manual opcode/ModRM/relative decoder, actual detailed Capstone 5.0.7 and all eleven saved context-zero instruction texts agree. Saved `__getptd` remains an analysis label, without authenticated TLS or calling-convention meaning.

The initial listing gate was accepted by Root before operands. Every address belongs to C0522E and every unit is complete, with no gaps or overlaps. Saved default/effective flows agree with override NONE: seven fallthroughs, two direct calls, one conditional jump, and one RET. Calls are `C0522F -> C051B7` and `C0523C -> BFBA09`; the conditional jump goes from C05238 to qualified start C05242. The two automatically returned target-function objects per pass remain entirely opaque.

Let body-entry ESP=T, ESI=U and EBP=H. PUSH ESI writes U at T-4. The first CALL writes return C05234 at T-8 and reaches numeric C051B7 with ESP=T-8. If it returns, ESP=Q1, EBP=B1, EAX=A1, ESI=I1, all other registers, flags, memory M1 and cleanup are unknown. MOV ESI,EAX then sets ESI=A1; TEST/JNZ selects the nonzero path.

If A1 is nonzero, MOV EAX,ESI carries A1. POP ESI reads M1[Q1], RET reads M1[Q1+4], and final ESP=Q1+8; EBP=B1. Normal restoration requires Q1=T-4 plus intact saved U and the original return word. Those conditions yield final ESP=T+4; they are not established callee behavior.

If A1 is zero, PUSH 10h stores that DWORD at Q1-4. The second CALL writes return C05241 at Q1-8 and reaches BFBA09 with ESI=0 and ESP=Q1-8. Any returned ESP=Q2, EBP=B2, EAX=A2, ESI=I2, other registers, flags, memory M2 and cleanup are unknown. No actual return or termination behavior is inferred.

After a second return, POP ECX reads M2[Q2]; MOV EAX,ESI overwrites A2 with I2. ESI is not assumed preserved as zero across this call, so the body return value is unknown I2 on this route. POP ESI reads M2[Q2+4], RET reads M2[Q2+8], and final ESP=Q2+Ch; EBP=B2. Normal restoration requires Q1=T-4, Q2=Q1-4, and intact saved/return words. POP ECX equals 10h only if the corresponding argument word also remains intact.

There is no local EBP save, read, write or restoration. Returned EBP therefore comes from the selected last external return. This does not bind the parent frame, authenticate a TLS pointer or external ABI, establish the Original action EBP=storeESP+20h requirement, or authorize OriginalFH3 invocation.

All fourteen retained CLI calls succeeded: four typed requests (96 raw responses), seven counts, two exact code reads and one context-zero text request. No data/stack bytes, external body, prototype repeat, decompile, disassembly or xref query was performed. Native identity/count closes unchanged.

The standalone archive is `local/cc12_application_crt_dispatch_first_call_body/evidence.zip` (562832 bytes; SHA-256 `eb8eac058d0691596e3dbc81215338ba38a56165991f0abb6d499e8f90da987b`). It physically contains exactly four helpers, every Native capture/stream, the accepted Root gate, twelve positive predecessor working inputs and Git blobs, and eight actual commit/tree objects. Strict guarded replay passes on the original and a physically distinct copy with equal results. After standard-library preload, only supplied-ZIP reads are allowed; imports, other filesystem access, mutations, network, processes and native-library loading are denied. Replay independently decodes stored bytes and checks the retained actual Capstone result; it does not reload Capstone.

Frozen worker base `184cf9919a6432c80de99b51a8345635ad2a4e94`; tree `960c98913d6979c37dfb83d18672749bcc5c125c`; src `af8efebaa30365fa0f7f2f591c45861a55b4b1b2`; include `09add3be594e63edcce6a16ec041ed965441e898`. These are worker base/final identities only, without a Source census or current/future Root equality assertion. Prior proofs remain unchanged. Exactly this note and its report are owned tracked outputs. No Ghidra/Source modification, build, new test, probe, execution or game validation occurred.

Complete operands, conditional equations and evidence pins: [`reports/cc12_application_crt_dispatch_first_call_body.json`](../reports/cc12_application_crt_dispatch_first_call_body.json).
