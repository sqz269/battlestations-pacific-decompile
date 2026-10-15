# Ordinary integer-word property publication

Whole Original `008F3710..008F376D` (exclusive end) is 93 bytes and 34
instructions. The complete disk/live bytes and export instruction boundaries
agree. Its three direct CALL sites bind only canonical allocation `00BF681B`
and whole actual bag insertion `008F33F0`; it has no indirect call, x87
instruction, custom-register argument or local FS/EH registration.
Names are descriptive hypotheses, not recovered symbols/classes.

Original receives an actual bag in ECX, a key at stack+4 and a full value DWORD
at stack+8, removes eight bytes, and returns insertion's actual bag in EAX on
both paths. The new `add_native_scene_property_integer_word_008f3710` interface
takes explicit bag/key/uint32 bits and the retained concrete insertion context.
It is an ordinary C++ API, without Original transport or binary replacement
claims. Cached void/no-argument metadata was not accepted as ABI evidence.

The body allocates a genuine canonical current-CRT 38h root. For a nonnull
result, nine physical DWORD stores write tag0 at +4 **before** phase00CE89D4
at +0, then all value bits at +C, then zero at +18/+1C/+20/+24/+30/+34 in
that order. One final BYTE store writes one at +2C. Exactly37 bytes are
written;19 stay untouched at [08,0C), [10,18), [28,2C), [2D,30).
There is no root zero-fill or coalescing. Accepted raw `008EF140` stores phase
before tag and is deliberately not substituted. The phase word is opaque DATA
and is never invoked as a Source vtable.

Private noinline physical helpers each emit one pointed DWORD or BYTE store,
without other pointed accesses, internal calls or relocations. Their added
Source call/return machinery is explicit, not an Original CALL inventory claim.
The Original stack value load precedes tag publication; the new compiler reads
its stable by-value parameter for the payload helper after tag/phase helpers.
With disjoint frame/root and no reentry, faults or observer callbacks, this does
not change ordinary behavior. No Original stack, register or flag equivalence
is inferred. All root memory stores retain the exact Original order and widths.

Both paths call actual insertion, including null allocation passed as null.
No fallback, OOM handling, cleanup or transaction is added. The successful
ordinary domain requires nonthrowing providers and valid memory. Value bits
are neither converted/narrowed nor dereferenced or treated as a payload owner.
Type0 has no owned child. Successful publication transfers only the fresh root
to the actual bag under the existing complete insertion/lifetime services.

Require a genuine raw61-produced live114h bag/map+4, stable closed ASCII key
domain/current C locale, the same canonical current heap, and the same retained
concrete string/physical processes through `NativeScenePropertyInsertionContext`.
Actual E175B0 nodes, E188B4 list, raw keys and E177E4/E17654 fallback cells stay
live. Explicit startup precedes context construction; this wrapper starts
nothing. Owning graphs are finite, unique, unshared and acyclic, disjoint from
incoming root, key/query, map/node, frame and replaced graph. Externally
serialize shared-list mutation. Retire records, keys and nodes before pool
teardown. Provider failure/throw, invalid memory, overflow, aliasing, reentry
and concurrent trim remain outside the existing ordinary contract.

The first/only strict MSVC Win3219.51.36244 compile returned `c515d9/exit0` with
`/std:c++17 /MD /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict`.
The object is2157 bytes, SHA256
`d4b24732f4b1e152ca74f37ca949493df9c02a75a241a98fe6ce58695a3ab75d`.
Complete inspection covers all seven sections,24 symbol indices including
auxiliaries, three code sections/245 bytes/96 reachable instructions,14 code
relocations and zero metadata relocations. The wrapper is211B/80 instructions;
each physical helper is17B/eight instructions. Tag/phase/payload calls at
4B/58/63 precede zeros6D/77/81/8E/98/A2 and byteAC, then insertionBA.
The JE44->B4 null branch skips every root store and still inserts null.

All file bytes, headers, symbols/auxiliaries, string table, literal305B `/MD`
directives,232B debugS record,4B zero weak AVX datum,56B checksums and real GS
cookie bindings are retained. Private helper names use literal `?A0xbf0d10a8`;
no symbol/relocation normalization was used. Actual83 compiler source/include
inputs and eight tool files are pinned and frozen with current providers.

The required worker `./scripts/build.ps1` ran once: original launch
`bf12a6/session80252`, natural terminal `f6b8c2/exit0`. Both existing checks
passed in7.01 seconds. Its unrelated existing LNK4006 duplicate
`spawn_request_id_matches` warning remains in the log. The new TU is deliberately
unregistered, so the strict compile is its separate compile gate. Root owns
independent review, CMake/metadata integration and Main admission. No new tests,
probe executable, runtime replay or game launch occurred.

The local family `local/WholeScenePropertyIntegerAddSource01` freezes the entire
35-file readiness family, actual current providers, Source/compiler/build/tool
inputs and complete object review. Original returned compiler/build tool objects
are bound literally to rollout UUID `01a14050-c68c-7302-a3db-435713ee4394`, prefix
through line3207 (8657434 bytes; SHA256
`1f296126267cbf7b03f5ff614ee6a26293196c87a242f9ebf6decdfc960d8e0d`).
Static/build evidence supplies no Original ABI/EH/class/private-CRT/startup,
whole parser/loader or game/runtime equivalence.
