# Dyn recursive profiler report

Complete normal `00C320D0..00C321AB` is reconstructed: **220 bytes, 74
instructions**, including recursion and the indirect sink. Descriptive name
`report_profile_node_00c320d0` is a hypothesis. This contributes one in-house
body; the reused formatter/storage and vendor runtime contribute zero credit.
The new C++ interface is unwired and has no runtime or binary-replacement claim.

## Target and original ABI

Live queries used the verifying BSP client for project `bsp`,
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`. Pseudocode was read before the
complete assembly. All 220 live bytes match the installed PE, SHA-256
`442ca9b5b112ec95c7324c9765ba069196762087ad5a4e5a04843c1ab612bc28`.
The installed PE hash is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Native ECX is the current 48h node, copied to ESI. Stack arguments are the sink
pointer, pointer to the first position word and pointer to the shared second
word. Saved EBX/EBP/ESI/EDI are restored; `C321A9` is `RET0Ch`. Native locals
occupy 408h bytes, including an uninitialized 400h text buffer and captured
tick words. Original stack/frame, exception/fault and exhaustion identity are
not supplied by the new cdecl API.

The sink boundary at `C32157` supplies ECX = current first word, EDX = second
word after its increment, and one stack text pointer. The caller does not
remove that argument; the sink must pop four bytes. The explicit fastcall
function pointer preserves this observed boundary and raw word bits. Position
names/meaning are hypotheses; no logger, native class or default sink is made.

## Actual storage and input custody

The source borrows existing `DynProfileNodeStorage` and
`DynProfileScopeContext::profile_slot_0109e9f8`. It does not allocate or replace
an owner/node. The node's +4/+8 words are child data/count; +38h is the captured
64-bit tick value, and +40h is the raw call count.

The actual `NativeLegacySboStringStorage` starts at node+10h, including its
untouched leading word. Buffer+14h and capacity+28h are checked against the
existing type. The source retains the pre-FP capacity comparison and later
pointer selection directly; invoking a data accessor would reread capacity.

Each invocation first reads the real publication cell, then owner+10h for its
denominator node. The existing scope implementation uses that word as cache
slot 1. It is not replaced by the supplied node, a root shortcut or a zero
fallback. The denominator's +38h value is read at the native x87 epoch. The
actual double cell `00D7A220` is a mandatory reference; original bytes
`0000000000005940` are 100.0. It is never captured as a default numeric mode.
Bindings must remain valid while callbacks may change the published owner or
live fields. Native validity/non-null/tree invariants remain required.

## Floating point, division and formatting

`C320E4` compares unsigned capacity against 16 before the low/high tick reads.
Those two captured words are copied into local storage and loaded as signed
64-bit by FILD. The exact ambient sequence is retained:

```text
FILD captured qword; FMUL actual D7A220 double;
FILD denominator-node+38h qword; FDIVP;
select the name using CF from the earlier capacity comparison;
FSTP double directly to the printf argument stack.
```

There is no host double arithmetic, extra floating reload/spill, PC24/PC53
assumption, control-word change, MXCSR change, zero-denominator guard or
percentage clamp. The original single double spill occurs before integer
division; its raw words stay on the printf argument stack through that call.

The captured high/low tick words are passed to the real x86 `__alldiv` helper
with signed divisor 1000. Only quotient low EAX is pushed as Time's `%d` bits;
there is no narrowing cast or invented time unit/frequency conversion. Calls
is read from +40h afterward and pushed as its raw `%d` word.

The installed helper `00BF7D40..00BF7DE9` is 170 bytes, ending `C2 10 00`.
The selected genuine `/MD` `msvcrt.lib` member `lldiv.obj` defines `__alldiv`
with 170 code bytes and zero relocations; those bytes are **identical** to the
installed helper. Code SHA-256:
`8964dcf0d769cb61dd4354feac00491ae8c1b34eb9627a4c5ec2d4350243d395`.
The private declaration is ASM-only symbol selection: four argument dwords,
callee RET16 and quotient EDX:EAX. It is not a callable cdecl helper API.
No vendor body or substitute implementation is added.

The actual existing `cc15_dyn_live::format_string_1024_0040cfc0` is composed.
It uses `vsprintf_s(buffer,400h,format,va)`; its existing vendor/context
qualification is inherited. The recovered literal is copied verbatim:
`%s: Calls:%d Time:%d Perc:%.2f%%%%` (four percent characters at the end).
The formatter result is ignored, as native; no new truncation, error recovery,
string owner or allocation is introduced. The text buffer remains alive until
this invocation returns, including its recursive children.

## Callback and live traversal epochs

After formatting, shared second +=20 wraps at 32 bits. Its value is read
before the first position value, then the sink receives both and the buffer.
The printf arguments remain live through these position operations; they are
removed before the text push, and the sink pops that four-byte argument.
This preserves aliases between the two words. Child data/count are read only
after the sink, initially data before count.

Each child receives a freshly initialized local first = current parent first
+20, the same second reference and the same genuine bindings. The child slot
is read after that local value is formed. After recursion, count is reloaded
before data, the old cursor advances four, and end is recomputed as wrapping
data+count*4. Termination is pointer equality, including wrapped count/product
cases; the cursor is not reset to new data and no count/less/cycle guard is
added. Callback changes to the publication cell, positions and child fields
therefore remain visible at the observed epochs. Concurrent machine-level
RMW/fault/stack identity is outside this explicit C++ interface.

## Original call evidence and validation

| Native site | Target | Contract |
|---|---|---|
| 00C3211F | 00BF7D40 | captured signed qword / 1000, callee pops16 |
| 00C32134 | 0040CFC0 | actual 1024-byte formatter, raw varargs |
| 00C3218A | 00C320D0 | actual child recursion |

Strict MSVC Win32 object compilation passes `/std:c++17 /EHsc /fp:strict
/O2 /Oy- /MD /W4 /WX`. A focused standard-Release variant using `/Ob2` without
`/Oy-` also passes and retains the EBP-based local addressing required by the
manual argument stack. Neither check links or executes an executable.

Object inspection finds exactly the five FP instructions above, with FDIVP
bytes `DE F9` and the single FSTP `[ESP]`. It confirms the real runtime/helper
relocations, raw formatter pushes, fastcall sink registers/cleanup and live
recursion reloads. Evidence and selected existing-runtime artifacts are under
`local/dyn_profile_report`; hashes are recorded in the report. No new tests,
probes, games, shared metadata or Ghidra mutations were made.

Original callers `C35390`, `C37B10`, `C40FC0` remain metadata-only source
obligations; their xrefs do not establish a live incoming binding. Root owns
registration, locked annotations, common build/link, genuine caller/sink/
publication-cell/scale custody and runtime or dead-chain conclusions.
