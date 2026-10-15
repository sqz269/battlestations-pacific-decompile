# Native enum symbol reader, CC12

`read_native_enum_symbols_008f31a0` supplies the complete ordinary enum-assignment
reader over six accepted raw providers. It borrows a genuine current-CRT19Ch
CEnum and an already-live838h tokenizer. The cached name
`BSP_PropTreeLibrary_SkipBlock` is provisional and does not describe this body:
there is no generic skip or nested-brace traversal.

## Original evidence and interface

Typed metadata verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base00400000 at modification148.
The complete returning non-thunk body is `[008F31A0,008F3261)`, 193 bytes, one
exact range. Complete live bytes match disk, SHA-256
`6763202fd868a30f6ca442557d46af4b7d1fc6729812ce7acc25fe97fa84a474`.
No listing repair is needed. Literal bytes also match disk:
CE4CD4=`7D00` (`}` then NUL), CE55BC=`3D00` (`=` then NUL),
CE5698=`3B00` (`;` then NUL).

ECX is the outer enum, entry stack+4 is the live tokenizer, and cleanup is RET4.
AL forwards the final008D9930 expect-close result; upper EAX is unspecified.
Original saves four registers without reserving locals. Its flag-output address
`ESP+14h` before pushing the pointer is the low byte of the incoming tokenizer
argument slot. EBX retains the tokenizer pointer across that overwrite. Source
uses a C++ byte object and explicit contexts, without claiming that Original
register/stack/segment/DF transport or layout.

## Whole loop and error schedule

Entry peeks once. Each termination check compares at most two bytes against
the actual closing binding, stopping before the second read if the first byte
differs, as REPE CMPSB/count2 does. Only the exact `}NUL` token stops the loop.

Each iteration re-peeks, then copies the whole current name including NUL into
the process scratch role00F89758. It consumes the name, expects `=`, stores
that AL into the byte output, and always invokes the raw integer reader using
the same byte. The integer reader overwrites the byte itself. Neither flag
controls this outer loop or its declaration.

Next it peeks and compares against `;` through whole00438E10. Only an equal
token is consumed. It always calls whole008F2E40 with the scratch name and
the integer reader's result, including zero on a failed conversion. It then
peeks again before repeating the two-byte termination check. On exit it calls
expect-close and returns that result.

The Original does not expect an opening brace here. It has no EOF, nesting,
failure exit, iteration limit, or success-only declaration gate. Unterminated
input may not terminate. Recovery, cache, position, line, token-EOF, duplicate
retention, metadata stores, and raw string ownership remain the accepted direct
providers' full schedules; none is replaced with a parser facade.

## Providers and genuine bindings

| Entry | Accepted Source |
| --- | --- |
| 00438E10 | `compare_insensitive_00438e10`, entity_identity.cpp |
| 008D8960 / 008D8A70 | raw consume/peek, native_scene_tokenizer.cpp |
| 008D9930 | raw expect, native_scene_token_text.cpp |
| 008D9AD0 | raw integer reader, native_scene_token_values.cpp |
| 008F2E40 | whole declaration, native_enum_symbol_declaration.cpp |

There is no computed or indirect call in this whole Original body. The accepted
integer Source's explicit indirect boundary is
`NativeSceneTokenValueCalls::scan_decimal_00bf7533`; admission binds its existing
concrete default body to current-CRT `std::sscanf`, not arbitrary callbacks.
Comparison uses the accepted current-CRT `_stricmp`; raw scanning/recovery uses
the accepted current-CRT `strchr` boundary. Ancestors were not reconstructed anew.

Context borrows the accepted token-value and enum-declaration contexts, three
actual immutable punctuator roles, and genuine writable process scratchF89758.
The scratch lies in writable `.data`, but its precise independent buffer extent
was not recovered. Caller must supply enough valid genuine storage for each
name; no local replacement array, capacity, default cell, or owner is invented.
Names must also fit the Original400h token buffers. Both contexts use canonical
raw-string authority; enum declaration retains real symbol poolE17578 and
distinct E186ED/E17654 fallback roles, without table/property pool substitution.

Admit the inherited successful stable/disjoint/coherent current-CRT owner,
pool, live-tokenizer, finite-chain, nonoverflowing ASCII/C-locale domains.
Shared scratch excludes reentry/concurrent mutation. Named incomplete file
constructor008D9CF0 is a separate nonprerequisite: this entry borrows an existing
tokenizer. Startup and application binding of owners/scratch remain separate.

## Static validation and limits

Strict MSVC Win32 `/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict /std:c++17`
passed in original chunk86b00d, exit0, with filename-only compiler output.
All emitted code and relocations were inspected: main COMDAT C3h, helper1Fh,
and eleven REL32 calls to only the six accepted entries. The emitted helper and
loop preserve short-circuit second-byte reads, forward NUL-inclusive scratch
copy, all re-peeks, flag store/read-int order, optional-semicolon consume,
unconditional declaration, and final AL forwarding. Compiler placement of the
C++ byte object is not an Original stack-layout claim.

The report records actual own-worktree build launch/natural-terminal receipts
and existing CTest results: session74528, launch ec4370, natural terminal
0d8b07/exit0, both existing suites passed. The build recorded LNK4006 for the
existing duplicate Lua-spawn definition. The new TU remains unregistered; its compile
evidence is the strict standalone object. No new tests, probes, target runtime,
Ghidra, ledger, shared header, CMake, or application wiring changes were made.
`local/WholeEnumSymbolReaderSource01/` retains the readiness gate, complete
Original bytes/constants/metadata, full Source/header copies, object/dumps,
and exact original paired tool receipts. These establish complete ordinary
Source/static evidence; Original ABI, FS/EH/privateCRT/locale/class/runtime/game
equivalence and runtime/startup/application admission remain false.
