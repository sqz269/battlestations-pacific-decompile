# Application CRT dispatcher first-call body: primary review

Accepted worker `b98c3433b63671e3e109af311f1cdac1fe7af473`, complete C0522E..C05245: 24 bytes/11 instructions. Four per-byte passes agree at epoch 98/count 64,730. Root verified actual frozen Git objects, twelve finite predecessor inputs, two owned outputs and four pinned physical helpers. Restricted replay permitted only one supplied ZIP read and denied subsequent imports, directories, processes, network and mutation. All fourteen Native calls are retained: four typed requests (96 responses), seven counts, two exact byte reads and one context-zero text query. No new Source census, build or test occurred.

Root independently checked each physical opcode/ModRM/relative field and actual detailed Capstone 5.0.7 operands, along with the retained saved texts and prior Root listing gate. Physical direct calls target C051B7 at C0522F and BFBA09 at C0523C; neither target body was followed. Saved `__getptd` and `_ptiddata __cdecl __getptd(void)` remain metadata.

With entry ESP=T and ESI=U, PUSH saves U and the first CALL pushes C05234. Returned ESP=Q1, EAX=A1 and all other registers/flags/memory are unknown. Local MOV sets ESI=A1. On A1 nonzero, MOV EAX,ESI gives A1, then POP/RET consume current [Q1] and [Q1+4], leaving Q1+8 if completed. On A1 zero, PUSH 16 and CALL BFBA09 produce a second unknown returned state Q2, ESI=I2 and memory M2. POP ECX consumes M2[Q2]; MOV EAX,ESI gives I2, not an assumed zero; the remaining POP/RET consume M2[Q2+4]/M2[Q2+8], leaving Q2+12.

There is no own EBP restoration. Ordinary saved-ESI/return restoration requires explicit Q1=T-4 and, when selected, Q2=Q1-4 conditions with intact words. TLS behavior, callee preservation/cleanup, termination, original CRT/FH3 identity, actual invocation and ABI remain unproved. Worker Source IDs remain frozen-revision identity evidence, with no future current-Root equality assertion.

See `reports/cc12_application_crt_dispatch_first_call_body_primary.json` and retained `local/cc12_application_crt_dispatch_first_call_body_primary/` proof. No Source activation, startup or gameplay claim follows.
