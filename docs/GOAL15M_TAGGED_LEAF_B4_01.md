# Tagged B4 leaf sweep 01

Twelve previously unrepresented whole native leaf routines now have genuine C++
bodies in `src/native_tagged_leaf_b4_01.cpp`, declared in the matching header.
This packet examined the first 20 screening candidates in the independent
`00B40000..00B4FFFF` band; it does not treat tags or absent ledger records as
proof that source was missing. Existing `src/`, `include/`, and `third_party/`
coverage was searched by exact address, and the neighboring
`src/render_fx_cc13.cpp` / `CC13_RENDER_1.md` table-model coverage was inspected.
None of these twelve addresses had a function/fragment/name record or an
existing address-bearing implementation before this packet.

The `bsp.py ghidra` client verifies project `bsp`, target
`/battlestationspacific.exe`, x86 language and image base before each live batch.
`config/target.json` points at `C:/Users/sqz269/bsp.gpr` and the installed Steam
PE. Full live bodies, saved exports, installed PE bytes and actual MSVC COFF
sections agree. The report records the installed PE and COFF hashes, exact bytes,
instruction operands, section lengths and zero relocations. Every selected
routine is exactly two instructions, including its final plain `RET`.

| Address | Complete body before RET | Bytes | Source hypothesis |
| --- | --- | ---: | --- |
| 00b40000 | LEA EAX,[ECX+0x1b4] | 7 | unsigned offset 0x1b4 |
| 00b40010 | LEA EAX,[ECX+0x184] | 7 | unsigned offset 0x184 |
| 00b40020 | LEA EAX,[ECX+0x194] | 7 | unsigned offset 0x194 |
| 00b40030 | LEA EAX,[ECX+0x1ec] | 7 | unsigned offset 0x1ec |
| 00b40070 | LEA EAX,[ECX+8] | 4 | unsigned offset 8 |
| 00b40080 | MOV EAX,[ECX] | 3 | raw four-byte read at 0 |
| 00b400a0 | MOV EAX,[ECX+4] | 4 | raw four-byte read at 4 |
| 00b400d0 | MOV EAX,ECX | 3 | raw input identity |
| 00b400e0 | MOV EAX,[ECX+4] | 4 | raw four-byte read at 4 |
| 00b40100 | MOV EAX,[ECX] | 3 | raw four-byte read at 0 |
| 00b40130 | MOV EAX,[ECX+0x10] | 4 | raw four-byte read at 0x10 |
| 00b40140 | MOV EAX,ECX | 3 | raw input identity |

OriginalABI for all twelve: ECX is the sole consumed input; the complete EAX is
returned, with plain `RET` and no callee stack cleanup. EDX, incoming EAX and
stack arguments are unconsumed. The instructions preserve flags and all other
registers; they neither allocate nor call another routine. `__fastcall` is a
Win32 source spelling of this measured physical contract, not an inferred
receiver/class declaration. Descriptive address-bearing C++ names are hypotheses;
existing Ghidra names and tags remain unchanged.

Unsigned arithmetic preserves the five LEA results modulo 2^32 for every input
bit pattern, including wraparound. The two identities return all ECX bits. The
five loads accept backed `const void*` storage readable through the indicated
four-byte span. `memcpy` expresses raw storage access without inventing a field
type or alignment requirement, and the actual compiler output is the original
single dword load. No null checks, defaults, globals, fake EDX input or stubs were
introduced.

All twelve indexed caller/callee counts were zero; live xrefs reported no known
references. This is not proof the routines are unused: indirect callers and
external entry paths remain possible. The live caller endpoint returned
`No callers found for function: null`, so that endpoint is retained as raw
evidence and is not taken as additional proof of absence. Class identities,
field meaning, ownership and real game callers remain uncertain.

Eight inspected bodies were deferred: `00b40040` uses an x87 float load/return;
`00b400f0` reads an absolute global. `00b40120` (single zero store), `00b40150`
(constant 14), `00b40160` and `00b40170` (dword reads), and `00b40180` /
`00b40190` (three zero stores) were left outside the 12-entry packet cap. These
six cap deferrals have only pseudocode screening, so no missing-source or
completed-reconstruction claim is made for them. The remaining 61 candidates
were not examined.

Strict independent module compilation used MSVC x86 19.51.36244 with
`/std:c++20 /MD /O2 /W4 /WX /fp:strict /Gy`. All twelve actual function COFF
sections exactly match the whole live and installed PE bodies: **56 bytes,
24 instructions, zero relocations**. No new tests or executable fixtures were
needed for these exact simple instruction bodies. The module is deliberately
unregistered in CMake. The required `scripts/build.ps1` baseline build therefore
does not establish module coverage; its independent result and elapsed time are
recorded separately in the report. There is no gameplay validation, binary
replacement installation, or caller integration claim.

Reproduction artifacts remain under ignored
`local/goal15m_tagged_leaf_b4_01/`, with the verifier at
`local/goal15m_b4_proof.py`; the object, compiler listing, raw live queries,
installed-span comparisons and proof JSON are retained. One local verifier
attempt failed while assuming JSON output for `bsp.py ghidra bytes`; the CLI
actually prints a hexdump. The corrected parser checks all bytes and the rerun
passed. Timing fields distinguish the measured initial orientation/qualification
interval, implementation/proof interval and baseline build; worktree creation is
the available total-working lower-bound start, excluding earlier brief/setup.
