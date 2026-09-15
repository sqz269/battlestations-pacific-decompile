# Canonical CRT small-block heap initialization

Addresses: `00C11CF5..00C11D3C` inclusive, 72 bytes.

`bsp::initialize_native_crt_canonical_sbh_00c11cf5` supplies the complete
original one-word cdecl `___sbh_heap_init` entry. It uses the actual current
heap handle at `0109E1BC`, the six actual output words, and the real
`KERNEL32.dll!HeapAlloc` import. The existing BK borrowed-state interface is
unchanged. Heap bootstrap, actual caller admission and gameplay remain separate.

| Routine | Coverage | Source | Original ABI |
|---|---|---|---|
| C11CF5, preserved CRT library name `___sbh_heap_init` | complete, 72 bytes / 17 instructions | `src/native_crt_canonical_sbh_initialization.cpp` | One stacked DWORD threshold, cdecl; frameless; EAX 0/1; plain RET on both paths |

The clean worker branch was pinned to
`c8010d8fa964667a58d9cec6144d3fc9b97e7a49` before evidence capture. The
packet report is `reports/native_crt_canonical_sbh_initialization_fv.json`;
the retained local evidence is `local/canonical_sbh_initialization_fv/`.

## Native schedule and aliasing

The native body is:

```asm
C11CF5 PUSH 140h
C11CFA PUSH 0
C11CFC PUSH DWORD PTR [0109E1BCh]
C11D02 CALL DWORD PTR [00CE20F8h]
C11D08 TEST EAX,EAX
C11D0A MOV [0109ED68h],EAX
C11D0F JNZ C11D12
C11D11 RET
C11D12 MOV ECX,[ESP+4]
C11D16 AND DWORD PTR [0109E310h],0
C11D1D AND DWORD PTR [0109ED64h],0
C11D24 MOV [0109ED70h],EAX
C11D29 XOR EAX,EAX
C11D2B MOV [0109ED6Ch],ECX
C11D31 MOV DWORD PTR [0109ED74h],10h
C11D3B INC EAX
C11D3C RET
```

The real stdcall import receives the current heap handle, flags zero and
`140h` bytes; it removes its own 12 argument bytes. It supplies its actual
allocation and failure effects. The body does not clear the allocation.
The original cdecl threshold remains on the caller's stack.

`TEST` precedes publication to `109ED68`; the intervening `MOV` preserves
the flags used by `JNZ`. A null allocation is published and returns zero
immediately, without reading the threshold or touching the other five output
words. On success, the threshold is read only after that publication and the
import. Each native `AND mem,0` reads and writes the current memory word,
in order. The saved allocation in EAX is then published to `109ED70` without
reloading `109ED68`; the saved threshold in ECX is published later.

The naked Win32 assembly preserves this exact schedule, including possible
aliasing with the incoming stack word, fault ordering, import effects,
volatile register writes, flags and plain returns. It introduces no C++
signed arithmetic, speculative reads, private state/heap, extra context,
substituted provider, null repair, allocation clearing or rollback. It adds
no EBP frame, callee-save work, exception frame or synthetic success policy.
The genuine import operand is relocated by the new build; code placement,
original fault/return PCs and original IAT placement are not reproduced.

## Storage and caller boundary

DD's `GameNativeMutableCrtData` commits `0109E000` read/write and zero-fills
it as the original loader does. The seven four-byte words used here lie in
that page and are all zero in the initial PE/analysis image. That is storage
admission, not an initialized CRT heap. The caller must own the canonical
storage and a valid initialized current heap handle for this call and the
resulting allocation lifetime.

Fresh live xrefs, requested with limit 20, contain one call site: `C119F6`
inside `__heap_init`, body `C119BF..C11A18`. Its complete listing calls the
actual `HeapCreate` import at `C119D0` through `CE208C`, publishes the result
to `109E1BC` at `C119D8`, and handles a null heap before reaching this entry.
It calls `C11964`, compares returned EAX with 3, and only on the relevant
branch pushes `3F8h` at `C119F1` and calls `C11CF5` at `C119F6`. The
`C119FD POP ECX` removes exactly the one four-byte cdecl argument after
testing the returned EAX. If the initializer fails, the caller invokes the
real `HeapDestroy` import and clears the current heap word. Those caller
actions are not added to this initializer. The `C11964` body is unread in
this packet; its broader contract remains separate.

The saved local index also lists `___sbh_heap_check` as a caller. The current
live xrefs and complete caller listing are the evidence used here. The live
Ghidra prototype still says `void` parameters; its saved analysis is unchanged.
Stack setup, cleanup and complete body bytes establish the recovered ABI.

Bounded searches of the frozen worker's `src`, `include` and `tests` find the
borrowed initializer name only at its declaration and definition. Its context
also supplies `bind_native_crt_sbh_state`, which borrows already initialized
cells and performs no initialization. A separate read-only search of the
primary's current source/include/tests has the same result, retained as
`raw/current_root_all_initializer_consumers.txt`. No present C++ call site
is therefore proved as a replacement point. The proved original call site is
`C119F6`. Future integration must establish the actual initialized heap and
owner lifetime, preserve `3F8h` threshold/publication ordering and caller
rollback, and obey the process owner's single-publication constraints. Merely
calling this entry after page admission would pass an initial null heap.

## Verification and provenance

Every live query uses the existing read-only `bsp.py ghidra` interface with
autostart disabled. The client checks project name `bsp`, program
`/battlestationspacific.exe`, x86 language and image base `00400000` before
each query. Configuration points to `C:/Users/sqz269/bsp.gpr`. No prototype,
name, comment, flow, listing, script-gate, import, save or restart mutation
was made. The current full comment and library name are retained with a
desired additive annotation for the primary; it is not applied here.

Fresh live bytes match the installed executable for all 72 body bytes, the
actual import slots and seven initial state words. The PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The body SHA256 is
`b40f758cf3227398a781e6c18edbc7e2366130a0a96949531ca7b7ac5846b9ec`.
PE import metadata resolves `CE20F8` to `KERNEL32.dll!HeapAlloc`, `CE208C`
to `HeapCreate` and `CE2090` to `HeapDestroy`. Virtual zero-fill is recorded
separately from on-disk raw bytes.

Eight original PE/live seed spans pass before the `scripts/build.ps1`
strict Win32 Release builds. The first build rejected six absolute-memory
write operands with MSVC C2415 before producing the owned object or running
tests. The exact failed source, guard, tracking and full log are retained.
The correction emits only those six original instruction encodings with
`_emit`, preserving their mnemonic/address comments and adding no DS prefix.
The genuine HeapAlloc dllimport call remains normal assembly. The second
full build and both math tests pass, but the whole-body audit rejects its
71-byte body: MSVC silently encoded the numeric memory PUSH as an immediate
heap-word address. That rejected object, identical archive member/header,
source, guard and audit are retained. The final correction emits the original
six-byte `FF 35 BC E1 09 01` memory PUSH. A fresh guard precedes the third
full build, explicitly authorized for the concrete encoding defect. Its
complete body passes the byte audit. The two existing math CTests pass; no test is
added and neither test exercises this initializer. Before the owned object
exists, the guard retains exact source/header/startup, relevant owner and
borrowed provider inputs, toolset, both x86 compiler-host candidates and SDK
HeapAlloc declaration inputs. The postbuild guard checks them unchanged and
retains the exact owned compiler command/read tracking. Grouped write tracking
retains the owned source/output positions and the complete batch hashes.
The first postbuild tracking method encountered both a stale failed-batch
root and the actual successful source root. Its failed output is retained.
A separate method checks the actual owned object output, retaining both
candidate-root hashes and positions; no failed output is treated as produced.

The actual owned COFF has all 72 original body bytes after replacing only
the four-byte DIR32 operand at offset `0Fh` for `__imp__HeapAlloc@12`.
Every fixed memory operand remains exact. The audit checks all defined and
undefined owned symbols, the actual identical unique `bsp_core.lib` member,
its header, both complete linker tables and uniqueness of every owned public
symbol in both tables. Retained evidence includes the object/member, body,
member header, both linker tables, tool inputs and focused compiler tracking.
The standard `tools/verify_report_calls.py` check validates the direct caller
row; its indirect import row is resolved separately by PE/IAT/body evidence.

All failed attempts remain retained: initial index/path/search orientation
errors are transcribed in `bootstrap_attempts.txt`; an unquoted shell regex
failed before the capture script and is recorded separately; a missing local
build-wrapper path failed before any build started, then the direct build
script invocation was used. Subsequent method corrections, if any, are kept
beside their original methods and failed outputs. The final seal inventories
every local relative path except exactly the top-level `seal.json`, using
two matching passes, and distinguishes physical file hashes from Git blobs.

This is complete original-entry static evidence plus compilation and existing
math fixtures. No original/canonical SBH initializer, heap, target, native
probe, game or forced DLL execution was performed. Native faults/unwind,
heap/bootstrap lifetime, admission into actual startup, linked reachability
and gameplay remain unvalidated.
