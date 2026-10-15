# CC12 whole reference setter 008F0420

`set_native_scene_property_reference_008f0420` reconstructs the complete
`[008F0420,008F045A)` body: 58 bytes and 19 instructions. The name is a
descriptive hypothesis. The original byte SHA-256 is
`8dac017d064d307eb6bd6f93de1476f5882325716cb81fd81a6b65ec1c707328`.
The accepted 55-byte `008F0340` payload-copy source remains an existing
dependency; this packet neither duplicates that body nor claims new credit for it.

The physical entry receives the actual record in ECX, ignores incoming EDX,
and takes three stack DWORDs: nullable target text, opaque scalar bits and
opaque reference-kind bits. EAX returns the same record; `RET 0Ch` removes
the three arguments. The body saves/restores ESI. Other provider register and
flag behavior has only the ordinary current-provider contract.

For a nonnull old record DWORD at `+1C`, it calls the current canonical
`singleton_lifetime_free` and then clears `+1C`. This binds the original
`00BF6989 -> 00BF65AC` free path. Only after that path completes does it load
the target and call the accepted physical ECX/RET0
`duplicate_native_string_00438e40`; null is a real null-to-null provider path.
It next loads the full kind DWORD, stores the duplicate/null at `+1C`, loads
the full scalar DWORD, and stores scalar at `+18` and kind at `+08`.

The distinct write footprint is exactly 12 bytes. The nonnull-old path adds
an earlier separate zero store to the same `+1C` DWORD. No other record byte
is touched: there is no implicit type/tag 5, phase, owner, ordinal, default,
target lookup, record allocation, dispatch, insertion, clone, startup or
call to `008F0340`/`008F0310`. Scalar and kind retain every 32-bit pattern;
they are not dereferenced, filtered or replaced with defaults.

The caller supplies a genuine live writable type-5 record. Its old name is
null or a sole-owned real current-heap duplicate. The incoming target is
borrowed null or stable live NUL-terminated storage with nonwrapping
length-plus-one and address range. It remains valid through duplication and
is disjoint from the retired name, record, outputs and active frame. Allocator
address reuse after retirement is permitted. This is the successful,
nonthrowing, nonaliasing domain with no reentry, concurrent mutation, invalid
memory, fault or overflow; DF is clear for the ordinary provider domain.
If duplication fails after retirement, the old value may already have been
freed and cleared. There is no rollback or transactional/RAII claim.

Only free and the actual duplicate are direct providers. No new context
factory, callback, model, pool, key resolver or ownership graph is introduced.
Outer bag composition retains its existing genuine-record, current-heap,
retained-process, pool-generation and explicit-startup preconditions.
The wider `008F5A00` parser/declared-key branch and `008F67B0` library loader
remain incomplete and are not credited by this leaf.

One strict MSVC Win32 compile used `/MD /W4 /WX /fp:strict`, with `/O2`,
`/Ob2`, `/Oy-`, `/EHsc`, `/Gy`, `/GL-`, `/Bv` and source-dependency output.
All 82 compiler-declared source/include files were among 119 candidates
copied and hashed before compilation. The same preflight froze 34 current
readiness inputs and 64 tools, including both MSVC tool chains and both
`clui.dll` files. All 83 files in the sealed readiness family were verified
and copied. The working configuration is LF 455 bytes versus Main's CRLF
467 bytes; normalized bytes and parsed JSON are identical. An initial
preflight stopped on that metadata difference before any compile/build.
Its partial copies were preserved and checked; no compile/build was replayed.

The strict object is 1,355 bytes, with five sections, 16 symbol-table indices,
one 58-byte code section, 19 instructions, two code relocations and no
noncode relocations. Every object byte, section header, primary/auxiliary
symbol and string-table byte is accounted for. All 50 non-CALL-operand bytes
match the whole original. The only rebound ranges are `[12,16)` for canonical
free and `[31,35)` for physical duplicate. The four noncode sections contain
link directives, CodeView compiler/object metadata, an unreferenced 4-byte
AVX2 weak-value zero-fill and 40 bytes of compiler checksum records.

One normal `scripts/build.ps1` run passed the existing `reconstructed_math`
and `tool_tests` checks, 2/2 in 7.56 seconds. This new TU was deliberately
unregistered during that build; its compilation proof is the strict object.
No new tests/probes, selected-entry execution, Native differential execution,
startup, original-process execution, runtime or game validation were performed.
The retained local family is `local/WholeScenePropertyReferenceSetSource01`.
Own session UUID is `01a141c0-29a9-76c3-a341-c05f4800eeb8`; the literal tool
receipts are strict `bdc53f`/exit 0, normal launch `98a17a`/session 18514 and
natural terminal `2aee66`/exit 0, with the raw calls and own rollout prefix.

The entry's instruction/stack behavior is statically established within
the ordinary current-provider success domain. Full original ABI compatibility,
original private CRT/EH, exception/failure cleanup, wider native-class/parser
equivalence and game behavior remain unproven. Admission and shared CMake,
registry, ledger and Ghidra integration belong to the primary integrator.
