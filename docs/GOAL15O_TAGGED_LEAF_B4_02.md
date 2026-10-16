# Tagged B4 leaf sweep 02

Eight closed whole native routines now have genuine C++ bodies in
`src/native_tagged_leaf_b4_02.cpp`, declared in the matching header. Seven read
one raw dword from backed storage; one returns all input bits unchanged.
Address-bearing C++ names describe hypotheses, not recovered symbols or classes.
The preceding packet's four source/header/document/report files are unchanged.

| Address | Complete body before RET | Native bytes | Source hypothesis |
| --- | --- | ---: | --- |
| 00b401c0 | MOV EAX,[ECX+8] | 4 | raw dword read at 8 |
| 00b40300 | MOV EAX,[ECX] | 3 | raw dword read at 0 |
| 00b40310 | MOV EAX,[ECX] | 3 | raw dword read at 0 |
| 00b40490 | MOV EAX,ECX | 3 | raw uint32 identity |
| 00b40510 | MOV EAX,[ECX+8] | 4 | raw dword read at 8 |
| 00b40550 | MOV EAX,[ECX+0x50] | 4 | raw dword read at 0x50 |
| 00b40560 | MOV EAX,[ECX+0x1c] | 4 | raw dword read at 0x1c |
| 00b40570 | MOV EAX,[ECX+0x10] | 4 | raw dword read at 0x10 |

OriginalABI is measured from each complete body: ECX is the sole consumed input;
the complete 32-bit EAX is returned. A plain RET performs no callee argument
cleanup. Incoming EAX, EDX and stack arguments are unconsumed; flags and other
registers are unchanged. `__fastcall` supplies the measured ECX input without
inventing a receiver, class or spare EDX parameter. Identity preserves every
uint32 bit pattern. The reads require real storage readable through the indicated
four-byte span. `memcpy` models raw, potentially unaligned storage without
assigning fictional object or field types; the compiler emits the exact native
single dword load in each case. There are no guards, defaults, globals or stubs.

These eight were selected from the previous packet's ignored read-only discovery
of twelve additional B4 bodies. The other four remain deferred: `00b401a0` and
`00b401b0` perform ordered triple-zero stores; `00b403f0` consumes ECX destination
and an actual stack source argument, returns destination in EAX and uses RET 4;
`00b40580` uses XOR/CMP/SETNZ to return full EAX 0 or 1. None is counted here.
No additional uninspected candidate bodies were screened for this packet.

Current Main `c5bde9e8bb30e8f9664a1d7793fa24f3cbcec428` was merged into the
worker before revalidation. A fresh index/lookup check found no function or
fragment coverage for these eight; exact hexadecimal address searches over
`src/`, `include/` and `third_party/` found no existing address-bearing source.
All eight were unleased before the exact address/file claim. Saved pseudocode
and complete assembly had been examined during discovery; fresh live body
extents and bytes were rechecked for this implementation.

Every `bsp.py ghidra` query verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base against the local
target configuration pointing at `C:/Users/sqz269/bsp.gpr`. Actual full COFF
sections, live bytes and the installed Steam PE agree: **8 whole functions,
29 bytes, 16 instructions, zero relocations**. Verification requires symbol
offset zero and whole section size equal to the entire native body, with exact
bytes and instruction operands through RET. It does not compare only a native
length prefix of a larger implementation.

Strict compilation used MSVC x86 19.51.36244 with
`/std:c++20 /MD /O2 /W4 /WX /fp:strict /Gy`. These exact simple instruction
bodies require no new tests or executable fixtures. The required warm-cache
`scripts/build.ps1` result and elapsed time are recorded separately from source
proof and full handoff time in the report. This module is deliberately
unregistered in CMake, so the registered build is baseline evidence and the
strict object supplies module coverage.

Indexed callers/callees are zero and live xrefs report no known references. This
does not establish that the entries are unused. The live caller endpoint's
`function: null` reply is retained but is inconclusive. Class identities, field
meaning, ownership and indirect callers remain unknown. There is no game
runtime validation, caller integration or binary replacement installation claim.
Existing Ghidra symbols, tags and comments were preserved; no Ghidra mutations
or CMake/packet edits were made. Ignored raw queries, exact object/listing and
proof JSON remain under `local/goal15o_tagged_leaf_b4_02/`; the verifier is
`local/goal15o_b4_proof.py`.
