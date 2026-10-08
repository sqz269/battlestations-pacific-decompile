# CC12: raw physical-stream constructor, 00BF50D0

## Result and contract

`raw_construct_native_physical_stream_00bf50d0` adds a distinct MSVC Win32
receiver-ABI entry to the existing physical-stream module. Its entire
43-byte, 11-instruction body is identical to installed/live Original
BF50D0..BF50FA, current production COFF, and the unique linked fixture body.
There are no calls, relocations or normalized operands. The Original literal
profile writes, CEB130 followed by D691B0, remain literal data writes in the
same order.

One fresh paired comparison on live, aligned backing and one typed Source
call passed. The normal `scripts/build.ps1` build and all three existing
CTests passed. The ordinary constructor, raw validity/seek entries, normal
numeric-profile dispatcher and all other existing Source behavior are
unchanged.

Admission is the bounded constructor on actual writable 20h-byte backing.
Incoming ECX is the receiver, EAX returns that same address, and plain RET
consumes no arguments. The second fastcall parameter occupies unused incoming
EDX, preserving a typed C++ declaration for the original register ABI.
This creates the original numeric profile values in the backing; it does
**not** publish a Source-owned function table, establish virtual lifetime,
produce a live file HANDLE, or qualify raw substream dispatch.

## Whole instruction and write sequence

```text
00BF50D0  8BC1              MOV EAX,ECX
00BF50D2  33C9              XOR ECX,ECX
00BF50D4  C70030B1CE00      MOV [EAX],00CEB130
00BF50DA  C7400401000000    MOV [EAX+04],1
00BF50E1  C700B091D600      MOV [EAX],00D691B0
00BF50E7  C74008FFFFFFFF    MOV [EAX+08],FFFFFFFF
00BF50EE  894810            MOV [EAX+10],ECX
00BF50F1  894814            MOV [EAX+14],ECX
00BF50F4  894818            MOV [EAX+18],ECX
00BF50F7  89481C            MOV [EAX+1C],ECX
00BF50FA  C3                RET
```

The base profile is written before references=1 and the final physical
profile. HANDLE+8 becomes FFFFFFFF. Position low/high at +10/+14 and cached
size low/high at +18/+1C become zero through separate DWORD writes. Field +C
and memory outside the 20h backing are untouched. The wrapper also leaves
incoming EDX and EBX/ESI/EDI/EBP unchanged; ECX is zero on return. No allocator,
pool, open, destructor, table dereference or imported provider is called.

The complete byte stream has SHA-256
`9f0a1e587ebcc5e8f5a5447de34adc30be4a0d7bc3ffdcbb605769ed4eb28716`.
Source's decorated symbol is
`?raw_construct_native_physical_stream_00bf50d0@bsp@@YIPAXPAXI@Z`.
The normal production object contains the full 43-byte section with zero
relocations. That entire object occurs once, byte-identically, in current
`bsp_core.lib`. The fixture map resolves the raw symbol to `20001000` from
that explicitly linked production object, and all 43 linked bytes occur
exactly once in the PE.

## Fresh native comparison

Evidence is in the ignored family
`local/cc12_physical_stream_raw_constructor_20261008a/run01/`.
Before any constructor execution, the driver verifies project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, the live and
installed complete Original body, the prior read-only constructor audit's
hash, whole COFF, exact archive membership and unique whole linked body.
The probe rechecks the actual loaded Source bytes against the delivered
Original bytes before either call.

Original runs from a private RX allocation at `00B70000`, a constant
32-bit code-address delta `FFF7AF30` from BF50D0. All 43 bytes remain unchanged.
There are no code fixups, import bindings or data substitutions; especially,
CEB130 and D691B0 are not shifted by that VM delta. The fixture never
dereferences either published profile. Its loaded Source entry is 20001000.

The receiver is a real, live aggregate of eight DWORD fields, aligned to 16
bytes, inside a 40h-byte object with 16-byte guards on each side. The same
actual receiver address is used for the paired calls. Before each call the
complete aggregate is reset to nonzero values, including a distinct +C
sentinel. Source and Original receive different EDX seeds. The comparison
checks all 64 bytes against both the expected image and each other:

| Field | Observed result |
| --- | --- |
| +0 profile | 00D691B0 |
| +4 references | 00000001 |
| +8 HANDLE word | FFFFFFFF |
| +C untouched sentinel | A1B2C3D4 |
| +10/+14 position | 00000000:00000000 |
| +18/+1C cached size | 00000000:00000000 |
| Both 16-byte guards | unchanged |

Both instrumented calls returned the actual receiver in EAX, zeroed ECX,
preserved their respective EDX seed, left ESP unchanged across CALL/RET,
and preserved EBX/ESI/EDI/EBP. One additional typed fastcall Source invocation
returned the same receiver and produced the same complete guarded image.
Loaded Source and Original code remained byte-identical after execution;
the private Original mapping was then released.

The final memory comparison observes final writes; exact whole-body
instruction identity establishes the intermediate CEB130/D691B0 write order.
The fixture does not claim an asynchronous observation of intermediate
states, concurrent mutation, faulting pointers, exception unwinding or
arbitrary CPU-state equivalence.

## Link failure, provenance and limits

The fresh two-object, zero-BSP-archive link failed with ten unresolved genuine
ordinary-module dependencies. `minimal_link.log` and its original response
file are preserved. No constructor executed before that failure or before
the subsequent whole-body gate. The accepted link supplies the current
module object and probe object plus existing `bsp_core.lib`,
`bsp_lua511.lib` and `bsp_zlib121.lib`, with real Windows/CRT libraries.
This is explicitly a **three-BSP-archive fixture**. No provider was invented,
stubbed, stripped or rebuilt as a surrogate. Unexecuted ordinary archive
dependencies are not newly qualified by the call-free constructor test.

The sealed input manifest records stable pre/post hashes for the installed
image and delivered Original43, six current production source/header files,
202 normal module inputs, 164 probe-consumed headers, 46 compiler/linker
files, scripts, generated compiler/linker recipes, all three supplied
archives, 19 searched libraries, the current object/probe object and final
PE. Frozen recipe/source/header/library copies retain matching hashes.
The actual Original instruction proof completed before execution; all pins
matched after the run. The existing checks were `reconstructed_math`,
`native_math_differential`, and `tool_tests`; all three passed. No tracked
test or CMake entry was added, and no previous seek/open fixture was replayed.

`reports/cc12_physical_stream_raw_constructor.json` records the exact evidence
paths, hashes and results. This closes the raw BF50D0 receiver-ABI
constructor increment identified by the table-readiness audit. The complete
Source-owned physical table, base/type and slot providers, allocation and
virtual lifetime, BF5520 alternate constructor, raw BF1090/BF10E0 composition,
and game validation remain outside this packet.
